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

#include <Utilities/Helpers.Alex.h>
#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Misc/FlyingStrings.h>
#include <Utilities/EnumFunctions.h>

#include <Ext/SWType/Body.h>

void WarheadTypeExt::ExtData::Detonate(TechnoClass* pOwner, HouseClass* pHouse, BulletClass* pBullet, CoordStruct coords)
{
	if (pOwner)
	{
		if (auto const pBulletExt = BulletExt::ExtMap.Find(pBullet))
		{
			auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pOwner->GetTechnoType());

			if (pTypeExt->Interceptor && pBulletExt->IsInterceptor)
				this->InterceptBullets(pOwner, pBullet->WeaponType, coords);
		}
	}

	if (pHouse)
	{
		if (this->BigGap)
		{
			for (auto pOtherHouse : *HouseClass::Array)
			{
				if (pOtherHouse->ControlledByHuman() &&	  // Not AI
					!pOtherHouse->IsObserver() &&		  // Not Observer
					!pOtherHouse->Defeated &&			  // Not Defeated
					pOtherHouse != pHouse &&			  // Not pThisHouse
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
					if (this->LaunchSW_IgnoreInhibitors || !SWTypeExt::HasInhibitor(pSWExt, pHouse, cell))
					{
						pSuper->SetReadiness(true);
						pSuper->Launch(cell, true);
						pSuper->Reset();
					}
				}
			}
		}

		auto pData = TechnoExt::ExtMap.Find(pOwner);
		if (pOwner && pData->PassengerList[0] != nullptr && pData->AllowCreatPassenger)
		{
			pData->AllowCreatPassenger = false;
			FootClass* CreatPassenger = pData->PassengerList[0];
			CoordStruct CreatPassengerlocation = pData->PassengerlocationList[0];
			int facing = pOwner->PrimaryFacing.current().value256();

			auto const pBulletExt = BulletExt::ExtMap.Find(pBullet);
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
				if (pWeaponExt->PassengerTransport_MoveToTarget && EnumFunctions::CanTargetHouse(pWeaponExt->PassengerTransport_MoveToTargetAllowHouses, pHouse, pTargetTechno->Owner))
				{
					auto const pBuilding = abstract_cast<BuildingClass*>(pBullet->Target);
					if (pBullet->Target->WhatAmI() == AbstractType::Building && pBuilding->Type->MaxNumberOccupants > 0)
					{
						InfantryClass* pInfantry = static_cast<InfantryClass*>(CreatPassenger);
						if (pBuilding->Occupants.Count < pBuilding->Type->MaxNumberOccupants && CreatPassenger->WhatAmI() == AbstractType::Infantry && pInfantry->Type->Occupier)
						{
							pBuilding->Occupants.AddItem(pInfantry);
						}
						else
						{
							CreatPassenger->Unlimbo(CreatPassengerlocation, facing);
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

								CreatPassenger->ForceMission(Mission::Stop);
								CreatPassenger->Guard();
							}
							else
							{
								CreatPassenger->Unlimbo(CreatPassengerlocation, facing);
								CreatPassenger->QueueMission(Mission::Stop, true);
								CreatPassenger->ForceMission(Mission::Guard);
								CreatPassenger->Guard();
							}
						}
						else
						{
							CreatPassenger->Unlimbo(CreatPassengerlocation, facing);
							CreatPassenger->QueueMission(Mission::Stop, true);
							CreatPassenger->ForceMission(Mission::Guard);
							CreatPassenger->Guard();
						}
					}
				}
				else
				{
					CreatPassenger->Unlimbo(CreatPassengerlocation, facing);
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
		HouseClass* player = HouseClass::Player;

		if (pHouse != nullptr && pHouse->ControlledByPlayer() && pHouse == player)
		{
			auto ThememIndex = ThemeClass::Instance->FindIndex(Theme.data());

			if (this->Theme_Queue)
			{
				ThemeClass::Instance->Queue(ThememIndex);
			}
			else
			{
				ThemeClass::Instance->Play(ThememIndex); //如果播放的音乐不循环的话，可能会导致不播放下一曲（WWSB！）。
				ThemeClass::Instance->Queue(ThememIndex);
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
		(//WeaponTypeGroup
			pWeaponExt != nullptr &&
			pWeaponExt->InvBlinkWeapon.Get()
		)
		;

	auto const pBulletExt = BulletExt::ExtMap.Find(pBullet);
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
			if (this->Transact && (pOwner == nullptr || this->CanTargetHouse(pOwner->GetOwningHouse(), pTarget)))
				this->TransactOnOneUnit(pTarget, pOwner, 1);
		}
		else if (auto pCell = abstract_cast<CellClass*>(pBullet->Target))
			this->DetonateOnCell(pHouse, pCell, pOwner);
	}
}

void WarheadTypeExt::ExtData::DetonateOnOneUnit(HouseClass* pHouse, TechnoClass* pTarget, TechnoClass* pOwner, BulletClass* pBullet, bool bulletWasIntercepted)
{
	if (!pTarget || pTarget->InLimbo || !pTarget->IsAlive || !pTarget->Health)
		return;

	if (!this->CanTargetHouse(pHouse, pTarget))
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

	if (pOwner != nullptr && pBullet != nullptr && pBullet->GetWeaponType() != nullptr)
	{
		WeaponTypeClass* pWeapon = pBullet->GetWeaponType();
		auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);
		if (pOwner != pTarget && pWeaponExt->InvBlinkWeapon.Get())
		{
			ApplyInvBlink(pOwner, pTarget, pWeaponExt);
		}
	}
}

void WarheadTypeExt::ExtData::DetonateOnAllUnits(HouseClass* pHouse, const CoordStruct coords, const float cellSpread, TechnoClass* pOwner, BulletClass* pBullet, bool bulletWasIntercepted)
{
	for (auto pTarget : Helpers::Alex::getCellSpreadItems(coords, cellSpread, true))
	{
		this->DetonateOnOneUnit(pHouse, pTarget, pOwner, pBullet, bulletWasIntercepted);
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
					shieldType = Shield_AttachTypes[Math::min(shieldIndex, (signed)Shield_AttachTypes.size() - 1)];
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
		pTarget->MindControlledBy->CaptureManager->FreeUnit(pTarget);
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
			WarheadTypeExt::DetonateAt(this->Crit_Warhead.Get(), pTechno, pOwner, damage);
		else
			pTechno->ReceiveDamage(&damage, 0, this->OwnerObject(), pOwner, false, false, pHouse);
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

	if (!this->Converts_To.empty())
	{
		bool success = false;
		auto percentage = pTarget->GetHealthPercentage();
		TechnoTypeClass* pOrigin = pTarget->GetTechnoType();

		if (this->Converts_From.size())
		{
			// explicitly unsigned because the compiler wants it
			for (unsigned int i = 0; i < this->Converts_From.size(); i++)
			{
				// Check if the target matches upgrade-from TechnoType and it has something to upgrade-to
				if (this->Converts_To.size() >= i && this->Converts_From[i] == pTarget->GetTechnoType())
				{
					TechnoTypeClass* pResultType = this->Converts_To[i];
					//auto pTechno = static_cast<TechnoClass*>(pResultType->CreateObject(pTarget->Owner));
					//pTechno->Unlimbo(pTarget->GetCoords(), pTarget->PrimaryFacing.current().value256());
					//pTechno->Limbo();

					if (pTarget->WhatAmI() == AbstractType::Infantry &&
						pResultType->WhatAmI() == AbstractType::InfantryType)
					{
						if (abstract_cast<InfantryClass*>(pTarget)->IsDeployed() && !static_cast<InfantryTypeClass*>(pResultType)->Deployer)
						{
							abstract_cast<InfantryClass*>(pTarget)->Type->UndeployDelay = 0;
							pTarget->ForceMission(Mission::Unload);
							pTarget->ForceMission(Mission::Guard);
						}
						pTarget->Owner->OwnedInfantryTypes.Decrement(pTarget->GetTechnoType()->GetArrayIndex());
						abstract_cast<InfantryClass*>(pTarget)->Type = static_cast<InfantryTypeClass*>(pResultType);
						abstract_cast<InfantryClass*>(pTarget)->Health = int(static_cast<InfantryTypeClass*>(pResultType)->Strength * percentage);
						abstract_cast<InfantryClass*>(pTarget)->Cloakable = static_cast<InfantryTypeClass*>(pResultType)->Cloakable;
						pTarget->Owner->OwnedInfantryTypes.Increment(pTarget->GetTechnoType()->GetArrayIndex());
						success = true;
					}
					else if (pTarget->WhatAmI() == AbstractType::Unit &&
						pResultType->WhatAmI() == AbstractType::UnitType)
					{
						pTarget->Owner->OwnedUnitTypes.Decrement(pTarget->GetTechnoType()->GetArrayIndex());
						abstract_cast<UnitClass*>(pTarget)->Type = static_cast<UnitTypeClass*>(pResultType);
						abstract_cast<UnitClass*>(pTarget)->Health = int(static_cast<UnitTypeClass*>(pResultType)->Strength * percentage);
						abstract_cast<UnitClass*>(pTarget)->Cloakable = static_cast<UnitTypeClass*>(pResultType)->Cloakable;
						pTarget->Owner->OwnedUnitTypes.Increment(pTarget->GetTechnoType()->GetArrayIndex());
						success = true;
					}
					else if (pTarget->WhatAmI() == AbstractType::Aircraft &&
						pResultType->WhatAmI() == AbstractType::AircraftType)
					{
						pTarget->Owner->OwnedUnitTypes.Decrement(pTarget->GetTechnoType()->GetArrayIndex());
						abstract_cast<AircraftClass*>(pTarget)->Type = static_cast<AircraftTypeClass*>(pResultType);
						abstract_cast<AircraftClass*>(pTarget)->Health = int(static_cast<AircraftTypeClass*>(pResultType)->Strength * percentage);
						abstract_cast<AircraftClass*>(pTarget)->Cloakable = static_cast<AircraftTypeClass*>(pResultType)->Cloakable;
						pTarget->Owner->OwnedUnitTypes.Increment(pTarget->GetTechnoType()->GetArrayIndex());
						success = true;
					}
					else
					{
						Debug::Log("Attempting to convert units of different categories: %s and %s!", pTarget->GetTechnoType()->get_ID(), pResultType->get_ID());
					}
					//pTechno->UnInit();
					break;
				}
			}
		}
		else
		{
			for (size_t i = 0; i < this->Converts_To.size(); i++)
			{
				TechnoTypeClass* pResultType = this->Converts_To[i];
				//pTechno->Unlimbo(pTarget->GetCoords(), pTarget->PrimaryFacing.current().value256());
				//pTechno->Limbo();

				if (pTarget->WhatAmI() == AbstractType::Infantry &&
					pResultType->WhatAmI() == AbstractType::InfantryType)
				{
					if (abstract_cast<InfantryClass*>(pTarget)->IsDeployed() && !static_cast<InfantryTypeClass*>(pResultType)->Deployer)
					{
						abstract_cast<InfantryClass*>(pTarget)->Type->UndeployDelay = 0;
						pTarget->ForceMission(Mission::Unload);
						pTarget->ForceMission(Mission::Guard);
					}
					pTarget->Owner->OwnedInfantryTypes.Decrement(pTarget->GetTechnoType()->GetArrayIndex());
					abstract_cast<InfantryClass*>(pTarget)->Type = static_cast<InfantryTypeClass*>(pResultType);
					pTarget->Owner->OwnedInfantryTypes.Increment(pTarget->GetTechnoType()->GetArrayIndex());
					success = true;
				}
				else if (pTarget->WhatAmI() == AbstractType::Unit &&
					pResultType->WhatAmI() == AbstractType::UnitType)
				{
					pTarget->Owner->OwnedUnitTypes.Decrement(pTarget->GetTechnoType()->GetArrayIndex());
					abstract_cast<UnitClass*>(pTarget)->Type = static_cast<UnitTypeClass*>(pResultType);
					pTarget->Owner->OwnedUnitTypes.Increment(pTarget->GetTechnoType()->GetArrayIndex());
					success = true;
				}
				else if (pTarget->WhatAmI() == AbstractType::Aircraft &&
					pResultType->WhatAmI() == AbstractType::AircraftType)
				{
					pTarget->Owner->OwnedUnitTypes.Decrement(pTarget->GetTechnoType()->GetArrayIndex());
					abstract_cast<AircraftClass*>(pTarget)->Type = static_cast<AircraftTypeClass*>(pResultType);
					pTarget->Owner->OwnedUnitTypes.Increment(pTarget->GetTechnoType()->GetArrayIndex());
					success = true;
				}
				else
				{
					Debug::Log("Attempting to convert units of different categories: %s and %s!", pTarget->GetTechnoType()->get_ID(), pResultType->get_ID());
				}
			}
		}

		if (success)
		{
			if (this->Converts_Anim != nullptr)
				GameCreate<AnimClass>(this->Converts_Anim, pTarget->GetCoords());

			TechnoTypeClass* pType = pTarget->GetTechnoType();
			auto pExt = TechnoExt::ExtMap.Find(pTarget);
			if (this->Converts_Duration > 0)
			{
				pExt->ConvertsCounts = this->Converts_Duration;
				pExt->ConvertsAnim = this->Converts_RecoverAnim;
			}
			else
				pExt->ConvertsOriginalType = pTarget->GetTechnoType();

			pTarget->Health = static_cast<int>(pTarget->GetTechnoType()->Strength * percentage);
			pTarget->Cloakable = pType->Cloakable;
			TechnoExt::FixManagers(pTarget);

			if (pOrigin->Locomotor != pType->Locomotor)
				TechnoExt::ChangeLocomotorTo(pTarget, pType->Locomotor);
		}
	}
}

void WarheadTypeExt::ExtData::ApplyInvBlink(TechnoClass* pOwner, TechnoClass* pTarget, WeaponTypeExt::ExtData* pWeaponTypeExt)
{
	if (pTarget->WhatAmI() == AbstractType::Building)
		return;

	CoordStruct PreSelfLocation = pOwner->GetCoords();
	CoordStruct PreTargetLocation = pTarget->GetCoords();

	if (pOwner->WhatAmI() == AbstractType::Building)
	{
		auto const pSelfBuilding = abstract_cast<BuildingClass*>(pOwner);
		int FoundationX = pSelfBuilding->GetFoundationData()->X, FoundationY = pSelfBuilding->GetFoundationData()->Y;
		if (FoundationX > 0)
		{
			FoundationX = 1;
		}
		if (FoundationY > 0)
		{
			FoundationY = 1;
		}
		PreSelfLocation += CoordStruct { (FoundationX * 256) / 2, (FoundationY * 256) / 2 };
	}

	if (pTarget->WhatAmI() == AbstractType::Building)
	{
		auto const pTargetBuilding = abstract_cast<BuildingClass*>(pTarget);
		int FoundationX = pTargetBuilding->GetFoundationData()->X, FoundationY = pTargetBuilding->GetFoundationData()->Y;
		if (FoundationX > 0)
		{
			FoundationX = 1;
		}
		if (FoundationY > 0)
		{
			FoundationY = 1;
		}
		PreTargetLocation += CoordStruct { (FoundationX * 256) / 2, (FoundationY * 256) / 2 };
	}

	for (auto it : pWeaponTypeExt->BlinkWeapon_SelfAnim)
	{
		if (it != nullptr)
			GameCreate<AnimClass>(it, PreSelfLocation);
	}
	for (auto it : pWeaponTypeExt->BlinkWeapon_TargetAnim)
	{
		if (it != nullptr)
			GameCreate<AnimClass>(it, PreTargetLocation);
	}

	CoordStruct location;
	CellClass* pCell = nullptr;
	CellStruct nCell;
	int iHeight = pTarget->GetHeight();
	auto pTargetTechnoType = pTarget->GetTechnoType();
	if (pWeaponTypeExt->BlinkWeapon_Overlap.Get())
	{
		nCell = CellClass::Coord2Cell(PreSelfLocation);
		pCell = MapClass::Instance->TryGetCellAt(nCell);
		location = PreSelfLocation;
	}
	else
	{
		bool allowBridges = pTargetTechnoType->SpeedType != SpeedType::Float;
		nCell = MapClass::Instance->NearByLocation(CellClass::Coord2Cell(PreSelfLocation),
			pTargetTechnoType->SpeedType, -1, pTargetTechnoType->MovementZone, false, 1, 1, true,
			false, false, allowBridges, CellStruct::Empty, false, false);
		pCell = MapClass::Instance->TryGetCellAt(nCell);
		location = PreSelfLocation;
	}
	if (pCell != nullptr)
		location = pCell->GetCoordsWithBridge();
	else
		location.Z = MapClass::Instance->GetCellFloorHeight(location);

	CoordStruct Src = pTarget->GetCoords();
	location.Z += iHeight;
	FootClass* pFoot = abstract_cast<FootClass*>(pTarget);
	CellStruct cell = CellClass::Coord2Cell(Src);
	pFoot->UnmarkAllOccupationBits(Src);
	pFoot->UnmarkAllOccupationBits(location);
	MapClass::Instance()->RemoveContentAt(&cell, pFoot);
	pFoot->Locomotor->Force_Track(-1, location);
	pFoot->Locomotor->Mark_All_Occupation_Bits(0);
	if (pFoot->WhatAmI() == AbstractType::Infantry)
		pFoot->Locomotor->Stop_Movement_Animation();
	pFoot->SetLocation(location);
	CellStruct targetcell = CellClass::Coord2Cell(location);
	pFoot->MarkAllOccupationBits(location);
	pFoot->Locomotor->Clear_Coords();
	pFoot->Locomotor->Force_Track(-1, location);
	pFoot->MarkAllOccupationBits(location);
	pFoot->QueueMission(Mission::Stop, true);
	pFoot->ForceMission(Mission::Guard);
	pFoot->Guard();

	if (pWeaponTypeExt->BlinkWeapon_KillTarget.Get())
		pTarget->ReceiveDamage(&pTarget->Health, 0, pWeaponTypeExt->OwnerObject()->Warhead, pOwner, true, false, pOwner->GetOwningHouse());
}

void WarheadTypeExt::ExtData::ApplyPaintBall(TechnoClass* pTarget)
{
	auto pExt = TechnoExt::ExtMap.Find(pTarget);

	bool canAffectTarget = GeneralUtils::GetWarheadVersusArmor(this->OwnerObject(), pTarget->GetTechnoType()->Armor) != 0.0;

	if (pTarget && pExt && canAffectTarget)
	{
		pExt->AllowToPaint = true;
		pExt->Paint_Count = this->PaintBall_Duration;

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
	if (!pTypeExt->ProtectPassengers)
	{
		auto const pTargetTechno = abstract_cast<TechnoClass*>(pTarget);
		auto const pBuilding = abstract_cast<BuildingClass*>(pTarget);
		if (pTarget->WhatAmI() == AbstractType::Building && pBuilding->Occupants.Count > 0)
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

					pPassenger->Unlimbo(location, ScenarioClass::Instance->Random.RandomRanged(0, 255));
					pPassenger->QueueMission(Mission::Stop, true);
					pPassenger->ForceMission(Mission::Guard);
					pPassenger->Guard();
					pBuilding->Occupants.RemoveItem(0);
				}
			}

			if (this->DamagePassengers && !pTypeExt->ProtectPassengers_Damage && pBullet != nullptr)
			{
				int passengercount = pBuilding->Occupants.Count;
				auto pPassenger = pBuilding->Occupants.GetItem(passengercount - 1);
				pPassenger->ReceiveDamage(&pWeapon->Damage, 0, pWeapon->Warhead, pBullet->Owner, true, false, pWeapon->GetOwningHouse());
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

						pTargetPassenger->Unlimbo(location, ScenarioClass::Instance->Random.RandomRanged(0, 255));
						pTargetPassenger->QueueMission(Mission::Stop, true);
						pTargetPassenger->ForceMission(Mission::Guard);
						pTargetPassenger->Guard();
						continue;
					}

					if (this->DamagePassengers && !pTypeExt->ProtectPassengers_Damage && pBullet != nullptr)
					{
						if (pLastTargetPassenger)
							pLastTargetPassenger->NextObject->ReceiveDamage(&pWeapon->Damage, 0, pWeapon->Warhead, pBullet->Owner, true, false, pWeapon->GetOwningHouse());
						else
							pTargetTechno->Passengers.FirstPassenger->ReceiveDamage(&pWeapon->Damage, 0, pWeapon->Warhead, pBullet->Owner, true, false, pWeapon->GetOwningHouse());
						continue;
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
			auto const pTypeExt = BulletTypeExt::ExtMap.Find(pBullet->Type);

			// 1/8th of a cell as a margin of error.
			if (pTypeExt->Interceptable && pBullet->Location.DistanceFrom(coords) <= Unsorted::LeptonsPerCell / 8.0)
				BulletExt::InterceptBullet(pBullet, pOwner, pWeapon);
		}
	}
	else
	{
		for (auto const& pBullet : *BulletClass::Array)
		{
			auto const pTypeExt = BulletTypeExt::ExtMap.Find(pBullet->Type);

			// Cells don't know about bullets that may or may not be located on them so it has to be this way.
			if (pTypeExt->Interceptable && pBullet->Location.DistanceFrom(coords) <= cellSpread * Unsorted::LeptonsPerCell)
				BulletExt::InterceptBullet(pBullet, pOwner, pWeapon);
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
