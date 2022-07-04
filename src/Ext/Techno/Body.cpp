#include "Body.h"

#include <BuildingClass.h>
#include <HouseClass.h>
#include <BulletClass.h>
#include <BulletTypeClass.h>
#include <ScenarioClass.h>
#include <SpawnManagerClass.h>
#include <InfantryClass.h>
#include <ParticleSystemClass.h>
#include <TacticalClass.h>
#include <Unsorted.h>
#include <BitFont.h>

#include <Utilities/PointerMapper.h>
#include <Utilities/EnumFunctions.h>
#include <Utilities/GeneralUtils.h>

#include <Ext/BulletType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Misc/FlyingStrings.h>
#include <Utilities/EnumFunctions.h>
#include <Ext/Team/Body.h>
#include <Ext/Script/Body.h>
#include <New/Type/IonCannonTypeClass.h>
#include <New/Type/GScreenAnimTypeClass.h>
#include <Misc/GScreenDisplay.h>
#include <Ext/Bullet/Body.h>
#include <JumpjetLocomotionClass.h>
#include <Utilities/PhobosGlobal.h>
#include <Utilities/EnumFunctions.h>
#include <Utilities/Helpers.Alex.h>

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
	
	if (Range <= 0)
		return;

	auto& Array = pThis->CaptureManager->ControlNodes;
	for (int i = 0; i < Array.Count; i++)
	{
		TechnoClass* pUnit = Array.GetItem(i)->Unit;
		if (pThis->DistanceFrom(pUnit) > Range)
			pThis->CaptureManager->FreeUnit(pUnit);
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

			for (auto const& pBullet : *BulletClass::Array)
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

				const auto& guardRange = pTypeExt->Interceptor_GuardRange.Get(pThis);
				const auto& minguardRange = pTypeExt->Interceptor_MinimumGuardRange.Get(pThis);

				auto distance = pBullet->Location.DistanceFrom(pThis->Location);

				if (distance > guardRange || distance < minguardRange)
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

	if (pTypeExt->MovePassengerToSpawn.Get())
	{
		SpawnManagerClass* pManager = pThis->SpawnManager;
		if (pManager != nullptr)
		{
			for (auto pItem : pManager->SpawnedNodes)
			{
				if (pItem->Unit->Passengers.NumPassengers == 0 && !(pItem->Status == SpawnNodeStatus::Idle || pItem->Status == SpawnNodeStatus::Reloading))
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
	else if(pExt->ConvertsCounts == 0)
	{
		auto percentage = pThis->GetHealthPercentage();

		if (pThis->WhatAmI() == AbstractType::Infantry &&
			pExt->ConvertsOriginalType->WhatAmI() == AbstractType::InfantryType)
		{
			if (abstract_cast<InfantryClass*>(pThis)->IsDeployed() && !static_cast<InfantryTypeClass*>(pExt->ConvertsOriginalType)->Deployer)
			{
				abstract_cast<InfantryClass*>(pThis)->Type->UndeployDelay = 0;
				pThis->ForceMission(Mission::Unload);
				pThis->ForceMission(Mission::Guard);
			}
			pThis->GetOwningHouse()->OwnedInfantryTypes.Decrement(pThis->GetTechnoType()->GetArrayIndex());
			abstract_cast<InfantryClass*>(pThis)->Type = static_cast<InfantryTypeClass*>(pExt->ConvertsOriginalType);
			abstract_cast<InfantryClass*>(pThis)->Health = int(static_cast<InfantryTypeClass*>(pExt->ConvertsOriginalType)->Strength * percentage);
			abstract_cast<InfantryClass*>(pThis)->Cloakable = static_cast<InfantryTypeClass*>(pExt->ConvertsOriginalType)->Cloakable;
			pThis->GetOwningHouse()->OwnedInfantryTypes.Increment(pThis->GetTechnoType()->GetArrayIndex());
		}
		else if (pThis->WhatAmI() == AbstractType::Unit &&
			pExt->ConvertsOriginalType->WhatAmI() == AbstractType::UnitType)
		{
			pThis->GetOwningHouse()->OwnedUnitTypes.Decrement(pThis->GetTechnoType()->GetArrayIndex());
			abstract_cast<UnitClass*>(pThis)->Type = static_cast<UnitTypeClass*>(pExt->ConvertsOriginalType);
			abstract_cast<UnitClass*>(pThis)->Health = int(static_cast<UnitTypeClass*>(pExt->ConvertsOriginalType)->Strength * percentage);
			abstract_cast<UnitClass*>(pThis)->Cloakable = static_cast<UnitTypeClass*>(pExt->ConvertsOriginalType)->Cloakable;
			pThis->GetOwningHouse()->OwnedUnitTypes.Increment(pThis->GetTechnoType()->GetArrayIndex());
		}
		else if (pThis->WhatAmI() == AbstractType::Aircraft &&
			pExt->ConvertsOriginalType->WhatAmI() == AbstractType::AircraftType)
		{
			pThis->GetOwningHouse()->OwnedAircraftTypes.Decrement(pThis->GetTechnoType()->GetArrayIndex());
			abstract_cast<AircraftClass*>(pThis)->Type = static_cast<AircraftTypeClass*>(pExt->ConvertsOriginalType);
			abstract_cast<AircraftClass*>(pThis)->Health  = int(static_cast<AircraftTypeClass*>(pExt->ConvertsOriginalType)->Strength * percentage);
			abstract_cast<AircraftClass*>(pThis)->Cloakable = static_cast<AircraftTypeClass*>(pExt->ConvertsOriginalType)->Cloakable;
			pThis->GetOwningHouse()->OwnedAircraftTypes.Increment(pThis->GetTechnoType()->GetArrayIndex());
		}

		if (pExt->ConvertsAnim != nullptr)
			GameCreate<AnimClass>(pExt->ConvertsAnim, pThis->GetCoords());

		pExt->ConvertsAnim = nullptr;
		pExt->ConvertsCounts--;
	}
	else
	{
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

void TechnoExt::DisableTurn(TechnoClass* pThis, TechnoExt::ExtData* pExt)
{
	if (pExt->DisableTurnCount > 0)
	{
		pThis->PrimaryFacing.set(pExt->SelfFacing);
		pThis->SecondaryFacing.set(pExt->TurretFacing);
		pExt->DisableTurnCount--;
	}
	else
	{
		pExt->SelfFacing = pThis->PrimaryFacing.current();
		pExt->TurretFacing = pThis->SecondaryFacing.current();
	}
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
			return true;
		else if (pThis->WhatAmI() == AbstractType::Unit && mission == Mission::Guard
			&& !pThis->IsSelected)
		{
			if (auto pFoot = abstract_cast<UnitClass*>(pThis))
				return pFoot->Locomotor->Is_Really_Moving_Now();
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
				if(pExt->MoveDamage_Warhead)
					pThis->ReceiveDamage(&pExt->MoveDamage, 0, pExt->MoveDamage_Warhead, nullptr, true, false, pThis->Owner);
				else
					pThis->ReceiveDamage(&pExt->MoveDamage, 0, RulesClass::Instance()->C4Warhead, nullptr, true, false, pThis->Owner);
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
				if(pTypeExt->MoveDamage_Warhead.isset())
					pThis->ReceiveDamage(&pTypeExt->MoveDamage, 0, pTypeExt->MoveDamage_Warhead, nullptr, true, false, pThis->Owner);
				else
					pThis->ReceiveDamage(&pTypeExt->MoveDamage, 0, RulesClass::Instance()->C4Warhead, nullptr, true, false, pThis->Owner);
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
	if (!pTypeExt->WeaponRangeShare_Techno.empty() && pTypeExt->WeaponRangeShare_Range > 0)
	{
		auto pExt = TechnoExt::ExtMap.Find(pThis);
		pExt->IsSharingWeaponRange = true;
		for (unsigned int i = 0; i < pTypeExt->WeaponRangeShare_Techno.size(); i++)
		{
			auto CanAffectTarget = pTypeExt->WeaponRangeShare_Techno[i];
			for (auto pAffect : Helpers::Alex::getCellSpreadItems(pThis->GetCoords(), pTypeExt->WeaponRangeShare_Range, true))
			{
				auto pAffectExt = TechnoExt::ExtMap.Find(pAffect);
				if (pAffect->GetTechnoType() == CanAffectTarget && pThis->GetOwningHouse() == pAffect->GetOwningHouse() && pAffect != pThis && !pAffectExt->IsSharingWeaponRange)
				{
					if (!pTypeExt->WeaponRangeShare_ForceAttack && pAffect->GetCurrentMission() != Mission::Guard)
						continue;
					pAffect->SetTarget(pTarget);

					const CoordStruct source = pAffect->Location;
					const CoordStruct target = pTarget->GetCoords();
					const DirStruct tgtDir = DirStruct(Math::arctanfoo(source.Y - target.Y, target.X - source.X));
					
					if (pAffect->WhatAmI() == AbstractType::Building)
					{
						pAffect->PrimaryFacing.turn(tgtDir);
						pAffect->SecondaryFacing.turn(tgtDir);
					}
					else
					{
						if (pAffect->HasTurret())
							pAffect->SecondaryFacing.turn(tgtDir);
						else
							pAffect->PrimaryFacing.turn(tgtDir);
					}

					BulletClass* pBullet = pAffect->TechnoClass::Fire(pTarget, 0);
					if (pBullet != nullptr) 
						pBullet->Owner = pAffect;
					pAffectExt->BeSharedWeaponRange = true;
				}
			}
		}
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
	else if (pExt->IsExtendGattling)
	{
		pData->WeaponFLHs = pExt->WeaponFLHs;
		pickedFLHs = pData->WeaponFLHs;
		FLHFound = true;
		FLH = pickedFLHs[Index].GetItem(0);
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
	auto const pData = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);

	if (pData && pWeaponExt->PassengerDeletion.Get())
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
					while (pTechnoData->PassengerlocationList[j] != CoordStruct {0, 0, 0 })
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

// Feature: Kill Object Automatically
void inline TechnoExt::KillSelf(TechnoClass* pThis, bool isPeaceful)
{
	if (isPeaceful)
	{
		pThis->Limbo();
		pThis->UnInit();
	}
	else
	{
		pThis->ReceiveDamage(&pThis->Health, 0, RulesClass::Instance()->C4Warhead, nullptr, true, false, pThis->Owner);
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

	if (isdeath)
	{
		if (pTypeExt->Death_Peaceful.Get())
		{
			pThis->Limbo();
			pThis->UnInit();
		}
		else
		{
			pThis->ReceiveDamage(&pThis->Health, 0, RulesClass::Instance()->C4Warhead, nullptr, true, false, pThis->Owner);
		}
	}
}

void TechnoExt::CheckDeathConditions(TechnoClass* pThis, TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt)
{
	const bool isPeaceful = pTypeExt->Death_Peaceful.Get();
	TechnoTypeClass* pType = pThis->GetTechnoType();

	// Death if no ammo
	if (pTypeExt->Death_NoAmmo.Get() && pType->Ammo > 0 && pThis->Ammo <= 0)
	{
		TechnoExt::KillSelf(pThis, isPeaceful);
		return;
	}

	// Death if countdown ends
	if (pTypeExt->Death_Countdown > 0)
	{
		if (pExt->Death_Countdown >= 0)
		{
			if (pExt->Death_Countdown > 0)
				pExt->Death_Countdown--; // Update countdown
			else
			{
				// Countdown ended. Kill the unit
				pExt->Death_Countdown = -1;
				TechnoExt::KillSelf(pThis, isPeaceful);
				return;
			}
		}
		else
		{
			pExt->Death_Countdown = pTypeExt->Death_Countdown; // Start countdown
		}
	}

	// Death if slave owner dead
	if (pTypeExt->Death_WithMaster.Get() && pType->Slaved && (!pThis->SlaveOwner || !pThis->SlaveOwner->IsAlive))
		TechnoExt::KillSelf(pThis, isPeaceful);
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
				if (pThis->WhatAmI() == AbstractType::Building)
				{
					auto const pSelf = abstract_cast<BuildingClass*>(pThis);
					int FoundationX = pSelf->GetFoundationData()->X, FoundationY = pSelf->GetFoundationData()->Y;
					if (FoundationX > 0 )
					{
						FoundationX = 1;
					}
					if (FoundationY > 0)
					{
						FoundationY = 1;
					}
					center = pThis->GetCoords() + CoordStruct { (FoundationX * 256) / 2, (FoundationY * 256) / 2 };
				}

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
			pData->IonCannonWeapon_Target = abstract_cast<ObjectClass*>(pTarget)->Location;
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
				pData->IonCannonWeapon_Target = pTarget->GetCoords() + CoordStruct { (FoundationX * 256) / 2, (FoundationY * 256) / 2 };
			}
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
				if (pThis->WhatAmI() == AbstractType::Building)
				{
					auto const pSelfBuilding = abstract_cast<BuildingClass*>(pThis);
					int FoundationX = pSelfBuilding->GetFoundationData()->X, FoundationY = pSelfBuilding->GetFoundationData()->Y;
					if (FoundationX > 0)
					{
						FoundationX = 1;
					}
					if (FoundationY > 0)
					{
						FoundationY = 1;
					}
					pData->BeamCannon_Self = pThis->GetCoords() + CoordStruct { (FoundationX * 256) / 2, (FoundationY * 256) / 2 };
				}
				pData->BeamCannon_Stop = false;
			}
			else
			{
				pData->BeamCannon_Target = abstract_cast<ObjectClass*>(pTarget)->Location;
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
					pData->BeamCannon_Target = pTarget->GetCoords() + CoordStruct { (FoundationX * 256) / 2, (FoundationY * 256) / 2 };
				}
				pData->BeamCannon_Self = pThis->GetCoords();
				if (pThis->WhatAmI() == AbstractType::Building)
				{
					auto const pSelfBuilding = abstract_cast<BuildingClass*>(pThis);
					int FoundationX = pSelfBuilding->GetFoundationData()->X, FoundationY = pSelfBuilding->GetFoundationData()->Y;
					if (FoundationX > 0)
					{
						FoundationX = 1;
					}
					if (FoundationY > 0)
					{
						FoundationY = 1;
					}
					pData->BeamCannon_Self = pThis->GetCoords() + CoordStruct { (FoundationX * 256) / 2, (FoundationY * 256) / 2 };
				}
				pData->BeamCannon_Stop = false;
			}
		}
	}
}

void TechnoExt::RunBeamCannon(TechnoClass* pThis, TechnoExt::ExtData* pExt)
{
	WeaponTypeClass* pWeapon = pExt->setBeamCannon.Get();
	
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

void TechnoExt::RunFireSelf(TechnoClass* pThis, TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt)
{	
	if (pThis->IsRedHP() && !pTypeExt->FireSelf_Weapon_RedHealth.empty()  && !pTypeExt->FireSelf_ROF_RedHealth.empty())
	{
		pExt->FireSelf_Weapon = pTypeExt->FireSelf_Weapon_RedHealth;
		pExt->FireSelf_ROF = pTypeExt->FireSelf_ROF_RedHealth;
	}
	else if (pThis->IsYellowHP() && !pTypeExt->FireSelf_Weapon_YellowHealth.empty() && !pTypeExt->FireSelf_ROF_YellowHealth.empty())
	{
		pExt->FireSelf_Weapon = pTypeExt->FireSelf_Weapon_YellowHealth;
		pExt->FireSelf_ROF = pTypeExt->FireSelf_ROF_YellowHealth;
	}
	else if (pThis->IsGreenHP() && !pTypeExt->FireSelf_Weapon_GreenHealth.empty() && !pTypeExt->FireSelf_ROF_GreenHealth.empty())
	{
		pExt->FireSelf_Weapon = pTypeExt->FireSelf_Weapon_GreenHealth;
		pExt->FireSelf_ROF = pTypeExt->FireSelf_ROF_GreenHealth;
	}
	else
	{
		pExt->FireSelf_Weapon = pTypeExt->FireSelf_Weapon;
		pExt->FireSelf_ROF = pTypeExt->FireSelf_ROF;
	}

	if (pExt->FireSelf_Weapon.empty()) return;
	if (pExt->FireSelf_Count.size() < pExt->FireSelf_Weapon.size())
	{
		int p = int(pExt->FireSelf_Count.size());
		while (pExt->FireSelf_Count.size() < pExt->FireSelf_Weapon.size())
		{
			int ROF = 10;
			if (p >= (int)pExt->FireSelf_ROF.size()) ROF = pExt->FireSelf_Weapon[p]->ROF;
			else ROF = pExt->FireSelf_ROF[p];
			pExt->FireSelf_Count.emplace_back(ROF);
		}
	}

	for (size_t i = 0; i < pExt->FireSelf_Count.size(); i++)
	{
		pExt->FireSelf_Count[i]--;
		if (pExt->FireSelf_Count[i] > 0) continue;
		else
		{
			int ROF = 10;
			if (i >= (int)pExt->FireSelf_ROF.size()) ROF = pExt->FireSelf_Weapon[i]->ROF;
			else ROF = pExt->FireSelf_ROF[i];
			pExt->FireSelf_Count[i] = ROF;
			WeaponTypeExt::DetonateAt(pExt->FireSelf_Weapon[i], pThis, pThis);
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
		while (pBuilding->GetFireError(pThis->Target, 0, true) != FireError::OK && count > 0 && pThis->GetCurrentMission() == Mission::Attack)
		{
			if (pBuilding->FiringOccupantIndex == pBuilding->Occupants.Count - 1)
				pBuilding->FiringOccupantIndex = 0;
			else
				pBuilding->FiringOccupantIndex++;

			count--;
		}
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

	if (pType->Passengers > 0)
	{
		if (pThis->Passengers.NumPassengers == 0 && pThis->GetCurrentMission() == Mission::Unload)
		{
			pThis->ForceMission(Mission::Stop);
			pThis->Target = nullptr;
			pThis->ForceMission(Mission::Guard);
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
		auto itAttachment = std::find_if(pParentExt->ChildAttachments.begin(), pParentExt->ChildAttachments.end(), [pThis](std::unique_ptr<AttachmentClass>& pAttachment)
		{
			return pThis == pAttachment->Child;
		 });
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

void TechnoExt::UpdateMindControlAnim(TechnoClass* pThis, TechnoExt::ExtData* pExt)
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

void TechnoExt::DrawSelfHealPips(TechnoClass* pThis, Point2D* pLocation, RectangleStruct* pBounds)
{
	bool drawPip = false;
	bool isInfantryHeal = false;
	int selfHealFrames = 0;

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

		Point2D position = { pLocation->X + xOffset, pLocation->Y + yOffset };

		auto flags = BlitterFlags::bf_400 | BlitterFlags::Centered;

		if (isSelfHealFrame)
			flags = flags | BlitterFlags::Darken;

		DSurface::Temp->DrawSHP(FileSystem::PALETTE_PAL, FileSystem::PIPS_SHP,
		pipFrame, &position, pBounds, flags, 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
	}
}

void TechnoExt::DrawGroupID_Building(TechnoClass* pThis, TechnoTypeExt::ExtData* pTypeExt, Point2D* pLocation)
{
	CoordStruct vCoords = { 0, 0, 0 };
	pThis->GetTechnoType()->Dimension2(&vCoords);

	CoordStruct vCoords2 = { -vCoords.X / 2, vCoords.Y / 2,vCoords.Z };

	Point2D vPos = { 0, 0 };
	TacticalClass::Instance->CoordsToScreen(&vPos, &vCoords2);

	Point2D vLoc = *pLocation;
	vLoc.X += vPos.X;
	vLoc.Y += vPos.Y;

	Point2D vOffset = pTypeExt->GroupID_Offset;

	vLoc.X += vOffset.X;
	vLoc.Y += vOffset.Y;

	if (pThis->Group >= 0)
	{

		const COLORREF GroupIDColor = Drawing::RGB2DWORD(pThis->GetOwningHouse()->Color.R, pThis->GetOwningHouse()->Color.G, pThis->GetOwningHouse()->Color.B);

		RectangleStruct rect
		{
			vLoc.X - 7,
			vLoc.Y + 26,
			12,13
		};

		DSurface::Temp->FillRect(&rect, COLOR_BLACK);
		DSurface::Temp->DrawRect(&rect, GroupIDColor);

		int groupid = (pThis->Group == 9) ? 0 : (pThis->Group + 1);

		wchar_t GroupID[0x20];
		swprintf_s(GroupID, L"%d", groupid);

		Point2D vGroupPos
		{
			vLoc.X - 4,
			vLoc.Y + 25
		};

		TextPrintType PrintType = TextPrintType(int(TextPrintType::NoShadow));

		DSurface::Temp->GetRect(&rect);
		DSurface::Temp->DrawTextA(GroupID, &rect, &vGroupPos, GroupIDColor, 0, PrintType);
	}
}

void TechnoExt::DrawGroupID_Other(TechnoClass* pThis, TechnoTypeExt::ExtData* pTypeExt, Point2D* pLocation)
{
	Point2D vLoc = *pLocation;

	Point2D vOffset = pTypeExt->GroupID_Offset;

	vLoc.X += vOffset.X;
	vLoc.Y += vOffset.Y;

	if (pThis->Group >= 0)
	{
		if (pThis->WhatAmI() == AbstractType::Infantry)
		{
			vLoc.X -= 20;
			vLoc.Y -= 25;
		}
		else
		{
			vLoc.X -= 30;
			vLoc.Y -= 23;
		}

		const COLORREF GroupIDColor = Drawing::RGB2DWORD(pThis->GetOwningHouse()->Color.R, pThis->GetOwningHouse()->Color.G, pThis->GetOwningHouse()->Color.B);

		RectangleStruct rect
		{
			vLoc.X,
			vLoc.Y,
			12,13
		};

		DSurface::Temp->FillRect(&rect, COLOR_BLACK);
		DSurface::Temp->DrawRect(&rect, GroupIDColor);

		int groupid = (pThis->Group == 9) ? 0 : (pThis->Group + 1);

		wchar_t GroupID[0x20];
		swprintf_s(GroupID, L"%d", groupid);

		DSurface::Temp->GetRect(&rect);

		Point2D vGroupPos
		{
			vLoc.X + 2,
			vLoc.Y - 1
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

void TechnoExt::DrawSelectBox(TechnoClass* pThis, TechnoTypeExt::ExtData* pTypeExt, int iLength, Point2D* pLocation, RectangleStruct* pBound, bool isInfantry)
{
	Point2D vPos = { 0, 0 };
	Point2D vLoc = *pLocation;
	Point2D vOfs = { 0, 0 };

	int frame, XOffset, YOffset;

	Vector3D<int> glbSelectbrdFrame = isInfantry ?
		RulesExt::Global()->SelectBox_Frame_Infantry.Get() :
		RulesExt::Global()->SelectBox_Frame_Unit.Get();
	Vector3D<int> selectboxFrame = pTypeExt->SelectBox_Frame.Get();
	auto const nFlag = BlitterFlags::Centered | BlitterFlags::Nonzero | BlitterFlags::MultiPass | EnumFunctions::GetTranslucentLevel(pTypeExt->SelectBox_TranslucentLevel.Get(RulesExt::Global()->SelectBox_DefaultTranslucentLevel.Get()));
	auto const canSee = pThis->Owner->IsAlliedWith(HouseClass::Player)
		|| HouseClass::IsPlayerObserver()
		|| pTypeExt->SelectBox_ShowEnemy.Get(RulesExt::Global()->SelectBox_DefaultShowEnemy.Get());

	if (selectboxFrame.X == -1)
		selectboxFrame = glbSelectbrdFrame;

	vOfs = pTypeExt->SelectBox_DrawOffset.Get();
	if (vOfs.X == NULL || vOfs.Y == NULL)
	{
		if (isInfantry)
			vOfs = RulesExt::Global()->SelectBox_DrawOffset_Infantry.Get();
		else
			vOfs = RulesExt::Global()->SelectBox_DrawOffset_Unit.Get();
	}

	XOffset = vOfs.X;

	YOffset = pThis->GetTechnoType()->PixelSelectionBracketDelta;
	YOffset += vOfs.Y;
	vLoc.Y -= 5;

	if (iLength == 8)
	{
		vPos.X = vLoc.X + 1 + XOffset;
		vPos.Y = vLoc.Y + 6 + YOffset;
	}
	else
	{
		vPos.X = vLoc.X + 2 + XOffset;
		vPos.Y = vLoc.Y + 6 + YOffset;
	}

	SHPStruct* SelectBoxSHP = pTypeExt->SHP_SelectBoxSHP;
	SHPStruct* GlbSelectBoxSHP = nullptr;
	if (isInfantry)
		GlbSelectBoxSHP = RulesExt::Global()->SHP_SelectBoxSHP_INF;
	else
		GlbSelectBoxSHP = RulesExt::Global()->SHP_SelectBoxSHP_UNIT;
	if (SelectBoxSHP == nullptr)
	{
		char FilenameSHP[0x20];
		strcpy_s(FilenameSHP, pTypeExt->SelectBox_SHP.data());

		if (strcmp(FilenameSHP, "") == 0)
		{
			if (GlbSelectBoxSHP == nullptr)
			{
				char GlbFilenameSHP[0x20];
				if (isInfantry)
					strcpy_s(GlbFilenameSHP, RulesExt::Global()->SelectBox_SHP_Infantry.data());
				else
					strcpy_s(GlbFilenameSHP, RulesExt::Global()->SelectBox_SHP_Unit.data());

				if (strcmp(GlbFilenameSHP, "") == 0)
					return;
				else
					SelectBoxSHP = pTypeExt->SHP_SelectBoxSHP = FileSystem::LoadSHPFile(GlbFilenameSHP);
			}
			else
				SelectBoxSHP = GlbSelectBoxSHP;
		}
		else
			SelectBoxSHP = pTypeExt->SHP_SelectBoxSHP = FileSystem::LoadSHPFile(FilenameSHP);
	}
	if (SelectBoxSHP == nullptr) return;

	ConvertClass* SelectBoxPAL = pTypeExt->SHP_SelectBoxPAL;
	ConvertClass* GlbSelectBoxPAL = nullptr;
	if (isInfantry)
		GlbSelectBoxPAL = RulesExt::Global()->SHP_SelectBoxPAL_INF;
	else
		GlbSelectBoxPAL = RulesExt::Global()->SHP_SelectBoxPAL_UNIT;
	if (SelectBoxPAL == nullptr)
	{
		char FilenamePAL[0x20];
		strcpy_s(FilenamePAL, pTypeExt->SelectBox_PAL.data());

		if (strcmp(FilenamePAL, "") == 0)
		{
			if (GlbSelectBoxPAL == nullptr)
			{
				char GlbFilenamePAL[0x20];
				if (isInfantry)
					strcpy_s(GlbFilenamePAL, RulesExt::Global()->SelectBox_PAL_Infantry.data());
				else
					strcpy_s(GlbFilenamePAL, RulesExt::Global()->SelectBox_PAL_Unit.data());

				if (strcmp(GlbFilenamePAL, "") == 0)
					return;
				else
					SelectBoxPAL = pTypeExt->SHP_SelectBoxPAL = FileSystem::LoadPALFile(GlbFilenamePAL, DSurface::Temp);
			}
			else
				SelectBoxPAL = GlbSelectBoxPAL;
		}
		else
			SelectBoxPAL = pTypeExt->SHP_SelectBoxPAL = FileSystem::LoadPALFile(FilenamePAL, DSurface::Temp);
	}
	if (SelectBoxPAL == nullptr) return;

	if (pThis->IsSelected && canSee)
	{
		if (pThis->IsGreenHP())
			frame = selectboxFrame.X;
		else if (pThis->IsYellowHP())
			frame = selectboxFrame.Y;
		else
			frame = selectboxFrame.Z;
		DSurface::Temp->DrawSHP(SelectBoxPAL, SelectBoxSHP,
			frame, &vPos, pBound, nFlag, 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
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

void TechnoExt::InitialShowHugeHP(TechnoClass* pThis)
{
	auto pType = pThis->GetTechnoType();
	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
	if (pType == nullptr || pThis->InLimbo || !pTypeExt->HugeHP_Show.Get())
		return;
	int Priority = pTypeExt->HugeHP_Priority.Get();
	auto& Technos = PhobosGlobal::Global()->Techno_HugeBar;
	auto it = Technos.find(Priority);
	while (it != Technos.end() && it->first == Priority)
	{
		if (it->second == pThis)
			return;
		it++;
	}
	Technos.emplace(Priority, pThis);
}

void TechnoExt::RunHugeHP()
{
	if (PhobosGlobal::Global()->Techno_HugeBar.empty())
		return;
	TechnoClass* pThis = PhobosGlobal::Global()->Techno_HugeBar.begin()->second;
	if (pThis != nullptr) TechnoExt::UpdateHugeHP(pThis);
}

void TechnoExt::EraseHugeHP(TechnoClass* pThis, TechnoTypeExt::ExtData* pTypeExt)
{
	if (!pTypeExt->HugeHP_Show.Get())
		return;
	int Priority = pTypeExt->HugeHP_Priority.Get();
	auto& Technos = PhobosGlobal::Global()->Techno_HugeBar;
	auto it = Technos.find(Priority);
	while (it != Technos.end() && it->first == Priority)
	{
		if (it->second == pThis)
		{
			Technos.erase(it);
			return;
		}
		it++;
	}
}

void TechnoExt::UpdateHugeHP(TechnoClass* pThis)
{
	DrawHugeHP(pThis);
	DrawHugeSP(pThis);
}

void TechnoExt::DrawHugeHP(TechnoClass* pThis)
{
	auto pTypeThis = pThis->GetTechnoType();
	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pTypeThis);
	auto pData = TechnoExt::ExtMap.Find(pThis);

	if (!(pTypeThis && pTypeExt && pData)) return;
	if (!pTypeExt->HugeHP_Show.Get()) return;

	int CurrentValue = pThis->Health;
	int MaxValue = pThis->GetTechnoType()->Strength;

	HealthState State;
	Vector3D<int> Color1Vector;
	Vector3D<int> Color2Vector;
	if (pThis->IsGreenHP())
	{
		Color1Vector = RulesExt::Global()->HugeHP_HighColor1.Get();
		Color2Vector = RulesExt::Global()->HugeHP_HighColor2.Get();
		State = HealthState::Green;
	}
	else if (pThis->IsYellowHP())
	{
		Color1Vector = RulesExt::Global()->HugeHP_MidColor1.Get();
		Color2Vector = RulesExt::Global()->HugeHP_MidColor2.Get();
		State = HealthState::Yellow;
	}
	else
	{
		Color1Vector = RulesExt::Global()->HugeHP_LowColor1.Get();
		Color2Vector = RulesExt::Global()->HugeHP_LowColor2.Get();
		State = HealthState::Red;
	}

	// 巨型血条
	if (RulesExt::Global()->HugeHP_CustomSHPShowBar.Get()) // 激活自定义SHP血条，关闭需单独定义格子的SHP血条，关闭矩形血条
	{
		SHPStruct* CustomSHP = RulesExt::Global()->SHP_HugeHPCustom;
		ConvertClass* CustomPAL = RulesExt::Global()->PAL_HugeHPCustom;
		if (CustomSHP == nullptr || CustomPAL == nullptr)
			return;

		// 读取自定义SHP文件信息
		int frames = CustomSHP->Frames;

		// 当前帧
		int currentFrameIndex = int((double)pThis->Health / (double)pThis->GetTechnoType()->Strength * frames) - 1;
		if (currentFrameIndex < 0)
			currentFrameIndex = 0;
		if (currentFrameIndex > frames - 1)
			currentFrameIndex = frames - 1;

		// 自定义SHP的左上角绘制位置
		Point2D posBarNW = {
			DSurface::Composite->GetWidth() / 2 - CustomSHP->Width / 2 ,
			220
		};

		// 读取整体位置offset
		Vector2D<int> offset = RulesExt::Global()->HugeHP_ShowOffset.Get();
		posBarNW += offset;

		// 绘制自定义血条，包含框和格子
		DSurface::Composite->DrawSHP(CustomPAL, CustomSHP, currentFrameIndex, &posBarNW, &DSurface::ViewBounds,
			BlitterFlags::None, 0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);

	}
	else if (RulesExt::Global()->HugeHP_UseSHPShowBar.Get()) // 激活SHP巨型血条、护盾条，关闭矩形巨型血条、护盾条
	{
		SHPStruct* PipsSHP = RulesExt::Global()->SHP_HugeHPPips;
		ConvertClass* PipsPAL = RulesExt::Global()->PAL_HugeHPPips;
		SHPStruct* BarSHP = RulesExt::Global()->SHP_HugeHPBar;
		ConvertClass* BarPAL = RulesExt::Global()->PAL_HugeHPBar;
		if (PipsSHP == nullptr || PipsPAL == nullptr || BarSHP == nullptr || BarPAL == nullptr) return;

		// 读取格子和框的SHP文件尺寸
		int pipWidth = RulesExt::Global()->HugeHP_PipWidth.Get(PipsSHP->Width);
		int pipHeight = PipsSHP->Height;
		int barWidth = BarSHP->Width;
		int barHeight = BarSHP->Height;

		// 满血时的格子数量
		int pipsCount = RulesExt::Global()->HugeHP_PipsCount.Get(100);

		int iPipsTotal = int((double)pThis->Health / (double)pThis->GetTechnoType()->Strength * pipsCount);
		if (iPipsTotal < 0)
			iPipsTotal = 0;
		if (iPipsTotal > pipsCount)
			iPipsTotal = pipsCount;

		Vector3D<int> framesStruct = RulesExt::Global()->HugeHP_PipsFrames.Get();
		if (framesStruct.X == -1 || framesStruct.Y == -1 || framesStruct.Z == -1)
		{
			// 格子文件检测帧数，小于3帧则选第1帧，3帧及以上则选第1、2、3帧
			// framesStruct = PipsSHP->Frames > 2 ? { 0, 1, 2 } : { 0, 0, 0 } ; // 报错
			if (PipsSHP->Frames > 2)
				framesStruct = { 0, 1, 2 };
			else
				framesStruct = { 0, 0, 0 };
		}
		int pipsFrame = framesStruct.X;
		if (State == HealthState::Yellow) pipsFrame = framesStruct.Y;
		if (State == HealthState::Red) pipsFrame = framesStruct.Z;

		framesStruct = RulesExt::Global()->HugeHP_BarFrames.Get();
		if (framesStruct.X == -1 || framesStruct.Y == -1 || framesStruct.Z == -1)
		{
			// 框文件检测帧数，小于3帧则选第1帧，3帧及以上则选第1、2、3帧
			// framesStruct = BarSHP->Frames > 2 ? { 0, 1, 2 } : { 0, 0, 0 } ; // 报错
			if (BarSHP->Frames > 2)
				framesStruct = { 0, 1, 2 };
			else
				framesStruct = { 0, 0, 0 };
		}
		int barFrame = framesStruct.X;
		if (State == HealthState::Yellow) barFrame = framesStruct.Y;
		if (State == HealthState::Red) barFrame = framesStruct.Z;

		// 格子的左上角绘制位置
		Point2D posPipNW = {
			DSurface::Composite->GetWidth() / 2 - pipWidth * pipsCount / 2 ,
			120
		};
		// 框的左上角绘制位置
		Point2D posBarNW = {
			DSurface::Composite->GetWidth() / 2 - barWidth / 2 ,
			120 - (barHeight - pipHeight) / 2
		};

		// 读取整体位置offset，同时影响框和格子，因此无法改变框和格子的相对位置
		Vector2D<int> offset = RulesExt::Global()->HugeHP_ShowOffset.Get();
		posPipNW += offset;
		posBarNW += offset;
		// 读取格子整体位置offset
		Vector2D<int> offsetPips = RulesExt::Global()->HugeHP_PipsOffset.Get();
		posPipNW += offsetPips;

		// 每个格子相对于前一个格子的实际XY偏移量
		Vector2D<int> realPipOffset = { pipWidth, 0 };
		realPipOffset += RulesExt::Global()->HugeHP_PipToPipOffset.Get();

		// 绘制框
		DSurface::Composite->DrawSHP(BarPAL, BarSHP, barFrame, &posBarNW, &DSurface::ViewBounds,
		BlitterFlags::None, 0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
		// 绘制格子
		for (int i = 0; i < iPipsTotal; i++)
		{
			DSurface::Composite->DrawSHP(PipsPAL, PipsSHP, pipsFrame, &posPipNW, &DSurface::ViewBounds,
			BlitterFlags::None, 0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);

			posPipNW += realPipOffset;
		}
		if (RulesExt::Global()->HugeHP_DrawOrderReverse.Get())
		{
			// 再次绘制框
			DSurface::Composite->DrawSHP(BarPAL, BarSHP, barFrame, &posBarNW, &DSurface::ViewBounds,
			BlitterFlags::None, 0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
		}

	}
	else
	{
		COLORREF CurrentColor1 = Drawing::RGB2DWORD(Color1Vector.X, Color1Vector.Y, Color1Vector.Z);
		COLORREF CurrentColor2 = Drawing::RGB2DWORD(Color2Vector.X, Color2Vector.Y, Color2Vector.Z);

		// 计算生命格子数

		Vector2D<int> RectWH = RulesExt::Global()->HugeHP_RectWH.Get({ DSurface::Composite->GetWidth() * 87 / 100 / 100, 50 });
		int RectCount = RulesExt::Global()->HugeHP_RectCount.Get(100);

		Vector2D<int> Offset = RulesExt::Global()->HugeHP_ShowOffset.Get();
		Vector2D<int> BorderWH = RulesExt::Global()->HugeHP_BorderWH.Get({ DSurface::Composite->GetWidth() * 87 / 100 / 100 * 100 + 12, 60 });

		int pipWidth = RectWH.X;
		int pipHeight = RectWH.Y;
		int pipsTotalWidth = pipWidth * RectCount;
		int pipWidthL = pipWidth * 7 / 10; // 浅色占大部分
		int pipWidthR = pipWidth - pipWidthL; // 右部分深色，模拟生命格子效果而非连续效果
		int SpaceX = (BorderWH.X - pipsTotalWidth) / 2;
		int SpaceY = (BorderWH.Y - pipHeight) / 2;
		int pip2left = DSurface::Composite->GetWidth() / 2 - pipsTotalWidth / 2 - SpaceX; // 屏幕左边缘与生命小格子的最小距离
		int iPipsTotal = int(double(CurrentValue) / MaxValue * RectCount);

		RectangleStruct pipbrdRect = {
			pip2left + Offset.X,
			44 + Offset.Y,
			BorderWH.X,
			BorderWH.Y
		}; // 大矩形框的左上角坐标X坐标Y，宽度，高度
		DSurface::Composite->DrawRect(&pipbrdRect, CurrentColor1); // 绘制生命条外框，即周圈大矩形框

		// 绘制浅色和深色小格子
		for (int i = 0; i < iPipsTotal; i++)
		{
			Point2D vPipsNW = {
				pip2left + SpaceX + pipWidth * i + Offset.X,
				44 + SpaceY + Offset.Y
			};
			RectangleStruct vPipRect = { vPipsNW.X, vPipsNW.Y, pipWidthL, pipHeight };
			DSurface::Composite->FillRect(&vPipRect, CurrentColor1);

			Point2D vPipsNWR = {
				pip2left + SpaceX + pipWidthL + pipWidth * i + Offset.X,
				44 + SpaceY + Offset.Y
			};
			RectangleStruct vPipRectR = { vPipsNWR.X, vPipsNWR.Y, pipWidthR, pipHeight };
			DSurface::Composite->FillRect(&vPipRectR, CurrentColor2);
		}

	}

	bool UseSHPValue = RulesExt::Global()->HugeHP_UseSHPShowValue.Get();
	if (UseSHPValue) DrawHugeHPValue_SHP(CurrentValue, MaxValue, State);
	else DrawHugeHPValue_Text(CurrentValue, MaxValue, State);
}

void TechnoExt::DrawHugeHPValue_Text(int CurrentValue, int MaxValue, HealthState State)
{
	// 初始化颜色和数值字符
	wchar_t vText1[0x20];
	wchar_t vText2[0x20];
	swprintf_s(vText1, L"%d ", CurrentValue);
	swprintf_s(vText2, L"/ %d", MaxValue);
	// 巨型生命条下方，固定位置的生命数值
	RectangleStruct vRectS = { 0, 0, 0, 0 };

	Vector2D<int> Offset = RulesExt::Global()->HugeHP_ShowValueOffset.Get();

	Point2D vPosTextTopMid = {
		DSurface::Composite->GetWidth() / 2 + Offset.X,
		25 + Offset.Y
	}; // 带斜杠的右半部分的文本框左上角的坐标

	Vector3D<int> ColorVector;
	COLORREF Color;

	if (State == HealthState::Green)
		ColorVector = RulesExt::Global()->HugeHP_HighValueColor;
	else if (State == HealthState::Yellow)
		ColorVector = RulesExt::Global()->HugeHP_MidValueColor;
	else
		ColorVector = RulesExt::Global()->HugeHP_LowValueColor;

	Color = Drawing::RGB2DWORD(ColorVector.X, ColorVector.Y, ColorVector.Z);

	// 左部分是当前生命数值，文本右对齐
	auto TextFlagsL = TextPrintType(int(TextPrintType::UseGradPal | TextPrintType::Metal12 | TextPrintType::Right));
	DSurface::Composite->GetRect(&vRectS);
	DSurface::Composite->DrawText(vText1, &vRectS, &vPosTextTopMid, Color, 0, TextFlagsL);

	DSurface::Composite->DrawText(vText1, &vRectS, &vPosTextTopMid, Color, 0, TextFlagsL);

	// 右部分是斜杠和生命数值上限，文本默认左对齐
	auto TextFlagsR = TextPrintType(int(TextPrintType::UseGradPal | TextPrintType::Metal12));

	DSurface::Composite->DrawText(vText2, &vRectS, &vPosTextTopMid, Color, 0, TextFlagsR);
}

void TechnoExt::DrawHugeHPValue_SHP(int CurrentValue, int MaxValue, HealthState State)
{
	int Interval = RulesExt::Global()->HugeHP_SHPNumberInterval.Get();
	int Width = RulesExt::Global()->HugeHP_SHPNumberWidth.Get();
	int TotalLength = Interval + Width;
	Vector2D<int> Offset = RulesExt::Global()->HugeHP_ShowValueOffset.Get();
	Point2D vPosTextTopMid = {
		DSurface::Composite->GetWidth() / 2 - 3 * TotalLength + Width / 2 + Offset.X,
		10 + Offset.Y
	};
	int base = 0;
	if (State == HealthState::Yellow) base = 10;
	if (State == HealthState::Red) base = 20;

	SHPStruct* NumberSHP = RulesExt::Global()->SHP_HugeHP;
	ConvertClass* NumberPAL = RulesExt::Global()->PAL_HugeHP;
	if (NumberSHP == nullptr || NumberPAL == nullptr) return;

	DynamicVectorClass<char> CurrentValueVector = IntToVector(CurrentValue);
	DynamicVectorClass<char> MaxValueVector = IntToVector(MaxValue);
	Point2D vPosCur = vPosTextTopMid;
	vPosCur.X -= TotalLength * CurrentValueVector.Count + Width / 2;
	for (int i = CurrentValueVector.Count - 1; i >= 0; i--)
	{
		int num = base + CurrentValueVector.GetItem(i);
		DSurface::Composite->DrawSHP(NumberPAL, NumberSHP, num, &vPosCur, &DSurface::ViewBounds,
			BlitterFlags::None, 0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
		vPosCur.X += TotalLength;
	}
	int frame = 30;
	if (base == 10) frame = 31;
	else if (base == 20) frame = 32;
	Point2D vPosMax = vPosTextTopMid;
	vPosMax.X -= Width / 2;
	DSurface::Composite->DrawSHP(NumberPAL, NumberSHP, frame, &vPosMax, &DSurface::ViewBounds,
			BlitterFlags::None, 0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
	vPosMax.X += TotalLength;
	for (int i = MaxValueVector.Count - 1; i >= 0; i--)
	{
		int num = base + MaxValueVector.GetItem(i);
		DSurface::Composite->DrawSHP(NumberPAL, NumberSHP, num, &vPosMax, &DSurface::ViewBounds,
			BlitterFlags::None, 0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
		vPosMax.X += TotalLength;
	}
}

void TechnoExt::DrawHugeSP(TechnoClass* pThis)
{
	auto pTypeThis = pThis->GetTechnoType();
	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pTypeThis);
	auto pData = TechnoExt::ExtMap.Find(pThis);
	const auto pExt = TechnoExt::ExtMap.Find(pThis);

	if (!(pTypeThis && pTypeExt && pData && pExt)) return;

	const auto pShield = pExt->Shield.get();

	if (pShield == nullptr) return;

	if (!pTypeExt->HugeHP_Show.Get()) return;

	int CurrentValue = pShield->GetHP();
	int MaxValue = pShield->GetType()->Strength.Get();

	HealthState State;
	Vector3D<int> Color1Vector;
	Vector3D<int> Color2Vector;
	if (pShield->IsGreenSP())
	{
		Color1Vector = RulesExt::Global()->HugeSP_HighColor1.Get();
		Color2Vector = RulesExt::Global()->HugeSP_HighColor2.Get();
		State = HealthState::Green;
	}
	else if (pShield->IsYellowSP())
	{
		Color1Vector = RulesExt::Global()->HugeSP_MidColor1.Get();
		Color2Vector = RulesExt::Global()->HugeSP_MidColor2.Get();
		State = HealthState::Yellow;
	}
	else
	{
		Color1Vector = RulesExt::Global()->HugeSP_LowColor1.Get();
		Color2Vector = RulesExt::Global()->HugeSP_LowColor2.Get();
		State = HealthState::Red;
	}

	int spBarFrameEmpty = RulesExt::Global()->HugeSP_BarFrameEmpty.Get();
	if (CurrentValue > 0 || spBarFrameEmpty >= 0)
	{
		// 巨型护盾条
		if (RulesExt::Global()->HugeSP_CustomSHPShowBar.Get()) // 激活自定义SHP血条，关闭需单独定义格子的SHP血条，关闭矩形血条
		{
			SHPStruct* CustomSHP = RulesExt::Global()->SHP_HugeSPCustom;
			ConvertClass* CustomPAL = RulesExt::Global()->PAL_HugeSPCustom;
			if (CustomSHP == nullptr || CustomPAL == nullptr)
				return;

			// 读取自定义SHP文件信息
			int frames = CustomSHP->Frames;

			// 当前帧
			int currentFrameIndex = int((double)pShield->GetHP() / (double)pShield->GetType()->Strength.Get() * frames) - 1;
			if (currentFrameIndex < 0)
				currentFrameIndex = 0;
			if (currentFrameIndex > frames - 1)
				currentFrameIndex = frames - 1;

			// 自定义SHP的左上角绘制位置
			Point2D posBarNW = {
				DSurface::Composite->GetWidth() / 2 - CustomSHP->Width / 2 ,
				120
			};

			// 读取整体位置offset
			Vector2D<int> offset = RulesExt::Global()->HugeSP_ShowOffset.Get();
			posBarNW += offset;

			// 绘制自定义血条，包含框和格子
			DSurface::Composite->DrawSHP(CustomPAL, CustomSHP, currentFrameIndex, &posBarNW, &DSurface::ViewBounds,
				BlitterFlags::None, 0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);

		}
		else if (RulesExt::Global()->HugeHP_UseSHPShowBar.Get()) // 激活SHP巨型血条、护盾条，关闭矩形巨型血条、护盾条
		{
			SHPStruct* PipsSHP = RulesExt::Global()->SHP_HugeHPPips;
			ConvertClass* PipsPAL = RulesExt::Global()->PAL_HugeHPPips;
			SHPStruct* BarSHP = RulesExt::Global()->SHP_HugeHPBar;
			ConvertClass* BarPAL = RulesExt::Global()->PAL_HugeHPBar;
			if (PipsSHP == nullptr || PipsPAL == nullptr || BarSHP == nullptr || BarPAL == nullptr) return;

			// 读取格子和框的SHP文件尺寸
			int pipWidth = RulesExt::Global()->HugeSP_PipWidth.Get(PipsSHP->Width); //new
			int pipHeight = PipsSHP->Height;
			int barWidth = BarSHP->Width;
			int barHeight = BarSHP->Height;

			// 满血时的格子数量
			int pipsCount = RulesExt::Global()->HugeSP_PipsCount.Get(100); //new

			int iPipsTotal = int((double)pShield->GetHP() / (double)pShield->GetType()->Strength.Get() * pipsCount);
			if (iPipsTotal < 0)
				iPipsTotal = 0;
			if (iPipsTotal > pipsCount)
				iPipsTotal = pipsCount;

			// 获取当前状态的格子帧序号
			Vector3D<int> framesStruct = RulesExt::Global()->HugeSP_PipsFrames.Get();
			if (framesStruct.X == -1 || framesStruct.Y == -1 || framesStruct.Z == -1)
			{
				// 格子文件检测帧数，1帧则选第1帧，2帧则选第2帧，6帧则选第4、5、6帧，3-5帧则选最后一帧
				if (PipsSHP->Frames > 5)
					framesStruct = { 3, 4, 5 };
				else if (PipsSHP->Frames > 1)
					framesStruct = { PipsSHP->Frames - 1, PipsSHP->Frames - 1, PipsSHP->Frames - 1 };
				else
					framesStruct = { 0, 0, 0 };
			}
			int pipsFrame = framesStruct.X;
			if (State == HealthState::Yellow) pipsFrame = framesStruct.Y;
			if (State == HealthState::Red) pipsFrame = framesStruct.Z;

			// 获取当前状态的框帧序号
			framesStruct = RulesExt::Global()->HugeSP_BarFrames.Get();
			if (framesStruct.X == -1 || framesStruct.Y == -1 || framesStruct.Z == -1)
			{
				// 框文件检测帧数，1帧则选第1帧，2帧则选第2帧，6帧则选第4、5、6帧，3-5帧则选最后一帧
				if (BarSHP->Frames > 5)
					framesStruct = { 3, 4, 5 };
				else if (BarSHP->Frames > 1)
					framesStruct = { BarSHP->Frames - 1, BarSHP->Frames - 1, BarSHP->Frames - 1 };
				else
					framesStruct = { 0, 0, 0 };
			}
			int barFrame = framesStruct.X;
			if (State == HealthState::Yellow) barFrame = framesStruct.Y;
			if (State == HealthState::Red) barFrame = framesStruct.Z;
			if (CurrentValue <= 0) barFrame = spBarFrameEmpty;

			// 格子的左上角绘制位置
			Point2D posPipNW = {
				DSurface::Composite->GetWidth() / 2 - pipWidth * pipsCount / 2 ,
				220
			};
			// 框的左上角绘制位置
			Point2D posBarNW = {
				DSurface::Composite->GetWidth() / 2 - barWidth / 2 ,
				220 - (barHeight - pipHeight) / 2
			};

			// 读取整体位置offset，同时影响框和格子，因此无法改变框和格子的相对位置
			Vector2D<int> offset = RulesExt::Global()->HugeSP_ShowOffset.Get();
			posPipNW += offset;
			posBarNW += offset;
			// 读取格子整体位置offset
			Vector2D<int> offsetPips = RulesExt::Global()->HugeSP_PipsOffset.Get(); //new
			posPipNW += offsetPips;

			// 每个格子相对于前一个格子的实际XY偏移量
			Vector2D<int> realPipOffset = { pipWidth, 0 };
			realPipOffset += RulesExt::Global()->HugeSP_PipToPipOffset.Get(); //new

			// 绘制框
			DSurface::Composite->DrawSHP(BarPAL, BarSHP, barFrame, &posBarNW, &DSurface::ViewBounds,
			BlitterFlags::None, 0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
			// 绘制格子
			for (int i = 0; i < iPipsTotal; i++)
			{
				DSurface::Composite->DrawSHP(PipsPAL, PipsSHP, pipsFrame, &posPipNW, &DSurface::ViewBounds,
				BlitterFlags::None, 0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);

				posPipNW += realPipOffset;
			}
			if (RulesExt::Global()->HugeHP_DrawOrderReverse.Get())
			{
				// 再次绘制框
				DSurface::Composite->DrawSHP(BarPAL, BarSHP, barFrame, &posBarNW, &DSurface::ViewBounds,
				BlitterFlags::None, 0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
			}

		}
		else
		{
			COLORREF CurrentColor1 = Drawing::RGB2DWORD(Color1Vector.X, Color1Vector.Y, Color1Vector.Z);
			COLORREF CurrentColor2 = Drawing::RGB2DWORD(Color2Vector.X, Color2Vector.Y, Color2Vector.Z);

			// 计算生命格子数

			Vector2D<int> RectWH = RulesExt::Global()->HugeSP_RectWH.Get({ DSurface::Composite->GetWidth() * 87 / 100 / 100, 25 });
			int RectCount = RulesExt::Global()->HugeSP_RectCount.Get(100);

			Vector2D<int> Offset = RulesExt::Global()->HugeSP_ShowOffset.Get();
			Vector2D<int> BorderWH = RulesExt::Global()->HugeSP_BorderWH.Get({ DSurface::Composite->GetWidth() * 87 / 100 / 100 * 100 + 12, 60 });

			int pipWidth = RectWH.X;
			int pipHeight = RectWH.Y;
			int pipsTotalWidth = pipWidth * RectCount;
			int pipWidthL = pipWidth * 7 / 10; // 浅色占大部分
			int pipWidthR = pipWidth - pipWidthL; // 右部分深色，模拟生命格子效果而非连续效果
			int SpaceX = (BorderWH.X - pipsTotalWidth) / 2;
			int SpaceY = (BorderWH.Y - pipHeight) / 2;
			int pip2left = DSurface::Composite->GetWidth() / 2 - pipsTotalWidth / 2 - SpaceX; // 屏幕左边缘与生命小格子的最小距离
			int iPipsTotal = int(double(CurrentValue) / MaxValue * RectCount);

			RectangleStruct pipbrdRect = {
				pip2left + Offset.X,
				44 + Offset.Y,
				BorderWH.X,
				BorderWH.Y
			}; // 大矩形框的左上角坐标X坐标Y，宽度，高度
			DSurface::Composite->DrawRect(&pipbrdRect, CurrentColor1); // 绘制生命条外框，即周圈大矩形框

			// 绘制浅色和深色小格子
			for (int i = 0; i < iPipsTotal; i++)
			{
				Point2D vPipsNW = {
					pip2left + SpaceX + pipWidth * i + Offset.X,
					44 + SpaceY + Offset.Y
				};
				RectangleStruct vPipRect = { vPipsNW.X, vPipsNW.Y, pipWidthL, pipHeight };
				DSurface::Composite->FillRect(&vPipRect, CurrentColor1);

				Point2D vPipsNWR = {
					pip2left + SpaceX + pipWidthL + pipWidth * i + Offset.X,
					44 + SpaceY + Offset.Y
				};
				RectangleStruct vPipRectR = { vPipsNWR.X, vPipsNWR.Y, pipWidthR, pipHeight };
				DSurface::Composite->FillRect(&vPipRectR, CurrentColor2);
			}
		}
	}

	if (CurrentValue > 0 || RulesExt::Global()->HugeSP_ShowValueAlways.Get())
	{
		bool UseSHPValue = RulesExt::Global()->HugeSP_UseSHPShowValue.Get();
		if (UseSHPValue) DrawHugeSPValue_SHP(CurrentValue, MaxValue, State);
		else DrawHugeSPValue_Text(CurrentValue, MaxValue, State);
	}
}

void TechnoExt::DrawHugeSPValue_Text(int CurrentValue, int MaxValue, HealthState State)
{
	wchar_t vText1[0x20];
	wchar_t vText2[0x20];
	swprintf_s(vText1, L"%d ", CurrentValue);
	swprintf_s(vText2, L"/ %d", MaxValue);
	// 巨型生命条下方，固定位置的生命数值
	RectangleStruct vRectS = { 0, 0, 0, 0 };

	Vector2D<int> Offset = RulesExt::Global()->HugeSP_ShowValueOffset.Get();

	Point2D vPosTextTopMid = {
		DSurface::Composite->GetWidth() / 2 + Offset.X,
		10 + Offset.Y
	}; // 带斜杠的右半部分的文本框左上角的坐标

	Vector3D<int> ColorVector;
	COLORREF Color;

	if (State == HealthState::Green)
		ColorVector = RulesExt::Global()->HugeSP_HighValueColor;
	else if (State == HealthState::Yellow)
		ColorVector = RulesExt::Global()->HugeSP_MidValueColor;
	else
		ColorVector = RulesExt::Global()->HugeSP_LowValueColor;

	Color = Drawing::RGB2DWORD(ColorVector.X, ColorVector.Y, ColorVector.Z);

	// 左部分是当前生命数值，文本右对齐
	auto TextFlagsL = TextPrintType(int(TextPrintType::UseGradPal | TextPrintType::Metal12 | TextPrintType::Right));
	DSurface::Composite->GetRect(&vRectS);
	DSurface::Composite->DrawText(vText1, &vRectS, &vPosTextTopMid, Color, 0, TextFlagsL);

	DSurface::Composite->DrawText(vText1, &vRectS, &vPosTextTopMid, Color, 0, TextFlagsL);

	// 右部分是斜杠和生命数值上限，文本默认左对齐
	auto TextFlagsR = TextPrintType(int(TextPrintType::UseGradPal | TextPrintType::Metal12));
	DSurface::Composite->DrawText(vText2, &vRectS, &vPosTextTopMid, Color, 0, TextFlagsR);
}

void TechnoExt::DrawHugeSPValue_SHP(int CurrentValue, int MaxValue, HealthState State)
{
	int Interval = RulesExt::Global()->HugeSP_SHPNumberInterval.Get();
	int Width = RulesExt::Global()->HugeSP_SHPNumberWidth.Get();
	int TotalLength = Interval + Width;
	Vector2D<int> Offset = RulesExt::Global()->HugeSP_ShowValueOffset.Get();
	Point2D vPosTextTopMid = {
		DSurface::Composite->GetWidth() / 2 - 3 * TotalLength + Width / 2 + Offset.X,
		-5 + Offset.Y
	};
	int base = 0;
	if (State == HealthState::Yellow) base = 10;
	if (State == HealthState::Red) base = 20;

	char FilenameSHP[0x20];
	strcpy_s(FilenameSHP, RulesExt::Global()->HugeSP_ShowValueSHP.data());
	char FilenamePAL[0x20];
	strcpy_s(FilenamePAL, RulesExt::Global()->HugeSP_ShowValuePAL.data());

	SHPStruct* NumberSHP = RulesExt::Global()->SHP_HugeSP;
	ConvertClass* NumberPAL = RulesExt::Global()->PAL_HugeSP;
	if (NumberSHP == nullptr || NumberPAL == nullptr) return;

	DynamicVectorClass<char> CurrentValueVector = IntToVector(CurrentValue);
	DynamicVectorClass<char> MaxValueVector = IntToVector(MaxValue);
	Point2D vPosCur = vPosTextTopMid;
	vPosCur.X -= TotalLength * CurrentValueVector.Count + Width / 2;
	for (int i = CurrentValueVector.Count - 1; i >= 0; i--)
	{
		int num = base + CurrentValueVector.GetItem(i);
		DSurface::Composite->DrawSHP(NumberPAL, NumberSHP, num, &vPosCur, &DSurface::ViewBounds,
			BlitterFlags::None, 0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
		vPosCur.X += TotalLength;
	}
	int frame = 30;
	if (base == 10) frame = 31;
	else if (base == 20) frame = 32;
	Point2D vPosMax = vPosTextTopMid;
	vPosMax.X -= Width / 2;
	DSurface::Composite->DrawSHP(NumberPAL, NumberSHP, frame, &vPosMax, &DSurface::ViewBounds,
			BlitterFlags::None, 0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
	vPosMax.X += TotalLength;
	for (int i = MaxValueVector.Count - 1; i >= 0; i--)
	{
		int num = base + MaxValueVector.GetItem(i);
		DSurface::Composite->DrawSHP(NumberPAL, NumberSHP, num, &vPosMax, &DSurface::ViewBounds,
			BlitterFlags::None, 0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
		vPosMax.X += TotalLength;
	}
}

void TechnoExt::AddFireScript(TechnoClass* pThis)
{
	auto pType = pThis->GetTechnoType();
	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
	if (strcmp(pTypeExt->Script_Fire.data(), "") == 0) return;
	auto pExt = TechnoExt::ExtMap.Find(pThis);
	if (pTypeExt->FireScriptType == nullptr) pTypeExt->FireScriptType = FireScriptTypeClass::GetScript(pTypeExt->Script_Fire.data());
	if (pTypeExt->FireScriptType == nullptr) return;
	CoordStruct Loc;
	if (pThis->Target->WhatAmI() == AbstractType::Cell)
		Loc = abstract_cast<CellClass*>(pThis->Target)->GetCenterCoords();
	else
		Loc = abstract_cast<ObjectClass*>(pThis->Target)->Location;
	auto pScript = new FireScriptClass(pTypeExt->FireScriptType, pThis, Loc);
	pExt->Processing_Scripts.emplace_back(pScript);
}

void TechnoExt::UpdateFireScript(TechnoClass* pThis, TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt)
{
	std::set<FireScriptClass*> Need_Delete;
	for (auto& it : pExt->Processing_Scripts)
	{
		if (it->CurrentLine >= (int)it->Type->ScriptLines.size()) Need_Delete.emplace(it);
		else it->ProcessScript(pTypeExt->Script_Fire_SelfCenter.Get());
	}
	for (auto& it : Need_Delete)
	{
		auto itv = find(pExt->Processing_Scripts.begin(), pExt->Processing_Scripts.end(), it);
		pExt->Processing_Scripts.erase(itv);
		delete it;
	}
}

void TechnoExt::RunBlinkWeapon(TechnoClass* pThis, AbstractClass* pTarget, WeaponTypeClass* pWeapon)
{
	if (pTarget->WhatAmI() != AbstractType::Unit && pTarget->WhatAmI() != AbstractType::Aircraft &&
		pTarget->WhatAmI() != AbstractType::Building && pTarget->WhatAmI() != AbstractType::Infantry)
		return;

	TechnoClass* pTargetTechno = abstract_cast<TechnoClass*>(pTarget);

	auto pType = pThis->GetTechnoType();
	auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);

	CoordStruct PreSelfLocation = pThis->Location;
	CoordStruct PreTargetLocation = pTargetTechno->Location;

	if (pTarget->WhatAmI() == AbstractType::Building)
	{
		auto const pTargetBuilding = abstract_cast<BuildingClass*>(pTargetTechno);
		int FoundationX = pTargetBuilding->GetFoundationData()->X, FoundationY = pTargetBuilding->GetFoundationData()->Y;
		
		if (FoundationX > 0)
			FoundationX = 1;

		if (FoundationY > 0)
			FoundationY = 1;
		
		PreTargetLocation += CoordStruct { (FoundationX * 256) / 2, (FoundationY * 256) / 2 };
	}

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
		pFoot->SetLocation(location);
		CellStruct targetcell = CellClass::Coord2Cell(location);
		pFoot->MarkAllOccupationBits(location);
		//pFoot->Locomotor->Clear_Coords();
		pFoot->Locomotor->Force_Track(-1, location);
		pFoot->MarkAllOccupationBits(location);
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
	CoordStruct crdAbs = pThis->GetCoords();
	Point2D  posScreen = { 0,0 };
	TacticalClass::Instance->CoordsToScreen(&posScreen, &crdAbs);
	posScreen -= TacticalClass::Instance->TacticalPos;
	return posScreen;
}

Point2D TechnoExt::GetHealthBarPosition(TechnoClass* pThis, bool Shield, HealthBarAnchors Anchor)
{
	Point2D posScreen = GetScreenLocation(pThis);
	Point2D posResult = { 0, 0 };
	int iLength = pThis->WhatAmI() == AbstractType::Infantry ? 8 : 17;
	TechnoTypeClass* pType = pThis->GetTechnoType();
	if (pThis->WhatAmI() == AbstractType::Building)
	{
		BuildingTypeClass* pBuildingType = abstract_cast<BuildingTypeClass*>(pThis->GetTechnoType());
		CoordStruct crdDim2 = { 0, 0, 0 };
		pBuildingType->Dimension2(&crdDim2);
		Point2D posFix = { 0, 0 };
		CoordStruct crdTmp = { -crdDim2.X / 2, crdDim2.Y / 2, crdDim2.Z };
		TacticalClass::Instance->CoordsToScreen(&posFix, &crdTmp);
		int iFoundationHeight = pBuildingType->GetFoundationHeight(false);
		iLength = iFoundationHeight * 7 + iFoundationHeight / 2;
		posResult.X = posFix.X + posScreen.X + 4 * iLength + 3 - (Shield ? 6 : 0);
		posResult.Y = posFix.Y + posScreen.Y - 2 * iLength + 4 - (Shield ? 3 : 0);

		if (Anchor & HealthBarAnchors::Center)
		{
			posResult.X -= iLength * 2 + 2;
			posResult.Y += iLength + 1;
		}
		else
		{
			if (!(Anchor & HealthBarAnchors::Right))
			{
				posResult.X -= (iLength + 1) * 4;
				posResult.Y += (iLength + 1) * 2;
			}
		}

		if (!(Anchor & HealthBarAnchors::Bottom))
		{
			posResult.Y += 4;
			posResult.X += 4;
		}
	}
	else
	{
		posResult.X = posScreen.X - iLength + (iLength == 8);
		posResult.Y = posScreen.Y - 28 + (iLength == 8);
		posResult.Y += pType->PixelSelectionBracketDelta;

		if (Shield)
		{
			posResult.Y -= 5;

			auto pExt = ExtMap.Find(pThis);

			if (pExt->Shield != nullptr && !pExt->Shield->IsBrokenAndNonRespawning())
				posResult.Y += pExt->Shield->GetType()->BracketDelta.Get();
		}

		if (Anchor & HealthBarAnchors::Center)
		{
			posResult.X += iLength;
		}
		else
		{
			if (Anchor & HealthBarAnchors::Right)
				posResult.X += iLength * 2;
		}

		if (Anchor & HealthBarAnchors::Bottom)
			posResult.Y += 4;
	}

	return posResult;
}

void TechnoExt::ProcessDigitalDisplays(TechnoClass* pThis)
{
	if (!Phobos::Config::DigitalDisplay_Enable)
		return;

	Point2D posLoc = GetScreenLocation(pThis);
	TechnoExt::ExtData* pExt = TechnoExt::ExtMap.Find(pThis);
	TechnoTypeExt::ExtData* pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	ValueableVector<DigitalDisplayTypeClass*>* pDefaultTypes = nullptr;
	AbstractType thisAbsType = pThis->WhatAmI();
	int iLength = 17;

	switch (thisAbsType)
	{
	case AbstractType::Building:
	{
		pDefaultTypes = &RulesExt::Global()->Buildings_DefaultDigitalDisplayTypes;
		BuildingTypeClass* pBuildingType = static_cast<BuildingTypeClass*>(pThis->GetTechnoType());
		int iFoundationHeight = pBuildingType->GetFoundationHeight(false);
		iLength = iFoundationHeight * 7 + iFoundationHeight / 2;
	}break;
	case AbstractType::Infantry:
	{
		pDefaultTypes = &RulesExt::Global()->Infantrys_DefaultDigitalDisplayTypes;
		iLength = 8;
	}break;
	case AbstractType::Unit:
	{
		pDefaultTypes = &RulesExt::Global()->Units_DefaultDigitalDisplayTypes;
	}break;
	case AbstractType::Aircraft:
	{
		pDefaultTypes = &RulesExt::Global()->Aircrafts_DefaultDigitalDisplayTypes;
	}break;
	default:
		return;
	}

	ValueableVector<DigitalDisplayTypeClass*>* pDisplayTypes = pTypeExt->DigitalDisplayTypes.empty() ? pDefaultTypes : &pTypeExt->DigitalDisplayTypes;

	for (DigitalDisplayTypeClass*& pDisplayType : *pDisplayTypes)
	{
		int iCur = -1;
		int iMax = -1;
		bool isBuilding = thisAbsType == AbstractType::Building;
		bool bHasShield = pExt->Shield != nullptr && !pExt->Shield->IsBrokenAndNonRespawning();
		bool bDiplayShield = pDisplayType->InfoType == DisplayInfoType::Shield;
		HealthBarAnchors Anchor =
			pDisplayType->Anchoring == DigitalDisplayTypeClass::AnchorType::Right
			|| pDisplayType->Anchoring == DigitalDisplayTypeClass::AnchorType::TopRight
			? HealthBarAnchors::TopRight : HealthBarAnchors::TopLeft;
		Point2D posDraw = TechnoExt::GetHealthBarPosition(pThis, bDiplayShield, Anchor);

		GetValuesForDisplay(pThis, pDisplayType->InfoType, iCur, iMax);

		if (iCur == -1 || iMax == -1)
			continue;

		pDisplayType->Draw(posDraw, iLength, iCur, iMax, isBuilding, bHasShield);
	}
}

void TechnoExt::GetValuesForDisplay(TechnoClass* pThis, DisplayInfoType infoType, int& iCur, int& iMax)
{
	TechnoTypeClass* pType = pThis->GetTechnoType();
	TechnoExt::ExtData* pExt = TechnoExt::ExtMap.Find(pThis);

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
		iCur = pThis->SpawnManager->SpawnCount;
		iMax = pType->SpawnsNumber;
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

void TechnoExt::AttackedWeaponTimer(TechnoExt::ExtData* pExt)
{
	ValueableVector<int>& vTimer = pExt->AttackedWeapon_Timer;

	for (int& iTime : vTimer)
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
	ValueableVector<int>& vTimer = pExt->AttackedWeapon_Timer;

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

		if (pWeapon == nullptr || bIsInROF || bResponseZeroDamage && !bBeforeDamageCheck
			|| !bResponseZeroDamage && (bBeforeDamageCheck || *args->Damage == 0) || pThis->Health<iMinHP || pThis->Health>iMaxHP)
			continue;

		bool bFireToAttacker = i < vFireToAttacker.size() ? vFireToAttacker[i] : false;
		bool bIgnoreRange = i < vIgnoreRange.size() ? vIgnoreRange[i] : false;
		AffectedHouse affectedHouse = vAffectHouse[i];
		int iRange = i < vRange.size() ? vRange[i] : pWeapon->Range;
		CoordStruct crdFLH = vFLH[i];

		//Debug::Log("[AttackedWeapon] bIgnoreROF[%d],iTime[%d],bIsInROF[%d],bResponseZeroDamage[%d],Damage[%d]\n",
		//	bIgnoreROF, iTime, bIsInROF, bResponseZeroDamage, *args->Damage);

		//Debug::Log("[AttackedWeapon] ROF Pass\n");

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

void TechnoExt::PassengerFixed(TechnoClass* pThis, TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt)
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

void TechnoExt::InitialPayloadFixed(TechnoClass* pThis, TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt)
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

BulletClass* TechnoExt::SimulatedFire(TechnoClass* pThis, WeaponStruct& weaponStruct, AbstractClass* pTarget)
{
	TechnoClass* pStand = PhobosGlobal::Global()->GetGenericStand();
	WeaponTypeClass* pWeapon = weaponStruct.WeaponType;

	if (pWeapon == nullptr)
		return nullptr;

	WarheadTypeClass* pWH = pWeapon->Warhead;

	if (pWH == nullptr)
		Debug::FatalErrorAndExit("Weapon [%s] has no warhead!\n", pWeapon->get_ID());

	if(pWeapon->Projectile==nullptr)
		Debug::FatalErrorAndExit("Weapon [%s] has no projectile!\n", pWeapon->get_ID());

	if (pWH->MindControl || pWH->Temporal || pWH->Parasite || pWeapon->DrainWeapon)
		return nullptr;

	TechnoTypeClass* pType = pStand->GetTechnoType();
	WeaponStruct& weaponCur = pType->GetWeapon(0, pStand->Veterancy.IsElite());
	WeaponStruct weaponOrigin = pType->GetWeapon(0, pStand->Veterancy.IsElite());
	bool bOmniFire = pWeapon->OmniFire;
	pWeapon->OmniFire = true;
	weaponCur = weaponStruct;
	pStand->SetLocation(pThis->GetCoords());

	BulletClass* pBullet = pStand->TechnoClass::Fire(pTarget, 0);

	if (pBullet != nullptr)
		pBullet->Owner = pThis;
	
	weaponCur = weaponOrigin;
	pWeapon->OmniFire = bOmniFire;

	return pBullet;
	//WeaponTypeClass* pWeapon = weaponStruct.WeaponType;
	//
	//if (pThis == nullptr || pWeapon == nullptr || pWeapon->Warhead == nullptr || pWeapon->Projectile == nullptr)
	//	return nullptr;

	//WeaponTypeExt::ExtData* pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);
	//BulletTypeClass* pBulletType = pWeapon->Projectile;
	//BulletTypeExt::ExtData* pBulletTypeExt = BulletTypeExt::ExtMap.Find(pBulletType);
	//ObjectClass* pObject = abstract_cast<ObjectClass*>(pTarget);
	//TechnoClass* pTechno = abstract_cast<TechnoClass*>(pTarget);
	//TechnoTypeClass* pTechnoType = pTechno != nullptr ? pTechno->GetTechnoType() : nullptr;
	//HouseClass* pOwner = pThis->GetOwningHouse();
	//HouseClass* pTargetOwner = pObject != nullptr ? pObject->GetOwningHouse() : nullptr;
	//TechnoTypeClass* pType = pThis->GetTechnoType();
	//TechnoTypeExt::ExtData* pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
	//CoordStruct crdThis = pThis->GetCoords();
	//CellClass* pCell = MapClass::Instance->GetCellAt(crdThis);
	//CoordStruct crdCell = pCell != nullptr ? pCell->GetCoords() : CoordStruct::Empty;
	//DirStruct dirThis = pThis->GetRealFacing();
	//WarheadTypeClass* pWH = pWeapon->Warhead;
	//bool bVeteran = pThis->Veterancy.IsVeteran();
	//bool bElite = pThis->Veterancy.IsElite();

	//if (Unsorted::ArmageddonMode || pTarget == nullptr || pObject != nullptr && pObject->InLimbo)
	//	return nullptr;

	////Suicide
	//if (pWeapon->Suicide)
	//{
	//	int iDamage = pType->Strength;
	//	pThis->ReceiveDamage(&iDamage, 0, RulesClass::Instance->C4Warhead, nullptr, true, false, nullptr);
	//	return nullptr;
	//}

	////unique
	//if (pWeapon->UseFireParticles && pThis->FireParticleSystem != nullptr
	//|| pWeapon->IsRailgun && pThis->RailgunParticleSystem != nullptr
	//|| pWeapon->UseSparkParticles && pThis->SparkParticleSystem != nullptr
	//|| pWeapon->IsSonic && pThis->Wave != nullptr)
	//{
	//	return nullptr;
	//}

	////Spawner
	//if (pWeapon->Spawner)
	//{
	//	pThis->SpawnManager->SetTarget(pTarget);

	//	if (pThis->IsHumanControlled || pThis->DiscoveredByPlayer)
	//	{
	//		if (!MapClass::Instance->IsLocationShrouded(crdThis))
	//		{
	//			if (!MapClass::Instance->IsLocationFogged(crdThis))
	//				return nullptr;
	//		}

	//		if (pThis->WhatAmI() == AbstractType::Aircraft && pThis->IsHumanControlled)
	//			return nullptr;
	//	}

	//	if (pObject == nullptr)
	//		return nullptr;

	//	if (pTargetOwner != nullptr && pTargetOwner->ControlledByPlayer() && pWeapon->RevealOnFire)
	//	{
	//		MapClass::Instance->RevealArea1(&crdThis, 3, pTargetOwner, CellStruct::Empty, false, false, true, false);
	//	}

	//	MapClass::Instance->RevealArea3(&crdThis, 0, 4, false);

	//	return 0;
	//}

	////Drain
	//if (pWeapon->DrainWeapon)
	//{
	//	if (pTechno == nullptr || !pTechnoType->Drainable)
	//		return nullptr;
	//	
	//	pThis->DrainBuilding(abstract_cast<BuildingClass*>(pTarget));
	//	pThis->SetTarget(nullptr);
	//	return nullptr;
	//}

	////i don't now what are these
	//{
	//	reference<byte, 0xB0EB30> byte_B0EB30;
	//	if ((byte_B0EB30 & 1) == 0)
	//	{
	//		reference<DWORD, 0xB0EAA8, 1> dword_B0EAA8;
	//		reference<DWORD, 0xB0EAAC, 1> dword_B0EAAC;
	//		reference<DWORD, 0xB0EAB4> dword_B0EAB4;
	//		reference<DWORD, 0xB0EAB8> dword_B0EAB8;
	//		reference<DWORD, 0xB0EAC0> dword_B0EAC0;
	//		reference<DWORD, 0xB0EAC4> dword_B0EAC4;
	//		reference<DWORD, 0xB0EACC> dword_B0EACC;
	//		reference<DWORD, 0xB0EAD0> dword_B0EAD0;
	//		reference<DWORD, 0xB0EAD8> dword_B0EAD8;
	//		reference<DWORD, 0xB0EADC> dword_B0EADC;
	//		reference<DWORD, 0xB0EAE4> dword_B0EAE4;
	//		reference<DWORD, 0xB0EAE8> dword_B0EAE8;
	//		reference<DWORD, 0xB0EAF0> dword_B0EAF0;
	//		reference<DWORD, 0xB0EAF4> dword_B0EAF4;
	//		reference<DWORD, 0xB0EAB0, 1> dword_B0EAB0;
	//		reference<DWORD, 0xB0EABC> dword_B0EABC;
	//		reference<DWORD, 0xB0EAC8> dword_B0EAC8;
	//		reference<DWORD, 0xB0EAD4> dword_B0EAD4;
	//		reference<DWORD, 0xB0EAE0> dword_B0EAE0;
	//		reference<DWORD, 0xB0EAEC> dword_B0EAEC;
	//		reference<DWORD, 0xB0EAF8> dword_B0EAF8;
	//		reference<DWORD, 0xB0EAFC> dword_B0EAFC;
	//		reference<DWORD, 0xB0EB00> dword_B0EB00;
	//		reference<DWORD, 0xB0EB04> dword_B0EB04;
	//		dword_B0EAA8[0] = 256;
	//		dword_B0EAAC[0] = 0;
	//		dword_B0EAB4 = 180;
	//		dword_B0EAB8 = 180;
	//		dword_B0EAC0 = 0;
	//		dword_B0EAC4 = 256;
	//		dword_B0EACC = -180;
	//		dword_B0EAD0 = 180;
	//		dword_B0EAD8 = -256;
	//		dword_B0EADC = 0;
	//		dword_B0EAE4 = -180;
	//		dword_B0EAE8 = -180;
	//		byte_B0EB30 |= 1u;
	//		dword_B0EAF0 = 0;
	//		dword_B0EAF4 = -256;
	//		dword_B0EAB0[0] = 0;
	//		dword_B0EABC = 0;
	//		dword_B0EAC8 = 0;
	//		dword_B0EAD4 = 0;
	//		dword_B0EAE0 = 0;
	//		dword_B0EAEC = 0;
	//		dword_B0EAF8 = 0;
	//		dword_B0EAFC = 180;
	//		dword_B0EB00 = -180;
	//		dword_B0EB04 = 0;
	//		atexit(TechnoClass::nullsub_44);
	//	}
	//}

	//CoordStruct crdSpray = CoordStruct::Empty;

	////SprayAttack
	//if (pType->SprayAttack)
	//{
	//	if (pThis->CurrentBurstIndex != 0)
	//		pThis->SprayOffsetIndex = static_cast<signed int>(8 / pWeapon->Burst + pThis->SprayOffsetIndex) % 8;
	//	else
	//		pThis->SprayOffsetIndex = ScenarioClass::Instance->Random.RandomRanged(0, 7);

	//	reference<DWORD, 0xB0EAAC, 22> dword_B0EAAC;
	//	reference<DWORD, 0xB0EAB0, 22> dword_B0EAB0;
	//	reference<DWORD, 0xB0EAA8, 22> dword_B0EAA8;

	//	crdSpray.X = pThis->Location.X + dword_B0EAA8[3 * pThis->SprayOffsetIndex];
	//	crdSpray.Y = pThis->Location.Y + dword_B0EAAC[3 * pThis->SprayOffsetIndex];
	//	crdSpray.Z = pThis->Location.Z + dword_B0EAB0[3 * pThis->SprayOffsetIndex];
	//}
	//else
	//{
	//	if (pWeapon->AreaFire)
	//	{
	//		crdSpray = crdCell;
	//		Point2D posTmp = { crdCell.X,crdCell.Y };
	//		pTarget = MapClass::Instance->GetTargetCell(posTmp);
	//	}
	//	else
	//	{

	//		if (pObject != nullptr)
	//			pObject->GetPosition_0(&crdSpray);
	//		else
	//			crdSpray = pTarget->GetAltCoords();

	//	}
	//}

	//CoordStruct crdFLH = weaponStruct.FLH;

	//if (pType->SprayAttack)
	//{
	//	Point2D posTmp = { crdSpray.X,crdSpray.Y };
	//	pTarget = MapClass::Instance->GetTargetCell(posTmp);
	//}

	//if (pBulletType->ROT != 0 || pBulletType->Dropping)
	//{
	//	
	//}
	//else
	//{
	//	
	//}
	//
	//int iDamage = pWeapon->Damage;

	//if (pWeapon->IsSonic || pWeapon->UseSparkParticles)
	//{
	//	iDamage = 0;
	//}
	//else
	//{
	//	if (iDamage >= 0)
	//	{

	//		iDamage = Game::F2I(pOwner->FirepowerMultiplier * pThis->FirepowerMultiplier * iDamage);

	//		if (bVeteran && pType->VeteranAbilities.FIREPOWER
	//			|| bElite && (pType->VeteranAbilities.FIREPOWER || pType->EliteAbilities.FIREPOWER))
	//			iDamage = Game::F2I(RulesClass::Instance->VeteranCombat * iDamage);
	//	}
	//}
	//
	//if (pThis->CanOccupyFire())
	//	iDamage = Game::F2I(RulesClass::Instance->OccupyDamageMultiplier * iDamage);

	//if (pThis->BunkerLinkedItem && pThis->WhatAmI() != AbstractType::Building)
	//	iDamage = Game::F2I(RulesClass::Instance->BunkerDamageMultiplier * iDamage);

	//if (pThis->InOpenToppedTransport)
	//	iDamage = Game::F2I(pTypeExt->OpenTopped_DamageMultiplier.Get(RulesClass::Instance->OpenToppedDamageMultiplier) * iDamage);

	//if (pWeapon->DiskLaser)
	//{
	//	DiskLaserClass* pDiskLaser = new DiskLaserClass();
	//	
	//	if (pDiskLaser != nullptr)
	//	{
	//		int iROF = GetROF(pThis, pWeapon);
	//		pThis->DiskLaserTimer.StartTime = Unsorted::CurrentFrame;
	//		pThis->DiskLaserTimer.TimeLeft = iROF;
	//		pDiskLaser->Fire(pThis, pTarget, pWeapon, iDamage);
	//		return nullptr;
	//	}
	//}
	//int iDistance = Game::F2I(crdSpray.DistanceFrom(crdThis));
	//pWeapon->sub_773070(iDistance);

	//BulletClass* pBullet = 
	//	pBulletType->CreateBullet
	//	(
	//		pTarget,
	//		pThis,
	//		iDamage,
	//		pWH,
	//		pWeapon->Speed,
	//		pWeapon->Bright
	//	);

	//if (pBullet != nullptr)
	//{
	//	pBullet->SetWeaponType(pWeapon);
	//	pBullet->Limbo();

	//	FootClass* pFoot = abstract_cast<FootClass*>(pThis);
	//	
	//	if (pFoot != nullptr)
	//	{
	//		if (pFoot->Locomotor.get() == nullptr)
	//			Game::RaiseError(-2147467261);

	//		if (pFoot->Locomotor->Is_Moving() && !pType->JumpJet)
	//			pBullet->unknown_B4 = true;
	//	}

	//	if (!pBullet->unknown_B4 && !pBulletType->Inaccurate)
	//	{
	//		if (pTechno != nullptr)
	//		{
	//			pTechno->EstimatedHealth -= pThis->CalculateDamage(pTechno, pWeapon);
	//		}
	//	}

	//	CoordStruct crdUnk1;
	//	pThis->sub_70BCB0(&crdUnk1);
	//	CoordStruct crdUnk2 = crdUnk1 - crdThis;
	//
	//	//too....
	//	if (pBulletType->Inaccurate && pBulletType->Arcing)
	//	{
	//		if (!pBulletType->FlakScatter || pBulletType->Inviso)
	//		{
	//			int iTmp1;
	//			iTmp1 = ScenarioClass::Instance->Random.RandomRanged
	//			(
	//				pBulletTypeExt->BallisticScatter_Min.isset() ? pBulletTypeExt->BallisticScatter_Min.Get().value : RulesClass::Instance->BallisticScatter / 2,
	//				pBulletTypeExt->BallisticScatter_Max.isset() ? pBulletTypeExt->BallisticScatter_Max.Get().value : RulesClass::Instance->BallisticScatter
	//			);
	//			int iTmp2 = ScenarioClass::Instance->Random.RandomRanged(0, 2147483646);
	//			iTmp2 = Game::F2I((iTmp2 * 4.656612877414201e-10 * 6.283185307179586 - 1.570796326794897) * -10430.06004058427) - 0x3FFF;
	//			double v155 = iTmp2 * -0.00009587672516830327;
	//			
	//			sin(iTmp2);
	//			int v58 = crdUnk2.Y - crdUnk2.Y * (*reinterpret_cast<double*>(&iTmp1));
	//			int iTmp3 = Game::F2I(v58);
	//			cos(*reinterpret_cast<double*>((static_cast<long long>(iTmp3) << 32) | ((DWORD)&v155)));
	//			crdUnk2.X = Game::F2I(v58 * (*reinterpret_cast<double*>(iTmp1 + crdUnk2.X)));
	//			crdUnk2.Y = iTmp2;
	//		}
	//		else
	//		{
	//			*(reinterpret_cast<float*>(&))
	//		}
	//	}


	//}
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
					pThis->CaptureManager = nullptr;
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

			if(!bSpawnSet)
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
		.Process(this->Death_Countdown)
		.Process(this->MindControlRingAnimType)
		.Process(this->OriginalPassengerOwner)
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
		.Process(this->DisableTurnCount)
		.Process(this->SelfFacing)
		.Process(this->TurretFacing)
		.Process(this->AllowToPaint)
		.Process(this->ColorToPaint)
		.Process(this->Paint_Count)
		.Process(this->Paint_IsDiscoColor)
		.Process(this->Paint_Colors)
		.Process(this->Paint_TransitionDuration)
		.Process(this->Paint_FramesPassed)
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

		.Process(this->LastLocation)
		.Process(this->MoveDamage_Duration)
		.Process(this->MoveDamage_Count)
		.Process(this->MoveDamage_Delay)
		.Process(this->MoveDamage)
		.Process(this->MoveDamage_Warhead)
		.Process(this->StopDamage_Duration)
		.Process(this->StopDamage_Count)
		.Process(this->StopDamage_Delay)
		.Process(this->StopDamage)
		.Process(this->StopDamage_Warhead)

		.Process(this->IsSharingWeaponRange)
		.Process(this->BeSharedWeaponRange)

		.Process(this->IFVWeapons)
		.Process(this->IFVTurrets)
		;
	for (auto& it : Processing_Scripts) delete it;
	FireSelf_Count.clear();
	FireSelf_Weapon.clear();
	FireSelf_ROF.clear();
	Processing_Scripts.clear();
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
