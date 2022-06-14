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

#include <PhobosHelper/Helper.h>

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
					pThis->SetTarget(pBullet);
					break;
				}
			}
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
	if (!TechnoExt::IsActive(pThis))
		return;

	if (pTypeExt->SilentPassenger.Get())
	{
		if (pThis->Passengers.NumPassengers > 0)
		{
			FootClass* pPassenger = pThis->Passengers.GetFirstPassenger();

			if (pExt->AllowPassengerToFire)
			{
				if (pExt->AllowFireCount)
					pExt->AllowFireCount--;
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
				pPassenger->ForceMission(Mission::Guard);
				pPassenger->Target = pTarget;
				if (auto const pManager = pPassenger->SpawnManager)
				{
					pManager->Target = pTarget;
				}
				pPassenger = static_cast<FootClass*>(pPassenger->NextObject);
			}
		}
	}
}

void TechnoExt::Spawner_SameLoseTarget(TechnoClass* pThis, TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt)
{
	if (!TechnoExt::IsActive(pThis))
		return;

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
	if (pExt->ConvertsCounts > 0)
	{
		pExt->ConvertsCounts--;
	}
	else if(pExt->ConvertsCounts == 0)
	{
		if (pThis->WhatAmI() == AbstractType::Infantry &&
			pExt->ConvertsOriginalType->WhatAmI() == AbstractType::InfantryType)
		{
			abstract_cast<InfantryClass*>(pThis)->Type = static_cast<InfantryTypeClass*>(pExt->ConvertsOriginalType);
			abstract_cast<InfantryClass*>(pThis)->Cloakable = static_cast<InfantryTypeClass*>(pExt->ConvertsOriginalType)->Cloakable;
		}
		else if (pThis->WhatAmI() == AbstractType::Unit &&
			pExt->ConvertsOriginalType->WhatAmI() == AbstractType::UnitType)
		{
			abstract_cast<UnitClass*>(pThis)->Type = static_cast<UnitTypeClass*>(pExt->ConvertsOriginalType);
			abstract_cast<UnitClass*>(pThis)->Cloakable = static_cast<UnitTypeClass*>(pExt->ConvertsOriginalType)->Cloakable;
		}
		else if (pThis->WhatAmI() == AbstractType::Aircraft &&
			pExt->ConvertsOriginalType->WhatAmI() == AbstractType::AircraftType)
		{
			abstract_cast<AircraftClass*>(pThis)->Type = static_cast<AircraftTypeClass*>(pExt->ConvertsOriginalType);
			abstract_cast<AircraftClass*>(pThis)->Cloakable = static_cast<AircraftTypeClass*>(pExt->ConvertsOriginalType)->Cloakable;
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
	//Debug::Log("[LaserTrails] Type[0x%X]\n", pThis->GetTechnoType());
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

	if (!pThis || weaponIndex < 0)
		return FLH;

	auto const pExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	auto pInf = abstract_cast<InfantryClass*>(pThis);
	auto& pickedFLHs = pExt->WeaponBurstFLHs;

	if (pThis->Veterancy.IsElite())
	{
		if (pInf && pInf->IsDeployed())
			pickedFLHs = pExt->EliteDeployedWeaponBurstFLHs;
		else if (pInf && pInf->Crawling)
			pickedFLHs = pExt->EliteCrouchedWeaponBurstFLHs;
		else
			pickedFLHs = pExt->EliteWeaponBurstFLHs;
	}
	else
	{
		if (pInf && pInf->IsDeployed())
			pickedFLHs = pExt->DeployedWeaponBurstFLHs;
		else if (pInf && pInf->Crawling)
			pickedFLHs = pExt->CrouchedWeaponBurstFLHs;
	}

	if (pickedFLHs[weaponIndex].Count > pThis->CurrentBurstIndex)
	{
		FLHFound = true;
		FLH = pickedFLHs[weaponIndex][pThis->CurrentBurstIndex];
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
	if (!TechnoExt::IsActive(pThis))
		return;

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
	if (!TechnoExt::IsActive(pThis))
		return;

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
	IonCannonTypeClass* pIonCannonType = nullptr;

	pIonCannonType = pExt->setIonCannonType.Get(nullptr);

	if (pIonCannonType == nullptr)
		return;

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
	
	if (pWeapon == nullptr)
		return;
	
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
	if (pThis->IsRedHP() && !pTypeExt->FireSelf_Weapon_RedHeath.empty()  && !pTypeExt->FireSelf_ROF_RedHeath.empty())
	{
		pExt->FireSelf_Weapon = pTypeExt->FireSelf_Weapon_RedHeath;
		pExt->FireSelf_ROF = pTypeExt->FireSelf_ROF_RedHeath;
	}
	else if (pThis->IsYellowHP() && !pTypeExt->FireSelf_Weapon_YellowHeath.empty() && !pTypeExt->FireSelf_ROF_YellowHeath.empty())
	{
		pExt->FireSelf_Weapon = pTypeExt->FireSelf_Weapon_YellowHeath;
		pExt->FireSelf_ROF = pTypeExt->FireSelf_ROF_YellowHeath;
	}
	else if (pThis->IsGreenHP() && !pTypeExt->FireSelf_Weapon_GreenHeath.empty() && !pTypeExt->FireSelf_ROF_GreenHeath.empty())
	{
		pExt->FireSelf_Weapon = pTypeExt->FireSelf_Weapon_GreenHeath;
		pExt->FireSelf_ROF = pTypeExt->FireSelf_ROF_GreenHeath;
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
		AttachmentClass* pAttachment = new AttachmentClass(&entry, pThis);
		pExt->ChildAttachments.emplace_back(pAttachment);
		pExt->ChildAttachments.back()->Initialize();
	}
}

void TechnoExt::HandleHostDestruction(TechnoClass* pThis)
{
	auto const pExt = TechnoExt::ExtMap.Find(pThis);
	for (auto const& pAttachment : pExt->ChildAttachments)
	{
		//Debug::Log("[Attachment] Ptr9[0x%X]\n", pAttachment);
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

void TechnoExt::DrawSelectBrd(TechnoClass* pThis, TechnoTypeExt::ExtData* pTypeExt, int iLength, Point2D* pLocation, RectangleStruct* pBound, bool isInfantry)
{
	Point2D vPos = { 0, 0 };
	Point2D vLoc = *pLocation;
	Point2D vOfs = { 0, 0 };

	int frame, XOffset, YOffset;

	Vector3D<int> glbSelectbrdFrame = isInfantry ?
		RulesExt::Global()->SelectBrd_Frame_Infantry.Get() :
		RulesExt::Global()->SelectBrd_Frame_Unit.Get();

	Vector3D<int> selectbrdFrame = pTypeExt->SelectBrd_Frame.Get();

	auto const nFlag = BlitterFlags::Centered | BlitterFlags::Nonzero | BlitterFlags::MultiPass | EnumFunctions::GetTranslucentLevel(pTypeExt->SelectBrd_TranslucentLevel.Get(RulesExt::Global()->SelectBrd_DefaultTranslucentLevel.Get()));

	auto const canSee = pThis->Owner->IsAlliedWith(HouseClass::Player)
		|| HouseClass::IsPlayerObserver()
		|| pTypeExt->SelectBrd_ShowEnemy.Get(RulesExt::Global()->SelectBrd_DefaultShowEnemy.Get());

	if (selectbrdFrame.X == -1)
	{
		selectbrdFrame = glbSelectbrdFrame;
	}

	vOfs = pTypeExt->SelectBrd_DrawOffset.Get();
	if (vOfs.X == NULL || vOfs.Y == NULL)
	{
		if (isInfantry)
			vOfs = RulesExt::Global()->SelectBrd_DrawOffset_Infantry.Get();
		else
			vOfs = RulesExt::Global()->SelectBrd_DrawOffset_Unit.Get();
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

	SHPStruct* SelectBrdSHP = pTypeExt->SHP_SelectBrdSHP;
	SHPStruct* GlbSelectBrdSHP = nullptr;
	if (isInfantry)
		GlbSelectBrdSHP = RulesExt::Global()->SHP_SelectBrdSHP_INF;
	else
		GlbSelectBrdSHP = RulesExt::Global()->SHP_SelectBrdSHP_UNIT;
	if (SelectBrdSHP == nullptr)
	{
		char FilenameSHP[0x20];
		strcpy_s(FilenameSHP, pTypeExt->SelectBrd_SHP.data());

		if (strcmp(FilenameSHP, "") == 0)
		{
			if (GlbSelectBrdSHP == nullptr)
			{
				char GlbFilenameSHP[0x20];
				if (isInfantry)
					strcpy_s(GlbFilenameSHP, RulesExt::Global()->SelectBrd_SHP_Infantry.data());
				else
					strcpy_s(GlbFilenameSHP, RulesExt::Global()->SelectBrd_SHP_Unit.data());

				if (strcmp(GlbFilenameSHP, "") == 0)
					return;
				else
					SelectBrdSHP = pTypeExt->SHP_SelectBrdSHP = FileSystem::LoadSHPFile(GlbFilenameSHP);
			}
			else
				SelectBrdSHP = GlbSelectBrdSHP;
		}
		else
			SelectBrdSHP = pTypeExt->SHP_SelectBrdSHP = FileSystem::LoadSHPFile(FilenameSHP);
	}
	if (SelectBrdSHP == nullptr) return;

	ConvertClass* SelectBrdPAL = pTypeExt->SHP_SelectBrdPAL;
	ConvertClass* GlbSelectBrdPAL = nullptr;
	if (isInfantry)
		GlbSelectBrdPAL = RulesExt::Global()->SHP_SelectBrdPAL_INF;
	else
		GlbSelectBrdPAL = RulesExt::Global()->SHP_SelectBrdPAL_UNIT;
	if (SelectBrdPAL == nullptr)
	{
		char FilenamePAL[0x20];
		strcpy_s(FilenamePAL, pTypeExt->SelectBrd_PAL.data());

		if (strcmp(FilenamePAL, "") == 0)
		{
			if (GlbSelectBrdPAL == nullptr)
			{
				char GlbFilenamePAL[0x20];
				if (isInfantry)
					strcpy_s(GlbFilenamePAL, RulesExt::Global()->SelectBrd_PAL_Infantry.data());
				else
					strcpy_s(GlbFilenamePAL, RulesExt::Global()->SelectBrd_PAL_Unit.data());

				if (strcmp(GlbFilenamePAL, "") == 0)
					return;
				else
					SelectBrdPAL = pTypeExt->SHP_SelectBrdPAL = FileSystem::LoadPALFile(GlbFilenamePAL, DSurface::Temp);
			}
			else
				SelectBrdPAL = GlbSelectBrdPAL;
		}
		else
			SelectBrdPAL = pTypeExt->SHP_SelectBrdPAL = FileSystem::LoadPALFile(FilenamePAL, DSurface::Temp);
	}
	if (SelectBrdPAL == nullptr) return;

	if (pThis->IsSelected && canSee)
	{
		if (pThis->IsGreenHP())
			frame = selectbrdFrame.X;
		else if (pThis->IsYellowHP())
			frame = selectbrdFrame.Y;
		else
			frame = selectbrdFrame.Z;
		DSurface::Temp->DrawSHP(SelectBrdPAL, SelectBrdSHP,
			frame, &vPos, pBound, nFlag, 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
	}
}

void TechnoExt::DisplayDamageNumberString(TechnoClass* pThis, int damage, bool isShieldDamage)
{
	if (!pThis || damage == 0)
		return;

	const auto pExt = TechnoExt::ExtMap.Find(pThis);

	auto color = isShieldDamage ? damage > 0 ? ColorStruct { 0, 160, 255 } : ColorStruct { 0, 255, 230 } :
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
	//Debug::Log("[FireScript::Info] This Loc(%d,%d,%d), Target Loc(%d,%d,%d)\n", pThis->Location.X, pThis->Location.Y, pThis->Location.Z, Loc.X, Loc.Y, Loc.Z);
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
		pThis->UnmarkAllOccupationBits(Src);
		pThis->SetLocation(location);
		pThis->MarkAllOccupationBits(location);
		//pThis->ForceMission(Mission::Stop);
		//pThis->Guard();

		if (pWeaponExt->BlinkWeapon_KillTarget.Get())
			pTargetTechno->ReceiveDamage(&pTargetTechno->Health, 0, pWeapon->Warhead, pThis, true, false, pThis->GetOwningHouse());

	}
}

void TechnoExt::ReceiveDamageAnim(TechnoClass* pThis, int damage)
{
	//Debug::Log("[ReceiveDamageAnim] Activated!\n");

	if (!pThis || damage == 0)
		return;

	auto pTypeThis = pThis->GetTechnoType();
	auto pTypeData = TechnoTypeExt::ExtMap.Find(pTypeThis);
	auto pData = TechnoExt::ExtMap.Find(pThis);

	GScreenAnimTypeClass* pReceiveDamageAnimType = nullptr;

	pReceiveDamageAnimType = pTypeData->GScreenAnimType.Get();

	if (pTypeThis && pTypeData && pData && pReceiveDamageAnimType)
	{
		//Debug::Log("[ReceiveDamageAnim] pTypeData->GScreenAnimType.Get() Successfully!\n");

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
			//Debug::Log("[ReceiveDamageAnim::Error] SHP file not found\n");
			return;
		}
		if (ShowAnimPAL == nullptr)
		{
			//Debug::Log("[ReceiveDamageAnim::Error] PAL file not found\n");
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
	CoordStruct Loc = pThis->GetCoords();
	Point2D res = { 0,0 };
	TacticalClass::Instance->CoordsToScreen(&res, &Loc);
	res -= TacticalClass::Instance->TacticalPos;
	return res;
}

Point2D TechnoExt::GetHealthBarPosition(TechnoClass* pThis, bool Shield, HealthBarAnchors Anchor)
{
	Point2D Loc = GetScreenLocation(pThis);
	Point2D Pos = { 0, 0 };
	AbstractType thisAbstractType = pThis->WhatAmI();
	int iLength = thisAbstractType == AbstractType::Infantry ? 8 : 17;
	TechnoTypeClass* pType = pThis->GetTechnoType();

	if (thisAbstractType == AbstractType::Building)
	{
		BuildingTypeClass* pBuildingType = abstract_cast<BuildingTypeClass*>(pThis->GetTechnoType());
		CoordStruct Coords = { 0, 0, 0 };
		pBuildingType->Dimension2(&Coords);
		Point2D Pos2 = { 0, 0 };
		CoordStruct Coords2 = { -Coords.X / 2, Coords.Y / 2, Coords.Z };
		TacticalClass::Instance->CoordsToScreen(&Pos2, &Coords2);
		int FoundationHeight = pBuildingType->GetFoundationHeight(false);

		iLength = FoundationHeight * 7 + FoundationHeight / 2;
		Pos.X = Pos2.X + Loc.X + 4 * iLength + 3 - (Shield ? 6 : 0);
		Pos.Y = Pos2.Y + Loc.Y - 2 * iLength + 4 - (Shield ? 3 : 0);

		if (Anchor & HealthBarAnchors::Center)
		{
			Pos.X -= iLength * 2 + 2;
			Pos.Y += iLength + 1;
		}
		else
		{
			if (!(Anchor & HealthBarAnchors::Right))
			{
				Pos.X -= (iLength + 1) * 4;
				Pos.Y += (iLength + 1) * 2;
			}
		}

		if (!(Anchor & HealthBarAnchors::Bottom))
		{
			Pos.Y += 4;
			Pos.X += 4;
		}
	}
	else
	{
		Pos.X = Loc.X - iLength + (iLength == 8);
		Pos.Y = Loc.Y - 28 + (iLength == 8);
		Pos.Y += pType->PixelSelectionBracketDelta;

		if (Shield)
		{
			Pos.Y -= 5;

			auto pExt = ExtMap.Find(pThis);

			if (pExt->Shield != nullptr && !pExt->Shield->IsBrokenAndNonRespawning())
				Pos.Y += pExt->Shield->GetType()->BracketDelta.Get();
		}

		if (Anchor & HealthBarAnchors::Center)
		{
			Pos.X += iLength;
		}
		else
		{
			if (Anchor & HealthBarAnchors::Right)
				Pos.X += iLength * 2;
		}

		if (Anchor & HealthBarAnchors::Bottom)
			Pos.Y += 4;
	}

	return Pos;
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
		;
	for (auto& it : Processing_Scripts) delete it;
	FireSelf_Count.clear();
	FireSelf_Weapon.clear();
	FireSelf_ROF.clear();
	Processing_Scripts.clear();
}

void TechnoExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	TechnoClass* pOldThis = nullptr;
	Stm.Load(pOldThis);
	PointerMapper::AddMapping(pOldThis, this->OwnerObject());
	Extension<TechnoClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void TechnoExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	TechnoClass* pThis = this->OwnerObject();
	Stm.Save(pThis);
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
