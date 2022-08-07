#include "Body.h"

#include <BitFont.h>
#include <BuildingClass.h>
#include <BulletClass.h>
#include <BulletTypeClass.h>
#include <HouseClass.h>
#include <InfantryClass.h>
#include <JumpjetLocomotionClass.h>
#include <ParticleSystemClass.h>
#include <DriveLocomotionClass.h>
#include <ScenarioClass.h>
#include <SpawnManagerClass.h>
#include <TacticalClass.h>
#include <Unsorted.h>

#include <Utilities/EnumFunctions.h>
#include <Utilities/GeneralUtils.h>
#include <Utilities/Helpers.Alex.h>
#include <Utilities/ShapeTextPrinter.h>

#include <Ext/BulletType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/Team/Body.h>
#include <Ext/Script/Body.h>
#include <Ext/Bullet/Body.h>

#include <New/Type/IonCannonTypeClass.h>
#include <New/Type/GScreenAnimTypeClass.h>
#include <New/Type/TemperatureTypeClass.h>

#include <Misc/FlyingStrings.h>
#include <Misc/GScreenDisplay.h>
#include <Misc/PhobosGlobal.h>

template<> const DWORD Extension<TechnoClass>::Canary = 0x55555555;
TechnoExt::ExtContainer TechnoExt::ExtMap;

bool TechnoExt::IsReallyAlive(TechnoClass* const pThis)
{
	return
		pThis &&
		pThis->IsAlive &&
		pThis->Health > 0;
}

bool TechnoExt::IsActive(TechnoClass* const pThis)
{
	return
		TechnoExt::IsReallyAlive(pThis) &&
		!pThis->TemporalTargetingMe &&
		!pThis->BeingWarpedOut &&
		!pThis->IsUnderEMP() &&
		//pThis->IsAlive &&
		//pThis->Health > 0 &&
		!pThis->InLimbo;
}

int TechnoExt::GetSizeLeft(FootClass* const pFoot)
{
	return pFoot->GetTechnoType()->Passengers - pFoot->Passengers.GetTotalSize();
}

void TechnoExt::Stop(TechnoClass* pThis, Mission eMission)
{
	pThis->ForceMission(eMission);
	pThis->CurrentTargets.Clear();
	pThis->SetTarget(nullptr);
	pThis->SetFocus(nullptr);
	pThis->SetDestination(nullptr, true);
}

void TechnoExt::ObjectKilledBy(TechnoClass* pVictim, TechnoClass* pKiller)
{
	if (auto pVictimTechno = static_cast<TechnoClass*>(pVictim))
	{
		auto pVictimTechnoData = TechnoExt::ExtMap.Find(pVictim);

		if (pVictimTechnoData && pKiller)
		{
			TechnoClass* pObjectKiller;

			if ((pKiller->GetTechnoType()->Spawned || pKiller->GetTechnoType()->MissileSpawn) && pKiller->SpawnOwner)
				pObjectKiller = pKiller->SpawnOwner;
			else
				pObjectKiller = pKiller;

			if (pObjectKiller && pObjectKiller->BelongsToATeam())
			{
				auto pKillerTechnoData = TechnoExt::ExtMap.Find(pObjectKiller);
				auto const pFootKiller = abstract_cast<FootClass*>(pObjectKiller);
				auto const pFocus = abstract_cast<TechnoClass*>(pFootKiller->Team->Focus);
				/*
				Debug::Log("DEBUG: pObjectKiller -> [%s] [%s] registered a kill of the type [%s]\n",
					pFootKiller->Team->Type->ID, pObjectKiller->get_ID(), pVictim->get_ID());
				*/
				pKillerTechnoData->LastKillWasTeamTarget = false;

				if (pFocus
					&& pVictim
					&& pFocus->GetTechnoType() == pVictim->GetTechnoType())
				{
					pKillerTechnoData->LastKillWasTeamTarget = true;
				}

				// Conditional Jump Script Action stuff
				if (auto pKillerTeamData = TeamExt::ExtMap.Find(pFootKiller->Team))
				{
					if (pKillerTeamData->ConditionalJump_EnabledKillsCount)
					{

						bool isValidKill = pKillerTeamData->ConditionalJump_Index < 0 ? false : ScriptExt::EvaluateObjectWithMask(pVictim, pKillerTeamData->ConditionalJump_Index, -1, -1, pKiller);;

						if (isValidKill || pKillerTechnoData->LastKillWasTeamTarget)
							pKillerTeamData->ConditionalJump_Counter++;
					}

					// Special case for interrupting current action
					if (pKillerTeamData->AbortActionAfterKilling
						&& pKillerTechnoData->LastKillWasTeamTarget)
					{
						pKillerTeamData->AbortActionAfterKilling = false;
						auto pTeam = pFootKiller->Team;

						Debug::Log("DEBUG: [%s] [%s] %d = %d,%d - Force next script action after successful kill: %d = %d,%d\n"
							, pTeam->Type->ID
							, pTeam->CurrentScript->Type->ID
							, pTeam->CurrentScript->CurrentMission
							, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Action
							, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument
							, pTeam->CurrentScript->CurrentMission + 1
							, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission + 1].Action
							, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission + 1].Argument);

						// Jumping to the next line of the script list
						pTeam->StepCompleted = true;

						return;
					}
				}
				pKillerTechnoData->LastKillWasTeamTarget = true;
			}
		}
	}
}

void TechnoExt::ApplyMindControlRangeLimit(TechnoClass* pThis, TechnoTypeExt::ExtData* pTypeExt)
{
	int Range = pTypeExt->MindControlRangeLimit.Get();

	if (Range <= 0 || pThis->CaptureManager == nullptr)
		return;

	std::vector<TechnoClass*> vTechnos;

	for (auto node : pThis->CaptureManager->ControlNodes)
	{
		vTechnos.emplace_back(node->Techno);
	}

	for (auto pTechno : vTechnos)
	{
		if (pThis->DistanceFrom(pTechno) > Range)
			pThis->CaptureManager->Free(pTechno);
	}
}

void TechnoExt::ApplyInterceptor(TechnoClass* pThis, TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt)
{
	bool interceptor = pTypeExt->Interceptor.Get();

	if (interceptor)
	{
		bool interceptor_Rookie = pTypeExt->Interceptor_Rookie.Get(true);
		bool interceptor_Veteran = pTypeExt->Interceptor_Veteran.Get(true);
		bool interceptor_Elite = pTypeExt->Interceptor_Elite.Get(true);

		if (pThis->Veterancy.IsRookie() && !interceptor_Rookie
			|| pThis->Veterancy.IsVeteran() && !interceptor_Veteran
			|| pThis->Veterancy.IsElite() && !interceptor_Elite)
			return;

		if (!pThis->Target && !(pThis->WhatAmI() == AbstractType::Aircraft && pThis->GetHeight() <= 0))
		{
			BulletClass* pTargetBullet = nullptr;

			std::vector<BulletClass*> vBullets(std::move(GeneralUtils::GetCellSpreadBullets(pThis->Location, pTypeExt->Interceptor_GuardRange.Get(pThis))));

			for (auto const& pBullet : vBullets)
			{
				auto pBulletTypeExt = BulletTypeExt::ExtMap.Find(pBullet->Type);

				if (!pBulletTypeExt->Interceptable)
					continue;

				auto pBulletExt = BulletExt::ExtMap.Find(pBullet);

				if (pBulletTypeExt->Armor >= 0)
				{
					int weaponIndex = pThis->SelectWeapon(pBullet);
					auto pWeapon = pThis->GetWeapon(weaponIndex)->WeaponType;
					double versus = GeneralUtils::GetWarheadVersusArmor(pWeapon->Warhead, pBulletTypeExt->Armor);

					if (versus == 0.0)
						continue;
				}

				const auto& minguardRange = pTypeExt->Interceptor_MinimumGuardRange.Get(pThis);

				auto distance = pBullet->Location.DistanceFrom(pThis->Location);

				if (distance < minguardRange)
					continue;

				auto bulletOwner = pBullet->Owner ? pBullet->Owner->Owner : pBulletExt->FirerHouse;

				if (EnumFunctions::CanTargetHouse(pTypeExt->Interceptor_CanTargetHouses.Get(), pThis->Owner, bulletOwner))
				{
					pTargetBullet = pBullet;

					if (pBulletExt->InterceptedStatus == InterceptedStatus::Targeted)
						continue;

					break;
				}
			}

			if (pTargetBullet)
				pThis->SetTarget(pTargetBullet);
		}
	}
}

void TechnoExt::ApplyPowered_KillSpawns(TechnoClass* pThis, TechnoTypeExt::ExtData* pTypeExt)
{
	if (pThis->WhatAmI() != AbstractType::Building)
		return;

	auto const pBuilding = abstract_cast<BuildingClass*>(pThis);
	if (pTypeExt->Powered_KillSpawns && pBuilding->Type->Powered && !pBuilding->IsPowerOnline())
	{
		auto pManager = pBuilding->SpawnManager;
		if (pManager != nullptr)
		{
			pManager->ResetTarget();
			for (auto pItem : pManager->SpawnedNodes)
			{
				if (pItem->Status == SpawnNodeStatus::Attacking || pItem->Status == SpawnNodeStatus::Returning)
				{
					pItem->Unit->ReceiveDamage(&pItem->Unit->Health, 0,
						RulesClass::Instance()->C4Warhead, nullptr, false, false, nullptr);
				}
			}
		}
	}
}

void TechnoExt::ApplySpawn_LimitRange(TechnoClass* pThis, TechnoTypeExt::ExtData* pTypeExt)
{
	if (!pTypeExt->Spawner_LimitRange.Get())
		return;

	if (auto const pManager = pThis->SpawnManager)
	{
		auto pTechnoType = pThis->GetTechnoType();
		int weaponRange = 0;
		int weaponRangeExtra = pTypeExt->Spawner_ExtraLimitRange * Unsorted::LeptonsPerCell;

		auto setWeaponRange = [&weaponRange](WeaponTypeClass* pWeaponType)
		{
			if (pWeaponType && pWeaponType->Spawner && pWeaponType->Range > weaponRange)
				weaponRange = pWeaponType->Range;
		};

		setWeaponRange(pTechnoType->Weapon[0].WeaponType);
		setWeaponRange(pTechnoType->Weapon[1].WeaponType);
		setWeaponRange(pTechnoType->EliteWeapon[0].WeaponType);
		setWeaponRange(pTechnoType->EliteWeapon[1].WeaponType);

		weaponRange += weaponRangeExtra;

		if (pManager->Target && (pThis->DistanceFrom(pManager->Target) > weaponRange))
			pManager->ResetTarget();
	}
}

void TechnoExt::MovePassengerToSpawn(TechnoClass* pThis, TechnoTypeExt::ExtData* pTypeExt)
{
	if (!TechnoExt::IsActive(pThis))
		return;

	if (pTypeExt->MovePassengerToSpawn)
	{
		SpawnManagerClass* pManager = pThis->SpawnManager;
		if (pManager != nullptr)
		{
			for (auto pItem : pManager->SpawnedNodes)
			{
				if (pItem->Unit->Passengers.NumPassengers == 0 &&
					!(pItem->Status == SpawnNodeStatus::Idle || pItem->Status == SpawnNodeStatus::Reloading))
				{
					pItem->Unit->Ammo = 0;
				}

				if (pThis->Passengers.NumPassengers > 0)
				{
					FootClass* pPassenger = pThis->Passengers.GetFirstPassenger();
					FootClass* pItemPassenger = pItem->Unit->Passengers.GetFirstPassenger();
					auto pItemType = pItem->Unit->GetTechnoType();

					if (pItem->Status == SpawnNodeStatus::Idle || pItem->Status == SpawnNodeStatus::Reloading)
					{
						ObjectClass* pLastPassenger = nullptr;

						while (pPassenger->NextObject)
						{
							pLastPassenger = pPassenger;
							pPassenger = static_cast<FootClass*>(pPassenger->NextObject);
						}

						TechnoTypeClass* passengerType;
						passengerType = pPassenger->GetTechnoType();

						if (passengerType->Size <= (pItemType->Passengers - pItem->Unit->Passengers.GetTotalSize()) && passengerType->Size <= pItemType->SizeLimit)
						{
							if (pLastPassenger)
								pLastPassenger->NextObject = nullptr;
							else
								pThis->Passengers.FirstPassenger = nullptr;

							--pThis->Passengers.NumPassengers;

							if (pPassenger)
							{
								ObjectClass* pLastItemPassenger = nullptr;

								while (pItemPassenger)
								{
									pLastItemPassenger = pItemPassenger;
									pItemPassenger = static_cast<FootClass*>(pItemPassenger->NextObject);
								}

								if (pLastItemPassenger)
									pLastItemPassenger->NextObject = pPassenger;
								else
									pItem->Unit->Passengers.FirstPassenger = pPassenger;

								++pItem->Unit->Passengers.NumPassengers;

								pPassenger->ForceMission(Mission::Stop);
								pPassenger->Guard();
							}
						}
					}
				}
			}
		}
	}
}

void TechnoExt::SilentPassenger(TechnoClass* pThis, TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt)
{
	if (pTypeExt->SilentPassenger.Get())
	{
		if (pThis->Passengers.NumPassengers > 0)
		{
			FootClass* pPassenger = pThis->Passengers.GetFirstPassenger();

			if (pExt->AllowPassengerToFire)
			{
				if (pExt->AllowFireCount)
				{
					if (pThis->GetCurrentMission() != Mission::Attack)
						pExt->AllowFireCount = 0;
					else
					{
						while (pPassenger)
						{
							pPassenger->ForceMission(Mission::Attack);
							pPassenger->SetTarget(pThis->Target);
							if (auto const pManager = pPassenger->SpawnManager)
							{
								pManager->SetTarget(pThis->Target);
							}
							pPassenger = static_cast<FootClass*>(pPassenger->NextObject);
						}
						pExt->AllowFireCount--;
					}
				}
				else
					pExt->AllowPassengerToFire = false;
			}
			else
			{
				while (pPassenger)
				{
					pPassenger->ForceMission(Mission::Stop);
					pPassenger->Guard();
					if (auto const pManager = pPassenger->SpawnManager)
					{
						pManager->ResetTarget();
					}
					pPassenger = static_cast<FootClass*>(pPassenger->NextObject);
				}
			}
		}
	}
}

void TechnoExt::AllowPassengerToFire(TechnoClass* pThis, AbstractClass* pTarget, WeaponTypeClass* pWeapon)
{
	TechnoTypeClass* pType = pThis->GetTechnoType();
	auto const pData = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (pData && pData->SilentPassenger && pType->OpenTopped)
	{
		auto pExt = TechnoExt::ExtMap.Find(pThis);
		pExt->AllowPassengerToFire = true;
		pExt->AllowFireCount = pWeapon->ROF;
		if (pThis->Passengers.NumPassengers > 0)
		{
			FootClass* pPassenger = pThis->Passengers.GetFirstPassenger();
			while (pPassenger)
			{
				pPassenger->ForceMission(Mission::Attack);
				pPassenger->SetTarget(pTarget);
				if (auto const pManager = pPassenger->SpawnManager)
				{
					pManager->SetTarget(pTarget);
				}
				pPassenger = static_cast<FootClass*>(pPassenger->NextObject);
			}
		}
	}
}

void TechnoExt::Spawner_SameLoseTarget(TechnoClass* pThis, TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt)
{
	if (pTypeExt->Spawner_SameLoseTarget.Get())
	{
		SpawnManagerClass* pManager = pThis->SpawnManager;
		if (pManager != nullptr)
		{
			if (pExt->SpawneLoseTarget)
				pManager->ResetTarget();
			else
				pExt->SpawneLoseTarget = true;
		}
	}
}

void TechnoExt::SpawneLoseTarget(TechnoClass* pThis)
{
	auto const pData = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	if (pData && pData->Spawner_SameLoseTarget)
	{
		auto pExt = TechnoExt::ExtMap.Find(pThis);
		pExt->SpawneLoseTarget = false;
	}
}

void TechnoExt::ConvertsRecover(TechnoClass* pThis, TechnoExt::ExtData* pExt)
{
	if (pThis->WhatAmI() == AbstractType::Building)
		return;

	if (pExt->ConvertsCounts > 0)
	{
		pExt->ConvertsCounts--;
	}
	else if (pExt->ConvertsCounts == 0)
	{
		Convert(pThis, pExt->ConvertsOriginalType, pExt->Convert_DetachedBuildLimit);
		
		if (pExt->ConvertsAnim != nullptr)
		{
			AnimClass* pAnim = GameCreate<AnimClass>(pExt->ConvertsAnim, pThis->GetCoords());
			pAnim->SetOwnerObject(pThis);
		}

		pExt->ConvertsAnim = nullptr;
		pExt->ConvertsCounts--;
		pExt->ConvertsOriginalType = pThis->GetTechnoType();
	}
}

void TechnoExt::InfantryConverts(TechnoClass* pThis, TechnoTypeExt::ExtData* pTypeExt)
{
	auto percentage = pThis->GetHealthPercentage();

	if (pThis->WhatAmI() != AbstractType::Infantry)
		return;

	auto pInf = abstract_cast<InfantryClass*>(pThis);
	if (!pTypeExt->Convert_Deploy.empty())
	{
		TechnoTypeClass* pResultType = pTypeExt->Convert_Deploy[0];

		auto pInf2 = static_cast<InfantryTypeClass*>(pResultType);

		if (pTypeExt->Convert_DeployAnim.Get() && pThis->GetCurrentMission() == Mission::Unload)
			GameCreate<AnimClass>(pTypeExt->Convert_DeployAnim, pThis->Location);

		if (pInf && pThis->GetCurrentMission() == Mission::Unload && pResultType->WhatAmI() == AbstractType::InfantryType)
		{
			pThis->GetOwningHouse()->OwnedInfantryTypes.Decrement(pThis->GetTechnoType()->GetArrayIndex());
			pInf->Type->UndeployDelay = 0;
			pThis->ForceMission(Mission::Unload);
			pThis->ForceMission(Mission::Guard);
			pInf->Type = pInf2;
			pInf->Health = int(pInf2->Strength * percentage);
			pInf->Cloakable = pInf2->Cloakable;
			pThis->GetOwningHouse()->OwnedInfantryTypes.Increment(pThis->GetTechnoType()->GetArrayIndex());
		}
	}
}

void TechnoExt::RecalculateROT(TechnoClass* pThis, TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt)
{
	if (pThis->WhatAmI() != AbstractType::Unit && pThis->WhatAmI() != AbstractType::Aircraft)
		return;

	bool disable = pExt->DisableTurnCount > 0;

	if (disable)
		--pExt->DisableTurnCount;

	TechnoTypeClass* pType = pThis->GetTechnoType();
	double dblROTMultiplier = 1.0 * !disable;
	int iROTBuff = 0;

	for (auto& pAE : pExt->AttachEffects)
	{
		dblROTMultiplier *= pAE->Type->ROT_Multiplier;
		iROTBuff += pAE->Type->ROT;
	}

	int iROT_Primary = static_cast<int>(pType->ROT * dblROTMultiplier) + iROTBuff;
	int iROT_Secondary = static_cast<int>(pTypeExt->TurretROT.Get(pType->ROT) * dblROTMultiplier) + iROTBuff;
	iROT_Primary = std::min(iROT_Primary, 127);
	iROT_Secondary = std::min(iROT_Secondary, 127);
	iROT_Primary = std::max(iROT_Primary, 0);
	iROT_Secondary = std::max(iROT_Secondary, 0);
	pThis->PrimaryFacing.ROT.Value = iROT_Primary == 0 ? 256 : static_cast<short>(iROT_Primary * 256);
	pThis->SecondaryFacing.ROT.Value = iROT_Secondary == 0 ? 256 : static_cast<short>(iROT_Secondary * 256);

	if (iROT_Primary == 0)
		pThis->PrimaryFacing.set(pExt->LastTurretFacing);

	if (iROT_Secondary == 0)
		pThis->SecondaryFacing.set(pExt->LastTurretFacing);

	pExt->LastSelfFacing = pThis->PrimaryFacing.Value;
	pExt->LastTurretFacing = pThis->SecondaryFacing.Value;
}

void TechnoExt::CanDodge(TechnoClass* pThis, TechnoExt::ExtData* pExt)
{
	if (pExt->DodgeDuration > 0)
		pExt->DodgeDuration--;
	else
	{
		pExt->CanDodge = false;
		pExt->Dodge_Anim = nullptr;
		pExt->Dodge_Chance = 0.0;
		pExt->Dodge_Houses = AffectedHouse::All;
		pExt->Dodge_MaxHealthPercent = 1.0;
		pExt->Dodge_MinHealthPercent = 0.0;
		pExt->Dodge_OnlyDodgePositiveDamage = true;
	}
}

void TechnoExt::LimitDamage(TechnoClass* pThis, TechnoExt::ExtData* pExt)
{
	if (pExt->LimitDamageDuration > 0)
		pExt->LimitDamageDuration--;
	else
	{
		pExt->LimitDamage = false;
		pExt->AllowMaxDamage = Vector2D<int> { INT_MAX, -INT_MAX };
		pExt->AllowMinDamage = Vector2D<int> { -INT_MAX, INT_MAX };
	}
}

bool TechnoExt::IsHarvesting(TechnoClass* pThis)
{
	if (!TechnoExt::IsActive(pThis))
		return false;

	auto slave = pThis->SlaveManager;
	if (slave && slave->State != SlaveManagerStatus::Ready)
		return true;

	if (pThis->WhatAmI() == AbstractType::Building && pThis->IsPowerOnline())
		return true;

	auto mission = pThis->GetCurrentMission();
	if (TechnoExt::HasAvailableDock(pThis))
	{
		if (mission == Mission::Harvest || mission == Mission::Unload || mission == Mission::Enter)
		{
			return true;
		}
		else if (mission == Mission::Guard && !pThis->IsSelected)
		{
			if (auto pUnit = abstract_cast<UnitClass*>(pThis))
				return pUnit->Locomotor->Is_Really_Moving_Now();
		}
	}

	return false;
}

void TechnoExt::MoveDamage(TechnoClass* pThis, TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt)
{
	if (pThis->WhatAmI() == AbstractType::Building)
		return;

	if (pExt->MoveDamage_Duration > 0)
	{
		if (pExt->LastLocation != pThis->Location)
		{
			pExt->LastLocation = pThis->Location;
			if (pExt->MoveDamage_Count > 0)
				pExt->MoveDamage_Count--;
			else
			{
				pExt->MoveDamage_Count = pExt->MoveDamage_Delay;
				if (pExt->MoveDamage_Warhead)
					pThis->ReceiveDamage(&pExt->MoveDamage, 0, pExt->MoveDamage_Warhead, nullptr, true, false, pThis->Owner);
				else
					pThis->ReceiveDamage(&pExt->MoveDamage, 0, RulesClass::Instance()->C4Warhead, nullptr, true, false, pThis->Owner);

				if (pExt->MoveDamage_Anim)
				{
					if (auto const pAnim = GameCreate<AnimClass>(pExt->MoveDamage_Anim, pThis->Location))
					{
						pAnim->SetOwnerObject(pThis);
						pAnim->Owner = pThis->Owner;
					}
				}
			}
		}
		else if (pExt->MoveDamage_Count > 0)
			pExt->MoveDamage_Count--;
		pExt->MoveDamage_Duration--;
	}
	else if (pTypeExt->MoveDamage > 0)
	{
		if (pExt->LastLocation != pThis->Location)
		{
			pExt->LastLocation = pThis->Location;
			if (pExt->MoveDamage_Count > 0)
				pExt->MoveDamage_Count--;
			else
			{
				pExt->MoveDamage_Count = pTypeExt->MoveDamage_Delay;
				if (pTypeExt->MoveDamage_Warhead.isset())
					pThis->ReceiveDamage(&pTypeExt->MoveDamage, 0, pTypeExt->MoveDamage_Warhead, nullptr, true, false, pThis->Owner);
				else
					pThis->ReceiveDamage(&pTypeExt->MoveDamage, 0, RulesClass::Instance()->C4Warhead, nullptr, true, false, pThis->Owner);

				if (pTypeExt->MoveDamage_Anim.isset())
				{
					if (auto const pAnim = GameCreate<AnimClass>(pTypeExt->MoveDamage_Anim, pThis->Location))
					{
						pAnim->SetOwnerObject(pThis);
						pAnim->Owner = pThis->Owner;
					}
				}
			}
		}
		else if (pExt->MoveDamage_Count > 0)
			pExt->MoveDamage_Count--;
	}
}

void TechnoExt::StopDamage(TechnoClass* pThis, TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt)
{
	if (pThis->WhatAmI() == AbstractType::Building)
		return;

	if (pExt->StopDamage_Duration > 0)
	{
		if (pExt->LastLocation == pThis->Location)
		{
			if (pExt->StopDamage_Count > 0)
				pExt->StopDamage_Count--;
			else
			{
				pExt->StopDamage_Count = pExt->StopDamage_Delay;
				if (pExt->StopDamage_Warhead)
					pThis->ReceiveDamage(&pExt->StopDamage, 0, pExt->StopDamage_Warhead, nullptr, true, false, pThis->Owner);
				else
					pThis->ReceiveDamage(&pExt->StopDamage, 0, RulesClass::Instance()->C4Warhead, nullptr, true, false, pThis->Owner);

				if (pExt->StopDamage_Anim)
				{
					if (auto const pAnim = GameCreate<AnimClass>(pExt->StopDamage_Anim, pThis->Location))
					{
						pAnim->SetOwnerObject(pThis);
						pAnim->Owner = pThis->Owner;
					}
				}
			}
		}
		else
		{
			if (pExt->StopDamage_Count > 0)
				pExt->StopDamage_Count--;
			pExt->LastLocation = pThis->Location;
		}
		pExt->StopDamage_Duration--;
	}
	else if (pTypeExt->StopDamage > 0)
	{
		if (pExt->LastLocation == pThis->Location)
		{
			if (pExt->StopDamage_Count > 0)
				pExt->StopDamage_Count--;
			else
			{
				pExt->StopDamage_Count = pTypeExt->StopDamage_Delay;
				if (pTypeExt->StopDamage_Warhead.isset())
					pThis->ReceiveDamage(&pTypeExt->StopDamage, 0, pTypeExt->StopDamage_Warhead, nullptr, true, false, pThis->Owner);
				else
					pThis->ReceiveDamage(&pTypeExt->StopDamage, 0, RulesClass::Instance()->C4Warhead, nullptr, true, false, pThis->Owner);

				if (pTypeExt->StopDamage_Anim.isset())
				{
					if (auto const pAnim = GameCreate<AnimClass>(pTypeExt->StopDamage_Anim, pThis->Location))
					{
						pAnim->SetOwnerObject(pThis);
						pAnim->Owner = pThis->Owner;
					}
				}
			}
		}
		else
		{
			if (pExt->StopDamage_Count > 0)
				pExt->StopDamage_Count--;
			pExt->LastLocation = pThis->Location;
		}
	}
}

void TechnoExt::ShareWeaponRange(TechnoClass* pThis, AbstractClass* pTarget, WeaponTypeClass* pWeapon)
{
	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	if (!pTypeExt->WeaponRangeShare_Technos.empty() && pTypeExt->WeaponRangeShare_Range > 0)
	{
		auto pExt = TechnoExt::ExtMap.Find(pThis);
		pExt->IsSharingWeaponRange = true;
		for (unsigned int i = 0; i < pTypeExt->WeaponRangeShare_Technos.size(); i++)
		{
			auto CanAffectTarget = pTypeExt->WeaponRangeShare_Technos[i];
			for (auto pAffect : Helpers::Alex::getCellSpreadItems(pThis->GetCoords(), pTypeExt->WeaponRangeShare_Range, true))
			{
				auto pAffectExt = TechnoExt::ExtMap.Find(pAffect);
				if (pAffect->GetTechnoType() == CanAffectTarget && pThis->GetOwningHouse() == pAffect->GetOwningHouse() && pAffect != pThis && !pAffectExt->IsSharingWeaponRange)
				{
					if (!pTypeExt->WeaponRangeShare_ForceAttack)
					{
						if (pAffect->GetTechnoType()->JumpJet)
						{
							FootClass* pFoot = abstract_cast<FootClass*>(pAffect);
							const auto pLoco = static_cast<JumpjetLocomotionClass*>(pFoot->Locomotor.get());
							if (!(pAffect->GetCurrentMission() == Mission::Guard || (pAffect->GetCurrentMission() == Mission::Move && !pLoco->LocomotionFacing.in_motion())))
								continue;
						}
						else
						{
							if (pAffect->GetCurrentMission() != Mission::Guard)
								continue;
						}
					}

					auto pAffectTypeExt = TechnoTypeExt::ExtMap.Find(pAffect->GetTechnoType());
					if (pAffect->GetFireErrorWithoutRange(pThis->Target, pAffectTypeExt->WeaponRangeShare_UseWeapon) != FireError::OK && pAffect->GetFireErrorWithoutRange(pThis->Target, pAffectTypeExt->WeaponRangeShare_UseWeapon) != FireError::FACING)
							continue;

					pAffect->SetTarget(pTarget);

					pAffectExt->ShareWeaponFire = true;
				}
			}
		}
	}
}

void TechnoExt::ShareWeaponRangeFire(TechnoClass* pThis, TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt)
{
	if (pExt->ShareWeaponFire)
	{
		if (pThis->Target)
		{
			auto pTarget = pThis->Target;
			const CoordStruct source = pThis->Location;
			const CoordStruct target = pTarget->GetCoords();
			const DirStruct tgtDir = DirStruct(Math::arctanfoo(source.Y - target.Y, target.X - source.X));

			if (!(pThis->GetWeapon(pTypeExt->WeaponRangeShare_UseWeapon)->WeaponType->OmniFire))
			{
				if (pThis->HasTurret())
				{
					if (pThis->SecondaryFacing.current() == tgtDir)
					{
						BulletClass* pBullet = pThis->TechnoClass::Fire(pTarget, pTypeExt->WeaponRangeShare_UseWeapon);
						if (pBullet != nullptr)
							pBullet->Owner = pThis;
						pExt->BeSharedWeaponRange = true;
						pExt->ShareWeaponFire = false;
					}
					else
						pThis->SecondaryFacing.turn(tgtDir);
				}
				else
				{
					if (pThis->PrimaryFacing.current() == tgtDir)
					{
						BulletClass* pBullet = pThis->TechnoClass::Fire(pTarget, pTypeExt->WeaponRangeShare_UseWeapon);
						if (pBullet != nullptr)
							pBullet->Owner = pThis;
						pExt->BeSharedWeaponRange = true;
						pExt->ShareWeaponFire = false;
					}
					else
						pThis->PrimaryFacing.turn(tgtDir);
				}
			}
			else
			{
				BulletClass* pBullet = pThis->TechnoClass::Fire(pTarget, pTypeExt->WeaponRangeShare_UseWeapon);
				if (pBullet != nullptr)
					pBullet->Owner = pThis;
				pExt->BeSharedWeaponRange = true;
				pExt->ShareWeaponFire = false;
			}
		}
		else
			pExt->ShareWeaponFire = false;
	}
}

void TechnoExt::ShareWeaponRangeRecover(TechnoClass* pThis, TechnoExt::ExtData* pExt)
{
	if (pExt->BeSharedWeaponRange)
	{
		pThis->Target = nullptr;
		pThis->ForceMission(Mission::Guard);
		pThis->Guard();
		pExt->BeSharedWeaponRange = false;
	}

	if (pExt->IsSharingWeaponRange)
		pExt->IsSharingWeaponRange = false;
}

void TechnoExt::TeamAffect(TechnoClass* pThis, TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt)
{
	if (pTypeExt->TeamAffect && pTypeExt->TeamAffect_Range > 0)
	{
		int TeamUnitNumber = 0;
		pExt->TeamAffectUnits.clear();
		if (pTypeExt->TeamAffect_Technos.empty())
		{
			for (auto pTeamUnit : Helpers::Alex::getCellSpreadItems(pThis->GetCoords(), pTypeExt->TeamAffect_Range, true))
			{
				if (EnumFunctions::CanTargetHouse(pTypeExt->TeamAffect_Houses, pThis->Owner, pTeamUnit->Owner) && pTeamUnit->IsAlive)
				{
					pExt->TeamAffectUnits.resize(TeamUnitNumber + 1);
					pExt->TeamAffectUnits[TeamUnitNumber] = pTeamUnit;
					TeamUnitNumber++;
				}

				if (pTypeExt->TeamAffect_MaxNumber >= pTypeExt->TeamAffect_Number && TeamUnitNumber == pTypeExt->TeamAffect_MaxNumber)
					break;
			}

			if (TeamUnitNumber >= pTypeExt->TeamAffect_Number)
			{
				if (pExt->TeamAffectCount > 0)
					pExt->TeamAffectCount--;
				else
				{
					if (pTypeExt->TeamAffect_Weapon.Get())
					{
						WeaponTypeExt::DetonateAt(pTypeExt->TeamAffect_Weapon, pThis, pThis);
						pExt->TeamAffectCount = pTypeExt->TeamAffect_ROF.isset() ? pTypeExt->TeamAffect_ROF : pTypeExt->TeamAffect_Weapon->ROF;
					}
				}

				if (pTypeExt->TeamAffect_Anim.isset() && pExt->TeamAffectAnim == nullptr)
				{
					pExt->TeamAffectAnim = GameCreate<AnimClass>(pTypeExt->TeamAffect_Anim, pThis->GetCoords());
					pExt->TeamAffectAnim->SetOwnerObject(pThis);
					pExt->TeamAffectAnim->RemainingIterations = 0xFFU;
					pExt->TeamAffectAnim->Owner = pThis->GetOwningHouse();
				}

				pExt->TeamAffectActive = true;
				return;
			}
			else
			{
				if (pExt->TeamAffectCount > 0)
					pExt->TeamAffectCount--;

				if (pExt->TeamAffectAnim != nullptr)
				{
					pExt->TeamAffectAnim->UnInit();
					pExt->TeamAffectAnim = nullptr;
				}
			}

			if (pExt->TeamAffectActive)
			{
				pExt->TeamAffectActive = false;
				if (pExt->TeamAffectLoseEfficacyCount > 0)
					pExt->TeamAffectLoseEfficacyCount--;
				else
				{
					if (pTypeExt->TeamAffect_LoseEfficacyWeapon.Get())
					{
						WeaponTypeExt::DetonateAt(pTypeExt->TeamAffect_LoseEfficacyWeapon, pThis, pThis);
						pExt->TeamAffectLoseEfficacyCount = pTypeExt->TeamAffect_LoseEfficacyROF.isset() ? pTypeExt->TeamAffect_LoseEfficacyROF : pTypeExt->TeamAffect_LoseEfficacyWeapon->ROF;
					}
				}
			}
		}
		else
		{
			for (auto pTeamUnit : Helpers::Alex::getCellSpreadItems(pThis->GetCoords(), pTypeExt->TeamAffect_Range, true))
			{
				if (EnumFunctions::CanTargetHouse(pTypeExt->TeamAffect_Houses, pThis->Owner, pTeamUnit->Owner) && pTeamUnit->IsAlive)
				{
					for (unsigned int i = 0; i < pTypeExt->TeamAffect_Technos.size(); i++)
					{
						if (pTeamUnit->GetTechnoType() == pTypeExt->TeamAffect_Technos[i])
						{
							pExt->TeamAffectUnits.resize(TeamUnitNumber + 1);
							pExt->TeamAffectUnits[TeamUnitNumber] = pTeamUnit;
							TeamUnitNumber++;
							break;
						}
					}
				}

				if (pTypeExt->TeamAffect_MaxNumber >= pTypeExt->TeamAffect_Number && TeamUnitNumber == pTypeExt->TeamAffect_MaxNumber)
					break;
			}

			if (TeamUnitNumber >= pTypeExt->TeamAffect_Number)
			{
				if (pExt->TeamAffectCount > 0)
					pExt->TeamAffectCount--;
				else
				{
					if (pTypeExt->TeamAffect_Weapon.Get())
					{
						WeaponTypeExt::DetonateAt(pTypeExt->TeamAffect_Weapon, pThis, pThis);
						pExt->TeamAffectCount = pTypeExt->TeamAffect_ROF.isset() ? pTypeExt->TeamAffect_ROF : pTypeExt->TeamAffect_Weapon->ROF;
					}
				}

				if (pTypeExt->TeamAffect_Anim.isset() && pExt->TeamAffectAnim == nullptr)
				{
					pExt->TeamAffectAnim = GameCreate<AnimClass>(pTypeExt->TeamAffect_Anim, pThis->GetCoords());
					pExt->TeamAffectAnim->SetOwnerObject(pThis);
					pExt->TeamAffectAnim->RemainingIterations = 0xFFU;
					pExt->TeamAffectAnim->Owner = pThis->GetOwningHouse();
				}

				pExt->TeamAffectActive = true;
				return;
			}
			else
			{
				if (pExt->TeamAffectCount > 0)
					pExt->TeamAffectCount--;

				if (pExt->TeamAffectAnim != nullptr)
				{
					pExt->TeamAffectAnim->UnInit();
					pExt->TeamAffectAnim = nullptr;
				}
			}

			if (pExt->TeamAffectActive)
			{
				pExt->TeamAffectActive = false;
				if (pExt->TeamAffectLoseEfficacyCount > 0)
					pExt->TeamAffectLoseEfficacyCount--;
				else
				{
					if (pTypeExt->TeamAffect_LoseEfficacyWeapon.Get())
					{
						WeaponTypeExt::DetonateAt(pTypeExt->TeamAffect_LoseEfficacyWeapon, pThis, pThis);
						pExt->TeamAffectLoseEfficacyCount = pTypeExt->TeamAffect_LoseEfficacyROF.isset() ? pTypeExt->TeamAffect_LoseEfficacyROF : pTypeExt->TeamAffect_LoseEfficacyWeapon->ROF;
					}
				}
			}
		}
	}
}

void TechnoExt::ReceiveShareDamage(TechnoClass* pThis, args_ReceiveDamage* args, std::vector<DynamicVectorClass<TechnoClass*>>& pAffect)
{
	for (unsigned int i = 0; i < pAffect.size(); i++)
	{
		if (pAffect[i].GetItem(0) != pThis)
			pAffect[i].GetItem(0)->ReceiveDamage(args->Damage, 0, args->WH, args->Attacker, true, false, args->SourceHouse);
	}
}

bool TechnoExt::HasAvailableDock(TechnoClass* pThis)
{
	for (auto pBld : pThis->GetTechnoType()->Dock)
	{
		if (pThis->Owner->CountOwnedAndPresent(pBld))
			return true;
	}

	return false;
}

void TechnoExt::InitializeLaserTrails(TechnoClass* pThis)
{
	auto pExt = TechnoExt::ExtMap.Find(pThis);

	if (pExt->LaserTrails.size())
		return;

	if (auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType()))
	{
		for (auto const& entry : pTypeExt->LaserTrailData)
		{
			if (auto const pLaserType = LaserTrailTypeClass::Array[entry.Type].get())
			{
				pExt->LaserTrails.push_back(std::make_unique<LaserTrailClass>(
					pLaserType, pThis->Owner, entry.FLH, entry.IsOnTurret));
			}
		}
	}
}

void TechnoExt::InitializeShield(TechnoClass* pThis)
{
	auto pExt = TechnoExt::ExtMap.Find(pThis);

	if (auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType()))
		pExt->CurrentShieldType = pTypeExt->ShieldType;
}

void TechnoExt::InitializeJJConvert(TechnoClass* pThis)
{
	auto pExt = TechnoExt::ExtMap.Find(pThis);
	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (pExt && pTypeExt && pTypeExt->JJConvert_Unload.Get())
	{
		pExt->needConvertWhenLanding = true;
		pExt->FloatingType = static_cast<UnitTypeClass*>(pThis->GetType());
		pExt->LandingType = pTypeExt->JJConvert_Unload.Get();
	}
}

void TechnoExt::FireWeaponAtSelf(TechnoClass* pThis, WeaponTypeClass* pWeaponType)
{
	WeaponTypeExt::DetonateAt(pWeaponType, pThis, pThis);
}

// reversed from 6F3D60
CoordStruct TechnoExt::GetFLHAbsoluteCoords(TechnoClass* pThis, CoordStruct pCoord, bool isOnTurret)
{
	auto const pType = pThis->GetTechnoType();
	auto const pFoot = abstract_cast<FootClass*>(pThis);
	Matrix3D mtx;

	// Step 1: get body transform matrix
	if (pFoot && pFoot->Locomotor)
		mtx = pFoot->Locomotor->Draw_Matrix(nullptr);
	else // no locomotor means no rotation or transform of any kind (f.ex. buildings) - Kerbiter
		mtx.MakeIdentity();

	// Steps 2-3: turret offset and rotation
	if (isOnTurret && pThis->HasTurret())
	{
		TechnoTypeExt::ApplyTurretOffset(pType, &mtx);

		double turretRad = (pThis->TurretFacing().value32() - 8) * -(Math::Pi / 16);
		double bodyRad = (pThis->PrimaryFacing.current().value32() - 8) * -(Math::Pi / 16);
		float angle = (float)(turretRad - bodyRad);

		mtx.RotateZ(angle);
	}

	// Step 4: apply FLH offset
	mtx.Translate((float)pCoord.X, (float)pCoord.Y, (float)pCoord.Z);

	Vector3D<float> result = Matrix3D::MatrixMultiply(mtx, Vector3D<float>::Empty);

	// Resulting coords are mirrored along X axis, so we mirror it back
	result.Y *= -1;

	// Step 5: apply as an offset to global object coords
	CoordStruct location = pThis->GetCoords();
	location += { (int)result.X, (int)result.Y, (int)result.Z };

	return location;
}

CoordStruct TechnoExt::GetBurstFLH(TechnoClass* pThis, int weaponIndex, bool& FLHFound)
{
	FLHFound = false;
	CoordStruct FLH = CoordStruct::Empty;
	int Index = weaponIndex;

	if (!pThis || Index < 0)
		return FLH;

	auto const pExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	auto pData = TechnoExt::ExtMap.Find(pThis);

	if (pExt->IsExtendGattling && !pThis->GetTechnoType()->IsGattling)
		Index = pData->GattlingWeaponIndex;

	auto pInf = abstract_cast<InfantryClass*>(pThis);
	pData->WeaponFLHs = pExt->WeaponBurstFLHs;

	if (pThis->Veterancy.IsElite())
	{
		if (pInf && pInf->IsDeployed())
			pData->WeaponFLHs = pExt->EliteDeployedWeaponBurstFLHs;
		else if (pInf && pInf->Crawling)
			pData->WeaponFLHs = pExt->EliteCrouchedWeaponBurstFLHs;
		else
			pData->WeaponFLHs = pExt->EliteWeaponBurstFLHs;
	}
	else if (pThis->Veterancy.IsVeteran())
	{
		if (pInf && pInf->IsDeployed())
			pData->WeaponFLHs = pExt->VeteranDeployedWeaponBurstFLHs;
		else if (pInf && pInf->Crawling)
			pData->WeaponFLHs = pExt->VeteranCrouchedWeaponBurstFLHs;
		else
			pData->WeaponFLHs = pExt->VeteranWeaponBurstFLHs;
	}
	else
	{
		if (pInf && pInf->IsDeployed())
			pData->WeaponFLHs = pExt->DeployedWeaponBurstFLHs;
		else if (pInf && pInf->Crawling)
			pData->WeaponFLHs = pExt->CrouchedWeaponBurstFLHs;
	}

	auto& pickedFLHs = pData->WeaponFLHs;

	if (pickedFLHs[Index].Count > pThis->CurrentBurstIndex)
	{
		FLHFound = true;
		FLH = pickedFLHs[Index][pThis->CurrentBurstIndex];
	}

	return FLH;
}

CoordStruct TechnoExt::GetSimpleFLH(InfantryClass* pThis, int weaponIndex, bool& FLHFound)
{
	FLHFound = false;
	CoordStruct FLH = CoordStruct::Empty;

	if (!pThis || weaponIndex < 0)
		return FLH;

	if (auto pTechnoType = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType()))
	{
		Nullable<CoordStruct> pickedFLH;

		if (pThis->IsDeployed())
		{
			if (weaponIndex == 0)
				pickedFLH = pTechnoType->DeployedPrimaryFireFLH;
			else if (weaponIndex == 1)
				pickedFLH = pTechnoType->DeployedSecondaryFireFLH;
		}
		else
		{
			if (pThis->Crawling)
			{
				if (weaponIndex == 0)
					pickedFLH = pTechnoType->PronePrimaryFireFLH;
				else if (weaponIndex == 1)
					pickedFLH = pTechnoType->ProneSecondaryFireFLH;
			}
		}

		if (pickedFLH.isset())
		{
			FLH = pickedFLH.Get();
			FLHFound = true;
		}
	}

	return FLH;
}

void TechnoExt::EatPassengers(TechnoClass* pThis, TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt)
{
	if (!TechnoExt::IsActive(pThis))
		return;

	if (pTypeExt->PassengerDeletion_Rate > 0 || pTypeExt->PassengerDeletion_UseCostAsRate)
	{
		if (pThis->Passengers.NumPassengers > 0)
		{
			FootClass* pPassenger = pThis->Passengers.GetFirstPassenger();

			if (pExt->PassengerDeletionCountDown < 0)
			{
				int timerLength = 0;

				if (pTypeExt->PassengerDeletion_UseCostAsRate)
				{
					// Use passenger cost as countdown.
					timerLength = (int)(pPassenger->GetTechnoType()->Cost * pTypeExt->PassengerDeletion_CostMultiplier);

					if (pTypeExt->PassengerDeletion_Rate > 0)
						timerLength = std::min(timerLength, pTypeExt->PassengerDeletion_Rate.Get());
				}
				else
				{
					// Use explicit rate optionally multiplied by unit size as countdown.
					timerLength = pTypeExt->PassengerDeletion_Rate;
					if (pTypeExt->PassengerDeletion_Rate_SizeMultiply && pPassenger->GetTechnoType()->Size > 1.0)
						timerLength *= static_cast<int>(pPassenger->GetTechnoType()->Size + 0.5);
				}

				pExt->PassengerDeletionCountDown = timerLength;
				pExt->PassengerDeletionTimer.Start(timerLength);
			}
			else
			{
				if (pExt->PassengerDeletionTimer.Completed())
				{
					ObjectClass* pLastPassenger = nullptr;

					// Passengers are designed as a FIFO queue but being implemented as a list
					while (pPassenger->NextObject)
					{
						pLastPassenger = pPassenger;
						pPassenger = abstract_cast<FootClass*>(pPassenger->NextObject);
					}

					if (pLastPassenger)
						pLastPassenger->NextObject = nullptr;
					else
						pThis->Passengers.FirstPassenger = nullptr;

					--pThis->Passengers.NumPassengers;

					if (pPassenger)
					{
						VocClass::PlayAt(pTypeExt->PassengerDeletion_ReportSound, pThis->GetCoords(), nullptr);

						if (pTypeExt->PassengerDeletion_Anim.isset())
						{
							const auto pAnimType = pTypeExt->PassengerDeletion_Anim.Get();
							if (auto const pAnim = GameCreate<AnimClass>(pAnimType, pThis->Location))
							{
								pAnim->SetOwnerObject(pThis);
								pAnim->Owner = pThis->Owner;
							}
						}

						// Check if there is money refund
						if (pTypeExt->PassengerDeletion_Soylent)
						{
							// Refund money to the Attacker
							int nMoneyToGive = pPassenger->GetTechnoType()->GetRefund(pPassenger->Owner, true);
							nMoneyToGive = (int)(nMoneyToGive * pTypeExt->PassengerDeletion_SoylentMultiplier);

							// Is allowed the refund of friendly units?
							if (!pTypeExt->PassengerDeletion_SoylentFriendlies && pPassenger->Owner->IsAlliedWith(pThis))
								nMoneyToGive = 0;

							if (nMoneyToGive > 0)
							{
								pThis->Owner->GiveMoney(nMoneyToGive);

								if (pTypeExt->PassengerDeletion_DisplaySoylent)
								{
									FlyingStrings::AddMoneyString(nMoneyToGive, pThis->Owner,
										pTypeExt->PassengerDeletion_DisplaySoylentToHouses, pThis->Location, pTypeExt->PassengerDeletion_DisplaySoylentOffset);
								}
							}
						}

						// Handle gunner change.
						if (pThis->GetTechnoType()->Gunner)
						{
							if (auto const pFoot = abstract_cast<FootClass*>(pThis))
							{
								pFoot->RemoveGunner(pPassenger);

								if (pThis->Passengers.NumPassengers > 0)
									pFoot->ReceiveGunner(pThis->Passengers.FirstPassenger);
							}
						}

						pPassenger->KillPassengers(pThis);
						pPassenger->RegisterDestruction(pThis);
						pPassenger->UnInit();
					}

					pExt->PassengerDeletionTimer.Stop();
					pExt->PassengerDeletionCountDown = -1;
				}
			}
		}
		else
		{
			pExt->PassengerDeletionTimer.Stop();
			pExt->PassengerDeletionCountDown = -1;
		}
	}
}

void TechnoExt::ChangePassengersList(TechnoClass* pThis, TechnoExt::ExtData* pExt)
{
	if (pExt->AllowChangePassenger)
	{
		int i = 0;
		do
		{
			pExt->PassengerlocationList[i] = pExt->PassengerlocationList[i + 1];
			i++;
		}
		while (pExt->PassengerlocationList[i] != CoordStruct { 0, 0, 0 });

		int j = 0;
		do
		{
			pExt->PassengerList[j] = pExt->PassengerList[j + 1];
			j++;
		}
		while (pExt->PassengerList[j] != nullptr);
		pExt->AllowCreatPassenger = true;
		pExt->AllowChangePassenger = false;
	}
}

void TechnoExt::FirePassenger(TechnoClass* pThis, AbstractClass* pTarget, WeaponTypeClass* pWeapon)
{
	auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);

	if (pWeaponExt->PassengerDeletion)
	{
		auto pTechnoData = TechnoExt::ExtMap.Find(pThis);
		pTechnoData->PassengerNumber = pThis->GetTechnoType()->Passengers;

		if (pThis->Passengers.NumPassengers > 0)
		{

			FootClass* pPassenger = pThis->Passengers.GetFirstPassenger();
			ObjectClass* pLastPassenger = nullptr;

			// Passengers are designed as a FIFO queue but being implemented as a list
			while (pPassenger->NextObject)
			{
				pLastPassenger = pPassenger;
				pPassenger = static_cast<FootClass*>(pPassenger->NextObject);
			}

			if (pLastPassenger)
				pLastPassenger->NextObject = nullptr;
			else
				pThis->Passengers.FirstPassenger = nullptr;

			--pThis->Passengers.NumPassengers;

			if (pPassenger)
			{
				if (pWeaponExt->PassengerTransport)
				{
					//TechnoClass* pTargetType = abstract_cast<TechnoClass*>(pTarget);

					TechnoTypeClass* passengerType;
					passengerType = pPassenger->GetTechnoType();

					bool allowBridges = passengerType->SpeedType != SpeedType::Float;
					CoordStruct location;

					if (pTarget->WhatAmI() == AbstractType::Cell)
					{
						auto pCell = abstract_cast<CellClass*>(pTarget);
						location = pCell->GetCoordsWithBridge();
					}
					else
					{
						auto pObject = abstract_cast<ObjectClass*>(pTarget);
						location = pObject->GetCoords();
						location.Z = MapClass::Instance->GetCellFloorHeight(location);
					}

					auto nCell = MapClass::Instance->NearByLocation(CellClass::Coord2Cell(location),
						passengerType->SpeedType, -1, passengerType->MovementZone, false, 1, 1, true,
						false, false, allowBridges, CellStruct::Empty, false, false);

					auto pCell = MapClass::Instance->TryGetCellAt(nCell);
					location = pCell->GetCoordsWithBridge();

					int j = 0;
					while (pTechnoData->PassengerlocationList[j] != CoordStruct { 0, 0, 0 })
					{
						j++;
					}
					pTechnoData->PassengerlocationList[j] = location;

					int i = 0;
					while (pTechnoData->PassengerList[i] != nullptr)
					{
						i++;
					}
					pTechnoData->PassengerList[i] = pPassenger;
				}
				else
				{
					pPassenger->UnInit();
				}
			}
		}
	}
}

void TechnoExt::IsInROF(TechnoClass* pThis, TechnoExt::ExtData* pExt)
{
	if (pExt->ROFCount > 0)
	{
		pExt->IsInROF = true;
		pExt->ROFCount--;
	}
	else
	{
		pExt->IsInROF = false;
		pExt->IsChargeROF = false;
	}
}

void TechnoExt::SetWeaponROF(TechnoClass* pThis, WeaponTypeClass* pWeapon)
{
	auto pExt = TechnoExt::ExtMap.Find(pThis);
	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (pTypeExt->DeterminedByRange)
	{
		if ((pWeapon == pThis->GetWeapon(1)->WeaponType && pTypeExt->DeterminedByRange_MainWeapon == 0) || (pWeapon == pThis->GetWeapon(0)->WeaponType && pTypeExt->DeterminedByRange_MainWeapon == 1))
		{
			int weaponrof = pWeapon->ROF;
			if (pThis->WhatAmI() == AbstractType::Infantry)
			{
				auto const pInf = abstract_cast<InfantryClass*>(pThis);
				weaponrof += pInf->Type->FireUp;
			}
			pExt->ROFCount = weaponrof;
		}
	}
}

void TechnoExt::WeaponFacingTarget(TechnoClass* pThis)
{
	if (pThis->Target != nullptr && pThis->WhatAmI() == AbstractType::Unit)
	{
		WeaponStruct* pWeaponStruct = pThis->GetWeapon(pThis->SelectWeapon(pThis->Target));

		if (pWeaponStruct == nullptr)
			return;

		WeaponTypeClass* pWeapon = pWeaponStruct->WeaponType;

		if (pWeapon == nullptr)
			return;

		auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);
		if (pWeaponExt->FacingTarget)
		{
			if (pThis->DistanceFrom(pThis->Target) <= pWeapon->Range)
			{
				const CoordStruct source = pThis->Location;
				const CoordStruct target = pThis->Target->GetCoords();
				const DirStruct tgtDir = DirStruct(Math::arctanfoo(source.Y - target.Y, target.X - source.X));
				pThis->PrimaryFacing.turn(tgtDir);
			}
			pThis->SecondaryFacing.turn(pThis->PrimaryFacing.current());
		}
	}
}

void TechnoExt::VeteranWeapon(TechnoClass* pThis, TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt)
{
	if (pTypeExt->VeteranPrimary)
	{
		pExt->PrimaryWeapon = pTypeExt->Primary;

		if (pThis->Veterancy.IsElite())
		{
			if (pTypeExt->ElitePrimary)
				pExt->PrimaryWeapon = pTypeExt->ElitePrimary;
		}
		else if (pThis->Veterancy.IsVeteran())
		{
			if (pTypeExt->VeteranPrimary)
				pExt->PrimaryWeapon = pTypeExt->VeteranPrimary;
		}
		pThis->GetWeapon(0)->WeaponType = pExt->PrimaryWeapon;
	}

	if (pTypeExt->VeteranSecondary)
	{
		pExt->SecondaryWeapon = pTypeExt->Secondary;

		if (pThis->Veterancy.IsElite())
		{
			if (pTypeExt->EliteSecondary)
				pExt->SecondaryWeapon = pTypeExt->EliteSecondary;
		}
		else if (pThis->Veterancy.IsVeteran())
		{
			if (pTypeExt->VeteranSecondary)
				pExt->SecondaryWeapon = pTypeExt->VeteranSecondary;
		}
		pThis->GetWeapon(1)->WeaponType = pExt->SecondaryWeapon;
	}
}

void TechnoExt::TechnoGattlingCount(TechnoClass* pThis, TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt)
{
	if (!pExt->HasCharged)
	{
		if (pExt->IsInROF && pThis->GetCurrentMission() == Mission::Attack && pThis->DistanceFrom(pThis->Target) <= pThis->GetWeaponRange(0))
		{
			pExt->GattlingCount += pThis->GetTechnoType()->RateUp;
			if (pExt->GattlingCount > pExt->MaxGattlingCount)
				pExt->GattlingCount = pExt->MaxGattlingCount;
		}
		else
		{
			pExt->GattlingCount -= pThis->GetTechnoType()->RateDown;
			if (pExt->GattlingCount < 0)
				pExt->GattlingCount = 0;
		}
	}
}

void TechnoExt::SetGattlingCount(TechnoClass* pThis, AbstractClass* pTarget, WeaponTypeClass* pWeapon)
{
	auto pExt = TechnoExt::ExtMap.Find(pThis);
	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (pTypeExt->IsExtendGattling && !pThis->GetTechnoType()->IsGattling)
	{
		pExt->GattlingCount += pThis->GetTechnoType()->RateUp;
		int weaponrof = pWeapon->ROF;
		if (pThis->WhatAmI() == AbstractType::Infantry)
		{
			auto const pInf = abstract_cast<InfantryClass*>(pThis);
			weaponrof += pInf->Type->FireUp;
		}
		pExt->ROFCount = weaponrof;
		pExt->AttackTarget = pTarget;
		if (pTypeExt->Gattling_Charge)
		{
			if (pExt->IsCharging)
				pExt->IsChargeROF = true;
			else
				pExt->IsChargeROF = false;
		}
	}
}

void TechnoExt::ResetGattlingCount(TechnoClass* pThis, TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt)
{
	if (pTypeExt->IsExtendGattling && !pThis->GetTechnoType()->IsGattling)
	{
		if (pTypeExt->Gattling_Cycle)
		{
			if (pExt->GattlingCount == pExt->MaxGattlingCount)
			{
				pExt->GattlingCount = 0;
				pExt->GattlingStage = 0;
			}
		}
		else if (pTypeExt->Gattling_Charge)
		{
			if (pExt->IsInROF && pExt->HasCharged && !pExt->IsChargeROF)
			{
				pExt->GattlingCount = 0;
				pExt->GattlingStage = 0;
				pExt->HasCharged = false;
			}
		}
	}
}

void TechnoExt::SelectGattlingWeapon(TechnoClass* pThis, TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt)
{
	auto pType = pThis->GetTechnoType();

	pExt->GattlingWeapons = pTypeExt->Weapons;
	pExt->GattlingStages = pTypeExt->Stages;

	if (pThis->Veterancy.IsVeteran())
	{
		pExt->GattlingWeapons = pTypeExt->VeteranWeapons;
		pExt->GattlingStages = pTypeExt->VeteranStages;
	}
	else if (pThis->Veterancy.IsElite())
	{
		pExt->GattlingWeapons = pTypeExt->EliteWeapons;
		pExt->GattlingStages = pTypeExt->EliteStages;
	}

	auto& weapons = pExt->GattlingWeapons;
	auto& stages = pExt->GattlingStages;

	pExt->MaxGattlingCount = stages[pType->WeaponStages - 1].GetItem(0);

	for (int i = 0; i < pType->WeaponStages; i++)
	{
		if (pExt->GattlingCount < stages[i].GetItem(0))
		{
			pExt->GattlingStage = i;
			break;
		}
	}

	for (int i = 0; i < pType->TurretCount; i++)
	{
		pThis->CurrentTurretNumber = i;
		if (pExt->GattlingCount < stages[i].GetItem(0))
		{
			break;
		}
	}

	if (pTypeExt->Gattling_Charge && !pTypeExt->Gattling_Cycle)
	{
		if (pThis->GetCurrentMission() == Mission::Unload)
		{
			pThis->GetWeapon(0)->WeaponType = weapons[pExt->GattlingWeaponIndex].GetItem(0);
			pExt->HasCharged = true;
			pExt->IsCharging = false;
			pThis->ForceMission(Mission::Stop);
			pThis->ForceMission(Mission::Attack);
			pThis->SetTarget(pExt->AttackTarget);
		}

		if (pThis->GetCurrentMission() == Mission::Attack)
		{
			auto maxValue = stages[pType->WeaponStages - 1].GetItem(0);;
			if (pExt->GattlingCount >= maxValue)
			{
				pThis->GetWeapon(0)->WeaponType = weapons[pExt->GattlingWeaponIndex].GetItem(0);
				pExt->HasCharged = true;
				pExt->IsCharging = false;
			}
			else if (!pExt->HasCharged)
			{
				pThis->GetWeapon(0)->WeaponType = weapons[0].GetItem(0);
				pExt->IsCharging = true;
			}
		}
	}
	else
	{
		pThis->GetWeapon(0)->WeaponType = weapons[pExt->GattlingWeaponIndex].GetItem(0);
	}
}

void TechnoExt::SetWeaponIndex(TechnoClass* pThis, TechnoExt::ExtData* pExt)
{
	if (pThis->GetCurrentMission() == Mission::Attack && pThis->Target)
	{
		if (pThis->Target->IsInAir())
		{
			pExt->GattlingWeaponIndex = pExt->GattlingStage * 2 + 1;
		}
		else
		{
			pExt->GattlingWeaponIndex = pExt->GattlingStage * 2;
		}
	}
}

bool TechnoExt::CanFireNoAmmoWeapon(TechnoClass* pThis, int weaponIndex)
{
	if (pThis->GetTechnoType()->Ammo > 0)
	{
		if (const auto pExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType()))
		{
			if (pThis->Ammo <= pExt->NoAmmoAmount && (pExt->NoAmmoWeapon = weaponIndex || pExt->NoAmmoWeapon == -1))
				return true;
		}
	}

	return false;
}

void TechnoExt::KillSelf(TechnoClass* pThis, AutoDeathBehavior deathOption)
{
	switch (deathOption)
	{

	case AutoDeathBehavior::Vanish:
	{
		pThis->KillPassengers(pThis);
		pThis->Limbo();
		pThis->RegisterKill(pThis->Owner);
		pThis->UnInit();

		return;
	}

	case AutoDeathBehavior::Sell:
	{
		if (auto pBld = abstract_cast<BuildingClass*>(pThis))
		{
			if (pBld->Type->LoadBuildup())
			{
				pBld->Sell(true);

				return;
			}
		}

		Debug::Log("[Runtime Warning] %s can't be sold, killing it instead\n", pThis->get_ID());
	}

	default: //must be AutoDeathBehavior::Kill
		pThis->ReceiveDamage(&pThis->Health, 0, RulesClass::Instance()->C4Warhead, nullptr, true, false, pThis->Owner);

		return;
	}
}

void TechnoExt::KillSelfForTypes(TechnoClass* pThis, TechnoTypeExt::ExtData* pTypeExt)
{
	bool isdeath = true;

	for (unsigned int i = 0; i < pTypeExt->Death_Types.size(); i++)
	{
		int pNumber;
		auto pType = pTypeExt->Death_Types[i];
		if (pType == nullptr)
			continue;

		pNumber = pThis->Owner->CountOwnedNow(pType);

		if (pNumber > 0)
		{
			isdeath = false;
			break;
		}
	}

	if (isdeath && pTypeExt->AutoDeath_Behavior.isset())
	{
		const auto howToDie = pTypeExt->AutoDeath_Behavior.Get();
		TechnoExt::KillSelf(pThis, howToDie);
	}
}

void TechnoExt::CheckDeathConditions(TechnoClass* pThis, TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt)
{
	TechnoTypeClass* pType = pThis->GetTechnoType();

	if (!pTypeExt->AutoDeath_Behavior.isset())
		return;

	// Self-destruction must be enabled
	const auto howToDie = pTypeExt->AutoDeath_Behavior.Get();

	// Death if no ammo
	if (pType->Ammo > 0 && pThis->Ammo <= 0 && pTypeExt->AutoDeath_OnAmmoDepletion)
	{
		TechnoExt::KillSelf(pThis, howToDie);
		return;
	}

	// Death if countdown ends
	if (pExt && pTypeExt->AutoDeath_AfterDelay > 0)
	{
		//using Expired() may be confusing
		if (pExt->AutoDeathTimer.StartTime == -1 && pExt->AutoDeathTimer.TimeLeft == 0)
		{
			pExt->AutoDeathTimer.Start(pTypeExt->AutoDeath_AfterDelay);
		}
		else if (!pThis->Transporter && pExt->AutoDeathTimer.Completed())
		{
			TechnoExt::KillSelf(pThis, howToDie);
			return;
		}
	}


}

void TechnoExt::CheckIonCannonConditions(TechnoClass* pThis, TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt)
{
	IonCannonTypeClass* pIonCannonType = nullptr;

	pIonCannonType = pTypeExt->IonCannonType.Get();

	if (pIonCannonType == nullptr)
		return;

	if (pIonCannonType->IonCannon_Radius >= 0)
	{
		if (!(pExt->IonCannon_Rate > 0))
		{
			if (pExt->IonCannon_setRadius)
			{
				pExt->IonCannon_Radius = pIonCannonType->IonCannon_Radius;
				pExt->IonCannon_RadiusReduce = pIonCannonType->IonCannon_RadiusReduce;
				pExt->IonCannon_Angle = pIonCannonType->IonCannon_Angle;
				pExt->IonCannon_Scatter_Max = pIonCannonType->IonCannon_Scatter_Max;
				pExt->IonCannon_Scatter_Min = pIonCannonType->IonCannon_Scatter_Min;
				pExt->IonCannon_Duration = pIonCannonType->IonCannon_Duration;
				pExt->IonCannon_setRadius = false;
			}

			if (pExt->IonCannon_Radius >= 0 && !pExt->IonCannon_Stop)
			{
				CoordStruct center = pThis->GetCoords(); // 获取单位的坐标

				WeaponTypeClass* pIonCannonWeapon = nullptr;
				if (pIonCannonType->IonCannon_Weapon.isset())
				{
					pIonCannonWeapon = pIonCannonType->IonCannon_Weapon.Get();
				}
				else
				{
					pIonCannonWeapon = pThis->GetWeapon(0)->WeaponType; // 获取单位的主武器
				}

				// 每xx角度生成一条光束，越小越密集
				int angleDelta = 360 / pIonCannonType->IonCannon_Lines;
				for (int angle = pExt->IonCannon_StartAngle; angle < pExt->IonCannon_StartAngle + 360; angle += angleDelta)
				{
					int ScatterX = ScenarioClass::Instance->Random(pExt->IonCannonWeapon_Scatter_Min, pExt->IonCannonWeapon_Scatter_Max);
					int ScatterY = ScenarioClass::Instance->Random(pExt->IonCannonWeapon_Scatter_Min, pExt->IonCannonWeapon_Scatter_Max);

					if (ScenarioClass::Instance->Random(0, 1))
						ScatterX = -ScatterX;

					if (ScenarioClass::Instance->Random(0, 1))
						ScatterY = -ScatterY;

					CoordStruct pos =
					{
						center.X + static_cast<int>(pExt->IonCannon_Radius * cos(angle * 3.14 / 180)) + ScatterX,
						center.Y + static_cast<int>(pExt->IonCannon_Radius * sin(angle * 3.14 / 180)) + ScatterY,
						0
					};
					CoordStruct posAir = pos + CoordStruct { 0, 0, pIonCannonType->IonCannon_LaserHeight };
					CoordStruct posAirEle = pos + CoordStruct { 0, 0, pIonCannonType->IonCannon_EleHeight };
					auto pCell = MapClass::Instance->TryGetCellAt(pos);

					if (pCell)
						pos.Z = pCell->GetCoordsWithBridge().Z;
					else
						pos.Z = MapClass::Instance->GetCellFloorHeight(pos);

					if (!(pIonCannonType->IonCannon_DrawLaserWithFire && pExt->IonCannon_ROF > 0))
					{
						if (pIonCannonType->IonCannon_DrawLaser)
						{
							LaserDrawClass* pLaser = GameCreate<LaserDrawClass>(
								posAir, pos,
								pIonCannonType->IonCannon_InnerColor, pIonCannonType->IonCannon_OuterColor, pIonCannonType->IonCannon_OuterSpread,
								pIonCannonType->IonCannon_LaserDuration);

							pLaser->Thickness = pIonCannonType->IonCannon_Thickness; // only respected if IsHouseColor
							pLaser->IsHouseColor = true;
							// pLaser->IsSupported = this->Type->IsIntense;
						}
					}

					if (!(pIonCannonType->IonCannon_DrawEBoltWithFire && pExt->IonCannon_ROF > 0))
					{
						if (pIonCannonType->IonCannon_DrawEBolt) // Uses laser
						{
							if (auto const pEBolt = GameCreate<EBolt>())
								pEBolt->Fire(posAirEle, pos, 0);
						}
					}

					if (!(pExt->IonCannon_ROF > 0))
					{
						WeaponTypeExt::DetonateAt(pIonCannonWeapon, pos, pThis); // 单位使用主武器攻击坐标点
					}
				}

				if (pExt->IonCannon_ROF > 0)
					pExt->IonCannon_ROF--;
				else
					pExt->IonCannon_ROF = pIonCannonType->IonCannon_ROF;

				if (pExt->IonCannon_RadiusReduce <= pIonCannonType->IonCannon_RadiusReduceMax
					&& pExt->IonCannon_RadiusReduce >= pIonCannonType->IonCannon_RadiusReduceMin)
					pExt->IonCannon_RadiusReduce += pIonCannonType->IonCannon_RadiusReduceAcceleration;

				if (pExt->IonCannon_Angle <= pIonCannonType->IonCannon_AngleMax && pExt->IonCannon_Angle >= pIonCannonType->IonCannon_AngleMin)
					pExt->IonCannon_Angle += pIonCannonType->IonCannon_AngleAcceleration;

				if (pExt->IonCannon_Scatter_Max <= pIonCannonType->IonCannon_Scatter_Max_IncreaseMax
					&& pExt->IonCannon_Scatter_Max >= pIonCannonType->IonCannon_Scatter_Max_IncreaseMin)
					pExt->IonCannon_Scatter_Max += pIonCannonType->IonCannon_Scatter_Max_Increase;

				if (pExt->IonCannon_Scatter_Min <= pIonCannonType->IonCannon_Scatter_Min_IncreaseMax
					&& pExt->IonCannon_Scatter_Min >= pIonCannonType->IonCannon_Scatter_Min_IncreaseMin)
					pExt->IonCannon_Scatter_Min += pIonCannonType->IonCannon_Scatter_Min_Increase;

				pExt->IonCannon_Radius -= pExt->IonCannon_RadiusReduce; //默认20; // 每次半径减少的量，越大光束聚集越快
				pExt->IonCannon_StartAngle -= pExt->IonCannon_Angle; // 每次旋转角度，越大旋转越快

				if (pIonCannonType->IonCannon_MaxRadius >= 0)
				{
					if (pExt->IonCannon_Radius > pIonCannonType->IonCannon_MaxRadius)
						pExt->IonCannon_Stop = true;
				}

				if (pIonCannonType->IonCannon_MinRadius >= 0)
				{
					if (pExt->IonCannon_Radius < pIonCannonType->IonCannon_MinRadius)
						pExt->IonCannon_Stop = true;
				}

				if (pIonCannonType->IonCannon_Duration >= 0)
				{
					if (pExt->IonCannon_Duration > 0)
						pExt->IonCannon_Duration--;
					else
						pExt->IonCannon_Stop = true;
				}
			}
			else
			{
				if (pIonCannonType->IonCannon_FireOnce)
				{
					// 单位自尽
					pThis->ReceiveDamage(&pThis->Health, 0, RulesClass::Instance()->C4Warhead, nullptr, true, false, pThis->Owner);
				}
				else
				{
					pExt->IonCannon_setRadius = true;
					pExt->IonCannon_Stop = false;
					pExt->IonCannon_Rate = pIonCannonType->IonCannon_Rate;
				}
			}
		}
		else
		{
			pExt->IonCannon_Rate--;
		}
	}
}

void TechnoExt::IonCannonWeapon(TechnoClass* pThis, AbstractClass* pTarget, WeaponTypeClass* pWeapon)
{
	auto pData = TechnoExt::ExtMap.Find(pThis);
	auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);

	pData->setIonCannonWeapon = pWeapon;
	pData->setIonCannonType = pWeaponExt->IonCannonType;

	if (pData->IonCannonWeapon_Stop)
	{
		if (pTarget->WhatAmI() == AbstractType::Cell)
		{
			pData->IonCannonWeapon_Target = abstract_cast<CellClass*>(pTarget)->GetCenterCoords();
			pData->IonCannonWeapon_Stop = false;
		}
		else
		{
			pData->IonCannonWeapon_Target = abstract_cast<ObjectClass*>(pTarget)->GetCoords();
			pData->IonCannonWeapon_Stop = false;
		}
	}
}

void TechnoExt::RunIonCannonWeapon(TechnoClass* pThis, TechnoExt::ExtData* pExt)
{
	auto pWeapon = pExt->setIonCannonWeapon;
	IonCannonTypeClass* pIonCannonType = pExt->setIonCannonType.Get();

	if (pIonCannonType->IonCannon_Radius >= 0)
	{
		if (pExt->IonCannonWeapon_setRadius)
		{
			pExt->IonCannonWeapon_Radius = pIonCannonType->IonCannon_Radius.Get();
			pExt->IonCannonWeapon_RadiusReduce = pIonCannonType->IonCannon_RadiusReduce.Get();
			pExt->IonCannonWeapon_Angle = pIonCannonType->IonCannon_Angle.Get();
			pExt->IonCannonWeapon_Scatter_Max = pIonCannonType->IonCannon_Scatter_Max.Get();
			pExt->IonCannonWeapon_Scatter_Min = pIonCannonType->IonCannon_Scatter_Min.Get();
			pExt->IonCannonWeapon_Duration = pIonCannonType->IonCannon_Duration.Get();
			pExt->IonCannonWeapon_setRadius = false;
		}

		CoordStruct target = pExt->IonCannonWeapon_Target;

		if (pExt->IonCannonWeapon_Radius >= 0 && !pExt->IonCannonWeapon_Stop)
		{

			WeaponTypeClass* pIonCannonWeapon = pIonCannonType->IonCannon_Weapon.Get(pWeapon);
			// 每xx角度生成一条光束，越小越密集
			int angleDelta = 360 / pIonCannonType->IonCannon_Lines;

			for (int angle = pExt->IonCannonWeapon_StartAngle;
				angle < pExt->IonCannonWeapon_StartAngle + 360;
				angle += angleDelta)
			{
				int ScatterX = (rand() % (pExt->IonCannonWeapon_Scatter_Max - pExt->IonCannonWeapon_Scatter_Min + 1)) + pExt->IonCannonWeapon_Scatter_Min;
				int ScatterY = (rand() % (pExt->IonCannonWeapon_Scatter_Max - pExt->IonCannonWeapon_Scatter_Min + 1)) + pExt->IonCannonWeapon_Scatter_Min;
				CoordStruct pos =
				{
					target.X + (int)(pExt->IonCannonWeapon_Radius * cos(angle * 3.14 / 180)) + ScatterX,
					target.Y + (int)(pExt->IonCannonWeapon_Radius * sin(angle * 3.14 / 180)) + ScatterY,
					0
				};
				CoordStruct posAir = pos + CoordStruct { 0, 0, pIonCannonType->IonCannon_LaserHeight.Get() };
				CoordStruct posAirEle = pos + CoordStruct { 0, 0, pIonCannonType->IonCannon_EleHeight.Get() };
				auto pCell = MapClass::Instance->TryGetCellAt(pos);

				if (rand() & 1)
					ScatterX = -ScatterX;

				if (rand() & 1)
					ScatterY = -ScatterY;

				if (pCell)
					pos.Z = pCell->GetCoordsWithBridge().Z;
				else
					pos.Z = MapClass::Instance->GetCellFloorHeight(pos);

				if (!(pIonCannonType->IonCannon_DrawLaserWithFire.Get() && pExt->IonCannonWeapon_ROF > 0))
				{
					if (pIonCannonType->IonCannon_DrawLaser.Get())
					{
						LaserDrawClass* pLaser =
							GameCreate<LaserDrawClass>
							(
							posAir,
							pos,
							pIonCannonType->IonCannon_InnerColor,
							pIonCannonType->IonCannon_OuterColor,
							pIonCannonType->IonCannon_OuterSpread,
							pIonCannonType->IonCannon_LaserDuration
							);
						// only respected if IsHouseColor
						pLaser->Thickness = pIonCannonType->IonCannon_Thickness.Get();
						pLaser->IsHouseColor = true;
						// pLaser->IsSupported = this->Type->IsIntense;
					}
				}

				if (!(pIonCannonType->IonCannon_DrawEBoltWithFire.Get() && pExt->IonCannonWeapon_ROF > 0))
				{
					if (pIonCannonType->IonCannon_DrawEBolt) // Uses laser
					{
						EBolt* pEBolt = GameCreate<EBolt>();

						if (pEBolt != nullptr)
							pEBolt->Fire(posAirEle, pos, 0);
					}
				}

				if (pExt->IonCannonWeapon_ROF <= 0)
					WeaponTypeExt::DetonateAt(pIonCannonWeapon, pos, pThis); // 单位使用主武器攻击坐标点
			}

			if (pExt->IonCannonWeapon_ROF > 0)
				pExt->IonCannonWeapon_ROF--;
			else
				pExt->IonCannonWeapon_ROF = pIonCannonType->IonCannon_ROF.Get();

			if (pExt->IonCannonWeapon_RadiusReduce <= pIonCannonType->IonCannon_RadiusReduceMax.Get()
				&& pExt->IonCannonWeapon_RadiusReduce >= pIonCannonType->IonCannon_RadiusReduceMin.Get())
			{
				pExt->IonCannonWeapon_RadiusReduce += pIonCannonType->IonCannon_RadiusReduceAcceleration.Get();
			}

			if (pExt->IonCannonWeapon_Angle <= pIonCannonType->IonCannon_AngleMax.Get()
				&& pExt->IonCannonWeapon_Angle >= pIonCannonType->IonCannon_AngleMin.Get())
			{
				pExt->IonCannonWeapon_Angle += pIonCannonType->IonCannon_AngleAcceleration.Get();
			}

			if (pExt->IonCannonWeapon_Scatter_Max <= pIonCannonType->IonCannon_Scatter_Max_IncreaseMax.Get()
				&& pExt->IonCannonWeapon_Scatter_Max >= pIonCannonType->IonCannon_Scatter_Max_IncreaseMin.Get())
			{
				pExt->IonCannonWeapon_Scatter_Max += pIonCannonType->IonCannon_Scatter_Max_Increase.Get();
			}

			if (pExt->IonCannonWeapon_Scatter_Min <= pIonCannonType->IonCannon_Scatter_Min_IncreaseMax.Get()
				&& pExt->IonCannonWeapon_Scatter_Min >= pIonCannonType->IonCannon_Scatter_Min_IncreaseMin.Get())
			{
				pExt->IonCannonWeapon_Scatter_Min += pIonCannonType->IonCannon_Scatter_Min_Increase.Get();
			}

			pExt->IonCannonWeapon_Radius -= pExt->IonCannonWeapon_RadiusReduce; //默认20; // 每次半径减少的量，越大光束聚集越快
			pExt->IonCannonWeapon_StartAngle -= pExt->IonCannonWeapon_Angle; // 每次旋转角度，越大旋转越快

			if (pIonCannonType->IonCannon_MaxRadius.Get() >= 0)
			{
				if (pExt->IonCannonWeapon_Radius > pIonCannonType->IonCannon_MaxRadius.Get())
					pExt->IonCannonWeapon_Stop = true;
			}

			if (pIonCannonType->IonCannon_MinRadius >= 0)
			{
				if (pExt->IonCannonWeapon_Radius < pIonCannonType->IonCannon_MinRadius.Get())
					pExt->IonCannonWeapon_Stop = true;
			}

			if (pIonCannonType->IonCannon_Duration.Get() >= 0)
			{
				if (pExt->IonCannonWeapon_Duration > 0)
					pExt->IonCannonWeapon_Duration--;
				else
					pExt->IonCannonWeapon_Stop = true;
			}
		}
		else
		{
			pExt->IonCannonWeapon_setRadius = true;
			pExt->IonCannonWeapon_Stop = true;
		}
	}
}

void TechnoExt::BeamCannon(TechnoClass* pThis, AbstractClass* pTarget, WeaponTypeClass* pWeapon)
{
	auto pData = TechnoExt::ExtMap.Find(pThis);
	auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);

	pData->setBeamCannon = pWeapon;

	if (pWeaponExt->IsBeamCannon)
	{
		if (pData->BeamCannon_Stop)
		{
			if (pTarget->WhatAmI() == AbstractType::Cell)
			{
				pData->BeamCannon_Target = abstract_cast<CellClass*>(pTarget)->GetCenterCoords();
				pData->BeamCannon_Self = pThis->GetCoords();
				pData->BeamCannon_Stop = false;
			}
			else
			{
				pData->BeamCannon_Target = abstract_cast<ObjectClass*>(pTarget)->GetCoords();
				pData->BeamCannon_Self = pThis->GetCoords();
				pData->BeamCannon_Stop = false;
			}
		}
	}
}

void TechnoExt::RunBeamCannon(TechnoClass* pThis, TechnoExt::ExtData* pExt)
{
	WeaponTypeClass* pWeapon = pExt->setBeamCannon;
	auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);

	if (pWeaponExt->IsBeamCannon.Get() && pWeaponExt->BeamCannon_Length.Get() >= 0)
	{
		if (pExt->BeamCannon_setLength)
		{
			pExt->BeamCannon_Length = 0;
			pExt->BeamCannon_LengthIncrease = pWeaponExt->BeamCannon_LengthIncrease;
			pExt->BeamCannon_setLength = false;
		}

		CoordStruct target = pExt->BeamCannon_Target;
		CoordStruct center = pExt->BeamCannon_Self;

		if (abs(pExt->BeamCannon_Length) <= pWeaponExt->BeamCannon_Length.Get() && !pExt->BeamCannon_Stop)
		{

			WeaponTypeClass* pBeamCannonWeapon = pWeaponExt->BeamCannonWeapon.Get(pWeapon);
			CoordStruct pos =
			{
				center.X + (int)((pExt->BeamCannon_Length + pWeaponExt->BeamCannon_Length_StartOffset.Get())
					* cos(atan2(target.Y - center.Y , target.X - center.X))),

				center.Y + (int)((pExt->BeamCannon_Length + pWeaponExt->BeamCannon_Length_StartOffset.Get())
					* sin(atan2(target.Y - center.Y , target.X - center.X))),

				0
			};

			auto pCell = MapClass::Instance->TryGetCellAt(pos);

			if (pCell)
				pos.Z = pCell->GetCoordsWithBridge().Z;
			else
				pos.Z = MapClass::Instance->GetCellFloorHeight(pos);

			CoordStruct posAir = pos + CoordStruct { 0, 0, pWeaponExt->BeamCannon_LaserHeight };
			CoordStruct posAirEle = pos + CoordStruct { 0, 0, pWeaponExt->BeamCannon_EleHeight };

			if (pWeaponExt->BeamCannon_DrawFromSelf.Get())
			{
				posAir = pThis->GetCoords() + CoordStruct { 0, 0, pWeaponExt->BeamCannon_DrawFromSelf_HeightOffset.Get() };
				posAirEle = pThis->GetCoords() + CoordStruct { 0, 0, pWeaponExt->BeamCannon_DrawFromSelf_HeightOffset.Get() };
			}

			if (pWeaponExt->BeamCannon_DrawLaser.Get())
			{
				LaserDrawClass* pLaser =
					GameCreate<LaserDrawClass>
					(
					posAir,
					pos,
					pWeaponExt->BeamCannon_InnerColor.Get(),
					pWeaponExt->BeamCannon_OuterColor.Get(),
					pWeaponExt->BeamCannon_OuterSpread.Get(),
					pWeaponExt->BeamCannon_Duration.Get()
					);
				// only respected if IsHouseColor
				pLaser->Thickness = pWeaponExt->BeamCannon_Thickness.Get();
				pLaser->IsHouseColor = true;
				// pLaser->IsSupported = this->Type->IsIntense;
			}

			if (pWeaponExt->BeamCannon_DrawEBolt.Get())
			{
				EBolt* pEBolt = GameCreate<EBolt>();
				if (pEBolt != nullptr)
					pEBolt->Fire(posAirEle, pos, 0);
			}

			if (pExt->BeamCannon_ROF > 0)
			{
				pExt->BeamCannon_ROF--;
			}
			else
			{
				WeaponTypeExt::DetonateAt(pBeamCannonWeapon, pos, pThis);
				pExt->BeamCannon_ROF = pWeaponExt->BeamCannon_ROF.Get();
			}

			if (pExt->BeamCannon_LengthIncrease <= pWeaponExt->BeamCannon_LengthIncreaseMax.Get()
				&& pExt->BeamCannon_LengthIncrease >= pWeaponExt->BeamCannon_LengthIncreaseMin.Get())
			{
				pExt->BeamCannon_LengthIncrease += pWeaponExt->BeamCannon_LengthIncreaseAcceleration.Get();
			}

			pExt->BeamCannon_Length += pExt->BeamCannon_LengthIncrease;
		}
		else
		{
			pExt->BeamCannon_setLength = true;
			pExt->BeamCannon_Stop = true;
		}
	}
}

void TechnoExt::ProcessFireSelf(TechnoClass* pThis, TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt)
{
	const ValueableVector<WeaponTypeClass*>& vWeapons = pTypeExt->FireSelf_Weapon.Get(pThis);
	const ValueableVector<int>& vROF = pTypeExt->FireSelf_ROF.Get(pThis);

	if (vWeapons.empty())
		return;

	std::vector<CDTimerClass>& vTimers = pExt->FireSelf_Timers;

	while (vTimers.size() < vWeapons.size())
	{
		size_t idx = vTimers.size();
		int iROF = idx < vROF.size() ? vROF[idx] : vWeapons[idx]->ROF;
		vTimers.emplace_back(pTypeExt->FireSelf_Immediately.Get(pThis) ? 0 : iROF);
	}

	for (size_t i = 0; i < vWeapons.size(); i++)
	{
		if (vTimers[i].Completed())
		{
			int iROF = i < vROF.size() ? vROF[i] : vWeapons[i]->ROF;
			WeaponTypeExt::DetonateAt(vWeapons[i], pThis, pThis);
			vTimers[i].Start(iROF);
		}
	}
}

void TechnoExt::CheckPaintConditions(TechnoClass* pThis, TechnoExt::ExtData* pExt)
{
	if (pExt->AllowToPaint)
	{
		if (pExt->Paint_Count > 0)
		{
			pExt->Paint_Count--;
			pExt->Paint_FramesPassed++;
		}
		else
		{
			pExt->AllowToPaint = false;
			pExt->Paint_IsDiscoColor = false;
			pExt->Paint_FramesPassed = 0;
		}

		if (pExt->Paint_IsDiscoColor)
		{
			auto& colors = pExt->Paint_Colors;

			int transitionCycle = (pExt->Paint_FramesPassed / pExt->Paint_TransitionDuration)
				% colors.size();
			int currentColorIndex = transitionCycle;
			int nextColorIndex = (transitionCycle + 1) % colors.size();
			double blendingCoef = (pExt->Paint_FramesPassed % pExt->Paint_TransitionDuration)
				/ (double)pExt->Paint_TransitionDuration;
			pExt->ColorToPaint = {
				(BYTE)(colors[currentColorIndex].R * (1 - blendingCoef) + colors[nextColorIndex].R * blendingCoef),
				(BYTE)(colors[currentColorIndex].G * (1 - blendingCoef) + colors[nextColorIndex].G * blendingCoef),
				(BYTE)(colors[currentColorIndex].B * (1 - blendingCoef) + colors[nextColorIndex].B * blendingCoef)
			};
		}
	}
}

void TechnoExt::CheckJJConvertConditions(TechnoClass* pThis, TechnoExt::ExtData* pExt)
{
	if (!pExt->needConvertWhenLanding)
		return;

	auto pHouse = pThis->Owner;

	if (!pExt->JJ_landed)
	{
		if (pThis->CurrentMission == Mission::Unload)
		{
			pHouse->OwnedUnitTypes.Decrement(pThis->GetTechnoType()->GetArrayIndex());
			abstract_cast<UnitClass*>(pThis)->Type = static_cast<UnitTypeClass*>(pExt->LandingType);
			pExt->JJ_landed = true;
			pHouse->OwnedUnitTypes.Increment(pThis->GetTechnoType()->GetArrayIndex());
		}
	}
	else
	{
		if (pThis->CurrentMission == Mission::Move)
		{
			pHouse->OwnedUnitTypes.Decrement(pThis->GetTechnoType()->GetArrayIndex());
			abstract_cast<UnitClass*>(pThis)->Type = static_cast<UnitTypeClass*>(pExt->FloatingType);
			pHouse->OwnedUnitTypes.Increment(pThis->GetTechnoType()->GetArrayIndex());
			pExt->JJ_landed = false;
		}
	}
}

void TechnoExt::OccupantsWeaponChange(TechnoClass* pThis, TechnoExt::ExtData* pExt)
{
	if (pThis->WhatAmI() != AbstractType::Building)
		return;

	auto const pBuilding = abstract_cast<BuildingClass*>(pThis);

	if (pBuilding->Occupants.Count > 0)
	{
		int count = pBuilding->Occupants.Count;
		int rofix = 0;
		auto pBuildingTypeExt = BuildingTypeExt::ExtMap.Find(pBuilding->Type);
		auto ROFMultiplier = pBuildingTypeExt->BuildingOccupyROFMult.isset() ? pBuildingTypeExt->BuildingOccupyROFMult :  RulesClass::Instance()->OccupyROFMultiplier;

		if (pThis->Target)
		{
			for (int i = 0; i < count; i++)
			{
				if (pBuilding->GetFireErrorWithoutRange(pThis->Target, 0) == FireError::OK)
					rofix++;

				if (pBuilding->FiringOccupantIndex == pBuilding->Occupants.Count - 1)
					pBuilding->FiringOccupantIndex = 0;
				else
					pBuilding->FiringOccupantIndex++;
			}
		}

		while (pBuilding->GetFireErrorWithoutRange(pThis->Target, 0) == FireError::ILLEGAL && count > 0 && pThis->GetCurrentMission() == Mission::Attack)
		{
			if (pBuilding->FiringOccupantIndex == pBuilding->Occupants.Count - 1)
				pBuilding->FiringOccupantIndex = 0;
			else
				pBuilding->FiringOccupantIndex++;

			count--;
		}

		if (rofix > 0)
			pExt->BuildingROFFix = static_cast<int>(pBuilding->GetWeapon(0)->WeaponType->ROF / (rofix * ROFMultiplier));
	}
}

void TechnoExt::OccupantsVeteranWeapon(TechnoClass* pThis)
{
	if (pThis->WhatAmI() != AbstractType::Building)
		return;

	auto const pBuilding = abstract_cast<BuildingClass*>(pThis);
	auto pExt = TechnoExt::ExtMap.Find(pThis);
	auto pBuildingTypeExt = TechnoTypeExt::ExtMap.Find(pBuilding->Type);

	if (!pBuilding->CanOccupyFire())
		return;

	if (pBuilding->Occupants.Count > 0)
	{
		auto pInf = pBuilding->Occupants.GetItem(pBuilding->FiringOccupantIndex);
		auto pType = pInf->GetTechnoType();
		auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

		if (pTypeExt->OccupyWeapon)
			pExt->PrimaryWeapon = pTypeExt->OccupyWeapon;
		else
			pExt->PrimaryWeapon = pTypeExt->Primary;

		if (pInf->Veterancy.IsElite())
		{
			if (pTypeExt->EliteOccupyWeapon)
				pExt->PrimaryWeapon = pTypeExt->EliteOccupyWeapon;
			else if (pTypeExt->ElitePrimary)
				pExt->PrimaryWeapon = pTypeExt->ElitePrimary;
		}
		else if (pInf->Veterancy.IsVeteran())
		{
			if (pTypeExt->VeteranOccupyWeapon)
				pExt->PrimaryWeapon = pTypeExt->VeteranOccupyWeapon;
			else if (pTypeExt->VeteranPrimary)
				pExt->PrimaryWeapon = pTypeExt->VeteranPrimary;
		}
	}
	else
	{
		pExt->PrimaryWeapon = pBuildingTypeExt->Primary;
	}

	pThis->GetWeapon(0)->WeaponType = pExt->PrimaryWeapon;
}

bool TechnoExt::AttachmentAI(TechnoClass* pThis)
{
	auto const pExt = TechnoExt::ExtMap.Find(pThis);

	if (pExt && pExt->ParentAttachment)
	{
		pExt->ParentAttachment->AI();
		return true;
	}

	return false;
}

void TechnoExt::SelectIFVWeapon(TechnoClass* pThis, TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt)
{
	auto pType = pThis->GetTechnoType();

	pExt->IFVWeapons = pTypeExt->Weapons;
	pExt->IFVTurrets = pTypeExt->Turrets;

	if (pThis->Veterancy.IsVeteran())
		pExt->IFVWeapons = pTypeExt->VeteranWeapons;
	else if (pThis->Veterancy.IsElite())
		pExt->IFVWeapons = pTypeExt->EliteWeapons;

	auto& weapons = pExt->IFVWeapons;
	auto& turrets = pExt->IFVTurrets;

	if (pThis->Passengers.NumPassengers > 0)
	{
		auto pFirstType = pThis->Passengers.FirstPassenger->GetTechnoType();
		if (pFirstType->IFVMode < pType->WeaponCount)
			pThis->GetWeapon(0)->WeaponType = weapons[pFirstType->IFVMode].GetItem(0);
		else
			pThis->GetWeapon(0)->WeaponType = weapons[0].GetItem(0);

		if (pFirstType->IFVMode < pType->TurretCount)
			pThis->CurrentTurretNumber = turrets[pFirstType->IFVMode].GetItem(0);
		else
			pThis->CurrentTurretNumber = turrets[0].GetItem(0);
	}
	else
	{
		pThis->GetWeapon(0)->WeaponType = weapons[0].GetItem(0);
		pThis->CurrentTurretNumber = turrets[0].GetItem(0);
	}
}

void TechnoExt::BuildingPassengerFix(TechnoClass* pThis)
{
	if (pThis->WhatAmI() != AbstractType::Building)
		return;

	auto pType = pThis->GetTechnoType();

	if (pType->Passengers > 0 && pThis->GetWeapon(0)->WeaponType != nullptr)
	{
		if (pThis->Passengers.NumPassengers == 0 && pThis->GetCurrentMission() == Mission::Unload)
		{
			pThis->ForceMission(Mission::Stop);
			pThis->Target = nullptr;
			pThis->ForceMission(Mission::Guard);
		}
	}
}

void TechnoExt::RememeberFirer(TechnoClass* pThis, AbstractClass* pTarget, WeaponTypeClass* pWeapon)
{
	auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);
	if (pWeaponExt->OnlyAllowOneFirer)
	{
		TechnoClass* pTargetTechno = abstract_cast<TechnoClass*>(pTarget);
		auto const pTargetExt = TechnoExt::ExtMap.Find(pTargetTechno);
		pTargetExt->Attacker = pThis;
		pTargetExt->Attacker_Count = pWeapon->ROF;
	}
}

void TechnoExt::ForgetFirer(TechnoClass* pThis, TechnoExt::ExtData* pExt)
{
	if (pExt->Attacker != nullptr)
	{
		if (pExt->Attacker->IsAlive && pExt->Attacker_Count > 0 && pExt->Attacker->GetCurrentMission() == Mission::Attack && pExt->Attacker->Target == pThis)
			pExt->Attacker_Count--;
		else
			pExt->Attacker = nullptr;
	}
	else
		pExt->Attacker_Count = 0;
}

void TechnoExt::BuildingSpawnFix(TechnoClass* pThis)
{
	if (pThis->WhatAmI() != AbstractType::Building)
		return;

	auto const pBuilding = abstract_cast<BuildingClass*>(pThis);

	auto pManager = pBuilding->SpawnManager;
	if (pManager != nullptr)
	{
		for (auto pItem : pManager->SpawnedNodes)
		{
			if (pItem->Status == SpawnNodeStatus::Returning
				&& pItem->Unit != nullptr
				&& pItem->Unit->GetHeight() == 0)
			{
				auto FoundationX = pBuilding->Type->GetFoundationHeight(true), FoundationY = pBuilding->Type->GetFoundationWidth();
				if (FoundationX < 0)
					FoundationX = 0;
				if (FoundationY < 0)
					FoundationY = 0;

				auto adjust = pThis->GetCoords() - CoordStruct { (FoundationX - 1) * 128, (FoundationY - 1) * 128 };

				pItem->Unit->SetLocation(adjust);
			}
		}
	}
}

void TechnoExt::ShieldPowered(TechnoClass* pThis, TechnoExt::ExtData* pExt)
{
	if (pExt->Shield && !pExt->CurrentShieldType->PoweredTechnos.empty())
	{
		std::vector<DynamicVectorClass<bool>> Check;
		Check.resize(pExt->CurrentShieldType->PoweredTechnos.size());

		for (unsigned int i = 0; i < pExt->CurrentShieldType->PoweredTechnos.size(); i++)
		{
			int count = 0;
			for (auto techno : *TechnoClass::Array)
			{
				if (techno->GetTechnoType() == pExt->CurrentShieldType->PoweredTechnos[i] && TechnoExt::IsActive(techno) && techno->Owner == pThis->Owner)
					count++;
			}

			if (count == 0)
				Check[i].AddItem(false);
			else
				Check[i].AddItem(true);
		}

		if (pExt->CurrentShieldType->PoweredTechnos_Any)
		{
			for (unsigned int i = 0; i < pExt->CurrentShieldType->PoweredTechnos.size(); i++)
			{
				if (Check[i].GetItem(0))
					return;
			}
			pExt->Shield->BreakShield();
		}
		else
		{
			for (unsigned int i = 0; i < pExt->CurrentShieldType->PoweredTechnos.size(); i++)
			{
				if (!Check[i].GetItem(0))
				{
					pExt->Shield->BreakShield();
					return;
				}
			}
		}
	}
}

void TechnoExt::PoweredUnit(TechnoClass* pThis, TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt)
{
	if (!pTypeExt->PoweredUnitBy.empty())
	{
		std::vector<DynamicVectorClass<bool>> Check;
		Check.resize(pTypeExt->PoweredUnitBy.size());

		for (unsigned int i = 0; i < pTypeExt->PoweredUnitBy.size(); i++)
		{
			int count = 0;
			for (auto techno : *TechnoClass::Array)
			{
				if (techno->WhatAmI() == AbstractType::Building)
				{
					const auto pBuilding = specific_cast<BuildingClass*>(techno);
					if (pBuilding->GetTechnoType() == pTypeExt->PoweredUnitBy[i] && TechnoExt::IsActive(pBuilding) && pBuilding->IsPowerOnline() && techno->Owner == pThis->Owner)
						count++;
				}
				else
				{
					if (techno->GetTechnoType() == pTypeExt->PoweredUnitBy[i] && TechnoExt::IsActive(techno) && techno->Owner == pThis->Owner)
						count++;
				}
			}

			if (count == 0)
				Check[i].AddItem(false);
			else
				Check[i].AddItem(true);
		}

		if (pTypeExt->PoweredUnitBy_Any)
		{
			for (unsigned int i = 0; i < pTypeExt->PoweredUnitBy.size(); i++)
			{
				if (Check[i].GetItem(0))
				{
					pExt->LosePower = false;
					return;
				}
			}
			pExt->LosePower = true;
		}
		else
		{
			for (unsigned int i = 0; i < pTypeExt->PoweredUnitBy.size(); i++)
			{
				if (!Check[i].GetItem(0))
				{
					pExt->LosePower = true;
					return;
				}
			}
			pExt->LosePower = false;
		}
	}
}

void TechnoExt::PoweredUnitDown(TechnoClass* pThis, TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt)
{
	if (!pTypeExt->PoweredUnitBy.empty())
	{
		if (pExt->LosePower)
		{
			if (!pThis->Deactivated)
			{
				pThis->Deactivate();
			}

			if (pExt->LosePowerParticleCount > 0)
				pExt->LosePowerParticleCount--;
			else
			{
				auto const pRandomX = ScenarioClass::Instance->Random.RandomRanged(pTypeExt->PoweredUnitBy_ParticleSystemXOffset.Get().X, pTypeExt->PoweredUnitBy_ParticleSystemXOffset.Get().Y);
				auto const pRandomY = ScenarioClass::Instance->Random.RandomRanged(pTypeExt->PoweredUnitBy_ParticleSystemYOffset.Get().X, pTypeExt->PoweredUnitBy_ParticleSystemYOffset.Get().Y);
				CoordStruct Location = { pThis->GetCoords().X + pRandomX, pThis->GetCoords().Y + pRandomY, pThis->GetCoords().Z };
				auto ParticleSystem = pTypeExt->PoweredUnitBy_ParticleSystem.isset() ? pTypeExt->PoweredUnitBy_ParticleSystem : RulesClass::Instance()->DefaultSparkSystem;

				GameCreate<ParticleSystemClass>(ParticleSystem, Location, nullptr, nullptr, CoordStruct::Empty, nullptr);

				pExt->LosePowerParticleCount = pTypeExt->PoweredUnitBy_ParticleSystemSpawnDelay;
			}

			auto Sparkles = pTypeExt->PoweredUnitBy_Sparkles.isset() ? pTypeExt->PoweredUnitBy_Sparkles : RulesClass::Instance()->EMPulseSparkles;
			if (pExt->LosePowerAnim == nullptr)
			{
				pExt->LosePowerAnim = GameCreate<AnimClass>(Sparkles, pThis->GetCoords());
				pExt->LosePowerAnim->SetOwnerObject(pThis);
				pExt->LosePowerAnim->RemainingIterations = 0xFFU;
				pExt->LosePowerAnim->Owner = pThis->GetOwningHouse();
			}
		}
		else
		{
			if (pThis->Deactivated)
			{
				pThis->Reactivate();
			}

			if (pExt->LosePowerParticleCount > 0)
				pExt->LosePowerParticleCount = 0;

			if (pExt->LosePowerAnim != nullptr)
			{
				pExt->LosePowerAnim->UnInit();
				pExt->LosePowerAnim = nullptr;
			}
		}
	}
}

void TechnoExt::TechnoUpgradeAnim(TechnoClass* pThis, TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt)
{
	if (pExt->CurrentRank == Rank::Invalid)
		pExt->CurrentRank = pThis->Veterancy.GetRemainingLevel();

	auto Rank = pThis->Veterancy.GetRemainingLevel();
	if (Rank != pExt->CurrentRank)
	{
		pExt->CurrentRank = Rank;

		if (pExt->CurrentRank == Rank::Elite && pTypeExt->EliteAnim.isset())
		{
			if (auto const pAnim = GameCreate<AnimClass>(pTypeExt->EliteAnim, pThis->Location))
			{
				pAnim->SetOwnerObject(pThis);
				pAnim->Owner = pThis->Owner;
			}
		}
		else if (pExt->CurrentRank == Rank::Veteran && pTypeExt->VeteranAnim.isset())
		{
			if (auto const pAnim = GameCreate<AnimClass>(pTypeExt->VeteranAnim, pThis->Location))
			{
				pAnim->SetOwnerObject(pThis);
				pAnim->Owner = pThis->Owner;
			}
		}
	}
}

void TechnoExt::CurePassengers(TechnoClass* pThis, TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt)
{
	if (!TechnoExt::IsActive(pThis))
		return;

	if (pTypeExt->PassengerHeal_Rate > 0)
	{
		if (pThis->Passengers.NumPassengers > 0)
		{
			FootClass* pPassenger = pThis->Passengers.GetFirstPassenger();

			if (pExt->PassengerHealCountDown < 0)
			{
				int timerLength = 0;

				timerLength = pTypeExt->PassengerHeal_Rate;

				pExt->PassengerHealCountDown = timerLength;
				pExt->PassengerHealTimer.Start(timerLength);
			}
			else
			{
				if (pExt->PassengerHealTimer.Completed())
				{
					bool HealFinish = false;
					if (pTypeExt->PassengerHeal_HealAll)
					{
						if (pThis->Passengers.FirstPassenger->GetHealthPercentage() < 1.0 && EnumFunctions::CanTargetHouse(pTypeExt->PassengerHeal_Houses, pThis->Owner, pThis->Passengers.FirstPassenger->Owner))
						{
							if (pThis->Passengers.FirstPassenger->GetTechnoType()->Strength - pThis->Passengers.FirstPassenger->Health >= pTypeExt->PassengerHeal_Amount)
								pThis->Passengers.FirstPassenger->Health += pTypeExt->PassengerHeal_Amount;
							else
								pThis->Passengers.FirstPassenger->Health += pThis->Passengers.FirstPassenger->GetTechnoType()->Strength - pThis->Passengers.FirstPassenger->Health;

							HealFinish = true;
						}

						ObjectClass* pLastPassenger = nullptr;

						while (pPassenger->NextObject)
						{
							pLastPassenger = pPassenger;
							pPassenger = abstract_cast<FootClass*>(pPassenger->NextObject);

							if (pLastPassenger->NextObject->GetHealthPercentage() < 1.0 && EnumFunctions::CanTargetHouse(pTypeExt->PassengerHeal_Houses, pThis->Owner, pLastPassenger->NextObject->GetOwningHouse()))
							{
								if (pLastPassenger->NextObject->GetTechnoType()->Strength - pLastPassenger->NextObject->Health >= pTypeExt->PassengerHeal_Amount)
									pLastPassenger->NextObject->Health += pTypeExt->PassengerHeal_Amount;
								else
									pLastPassenger->NextObject->Health += pLastPassenger->NextObject->GetTechnoType()->Strength - pLastPassenger->NextObject->Health;

								HealFinish = true;
							}
						}
					}
					else
					{
						if (pThis->Passengers.FirstPassenger->GetHealthPercentage() < 1.0 && EnumFunctions::CanTargetHouse(pTypeExt->PassengerHeal_Houses, pThis->Owner, pThis->Passengers.FirstPassenger->GetOwningHouse()))
						{
							if (pThis->Passengers.FirstPassenger->GetTechnoType()->Strength - pThis->Passengers.FirstPassenger->Health >= pTypeExt->PassengerHeal_Amount)
								pThis->Passengers.FirstPassenger->Health += pTypeExt->PassengerHeal_Amount;
							else
								pThis->Passengers.FirstPassenger->Health += pThis->Passengers.FirstPassenger->GetTechnoType()->Strength - pThis->Passengers.FirstPassenger->Health;

							HealFinish = true;
						}
						else
						{
							ObjectClass* pLastPassenger = nullptr;

							while (pPassenger->NextObject)
							{
								pLastPassenger = pPassenger;
								pPassenger = abstract_cast<FootClass*>(pPassenger->NextObject);

								if (pLastPassenger->NextObject->GetHealthPercentage() < 1.0 && EnumFunctions::CanTargetHouse(pTypeExt->PassengerHeal_Houses, pThis->Owner, pLastPassenger->NextObject->GetOwningHouse()))
								{
									if (pLastPassenger->NextObject->GetTechnoType()->Strength - pLastPassenger->NextObject->Health >= pTypeExt->PassengerHeal_Amount)
										pLastPassenger->NextObject->Health += pTypeExt->PassengerHeal_Amount;
									else
										pLastPassenger->NextObject->Health += pLastPassenger->NextObject->GetTechnoType()->Strength - pLastPassenger->NextObject->Health;

									HealFinish = true;
									break;
								}
							}
						}
					}

					if (HealFinish)
					{
						VocClass::PlayAt(pTypeExt->PassengerHeal_Sound, pThis->GetCoords(), nullptr);

						if (pTypeExt->PassengerHeal_Anim.isset())
						{
							const auto pAnimType = pTypeExt->PassengerHeal_Anim.Get();
							if (auto const pAnim = GameCreate<AnimClass>(pAnimType, pThis->Location))
							{
								pAnim->SetOwnerObject(pThis);
								pAnim->Owner = pThis->Owner;
							}
						}
					}

					pExt->PassengerHealTimer.Stop();
					pExt->PassengerHealCountDown = -1;
				}
			}
		}
		else
		{
			pExt->PassengerHealTimer.Stop();
			pExt->PassengerHealCountDown = -1;
		}
	}
}

// Attaches this techno in a first available attachment "slot".
// Returns true if the attachment is successful.
bool TechnoExt::AttachTo(TechnoClass* pThis, TechnoClass* pParent)
{
	auto const pParentExt = TechnoExt::ExtMap.Find(pParent);

	for (auto const& pAttachment : pParentExt->ChildAttachments)
	{
		if (pAttachment->AttachChild(pThis))
			return true;
	}

	return false;
}

bool TechnoExt::DetachFromParent(TechnoClass* pThis, bool isForceDetachment)
{
	auto const pExt = TechnoExt::ExtMap.Find(pThis);
	return pExt->ParentAttachment->DetachChild(isForceDetachment);
}

void TechnoExt::InitializeAttachments(TechnoClass* pThis)
{
	auto const pExt = TechnoExt::ExtMap.Find(pThis);
	auto const pType = pThis->GetTechnoType();
	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

	for (auto& entry : pTypeExt->AttachmentData)
	{
		AttachmentClass* pAttachment = new AttachmentClass(entry.get(), pThis);
		pExt->ChildAttachments.emplace_back(pAttachment);
		pExt->ChildAttachments.back()->Initialize();
	}
}

void TechnoExt::HandleHostDestruction(TechnoClass* pThis)
{
	auto const pExt = TechnoExt::ExtMap.Find(pThis);
	for (auto const& pAttachment : pExt->ChildAttachments)
	{
		pAttachment->Uninitialize();
	}
}

void TechnoExt::Destoryed_EraseAttachment(TechnoClass* pThis)
{
	auto const pExt = TechnoExt::ExtMap.Find(pThis);
	if (pExt->ParentAttachment != nullptr)
	{
		pExt->ParentAttachment->ChildDestroyed();

		TechnoClass* pParent = pExt->ParentAttachment->Parent;
		auto pParentExt = TechnoExt::ExtMap.Find(pParent);
		auto itAttachment = std::find_if
		(
			pParentExt->ChildAttachments.begin(),
			pParentExt->ChildAttachments.end(),
			[pThis](std::unique_ptr<AttachmentClass>& pAttachment)
			{
				return pThis == pAttachment->Child;
			}
		);
		pParentExt->ChildAttachments.erase(itAttachment);

		if (pExt->ParentAttachment->GetType()->DeathTogether_Parent.Get())
			pParent->ReceiveDamage(&pParent->Health, 0, RulesClass::Instance()->C4Warhead, nullptr, true, false, pParent->Owner);

		pExt->ParentAttachment = nullptr;

	}
	for (auto& pAttachment : pExt->ChildAttachments)
	{
		TechnoClass* pChild = pAttachment->Child;
		auto pChildExt = TechnoExt::ExtMap.Find(pChild);
		pChildExt->ParentAttachment = nullptr;
		if (pAttachment->GetType()->DeathTogether_Child.Get())
			pChild->ReceiveDamage(&pChild->Health, 0, RulesClass::Instance()->C4Warhead, nullptr, true, false, pChild->Owner);
	}
	pExt->ChildAttachments.clear();
}

void TechnoExt::UnlimboAttachments(TechnoClass* pThis)
{
	auto const pExt = TechnoExt::ExtMap.Find(pThis);
	for (auto const& pAttachment : pExt->ChildAttachments)
	{
		pAttachment->Unlimbo();
	}
}

void TechnoExt::LimboAttachments(TechnoClass* pThis)
{
	auto const pExt = TechnoExt::ExtMap.Find(pThis);
	for (auto const& pAttachment : pExt->ChildAttachments)
	{
		pAttachment->Limbo();
	}
}

bool TechnoExt::IsParentOf(TechnoClass* pThis, TechnoClass* pOtherTechno)
{
	auto const pExt = TechnoExt::ExtMap.Find(pThis);

	if (!pOtherTechno)
		return false;

	for (auto const& pAttachment : pExt->ChildAttachments)
	{
		if (pAttachment->Child &&
			(pAttachment->Child == pOtherTechno ||
				TechnoExt::IsParentOf(pAttachment->Child, pOtherTechno)))
		{
			return true;
		}
	}

	return false;
}

void TechnoExt::UpdateSharedAmmo(TechnoClass* pThis)
{
	if (!pThis)
		return;

	if (const auto pType = pThis->GetTechnoType())
	{
		if (pType->OpenTopped && pThis->Passengers.NumPassengers > 0)
		{
			if (const auto pExt = TechnoTypeExt::ExtMap.Find(pType))
			{
				if (pExt->Ammo_Shared && pType->Ammo > 0)
				{
					auto passenger = pThis->Passengers.FirstPassenger;
					TechnoTypeClass* passengerType;

					do
					{
						passengerType = passenger->GetTechnoType();
						auto pPassengerExt = TechnoTypeExt::ExtMap.Find(passengerType);

						if (pPassengerExt && pPassengerExt->Ammo_Shared)
						{
							if (pExt->Ammo_Shared_Group < 0 || pExt->Ammo_Shared_Group == pPassengerExt->Ammo_Shared_Group)
							{
								if (pThis->Ammo > 0 && (passenger->Ammo < passengerType->Ammo))
								{
									pThis->Ammo--;
									passenger->Ammo++;
								}
							}
						}

						passenger = static_cast<FootClass*>(passenger->NextObject);
					}
					while (passenger);
				}
			}
		}
	}
}

void TechnoExt::UpdateMindControlAnim(TechnoClass* pThis)
{
	if (const auto pExt = TechnoExt::ExtMap.Find(pThis))
	{
		if (pThis->IsMindControlled())
		{
			if (pThis->MindControlRingAnim && !pExt->MindControlRingAnimType)
			{
				pExt->MindControlRingAnimType = pThis->MindControlRingAnim->Type;
			}
			else if (!pThis->MindControlRingAnim && pExt->MindControlRingAnimType &&
				pThis->CloakState == CloakState::Uncloaked && !pThis->InLimbo && pThis->IsAlive)
			{
				auto coords = CoordStruct::Empty;
				coords = *pThis->GetCoords(&coords);
				int offset = 0;

				if (const auto pBuilding = specific_cast<BuildingClass*>(pThis))
					offset = Unsorted::LevelHeight * pBuilding->Type->Height;
				else
					offset = pThis->GetTechnoType()->MindControlRingOffset;

				coords.Z += offset;
				auto anim = GameCreate<AnimClass>(pExt->MindControlRingAnimType, coords, 0, 1);

				if (anim)
				{
					pThis->MindControlRingAnim = anim;
					pThis->MindControlRingAnim->SetOwnerObject(pThis);

					if (pThis->WhatAmI() == AbstractType::Building)
						pThis->MindControlRingAnim->ZAdjust = -1024;
				}
			}
		}
		else if (pExt->MindControlRingAnimType)
		{
			pExt->MindControlRingAnimType = nullptr;
		}
	}
}

void TechnoExt::ApplyGainedSelfHeal(TechnoClass* pThis)
{
	int healthDeficit = pThis->GetTechnoType()->Strength - pThis->Health;

	if (pThis->Health && healthDeficit > 0)
	{
		if (auto const pExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType()))
		{
			bool isBuilding = pThis->WhatAmI() == AbstractType::Building;
			bool isOrganic = pThis->WhatAmI() == AbstractType::Infantry || pThis->WhatAmI() == AbstractType::Unit && pThis->GetTechnoType()->Organic;
			auto defaultSelfHealType = isBuilding ? SelfHealGainType::None : isOrganic ? SelfHealGainType::Infantry : SelfHealGainType::Units;
			auto selfHealType = pExt->SelfHealGainType.Get(defaultSelfHealType);

			if (selfHealType == SelfHealGainType::None)
				return;

			bool applyHeal = false;
			int amount = 0;

			if (selfHealType == SelfHealGainType::Infantry)
			{
				int count = RulesExt::Global()->InfantryGainSelfHealCap.isset() ?
					std::min(std::max(RulesExt::Global()->InfantryGainSelfHealCap.Get(), 1), pThis->Owner->InfantrySelfHeal) :
					pThis->Owner->InfantrySelfHeal;

				amount = RulesClass::Instance->SelfHealInfantryAmount * count;

				if (!(Unsorted::CurrentFrame % RulesClass::Instance->SelfHealInfantryFrames) && amount)
					applyHeal = true;
			}
			else
			{
				int count = RulesExt::Global()->UnitsGainSelfHealCap.isset() ?
					std::min(std::max(RulesExt::Global()->UnitsGainSelfHealCap.Get(), 1), pThis->Owner->UnitsSelfHeal) :
					pThis->Owner->UnitsSelfHeal;

				amount = RulesClass::Instance->SelfHealUnitAmount * count;

				if (!(Unsorted::CurrentFrame % RulesClass::Instance->SelfHealUnitFrames) && amount)
					applyHeal = true;
			}

			if (applyHeal && amount)
			{
				if (amount >= healthDeficit)
					amount = healthDeficit;

				bool wasDamaged = pThis->GetHealthPercentage() <= RulesClass::Instance->ConditionYellow;

				pThis->Health += amount;

				if (wasDamaged && (pThis->GetHealthPercentage() > RulesClass::Instance->ConditionYellow
					|| pThis->GetHeight() < -10))
				{
					if (auto const pBuilding = abstract_cast<BuildingClass*>(pThis))
					{
						pBuilding->UpdatePlacement(PlacementType::Redraw);
						pBuilding->ToggleDamagedAnims(false);
					}

					if (pThis->WhatAmI() == AbstractType::Unit || pThis->WhatAmI() == AbstractType::Building)
					{
						auto dmgParticle = pThis->DamageParticleSystem;

						if (dmgParticle)
							dmgParticle->UnInit();
					}
				}
			}
		}
	}

	return;
}

void TechnoExt::SyncIronCurtainStatus(TechnoClass* pFrom, TechnoClass* pTo)
{
	if (pFrom->IsIronCurtained() && !pFrom->ForceShielded)
	{
		const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pFrom->GetTechnoType());
		if (pTypeExt->IronCurtain_KeptOnDeploy.Get(RulesExt::Global()->IronCurtain_KeptOnDeploy))
		{
			pTo->IronCurtain(pFrom->IronCurtainTimer.GetTimeLeft(), pFrom->Owner, false);
			pTo->IronTintStage = pFrom->IronTintStage;
		}
	}
}

void TechnoExt::ApplyMobileRefinery(TechnoClass* pThis)
{
	if (abstract_cast<FootClass*>(pThis) == nullptr)
		return;

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (!pTypeExt || !pTypeExt->MobileRefinery || (pTypeExt->MobileRefinery_TransRate > 0 &&
		Unsorted::CurrentFrame % pTypeExt->MobileRefinery_TransRate))
		return;

	size_t cellCount = Math::max(pTypeExt->MobileRefinery_FrontOffset.size(), pTypeExt->MobileRefinery_LeftOffset.size());

	if (!cellCount)
		cellCount = 1;

	CoordStruct flh = { 0,0,0 };

	for (size_t idx = 0; idx < cellCount; idx++)
	{
		flh.X = static_cast<int>(pTypeExt->MobileRefinery_FrontOffset.size() > idx ? pTypeExt->MobileRefinery_FrontOffset[idx] * Unsorted::LeptonsPerCell : 0);
		flh.Y = static_cast<int>(pTypeExt->MobileRefinery_LeftOffset.size() > idx ? pTypeExt->MobileRefinery_LeftOffset[idx] * Unsorted::LeptonsPerCell : 0);
		CoordStruct pos = TechnoExt::GetFLHAbsoluteCoords(pThis, flh, false);
		CellClass* pCell = MapClass::Instance->TryGetCellAt(pos);

		if (!pCell)
			return;

		auto loc = pCell->GetCoords();
		loc.Z = pThis->Location.Z;
		int tValue = pCell->GetContainedTiberiumValue();

		if (tValue)
		{
			int tibValue = TiberiumClass::Array->GetItem(pCell->GetContainedTiberiumIndex())->Value;
			int tAmount = static_cast<int>(tValue * 1.0 / tibValue);
			int amount = pTypeExt->MobileRefinery_AmountPerCell ? Math::min(pTypeExt->MobileRefinery_AmountPerCell, tAmount) : tAmount;
			pCell->ReduceTiberium(amount);
			int value = static_cast<int>(amount * tibValue * pTypeExt->MobileRefinery_CashMultiplier);
			pThis->Owner->TransactMoney(value);

			if (pTypeExt->MobileRefinery_Display)
			{
				Point2D location = { 0,0 };
				TacticalClass::Instance->CoordsToScreen(&location, &loc);
				location -= TacticalClass::Instance->TacticalPos;
				RectangleStruct rect = DSurface::Temp->GetRect();
				RectangleStruct bound = { location.X, location.Y, 10, 12 };

				if (bound.X > 0 && bound.X + bound.Width < rect.Width &&
					bound.Y > 0 && bound.Y + bound.Height < rect.Height - 32)
				{
					ColorStruct color = pTypeExt->MobileRefinery_DisplayColor;
					wchar_t moneyStr[0x20];
					swprintf_s(moneyStr, L"%ls%ls%d", L"+", Phobos::UI::CostLabel, value);
					FlyingStrings::Add(moneyStr, loc, color);
				}
			}

			if (!pTypeExt->MobileRefinery_Anims.empty())
			{
				AnimTypeClass* pAnimType = nullptr;
				int facing = pThis->PrimaryFacing.current().value8();

				if (facing >= 7)
					facing = 0;
				else
					facing++;

				switch (pTypeExt->MobileRefinery_Anims.size())
				{
				case 1:
					pAnimType = pTypeExt->MobileRefinery_Anims[0];
					break;
				case 8:
					pAnimType = pTypeExt->MobileRefinery_Anims[facing];
					break;
				default:
					pAnimType = pTypeExt->MobileRefinery_Anims[
						ScenarioClass::Instance->Random.RandomRanged(0, pTypeExt->MobileRefinery_Anims.size() - 1)];
					break;
				}

				if (pAnimType)
				{
					if (auto pAnim = GameCreate<AnimClass>(pAnimType, pos))
					{
						pAnim->Owner = pThis->Owner;

						if (pTypeExt->MobileRefinery_AnimMove)
							pAnim->SetOwnerObject(pThis);
					}
				}
			}
		}
	}
}

void TechnoExt::DrawSelfHealPips(TechnoClass* pThis, Point2D* pLocation, RectangleStruct* pBounds)
{
	bool drawPip = false;
	bool isInfantryHeal = false;
	int selfHealFrames = 0;
	Point2D Offset = { 0,0 };

	if (auto const pExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType()))
	{
		if (pExt->SelfHealGainType.isset() && pExt->SelfHealGainType.Get() == SelfHealGainType::None)
			return;

		bool hasInfantrySelfHeal = pExt->SelfHealGainType.isset() && pExt->SelfHealGainType.Get() == SelfHealGainType::Infantry;
		bool hasUnitSelfHeal = pExt->SelfHealGainType.isset() && pExt->SelfHealGainType.Get() == SelfHealGainType::Units;
		bool isOrganic = false;

		if (pThis->WhatAmI() == AbstractType::Infantry ||
			pThis->GetTechnoType()->Organic && pThis->WhatAmI() == AbstractType::Unit)
		{
			isOrganic = true;
		}

		if (pThis->Owner->InfantrySelfHeal > 0 && (hasInfantrySelfHeal || isOrganic))
		{
			drawPip = true;
			selfHealFrames = RulesClass::Instance->SelfHealInfantryFrames;
			isInfantryHeal = true;
		}
		else if (pThis->Owner->UnitsSelfHeal > 0 && (hasUnitSelfHeal || pThis->WhatAmI() == AbstractType::Unit))
		{
			drawPip = true;
			selfHealFrames = RulesClass::Instance->SelfHealUnitFrames;
		}

		Offset = pExt->SelfHealPips_Offset.Get();
	}

	if (drawPip)
	{
		Valueable<Point2D> pipFrames;
		bool isSelfHealFrame = false;
		int xOffset = 0;
		int yOffset = 0;

		if (Unsorted::CurrentFrame % selfHealFrames <= 5
			&& pThis->Health < pThis->GetTechnoType()->Strength)
		{
			isSelfHealFrame = true;
		}

		if (pThis->WhatAmI() == AbstractType::Unit || pThis->WhatAmI() == AbstractType::Aircraft)
		{
			auto& offset = RulesExt::Global()->Pips_SelfHeal_Units_Offset.Get();
			pipFrames = RulesExt::Global()->Pips_SelfHeal_Units;
			xOffset = offset.X;
			yOffset = offset.Y + pThis->GetTechnoType()->PixelSelectionBracketDelta;
		}
		else if (pThis->WhatAmI() == AbstractType::Infantry)
		{
			auto& offset = RulesExt::Global()->Pips_SelfHeal_Infantry_Offset.Get();
			pipFrames = RulesExt::Global()->Pips_SelfHeal_Infantry;
			xOffset = offset.X;
			yOffset = offset.Y + pThis->GetTechnoType()->PixelSelectionBracketDelta;
		}
		else
		{
			auto pType = abstract_cast<BuildingTypeClass*>(pThis->GetTechnoType());
			int fHeight = pType->GetFoundationHeight(false);
			int yAdjust = -Unsorted::CellHeightInPixels / 2;

			auto& offset = RulesExt::Global()->Pips_SelfHeal_Buildings_Offset.Get();
			pipFrames = RulesExt::Global()->Pips_SelfHeal_Buildings;
			xOffset = offset.X + Unsorted::CellWidthInPixels / 2 * fHeight;
			yOffset = offset.Y + yAdjust * fHeight + pType->Height * yAdjust;
		}

		int pipFrame = isInfantryHeal ? pipFrames.Get().X : pipFrames.Get().Y;

		Point2D position = { pLocation->X + xOffset + Offset.X, pLocation->Y + yOffset + Offset.Y };

		auto flags = BlitterFlags::bf_400 | BlitterFlags::Centered;

		if (isSelfHealFrame)
			flags = flags | BlitterFlags::Darken;

		DSurface::Temp->DrawSHP(FileSystem::PALETTE_PAL, FileSystem::PIPS_SHP,
		pipFrame, &position, pBounds, flags, 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
	}
}

//自定义编号 - 建筑
void TechnoExt::DrawGroupID_Building(TechnoClass* pThis, TechnoTypeExt::ExtData* pTypeExt, Point2D* pLocation)
{
	CoordStruct vCoords = { 0, 0, 0 };
	pThis->GetTechnoType()->Dimension2(&vCoords);
	Point2D vPos2 = { 0, 0 };
	CoordStruct vCoords2 = { -vCoords.X / 2, vCoords.Y / 2,vCoords.Z };
	TacticalClass::Instance->CoordsToScreen(&vPos2, &vCoords2);

	Point2D vLoc = *pLocation;
	Point2D vPos = { 0, 0 };
	Point2D vOffset = pTypeExt->GroupID_Offset.Get();

	vPos.X = vLoc.X + vOffset.X;
	vPos.Y = vPos2.Y + vLoc.Y + vOffset.Y + 16;

	if (pThis->Group >= 0)
	{
		const COLORREF GroupIDColor = Drawing::RGB2DWORD(pThis->GetOwningHouse()->Color.R, pThis->GetOwningHouse()->Color.G, pThis->GetOwningHouse()->Color.B);

		RectangleStruct rect
		{
			vPos.X,
			vPos.Y,
			11,13
		};

		DSurface::Temp->FillRect(&rect, COLOR_BLACK);
		DSurface::Temp->DrawRect(&rect, GroupIDColor);

		int groupid = (pThis->Group == 9) ? 0 : (pThis->Group + 1);

		wchar_t GroupID[0x20];
		swprintf_s(GroupID, L"%d", groupid);

		Point2D vGroupPos
		{
			vPos.X + 3,
			vPos.Y - 2
		};

		TextPrintType PrintType = TextPrintType(int(TextPrintType::NoShadow));

		DSurface::Temp->GetRect(&rect);
		DSurface::Temp->DrawTextA(GroupID, &rect, &vGroupPos, GroupIDColor, 0, PrintType);
	}
}

//自定义编号 - 单位
void TechnoExt::DrawGroupID_Other(TechnoClass* pThis, TechnoTypeExt::ExtData* pTypeExt, Point2D* pLocation)
{
	Point2D vLoc = *pLocation;

	Point2D vOffset = pTypeExt->GroupID_Offset.Get();

	int XOffset = vOffset.X;
	int YOffset = vOffset.Y + pThis->GetTechnoType()->PixelSelectionBracketDelta;

	vLoc.X += XOffset;
	vLoc.Y += YOffset;

	if (pThis->Group >= 0)
	{
		if (pThis->WhatAmI() == AbstractType::Infantry)
		{
			vLoc.X -= 7;
			vLoc.Y -= 37;
		}
		else
		{
			vLoc.X -= 17;
			vLoc.Y -= 38;
		}

		const COLORREF GroupIDColor = Drawing::RGB2DWORD(pThis->GetOwningHouse()->Color.R, pThis->GetOwningHouse()->Color.G, pThis->GetOwningHouse()->Color.B);

		RectangleStruct rect
		{
			vLoc.X,
			vLoc.Y,
			11,13
		};

		DSurface::Temp->FillRect(&rect, COLOR_BLACK);
		DSurface::Temp->DrawRect(&rect, GroupIDColor);

		int groupid = (pThis->Group == 9) ? 0 : (pThis->Group + 1);

		wchar_t GroupID[0x20];
		swprintf_s(GroupID, L"%d", groupid);

		DSurface::Temp->GetRect(&rect);

		Point2D vGroupPos
		{
			vLoc.X + 3,
			vLoc.Y - 2
		};

		TextPrintType PrintType = TextPrintType(int(TextPrintType::NoShadow));

		DSurface::Temp->DrawTextA(GroupID, &rect, &vGroupPos, GroupIDColor, 0, PrintType);
	}
}

void TechnoExt::DrawHealthBar_Building(TechnoClass* pThis, TechnoTypeExt::ExtData* pTypeExt, int iLength, Point2D* pLocation, RectangleStruct* pBound)
{
	CoordStruct vCoords = { 0, 0, 0 };
	pThis->GetTechnoType()->Dimension2(&vCoords);
	Point2D vPos2 = { 0, 0 };
	CoordStruct vCoords2 = { -vCoords.X / 2, vCoords.Y / 2,vCoords.Z };
	TacticalClass::Instance->CoordsToScreen(&vPos2, &vCoords2);

	Point2D vLoc = *pLocation;
	vLoc.Y += 1;

	Point2D vPos = { 0, 0 };

	SHPStruct* PipsSHP = pTypeExt->SHP_PipsSHP;
	if (PipsSHP == nullptr)
	{
		char FilenameSHP[0x20];
		strcpy_s(FilenameSHP, pTypeExt->HealthBar_PipsSHP.data());

		if (strcmp(FilenameSHP, "") == 0)
			PipsSHP = pTypeExt->SHP_PipsSHP = FileSystem::PIPS_SHP;
		else
			PipsSHP = pTypeExt->SHP_PipsSHP = FileSystem::LoadSHPFile(FilenameSHP);
	}
	if (PipsSHP == nullptr) return;

	ConvertClass* PipsPAL = pTypeExt->SHP_PipsPAL;
	if (PipsPAL == nullptr)
	{
		char FilenamePAL[0x20];
		strcpy_s(FilenamePAL, pTypeExt->HealthBar_PipsPAL.data());

		if (strcmp(FilenamePAL, "") == 0)
			PipsPAL = pTypeExt->SHP_PipsPAL = FileSystem::PALETTE_PAL;
		else
			PipsPAL = pTypeExt->SHP_PipsPAL = FileSystem::LoadPALFile(FilenamePAL, DSurface::Temp);
	}
	if (PipsPAL == nullptr) return;

	const int iTotal = DrawHealthBar_PipAmount(pThis, pTypeExt, iLength);
	int frame = DrawHealthBar_Pip(pThis, pTypeExt, true);

	if (iTotal > 0)
	{
		int frameIdx, deltaX, deltaY;
		for (frameIdx = iTotal, deltaX = 0, deltaY = 0;
			frameIdx;
			frameIdx--, deltaX += 4, deltaY -= 2)
		{
			vPos.X = vPos2.X + vLoc.X + 4 * iLength + 3 - deltaX;
			vPos.Y = vPos2.Y + vLoc.Y - 2 * iLength + 4 - deltaY;

			DSurface::Composite->DrawSHP(PipsPAL, PipsSHP,
				frame, &vPos, pBound, BlitterFlags(0x600), 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
		}
	}

	if (iTotal < iLength)
	{
		int frameIdx, deltaX, deltaY;
		for (frameIdx = iLength - iTotal, deltaX = 4 * iTotal, deltaY = -2 * iTotal;
			frameIdx;
			frameIdx--, deltaX += 4, deltaY -= 2)
		{
			vPos.X = vPos2.X + vLoc.X + 4 * iLength + 3 - deltaX;
			vPos.Y = vPos2.Y + vLoc.Y - 2 * iLength + 4 - deltaY;

			DSurface::Composite->DrawSHP(PipsPAL, PipsSHP,
				0, &vPos, pBound, BlitterFlags(0x600), 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
		}
	}

}

void TechnoExt::DrawHealthBar_Other(TechnoClass* pThis, TechnoTypeExt::ExtData* pTypeExt, int iLength, Point2D* pLocation, RectangleStruct* pBound)
{
	Point2D vPos = { 0,0 };
	Point2D vLoc = *pLocation;

	int frame, XOffset, YOffset;// , XOffset2;
	YOffset = pThis->GetTechnoType()->PixelSelectionBracketDelta;
	vLoc.Y -= 5;

	vLoc.X += pTypeExt->HealthBar_XOffset.Get();

	if (iLength == 8)
	{
		vPos.X = vLoc.X + 11;
		vPos.Y = vLoc.Y - 20 + YOffset;
		frame = 1;
		XOffset = -5;
		YOffset -= 19;
	}
	else
	{
		vPos.X = vLoc.X + 1;
		vPos.Y = vLoc.Y - 21 + YOffset;
		frame = 0;
		XOffset = -15;
		YOffset -= 20;
	}

	SHPStruct* PipsSHP = pTypeExt->SHP_PipsSHP;
	if (PipsSHP == nullptr)
	{
		char FilenameSHP[0x20];
		strcpy_s(FilenameSHP, pTypeExt->HealthBar_PipsSHP.data());

		if (strcmp(FilenameSHP, "") == 0)
			PipsSHP = pTypeExt->SHP_PipsSHP = FileSystem::PIPS_SHP;
		else
			PipsSHP = pTypeExt->SHP_PipsSHP = FileSystem::LoadSHPFile(FilenameSHP);
	}
	if (PipsSHP == nullptr) return;

	ConvertClass* PipsPAL = pTypeExt->SHP_PipsPAL;
	if (PipsPAL == nullptr)
	{
		char FilenamePAL[0x20];
		strcpy_s(FilenamePAL, pTypeExt->HealthBar_PipsPAL.data());

		if (strcmp(FilenamePAL, "") == 0)
			PipsPAL = pTypeExt->SHP_PipsPAL = FileSystem::PALETTE_PAL;
		else
			PipsPAL = pTypeExt->SHP_PipsPAL = FileSystem::LoadPALFile(FilenamePAL, DSurface::Temp);
	}
	if (PipsPAL == nullptr) return;

	SHPStruct* PipBrdSHP = pTypeExt->SHP_PipBrdSHP;
	if (PipBrdSHP == nullptr)
	{
		char FilenameSHP[0x20];
		strcpy_s(FilenameSHP, pTypeExt->HealthBar_PipBrdSHP.data());

		if (strcmp(FilenameSHP, "") == 0)
			PipBrdSHP = pTypeExt->SHP_PipBrdSHP = FileSystem::PIPBRD_SHP;
		else
			PipBrdSHP = pTypeExt->SHP_PipBrdSHP = FileSystem::LoadSHPFile(FilenameSHP);
	}
	if (PipBrdSHP == nullptr) return;

	ConvertClass* PipBrdPAL = pTypeExt->SHP_PipBrdPAL;
	if (PipBrdPAL == nullptr)
	{
		char FilenamePAL[0x20];
		strcpy_s(FilenamePAL, pTypeExt->HealthBar_PipBrdPAL.data());

		if (strcmp(FilenamePAL, "") == 0)
			PipBrdPAL = pTypeExt->SHP_PipBrdPAL = FileSystem::PALETTE_PAL;
		else
			PipBrdPAL = pTypeExt->SHP_PipBrdPAL = FileSystem::LoadPALFile(FilenamePAL, DSurface::Temp);
	}
	if (PipBrdPAL == nullptr) return;

	if (pThis->IsSelected)
	{
		Point2D PipBrdOffset = pTypeExt->HealthBar_PipBrdOffset.Get();

		vPos.X += PipBrdOffset.X;
		vPos.Y += PipBrdOffset.Y;

		DSurface::Temp->DrawSHP(PipBrdPAL, PipBrdSHP,
			pTypeExt->HealthBar_PipBrd.Get(frame), &vPos, pBound, BlitterFlags(0xE00), 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
	}

	iLength = pTypeExt->HealthBar_PipsLength.Get(iLength);
	const int iTotal = DrawHealthBar_PipAmount(pThis, pTypeExt, iLength);

	frame = DrawHealthBar_Pip(pThis, pTypeExt, false);

	Point2D DrawOffset = pTypeExt->HealthBar_Pips_DrawOffset.Get({ 2,0 });

	for (int i = 0; i < iTotal; ++i)
	{
		vPos.X = vLoc.X + XOffset + DrawOffset.X * i;
		vPos.Y = vLoc.Y + YOffset + DrawOffset.Y * i;

		DSurface::Temp->DrawSHP(PipsPAL, PipsSHP,
			frame, &vPos, pBound, BlitterFlags(0x600), 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
	}
}

int TechnoExt::DrawHealthBar_Pip(TechnoClass* pThis, TechnoTypeExt::ExtData* pTypeExt, const bool isBuilding)
{
	const auto strength = pThis->GetTechnoType()->Strength;

	const auto Pip = (isBuilding ? pTypeExt->HealthBar_Pips.Get(RulesExt::Global()->Pips_Buildings.Get()) :
		pTypeExt->HealthBar_Pips.Get(RulesExt::Global()->Pips.Get()));

	if (pThis->Health > RulesClass::Instance->ConditionYellow * strength && Pip.X != -1)
		return Pip.X;
	else if (pThis->Health > RulesClass::Instance->ConditionRed * strength && (Pip.Y != -1 || Pip.X != -1))
		return Pip.Y == -1 ? Pip.X : Pip.Y;
	else if (Pip.Z != -1 || Pip.X != -1)
		return Pip.Z == -1 ? Pip.X : Pip.Z;

	return isBuilding ? 5 : 16;
}

int TechnoExt::DrawHealthBar_PipAmount(TechnoClass* pThis, TechnoTypeExt::ExtData* pTypeExt, int iLength)
{
	return pThis->Health > 0
		? Math::clamp((int)round(GetHealthRatio(pThis) * iLength), 1, iLength)
		: 0;
}

double TechnoExt::GetHealthRatio(TechnoClass* pThis)
{
	return static_cast<double>(pThis->Health) / pThis->GetTechnoType()->Strength;
}

// Based on Ares source.
void TechnoExt::DrawInsignia(TechnoClass* pThis, Point2D* pLocation, RectangleStruct* pBounds)
{
	Point2D offset = *pLocation;

	SHPStruct* pShapeFile = FileSystem::PIPS_SHP;
	int defaultFrameIndex = -1;

	auto pTechnoType = pThis->GetTechnoType();
	auto pOwner = pThis->Owner;

	if (pThis->IsDisguised() && !pThis->IsClearlyVisibleTo(HouseClass::Player))
	{
		if (auto const pType = TechnoTypeExt::GetTechnoType(pThis->Disguise))
		{
			pTechnoType = pType;
			pOwner = pThis->DisguisedAsHouse;
		}
		else if (!pOwner->IsAlliedWith(HouseClass::Player) && !HouseClass::IsPlayerObserver())
		{
			return;
		}
	}

	TechnoTypeExt::ExtData* pExt = TechnoTypeExt::ExtMap.Find(pTechnoType);

	bool isVisibleToPlayer = (pOwner && pOwner->IsAlliedWith(HouseClass::Player))
		|| HouseClass::IsPlayerObserver()
		|| pExt->Insignia_ShowEnemy.Get(RulesExt::Global()->EnemyInsignia);

	if (!isVisibleToPlayer)
		return;

	bool isCustomInsignia = false;

	if (SHPStruct* pCustomShapeFile = pExt->Insignia.Get(pThis))
	{
		pShapeFile = pCustomShapeFile;
		defaultFrameIndex = 0;
		isCustomInsignia = true;
	}
	VeterancyStruct* pVeterancy = &pThis->Veterancy;
	auto& insigniaFrames = pExt->InsigniaFrames.Get();
	int insigniaFrame = insigniaFrames.X;

	if (pVeterancy->IsVeteran())
	{
		defaultFrameIndex = !isCustomInsignia ? 14 : defaultFrameIndex;
		insigniaFrame = insigniaFrames.Y;
	}
	else if (pVeterancy->IsElite())
	{
		defaultFrameIndex = !isCustomInsignia ? 15 : defaultFrameIndex;
		insigniaFrame = insigniaFrames.Z;
	}

	int frameIndex = pExt->InsigniaFrame.Get(pThis);
	frameIndex = frameIndex == -1 ? insigniaFrame : frameIndex;

	if (frameIndex == -1)
		frameIndex = defaultFrameIndex;

	if (frameIndex != -1 && pShapeFile)
	{
		offset.X += 5;
		offset.Y += 2;

		if (pThis->WhatAmI() != AbstractType::Infantry)
		{
			offset.X += 5;
			offset.Y += 4;
		}

		DSurface::Temp->DrawSHP(
			FileSystem::PALETTE_PAL, pShapeFile, frameIndex, &offset, pBounds, BlitterFlags(0xE00), 0, -2, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
	}

	return;
}

double TechnoExt::GetCurrentSpeedMultiplier(FootClass* pThis)
{
	double houseMultiplier = 1.0;

	if (pThis->WhatAmI() == AbstractType::Aircraft)
		houseMultiplier = pThis->Owner->Type->SpeedAircraftMult;
	else if (pThis->WhatAmI() == AbstractType::Infantry)
		houseMultiplier = pThis->Owner->Type->SpeedInfantryMult;
	else
		houseMultiplier = pThis->Owner->Type->SpeedUnitsMult;

	return pThis->SpeedMultiplier * houseMultiplier *
		(pThis->HasAbility(Ability::Faster) ? RulesClass::Instance->VeteranSpeed : 1.0);
}

void TechnoExt::DrawHealthBar_Picture(TechnoClass* pThis, TechnoTypeExt::ExtData* pTypeExt, int iLength, Point2D* pLocation, RectangleStruct* pBound)
{
	Point2D vPos = { 0,0 };
	Point2D vLoc = *pLocation;

	int YOffset;
	YOffset = pThis->GetTechnoType()->PixelSelectionBracketDelta;
	vLoc.Y -= 5;

	vLoc.X += pTypeExt->HealthBar_XOffset.Get();

	if (iLength == 8)
	{
		vPos.X = vLoc.X + 11;
		vPos.Y = vLoc.Y - 20 + YOffset;
	}
	else
	{
		vPos.X = vLoc.X + 1;
		vPos.Y = vLoc.Y - 21 + YOffset;
	}

	SHPStruct* PictureSHP = pTypeExt->SHP_PictureSHP;
	if (PictureSHP == nullptr)
	{
		char FilenameSHP[0x20];
		strcpy_s(FilenameSHP, pTypeExt->HealthBar_PictureSHP.data());

		if (strcmp(FilenameSHP, "") == 0)
			PictureSHP = pTypeExt->SHP_PictureSHP = FileSystem::PIPS_SHP;
		else
			PictureSHP = pTypeExt->SHP_PictureSHP = FileSystem::LoadSHPFile(FilenameSHP);
	}
	if (PictureSHP == nullptr) return;

	ConvertClass* PicturePAL = pTypeExt->SHP_PicturePAL;
	if (PicturePAL == nullptr)
	{
		char FilenamePAL[0x20];
		strcpy_s(FilenamePAL, pTypeExt->HealthBar_PicturePAL.data());

		if (strcmp(FilenamePAL, "") == 0)
			PicturePAL = pTypeExt->SHP_PicturePAL = FileSystem::PALETTE_PAL;
		else
			PicturePAL = pTypeExt->SHP_PicturePAL = FileSystem::LoadPALFile(FilenamePAL, DSurface::Temp);
	}
	if (PicturePAL == nullptr) return;

	iLength = pTypeExt->HealthBar_PipsLength.Get(iLength);
	const int iTotal = DrawHealthBar_PipAmount(pThis, pTypeExt, iLength);

	vPos.X += pTypeExt->HealthBar_XOffset.Get();

	DSurface::Temp->DrawSHP(PicturePAL, PictureSHP,
		iTotal, &vPos, pBound, EnumFunctions::GetTranslucentLevel(pTypeExt->HealthBar_PictureTransparency.Get()), 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
}

void TechnoExt::DrawSelectBox(TechnoClass* pThis, TechnoTypeExt::ExtData* pTypeExt, Point2D* pLocation, RectangleStruct* pBound, bool isInfantry)
{
	const auto canHouse = pTypeExt->SelectBox_CanSee.Get(RulesExt::Global()->SelectBox_CanSee);
	bool canSee = false;

	if (HouseClass::IsPlayerObserver())
	{
		if (pTypeExt->SelectBox_CanObserverSee.Get(RulesExt::Global()->SelectBox_CanObserverSee))
		{
			canSee = true;
		}
	}
	else
	{
		switch (canHouse)
		{
		case AffectedHouse::All:
			canSee = true;
			break;

		case AffectedHouse::Owner:
			if (pThis->Owner->ControlledByPlayer())
				canSee = true;
			break;

		case AffectedHouse::NotOwner:
			if (!pThis->Owner->ControlledByPlayer())
				canSee = true;
			break;

		case AffectedHouse::Allies:
		case AffectedHouse::Team:
			if (pThis->Owner->IsAlliedWith(HouseClass::Player))
				canSee = true;
			break;

		case AffectedHouse::Enemies:
		case AffectedHouse::NotAllies:
			if (!pThis->Owner->IsAlliedWith(HouseClass::Player))
				canSee = true;
			break;

		case AffectedHouse::None:
		default:
			break;
		}
	}

	if (!canSee)
		return;

	int frame;
	Point2D vPos = { 0, 0 };
	Point2D vOffset = pTypeExt->SelectBox_DrawOffset.Get(isInfantry ?
		RulesExt::Global()->SelectBox_DrawOffset_Infantry.Get() : RulesExt::Global()->SelectBox_DrawOffset_Unit.Get());

	Vector3D<int> glbSelectboxFrame = isInfantry ?
		RulesExt::Global()->SelectBox_Frame_Infantry.Get() :
		RulesExt::Global()->SelectBox_Frame_Unit.Get();
	Vector3D<int> selectboxFrame = pTypeExt->SelectBox_Frame.Get();
	auto const nFlag = BlitterFlags::Centered | BlitterFlags::Nonzero | BlitterFlags::MultiPass | EnumFunctions::GetTranslucentLevel(pTypeExt->SelectBox_TranslucentLevel.Get(RulesExt::Global()->SelectBox_TranslucentLevel.Get()));

	if (selectboxFrame.X == -1)
		selectboxFrame = glbSelectboxFrame;

	if (isInfantry)
	{
		vPos.X = pLocation->X + 1 + vOffset.X;
		vPos.Y = pLocation->Y + 1 + pThis->GetTechnoType()->PixelSelectionBracketDelta + vOffset.Y;
	}
	else
	{
		vPos.X = pLocation->X + 2 + vOffset.X;
		vPos.Y = pLocation->Y + 1 + pThis->GetTechnoType()->PixelSelectionBracketDelta + vOffset.Y;
	}

	SHPStruct* pShape = nullptr;

	if (isInfantry)
		pShape = pTypeExt->SelectBox_Shape.Get(RulesExt::Global()->SelectBox_Shape_Infantry);
	else
		pShape = pTypeExt->SelectBox_Shape.Get(RulesExt::Global()->SelectBox_Shape_Unit);

	if (pShape == nullptr)
		return;

	ConvertClass* pPalette = nullptr;

	if (isInfantry)
		pPalette = pTypeExt->SelectBox_Palette.GetOrDefaultConvert(RulesExt::Global()->SelectBox_Palette_Infantry.GetOrDefaultConvert(FileSystem::PALETTE_PAL));
	else
		pPalette = pTypeExt->SelectBox_Palette.GetOrDefaultConvert(RulesExt::Global()->SelectBox_Palette_Unit.GetOrDefaultConvert(FileSystem::PALETTE_PAL));

	if (pThis->IsSelected)
	{
		if (pThis->IsGreenHP())
			frame = selectboxFrame.X;
		else if (pThis->IsYellowHP())
			frame = selectboxFrame.Y;
		else
			frame = selectboxFrame.Z;
		DSurface::Temp->DrawSHP(pPalette, pShape, frame, &vPos, pBound, nFlag, 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
	}
}

void TechnoExt::DisplayDamageNumberString(TechnoClass* pThis, int damage, bool isShieldDamage)
{
	if (!pThis || damage == 0)
		return;

	const auto pExt = TechnoExt::ExtMap.Find(pThis);

	ColorStruct color = isShieldDamage ? damage > 0 ? ColorStruct { 0, 160, 255 } : ColorStruct { 0, 255, 230 } :
		damage > 0 ? ColorStruct { 255, 0, 0 } : ColorStruct { 0, 255, 0 };

	wchar_t damageStr[0x20];
	swprintf_s(damageStr, L"%d", damage);
	auto coords = CoordStruct::Empty;
	coords = *pThis->GetCenterCoord(&coords);

	int maxOffset = Unsorted::CellWidthInPixels / 2;
	int width = 0, height = 0;
	BitFont::Instance->GetTextDimension(damageStr, &width, &height, 120);

	if (pExt->DamageNumberOffset >= maxOffset || pExt->DamageNumberOffset < -maxOffset)
		pExt->DamageNumberOffset = -maxOffset;

	FlyingStrings::Add(damageStr, coords, color, Point2D { pExt->DamageNumberOffset - (width / 2), 0 });

	pExt->DamageNumberOffset = pExt->DamageNumberOffset + width;
}

void TechnoExt::InitializeHugeBar(TechnoClass* pThis)
{
	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (pTypeExt->HugeBar)
	{
		auto& mTechno = PhobosGlobal::Global()->Techno_HugeBar;
		auto it = mTechno.find(pTypeExt->HugeBar_Priority);

		while (it != mTechno.end() && it->first == pTypeExt->HugeBar_Priority)
		{
			if (it->second == pThis)
				return;

			++it;
		}

		mTechno.emplace(pTypeExt->HugeBar_Priority, pThis);
	}
}

void TechnoExt::RemoveHugeBar(TechnoClass* pThis)
{
	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (pTypeExt->HugeBar)
	{
		auto& mTechno = PhobosGlobal::Global()->Techno_HugeBar;
		auto it = mTechno.find(pTypeExt->HugeBar_Priority);

		while (it != mTechno.end() && it->first == pTypeExt->HugeBar_Priority)
		{
			if (it->second == pThis)
			{
				mTechno.erase(it);

				return;
			}

			++it;
		}
	}
}

void TechnoExt::ProcessHugeBar()
{
	if (PhobosGlobal::Global()->Techno_HugeBar.empty())
		return;

	TechnoClass* pTechno = PhobosGlobal::Global()->Techno_HugeBar.begin()->second;

	if (pTechno == nullptr)
		return;

	auto& configs = RulesExt::Global()->HugeBar_Config;

	for (size_t i = 0; i < configs.size(); i++)
	{
		int iCurrent = -1;
		int iMax = -1;
		GetValuesForDisplay(pTechno, configs[i]->InfoType, iCurrent, iMax);

		if (iCurrent != -1)
			DrawHugeBar(configs[i].get(), iCurrent, iMax);
	}
}

void TechnoExt::DrawHugeBar(RulesExt::ExtData::HugeBarData* pConfig, int iCurrent, int iMax)
{
	double ratio = static_cast<double>(iCurrent) / iMax;
	int iPipNumber = std::max(static_cast<int>(ratio * pConfig->HugeBar_Pips_Num), (iCurrent == 0 ? 0 : 1));
	Point2D posDraw = pConfig->HugeBar_Offset.Get() + pConfig->Anchor.OffsetPosition(DSurface::Composite->GetRect());
	Point2D posDrawValue = posDraw;
	RectangleStruct rBound = std::move(DSurface::Composite->GetRect());
	
	if (pConfig->HugeBar_Shape != nullptr
		&& pConfig->HugeBar_Pips_Shape != nullptr
		&& pConfig->HugeBar_Frame.Get(ratio) >= 0
		&& pConfig->HugeBar_Pips_Frame.Get(ratio) >= 0)
	{
		SHPStruct* pShp_Bar = pConfig->HugeBar_Shape;
		ConvertClass* pPal_Bar = pConfig->HugeBar_Palette.GetOrDefaultConvert(FileSystem::PALETTE_PAL);
		SHPStruct* pShp_Pips = pConfig->HugeBar_Pips_Shape;
		ConvertClass* pPal_Pips = pConfig->HugeBar_Pips_Palette.GetOrDefaultConvert(FileSystem::PALETTE_PAL);
		int iPipFrame = pConfig->HugeBar_Pips_Frame.Get(ratio);
		
		switch (pConfig->Anchor.Horizontal)
		{
		case HorizontalPosition::Left:
			posDrawValue.X += pShp_Bar->Width / 2;
			break;

		case HorizontalPosition::Center:
			posDrawValue.X = posDraw.X;
			posDraw.X -= pShp_Bar->Width / 2;
			break;

		case HorizontalPosition::Right:
			posDraw.X -= pShp_Bar->Width;
			posDrawValue.X -= pShp_Bar->Width / 2;
			break;

		default:
			break;
		}

		switch (pConfig->Anchor.Vertical)
		{
		case VerticalPosition::Top:
			posDrawValue.Y += pShp_Bar->Height;
			break;

		case VerticalPosition::Center:
			posDraw.Y -= pShp_Bar->Height / 2;
			posDrawValue.Y += pShp_Bar->Height;
			break;

		case VerticalPosition::Bottom:
			posDraw.Y -= pShp_Bar->Height;
			break;

		default:
			break;
		}

		DSurface::Composite->DrawSHP
		(
			pPal_Bar,
			pShp_Bar,
			pConfig->HugeBar_Frame.Get(ratio),
			&posDraw,
			&rBound,
			BlitterFlags::None,
			0,
			0,
			ZGradient::Ground,
			1000,
			0,
			nullptr,
			0,
			0,
			0
		);

		posDraw += pConfig->HugeBar_Pips_Offset.Get();

		for (int i = 0; i < iPipNumber; i++)
		{
			DSurface::Composite->DrawSHP
			(
				pPal_Pips,
				pShp_Pips,
				iPipFrame,
				&posDraw,
				&rBound,
				BlitterFlags::None,
				0,
				0,
				ZGradient::Ground,
				1000,
				0,
				nullptr,
				0,
				0,
				0
			);

			posDraw.X += pConfig->HugeBar_Pips_Interval;
		}
	}
	else
	{
		COLORREF color1 = Drawing::RGB2DWORD(pConfig->HugeBar_Pips_Color1.Get(ratio));
		COLORREF color2 = Drawing::RGB2DWORD(pConfig->HugeBar_Pips_Color2.Get(ratio));
		Vector2D<int> rectWH = pConfig->HugeBar_RectWH;

		if (rectWH.X < 0)
		{
			rectWH.X = static_cast<int>(pConfig->HugeBar_RectWidthPercentage * rBound.Width);
			// make sure width is even
			rectWH.X += rectWH.X & 1;
		}

		switch (pConfig->Anchor.Horizontal)
		{
		case HorizontalPosition::Left:
			posDrawValue.X += rectWH.X / 2;
			break;

		case HorizontalPosition::Center:
			posDrawValue.X = posDraw.X;
			posDraw.X -= rectWH.X / 2;
			break;

		case HorizontalPosition::Right:
			posDraw.X -= rectWH.X;
			posDrawValue.X -= rectWH.X / 2;
			break;

		default:
			break;
		}

		switch (pConfig->Anchor.Vertical)
		{
		case VerticalPosition::Top:
			posDrawValue.Y += rectWH.Y;
			break;

		case VerticalPosition::Center:
			posDraw.Y -= rectWH.Y / 2;
			posDrawValue.Y += rectWH.Y;
			break;

		case VerticalPosition::Bottom:
			posDraw.Y -= rectWH.Y;
			break;

		default:
			break;
		}

		RectangleStruct rect = { posDraw.X, posDraw.Y, rectWH.X, rectWH.Y };
		DSurface::Composite->DrawRect(&rect, color2);
		int iPipWidth = 0;
		int iPipHeight = 0;
		int iPipTotal = pConfig->HugeBar_Pips_Num;

		if (pConfig->HugeBar_Pips_Offset.isset())
		{
			Point2D offset = pConfig->HugeBar_Pips_Offset.Get();
			posDraw += offset;
			//center
			iPipWidth = (rectWH.X - offset.X * 2) / iPipTotal;
			iPipHeight = rectWH.Y - offset.Y * 2;
		}
		else
		{
			// default has 5 interval between border and pips at least
			const int iInterval = 5;
			iPipWidth = (rectWH.X - iInterval * 2) / iPipTotal;
			iPipHeight = rectWH.Y - iInterval * 2;
			posDraw.X += (rectWH.X - iPipTotal * iPipWidth) / 2;
			posDraw.Y += (rectWH.Y - iPipHeight) / 2;
		}

		if (iPipWidth <= 0 || iPipHeight <= 0)
			return;

		// Color1 75% Color2 25%, simulated healthbar
		int iPipColor1Width = std::max(static_cast<int>(iPipWidth * 0.75), std::min(3, iPipWidth));
		int iPipColor2Width = iPipWidth - iPipColor1Width;
		rect = { posDraw.X, posDraw.Y, iPipColor1Width , iPipHeight };

		for (int i = 0; i < iPipNumber; i++)
		{
			DSurface::Composite->FillRect(&rect, color1);
			rect.X += iPipColor1Width;
			rect.Width = iPipColor2Width;
			DSurface::Composite->FillRect(&rect, color2);
			rect.X += iPipColor2Width;
			rect.Width = iPipColor1Width;
		}
	}

	HugeBar_DrawValue(pConfig, posDrawValue, iCurrent, iMax);
}

void TechnoExt::HugeBar_DrawValue(RulesExt::ExtData::HugeBarData* pConfig, Point2D& posDraw, int iCurrent, int iMax)
{
	RectangleStruct rBound = std::move(DSurface::Composite->GetRect());
	double ratio = static_cast<double>(iCurrent) / iMax;
	posDraw += pConfig->Value_Offset;

	if (pConfig->Value_Shape != nullptr)
	{
		SHPStruct* pShp = pConfig->Value_Shape;
		ConvertClass* pPal = pConfig->Value_Palette.GetOrDefaultConvert(FileSystem::PALETTE_PAL);

		if (pConfig->Anchor.Vertical == VerticalPosition::Bottom)
			posDraw.Y -= pShp->Height * (static_cast<int>(pConfig->InfoType) + 1);
		else
			posDraw.Y += pShp->Height * static_cast<int>(pConfig->InfoType);

		std::string text;

		if (pConfig->Value_Percentage)
			text = GeneralUtils::IntToDigits(static_cast<int>(ratio * 100));
		else
			text = GeneralUtils::IntToDigits(iCurrent) + "/" + GeneralUtils::IntToDigits(iMax);

		int iNumBaseFrame = pConfig->Value_Num_BaseFrame;
		int iSignBaseFrame = pConfig->Value_Sign_BaseFrame;

		if (ratio <= RulesClass::Instance->ConditionYellow)
		{
			// number 0-9
			iNumBaseFrame += 10;
			// sign /%
			iSignBaseFrame += 2;
		}

		if (ratio <= RulesClass::Instance->ConditionRed)
		{
			iNumBaseFrame += 10;
			iSignBaseFrame += 2;
		}

		posDraw.X -= text.length() * pConfig->Value_Shape_Interval / 2;

		ShapeTextPrintData printData
		(
			pShp,
			pPal,
			iNumBaseFrame,
			iSignBaseFrame,
			Point2D({ pConfig->Value_Shape_Interval, 0 })
		);
		ShapeTextPrinter::PrintShape(text.c_str(), printData, posDraw, rBound, DSurface::Composite);
	}
	else
	{
		const int iTextHeight = 15;

		if (pConfig->Anchor.Vertical == VerticalPosition::Bottom)
			posDraw.Y -= iTextHeight * (static_cast<int>(pConfig->InfoType) + 1);
		else
			posDraw.Y += iTextHeight * static_cast<int>(pConfig->InfoType);

		wchar_t text[0x16] = L"";

		if (pConfig->Value_Percentage)
		{
			swprintf_s(text, L"%d", static_cast<int>(ratio * 100));
			wcscat_s(text, L"%%");
		}
		else
		{
			swprintf_s(text, L"%d/%d", iCurrent, iMax);
		}

		COLORREF color = Drawing::RGB2DWORD(pConfig->Value_Text_Color.Get(ratio));
		DSurface::Composite->DrawTextA(text, &rBound, &posDraw, color, COLOR_BLACK, TextPrintType::Center);
	}
}

void TechnoExt::RunBlinkWeapon(TechnoClass* pThis, AbstractClass* pTarget, WeaponTypeClass* pWeapon)
{
	TechnoClass* pTargetTechno = abstract_cast<TechnoClass*>(pTarget);

	if (pTargetTechno == nullptr)
		return;

	auto pType = pThis->GetTechnoType();
	auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);
	CoordStruct PreSelfLocation = pThis->Location;
	CoordStruct PreTargetLocation = pTargetTechno->GetCoords();

	if (pWeaponExt->BlinkWeapon.Get() && pThis->WhatAmI() != AbstractType::Building)
	{
		for (auto it : pWeaponExt->BlinkWeapon_SelfAnim)
		{
			if (it != nullptr)
				GameCreate<AnimClass>(it, PreSelfLocation);
		}
		for (auto it : pWeaponExt->BlinkWeapon_TargetAnim)
		{
			if (it != nullptr)
				GameCreate<AnimClass>(it, PreTargetLocation);
		}

		CoordStruct location;
		CellClass* pCell = nullptr;
		CellStruct nCell;
		int iHeight = pTargetTechno->GetHeight();
		if (pWeaponExt->BlinkWeapon_Overlap.Get())
		{
			nCell = CellClass::Coord2Cell(PreTargetLocation);
			pCell = MapClass::Instance->TryGetCellAt(nCell);
			location = PreTargetLocation;
		}
		else
		{
			bool allowBridges = pType->SpeedType != SpeedType::Float;
			nCell = MapClass::Instance->NearByLocation(CellClass::Coord2Cell(PreTargetLocation),
				pType->SpeedType, -1, pType->MovementZone, false, 1, 1, true,
				false, false, allowBridges, CellStruct::Empty, false, false);
			pCell = MapClass::Instance->TryGetCellAt(nCell);
			location = PreTargetLocation;

		}

		if (pCell != nullptr)
			location = pCell->GetCoordsWithBridge();
		else
			location.Z = MapClass::Instance->GetCellFloorHeight(location);

		location.Z += iHeight;
		CoordStruct Src = pThis->GetCoords();
		FootClass* pFoot = abstract_cast<FootClass*>(pThis);
		CellStruct cell = CellClass::Coord2Cell(Src);
		pFoot->UnmarkAllOccupationBits(Src);
		pFoot->UnmarkAllOccupationBits(location);
		MapClass::Instance()->RemoveContentAt(&cell, pFoot);
		pFoot->Locomotor->Force_Track(-1, location);
		pFoot->Locomotor->Mark_All_Occupation_Bits(0);
		if (pFoot->WhatAmI() == AbstractType::Infantry)
			pFoot->Locomotor->Stop_Movement_Animation();
		pFoot->Locomotor->Lock();
		pFoot->SetLocation(location);
		CellStruct targetcell = CellClass::Coord2Cell(location);
		pFoot->MarkAllOccupationBits(location);
		//pFoot->Locomotor->Clear_Coords();
		pFoot->Locomotor->Force_Track(-1, location);
		pFoot->MarkAllOccupationBits(location);
		pFoot->Locomotor->Unlock();
		//pThis->ForceMission(Mission::Stop);
		//pThis->Guard();

		if (pWeaponExt->BlinkWeapon_KillTarget.Get())
			pTargetTechno->ReceiveDamage(&pTargetTechno->Health, 0, pWeapon->Warhead, pThis, true, false, pThis->GetOwningHouse());

	}
}

void TechnoExt::ReceiveDamageAnim(TechnoClass* pThis, int damage)
{
	if (!pThis || damage == 0)
		return;

	auto pTypeThis = pThis->GetTechnoType();
	auto pTypeData = TechnoTypeExt::ExtMap.Find(pTypeThis);
	auto pData = TechnoExt::ExtMap.Find(pThis);

	GScreenAnimTypeClass* pReceiveDamageAnimType = nullptr;

	pReceiveDamageAnimType = pTypeData->GScreenAnimType.Get();

	if (pTypeThis && pTypeData && pData && pReceiveDamageAnimType)
	{
		// 设置冷却时间防止频繁触发而明显掉帧
		// 初始化激活时的游戏帧
		if (pData->ShowAnim_LastActivatedFrame < 0)
			pData->ShowAnim_LastActivatedFrame = -pReceiveDamageAnimType->ShowAnim_CoolDown;
		// 若本次受伤害的游戏帧未达到指定值，拒绝Add
		if (Unsorted::CurrentFrame < pData->ShowAnim_LastActivatedFrame + pReceiveDamageAnimType->ShowAnim_CoolDown)
			return;

		SHPStruct* ShowAnimSHP = pReceiveDamageAnimType->SHP_ShowAnim;
		ConvertClass* ShowAnimPAL = pReceiveDamageAnimType->PAL_ShowAnim;

		if (ShowAnimSHP == nullptr)
		{
			Debug::Log("[ReceiveDamageAnim::Error] SHP file not found\n");
			return;
		}
		if (ShowAnimPAL == nullptr)
		{
			Debug::Log("[ReceiveDamageAnim::Error] PAL file not found\n");
			return;
		}

		// 左上角坐标，默认将SHP文件放置到屏幕中央
		Point2D posAnim = {
			DSurface::Composite->GetWidth() / 2 - ShowAnimSHP->Width / 2,
			DSurface::Composite->GetHeight() / 2 - ShowAnimSHP->Height / 2
		};
		posAnim += pReceiveDamageAnimType->ShowAnim_Offset.Get();

		// 透明度
		int translucentLevel = pReceiveDamageAnimType->ShowAnim_TranslucentLevel.Get();

		// 每帧shp文件实际重复播放几帧
		int frameKeep = pReceiveDamageAnimType->ShowAnim_FrameKeep;

		// shp文件循环次数
		int loopCount = pReceiveDamageAnimType->ShowAnim_LoopCount;

		// 信息加入vector
		GScreenDisplay::Add(ShowAnimPAL, ShowAnimSHP, posAnim, translucentLevel, frameKeep, loopCount);
		// 激活则立即记录激活时的游戏帧
		pData->ShowAnim_LastActivatedFrame = Unsorted::CurrentFrame;
	}

}

Point2D TechnoExt::GetScreenLocation(TechnoClass* pThis)
{
	CoordStruct crdAbsolute = pThis->GetCoords();
	Point2D  posScreen = { 0,0 };
	TacticalClass::Instance->CoordsToScreen(&posScreen, &crdAbsolute);
	posScreen -= TacticalClass::Instance->TacticalPos;

	return posScreen;
}

Point2D TechnoExt::GetFootSelectBracketPosition(TechnoClass* pThis, Anchor anchor)
{
	int iLength = 17;
	Point2D posScreen = GetScreenLocation(pThis);

	if (pThis->WhatAmI() == AbstractType::Infantry)
		iLength = 8;

	RectangleStruct rBracket =
	{
		posScreen.X - iLength + (iLength == 8),
		posScreen.Y - 28 + (iLength == 8),
		iLength * 2,
		iLength * 3
	};
	Point2D posRes = anchor.OffsetPosition(rBracket);

	return posRes;
}

Point2D TechnoExt::GetBuildingSelectBracketPosition(TechnoClass* pThis, BuildingSelectBracketPosition ePos)
{
	BuildingTypeClass* pBuildingType = static_cast<BuildingTypeClass*>(pThis->GetTechnoType());
	Point2D posRes = GetScreenLocation(pThis);
	CoordStruct crdDim2 = CoordStruct::Empty;
	pBuildingType->Dimension2(&crdDim2);
	Point2D posFix = Point2D::Empty;
	CoordStruct crdTmp = { -crdDim2.X / 2, crdDim2.Y / 2, crdDim2.Z };
	TacticalClass::Instance->CoordsToScreen(&posFix, &crdTmp);
	int iFoundationHeight = pBuildingType->GetFoundationHeight(false);
	int iFoundationWidth = pBuildingType->GetFoundationWidth();
	int iHeight = pBuildingType->Height * 12;
	int iLengthH = iFoundationHeight * 7 + iFoundationHeight / 2;
	int iLengthW = iFoundationWidth * 7 + iFoundationWidth / 2;
	posRes.X += posFix.X + 3 + iLengthH * 4;
	posRes.Y += posFix.Y + 4 - iLengthH * 2;

	switch (ePos)
	{
	case BuildingSelectBracketPosition::Top:
		break;
	case BuildingSelectBracketPosition::LeftTop:
		posRes.X -= iLengthH * 4;
		posRes.Y += iLengthH * 2;
		break;
	case BuildingSelectBracketPosition::LeftBottom:
		posRes.X -= iLengthH * 4;
		posRes.Y += iLengthH * 2 + iHeight;
		break;
	case BuildingSelectBracketPosition::Bottom:
		posRes.Y += iLengthW * 2 + iLengthH * 2 + iHeight;
		break;
	case BuildingSelectBracketPosition::RightBottom:
		posRes.X += iLengthW * 4;
		posRes.Y += iLengthW * 2 + iHeight;
		break;
	case BuildingSelectBracketPosition::RightTop:
		posRes.X += iLengthW * 4;
		posRes.Y += iLengthW * 2;
	default:
		break;
	}

	return posRes;
}

void TechnoExt::ProcessDigitalDisplays(TechnoClass* pThis)
{
	if (!Phobos::Config::DigitalDisplay_Enable)
		return;

	TechnoTypeClass* pType = pThis->GetTechnoType();
	TechnoTypeExt::ExtData* pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

	if (pTypeExt->DigitalDisplay_Disable)
		return;

	TechnoExt::ExtData* pExt = TechnoExt::ExtMap.Find(pThis);
	int iLength = 17;
	ValueableVector<DigitalDisplayTypeClass*>* pDisplayTypes = nullptr;

	if (!pTypeExt->DigitalDisplayTypes.empty())
	{
		pDisplayTypes = &pTypeExt->DigitalDisplayTypes;
	}
	else
	{
		switch (pThis->WhatAmI())
		{
		case AbstractType::Building:
		{
			pDisplayTypes = &RulesExt::Global()->Buildings_DefaultDigitalDisplayTypes;
			BuildingTypeClass* pBuildingType = static_cast<BuildingTypeClass*>(pThis->GetTechnoType());
			int iFoundationHeight = pBuildingType->GetFoundationHeight(false);
			iLength = iFoundationHeight * 7 + iFoundationHeight / 2;
		}break;
		case AbstractType::Infantry:
		{
			pDisplayTypes = &RulesExt::Global()->Infantry_DefaultDigitalDisplayTypes;
			iLength = 8;
		}break;
		case AbstractType::Unit:
		{
			pDisplayTypes = &RulesExt::Global()->Vehicles_DefaultDigitalDisplayTypes;
		}break;
		case AbstractType::Aircraft:
		{
			pDisplayTypes = &RulesExt::Global()->Aircraft_DefaultDigitalDisplayTypes;
		}break;
		default:
			return;
		}
	}

	for (DigitalDisplayTypeClass*& pDisplayType : *pDisplayTypes)
	{
		if (HouseClass::IsPlayerObserver() && !pDisplayType->CanSee_Observer)
			continue;

		if (!HouseClass::IsPlayerObserver() && !EnumFunctions::CanTargetHouse(pDisplayType->CanSee, pThis->Owner, HouseClass::Player))
			continue;

		int iCur = -1;
		int iMax = -1;

		GetValuesForDisplay(pThis, pDisplayType->InfoType.Get(), iCur, iMax);

		if (iCur == -1 || iMax == -1)
			continue;

		bool isBuilding = pThis->WhatAmI() == AbstractType::Building;
		bool hasShield = pExt->Shield != nullptr && !pExt->Shield->IsBrokenAndNonRespawning();
		Point2D posDraw = pThis->WhatAmI() == AbstractType::Building ?
			GetBuildingSelectBracketPosition(pThis, pDisplayType->AnchorType_Building)
			: GetFootSelectBracketPosition(pThis, pDisplayType->AnchorType);
		posDraw.Y += pType->PixelSelectionBracketDelta;

		if (pDisplayType->InfoType == DisplayInfoType::Shield)
			posDraw.Y += pExt->Shield->GetType()->BracketDelta;

		pDisplayType->Draw(posDraw, iLength, iCur, iMax, isBuilding, hasShield);
	}
}

void TechnoExt::GetValuesForDisplay(TechnoClass* pThis, DisplayInfoType infoType, int& iCur, int& iMax)
{
	TechnoTypeClass* pType = pThis->GetTechnoType();
	TechnoExt::ExtData* pExt = TechnoExt::ExtMap.Find(pThis);
	TechnoTypeExt::ExtData* pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

	switch (infoType)
	{
	case DisplayInfoType::Health:
	{
		iCur = pThis->Health;
		iMax = pType->Strength;
		break;
	}
	case DisplayInfoType::Shield:
	{
		if (pExt->Shield == nullptr || pExt->Shield->IsBrokenAndNonRespawning())
			return;
		iCur = pExt->Shield->GetHP();
		iMax = pExt->Shield->GetType()->Strength.Get();
		break;
	}
	case DisplayInfoType::Ammo:
	{
		if (pType->Ammo <= 0)
			return;
		iCur = pThis->Ammo;
		iMax = pType->Ammo;
		break;
	}
	case DisplayInfoType::MindControl:
	{
		if (pThis->CaptureManager == nullptr)
			return;
		iCur = pThis->CaptureManager->ControlNodes.Count;
		iMax = pThis->CaptureManager->MaxControlNodes;
		break;
	}
	case DisplayInfoType::Spawns:
	{
		if (pThis->SpawnManager == nullptr || pType->Spawns == nullptr || pType->SpawnsNumber <= 0)
			return;
		iCur = pThis->SpawnManager->CountAliveSpawns();
		iMax = pThis->SpawnManager->SpawnedNodes.Count;
		break;
	}
	case DisplayInfoType::Passengers:
	{
		if (pType->Passengers <= 0)
			return;
		iCur = pThis->Passengers.NumPassengers;
		iMax = pType->Passengers;
		break;
	}
	case DisplayInfoType::Tiberium:
	{
		if (pType->Storage <= 0)
			return;
		iCur = static_cast<int>(pThis->Tiberium.GetTotalAmount());
		iMax = pType->Storage;
		break;
	}
	case DisplayInfoType::Experience:
	{
		iCur = static_cast<int>(pThis->Veterancy.Veterancy * RulesClass::Instance->VeteranRatio * pType->GetCost());
		iMax = static_cast<int>(2.0 * RulesClass::Instance->VeteranRatio * pType->GetCost());
		break;
	}
	case DisplayInfoType::Occupants:
	{
		if (pThis->WhatAmI() != AbstractType::Building)
			return;
		BuildingTypeClass* pBuildingType = abstract_cast<BuildingTypeClass*>(pType);
		BuildingClass* pBuilding = abstract_cast<BuildingClass*>(pThis);
		if (!pBuildingType->CanBeOccupied)
			return;
		iCur = pBuilding->Occupants.Count;
		iMax = pBuildingType->MaxNumberOccupants;
		break;
	}
	case DisplayInfoType::GattlingStage:
	{
		if (!pType->IsGattling)
			return;
		iCur = pThis->CurrentGattlingStage;
		iMax = pType->WeaponStages;
		break;
	}
	case DisplayInfoType::Temperature:
	{
		iCur = pExt->Temperature[0];
		iMax = pTypeExt->Temperature[0];
		break;
	}
	default:
	{
		iCur = pThis->Health;
		iMax = pType->Strength;
		break;
	}
	}
}

void TechnoExt::InitializeBuild(TechnoClass* pThis, TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt)
{
	//	auto const pExt = TechnoExt::ExtMap.Find(pThis);

	if (pExt->Build_As_OnlyOne)
		return;

	//	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	for (unsigned int i = 0; i < pTypeExt->BuildLimit_As.size(); i++)
	{
		pExt->Build_As.push_back(pTypeExt->BuildLimit_As[i]);

		auto pType = pExt->Build_As[i];

		auto pTechno = static_cast<TechnoClass*>(pType->CreateObject(pThis->Owner));
		pTechno->Unlimbo(pThis->GetCoords(), pThis->PrimaryFacing.current().value256());
		pTechno->Limbo();

		if (pType->WhatAmI() == AbstractType::InfantryType)
		{
			auto pInf = abstract_cast<InfantryTypeClass*>(pType);
			pThis->Owner->OwnedInfantryTypes.Increment(pInf->GetArrayIndex());
		}
		else if (pType->WhatAmI() == AbstractType::UnitType)
		{
			auto punit = abstract_cast<UnitTypeClass*>(pType);
			pThis->Owner->OwnedUnitTypes.Increment(punit->GetArrayIndex());
		}
		else if (pType->WhatAmI() == AbstractType::AircraftType)
		{
			auto pair = abstract_cast<AircraftTypeClass*>(pType);
			pThis->Owner->OwnedAircraftTypes.Increment(pair->GetArrayIndex());
		}
		else
		{
			auto pbuilding = abstract_cast<BuildingTypeClass*>(pType);
			pThis->Owner->OwnedBuildingTypes.Increment(pbuilding->GetArrayIndex());
		}

		Debug::Log("Test %d : %s.\n", i, pExt->Build_As[i]->get_ID());

		pTechno->UnInit();
	}

	pExt->Build_As_OnlyOne = true;
}

void TechnoExt::DeleteTheBuild(TechnoClass* pThis)
{
	auto pExt = TechnoExt::ExtMap.Find(pThis);

	if (!pExt->Build_As_OnlyOne)
		return;

	for (unsigned int i = 0; i < pExt->Build_As.size(); i++)
	{
		auto pType = pExt->Build_As[i];

		if (pType->WhatAmI() == AbstractType::InfantryType)
		{
			auto pInf = abstract_cast<InfantryTypeClass*>(pType);
			pThis->Owner->OwnedInfantryTypes.Decrement(pInf->GetArrayIndex());
		}
		else if (pType->WhatAmI() == AbstractType::UnitType)
		{
			auto punit = abstract_cast<UnitTypeClass*>(pType);
			pThis->Owner->OwnedUnitTypes.Decrement(punit->GetArrayIndex());
		}
		else if (pType->WhatAmI() == AbstractType::AircraftType)
		{
			auto pair = abstract_cast<AircraftTypeClass*>(pType);
			pThis->Owner->OwnedAircraftTypes.Decrement(pair->GetArrayIndex());
		}
		else
		{
			auto pbuilding = abstract_cast<BuildingTypeClass*>(pType);
			pThis->Owner->OwnedBuildingTypes.Decrement(pbuilding->GetArrayIndex());
		}

		Debug::Log("%s Test %d : %s.\n", pThis->get_ID(), (i + pExt->Build_As.size()), pExt->Build_As[i]->get_ID());
	}
}

void TechnoExt::ExtData::UpdateAttackedWeaponTimer()
{
	for (int& iTime : AttackedWeapon_Timer)
	{
		if (iTime > 0)
			iTime--;
	}
}

void TechnoExt::ProcessAttackedWeapon(TechnoClass* pThis, args_ReceiveDamage* args, bool bBeforeDamageCheck)
{
	if (pThis->Health <= 0 || pThis == args->Attacker)
		return;

	TechnoTypeClass* pType = pThis->GetTechnoType();
	TechnoTypeExt::ExtData* pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
	TechnoExt::ExtData* pExt = TechnoExt::ExtMap.Find(pThis);

	ValueableVector<WeaponTypeClass*>* pWeapons = &pTypeExt->AttackedWeapon;

	if (pThis->Veterancy.IsVeteran() && !pTypeExt->AttackedWeapon_Veteran.empty())
		pWeapons = &pTypeExt->AttackedWeapon_Veteran;

	if (pThis->Veterancy.IsElite() && !pTypeExt->AttackedWeapon_Elite.empty())
		pWeapons = &pTypeExt->AttackedWeapon_Elite;

	if (pWeapons->empty())
		return;

	WarheadTypeClass* pWH = args->WH;
	WarheadTypeExt::ExtData* pWHExt = WarheadTypeExt::ExtMap.Find(pWH);
	HouseClass* pOwner = pThis->GetOwningHouse();
	HouseClass* pAttacker = args->SourceHouse;

	if (pWHExt->AttackedWeapon_ForceNoResponse.Get()
		|| !pWHExt->AttackedWeapon_ResponseTechno.empty() && !pWHExt->AttackedWeapon_ResponseTechno.Contains(pType)
		|| !pWHExt->AttackedWeapon_NoResponseTechno.empty() && pWHExt->AttackedWeapon_NoResponseTechno.Contains(pType))
		return;

	//Debug::Log("[AttackedWeapon] Warhead Pass\n");

	if (!pTypeExt->AttackedWeapon_ResponseWarhead.empty() && !pTypeExt->AttackedWeapon_ResponseWarhead.Contains(pWH)
		|| !pTypeExt->AttackedWeapon_NoResponseWarhead.empty() && pTypeExt->AttackedWeapon_NoResponseWarhead.Contains(pWH))
		return;

	//Debug::Log("[AttackedWeapon] Techno Pass\n");

	ValueableVector<int>& vROF = pTypeExt->AttackedWeapon_ROF;
	ValueableVector<bool>& vFireToAttacker = pTypeExt->AttackedWeapon_FireToAttacker;
	ValueableVector<bool>& vIgnoreROF = pTypeExt->AttackedWeapon_IgnoreROF;
	ValueableVector<bool>& vIgnoreRange = pTypeExt->AttackedWeapon_IgnoreRange;
	ValueableVector<int>& vRange = pTypeExt->AttackedWeapon_Range;
	ValueableVector<bool>& vReponseZeroDamage = pTypeExt->AttackedWeapon_ResponseZeroDamage;
	std::vector<AffectedHouse>& vAffectHouse = pTypeExt->AttackedWeapon_ResponseHouse;
	ValueableVector<int>& vMaxHP = pTypeExt->AttackedWeapon_ActiveMaxHealth;
	ValueableVector<int>& vMinHP = pTypeExt->AttackedWeapon_ActiveMinHealth;
	std::vector<CoordStruct>& vFLH = pTypeExt->AttackedWeapon_FLHs;
	std::vector<int>& vTimer = pExt->AttackedWeapon_Timer;

	while (vTimer.size() < pWeapons->size())
		vTimer.emplace_back(0);

	while (vTimer.size() > pWeapons->size())
		vTimer.pop_back();

	for (size_t i = 0; i < pWeapons->size(); i++)
	{
		WeaponTypeClass* pWeapon = pWeapons->at(i);
		int iMaxHP = i < vMaxHP.size() ? vMaxHP[i] : INT_MAX;
		int iMinHP = i < vMinHP.size() ? vMinHP[i] : 0;
		int iROF = i < vROF.size() ? vROF[i] : pWeapon->ROF;
		int& iTime = vTimer[i];
		bool bIgnoreROF = i < vIgnoreROF.size() ? vIgnoreROF[i] : false;
		bool bIsInROF = bIgnoreROF ? false : iTime != 0;
		bool bResponseZeroDamage = i < vReponseZeroDamage.size() ? vReponseZeroDamage[i] : false;

		if (iMaxHP < iMinHP)
			Debug::Log("[AttackedWeapon::Warning] TechnoType[%s] attacked weapon index[%u](start from 0) ActiveMaxHealth[%d] less than ActiveMinHealth[%d] !\n",
				pType->get_ID(), i, iMaxHP, iMinHP);

		if (pWeapon == nullptr
			|| bIsInROF
			|| bResponseZeroDamage && !bBeforeDamageCheck
			|| !bResponseZeroDamage && (bBeforeDamageCheck || *args->Damage == 0)
			|| pThis->Health < iMinHP
			|| pThis->Health > iMaxHP)
			continue;

		bool bFireToAttacker = i < vFireToAttacker.size() ? vFireToAttacker[i] : false;
		bool bIgnoreRange = i < vIgnoreRange.size() ? vIgnoreRange[i] : false;
		AffectedHouse affectedHouse = vAffectHouse[i];
		int iRange = i < vRange.size() ? vRange[i] : pWeapon->Range;
		CoordStruct crdFLH = vFLH[i];

		if (!EnumFunctions::CanTargetHouse(affectedHouse, pOwner, pAttacker))
			continue;

		if (bFireToAttacker)
		{
			if (args->Attacker != nullptr)
			{
				if (bIgnoreRange || iRange >= pThis->DistanceFrom(args->Attacker))
				{
					if (!bIgnoreROF)
						iTime = iROF;

					WeaponStruct weaponStruct;
					weaponStruct.WeaponType = pWeapon;
					weaponStruct.FLH = crdFLH;
					TechnoExt::SimulatedFire(pThis, weaponStruct, args->Attacker);
				}
			}
		}
		else
		{
			if (!bIgnoreROF)
				iTime = iROF;
			else
				iTime = 1;

			WeaponTypeExt::DetonateAt(pWeapon, pThis->GetCoords(), pThis);
		}
	}
}

void TechnoExt::PassengerFixed(TechnoClass* pThis)
{
	if (pThis->WhatAmI() != AbstractType::Unit && pThis->WhatAmI() != AbstractType::Aircraft)
		return;

	if (pThis->Passengers.NumPassengers > 0)
	{
		auto pPassenger = pThis->Passengers.GetFirstPassenger();

		for (int i = pThis->Passengers.NumPassengers; i > 0; i--)
		{
			const auto pTechno = abstract_cast<TechnoClass*>(pPassenger);

			if (pThis->GetTechnoType()->OpenTopped)
			{
				pThis->EnteredOpenTopped(pTechno);
			}

			if (pThis->GetTechnoType()->Gunner)
			{
				abstract_cast<FootClass*>(pThis)->ReceiveGunner(pPassenger);
			}

			pPassenger->Transporter = pThis;
			pPassenger = abstract_cast<FootClass*>(pPassenger->NextObject);
		}
	}
}

void TechnoExt::InitialPayloadFixed(TechnoClass* pThis, TechnoTypeExt::ExtData* pTypeExt)
{
	if (pThis->WhatAmI() != AbstractType::Unit && pThis->WhatAmI() != AbstractType::Aircraft)
		return;

	if (pThis->Passengers.NumPassengers > 0)
		return;

	NullableVector<TechnoTypeClass*> InitialPayload_Types;
	NullableVector<int> InitialPayload_Nums;

	if (!pTypeExt->InitialPayload_Types.empty())
	{
		for (size_t i = 0; i < pTypeExt->InitialPayload_Types.size(); i++)
		{
			InitialPayload_Types.push_back(pTypeExt->InitialPayload_Types[i]);

			if (pTypeExt->InitialPayload_Nums[i] > 0)
				InitialPayload_Nums.push_back(pTypeExt->InitialPayload_Nums[i]);
			else
				InitialPayload_Nums.push_back(1);
		}

		for (size_t j = 0; j < pTypeExt->InitialPayload_Types.size(); j++)
		{
			for (int k = 0; k < InitialPayload_Nums[j]; k++)
			{
				TechnoTypeClass* pType = InitialPayload_Types[j];

				if (!pType)
					continue;

				FootClass* pFoot = abstract_cast<FootClass*>(pType->CreateObject(pThis->Owner));

				if (pFoot == nullptr)
					continue;

				pFoot->SetLocation(pThis->GetCoords());
				pFoot->Limbo();

				pFoot->Transporter = pThis;

				const auto old = VocClass::VoicesEnabled ? true : false;
				VocClass::VoicesEnabled = false;
				pThis->AddPassenger(pFoot);
				VocClass::VoicesEnabled = old;
			}
		}
	}
}

BulletClass* TechnoExt::SimulatedFire(TechnoClass* pThis, const WeaponStruct& weaponStruct, AbstractClass* pTarget)
{
	TechnoClass* pStand = PhobosGlobal::Global()->GetGenericStand();
	WeaponTypeClass* pWeapon = weaponStruct.WeaponType;

	if (pWeapon == nullptr)
		return nullptr;

	WarheadTypeClass* pWH = pWeapon->Warhead;

	if (pWH->MindControl || pWH->Temporal || pWH->Parasite || pWeapon->DrainWeapon || pWeapon->Spawner)
		return nullptr;

	ExtData* pExt = TechnoExt::ExtMap.Find(pThis);
	double dblFirePowerMultiplier = pThis->FirepowerMultiplier;
	int iDamageBuff = 0;

	for (auto& pAE : pExt->AttachEffects)
	{
		dblFirePowerMultiplier *= pAE->Type->FirePower_Multiplier;
		iDamageBuff += pAE->Type->FirePower;
	}

	HouseClass* pStandOriginOwner = pStand->Owner;
	pStand->Owner = pThis->Owner;
	TechnoTypeClass* pType = pStand->GetTechnoType();
	WeaponStruct& weaponCur = pType->GetWeapon(0, pStand->Veterancy.IsElite());
	WeaponStruct weaponOrigin = pType->GetWeapon(0, pStand->Veterancy.IsElite());
	bool bOmniFire = pWeapon->OmniFire;
	int iDamageOrigin = pWeapon->Damage;
	pWeapon->Damage = std::max(static_cast<int>(pWeapon->Damage * dblFirePowerMultiplier) + iDamageBuff, 0);
	pWeapon->OmniFire = true;
	weaponCur.WeaponType = weaponStruct.WeaponType;
	weaponCur.FLH = CoordStruct::Empty;
	CoordStruct absFLH = GetFLHAbsoluteCoords(pThis, weaponStruct.FLH, true);
	pStand->SetLocation(absFLH);
	BulletClass* pBullet = pStand->TechnoClass::Fire(pTarget, 0);

	if (pBullet != nullptr)
		pBullet->Owner = pThis;

	weaponCur = weaponOrigin;
	pWeapon->Damage = iDamageOrigin;
	pWeapon->OmniFire = bOmniFire;
	pStand->Owner = pStandOriginOwner;

	return pBullet;
}

void TechnoExt::FixManagers(TechnoClass* pThis)
{
	TechnoTypeClass* pType = pThis->GetTechnoType();
	std::vector<WeaponTypeClass*> vWeapons(std::move(TechnoTypeExt::GetAllWeapons(pType)));
	bool bCaptureSet = false;
	bool bSpawnSet = false;
	bool hasCapture = false;
	bool hasTemporal = false;
	bool hasSpawn = false;
	bool hasParasite = false;
	FootClass* pFoot = abstract_cast<FootClass*>(pThis);

	for (size_t i = 0; i < vWeapons.size(); i++)
	{
		WeaponTypeClass* pWeapon = vWeapons[i];

		if (pWeapon->Warhead->MindControl)
		{
			hasCapture = true;

			if (!bCaptureSet)
			{
				std::vector<ControlNode*> vCaptured;

				if (pThis->CaptureManager != nullptr)
				{
					DynamicVectorClass<ControlNode*> nodes = pThis->CaptureManager->ControlNodes;
					for (int j = 0; j < nodes.Count; j++)
					{
						vCaptured.emplace_back(nodes.GetItem(j));
					}
					pThis->CaptureManager->ControlNodes.Clear();
					GameDelete(pThis->CaptureManager);
				}

				pThis->CaptureManager = GameCreate<CaptureManagerClass>(pThis, pWeapon->Damage, pWeapon->InfiniteMindControl);

				for (ControlNode* node : vCaptured)
				{
					pThis->CaptureManager->ControlNodes.AddItem(node);
				}

				bCaptureSet = true;
			}
		}

		if (pWeapon->Warhead->Temporal && pThis->TemporalImUsing == nullptr)
		{
			hasTemporal = true;
			pThis->TemporalImUsing = GameCreate<TemporalClass>(pThis);
		}

		if (pWeapon->Spawner)
		{
			hasSpawn = true;

			if (!bSpawnSet)
			{
				if (pThis->SpawnManager == nullptr || pThis->SpawnManager->SpawnType != pType->Spawns)
				{
					if (pThis->SpawnManager != nullptr)
					{
						pThis->SpawnManager->KillNodes();
						GameDelete(pThis->SpawnManager);
					}

					pThis->SpawnManager =
						GameCreate<SpawnManagerClass>
						(
							pThis,
							pType->Spawns,
							pType->SpawnsNumber,
							pType->SpawnRegenRate,
							pType->SpawnReloadRate
						);
				}

				SpawnManagerClass* pManager = pThis->SpawnManager;

				pManager->SpawnCount = pType->SpawnsNumber;
				pManager->RegenRate = pType->SpawnRegenRate;
				pManager->ReloadRate = pType->SpawnReloadRate;

				while (pManager->SpawnedNodes.Count > pType->SpawnsNumber)
				{
					TechnoClass* pSpawn = pManager->SpawnedNodes.GetItem(0)->Unit;
					pThis->SpawnManager->SpawnedNodes.GetItem(0)->Unit->
						ReceiveDamage(&pSpawn->Health, 0, RulesClass::Instance->C4Warhead, nullptr, true, false, nullptr);
				}

				bSpawnSet = true;
			}
		}

		if (pWeapon->Warhead->Parasite && pFoot != nullptr)
		{
			hasParasite = true;

			if (pFoot->ParasiteImUsing == nullptr)
			{
				pFoot->ParasiteImUsing = GameCreate<ParasiteClass>(pFoot);
			}
		}
	}

	if (!hasCapture && pThis->CaptureManager != nullptr)
	{
		pThis->CaptureManager->FreeAll();
		GameDelete(pThis->CaptureManager);
		pThis->CaptureManager = nullptr;
	}

	if (!hasTemporal && pThis->TemporalImUsing != nullptr)
	{
		if (pThis->TemporalImUsing->Target != nullptr)
			pThis->TemporalImUsing->Detach();

		GameDelete(pThis->TemporalImUsing);
		pThis->TemporalImUsing = nullptr;
	}

	if (!hasSpawn && pThis->SpawnManager != nullptr)
	{
		pThis->SpawnManager->KillNodes();
		GameDelete(pThis->SpawnManager);
		pThis->SpawnManager = nullptr;
	}

	if (!hasParasite && pFoot != nullptr && pFoot->ParasiteImUsing != nullptr)
	{
		if (pFoot->ParasiteImUsing->Victim != nullptr)
			pFoot->ParasiteImUsing->ExitUnit();

		GameDelete(pFoot->ParasiteImUsing);
		pFoot->ParasiteImUsing = nullptr;
	}
}

void TechnoExt::ChangeLocomotorTo(TechnoClass* pThis, _GUID& locomotor)
{
	FootClass* pFoot = abstract_cast<FootClass*>(pThis);

	if (pFoot == nullptr)
		return;

	ILocomotion* pSource = pFoot->Locomotor.release();

	pFoot->Locomotor.reset(LocomotionClass::CreateInstance(locomotor).release());
	pFoot->Locomotor->Link_To_Object(pFoot);
	pSource->Release();
	Mission curMission = pFoot->GetCurrentMission();

	if (pFoot->Target != nullptr)
	{
		AbstractClass* pTarget = pFoot->Target;
		CoordStruct crdDest = pFoot->GetDestination();

		pFoot->ForceMission(Mission::Move);
		pFoot->MoveTo(&crdDest);
		pFoot->ForceMission(curMission);
		pFoot->ClickedMission(curMission, abstract_cast<ObjectClass*>(pTarget), abstract_cast<CellClass*>(pTarget), nullptr);
		pFoot->Guard();
		pFoot->Target = nullptr;
		pFoot->ClickedMission(curMission, abstract_cast<ObjectClass*>(pTarget), abstract_cast<CellClass*>(pTarget), nullptr);
	}
	else
	{
		CoordStruct crdTarget = pFoot->GetTargetCoords();
		pFoot->MoveTo(&crdTarget);
	}
}

bool TechnoExt::AttachEffect(TechnoClass* pThis, TechnoClass* pInvoker, AttachEffectTypeClass* pAttachType, int duration, int delay)
{
	if (pAttachType == nullptr || !pAttachType->PenetratesIronCurtain && pThis->IsIronCurtained())
		return false;

	ExtData* pExt = ExtMap.Find(pThis);
	std::vector<std::unique_ptr<AttachEffectClass>>& vAE = pExt->AttachEffects;

	if (!pAttachType->Cumulative)
	{
		auto it = std::find_if(vAE.begin(), vAE.end(),
			[pAttachType](std::unique_ptr<AttachEffectClass>& pAE)
			{
				return pAE->Type == pAttachType;
			});

		if (it != vAE.end())
		{
			auto& pAE = *it;

			if (pAE->Type->ResetIfExist_Timer)
				pAE->Timer.Start(std::max(duration, it->get()->Timer.GetTimeLeft()));

			if (pAE->Type->ResetIfExist_Anim)
			{
				pAE->KillAnim();
				pAE->CreateAnim();
			}

			return false;
		}
	}

	vAE.emplace_back(new AttachEffectClass(pAttachType, pInvoker, pThis, duration, delay));

	return true;
}

void TechnoExt::ExtData::CheckAttachEffects()
{
	AttachEffects.erase
	(
		std::remove_if(
			AttachEffects.begin(),
			AttachEffects.end(),
			[](std::unique_ptr<AttachEffectClass>& pAE)
			{
				return pAE == nullptr || pAE->Timer.Completed();
			})
		, AttachEffects.end()
	);

	if (!AttachEffects_Initialized)
	{
		TechnoTypeExt::ExtData* pTypeExt = TechnoTypeExt::ExtMap.Find(OwnerObject()->GetTechnoType());
		size_t size = std::min(pTypeExt->AttachEffects.size(), pTypeExt->AttachEffects_Duration.size());

		for (size_t i = 0; i < size; i++)
		{
			int iDelay = i < pTypeExt->AttachEffects_Delay.size() ? pTypeExt->AttachEffects_Delay[i] : 0;
			AttachEffect(OwnerObject(), OwnerObject(), pTypeExt->AttachEffects[i], pTypeExt->AttachEffects_Duration[i], iDelay);
		}

		AttachEffects_Initialized = true;
	}

	for (auto& pAE : AttachEffects)
	{
		pAE->Update();
	}
}

void TechnoExt::CheckTemperature(TechnoClass* pThis)
{
	for (const auto& pTempType : TemperatureTypeClass::Array)
	{
		pTempType->Update(pThis);
	}
}

void TechnoExt::Convert(TechnoClass* pThis, TechnoTypeClass* pTargetType, bool bDetachedBuildLimit)
{
	TechnoTypeClass* pOriginType = pThis->GetTechnoType();

	if (pOriginType->WhatAmI() != pTargetType->WhatAmI() || pOriginType == pTargetType)
		return;

	HouseClass* pHouse = pThis->GetOwningHouse();
	double healthPercentage = pThis->GetHealthPercentage();
	ExtData* pExt = ExtMap.Find(pThis);
	HouseExt::RegisterLoss(pHouse, pThis);
	int originIdx = pOriginType->GetArrayIndex();
	int targetIdx = pTargetType->GetArrayIndex();

	switch (pThis->WhatAmI())
	{
	case AbstractType::Infantry:
	{
		InfantryClass* pInf = abstract_cast<InfantryClass*>(pThis);
		InfantryTypeClass* pInfType = static_cast<InfantryTypeClass*>(pTargetType);

		if (pInf->IsDeployed() && !pInfType->Deployer)
		{
			pInf->ForceMission(Mission::Unload);
		}

		pInf->Type = pInfType;

		if (bDetachedBuildLimit)
		{
			pHouse->OwnedInfantryTypes.Increment(targetIdx);
			pHouse->OwnedInfantryTypes.Decrement(originIdx);
		}
		else
		{
			auto it = std::find(pExt->Convert_FromTypes.begin(), pExt->Convert_FromTypes.end(), pTargetType);

			if (it == pExt->Convert_FromTypes.end())
				pHouse->OwnedInfantryTypes.Increment(targetIdx);
			else
				pExt->Convert_FromTypes.erase(it);

			pExt->Convert_FromTypes.emplace_back(pOriginType);
		}
	}break;
	case AbstractType::Unit:
	{
		TechnoTypeExt::ExtData* pTargetTypeExt = TechnoTypeExt::ExtMap.Find(pTargetType);
		UnitClass* pUnit = abstract_cast<UnitClass*>(pThis);
		UnitTypeClass* pUnitType = static_cast<UnitTypeClass*>(pTargetType);
		pUnit->Type = pUnitType;
		pThis->PrimaryFacing.ROT.Value = static_cast<short>(std::min(pTargetType->ROT, 127) * 256);
		pThis->SecondaryFacing.ROT.Value = static_cast<short>(std::min(pTargetTypeExt->TurretROT.Get(pTargetType->ROT), 127) * 256);

		if (bDetachedBuildLimit)
		{
			pHouse->OwnedUnitTypes.Increment(targetIdx);
			pHouse->OwnedUnitTypes.Decrement(originIdx);
		}
		else
		{
			auto it = std::find(pExt->Convert_FromTypes.begin(), pExt->Convert_FromTypes.end(), pTargetType);

			if (it == pExt->Convert_FromTypes.end())
				pHouse->OwnedUnitTypes.Increment(targetIdx);
			else
				pExt->Convert_FromTypes.erase(it);

			pExt->Convert_FromTypes.emplace_back(pOriginType);
		}
	}break;
	case AbstractType::Aircraft:
	{
		TechnoTypeExt::ExtData* pTargetTypeExt = TechnoTypeExt::ExtMap.Find(pTargetType);
		AircraftClass* pAir = abstract_cast<AircraftClass*>(pThis);
		AircraftTypeClass* pAirType = static_cast<AircraftTypeClass*>(pTargetType);
		pAir->Type = pAirType;
		pThis->PrimaryFacing.ROT.Value = static_cast<short>(std::min(pTargetType->ROT, 127) * 256);
		pThis->SecondaryFacing.ROT.Value = static_cast<short>(std::min(pTargetTypeExt->TurretROT.Get(pTargetType->ROT), 127) * 256);

		if (bDetachedBuildLimit)
		{
			pHouse->OwnedAircraftTypes.Increment(targetIdx);
			pHouse->OwnedAircraftTypes.Decrement(originIdx);
		}
		else
		{
			auto it = std::find(pExt->Convert_FromTypes.begin(), pExt->Convert_FromTypes.end(), pTargetType);
			
			if (it == pExt->Convert_FromTypes.end())
				pHouse->OwnedAircraftTypes.Increment(targetIdx);
			else
				pExt->Convert_FromTypes.erase(it);

			pExt->Convert_FromTypes.emplace_back(pOriginType);
		}
	}break;
	case AbstractType::Building:
	{
		/*BuildingClass* pBuilding = abstract_cast<BuildingClass*>(pThis);
		BuildingTypeClass* pBuildingType = static_cast<BuildingTypeClass*>(pTargetType);
		pBuilding->Type = pBuildingType;
		pHouse->OwnedBuildingTypes.Increment(pTargetType->GetArrayIndex());

		if (bDetachedBuildLimit)
			pHouse->OwnedBuildingTypes.Decrement(pOriginType->GetArrayIndex());*/
		return;
	}break;
	default:
		return;
	}

	HouseExt::RegisterGain(pHouse, pThis);
	pThis->Health = std::max(static_cast<int>(pTargetType->Strength * healthPercentage), 1);
	pThis->Cloakable = pTargetType->Cloakable;
	FixManagers(pThis);
	FootClass* pFoot = abstract_cast<FootClass*>(pThis);
	TechnoTypeExt::ExtData* pTargetTypeExt = TechnoTypeExt::ExtMap.Find(pTargetType);

	if (pTargetTypeExt->GiftBoxData)
		pExt->AttachedGiftBox = std::make_unique<GiftBoxClass>(pThis);

	if (pFoot != nullptr && pOriginType->Locomotor != pTargetType->Locomotor)
		ChangeLocomotorTo(pThis, pTargetType->Locomotor);
}

void TechnoExt::RegisterLoss_ClearConvertFromTypesCounter(TechnoClass* pThis)
{
	ExtData* pExt = ExtMap.Find(pThis);
	HouseClass* pHouse = pThis->GetOwningHouse();
	AbstractType thisAbsType = pThis->WhatAmI();

	for (const auto pTechnoType : pExt->Convert_FromTypes)
	{
		int idx = pTechnoType->GetArrayIndex();

		switch (thisAbsType)
		{
		case AbstractType::Aircraft:
			pHouse->OwnedAircraftTypes.Decrement(idx);
			break;
		case AbstractType::Building:
			pHouse->OwnedBuildingTypes.Decrement(idx);
			break;
		case AbstractType::Infantry:
			pHouse->OwnedInfantryTypes.Decrement(idx);
			break;
		case AbstractType::Unit:
			pHouse->OwnedUnitTypes.Decrement(idx);
			break;
		default:
			return;
		}
	}
}

//变形逻辑扩展
void TechnoExt::InitialConvert(TechnoClass* pThis, TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt)
{
	if (pThis->GetTechnoType()->Passengers <= 0)
		return;

	if (pThis->WhatAmI() != AbstractType::Unit)
		return;

	if (pExt->OrignType == nullptr)
		pExt->OrignType = pThis->GetTechnoType();

	for (size_t i = 0; i < pTypeExt->Convert_Passangers.size(); i++)
	{
		auto pass = TechnoTypeClass::Array()->GetItem(pTypeExt->Convert_Passangers[i]);
		auto tech = TechnoTypeClass::Array()->GetItem(pTypeExt->Convert_Types[i]);

		pExt->Convert_Passangers.push_back(pass);
		pExt->Convert_Types.push_back(tech);
	}
}
//变形逻辑扩展
void TechnoExt::CheckPassanger(TechnoClass* const pThis, TechnoTypeClass* const pType, TechnoExt::ExtData* const pExt, TechnoTypeExt::ExtData* const pTypeExt)
{
	if (pThis->WhatAmI() != AbstractType::Unit)
		return;

	if (pExt->Convert_Passangers.empty() || pExt->Convert_Types.empty())
		return;

	if (!pTypeExt->UseConvert.Get())
		return;

	TechnoTypeClass* PassType = abstract_cast<TechnoClass*>(pThis->Passengers.GetFirstPassenger())->GetTechnoType();

	if (!PassType)
		return;

	if (std::find(pExt->Convert_Passangers.begin(), pExt->Convert_Passangers.end(), PassType) == pExt->Convert_Passangers.end())
		return;

	Nullable<TechnoTypeClass*> ChangeType;

	for (size_t i = 0; i < pTypeExt->Convert_Passangers.size(); i++)
	{
		TechnoTypeClass* Passanger = pExt->Convert_Passangers[i];

		if (strcmp(Passanger->get_ID(), PassType->get_ID()) == 0)
		{
			ChangeType = pExt->Convert_Types[i];
			break;

		}
	}

	if (!ChangeType)
		ChangeType = pExt->Convert_Types[0];

	if (!ChangeType)
		return;

	TechnoExt::UnitConvert(pThis, ChangeType, pThis->Passengers.GetFirstPassenger());
}
//变形逻辑扩展
void TechnoExt::UnitConvert(TechnoClass* pThis, TechnoTypeClass* pTargetType, FootClass* pFirstPassenger)
{
	if (pThis->WhatAmI() != AbstractType::Unit)
		return;

	Convert(pThis, pTargetType, true);

	if (pThis->GetTechnoType()->Gunner)
	{
		FootClass* pFoot = abstract_cast<FootClass*>(pThis);

		if (pThis->Passengers.NumPassengers > 0)
			pFoot->ReceiveGunner(pFirstPassenger);
		else
			pFoot->RemoveGunner(pFirstPassenger);
	}
}

// =============================
// load / save

template <typename T>
void TechnoExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Shield)
		.Process(this->LaserTrails)
		.Process(this->ReceiveDamage)
		.Process(this->AttachedGiftBox)
		.Process(this->PassengerDeletionTimer)
		.Process(this->PassengerDeletionCountDown)
		.Process(this->CurrentShieldType)
		.Process(this->LastWarpDistance)
		.Process(this->AutoDeathTimer)
		.Process(this->MindControlRingAnimType)
		.Process(this->OriginalPassengerOwner)
		.Process(this->CurrentLaserWeaponIndex)
		.Process(this->IsLeggedCyborg)

		.Process(this->ParentAttachment)
		.Process(this->ChildAttachments)

		.Process(this->IonCannon_setRadius)
		.Process(this->IonCannon_Radius)
		.Process(this->IonCannon_StartAngle)
		.Process(this->IonCannon_Stop)
		.Process(this->IonCannon_Rate)
		.Process(this->IonCannon_ROF)
		.Process(this->IonCannon_RadiusReduce)
		.Process(this->IonCannon_Angle)
		.Process(this->IonCannon_Scatter_Max)
		.Process(this->IonCannon_Scatter_Min)
		.Process(this->IonCannon_Duration)

		.Process(this->setIonCannonWeapon)
		.Process(this->setIonCannonType)
		.Process(this->IonCannonWeapon_setRadius)
		.Process(this->IonCannonWeapon_Radius)
		.Process(this->IonCannonWeapon_StartAngle)
		.Process(this->IonCannonWeapon_Stop)
		.Process(this->IonCannonWeapon_Target)
		.Process(this->IonCannonWeapon_ROF)
		.Process(this->IonCannonWeapon_RadiusReduce)
		.Process(this->IonCannonWeapon_Angle)
		.Process(this->IonCannonWeapon_Scatter_Max)
		.Process(this->IonCannonWeapon_Scatter_Min)
		.Process(this->IonCannonWeapon_Duration)
		.Process(this->setBeamCannon)
		.Process(this->BeamCannon_setLength)
		.Process(this->BeamCannon_Length)
		.Process(this->BeamCannon_Stop)
		.Process(this->BeamCannon_Target)
		.Process(this->BeamCannon_Self)
		.Process(this->BeamCannon_ROF)
		.Process(this->BeamCannon_LengthIncrease)
		.Process(this->PassengerNumber)
		.Process(this->PassengerList)
		.Process(this->PassengerlocationList)
		.Process(this->AllowCreatPassenger)
		.Process(this->AllowChangePassenger)
		.Process(this->AllowPassengerToFire)
		.Process(this->AllowFireCount)
		.Process(this->SpawneLoseTarget)

		.Process(this->ConvertsCounts)
		.Process(this->ConvertsOriginalType)
		.Process(this->ConvertsAnim)
		.Process(this->Convert_FromTypes)
		.Process(this->Convert_DetachedBuildLimit)

		.Process(this->DisableTurnCount)
		.Process(this->LastSelfFacing)
		.Process(this->LastTurretFacing)

		.Process(this->AllowToPaint)
		.Process(this->ColorToPaint)
		.Process(this->Paint_Count)
		.Process(this->Paint_IsDiscoColor)
		.Process(this->Paint_Colors)
		.Process(this->Paint_TransitionDuration)
		.Process(this->Paint_FramesPassed)
		.Process(this->Paint_IgnoreTintStatus)

		.Process(this->IsInROF)
		.Process(this->ROFCount)
		.Process(this->IsChargeROF)
		.Process(this->GattlingCount)
		.Process(this->GattlingStage)
		.Process(this->GattlingWeaponIndex)
		.Process(this->MaxGattlingCount)
		.Process(this->IsCharging)
		.Process(this->HasCharged)
		.Process(this->AttackTarget)
		.Process(this->GattlingWeapons)
		.Process(this->GattlingStages)
		.Process(this->PrimaryWeapon)
		.Process(this->SecondaryWeapon)
		.Process(this->WeaponFLHs)
		.Process(this->needConvertWhenLanding)
		.Process(this->JJ_landed)
		.Process(this->FloatingType)
		.Process(this->LandingType)

		.Process(this->Build_As)
		.Process(this->Build_As_OnlyOne)

		.Process(this->AttackedWeapon_Timer)

		.Process(this->CanDodge)
		.Process(this->DodgeDuration)
		.Process(this->Dodge_Houses)
		.Process(this->Dodge_MaxHealthPercent)
		.Process(this->Dodge_MinHealthPercent)
		.Process(this->Dodge_Chance)
		.Process(this->Dodge_Anim)
		.Process(this->Dodge_OnlyDodgePositiveDamage)

		.Process(this->LastLocation)
		.Process(this->MoveDamage_Duration)
		.Process(this->MoveDamage_Count)
		.Process(this->MoveDamage_Delay)
		.Process(this->MoveDamage)
		.Process(this->MoveDamage_Warhead)
		.Process(this->MoveDamage_Anim)
		.Process(this->StopDamage_Duration)
		.Process(this->StopDamage_Count)
		.Process(this->StopDamage_Delay)
		.Process(this->StopDamage)
		.Process(this->StopDamage_Warhead)
		.Process(this->StopDamage_Anim)

		.Process(this->IsSharingWeaponRange)
		.Process(this->BeSharedWeaponRange)
		.Process(this->ShareWeaponFire)

		.Process(this->IFVWeapons)
		.Process(this->IFVTurrets)

		.Process(this->BuildingROFFix)

		.Process(this->Attacker)
		.Process(this->Attacker_Count)

		.Process(this->LimitDamage)
		.Process(this->LimitDamageDuration)
		.Process(this->AllowMaxDamage)
		.Process(this->AllowMinDamage)

		.Process(this->TeamAffectCount)
		.Process(this->TeamAffectActive)
		.Process(this->TeamAffectLoseEfficacyCount)
		.Process(this->TeamAffectAnim)
		.Process(this->TeamAffectUnits)

		.Process(this->AttachEffects)
		.Process(this->AttachEffects_Initialized)
		.Process(this->AttachWeapon_Timers)

		.Process(this->FireSelf_Timers)

		.Process(this->LosePower)
		.Process(this->LosePowerAnim)
		.Process(this->LosePowerParticleCount)

		.Process(this->Temperature)
		.Process(this->Temperature_HeatUpTimer)
		.Process(this->Temperature_HeatUpDelayTimer)
		.Process(this->Temperature_WeaponTimer)

		.Process(this->ConvertPassanger)
		.Process(this->Convert_Passangers)
		.Process(this->Convert_Types)
		.Process(this->IsConverted)
		.Process(this->OrignType)

		.Process(this->CurrentRank)
		;
}

void TechnoExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<TechnoClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void TechnoExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<TechnoClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool TechnoExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Success();
}

bool TechnoExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Success();
}


// =============================
// container

TechnoExt::ExtContainer::ExtContainer() : Container("TechnoClass") { }

TechnoExt::ExtContainer::~ExtContainer() = default;

void TechnoExt::ExtContainer::InvalidatePointer(void* ptr, bool bRemoved) { }

// =============================
// container hooks

DEFINE_HOOK(0x6F3260, TechnoClass_CTOR, 0x5)
{
	GET(TechnoClass*, pItem, ESI);

	TechnoExt::ExtMap.FindOrAllocate(pItem);

	return 0;
}

DEFINE_HOOK(0x6F4500, TechnoClass_DTOR, 0x5)
{
	GET(TechnoClass*, pItem, ECX);

	TechnoExt::ExtMap.Remove(pItem);

	return 0;
}

DEFINE_HOOK_AGAIN(0x70C250, TechnoClass_SaveLoad_Prefix, 0x8)
DEFINE_HOOK(0x70BF50, TechnoClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(TechnoClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	TechnoExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x70C249, TechnoClass_Load_Suffix, 0x5)
{
	TechnoExt::ExtMap.LoadStatic();

	return 0;
}

DEFINE_HOOK(0x70C264, TechnoClass_Save_Suffix, 0x5)
{
	TechnoExt::ExtMap.SaveStatic();

	return 0;
}
