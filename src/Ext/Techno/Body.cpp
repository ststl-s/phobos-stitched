#include "Body.h"

#include <BuildingClass.h>
#include <HouseClass.h>
#include <BulletClass.h>
#include <BulletTypeClass.h>
#include <ScenarioClass.h>
#include <SpawnManagerClass.h>
#include <InfantryClass.h>
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
#include <New/Entity/TechnoHugeHP.h>
#include <New/Type/DigitalDisplayTypeClass.h>
#include <Ext/Team/Body.h>
#include <Ext/Script/Body.h>
#include <New/Type/IonCannonTypeClass.h>
#include <New/Type/GScreenAnimTypeClass.h>
#include <Misc/GScreenDisplay.h>
#include <JumpjetLocomotionClass.h>

template<> const DWORD Extension<TechnoClass>::Canary = 0x55555555;
TechnoExt::ExtContainer TechnoExt::ExtMap;
int TechnoExt::ExtData::counter = 0;

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

void TechnoExt::ApplyMindControlRangeLimit(TechnoClass* pThis)
{
	if (auto pCapturer = pThis->MindControlledBy)
	{
		auto pCapturerExt = TechnoTypeExt::ExtMap.Find(pCapturer->GetTechnoType());
		if (pCapturerExt && pCapturerExt->MindControlRangeLimit.Get() > 0 &&
			pThis->DistanceFrom(pCapturer) > pCapturerExt->MindControlRangeLimit.Get())
		{
			pCapturer->CaptureManager->FreeUnit(pThis);
		}
	}
}

void TechnoExt::ApplyInterceptor(TechnoClass* pThis)
{
	auto pData = TechnoExt::ExtMap.Find(pThis);
	auto const pTypeData = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	bool interceptor = pTypeData->Interceptor;

	if (pData && pTypeData && interceptor)
	{
		bool interceptor_Rookie = pTypeData->Interceptor_Rookie.Get(true);
		bool interceptor_Veteran = pTypeData->Interceptor_Veteran.Get(true);
		bool interceptor_Elite = pTypeData->Interceptor_Elite.Get(true);

		if (pThis->Veterancy.IsRookie() && !interceptor_Rookie)
			interceptor = false;

		if (pThis->Veterancy.IsVeteran() && !interceptor_Veteran)
			interceptor = false;

		if (pThis->Veterancy.IsElite() && !interceptor_Elite)
			interceptor = false;

		if (interceptor && !pThis->Target &&
			!(pThis->WhatAmI() == AbstractType::Aircraft && pThis->GetHeight() <= 0))
		{
			for (auto const& pBullet : *BulletClass::Array)
			{
				if (auto pBulletTypeData = BulletTypeExt::ExtMap.Find(pBullet->Type))
				{
					if (!pBulletTypeData->Interceptable)
						continue;
				}

			const auto guardRange = pThis->Veterancy.IsElite() ?
				pTypeData->Interceptor_EliteGuardRange :
				pTypeData->Interceptor_GuardRange;
			const auto minguardRange = pThis->Veterancy.IsElite() ?
				pTypeData->Interceptor_EliteMinimumGuardRange :
				pTypeData->Interceptor_MinimumGuardRange;

			auto distance = pBullet->Location.DistanceFrom(pThis->Location);
			if (distance > guardRange.Get() || distance < minguardRange.Get())
				continue;

			/*
			if (pBullet->Location.DistanceFrom(pBullet->TargetCoords) >
				double(ScenarioClass::Instance->Random.RandomRanged(128, (int)guardRange / 10)) * 10)
			{
				continue;
			}
			*/

			if (!pThis->Owner->IsAlliedWith(pBullet->Owner))
			{
				pThis->SetTarget(pBullet);
				pData->InterceptedBullet = pBullet;
				break;
			}
			}
		}
	}
}

void TechnoExt::ApplyPowered_KillSpawns(TechnoClass* pThis)
{
	auto const pTypeData = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	if (pTypeData && pThis->WhatAmI() == AbstractType::Building)
	{
		auto const pBuilding = abstract_cast<BuildingClass*>(pThis);
		if (pTypeData->Powered_KillSpawns && pBuilding->Type->Powered && !pBuilding->IsPowerOnline())
		{
			if (auto pManager = pBuilding->SpawnManager)
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
}

void TechnoExt::ApplySpawn_LimitRange(TechnoClass* pThis)
{
	auto const pTypeData = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	if (pTypeData && pTypeData->Spawn_LimitedRange)
	{
		if (auto const pManager = pThis->SpawnManager)
		{
			auto pTechnoType = pThis->GetTechnoType();
			int weaponRange = 0;
			int weaponRangeExtra = pTypeData->Spawn_LimitedExtraRange * 256;

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
	if ((mission == Mission::Harvest || mission == Mission::Unload || mission == Mission::Enter)
		&& TechnoExt::HasAvailableDock(pThis))
	{
		return true;
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

void TechnoExt::EatPassengers(TechnoClass* pThis)
{
	if (!TechnoExt::IsActive(pThis))
		return;

	auto const pData = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (pData && pData->PassengerDeletion_Rate > 0)
	{
		auto pExt = TechnoExt::ExtMap.Find(pThis);

		if (pThis->Passengers.NumPassengers > 0)
		{
			FootClass* pPassenger = pThis->Passengers.GetFirstPassenger();

			if (pExt->PassengerDeletionCountDown < 0)
			{
				// Setting & start countdown. Bigger units needs more time
				int passengerSize = pData->PassengerDeletion_Rate;
				if (pData->PassengerDeletion_Rate_SizeMultiply && pPassenger->GetTechnoType()->Size > 1.0)
					passengerSize *= (int)(pPassenger->GetTechnoType()->Size + 0.5);

				pExt->PassengerDeletionCountDown = passengerSize;
				pExt->PassengerDeletionTimer.Start(passengerSize);
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
						pPassenger = static_cast<FootClass*>(pPassenger->NextObject);
					}

					if (pLastPassenger)
						pLastPassenger->NextObject = nullptr;
					else
						pThis->Passengers.FirstPassenger = nullptr;

					--pThis->Passengers.NumPassengers;

					if (pPassenger)
					{
						if (auto const pPassengerType = pPassenger->GetTechnoType())
						{
							VocClass::PlayAt(pData->PassengerDeletion_ReportSound, pThis->GetCoords(), nullptr);

							if (pData->PassengerDeletion_Anim.isset())
							{
								const auto pAnimType = pData->PassengerDeletion_Anim.Get();
								if (auto const pAnim = GameCreate<AnimClass>(pAnimType, pThis->Location))
								{
									pAnim->SetOwnerObject(pThis);
									pAnim->Owner = pThis->Owner;
								}
							}

							// Check if there is money refund
							if (pData->PassengerDeletion_Soylent)
							{
								int nMoneyToGive = 0;

								// Refund money to the Attacker
								if (pPassengerType && pPassengerType->Soylent > 0)
									nMoneyToGive = pPassengerType->Soylent;

								// Is allowed the refund of friendly units?
								if (!pData->PassengerDeletion_SoylentFriendlies && pPassenger->Owner->IsAlliedWith(pThis))
									nMoneyToGive = 0;

								if (nMoneyToGive > 0)
									pThis->Owner->GiveMoney(nMoneyToGive);
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

void TechnoExt::ChangePassengersList(TechnoClass* pThis)
{
	if (!TechnoExt::IsActive(pThis))
		return;

	auto pData = TechnoExt::ExtMap.Find(pThis);

	if (pData->AllowChangePassenger)
	{
		int i = 0;
		do
		{
			pData->PassengerlocationList[i] = pData->PassengerlocationList[i + 1];
			i++;
		}
		while (pData->PassengerlocationList[i] != CoordStruct { 0, 0, 0 });

		int j = 0;
		do
		{
			pData->PassengerList[j] = pData->PassengerList[j + 1];
			j++;
		}
		while (pData->PassengerList[j] != nullptr);
		pData->AllowCreatPassenger = true;
		pData->AllowChangePassenger = false;
	}
}

void TechnoExt::FirePassenger(TechnoClass* pThis, AbstractClass* pTarget, WeaponTypeClass* pWeapon)
{
	if (!TechnoExt::IsActive(pThis))
		return;

	auto const pData = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);

	if (pData && pWeaponExt->PassengerDeletion)
	{
		auto pExt = TechnoExt::ExtMap.Find(pThis);

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
					auto pTechnoData = TechnoExt::ExtMap.Find(pThis);
					TechnoClass* pTargetType = abstract_cast<TechnoClass*>(pTarget);

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
void TechnoExt::CheckDeathConditions(TechnoClass* pThis)
{
	auto pTypeThis = pThis->GetTechnoType();
	auto pTypeData = TechnoTypeExt::ExtMap.Find(pTypeThis);
	auto pData = TechnoExt::ExtMap.Find(pThis);

	const bool peacefulDeath = pTypeData->Death_Peaceful.Get();
	// Death if no ammo
	if (pTypeThis && pTypeData && pTypeData->Death_NoAmmo)
	{
		if (pTypeThis->Ammo > 0 && pThis->Ammo <= 0)
		{
			if (peacefulDeath)
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

	// Death if countdown ends
	if (pTypeThis && pData && pTypeData && pTypeData->Death_Countdown > 0)
	{
		if (pData->Death_Countdown >= 0)
		{
			if (pData->Death_Countdown > 0)
			{
				pData->Death_Countdown--; // Update countdown
			}
			else
			{
				// Countdown ended. Kill the unit
				pData->Death_Countdown = -1;
				if (peacefulDeath)
				{
					pThis->Limbo();
					pThis->UnInit();
				}
				else
				pThis->ReceiveDamage(&pThis->Health, 0, RulesClass::Instance()->C4Warhead, nullptr, true, false, pThis->Owner);
			}
		}
		else
		{
			pData->Death_Countdown = pTypeData->Death_Countdown; // Start countdown
		}
	}
}

void TechnoExt::CheckIonCannonConditions(TechnoClass* pThis)
{
	auto pTypeThis = pThis->GetTechnoType();
	auto pTypeData = TechnoTypeExt::ExtMap.Find(pTypeThis);
	auto pData = TechnoExt::ExtMap.Find(pThis);

	IonCannonTypeClass* pIonCannonType = nullptr;

	pIonCannonType = pTypeData->IonCannonType.Get();
	
	if (pIonCannonType == nullptr)
		return;

	if (pTypeThis && pIonCannonType && pIonCannonType->IonCannon_Radius >= 0)
	{
		if (!(pData->IonCannon_Rate > 0))
		{
			if (pData->IonCannon_setRadius)
			{
				pData->IonCannon_Radius = pIonCannonType->IonCannon_Radius;
				pData->IonCannon_RadiusReduce = pIonCannonType->IonCannon_RadiusReduce;
				pData->IonCannon_Angle = pIonCannonType->IonCannon_Angle;
				pData->IonCannon_Scatter_Max = pIonCannonType->IonCannon_Scatter_Max;
				pData->IonCannon_Scatter_Min = pIonCannonType->IonCannon_Scatter_Min;
				pData->IonCannon_Duration = pIonCannonType->IonCannon_Duration;
				pData->IonCannon_setRadius = false;
			}

			if (pData->IonCannon_Radius >= 0 && !pData->IonCannon_Stop)
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
				for (int angle = pData->IonCannon_StartAngle; angle < pData->IonCannon_StartAngle + 360; angle += angleDelta)
				{
					int ScatterX = (rand() % (pData->IonCannon_Scatter_Max - pData->IonCannon_Scatter_Min + 1)) + pData->IonCannon_Scatter_Min;
					int ScatterY = (rand() % (pData->IonCannon_Scatter_Max - pData->IonCannon_Scatter_Min + 1)) + pData->IonCannon_Scatter_Min;
					
					if (rand() % 2)
					{
						ScatterX = -ScatterX;
					}
					if (rand() % 2)
					{
						ScatterY = -ScatterY;
					}

					CoordStruct pos =
					{
						center.X + (int)(pData->IonCannon_Radius * cos(angle * 3.14 / 180)) + ScatterX,
						center.Y + (int)(pData->IonCannon_Radius * sin(angle * 3.14 / 180)) + ScatterY,
						0
					};

					CoordStruct posAir = pos + CoordStruct { 0, 0, pIonCannonType->IonCannon_LaserHeight };

					CoordStruct posAirEle = pos + CoordStruct { 0, 0, pIonCannonType->IonCannon_EleHeight };

					auto pCell = MapClass::Instance->TryGetCellAt(pos);
					if (pCell)
					{
						pos.Z = pCell->GetCoordsWithBridge().Z;
					}
					else
					{
						pos.Z = MapClass::Instance->GetCellFloorHeight(pos);
					}

					if (!(pIonCannonType->IonCannon_DrawLaserWithFire && pData->IonCannon_ROF > 0))
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

					if (!(pIonCannonType->IonCannon_DrawEBoltWithFire && pData->IonCannon_ROF > 0))
					{
						if (pIonCannonType->IonCannon_DrawEBolt) // Uses laser
						{
							if (auto const pEBolt = GameCreate<EBolt>())
								pEBolt->Fire(posAirEle, pos, 0);
						}
					}

					if (!(pData->IonCannon_ROF > 0))
					{
						WeaponTypeExt::DetonateAt(pIonCannonWeapon, pos, pThis); // 单位使用主武器攻击坐标点
					}
				}

				if (pData->IonCannon_ROF > 0)
				{
					pData->IonCannon_ROF--;
				}
				else
				{
					pData->IonCannon_ROF = pIonCannonType->IonCannon_ROF;
				}

				if (pData->IonCannon_RadiusReduce <= pIonCannonType->IonCannon_RadiusReduceMax && pData->IonCannon_RadiusReduce >= pIonCannonType->IonCannon_RadiusReduceMin)
				{
					pData->IonCannon_RadiusReduce += pIonCannonType->IonCannon_RadiusReduceAcceleration;
				}
				if (pData->IonCannon_Angle <= pIonCannonType->IonCannon_AngleMax && pData->IonCannon_Angle >= pIonCannonType->IonCannon_AngleMin)
				{
					pData->IonCannon_Angle += pIonCannonType->IonCannon_AngleAcceleration;
				}

				if (pData->IonCannon_Scatter_Max <= pIonCannonType->IonCannon_Scatter_Max_IncreaseMax && pData->IonCannon_Scatter_Max >= pIonCannonType->IonCannon_Scatter_Max_IncreaseMin)
				{
					pData->IonCannon_Scatter_Max += pIonCannonType->IonCannon_Scatter_Max_Increase;
				}
				if (pData->IonCannon_Scatter_Min <= pIonCannonType->IonCannon_Scatter_Min_IncreaseMax && pData->IonCannon_Scatter_Min >= pIonCannonType->IonCannon_Scatter_Min_IncreaseMin)
				{
					pData->IonCannon_Scatter_Min += pIonCannonType->IonCannon_Scatter_Min_Increase;
				}

				pData->IonCannon_Radius -= pData->IonCannon_RadiusReduce; //默认20; // 每次半径减少的量，越大光束聚集越快
				pData->IonCannon_StartAngle -= pData->IonCannon_Angle; // 每次旋转角度，越大旋转越快

				if (pIonCannonType->IonCannon_MaxRadius >= 0)
				{
					if (pData->IonCannon_Radius > pIonCannonType->IonCannon_MaxRadius)
					{
						pData->IonCannon_Stop = true;
					}
				}

				if (pIonCannonType->IonCannon_MinRadius >= 0)
				{
					if (pData->IonCannon_Radius < pIonCannonType->IonCannon_MinRadius)
					{
						pData->IonCannon_Stop = true;
					}
				}

				if (pIonCannonType->IonCannon_Duration >= 0)
				{
					if (pData->IonCannon_Duration > 0)
					{
						pData->IonCannon_Duration--;
					}
					else
					{
						pData->IonCannon_Stop = true;
					}
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
					pData->IonCannon_setRadius = true;
					pData->IonCannon_Stop = false;
					pData->IonCannon_Rate = pIonCannonType->IonCannon_Rate;
				}
			}
		}
		else
		{
			pData->IonCannon_Rate--;
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

void TechnoExt::RunIonCannonWeapon(TechnoClass* pThis)
{
	
	auto pTypeThis = pThis->GetTechnoType();
	auto pData = TechnoExt::ExtMap.Find(pThis);
	auto pWeapon = pData->setIonCannonWeapon;
	//auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);

	IonCannonTypeClass* pIonCannonType = nullptr;

	pIonCannonType = pData->setIonCannonType;

	if (pIonCannonType == nullptr)
		return;

	if (pTypeThis && pIonCannonType && pIonCannonType->IonCannon_Radius >= 0)
	{
		if (pData->IonCannonWeapon_setRadius)
		{
			pData->IonCannonWeapon_Radius = pIonCannonType->IonCannon_Radius;
			pData->IonCannonWeapon_RadiusReduce = pIonCannonType->IonCannon_RadiusReduce;
			pData->IonCannonWeapon_Angle = pIonCannonType->IonCannon_Angle;
			pData->IonCannonWeapon_Scatter_Max = pIonCannonType->IonCannon_Scatter_Max;
			pData->IonCannonWeapon_Scatter_Min = pIonCannonType->IonCannon_Scatter_Min;
			pData->IonCannonWeapon_Duration = pIonCannonType->IonCannon_Duration;
			pData->IonCannonWeapon_setRadius = false;
		}

		CoordStruct target = pData->IonCannonWeapon_Target;

		if (pData->IonCannonWeapon_Radius >= 0 && !pData->IonCannonWeapon_Stop)
		{

			WeaponTypeClass* pIonCannonWeapon = pWeapon;
			if (pIonCannonType->IonCannon_Weapon.isset())
			{
				pIonCannonWeapon = pIonCannonType->IonCannon_Weapon.Get();
			}

			// 每xx角度生成一条光束，越小越密集
			int angleDelta = 360 / pIonCannonType->IonCannon_Lines;
			for (int angle = pData->IonCannonWeapon_StartAngle; angle < pData->IonCannonWeapon_StartAngle + 360; angle += angleDelta)
			{
				int ScatterX = (rand() % (pData->IonCannonWeapon_Scatter_Max - pData->IonCannonWeapon_Scatter_Min + 1)) + pData->IonCannonWeapon_Scatter_Min;
				int ScatterY = (rand() % (pData->IonCannonWeapon_Scatter_Max - pData->IonCannonWeapon_Scatter_Min + 1)) + pData->IonCannonWeapon_Scatter_Min;

				if (rand() % 2)
				{
					ScatterX = -ScatterX;
				}
				if (rand() % 2)
				{
					ScatterY = -ScatterY;
				}

				CoordStruct pos =
				{
					target.X + (int)(pData->IonCannonWeapon_Radius * cos(angle * 3.14 / 180)) + ScatterX,
					target.Y + (int)(pData->IonCannonWeapon_Radius * sin(angle * 3.14 / 180)) + ScatterY,
					0
				};

				CoordStruct posAir = pos + CoordStruct { 0, 0, pIonCannonType->IonCannon_LaserHeight };

				CoordStruct posAirEle = pos + CoordStruct { 0, 0, pIonCannonType->IonCannon_EleHeight };

				auto pCell = MapClass::Instance->TryGetCellAt(pos);
				if (pCell)
				{
					pos.Z = pCell->GetCoordsWithBridge().Z;
				}
				else
				{
					pos.Z = MapClass::Instance->GetCellFloorHeight(pos);
				}

				if (!(pIonCannonType->IonCannon_DrawLaserWithFire && pData->IonCannonWeapon_ROF > 0))
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

				if (!(pIonCannonType->IonCannon_DrawEBoltWithFire && pData->IonCannonWeapon_ROF > 0))
				{
					if (pIonCannonType->IonCannon_DrawEBolt) // Uses laser
					{
						if (auto const pEBolt = GameCreate<EBolt>())
							pEBolt->Fire(posAirEle, pos, 0);
					}
				}

				if (!(pData->IonCannonWeapon_ROF > 0))
				{
					WeaponTypeExt::DetonateAt(pIonCannonWeapon, pos, pThis); // 单位使用主武器攻击坐标点
				}
			}

			if (pData->IonCannonWeapon_ROF > 0)
			{
				pData->IonCannonWeapon_ROF--;
			}
			else
			{
				pData->IonCannonWeapon_ROF = pIonCannonType->IonCannon_ROF;
			}

			if (pData->IonCannonWeapon_RadiusReduce <= pIonCannonType->IonCannon_RadiusReduceMax && pData->IonCannonWeapon_RadiusReduce >= pIonCannonType->IonCannon_RadiusReduceMin)
			{
				pData->IonCannonWeapon_RadiusReduce += pIonCannonType->IonCannon_RadiusReduceAcceleration;
			}
			if (pData->IonCannonWeapon_Angle <= pIonCannonType->IonCannon_AngleMax && pData->IonCannonWeapon_Angle >= pIonCannonType->IonCannon_AngleMin)
			{
				pData->IonCannonWeapon_Angle += pIonCannonType->IonCannon_AngleAcceleration;
			}

			if (pData->IonCannonWeapon_Scatter_Max <= pIonCannonType->IonCannon_Scatter_Max_IncreaseMax && pData->IonCannonWeapon_Scatter_Max >= pIonCannonType->IonCannon_Scatter_Max_IncreaseMin)
			{
				pData->IonCannonWeapon_Scatter_Max += pIonCannonType->IonCannon_Scatter_Max_Increase;
			}
			if (pData->IonCannonWeapon_Scatter_Min <= pIonCannonType->IonCannon_Scatter_Min_IncreaseMax && pData->IonCannonWeapon_Scatter_Min >= pIonCannonType->IonCannon_Scatter_Min_IncreaseMin)
			{
				pData->IonCannonWeapon_Scatter_Min += pIonCannonType->IonCannon_Scatter_Min_Increase;
			}

			pData->IonCannonWeapon_Radius -= pData->IonCannonWeapon_RadiusReduce; //默认20; // 每次半径减少的量，越大光束聚集越快
			pData->IonCannonWeapon_StartAngle -= pData->IonCannonWeapon_Angle; // 每次旋转角度，越大旋转越快

			if (pIonCannonType->IonCannon_MaxRadius >= 0)
			{
				if (pData->IonCannonWeapon_Radius > pIonCannonType->IonCannon_MaxRadius)
				{
					pData->IonCannonWeapon_Stop = true;
				}
			}

			if (pIonCannonType->IonCannon_MinRadius >= 0)
			{
				if (pData->IonCannonWeapon_Radius < pIonCannonType->IonCannon_MinRadius)
				{
					pData->IonCannonWeapon_Stop = true;
				}
			}

			if (pIonCannonType->IonCannon_Duration >= 0)
			{
				if (pData->IonCannonWeapon_Duration > 0)
				{
					pData->IonCannonWeapon_Duration--;
				}
				else
				{
					pData->IonCannonWeapon_Stop = true;
				}
			}
		}
		else
		{
			pData->IonCannonWeapon_setRadius = true;
			pData->IonCannonWeapon_Stop = true;
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

void TechnoExt::RunBeamCannon(TechnoClass* pThis)
{
	auto pTypeThis = pThis->GetTechnoType();
	auto pData = TechnoExt::ExtMap.Find(pThis);
	auto pWeapon = pData->setBeamCannon;
	auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);

	if (pTypeThis && pWeaponExt && pWeaponExt->IsBeamCannon && pWeaponExt->BeamCannon_Length >= 0)
	{
		if (pData->BeamCannon_setLength)
		{
			pData->BeamCannon_Length = 0;
			pData->BeamCannon_LengthIncrease = pWeaponExt->BeamCannon_LengthIncrease;
			pData->BeamCannon_setLength = false;
		}

		CoordStruct target = pData->BeamCannon_Target;
		CoordStruct center = pData->BeamCannon_Self;

		if (abs(pData->BeamCannon_Length) <= pWeaponExt->BeamCannon_Length && !pData->BeamCannon_Stop)
		{

			WeaponTypeClass* pBeamCannonWeapon = pWeapon;
			if (pWeaponExt->BeamCannonWeapon.isset())
			{
				pBeamCannonWeapon = pWeaponExt->BeamCannonWeapon.Get();
			}

			CoordStruct pos =
			{
				center.X + (int)((pData->BeamCannon_Length + pWeaponExt->BeamCannon_Length_StartOffset) * cos(atan2(target.Y - center.Y , target.X - center.X))),
				center.Y + (int)((pData->BeamCannon_Length + pWeaponExt->BeamCannon_Length_StartOffset) * sin(atan2(target.Y - center.Y , target.X - center.X))),
				0
			};

			auto pCell = MapClass::Instance->TryGetCellAt(pos);
			if (pCell)
			{
				pos.Z = pCell->GetCoordsWithBridge().Z;
			}
			else
			{
				pos.Z = MapClass::Instance->GetCellFloorHeight(pos);
			}

			CoordStruct posAir = pos + CoordStruct { 0, 0, pWeaponExt->BeamCannon_LaserHeight };
			CoordStruct posAirEle = pos + CoordStruct { 0, 0, pWeaponExt->BeamCannon_EleHeight };
			if (pWeaponExt->BeamCannon_DrawFromSelf)
			{
				posAir = pThis->GetCoords() + CoordStruct { 0, 0, pWeaponExt->BeamCannon_DrawFromSelf_HeightOffset };
				posAirEle = pThis->GetCoords() + CoordStruct { 0, 0, pWeaponExt->BeamCannon_DrawFromSelf_HeightOffset };
			}


			if (pWeaponExt->BeamCannon_DrawLaser)
			{
				LaserDrawClass* pLaser = GameCreate<LaserDrawClass>(
					posAir, pos,
					pWeaponExt->BeamCannon_InnerColor, pWeaponExt->BeamCannon_OuterColor, pWeaponExt->BeamCannon_OuterSpread,
					pWeaponExt->BeamCannon_Duration);

				pLaser->Thickness = pWeaponExt->BeamCannon_Thickness; // only respected if IsHouseColor
				pLaser->IsHouseColor = true;
				// pLaser->IsSupported = this->Type->IsIntense;
			}
			if (pWeaponExt->BeamCannon_DrawEBolt)
			{
				if (auto const pEBolt = GameCreate<EBolt>())
					pEBolt->Fire(posAirEle, pos, 0);
			}

			if (pData->BeamCannon_ROF > 0)
			{
				pData->BeamCannon_ROF--;
			}
			else
			{
				WeaponTypeExt::DetonateAt(pBeamCannonWeapon, pos, pThis);
				pData->BeamCannon_ROF = pWeaponExt->BeamCannon_ROF;
			}

			if (pData->BeamCannon_LengthIncrease <= pWeaponExt->BeamCannon_LengthIncreaseMax && pData->BeamCannon_LengthIncrease >= pWeaponExt->BeamCannon_LengthIncreaseMin)
			{
				pData->BeamCannon_LengthIncrease += pWeaponExt->BeamCannon_LengthIncreaseAcceleration;
			}

			pData->BeamCannon_Length += pData->BeamCannon_LengthIncrease;

		}
		else
		{
			pData->BeamCannon_setLength = true;
			pData->BeamCannon_Stop = true;
		}
	}
}

void TechnoExt::RunFireSelf(TechnoClass* pThis)
{
	auto pExt = TechnoExt::ExtMap.Find(pThis);
	auto pType = pThis->GetTechnoType();
	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
	
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
	for (int i = 0; i < (int)pExt->FireSelf_Count.size(); i++)
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
	// auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (pExt && pExt->ParentAttachment)
	{
		//Debug::Log("[Attachment] Ptr6[0x%X]\n", pExt->ParentAttachment);
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
		//Debug::Log("[Attachment] Ptr7[0x%X]\n", pAttachment);
		if (pAttachment->AttachChild(pThis))
			return true;
	}

	return false;
}

bool TechnoExt::DetachFromParent(TechnoClass* pThis, bool isForceDetachment)
{
	auto const pExt = TechnoExt::ExtMap.Find(pThis);
	//Debug::Log("[Attachment] Ptr8[0x%X]\n", pExt->ParentAttachment);
	return pExt->ParentAttachment->DetachChild(isForceDetachment);
}

void TechnoExt::InitializeAttachments(TechnoClass* pThis)
{
	auto const pExt = TechnoExt::ExtMap.Find(pThis);
	auto const pType = pThis->GetTechnoType();
	//Debug::Log("[Attachment] Type[0x%X]\n", pType);
	//Debug::Log("[Attachment] TypeId[%s]\n", pType->get_ID());
	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
	//Debug::Log("[Attachment] TypeExt[0x%X]\n", pTypeExt);
	//Debug::Log("[Attachment] Init: Size[%u]", pTypeExt->AttachmentData.size());

	for (auto& entry : pTypeExt->AttachmentData)
	{
		//Debug::Log("[Attachment] Init: Entry[0x%X] \n", &entry);
		//Debug::Log("[Attachment] Init: Entry->TypeIdx[%d],TechnoType[0x%X]{%s}\n", entry.Type, entry.TechnoType[0], entry.TechnoType[0]->get_ID());
		std::unique_ptr<AttachmentClass> pAttachment(nullptr);
		pExt->ChildAttachments.push_back(new AttachmentClass(&entry, pThis, nullptr));
		pAttachment.reset(pExt->ChildAttachments.back());
		AttachmentClass::Array.push_back(std::move(pAttachment));
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
		auto itAttachment = std::find_if(pParentExt->ChildAttachments.begin(), pParentExt->ChildAttachments.end(), [pThis](AttachmentClass* pAttachment)
		{
			return pThis == pAttachment->Child;
		 });
		pParentExt->ChildAttachments.erase(itAttachment);

		auto pTmp = pExt->ParentAttachment;

		auto itAttachmentGlobal = std::find_if(AttachmentClass::Array.begin(), AttachmentClass::Array.end(), [pTmp](std::unique_ptr<AttachmentClass>& pItem)
		{
			return pTmp == pItem.get();
		});
		AttachmentClass::Array.erase(itAttachmentGlobal);

		if (pExt->ParentAttachment->GetType()->DeathTogether_Parent.Get())
			pParent->ReceiveDamage(&pParent->Health, 0, RulesClass::Instance()->C4Warhead, nullptr, true, false, pParent->Owner);

		pExt->ParentAttachment = nullptr;

	}
	//Debug::Log("[Attachment::Destory] Finish UninitParent\n");
	for (auto& pAttachment : pExt->ChildAttachments)
	{
		TechnoClass* pChild = pAttachment->Child;
		auto pChildExt = TechnoExt::ExtMap.Find(pChild);
		pChildExt->ParentAttachment = nullptr;

		auto itAttachmentGlobal = std::find_if(AttachmentClass::Array.begin(), AttachmentClass::Array.end(), [pAttachment](std::unique_ptr<AttachmentClass>& pItem)
		{
			return pAttachment == pItem.get();
		});
		AttachmentClass::Array.erase(itAttachmentGlobal);

		if (pAttachment->GetType()->DeathTogether_Child.Get())
			pChild->ReceiveDamage(&pChild->Health, 0, RulesClass::Instance()->C4Warhead, nullptr, true, false, pChild->Owner);

		pAttachment = nullptr;
	}
	//Debug::Log("[Attachment::Destory] Finish UninitChild\n");
	pExt->ChildAttachments.clear();
}

void TechnoExt::UnlimboAttachments(TechnoClass* pThis)
{
	auto const pExt = TechnoExt::ExtMap.Find(pThis);
	for (auto const& pAttachment : pExt->ChildAttachments)
	{
		//Debug::Log("[Attachment] Ptr3[0x%X]\n", pAttachment);
		pAttachment->Unlimbo();
	}
}

void TechnoExt::LimboAttachments(TechnoClass* pThis)
{
	auto const pExt = TechnoExt::ExtMap.Find(pThis);
	for (auto const& pAttachment : pExt->ChildAttachments)
	{
		//Debug::Log("[Attachment] Ptr4[0x%X]\n", pAttachment);
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
		//Debug::Log("[Attachment] Ptr5[0x%X]\n", pAttachment);
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

void TechnoExt::DrawSelfHealPips(TechnoClass* pThis, TechnoTypeExt::ExtData* pTypeExt, Point2D* pLocation, RectangleStruct* pBounds)
{
	bool drawPip = false;
	bool isOrganic = true;
	int selfHealFrames = 0;

	if (auto const pExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType()))
	{
		if (pExt->SelfHealGainType.isset() && pExt->SelfHealGainType.Get() == SelfHealGainType::None)
			return;

		if (pThis->Owner->InfantrySelfHeal > 0 &&
			((!pExt->SelfHealGainType.isset() && (pThis->WhatAmI() == AbstractType::Infantry ||
				pThis->GetTechnoType()->Organic && pThis->WhatAmI() == AbstractType::Unit)) ||
				pExt->SelfHealGainType.isset() && pExt->SelfHealGainType.Get() == SelfHealGainType::Infantry))
		{
			drawPip = true;
			selfHealFrames = RulesClass::Instance->SelfHealInfantryFrames;
		}
		else if (pThis->Owner->UnitsSelfHeal > 0 &&
			((!pExt->SelfHealGainType.isset() && pThis->WhatAmI() == AbstractType::Unit) ||
				pExt->SelfHealGainType.isset() && pExt->SelfHealGainType.Get() == SelfHealGainType::Units))
		{
			drawPip = true;
			selfHealFrames = RulesClass::Instance->SelfHealUnitFrames;
			isOrganic = false;
		}
	}

	if (drawPip)
	{
		Valueable<Point2D> pipFrames;
		bool isSelfHealFrame = false;
		int xOffset = 0;
		int yOffset = 0;
		int xOffsetExtra = 0;
		int yOffsetExtra = 0;

		if (Unsorted::CurrentFrame % selfHealFrames <= 5
			&& pThis->Health < pThis->GetTechnoType()->Strength)
		{
			isSelfHealFrame = true;
		}

		if (pThis->WhatAmI() == AbstractType::Unit || pThis->WhatAmI() == AbstractType::Aircraft)
		{
			pipFrames = RulesExt::Global()->Pips_SelfHeal_Units;
			xOffset = 38;
			yOffset = -32;
			xOffsetExtra = -5;
			yOffsetExtra = pThis->GetTechnoType()->PixelSelectionBracketDelta;
		}
		else if (pThis->WhatAmI() == AbstractType::Infantry)
		{
			pipFrames = RulesExt::Global()->Pips_SelfHeal_Infantry;
			xOffset = 19;
			yOffset = -35;
			xOffsetExtra = 6;
			yOffsetExtra = pThis->GetTechnoType()->PixelSelectionBracketDelta;
		}
		else
		{
			auto pType = abstract_cast<BuildingTypeClass*>(pThis->GetTechnoType());
			pipFrames = RulesExt::Global()->Pips_SelfHeal_Buildings;
			xOffset = 38;
			yOffset = -6;
			xOffsetExtra = 6;
			yOffsetExtra = -1 + pType->Height * -15;
		}

		int pipFrame = isOrganic ? pipFrames.Get().X : pipFrames.Get().Y;

		Point2D position = pTypeExt->SelfHealPips_Offset;

		position.X += pLocation->X + xOffset + xOffsetExtra;
		position.Y += pLocation->Y + yOffset + yOffsetExtra;

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
		? Math::clamp((int)round(GetHealthRatio(pThis) * iLength), 0, iLength)
		: 0;
}

double TechnoExt::GetHealthRatio(TechnoClass* pThis)
{
	return static_cast<double>(pThis->Health) / pThis->GetTechnoType()->Strength;
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

	int frame, XOffset, YOffset, XOffset2;
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

	int maxOffset = 30;
	int width = 0, height = 0;
	BitFont::Instance->GetTextDimension(damageStr, &width, &height, 120);

	if (!pExt->DamageNumberOffset.isset() || pExt->DamageNumberOffset >= maxOffset)
		pExt->DamageNumberOffset = -maxOffset;

	FlyingStrings::Add(damageStr, coords, color, Point2D { pExt->DamageNumberOffset - (width / 2), 0 });

	pExt->DamageNumberOffset = pExt->DamageNumberOffset + width;
}

//Is there a already implemented function to tell whether the pTechno can target at pTarget?
static bool __fastcall CanFireAt(TechnoClass* pTechno, AbstractClass* pTarget)
{
	const int wpnIdx = pTechno->SelectWeapon(pTarget);
	const FireError fErr = pTechno->GetFireError(pTarget, wpnIdx, true);
	if (fErr != FireError::ILLEGAL
		&& fErr != FireError::CANT
		&& fErr != FireError::MOVING
		&& fErr != FireError::RANGE)
	{
		return pTechno->IsCloseEnough(pTarget, wpnIdx);
	}
	else
		return false;
}

void TechnoExt::JumpjetUnitFacingFix(TechnoClass* pThis)
{
	const auto pType = pThis->GetTechnoType();
	if (pType->Locomotor == LocomotionClass::CLSIDs::Jumpjet && pThis->IsInAir()
		&& pThis->WhatAmI() == AbstractType::Unit && !pType->TurretSpins)
	{
		const auto pFoot = abstract_cast<UnitClass*>(pThis);
		const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
		if (pTypeExt && pTypeExt->JumpjetFacingTarget.Get(RulesExt::Global()->JumpjetFacingTarget)
			&& pFoot && pFoot->GetCurrentSpeed() == 0)
		{
			if (const auto pTarget = pThis->Target)
			{
				const auto pLoco = static_cast<JumpjetLocomotionClass*>(pFoot->Locomotor.get());
				if (pLoco && !pLoco->LocomotionFacing.in_motion() && CanFireAt(pThis, pTarget))
				{
					const CoordStruct source = pThis->Location;
					const CoordStruct target = pTarget->GetCoords();
					const DirStruct tgtDir = DirStruct(Math::arctanfoo(source.Y - target.Y, target.X - source.X));
					if (pThis->GetRealFacing().value32() != tgtDir.value32())
						pLoco->LocomotionFacing.turn(tgtDir);
				}
			}
		}
	}
}

// =============================
// load / save

template <typename T>
void TechnoExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->InterceptedBullet)
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
		.Process(this->IsLeggedCyborg)

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

		.Process(this->PassengerList)
		.Process(this->PassengerlocationList)
		.Process(this->AllowCreatPassenger)
		.Process(this->AllowChangePassenger)
		//.Process(this->ParentAttachment)
		//.Process(this->ChildAttachments)
		;
	Techno_Huge_HP.Clear();
	for (auto& it : Processing_Scripts) delete it;
	FireSelf_Count.clear();
	FireSelf_Weapon.clear();
	FireSelf_ROF.clear();
	Processing_Scripts.clear();
}

void TechnoExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	this->ParentAttachment = nullptr;
	this->ChildAttachments.clear();
	ExistTechnoExt::Array.push_back(this);
	TechnoClass* pOldThis = nullptr;
	Stm.Load(pOldThis);
	//SwizzleManagerClass::Instance->Here_I_Am(reinterpret_cast<long>(pOldThis), this->OwnerObject());
	PointerMapper::AddMapping(reinterpret_cast<long>(pOldThis), reinterpret_cast<long>(this->OwnerObject()));
	//Debug::Log("[TechnoClass] old[0x%X],new[0x%X]\n", pOldThis, this->OwnerObject());
	//Debug::Log("[TechnoExt] Load TechnoExt[0x%X],TechnoClass[0x%X]\n", this, this->OwnerObject());
	Extension<TechnoClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
	if (ExtData::counter == TechnoClass::Array->Count)
	{
		AttachmentClass::LoadGlobals(Stm);
	}
}

void TechnoExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	TechnoClass* pThis = this->OwnerObject();
	Stm.Save(pThis);
	//Debug::Log("[TechnoExt] Save TechnoExt[0x%X],TechnoClass[0x%X]\n", this, this->OwnerObject());
	Extension<TechnoClass>::SaveToStream(Stm);
	this->Serialize(Stm);
	ExtData::counter++;
	if (ExtData::counter == TechnoClass::Array->Count)
	{
		AttachmentClass::SaveGlobals(Stm);
	}
}

bool TechnoExt::LoadGlobals(PhobosStreamReader& Stm)
{
	Stm.Load(TechnoExt::ExtData::counter);
	Stm.Load(TechnoTypeExt::ExtData::counter);
	//Debug::Log("[TechnoClass] Read Counter[%d]\n", TechnoExt::ExtData::counter);
	//Debug::Log("[TechnoTypeClass] Read Counter[%d]\n", TechnoTypeExt::ExtData::counter);
	Techno_Huge_HP.Clear();
	ExistTechnoExt::Array.clear();
	PointerMapper::Map.clear();
	return Stm
		.Success();
}

bool TechnoExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	Stm.Save(TechnoClass::Array->Count);
	Stm.Save(TechnoTypeClass::Array->Count);
	TechnoExt::ExtData::counter = 0;
	TechnoTypeExt::ExtData::counter = 0;
	//Debug::Log("[TechnoClass] Write Counter[%d]\n", TechnoClass::Array->Count);
	//Debug::Log("[TechnoTypeClass] Write Counter[%d]\n", TechnoTypeClass::Array->Count);
	return Stm
		.Success();
}

void TechnoExt::DigitalDisplayHealth(TechnoClass* pThis, Point2D* pLocation)
{//pos use for reference ShieldClass::DrawShieldBar_Building

	TechnoTypeClass* pType = pThis->GetTechnoType();
	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
	auto pExt = TechnoExt::ExtMap.Find(pThis);
	int iLength = pThis->WhatAmI() == AbstractType::Infantry ? 8 : 17;
	Point2D Loc = *pLocation;

	if (pThis->WhatAmI() == AbstractType::Building)
	{
		Loc.X -= 5;
		Loc.Y -= 3;
	}

	Loc.Y -= 5;

	DigitalDisplayTypeClass* pDisplayType = nullptr;
	AbstractType ThisAbstractType = pThis->WhatAmI();

	switch (ThisAbstractType)
	{
	case AbstractType::Building:
		pDisplayType = pTypeExt->DigitalDisplayType.Get(RulesExt::Global()->Buildings_DefaultDigitalDisplayTypeHP.Get());
		break;
	case AbstractType::Infantry:
		pDisplayType = pTypeExt->DigitalDisplayType.Get(RulesExt::Global()->Infantrys_DefaultDigitalDisplayTypeHP.Get());
		break;
	case AbstractType::Unit:
		pDisplayType = pTypeExt->DigitalDisplayType.Get(RulesExt::Global()->Units_DefaultDigitalDisplayTypeHP.Get());
		break;
	case AbstractType::Aircraft:
		pDisplayType = pTypeExt->DigitalDisplayType.Get(RulesExt::Global()->Aircrafts_DefaultDigitalDisplayTypeHP.Get());
		break;
	default:
		break;
	}

	if (pDisplayType == nullptr)
		return;

	Point2D PosH = { 0, 0 };

	if (pThis->WhatAmI() == AbstractType::Building)
	{
		CoordStruct Coords = { 0, 0, 0 };
		pThis->GetTechnoType()->Dimension2(&Coords);

		Point2D Pos2 = { 0, 0 };
		CoordStruct Coords2 = { -Coords.X / 2, Coords.Y / 2, Coords.Z };
		TacticalClass::Instance->CoordsToScreen(&Pos2, &Coords2);

		PosH.X = Pos2.X + Loc.X + 4 * 17 - 110;
		PosH.Y = Pos2.Y + Loc.Y - 2 * 17 + 40;
	}
	else
	{
		if (iLength == 8)
		{
			PosH.X = Loc.X - 15;
			PosH.Y = Loc.Y - 50;
		}
		else
		{
			PosH.X = Loc.X - 20;
			PosH.Y = Loc.Y - 45;
		}
		PosH.Y += pType->PixelSelectionBracketDelta;
		//Building's PixelSelectionBracketDetla is ineffctive

	}

	if (pExt->Shield == nullptr ||
		pExt->Shield->IsBrokenAndNonRespawning())
	{
		PosH.X += pDisplayType->Offset_WithoutShield.Get(pDisplayType->Offset.Get()).X;
		PosH.Y += pDisplayType->Offset_WithoutShield.Get(pDisplayType->Offset.Get()).Y;
	}
	else
	{
		PosH.X += pDisplayType->Offset.Get().X;
		PosH.Y += pDisplayType->Offset.Get().Y;
	}

	bool UseSHPShow = pDisplayType->UseSHP;

	if (UseSHPShow)
	{
		if (pThis->WhatAmI() == AbstractType::Building)
		{
			PosH.X += 10;
			PosH.Y -= 27;
		}
		else
		{
			PosH.X -= 8;
			PosH.Y -= 15;

			if (iLength == 8)
			{
				PosH.X -= 4;
				PosH.Y += 2;
			}
		}
		DigitalDisplaySHPHealth(pThis, pDisplayType, PosH);
	}
	else
	{
		if (pThis->WhatAmI() == AbstractType::Building)
		{
			PosH.X += 35;
			PosH.Y += 5;
		}
		else
		{
			PosH.X += 18;

			if (iLength != 8)
			{
				PosH.X += 4;
			}
		}
		DigitalDisplayTextHealth(pThis, pDisplayType, PosH);
	}
}

void TechnoExt::DigitalDisplayTextHealth(TechnoClass* pThis, DigitalDisplayTypeClass* pDisplayType, Point2D Pos)
{
	wchar_t Healthpoint[0x20];

	if (pDisplayType->Percentage.Get())
		swprintf_s(Healthpoint, L"%d%%", int(pThis->GetHealthPercentage() * 100));
	else if (pDisplayType->HideStrength.Get())
		swprintf_s(Healthpoint, L"%d", pThis->Health);
	else
		swprintf_s(Healthpoint, L"%d/%d", pThis->Health, pThis->GetTechnoType()->Strength);

	COLORREF HPColor;

	if (pThis->IsGreenHP())
		HPColor = Drawing::RGB2DWORD(pDisplayType->Text_ColorHigh.Get());
	else if (pThis->IsYellowHP())
		HPColor = Drawing::RGB2DWORD(pDisplayType->Text_ColorMid.Get());
	else
		HPColor = Drawing::RGB2DWORD(pDisplayType->Text_ColorLow.Get());

	bool ShowBackground = pDisplayType->Text_Background;
	RectangleStruct rect = { 0,0,0,0 };
	DSurface::Temp->GetRect(&rect);
	COLORREF BackColor = 0;
	TextPrintType PrintType;

	switch (pDisplayType->Alignment)
	{
	case DigitalDisplayTypeClass::AlignType::Left:
		PrintType = TextPrintType::NoShadow;
		break;
	case DigitalDisplayTypeClass::AlignType::Right:
		PrintType = TextPrintType::Right;
		break;
	case DigitalDisplayTypeClass::AlignType::Center:
		PrintType = TextPrintType::Center;
	default:
		if (pThis->WhatAmI() == AbstractType::Building)
			PrintType = TextPrintType::Right;
		else
			PrintType = TextPrintType::Center;
		break;
	}

	//0x400 is TextPrintType::Background pr#563 YRpp
	PrintType = TextPrintType(int(PrintType) + (ShowBackground ? 0x400 : 0));

	//DSurface::Temp->DrawText(Healthpoint, vPosH.X, vPosH.Y, ShowHPColor);
	DSurface::Temp->DrawTextA(Healthpoint, &rect, &Pos, HPColor, BackColor, PrintType);
}

void TechnoExt::DigitalDisplaySHPHealth(TechnoClass* pThis, DigitalDisplayTypeClass* pDisplayType, Point2D Pos)
{
	DynamicVectorClass<char>vStrength;
	DynamicVectorClass<char>vHealth;
	const int Length = vStrength.Count + vHealth.Count + 1;
	const Vector2D<int> Interval = (pThis->WhatAmI() == AbstractType::Building ? pDisplayType->SHP_Interval_Building.Get() : pDisplayType->SHP_Interval.Get());
	SHPStruct* SHPFile = pDisplayType->SHPFile;
	ConvertClass* PALFile = pDisplayType->PALFile;
	bool Percentage = pDisplayType->Percentage.Get();
	bool HideStrength = pDisplayType->HideStrength.Get();

	if (SHPFile == nullptr ||
		PALFile == nullptr)
		return;

	if (Percentage)
	{
		vHealth = IntToVector(int(pThis->GetHealthPercentage() * 100));
	}
	else
	{
		vHealth = IntToVector(pThis->Health);

		if (!HideStrength)
			vStrength = IntToVector(pThis->GetTechnoType()->Strength);
	}

	bool LeftToRight = true;

	switch (pDisplayType->Alignment)
	{
	case DigitalDisplayTypeClass::AlignType::Left:
		break;
	case DigitalDisplayTypeClass::AlignType::Right:
	{
		LeftToRight = false;
	}
	break;
	case DigitalDisplayTypeClass::AlignType::Center:
	{
		if (Percentage)
			Pos.X -= (vHealth.Count * Interval.X + Interval.X) / 2;
		else if (HideStrength)
			Pos.X -= (vHealth.Count * Interval.X) / 2;
		else
			Pos.X -= (vHealth.Count * Interval.X + vStrength.Count * Interval.X + Interval.X) / 2;
	}
	break;
	default:
	{
		if (pThis->WhatAmI() != AbstractType::Building)
		{
			if (Percentage)
				Pos.X -= (vHealth.Count * Interval.X + Interval.X) / 2;
			else if (HideStrength)
				Pos.X -= (vHealth.Count * Interval.X) / 2;
			else
				Pos.X -= (vHealth.Count * Interval.X + vStrength.Count * Interval.X + Interval.X) / 2;
		}
	}
	break;
	}

	int base = 0;

	if (pThis->IsYellowHP())
		base = 10;
	else if (pThis->IsRedHP())
		base = 20;

	for (int i = vHealth.Count - 1; i >= 0; i--)
	{
		int num = base + vHealth.GetItem(i);

		if (LeftToRight)
			Pos.X += Interval.X;
		else
			Pos.X -= Interval.X;

		Pos.Y += Interval.Y;

		DSurface::Composite->DrawSHP(PALFile, SHPFile, num, &Pos, &DSurface::ViewBounds,
			BlitterFlags::None, 0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
	}

	if (!Percentage && HideStrength)
		return;

	if (LeftToRight)
		Pos.X += Interval.X;
	else
		Pos.X -= Interval.X;

	Pos.Y += Interval.Y;

	int frame = 30;

	if (base == 10)
		frame = 31;
	else if (base == 20)
		frame = 32;

	if (Percentage)
	{
		frame = 33;

		if (base == 10)
			frame = 34;
		else if (base == 20)
			frame = 35;
	}

	DSurface::Composite->DrawSHP(PALFile, SHPFile, frame, &Pos, &DSurface::ViewBounds,
			BlitterFlags::None, 0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);

	if (Percentage)
		return;

	for (int i = vStrength.Count - 1; i >= 0; i--)
	{
		int num = base + vStrength.GetItem(i);

		if (LeftToRight)
			Pos.X += Interval.X;
		else
			Pos.X -= Interval.X;

		Pos.Y += Interval.Y;

		DSurface::Composite->DrawSHP(PALFile, SHPFile, num, &Pos, &DSurface::ViewBounds,
			BlitterFlags::None, 0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);
	}
}

void TechnoExt::InitialShowHugeHP(TechnoClass* pThis)
{
	auto pType = pThis->GetTechnoType();
	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
	if (pThis->InLimbo || !pTypeExt->HugeHP_Show.Get()) return;
	if (Techno_Huge_HP.AddUnique({ pThis,pTypeExt }))
	{
		//Debug::Log("[HugeHP] New Huge HP Techno: Type[%s],Address[0x%X] \n", pType->get_ID(), pThis);
	}
	std::sort(Techno_Huge_HP.begin(), Techno_Huge_HP.end());
}

void TechnoExt::RunHugeHP()
{
	TechnoClass* pThis = nullptr;

	while (Techno_Huge_HP.Count > 0)
	{
		pThis = Techno_Huge_HP.back()->pThis;
		auto pTypeExt = Techno_Huge_HP.back()->pTypeExt;
		if (pThis->InLimbo || !pTypeExt->HugeHP_Show.Get())
		{
			EraseHugeHP(pThis, pTypeExt);
			pThis = nullptr;
		}
		else break;
	}
	if (pThis != nullptr) TechnoExt::UpdateHugeHP(pThis);
}

void TechnoExt::DetectDeath_HugeHP(TechnoClass* pThis)
{
	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	if (pThis->InLimbo && pTypeExt->HugeHP_Show.Get())
	{
		//Debug::Log("[HugeHP] Detected Death Techno: Type[%s], Address[0x%X]", pThis->GetTechnoType()->get_ID(), pThis);
		EraseHugeHP(pThis, pTypeExt);
	}
}

void TechnoExt::EraseHugeHP(TechnoClass* pThis, TechnoTypeExt::ExtData* pTypeExt)
{
	const int Priority = pTypeExt->HugeHP_Priority.Get();
	//Debug::Log("[HugeHP] Erased Huge HP Techno: Address[0x%X]\n", pThis);
	Techno_Huge_HP.Remove(Techno_With_Type(pThis, pTypeExt));
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
	if (RulesExt::Global()->HugeHP_UseSHPShowBar.Get()) // 激活SHP巨型血条、护盾条，关闭矩形巨型血条、护盾条
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
		if (RulesExt::Global()->HugeHP_UseSHPShowBar.Get()) // 激活SHP巨型血条、护盾条，关闭矩形巨型血条、护盾条
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

void TechnoExt::UpdateFireScript(TechnoClass* pThis)
{
	auto pExt = TechnoExt::ExtMap.Find(pThis);
	std::set<FireScriptClass*> Need_Delete;
	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
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
	//if (pTarget->WhatAmI() == AbstractType::Cell) return;

	//why TechnoClass objects' AbsDerivateID==Abstract::Object???
	//ObjectClass* pTargetObject = abstract_cast<ObjectClass*>(pTarget);
	//if (pTargetObject->AbsDerivateID != AbstractFlags::Techno) return;
	if (pTarget->WhatAmI() != AbstractType::Unit && pTarget->WhatAmI() != AbstractType::Aircraft &&
		pTarget->WhatAmI() != AbstractType::Building && pTarget->WhatAmI() != AbstractType::Infantry) return;
	TechnoClass* pTargetTechno = abstract_cast<TechnoClass*>(pTarget);
	auto pType = pThis->GetTechnoType();
	auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);

	CoordStruct PreSelfLocation = pThis->Location;
	CoordStruct PreTargetLocation = pTargetTechno->Location;
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
		PreSelfLocation = pThis->GetCoords() + CoordStruct { (FoundationX * 256) / 2, (FoundationY * 256) / 2 };
	}
	if (pTarget->WhatAmI() == AbstractType::Building)
	{
		auto const pTargetBuilding = abstract_cast<BuildingClass*>(pTargetTechno);
		int FoundationX = pTargetBuilding->GetFoundationData()->X, FoundationY = pTargetBuilding->GetFoundationData()->Y;
		if (FoundationX > 0)
		{
			FoundationX = 1;
		}
		if (FoundationY > 0)
		{
			FoundationY = 1;
		}
		PreTargetLocation = pTarget->GetCoords() + CoordStruct { (FoundationX * 256) / 2, (FoundationY * 256) / 2 };
	}

	//Debug::Log("Apply Assault Weapon\n");
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
			location = pTargetTechno->GetCoords();

			if (pTarget->WhatAmI() == AbstractType::Building)
			{
				location = PreTargetLocation;
			}

		}
		if (pCell != nullptr)
			location = pCell->GetCoordsWithBridge();
		else
			location.Z = MapClass::Instance->GetCellFloorHeight(location);
		location.Z += iHeight;
		pThis->SetLocation(location);
		pThis->ForceMission(Mission::Stop);
		pThis->Guard();
	}
	if (pWeaponExt->InvBlinkWeapon.Get() && pTarget->WhatAmI() != AbstractType::Building)
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
		auto pTargetTechnoType = pTargetTechno->GetTechnoType();
		if (pWeaponExt->BlinkWeapon_Overlap.Get())
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
			location = pThis->GetCoords();

			if (pThis->WhatAmI() == AbstractType::Building)
			{
				location = PreSelfLocation;
			}

		}
		if (pCell != nullptr)
			location = pCell->GetCoordsWithBridge();
		else
			location.Z = MapClass::Instance->GetCellFloorHeight(location);
		location.Z += iHeight;
		pTargetTechno->SetLocation(location);
		pTargetTechno->ForceMission(Mission::Stop);
		pTargetTechno->Guard();
	}
	if (pWeaponExt->BlinkWeapon_KillTarget.Get())
	{
		pTargetTechno->ReceiveDamage(&pTargetTechno->Health, 0, pWeapon->Warhead, pThis, true, false, pThis->Owner);
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
            pData->ShowAnim_LastActivatedFrame = - pReceiveDamageAnimType->ShowAnim_CoolDown;
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
	//Debug::Log("[TechnoClass] Create Techno[0x%X]\n", pItem);

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
