#include "Body.h"

#include <InfantryClass.h>
#include <BulletClass.h>
#include <HouseClass.h>
#include <ScenarioClass.h>
#include <AnimTypeClass.h>
#include <AnimClass.h>
#include <AircraftClass.h>
#include <BitFont.h>
#include <SuperClass.h>
#include <ThemeClass.h>
#include <TagTypeClass.h>

#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/SWType/Body.h>

#include <New/Type/TemperatureTypeClass.h>
#include <New/Armor/Armor.h>

#include <Utilities/Helpers.Alex.h>
#include <Utilities/EnumFunctions.h>

#include <Misc/FlyingStrings.h>
#include <Ext/Scenario/Body.h>

void WarheadTypeExt::ExtData::Detonate(TechnoClass* pOwner, HouseClass* pHouse, BulletExt::ExtData* pBulletExt, CoordStruct coords)
{
	auto const pBullet = pBulletExt ? pBulletExt->OwnerObject() : nullptr;

	if (pOwner && pBulletExt)
	{
		auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pOwner->GetTechnoType());

		if (pTypeExt != nullptr && pBulletExt != nullptr)
		{
			if (pTypeExt->Interceptor && pBulletExt->IsInterceptor)
				this->InterceptBullets(pOwner, pBullet->WeaponType, coords);
		}

		if (pBullet->Target != nullptr && pBullet->WeaponType != nullptr)
		{
			auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pBullet->WeaponType);

			if (pWeaponExt->BlinkWeapon)
			{
				TechnoExt::ProcessBlinkWeapon(pOwner, pBullet->Target, pBullet->WeaponType);
			}
		}
		if (pTypeExt->Interceptor && pBulletExt->IsInterceptor)
			this->InterceptBullets(pOwner, pBullet->WeaponType, coords);
	}

	if (pHouse)
	{
		if (this->BigGap)
		{
			for (auto pOtherHouse : *HouseClass::Array)
			{
				if (pOtherHouse->IsControlledByHuman() &&   // Not AI
					!pOtherHouse->IsObserver() &&         // Not Observer
					!pOtherHouse->Defeated &&             // Not Defeated
					pOtherHouse != pHouse &&              // Not pThisHouse
					!pHouse->IsAlliedWith(pOtherHouse))   // Not Allied
				{
					pOtherHouse->ReshroudMap();
				}
			}
		}

		if (this->SpySat)
			MapClass::Instance->Reveal(pHouse);

		if (this->TransactMoney)
		{
			pHouse->TransactMoney(this->TransactMoney);

			if (this->TransactMoney_Display)
			{
				auto displayCoords = this->TransactMoney_Display_AtFirer ? (pOwner ? pOwner->Location : coords) : coords;
				FlyingStrings::AddMoneyString(this->TransactMoney, pHouse, this->TransactMoney_Display_Houses, displayCoords, this->TransactMoney_Display_Offset);
			}
		}

		for (const auto pSWType : this->LaunchSW)
		{
			if (const auto pSuper = pHouse->Supers.GetItem(SuperWeaponTypeClass::Array->FindItemIndex(pSWType)))
			{
				const auto pSWExt = SWTypeExt::ExtMap.Find(pSWType);
				const auto cell = CellClass::Coord2Cell(coords);
				if ((pSWExt && pSuper->IsCharged && pHouse->CanTransactMoney(pSWExt->Money_Amount)) || !this->LaunchSW_RealLaunch)
				{
					if (this->LaunchSW_IgnoreInhibitors || !pSWExt->HasInhibitor(pHouse, cell)
					&& (this->LaunchSW_IgnoreDesignators || pSWExt->HasDesignator(pHouse, cell)))
					{
						pSuper->SetReadiness(true);
						pSuper->Launch(cell, true);
						pSuper->Reset();
					}
				}
			}
		}

		auto pData = TechnoExt::ExtMap.Find(pOwner);
		if (pOwner && !pOwner->InLimbo && pOwner->IsAlive && pBullet && pData->PassengerList[0] != nullptr && pData->AllowCreatPassenger)
		{
			pData->AllowCreatPassenger = false;
			FootClass* CreatPassenger = pData->PassengerList[0];
			CoordStruct CreatPassengerlocation = pData->PassengerlocationList[0];
			int facing = pOwner->PrimaryFacing.Current().GetValue<16>();

			if (pBulletExt->InterceptedStatus == InterceptedStatus::Intercepted)
			{
				CreatPassenger->UnInit();
			}
			else
			{
				WeaponTypeExt::ExtData* pWeaponExt = nullptr;

				if (pBullet != nullptr && pBullet->GetWeaponType() != nullptr)
					pWeaponExt = WeaponTypeExt::ExtMap.Find(pBullet->GetWeaponType());

				auto const pTargetTechno = abstract_cast<TechnoClass*>(pBullet->Target);

				if (pTargetTechno != nullptr
					&& pWeaponExt->PassengerTransport_MoveToTarget
					&& EnumFunctions::CanTargetHouse(pWeaponExt->PassengerTransport_MoveToTargetAllowHouses, pHouse, pTargetTechno->Owner))
				{
					auto const pBuilding = abstract_cast<BuildingClass*>(pBullet->Target);
					if (pBullet->Target->WhatAmI() == AbstractType::Building && pBuilding->Type->MaxNumberOccupants > 0)
					{
						InfantryClass* pInfantry = static_cast<InfantryClass*>(CreatPassenger);
						if (pBuilding->Occupants.Count < pBuilding->Type->MaxNumberOccupants && CreatPassenger->WhatAmI() == AbstractType::Infantry && pInfantry->Type->Occupier)
						{
							CreatPassenger->Transporter = nullptr;
							pBuilding->Occupants.AddItem(pInfantry);
						}
						else
						{
							CreatPassenger->Transporter = nullptr;
							CreatPassenger->Unlimbo(CreatPassengerlocation, static_cast<DirType>(facing));
							CreatPassenger->QueueMission(Mission::Stop, true);
							CreatPassenger->ForceMission(Mission::Guard);
							CreatPassenger->Guard();
						}
					}
					else
					{
						if (pTargetTechno->GetTechnoType()->Passengers > 0)
						{
							if (CreatPassenger->GetTechnoType()->Size <= (pTargetTechno->GetTechnoType()->Passengers - pTargetTechno->Passengers.GetTotalSize()) && CreatPassenger->GetTechnoType()->Size <= pTargetTechno->GetTechnoType()->SizeLimit)
							{
								FootClass* pTargetPassenger = pTargetTechno->Passengers.GetFirstPassenger();
								ObjectClass* pLastTargetPassenger = nullptr;

								while (pTargetPassenger)
								{
									pLastTargetPassenger = pTargetPassenger;
									pTargetPassenger = static_cast<FootClass*>(pTargetPassenger->NextObject);
								}

								if (pLastTargetPassenger)
									pLastTargetPassenger->NextObject = CreatPassenger;
								else
									pTargetTechno->Passengers.FirstPassenger = CreatPassenger;

								++pTargetTechno->Passengers.NumPassengers;

								CreatPassenger->Transporter = nullptr;
								CreatPassenger->ForceMission(Mission::Stop);
								CreatPassenger->Guard();
							}
							else
							{
								CreatPassenger->Transporter = nullptr;
								CreatPassenger->Unlimbo(CreatPassengerlocation, static_cast<DirType>(facing));
								CreatPassenger->QueueMission(Mission::Stop, true);
								CreatPassenger->ForceMission(Mission::Guard);
								CreatPassenger->Guard();
							}
						}
						else
						{
							CreatPassenger->Transporter = nullptr;
							CreatPassenger->Unlimbo(CreatPassengerlocation, static_cast<DirType>(facing));
							CreatPassenger->QueueMission(Mission::Stop, true);
							CreatPassenger->ForceMission(Mission::Guard);
							CreatPassenger->Guard();
						}
					}
				}
				else
				{
					CreatPassenger->Transporter = nullptr;
					CreatPassenger->Unlimbo(CreatPassengerlocation, static_cast<DirType>(facing));
					CreatPassenger->QueueMission(Mission::Stop, true);
					CreatPassenger->ForceMission(Mission::Guard);
					CreatPassenger->Guard();
				}
			}
			pData->AllowChangePassenger = true;
		}
	}

	if (strcmp(this->Theme.data(), ""))
	{
		HouseClass* player = HouseClass::CurrentPlayer;

		if (strcmp(this->Theme.data(), "-1") != 0)
			ScenarioExt::Global()->LastTheme = ThemeClass::Instance->CurrentTheme;

		if (player)
		{
			if (pHouse->IsControlledByCurrentPlayer() && pHouse == player)
			{
				auto ThememIndex = ThemeClass::Instance->FindIndex(Theme.data());

				if (strcmp(this->Theme.data(), "-1") == 0 && ScenarioExt::Global()->LastTheme >= 0)
				{
					ThememIndex = ScenarioExt::Global()->LastTheme;
					ScenarioExt::Global()->LastTheme = -1;
				}

				if (auto IsQueue = this->Theme_Queue)
				{
					ThemeClass::Instance->Queue(ThememIndex);
				}
				else
				{
					ThemeClass::Instance->Play(ThememIndex);
					ThemeClass::Instance->Queue(ThememIndex);
				}
			}
		}
	}

	this->HasCrit = false;
	this->RandomBuffer = ScenarioClass::Instance->Random.RandomDouble();

	WeaponTypeExt::ExtData* pWeaponExt = nullptr;

	if (pBullet != nullptr && pBullet->GetWeaponType() != nullptr)
		pWeaponExt = WeaponTypeExt::ExtMap.Find(pBullet->GetWeaponType());

	// List all Warheads here that respect CellSpread
	const bool isCellSpreadWarhead =
		this->RemoveDisguise ||
		this->RemoveMindControl ||
		this->Crit_Chance ||
		this->GattlingStage > 0 ||
		this->GattlingRateUp != 0 ||
		this->ReloadAmmo != 0 ||
		this->Converts ||
		this->Shield_Break ||
		this->Shield_Respawn_Duration > 0 ||
		this->Shield_SelfHealing_Duration > 0 ||
		this->Shield_AttachTypes.size() > 0 ||
		this->Shield_RemoveTypes.size() > 0 ||
		this->PaintBall_Duration > 0 ||
		this->Transact ||
		this->ClearPassengers ||
		this->DamagePassengers ||
		this->ReleasePassengers ||
		this->DisableTurn_Duration > 0 ||
		this->DodgeAttach_Duration > 0 ||
		this->MoveDamageAttach_Duration > 0 ||
		this->StopDamageAttach_Duration > 0 ||
		this->ChangeOwner ||
		this->AttachTag ||
		this->DamageLimitAttach_Duration > 0 ||
		!this->AttachEffects.empty() ||
		!this->Temperature.empty() ||
		this->Directional.Get(RulesExt::Global()->DirectionalWarhead) ||
		(//WeaponType
			pWeaponExt != nullptr &&
			pWeaponExt->InvBlinkWeapon.Get()
		)
		;

	bool bulletWasIntercepted = pBulletExt && pBulletExt->InterceptedStatus == InterceptedStatus::Intercepted;
	const float cellSpread = this->OwnerObject()->CellSpread;

	if (cellSpread && isCellSpreadWarhead)
	{
		this->DetonateOnAllUnits(pHouse, coords, cellSpread, pOwner, pBullet);

		if (this->Transact)
			this->TransactOnAllUnits(pHouse, coords, cellSpread, pOwner, this);
	}
	else if (pBullet && isCellSpreadWarhead)
	{
		if (auto pTarget = abstract_cast<TechnoClass*>(pBullet->Target))
		{
			this->DetonateOnOneUnit(pHouse, pTarget, pOwner, pBullet, bulletWasIntercepted);

			if (pWeaponExt != nullptr && pWeaponExt->InvBlinkWeapon)
				this->ApplyInvBlink(pOwner, pHouse, std::vector<TechnoClass*>(1, pTarget), pWeaponExt);

			if (this->Transact && (pOwner == nullptr || this->CanTargetHouse(pOwner->GetOwningHouse(), pTarget)))
				this->TransactOnOneUnit(pTarget, pOwner, 1);
		}
		else if (auto pCell = abstract_cast<CellClass*>(pBullet->Target))
		{
			this->DetonateOnCell(pHouse, pCell, pOwner);
		}
	}
}

void WarheadTypeExt::ExtData::DetonateOnOneUnit(HouseClass* pHouse, TechnoClass* pTarget, TechnoClass* pOwner, BulletClass* pBullet, bool bulletWasIntercepted)
{
	if (!pTarget || pTarget->InLimbo || !pTarget->IsAlive || !pTarget->Health)
		return;

	if (!this->CanTargetHouse(pHouse, pTarget))
		return;

	auto pTargetExt = TechnoExt::ExtMap.Find(pTarget);

	if (CustomArmor::GetVersus(this, pTargetExt->GetArmorIdx(this->OwnerObject())) == 0.0)
		return;

	this->ApplyShieldModifiers(pTarget);

	if (this->RemoveDisguise)
		this->ApplyRemoveDisguiseToInf(pHouse, pTarget);

	if (this->RemoveMindControl)
		this->ApplyRemoveMindControl(pHouse, pTarget);

	if (this->Crit_Chance && (!this->Crit_SuppressWhenIntercepted || !bulletWasIntercepted))
		this->ApplyCrit(pHouse, pTarget, pOwner);

	if (this->GattlingStage > 0)
		this->ApplyGattlingStage(pTarget, this->GattlingStage);

	if (this->GattlingRateUp != 0)
		this->ApplyGattlingRateUp(pTarget, this->GattlingRateUp);

	if (this->ReloadAmmo != 0)
		this->ApplyReloadAmmo(pTarget, this->ReloadAmmo);

	if (this->Converts)
		this->ApplyUpgrade(pHouse, pTarget);

	if (this->PaintBall_Duration > 0)
		this->ApplyPaintBall(pTarget);

	if (this->DisableTurn_Duration > 0)
		this->ApplyDisableTurn(pTarget);

	if (this->ClearPassengers || this->ReleasePassengers || this->DamagePassengers)
	{
		if (pBullet != nullptr)
			this->ApplyAffectPassenger(pTarget, pBullet->GetWeaponType(), pBullet);
	}

	if (this->DodgeAttach_Duration > 0)
		this->ApplyCanDodge(pTarget);

	if (this->MoveDamageAttach_Duration > 0)
		this->ApplyMoveDamage(pTarget);

	if (this->StopDamageAttach_Duration > 0)
		this->ApplyStopDamage(pTarget);

	if (this->ChangeOwner)
		this->ApplyChangeOwner(pHouse, pTarget);

	if (this->AttachTag)
		this->ApplyAttachTag(pTarget);

	if (this->DamageLimitAttach_Duration > 0)
		this->ApplyCanLimitDamage(pTarget);

	if (!this->AttachEffects.empty())
		this->ApplyAttachEffects(pOwner, pTarget);

	if (!this->Temperature.empty())
		this->ApplyTemperature(pTarget);
}

void WarheadTypeExt::ExtData::DetonateOnAllUnits(HouseClass* pHouse, const CoordStruct coords, const float cellSpread, TechnoClass* pOwner, BulletClass* pBullet, bool bulletWasIntercepted)
{
	std::vector<TechnoClass*> items(std::move(Helpers::Alex::getCellSpreadItems(coords, cellSpread, true)));

	for (auto pTarget : items)
	{
		this->DetonateOnOneUnit(pHouse, pTarget, pOwner, pBullet, bulletWasIntercepted);
	}

	if (pBullet != nullptr)
	{
		if (auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pBullet->WeaponType))
		{
			if (pWeaponExt->InvBlinkWeapon)
				this->ApplyInvBlink(pOwner, pHouse, items, pWeaponExt);
		}

		if (this->Directional.Get(RulesExt::Global()->DirectionalWarhead))
			this->ApplyDirectional(pBullet);
	}
}

void WarheadTypeExt::ExtData::DetonateOnCell(HouseClass* pHouse, CellClass* pTarget, TechnoClass* pOwner)
{
	if (!pTarget)
		return;

	if (this->Crit_Chance)
		this->ApplyCrit(pHouse, pTarget, pOwner);
}

void WarheadTypeExt::ExtData::ApplyShieldModifiers(TechnoClass* pTarget)
{
	if (auto pExt = TechnoExt::ExtMap.Find(pTarget))
	{
		bool canAffectTarget = GeneralUtils::GetWarheadVersusArmor(this->OwnerObject(), pTarget->GetTechnoType()->Armor) != 0.0;

		int shieldIndex = -1;
		double ratio = 1.0;

		// Remove shield.
		if (pExt->Shield && canAffectTarget)
		{
			const auto shieldType = pExt->Shield->GetType();
			shieldIndex = this->Shield_RemoveTypes.IndexOf(shieldType);

			if (shieldIndex >= 0)
			{
				ratio = pExt->Shield->GetHealthRatio();
				pExt->CurrentShieldType = ShieldTypeClass::FindOrAllocate(NONE_STR);
				pExt->Shield->KillAnim();
				pExt->Shield = nullptr;
			}
		}

		// Attach shield.
		if (canAffectTarget && Shield_AttachTypes.size() > 0)
		{
			ShieldTypeClass* shieldType = nullptr;

			if (this->Shield_ReplaceOnly)
			{
				if (shieldIndex >= 0)
					shieldType = Shield_AttachTypes[Math::min(shieldIndex, static_cast<int>(Shield_AttachTypes.size()) - 1)];
			}
			else
			{
				shieldType = Shield_AttachTypes.size() > 0 ? Shield_AttachTypes[0] : nullptr;
			}

			if (shieldType)
			{
				if (shieldType->Strength && (!pExt->Shield || (this->Shield_ReplaceNonRespawning && pExt->Shield->IsBrokenAndNonRespawning() &&
					pExt->Shield->GetFramesSinceLastBroken() >= this->Shield_MinimumReplaceDelay)))
				{
					pExt->CurrentShieldType = shieldType;
					pExt->Shield = std::make_unique<ShieldClass>(pTarget, true);

					if (this->Shield_ReplaceOnly && this->Shield_InheritStateOnReplace)
					{
						pExt->Shield->SetHP((int)(shieldType->Strength * ratio));

						if (pExt->Shield->GetHP() == 0)
							pExt->Shield->SetRespawn(shieldType->Respawn_Rate, shieldType->Respawn, shieldType->Respawn_Rate, true);
					}
				}
			}
		}

		// Apply other modifiers.
		if (pExt->Shield)
		{
			if (this->Shield_AffectTypes.size() > 0 && !this->Shield_AffectTypes.Contains(pExt->Shield->GetType()))
				return;

			if (this->Shield_Break && pExt->Shield->IsActive())
				pExt->Shield->BreakShield(this->Shield_BreakAnim.Get(nullptr), this->Shield_BreakWeapon.Get(nullptr));

			if (this->Shield_Respawn_Duration > 0)
				pExt->Shield->SetRespawn(this->Shield_Respawn_Duration, this->Shield_Respawn_Amount, this->Shield_Respawn_Rate, this->Shield_Respawn_ResetTimer);

			if (this->Shield_SelfHealing_Duration > 0)
			{
				double amount = this->Shield_SelfHealing_Amount.Get(pExt->Shield->GetType()->SelfHealing);
				pExt->Shield->SetSelfHealing(this->Shield_SelfHealing_Duration, amount, this->Shield_SelfHealing_Rate, this->Shield_SelfHealing_ResetTimer);
			}
		}
	}
}

void WarheadTypeExt::ExtData::ApplyRemoveMindControl(HouseClass* pHouse, TechnoClass* pTarget)
{
	if (auto pController = pTarget->MindControlledBy)
		pTarget->MindControlledBy->CaptureManager->Free(pTarget);
}

void WarheadTypeExt::ExtData::ApplyRemoveDisguiseToInf(HouseClass* pHouse, TechnoClass* pTarget)
{
	if (pTarget->WhatAmI() == AbstractType::Infantry)
	{
		auto pInf = abstract_cast<InfantryClass*>(pTarget);
		if (pInf->IsDisguised())
			pInf->Disguised = false;
	}
}

void WarheadTypeExt::ExtData::ApplyCrit(HouseClass* pHouse, AbstractClass* pTarget, TechnoClass* pOwner)
{
	double dice;

	if (this->Crit_ApplyChancePerTarget)
		dice = ScenarioClass::Instance->Random.RandomDouble();
	else
		dice = this->RandomBuffer;

	if (this->Crit_Chance < dice)
		return;

	auto pTechno = abstract_cast<TechnoClass*>(pTarget);
	CellClass* pTargetCell = nullptr;

	if (pTechno)
	{
		if (auto pTypeExt = TechnoTypeExt::ExtMap.Find(pTechno->GetTechnoType()))
		{
			if (pTypeExt->ImmuneToCrit)
				return;

			if (pTechno->GetHealthPercentage() > this->Crit_AffectBelowPercent)
				return;
		}

		pTargetCell = pTechno->GetCell();
	}
	else if (auto pCell = abstract_cast<CellClass*>(pTarget))
	{
		pTargetCell = pCell;
	}

	if (!pTechno && pTargetCell && !this->Crit_Warhead.isset())
		return;

	if (pTargetCell && !EnumFunctions::IsCellEligible(pTargetCell, this->Crit_Affects, true))

		if (pTechno && !EnumFunctions::IsTechnoEligible(pTechno, this->Crit_Affects))
			return;

	this->HasCrit = true;

	if (this->Crit_AnimOnAffectedTargets && this->Crit_AnimList.size())
	{
		int idx = this->OwnerObject()->EMEffect || this->Crit_AnimList_PickRandom.Get(this->AnimList_PickRandom) ?
			ScenarioClass::Instance->Random.RandomRanged(0, this->Crit_AnimList.size() - 1) : 0;

		if (pTechno || pTargetCell)
			GameCreate<AnimClass>(this->Crit_AnimList[idx], pTechno ? pTechno->Location : pTargetCell->GetCoords());
	}

	auto damage = this->Crit_ExtraDamage.Get();

	if (pTechno)
	{
		if (this->Crit_Warhead.isset())
		{
			WarheadTypeExt::DetonateAt(this->Crit_Warhead.Get(), pTechno, pOwner, damage);
		}
		else
		{
			pTechno->TakeDamage(damage, pHouse, pOwner, this->OwnerObject(), false, false);
		}
	}
	else if (pTargetCell && this->Crit_Warhead.isset())
	{
		WarheadTypeExt::DetonateAt(this->Crit_Warhead.Get(), pTargetCell->GetCoords(), pOwner, damage);
	}
}

void WarheadTypeExt::ExtData::ApplyGattlingStage(TechnoClass* pTarget, int Stage)
{
	auto pData = pTarget->GetTechnoType();
	if (pData->IsGattling)
	{
		// if exceeds, pick the largest stage
		if (Stage > pData->WeaponStages)
		{
			Stage = pData->WeaponStages;
		}

		pTarget->CurrentGattlingStage = Stage - 1;
		if (Stage == 1)
		{
			pTarget->GattlingValue = 0;
			pTarget->unknown_bool_4B8 = false;
		}
		else
		{
			pTarget->GattlingValue = pTarget->Veterancy.IsElite() ? pData->EliteStage[Stage - 2] : pData->WeaponStage[Stage - 2];
			pTarget->unknown_bool_4B8 = true;
		}
	}

	auto const pDataExt = TechnoTypeExt::ExtMap.Find(pData);
	auto pTargetData = TechnoExt::ExtMap.Find(pTarget);
	if (pDataExt->IsExtendGattling)
	{
		if (Stage > pData->WeaponStages)
		{
			Stage = pData->WeaponStages;
		}

		pTargetData->GattlingStage = Stage - 1;
		if (Stage == 1)
		{
			pTargetData->GattlingCount = 0;
		}
		else
		{
			auto& setstages = pTargetData->GattlingStages;
			pTargetData->GattlingCount = setstages[Stage - 2].GetItem(0);
		}
	}
}

void WarheadTypeExt::ExtData::ApplyGattlingRateUp(TechnoClass* pTarget, int RateUp)
{
	auto pData = pTarget->GetTechnoType();
	if (pData->IsGattling)
	{
		auto curValue = pTarget->GattlingValue + RateUp;
		auto maxValue = pTarget->Veterancy.IsElite() ? pData->EliteStage[pData->WeaponStages - 1] : pData->WeaponStage[pData->WeaponStages - 1];

		//set current weapon stage manually
		if (curValue <= 0)
		{
			pTarget->GattlingValue = 0;
			pTarget->CurrentGattlingStage = 0;
			pTarget->unknown_bool_4B8 = false;
		}
		else if (curValue >= maxValue)
		{
			pTarget->GattlingValue = maxValue;
			pTarget->CurrentGattlingStage = pData->WeaponStages - 1;
			pTarget->unknown_bool_4B8 = true;
		}
		else
		{
			pTarget->GattlingValue = curValue;
			pTarget->unknown_bool_4B8 = true;
			for (int i = 0; i < pData->WeaponStages; i++)
			{
				if (pTarget->Veterancy.IsElite() && curValue < pData->EliteStage[i])
				{
					pTarget->CurrentGattlingStage = i;
					break;
				}
				else if (curValue < pData->WeaponStage[i])
				{
					pTarget->CurrentGattlingStage = i;
					break;
				}
			}
		}
	}

	auto const pDataExt = TechnoTypeExt::ExtMap.Find(pData);
	auto pTargetData = TechnoExt::ExtMap.Find(pTarget);
	if (pDataExt->IsExtendGattling)
	{
		auto curValue = pTargetData->GattlingCount + RateUp;

		if (curValue <= 0)
		{
			pTargetData->GattlingCount = 0;
			pTargetData->GattlingStage = 0;
		}
		else if (curValue >= pTargetData->MaxGattlingCount)
		{
			pTargetData->GattlingCount = pTargetData->MaxGattlingCount;
			pTargetData->GattlingStage = pData->WeaponStages - 1;
		}
		else
		{
			pTargetData->GattlingCount = curValue;
			for (int i = 0; i < pData->WeaponStages; i++)
			{
				auto& setstages = pTargetData->GattlingStages;
				if (curValue < setstages[i].GetItem(0))
				{
					pTargetData->GattlingStage = i;
					break;
				}
			}
		}
	}
}

void WarheadTypeExt::ExtData::ApplyReloadAmmo(TechnoClass* pTarget, int ReloadAmount)
{
	auto pData = pTarget->GetTechnoType();
	if (pData->Ammo > 0)
	{
		auto const ammo = pTarget->Ammo + ReloadAmount;
		pTarget->Ammo = Math::clamp(ammo, 0, pData->Ammo);
	}
}

void WarheadTypeExt::ExtData::ApplyUpgrade(HouseClass* pHouse, TechnoClass* pTarget)
{
	if (!this->CanTargetHouse(pHouse, pTarget))
		return;

	TechnoTypeClass* pAimType = nullptr;
	TechnoTypeClass* pOriginType = pTarget->GetTechnoType();

	if (Converts_From.empty())
	{
		auto fSameAbstractType = [pOriginType](TechnoTypeClass* pType)
		{
			return pOriginType->WhatAmI() == pType->WhatAmI();
		};
		auto it = std::find_if(Converts_To.begin(), Converts_To.end(), fSameAbstractType);

		if (it == Converts_To.end())
			return;

		pAimType = *it;
	}
	else
	{
		auto it = std::find(Converts_From.begin(), Converts_From.end(), pTarget->GetTechnoType());
		size_t idx = it - Converts_From.begin();

		if (it == Converts_From.end() || idx >= Converts_To.size() || Converts_To[idx]->WhatAmI() != pTarget->GetTechnoType()->WhatAmI())
			return;

		pAimType = Converts_To[idx];
	}

	TechnoExt::Convert(pTarget, pAimType, Converts_DetachedBuildLimit);

	if (Converts_Anim != nullptr)
	{
		AnimClass* pAnim = GameCreate<AnimClass>(this->Converts_Anim, pTarget->GetCoords());
		pAnim->SetOwnerObject(pTarget);
	}

	TechnoExt::ExtData* pTargetExt = TechnoExt::ExtMap.Find(pTarget);

	if (Converts_Duration > 0)
	{
		pTargetExt->ConvertsCounts = Converts_Duration;
		pTargetExt->ConvertsAnim = Converts_RecoverAnim;
		pTargetExt->ConvertsOriginalType = pOriginType;
	}
}

void WarheadTypeExt::ExtData::ApplyInvBlink(TechnoClass* pOwner, HouseClass* pHouse, const std::vector<TechnoClass*>& vTargets, const WeaponTypeExt::ExtData* pWeaponExt)
{
	std::vector<TechnoClass*> vAffected;

	for (TechnoClass* pTarget : vTargets)
	{
		if (!TechnoExt::IsReallyAlive(pOwner))
			break;

		if (!TechnoExt::IsReallyAlive(pTarget) || pTarget->InLimbo || pTarget == pOwner)
			continue;

		if (pTarget->WhatAmI() == AbstractType::Building)
			continue;

		if (!this->CanTargetHouse(pHouse, pTarget))
			continue;

		auto pTargetExt = TechnoExt::ExtMap.Find(pTarget);

		if (CustomArmor::GetVersus(this, pTargetExt->GetArmorIdx(this->OwnerObject())) == 0.0)
			continue;

		CoordStruct crdOwner = pOwner->GetCoords();
		CoordStruct crdSrc = pTarget->GetCoords();

		for (AnimTypeClass* pAnimType : pWeaponExt->BlinkWeapon_SelfAnim)
		{
			if (pAnimType != nullptr)
			{
				AnimClass* pAnim = GameCreate<AnimClass>(pAnimType, crdOwner);
				pAnim->SetOwnerObject(pOwner);
				pAnim->Owner = pOwner->Owner;
			}
		}

		for (auto pAnimType : pWeaponExt->BlinkWeapon_TargetAnim)
		{
			if (pAnimType != nullptr)
			{
				AnimClass* pAnim = GameCreate<AnimClass>(pAnimType, crdSrc);
				pAnim->SetOwnerObject(pTarget);
				pAnim->Owner = pOwner->Owner;
			}
		}

		CellClass* pCell = nullptr;
		int iHeight = pTarget->GetHeight();
		auto pTargetType = pTarget->GetTechnoType();

		if (pWeaponExt->BlinkWeapon_Overlap.Get())
		{
			pCell = MapClass::Instance->TryGetCellAt(CellClass::Coord2Cell(crdOwner));
		}
		else
		{
			bool allowBridges = pTargetType->SpeedType != SpeedType::Float;
			CellStruct cellDest =
				MapClass::Instance->NearByLocation
				(
					CellClass::Coord2Cell(crdOwner),
					pTargetType->SpeedType,
					-1,
					pTargetType->MovementZone,
					false,
					1,
					1,
					true,
					false,
					false,
					allowBridges,
					CellStruct::Empty,
					false,
					false
				);
			pCell = MapClass::Instance->TryGetCellAt(cellDest);
		}

		CoordStruct crdDest;

		if (pCell != nullptr)
		{
			crdDest = pCell->GetCoordsWithBridge();
		}
		else
		{
			crdDest = crdOwner;
			crdDest.Z = MapClass::Instance->GetCellFloorHeight(crdDest);
		}

		crdDest.Z += iHeight;
		FootClass* pFoot = abstract_cast<FootClass*>(pTarget);
		CellStruct cellDest = CellClass::Coord2Cell(crdDest);
		pTarget->Limbo();
		ILocomotion* pLoco = pFoot->Locomotor.release();
		pFoot->Locomotor.reset(LocomotionClass::CreateInstance(pTargetType->Locomotor).release());
		pFoot->Locomotor->Link_To_Object(pFoot);
		pLoco->Release();
		++Unsorted::IKnowWhatImDoing;
		pTarget->Unlimbo(crdDest, pTarget->PrimaryFacing.Current().GetDir());
		--Unsorted::IKnowWhatImDoing;
		pTarget->MarkAllOccupationBits(crdDest);
		pTarget->Guard();
		vAffected.emplace_back(pTarget);

		if (pWeaponExt->BlinkWeapon_KillTarget.Get())
			pTarget->TakeDamage(pTarget->Health);
	}

	for (TechnoClass* pTarget : vAffected)
		pTarget->UnmarkAllOccupationBits(pTarget->GetCoords());
}

void WarheadTypeExt::ExtData::ApplyPaintBall(TechnoClass* pTarget)
{
	auto pExt = TechnoExt::ExtMap.Find(pTarget);

	bool canAffectTarget = GeneralUtils::GetWarheadVersusArmor(this->OwnerObject(), pTarget->GetTechnoType()->Armor) != 0.0;

	if (pTarget && pExt && canAffectTarget)
	{
		pExt->AllowToPaint = true;
		pExt->Paint_Count = this->PaintBall_Duration;
		pExt->Paint_IgnoreTintStatus = this->PaintBall_IgnoreTintStatus;

		if (this->PaintBall_IsDiscoColor.Get())
		{
			pExt->Paint_IsDiscoColor = true;
			pExt->Paint_Colors.assign(this->PaintBall_Colors.begin(), this->PaintBall_Colors.end());
			pExt->Paint_TransitionDuration = this->PaintBall_TransitionDuration;
			pExt->Paint_FramesPassed = 0;
		}
		else
		{
			pExt->ColorToPaint = this->PaintBall_Color;
		}
	}
}

void WarheadTypeExt::ExtData::ApplyDisableTurn(TechnoClass* pTarget)
{
	auto pExt = TechnoExt::ExtMap.Find(pTarget);

	bool canAffectTarget = GeneralUtils::GetWarheadVersusArmor(this->OwnerObject(), pTarget->GetTechnoType()->Armor) != 0.0;

	if (pTarget && pExt && canAffectTarget)
	{
		if (pTarget->WhatAmI() == AbstractType::Infantry || pTarget->WhatAmI() == AbstractType::Unit || pTarget->WhatAmI() == AbstractType::Aircraft || pTarget->WhatAmI() == AbstractType::Building)
		{
			auto pTargetData = TechnoExt::ExtMap.Find(abstract_cast<TechnoClass*>(pTarget));
			pTargetData->DisableTurnCount = this->DisableTurn_Duration;
		}
	}
}

void WarheadTypeExt::ExtData::ApplyAffectPassenger(TechnoClass* pTarget, WeaponTypeClass* pWeapon, BulletClass* pBullet)
{
	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pTarget->GetTechnoType());
	TechnoClass* pOwner = pBullet->Owner;
	HouseClass* pHouse = pOwner == nullptr ? nullptr : pBullet->Owner->Owner;

	if (!pTypeExt->ProtectPassengers)
	{
		if (auto const pTargetTechno = abstract_cast<TechnoClass*>(pTarget))
		{
			auto const pBuilding = abstract_cast<BuildingClass*>(pTarget);

			if (pBuilding != nullptr && pBuilding->Occupants.Count > 0)
			{
				if (this->ClearPassengers && !pTypeExt->ProtectPassengers_Clear)
					pBuilding->Occupants.Clear();

				if (this->ReleasePassengers && !pTypeExt->ProtectPassengers_Release)
				{
					int passengercount = pBuilding->Occupants.Count;
					for (int i = 0; i < passengercount; i++)
					{
						auto pPassenger = pBuilding->Occupants.GetItem(0);
						TechnoTypeClass* passengerType;
						passengerType = pPassenger->GetTechnoType();

						bool allowBridges = passengerType->SpeedType != SpeedType::Float;
						CoordStruct location = pTargetTechno->GetCoords();
						location.Z = MapClass::Instance->GetCellFloorHeight(location);

						auto nCell = MapClass::Instance->NearByLocation(CellClass::Coord2Cell(location),
							passengerType->SpeedType, -1, passengerType->MovementZone, false, 1, 1, true,
							false, false, allowBridges, CellStruct::Empty, false, false);

						auto pCell = MapClass::Instance->TryGetCellAt(nCell);
						location = pCell->GetCoordsWithBridge();

						pPassenger->Transporter = nullptr;
						pPassenger->Unlimbo(location, static_cast<DirType>(ScenarioClass::Instance->Random.RandomRanged(0, 255)));
						pPassenger->QueueMission(Mission::Stop, true);
						pPassenger->ForceMission(Mission::Guard);
						pPassenger->Guard();
						pBuilding->Occupants.RemoveItem(0);
					}
				}

				if (this->DamagePassengers && !pTypeExt->ProtectPassengers_Damage && pBullet != nullptr)
				{
					int passengercount = pBuilding->Occupants.Count;

					if (this->DamagePassengers_AffectAllPassengers)
					{
						for (int i = 0; i < pBuilding->Occupants.Count; i++)
						{
							auto pPassenger = pBuilding->Occupants.GetItem(i);

							if (pPassenger != nullptr)
							{
								if (pPassenger->Health > pWeapon->Damage)
								{
									pPassenger->TakeDamage(pWeapon->Damage, pHouse, pOwner, pWeapon->Warhead);
								}
								else
								{
									pPassenger->TakeDamage(pWeapon->Damage, pHouse, pOwner, pWeapon->Warhead);
									i--;
								}
							}

							if (pBuilding->Occupants.Count == 0)
							{
								pBuilding->FiringOccupantIndex = 0;
								break;
							}
						}
					}
					else
					{
						auto pPassenger = pBuilding->Occupants.GetItem(passengercount - 1);
						pPassenger->TakeDamage(pWeapon->Damage, pHouse, pOwner, pWeapon->Warhead);
					}
				}
			}
			else if (pTarget->WhatAmI() == AbstractType::Building || pTarget->WhatAmI() == AbstractType::Unit || pTarget->WhatAmI() == AbstractType::Aircraft)
			{
				if (pTargetTechno->Passengers.NumPassengers > 0)
				{
					while (pTargetTechno->Passengers.GetFirstPassenger())
					{
						FootClass* pTargetPassenger = pTargetTechno->Passengers.GetFirstPassenger();
						ObjectClass* pLastTargetPassenger = nullptr;

						TechnoTypeClass* passengerType;
						passengerType = pTargetPassenger->GetTechnoType();
						bool allowBridges = passengerType->SpeedType != SpeedType::Float;
						CoordStruct location = pTargetTechno->GetCoords();
						location.Z = MapClass::Instance->GetCellFloorHeight(location);

						auto nCell = MapClass::Instance->NearByLocation(CellClass::Coord2Cell(location),
							passengerType->SpeedType, -1, passengerType->MovementZone, false, 1, 1, true,
							false, false, allowBridges, CellStruct::Empty, false, false);

						auto pCell = MapClass::Instance->TryGetCellAt(nCell);
						location = pCell->GetCoordsWithBridge();

						while (pTargetPassenger->NextObject)
						{
							pLastTargetPassenger = pTargetPassenger;
							pTargetPassenger = static_cast<FootClass*>(pTargetPassenger->NextObject);
						}

						if (this->ClearPassengers && !pTypeExt->ProtectPassengers_Clear)
						{
							if (pLastTargetPassenger)
								pLastTargetPassenger->NextObject = nullptr;
							else
								pTargetTechno->Passengers.FirstPassenger = nullptr;

							--pTargetTechno->Passengers.NumPassengers;

							pTargetPassenger->UnInit();
							continue;
						}

						if (this->ReleasePassengers && !pTypeExt->ProtectPassengers_Release)
						{
							if (pLastTargetPassenger)
								pLastTargetPassenger->NextObject = nullptr;
							else
								pTargetTechno->Passengers.FirstPassenger = nullptr;

							--pTargetTechno->Passengers.NumPassengers;

							pTargetPassenger->Transporter = nullptr;
							pTargetPassenger->Unlimbo(location, static_cast<DirType>(ScenarioClass::Instance->Random.RandomRanged(0, 255)));
							pTargetPassenger->QueueMission(Mission::Stop, true);
							pTargetPassenger->ForceMission(Mission::Guard);
							pTargetPassenger->Guard();
							continue;
						}

						if (this->DamagePassengers && !pTypeExt->ProtectPassengers_Damage && pBullet != nullptr)
						{
							if (this->DamagePassengers_AffectAllPassengers)
							{
								FootClass* pNowPassenger = pTargetTechno->Passengers.GetFirstPassenger();
								ObjectClass* pNextPassenger = pNowPassenger->NextObject;

								if (pNowPassenger->Health > pWeapon->Damage)
								{
									pNowPassenger->TakeDamage(pWeapon->Damage, pHouse, pOwner, pWeapon->Warhead);
								}
								else
								{
									if (pNowPassenger->NextObject)
									{
										auto pNow = pTargetTechno->Passengers.GetFirstPassenger();
										pTargetTechno->Passengers.FirstPassenger = static_cast<FootClass*>(pLastTargetPassenger->NextObject);
										pNow->TakeDamage(pWeapon->Damage, pHouse, pOwner, pWeapon->Warhead);

										continue;
									}
									else
									{
										auto pNow = pTargetTechno->Passengers.GetFirstPassenger();
										pTargetTechno->Passengers.FirstPassenger = nullptr;
										--pTargetTechno->Passengers.NumPassengers;
										pNow->TakeDamage(pWeapon->Damage, pHouse, pOwner, pWeapon->Warhead);

										break;
									}

								}

								while (pNowPassenger->NextObject)
								{
									if (pNowPassenger->NextObject->Health > pWeapon->Damage)
									{
										pNowPassenger->TakeDamage(pWeapon->Damage, pHouse, pOwner, pWeapon->Warhead);
									}
									else
									{
										pNowPassenger->TakeDamage(pWeapon->Damage, pHouse, pOwner, pWeapon->Warhead);
										pNowPassenger->NextObject = pNextPassenger->NextObject;
									}
									pNowPassenger = static_cast<FootClass*>(pNextPassenger);
									pNextPassenger = pNextPassenger->NextObject;
								}

								break;
							}
							else
							{
								if (pLastTargetPassenger)
								{
									pLastTargetPassenger->NextObject->TakeDamage(pWeapon->Damage, pHouse, pOwner, pWeapon->Warhead);
								}
								else
								{
									pTargetTechno->Passengers.FirstPassenger->TakeDamage(pWeapon->Damage, pHouse, pOwner, pWeapon->Warhead);
								}

								break;
							}
						}
					}
				}
			}
		}
	}
}

void WarheadTypeExt::ExtData::ApplyCanDodge(TechnoClass* pTarget)
{
	auto pExt = TechnoExt::ExtMap.Find(pTarget);

	bool canAffectTarget = GeneralUtils::GetWarheadVersusArmor(this->OwnerObject(), pTarget->GetTechnoType()->Armor) != 0.0;

	if (pTarget && pExt && canAffectTarget)
	{
		if (pTarget->WhatAmI() == AbstractType::Infantry || pTarget->WhatAmI() == AbstractType::Unit || pTarget->WhatAmI() == AbstractType::Aircraft || pTarget->WhatAmI() == AbstractType::Building)
		{
			auto pTargetData = TechnoExt::ExtMap.Find(abstract_cast<TechnoClass*>(pTarget));
			pTargetData->CanDodge = true;
			pTargetData->DodgeDuration = this->DodgeAttach_Duration;
			pTargetData->Dodge_Anim = this->DodgeAttach_Anim;
			pTargetData->Dodge_Chance = this->DodgeAttach_Chance;
			pTargetData->Dodge_Houses = this->DodgeAttach_Houses;
			pTargetData->Dodge_MaxHealthPercent = this->DodgeAttach_MaxHealthPercent;
			pTargetData->Dodge_MinHealthPercent = this->DodgeAttach_MinHealthPercent;
			pTargetData->Dodge_OnlyDodgePositiveDamage = this->DodgeAttach_OnlyDodgePositiveDamage;
		}
	}
}

void WarheadTypeExt::ExtData::ApplyCanLimitDamage(TechnoClass* pTarget)
{
	auto pExt = TechnoExt::ExtMap.Find(pTarget);

	bool canAffectTarget = GeneralUtils::GetWarheadVersusArmor(this->OwnerObject(), pTarget->GetTechnoType()->Armor) != 0.0;

	if (pTarget && pExt && canAffectTarget)
	{
		if (pTarget->WhatAmI() == AbstractType::Infantry || pTarget->WhatAmI() == AbstractType::Unit || pTarget->WhatAmI() == AbstractType::Aircraft || pTarget->WhatAmI() == AbstractType::Building)
		{
			auto pTargetData = TechnoExt::ExtMap.Find(abstract_cast<TechnoClass*>(pTarget));
			pTargetData->LimitDamage = true;
			pTargetData->LimitDamageDuration = this->DamageLimitAttach_Duration;
			pTargetData->AllowMaxDamage = this->DamageLimitAttach_AllowMaxDamage;
			pTargetData->AllowMinDamage = this->DamageLimitAttach_AllowMinDamage;
		}
	}
}

void WarheadTypeExt::ExtData::ApplyMoveDamage(TechnoClass* pTarget)
{
	auto pExt = TechnoExt::ExtMap.Find(pTarget);

	bool canAffectTarget = GeneralUtils::GetWarheadVersusArmor(this->OwnerObject(), pTarget->GetTechnoType()->Armor) != 0.0;

	if (pTarget && pExt && canAffectTarget)
	{
		if (pTarget->WhatAmI() == AbstractType::Infantry || pTarget->WhatAmI() == AbstractType::Unit || pTarget->WhatAmI() == AbstractType::Aircraft)
		{
			auto pTargetData = TechnoExt::ExtMap.Find(abstract_cast<TechnoClass*>(pTarget));
			pTargetData->MoveDamage = this->MoveDamageAttach_Damage;
			pTargetData->MoveDamage_Duration = this->MoveDamageAttach_Duration;
			pTargetData->MoveDamage_Warhead = this->MoveDamageAttach_Warhead;
			pTargetData->MoveDamage_Delay = this->MoveDamageAttach_Delay;
			pTargetData->MoveDamage_Anim = this->MoveDamageAttach_Anim;
		}
	}
}

void WarheadTypeExt::ExtData::ApplyStopDamage(TechnoClass* pTarget)
{
	auto pExt = TechnoExt::ExtMap.Find(pTarget);

	bool canAffectTarget = GeneralUtils::GetWarheadVersusArmor(this->OwnerObject(), pTarget->GetTechnoType()->Armor) != 0.0;

	if (pTarget && pExt && canAffectTarget)
	{
		if (pTarget->WhatAmI() == AbstractType::Infantry || pTarget->WhatAmI() == AbstractType::Unit || pTarget->WhatAmI() == AbstractType::Aircraft)
		{
			auto pTargetData = TechnoExt::ExtMap.Find(abstract_cast<TechnoClass*>(pTarget));
			pTargetData->StopDamage = this->StopDamageAttach_Damage;
			pTargetData->StopDamage_Duration = this->StopDamageAttach_Duration;
			pTargetData->StopDamage_Warhead = this->StopDamageAttach_Warhead;
			pTargetData->StopDamage_Delay = this->StopDamageAttach_Delay;
			pTargetData->StopDamage_Anim = this->StopDamageAttach_Anim;
		}
	}
}

void WarheadTypeExt::ExtData::InterceptBullets(TechnoClass* pOwner, WeaponTypeClass* pWeapon, CoordStruct coords)
{
	if (!pOwner || !pWeapon)
		return;

	float cellSpread = this->OwnerObject()->CellSpread;

	if (cellSpread == 0.0)
	{
		if (auto const pBullet = specific_cast<BulletClass*>(pOwner->Target))
		{
			auto const pExt = BulletExt::ExtMap.Find(pBullet);
			auto const pTypeExt = pExt->TypeExtData;

			// 1/8th of a cell as a margin of error.
			if (pTypeExt && pTypeExt->Interceptable && pBullet->Location.DistanceFrom(coords) <= Unsorted::LeptonsPerCell / 8.0)
				pExt->InterceptBullet(pOwner, pWeapon);
		}
	}
	else
	{
		for (auto const& pBullet : *BulletClass::Array)
		{
			auto const pExt = BulletExt::ExtMap.Find(pBullet);
			auto const pTypeExt = pExt->TypeExtData;

			// Cells don't know about bullets that may or may not be located on them so it has to be this way.
			if (pTypeExt && pTypeExt->Interceptable && pBullet->Location.DistanceFrom(coords) <= cellSpread * Unsorted::LeptonsPerCell)
				pExt->InterceptBullet(pOwner, pWeapon);
		}
	}
}

void WarheadTypeExt::ExtData::ApplyChangeOwner(HouseClass* pHouse, TechnoClass* pTarget)
{
	const auto pType = pTarget->GetTechnoType();
	bool AllowType = true;
	bool IgnoreType = false;

	if (this->ChangeOwner_Types.size() > 0)
	{
		AllowType = this->ChangeOwner_Types.Contains(pType);

	}

	if (this->ChangeOwner_Ignore.size() > 0)
	{
		IgnoreType = this->ChangeOwner_Ignore.Contains(pType);
	}

	if (!AllowType || IgnoreType)
		return;

	HouseClass* House = nullptr;

	if (this->ChangeOwner_CountryIndex >= 0)
	{
		if (HouseClass::Index_IsMP(this->ChangeOwner_CountryIndex))
			House = HouseClass::FindByIndex(this->ChangeOwner_CountryIndex);
		else
			House = HouseClass::FindByCountryIndex(this->ChangeOwner_CountryIndex);
	}
	else
	{
		House = pHouse ? pHouse : HouseClass::FindCivilianSide();
	}

	if (House == nullptr)
		return;

	bool canAffectTarget = GeneralUtils::GetWarheadVersusArmor(this->OwnerObject(), pTarget->GetTechnoType()->Armor) != 0.0;

	if (!pTarget->IsMindControlled() && canAffectTarget)
	{
		if (this->ChangeOwner_EffectToPsionics || !pTarget->GetTechnoType()->ImmuneToPsionics)
		{
			pTarget->SetOwningHouse(House);

			if (pTarget->WhatAmI() == AbstractType::Building)
			{
				pTarget->ForceMission(Mission::Guard);
			}
		}
	}
}

void WarheadTypeExt::ExtData::ApplyAttachTag(TechnoClass* pTarget)
{
	if (this->AttachTag == nullptr)
		return;

	const auto pType = pTarget->GetTechnoType();
	bool AllowType = true;
	bool IgnoreType = false;

	if (!this->AttachTag_Types.empty())
	{
		AllowType = this->AttachTag_Types.Contains(pType);
	}

	if (!this->AttachTag_Types.empty())
	{
		IgnoreType = this->AttachTag_Types.Contains(pType);
	}

	if (!AllowType || IgnoreType)
		return;

	auto TagID = this->AttachTag.data();
	auto Imposed = this->AttachTag_Imposed;

	bool canAffectTarget = GeneralUtils::GetWarheadVersusArmor(this->OwnerObject(), pTarget->GetTechnoType()->Armor) != 0.0;

	if ((pTarget->AttachedTag == nullptr || Imposed) && canAffectTarget)
	{
		auto pTagType = TagTypeClass::FindOrAllocate(TagID);
		auto pTag = TagClass::GetInstance(pTagType);

		pTarget->AttachTrigger(pTag);
	}
}

void WarheadTypeExt::ExtData::ApplyAttachEffects(TechnoClass* pOwner, TechnoClass* pTarget)
{
	TechnoExt::AttachEffect(pTarget, pOwner, this);
}

void WarheadTypeExt::ExtData::ApplyTemperature(TechnoClass* pTarget)
{
	TechnoExt::ExtData* pTargetExt = TechnoExt::ExtMap.Find(pTarget);
	TechnoTypeClass* pTargetType = pTarget->GetTechnoType();
	TechnoTypeExt::ExtData* pTargetTypeExt = TechnoTypeExt::ExtMap.Find(pTargetType);

	for (const auto& item : Temperature)
	{
		size_t idx = item.first;

		if (pTargetTypeExt->Temperatrue_Disable[idx] || !Temperature_IgnoreIronCurtain[idx] && pTarget->IsIronCurtained())
			continue;

		int addend = Game::F2I(item.second * (Temperature_IgnoreVersus[idx] ? 1.0 : GeneralUtils::GetWarheadVersusArmor(OwnerObject(), pTargetType->Armor)));
		int& temperature = pTargetExt->Temperature[idx];
		TemperatureTypeClass* pTempType = TemperatureTypeClass::Array[idx].get();
		temperature += addend;
		temperature = std::max(pTempType->Minimum.Get(), temperature);
		temperature = std::min(pTargetTypeExt->Temperature[idx], temperature);
		pTargetExt->Temperature_HeatUpDelayTimer[idx].Start(
			pTargetTypeExt->Temperature_HeatUpDelay.count(idx)
			? pTargetTypeExt->Temperature_HeatUpDelay[idx]
			: pTempType->HeatUp_Delay);
		pTargetExt->Temperature_HeatUpTimer[idx].Start(
			pTargetTypeExt->Temperature_HeatUpFrame.count(idx)
			? pTargetTypeExt->Temperature_HeatUpFrame[idx]
			: pTempType->HeatUp_Frame);
	}
}

void WarheadTypeExt::ExtData::ApplyDirectional(BulletClass* pBullet)
{
	if (!pBullet)
		return;

	const auto pObj = pBullet->GetCell()->FindObjectOfType(AbstractType::Unit, false);
	if (!pObj)
		return;
	const auto pTarget = abstract_cast<TechnoClass*>(pObj);
	if (!pTarget || pBullet->IsInAir() != pTarget->IsInAir() || pTarget->IsIronCurtained())
		return;

	const auto pTarExt = TechnoExt::ExtMap.Find(pTarget);
	if (!pTarExt || (pTarExt->Shield && pTarExt->Shield->IsActive()))
		return;

	const auto pTarTypeExt = pTarExt->TypeExtData;
	const auto pRulesExt = RulesExt::Global();

	if (!pTarTypeExt->DirectionalArmor.Get(pRulesExt->DirectionalArmor) || pTarget->WhatAmI() != AbstractType::Unit || pBullet->Type->Vertical)
		return;

	const int tarFacing = pTarget->PrimaryFacing.Current().GetValue<16>();
	int bulletFacing = BulletExt::ExtMap.Find(pBullet)->BulletDir.GetValue<16>();

	const int angle = abs(bulletFacing - tarFacing);
	auto frontField = 64 * pTarTypeExt->DirectionalArmor_FrontField;
	auto backField = 64 * pTarTypeExt->DirectionalArmor_BackField;

	if (angle >= 128 - frontField && angle <= 128 + frontField)//�����ܻ�
		pTarExt->ReceiveDamageMultiplier = pTarTypeExt->DirectionalArmor_FrontMultiplier.Get(pRulesExt->DirectionalArmor_FrontMultiplier) *
		this->Directional_Multiplier.Get(pRulesExt->Directional_Multiplier);
	else if ((angle < backField && angle >= 0) || (angle > 192 + backField && angle <= 256))//�����ܻ�
		pTarExt->ReceiveDamageMultiplier = pTarTypeExt->DirectionalArmor_BackMultiplier.Get(pRulesExt->DirectionalArmor_BackMultiplier) *
		this->Directional_Multiplier.Get(pRulesExt->Directional_Multiplier);
	else//�����ܻ�
		pTarExt->ReceiveDamageMultiplier = pTarTypeExt->DirectionalArmor_SideMultiplier.Get(pRulesExt->DirectionalArmor_SideMultiplier) *
		this->Directional_Multiplier.Get(pRulesExt->Directional_Multiplier);
}
