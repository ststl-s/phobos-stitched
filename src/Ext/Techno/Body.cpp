#include "Body.h"

#include <BitFont.h>
#include <JumpjetLocomotionClass.h>

#include <Helpers/Macro.h>

#include <Ext/Abstract/AbstractExt.h>
#include <Ext/Building/Body.h>
#include <Ext/House/Body.h>
#include <Ext/HouseType/Body.h>
#include <Ext/Object/ObjectExt.h>
#include <Ext/Script/Body.h>
#include <Ext/Team/Body.h>
#include <Ext/Techno/AresExtData.h>
#include <Ext/Anim/Body.h>

#include <New/Armor/Armor.h>
#include <New/Type/TemperatureTypeClass.h>

#include <Misc/FlyingStrings.h>
#include <Misc/GScreenDisplay.h>
#include <Misc/PhobosGlobal.h>
#include <Misc/CaptureManager.h>

#include <Utilities/EnumFunctions.h>
#include <Utilities/Helpers.Alex.h>
#include <Utilities/ShapeTextPrinter.h>
#include <Utilities/TemplateDef.h>

template<> const DWORD Extension<TechnoClass>::Canary = 0x55555555;
TechnoExt::ExtContainer TechnoExt::ExtMap;

bool __fastcall TechnoExt::IsReallyAlive(const ObjectClass* const pThis)
{
	bool alive = pThis
		&& pThis->IsAlive
		&& pThis->Health > 0
		&& ObjectExt::ExtMap.Find(pThis) != nullptr
		;

	return alive;
}

bool __fastcall TechnoExt::IsActive(const TechnoClass* const pThis)
{
	return TechnoExt::IsReallyAlive(pThis)
		&& !pThis->InLimbo
		&& !pThis->TemporalTargetingMe
		&& !pThis->BeingWarpedOut
		&& !pThis->IsUnderEMP();
}

bool __fastcall TechnoExt::IsActivePower(const TechnoClass* const pThis)
{
	bool active = IsActive(pThis);

	if (const auto pBuilding = abstract_cast<const BuildingClass* const>(pThis))
		active &= pBuilding->IsPowerOnline();

	return active;
}

int __fastcall TechnoExt::GetSizeLeft(FootClass* const pFoot)
{
	return pFoot->GetTechnoType()->Passengers - pFoot->Passengers.GetTotalSize();
}

void __fastcall TechnoExt::Stop(TechnoClass* pThis, Mission eMission)
{
	pThis->ForceMission(eMission);
	pThis->CurrentTargets.Clear();
	pThis->SetTarget(nullptr);
	pThis->SetFocus(nullptr);
	pThis->SetDestination(nullptr, true);
}

bool __fastcall TechnoExt::CanICloakByDefault(TechnoClass* pThis)
{
	if (!IsReallyAlive(pThis))
		return false;

	auto pType = pThis->GetTechnoType();
	return pType->Cloakable || pThis->HasAbility(Ability::Cloak);
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

			if (pObjectKiller && Phobos::Debug_DisplayKillMsg)
			{
				wchar_t msg[0x100] = L"\0";
				const wchar_t* strKiller = pObjectKiller->GetTechnoType()->UIName;
				const wchar_t* strVictim = pVictim->GetTechnoType()->UIName;
				swprintf_s(msg, L"%ls %ls %ls", strKiller, Phobos::UI::KillLabel, strVictim);

				if (HouseClass::CurrentPlayer->IsObserver() || !RulesExt::Global()->KillMessageDisplay_OnlySelf)
				{
					// MessageListClass::Instance->PrintMessage(msg, RulesClass::Instance->MessageDelay, HouseClass::CurrentPlayer->ColorSchemeIndex, true);
					MessageListClass::Instance->PrintMessage(msg, RulesClass::Instance->MessageDelay, pObjectKiller->Owner->ColorSchemeIndex, true);
				}
				else
				{
					if (pObjectKiller->Owner == HouseClass::CurrentPlayer || pVictim->Owner == HouseClass::CurrentPlayer)
					{
						if (RulesExt::Global()->KillMessageDisplay_Type == ShowMessageHouse::All ||
							(RulesExt::Global()->KillMessageDisplay_Type == ShowMessageHouse::Invoker && pObjectKiller->Owner == HouseClass::CurrentPlayer) ||
							(RulesExt::Global()->KillMessageDisplay_Type == ShowMessageHouse::Victim && pVictim->Owner == HouseClass::CurrentPlayer))
						{
							MessageListClass::Instance->PrintMessage(msg, RulesClass::Instance->MessageDelay, pObjectKiller->Owner->ColorSchemeIndex, true);
						}
					}
				}

			}

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
				if (pItem == nullptr || pItem->Status == SpawnNodeStatus::Dead)
					continue;

				if (pItem->Techno->Passengers.NumPassengers == 0 &&
					!(pItem->Status == SpawnNodeStatus::Idle || pItem->Status == SpawnNodeStatus::Reloading))
				{
					pItem->Techno->Ammo = 0;
				}

				if (pThis->Passengers.NumPassengers > 0)
				{
					FootClass* pPassenger = pThis->Passengers.GetFirstPassenger();
					FootClass* pItemPassenger = pItem->Techno->Passengers.GetFirstPassenger();
					auto pItemType = pItem->Techno->GetTechnoType();

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

						if (passengerType->Size <= (pItemType->Passengers - pItem->Techno->Passengers.GetTotalSize()) && passengerType->Size <= pItemType->SizeLimit)
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
									pItem->Techno->Passengers.FirstPassenger = pPassenger;

								++pItem->Techno->Passengers.NumPassengers;

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

void TechnoExt::SpawneLoseTarget(TechnoClass* pThis)
{
	auto const pData = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	if (pData && pData->Spawner_SameLoseTarget)
	{
		auto pExt = TechnoExt::ExtMap.Find(pThis);
		pExt->SpawneLoseTarget = false;
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

	if (TechnoExt::HasAvailableDock(pThis))
	{
		switch (pThis->GetCurrentMission())
		{
		case Mission::Harvest:
		case Mission::Unload:
		case Mission::Enter:
			return true;
		case Mission::Guard: // issue#603: not exactly correct, but idk how to do better
			if (auto pUnit = abstract_cast<UnitClass*>(pThis))
				return pUnit->IsHarvesting || pUnit->Locomotor->Is_Really_Moving_Now() || pUnit->HasAnyLink();
		default:
			return false;
		}
	}

	return false;
}

void TechnoExt::ShareWeaponRange(TechnoClass* pThis, AbstractClass* pTarget, WeaponTypeClass* pWeapon)
{
	if (!TechnoExt::IsReallyAlive(pThis))
		return;

	if (auto pTargetObject = abstract_cast<ObjectClass*>(pTarget))
	{
		if (!TechnoExt::IsReallyAlive(pTargetObject))
			return;
	}
	else
	{
		if (!AbstractExt::IsAbstract(pTarget))
			return;
	}

	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (!pTypeExt->WeaponRangeShare_Technos.empty() && pTypeExt->WeaponRangeShare_Range > 0)
	{
		auto pExt = TechnoExt::ExtMap.Find(pThis);
		pExt->IsSharingWeaponRange = true;
		const std::vector<TechnoClass*> vTechnos(std::move(Helpers::Alex::getCellSpreadItems(pThis->GetCoords(), pTypeExt->WeaponRangeShare_Range, true)));

		for (TechnoClass* pAffect : vTechnos)
		{
			auto pAffectExt = TechnoExt::ExtMap.Find(pAffect);

			if (pTypeExt->WeaponRangeShare_Technos.Contains(pAffect->GetTechnoType()) &&
				pThis->Owner == pAffect->Owner &&
				pAffect != pThis &&
				!pAffectExt->IsSharingWeaponRange)
			{
				if (!pTypeExt->WeaponRangeShare_ForceAttack)
				{
					if (pAffect->GetTechnoType()->JumpJet)
					{
						FootClass* pFoot = abstract_cast<FootClass*>(pAffect);
						const auto pLoco = static_cast<JumpjetLocomotionClass*>(pFoot->Locomotor.get());

						if (!(pAffect->GetCurrentMission() == Mission::Guard ||	pAffect->GetCurrentMission() == Mission::Move && !pLoco->LocomotionFacing.Is_Rotating()))
							continue;
					}
					else
					{
						if (pAffect->GetCurrentMission() != Mission::Guard)
							continue;
					}
				}

				auto pAffectTypeExt = TechnoTypeExt::ExtMap.Find(pAffect->GetTechnoType());

				if ((pAffect->GetFireErrorWithoutRange(pThis->Target, pAffectTypeExt->WeaponRangeShare_UseWeapon) != FireError::OK &&
					pAffect->GetFireErrorWithoutRange(pThis->Target, pAffectTypeExt->WeaponRangeShare_UseWeapon) != FireError::FACING) ||
					pAffect->DiskLaserTimer.GetTimeLeft() > 0)
					continue;

				ShareWeaponRangeFire(pAffect, pTarget);
			}
		}

		pExt->IsSharingWeaponRange = false;
	}
}

void TechnoExt::ShareWeaponRangeFire(TechnoClass* pThis, AbstractClass* pTarget)
{
	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	auto const pExt = TechnoExt::ExtMap.Find(pThis);

	CoordStruct& source = pThis->Location;
	CoordStruct target = pTarget->GetCoords();
	DirStruct tgtDir = DirStruct { Math::atan2(source.Y - target.Y, target.X - source.X) };

	if (!pThis->GetWeapon(pTypeExt->WeaponRangeShare_UseWeapon)->WeaponType->OmniFire)
	{
		if (pThis->HasTurret())
		{
			if (pThis->TurretFacing().GetFacing<32>() != tgtDir.GetFacing<32>())
			{
				pThis->SecondaryFacing.SetDesired(tgtDir);
				pThis->PrimaryFacing.SetDesired(tgtDir);
				pExt->ShareWeaponRangeTarget = pTarget;
				pExt->ShareWeaponRangeFacing = tgtDir;
				return;
			}
		}
		else
		{
			if (pThis->GetRealFacing().GetFacing<32>() != tgtDir.GetFacing<32>())
			{
				pThis->PrimaryFacing.SetDesired(tgtDir);
				pExt->ShareWeaponRangeTarget = pTarget;
				pExt->ShareWeaponRangeFacing = tgtDir;
				return;
			}
		}
	}

	SimulatedFire(pThis, *pThis->GetWeapon(pTypeExt->WeaponRangeShare_UseWeapon), pTarget);

	int rofBuff;
	double rofMulti = pExt->GetAEROFMul(&rofBuff) * pThis->Owner->ROFMultiplier * pThis->AresExtData->ROFMultiplier;

	pThis->DiskLaserTimer.Start(Game::F2I(pThis->GetWeapon(pTypeExt->WeaponRangeShare_UseWeapon)->WeaponType->ROF * rofMulti) + rofBuff);

	if (pExt->ShareWeaponRangeTarget != nullptr)
	{
		pExt->ShareWeaponRangeTarget = nullptr;
	}
}

void TechnoExt::ReceiveShareDamage(TechnoClass* pThis, args_ReceiveDamage* args, std::vector<TechnoClass*>& teamTechnos)
{
	for (TechnoClass* pTechno : teamTechnos)
	{
		if (pTechno != pThis)
		{
			pTechno->TakeDamage(*args->Damage, args->SourceHouse, args->Attacker, args->WH);
		}
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

void TechnoExt::FireWeaponAtSelf(TechnoClass* pThis, WeaponTypeClass* pWeaponType)
{
	WeaponTypeExt::DetonateAt(pWeaponType, pThis, pThis);
}

Matrix3D TechnoExt::GetTransform(TechnoClass* pThis, VoxelIndexKey* pKey, bool isShadow)
{
	Matrix3D mtx;

	if ((pThis->AbstractFlags & AbstractFlags::Foot) && ((FootClass*)pThis)->Locomotor)
		mtx = isShadow ? ((FootClass*)pThis)->Locomotor->Shadow_Matrix(pKey) : ((FootClass*)pThis)->Locomotor->Draw_Matrix(pKey);
	else // no locomotor means no rotation or transform of any kind (f.ex. buildings) - Kerbiter
		mtx.MakeIdentity();

	return mtx;
}

Matrix3D TechnoExt::TransformFLHForTurret(TechnoClass* pThis, Matrix3D mtx, bool isOnTurret, double factor)
{
	auto const pType = pThis->GetTechnoType();

	// turret offset and rotation
	if (isOnTurret && pThis->HasTurret())
	{
		TechnoTypeExt::ApplyTurretOffset(pType, &mtx, factor);

		double turretRad = pThis->TurretFacing().GetRadian<32>();
		double bodyRad = pThis->PrimaryFacing.Current().GetRadian<32>();
		float angle = (float)(turretRad - bodyRad);

		mtx.RotateZ(angle);
	}

	return mtx;
}

Matrix3D TechnoExt::GetFLHMatrix(TechnoClass* pThis, CoordStruct pCoord, bool isOnTurret, double factor, bool isShadow)
{
	Matrix3D transform = TechnoExt::GetTransform(pThis, nullptr, isShadow);
	Matrix3D mtx = TechnoExt::TransformFLHForTurret(pThis, transform, isOnTurret, factor);

	CoordStruct scaledCoord = pCoord * factor;
	// apply FLH offset
	mtx.Translate((float)scaledCoord.X, (float)scaledCoord.Y, (float)scaledCoord.Z);

	return mtx;
}

// 为了不改变其他的东西。
CoordStruct TechnoExt::GetFLHAbsoluteCoords(TechnoClass* pThis, CoordStruct pCoord, bool isOnTurret)
{
	return TechnoExt::GetFLHAbsoluteCoords(pThis, pCoord, pThis->GetCoords(), isOnTurret);
}

// reversed from 6F3D60
CoordStruct TechnoExt::GetFLHAbsoluteCoords(TechnoClass* pThis, CoordStruct flh, CoordStruct oriflh, bool isOnTurret)
{
	auto result = TechnoExt::GetFLHMatrix(pThis, flh, isOnTurret) * Vector3D<float>::Empty;

	// Resulting coords are mirrored along X axis, so we mirror it back
	result.Y *= -1;

	// apply as an offset to global object coords
	CoordStruct location = oriflh;
	location += { std::lround(result.X), std::lround(result.Y), std::lround(result.Z) };

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

void TechnoExt::FirePassenger(TechnoClass* pThis, WeaponTypeExt::ExtData* pWeaponExt)
{
	if (pWeaponExt->PassengerDeletion)
	{
		auto pTechnoData = TechnoExt::ExtMap.Find(pThis);

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
					pTechnoData->SendPassenger = pPassenger;
					pTechnoData->SendPassengerData = pWeaponExt->PassengerTransport_UsePassengerData;
					pTechnoData->SendPassengerMove = pWeaponExt->PassengerTransport_MoveToTarget;
					pTechnoData->SendPassengerMoveHouse = pWeaponExt->PassengerTransport_MoveToTargetAllowHouses;
					pTechnoData->SendPassengerOverlap = pWeaponExt->PassengerTransport_Overlap;
					pTechnoData->SendPassengerUseParachute = pWeaponExt->PassengerTransport_UseParachute;
					pTechnoData->SendPassengerMoveHouse_IgnoreNeturalHouse = pWeaponExt->PassengerTransport_MoveToTargetAllowHouses_IgnoreNeturalHouse;
				}
				else
				{
					pPassenger->KillPassengers(pThis);
					pPassenger->RegisterDestruction(pThis);
					pPassenger->UnInit();
				}
			}
		}
	}
}

void TechnoExt::FireSelf(TechnoClass* pThis, WeaponTypeExt::ExtData* pWeaponExt)
{
	if (pWeaponExt->SelfTransport && !pThis->InLimbo)
	{
		auto pTechnoData = TechnoExt::ExtMap.Find(pThis);

		if (pWeaponExt->SelfTransport_Anim)
		{
			AnimClass* anim = GameCreate<AnimClass>(pWeaponExt->SelfTransport_Anim, pThis->GetCoords());
			anim->Owner = pThis->Owner;
		}

		pTechnoData->SendPassenger = static_cast<FootClass*>(pThis);
		pTechnoData->SendPassengerData = pWeaponExt->SelfTransport_UseData;
		pTechnoData->SendPassengerMove = pWeaponExt->SelfTransport_MoveToTarget;
		pTechnoData->SendPassengerMoveHouse = pWeaponExt->SelfTransport_MoveToTargetAllowHouses;
		pTechnoData->SendPassengerMoveHouse_IgnoreNeturalHouse = pWeaponExt->SelfTransport_MoveToTargetAllowHouses_IgnoreNeturalHouse;
		pTechnoData->SendPassengerOverlap = pWeaponExt->SelfTransport_Overlap;
		pTechnoData->SendPassengerSelect = pThis->IsSelected;
		pTechnoData->SendPassengerUseParachute = pWeaponExt->SelfTransport_UseParachute;
		pThis->Limbo();
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
				const DirStruct tgtDir = DirStruct(Math::atan2(source.Y - target.Y, target.X - source.X));
				pThis->PrimaryFacing.SetDesired(tgtDir);
			}
			pThis->SecondaryFacing.SetDesired(pThis->PrimaryFacing.Current());
		}
	}
}

void TechnoExt::TechnoGattlingCount(TechnoClass* pThis, TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt)
{
	if (!pExt->HasCharged)
	{
		if (!pThis->DiskLaserTimer.Completed())
		{
			if (auto const pInf = abstract_cast<InfantryClass*>(pThis))
			{
				if (pInf->Type->FireUp > 0)
					pExt->FireUpTimer.Start(pInf->Type->FireUp + pThis->DiskLaserTimer.TimeLeft);
			}

			pExt->GattlingCount += pThis->GetTechnoType()->RateUp;
			if (pExt->GattlingCount > pExt->MaxGattlingCount)
				pExt->GattlingCount = pExt->MaxGattlingCount;
		}
		else
		{
			if (pExt->FireUpTimer.InProgress())
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
}

void TechnoExt::SetGattlingCount(TechnoClass* pThis, AbstractClass* pTarget, WeaponTypeClass* pWeapon)
{
	auto pExt = TechnoExt::ExtMap.Find(pThis);
	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (pTypeExt->IsExtendGattling && !pThis->GetTechnoType()->IsGattling)
	{
		pExt->GattlingCount += pThis->GetTechnoType()->RateUp;

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
			if (!pThis->DiskLaserTimer.Completed() && pExt->HasCharged && !pExt->IsChargeROF)
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

	pExt->GattlingStages = pTypeExt->Stages;

	if (pThis->Veterancy.IsVeteran())
	{
		pExt->GattlingStages = pTypeExt->VeteranStages;
	}
	else if (pThis->Veterancy.IsElite())
	{
		pExt->GattlingStages = pTypeExt->EliteStages;
	}

	auto& stages = pExt->GattlingStages;

	pExt->MaxGattlingCount = stages[stages.size() - 1].GetItem(0);

	for (size_t i = 0; i < stages.size(); i++)
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
		pThis->Stun(); // Stun? what is this?
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
				BuildingExt::ExtMap.Find(pBld)->SellingForbidden = false;
				pBld->Sell(true);

				return;
			}
		}

		if (Phobos::Config::DevelopmentCommands)
			Debug::Log("[Runtime Warning] %s can't be sold, killing it instead\n", pThis->get_ID());
	}
	default:
		// Due to Ares, ignoreDefense=true will prevent passenger/crew/hijacker from escaping
		pThis->TakeDamage(pThis->Health, pThis->Owner);
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

void TechnoExt::BeamCannon(TechnoClass* pThis, AbstractClass* pTarget, WeaponTypeClass* pWeapon)
{
	auto pData = TechnoExt::ExtMap.Find(pThis);
	auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);

	pData->setBeamCannon = pWeapon;

	if (pWeaponExt->IsBeamCannon)
	{
		if (pData->BeamCannon_Stop)
		{
			if (auto pCell = abstract_cast<CellClass*>(pTarget))
			{
				pData->BeamCannon_Target = pCell->GetCenterCoords();
				for (int i = 0; i < pWeaponExt->BeamCannon_Burst; i++)
				{
					pData->BeamCannon_Center.emplace_back(TechnoExt::GetFLHAbsoluteCoords(pThis, pWeaponExt->BeamCannon_Start[i], pThis->HasTurret()));
				}
				pData->BeamCannon_Stop = false;
			}
			else
			{
				pData->BeamCannon_Target = pTarget->GetCoords();
				for (int i = 0; i < pWeaponExt->BeamCannon_Burst; i++)
				{
					pData->BeamCannon_Center.emplace_back(TechnoExt::GetFLHAbsoluteCoords(pThis, pWeaponExt->BeamCannon_Start[i], pThis->HasTurret()));
				}
				pData->BeamCannon_Stop = false;
			}
		}
	}
}

void TechnoExt::InitializeJJConvert(TechnoClass* pThis)
{
	if (pThis->WhatAmI() != AbstractType::Unit)
		return;

	auto pExt = ExtMap.Find(pThis);
	TechnoTypeClass* pType = pThis->GetTechnoType();
	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

	if (pTypeExt->JJConvert_Unload != nullptr)
	{
		pExt->NeedConvertWhenLanding = true;
		pExt->LandingType = pTypeExt->JJConvert_Unload;
		pExt->FloatingType = static_cast<UnitTypeClass*>(pType);
	}
}

void TechnoExt::SelectIFVWeapon(TechnoClass* pThis, TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt)
{
	if (pThis->WhatAmI() != AbstractType::Building)
		return;

	TechnoTypeClass* pType = pThis->GetTechnoType();
	pExt->IFVTurrets = pTypeExt->Turrets;

	auto& turrets = pExt->IFVTurrets;

	if (pThis->Passengers.NumPassengers > 0)
	{
		auto pFirstType = pThis->Passengers.FirstPassenger->GetTechnoType();
		if (pFirstType->IFVMode < pType->WeaponCount)
			pExt->IFVMode = pFirstType->IFVMode;
		else
			pExt->IFVMode = 0;

		if (pFirstType->IFVMode < pType->TurretCount)
			pThis->CurrentTurretNumber = turrets[pFirstType->IFVMode].GetItem(0);
		else
			pThis->CurrentTurretNumber = turrets[0].GetItem(0);
	}
	else
	{
		pExt->IFVMode = 0;
		pThis->CurrentTurretNumber = turrets[0].GetItem(0);
	}

	pThis->UpdatePlacement(PlacementType::Redraw);
}

void TechnoExt::BuildingPassengerFix(TechnoClass* pThis)
{
	if (pThis->WhatAmI() != AbstractType::Building)
		return;

	TechnoTypeClass* pType = pThis->GetTechnoType();

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
		pTargetExt->Attacker_Weapon = pWeapon;
		pTargetExt->Attacker_Count = pWeaponExt->OnlyAllowOneFirer_Count > 0 ? pWeaponExt->OnlyAllowOneFirer_Count : pWeapon->ROF;
	}
}

void TechnoExt::BuildingSpawnFix(TechnoClass* pThis)
{
	if (auto const pBuilding = abstract_cast<BuildingClass*>(pThis))
	{
		auto pManager = pBuilding->SpawnManager;

		if (pManager != nullptr)
		{
			for (auto pItem : pManager->SpawnedNodes)
			{
				if (pItem->Status == SpawnNodeStatus::Returning
					&& pItem->Techno != nullptr
					&& pItem->Techno->GetHeight() == 0)
				{
					auto FoundationY = pBuilding->Type->GetFoundationHeight(true), FoundationX = pBuilding->Type->GetFoundationWidth();

					if (FoundationX < 0)
						FoundationX = 0;

					if (FoundationY < 0)
						FoundationY = 0;

					auto adjust = pThis->GetCoords() - CoordStruct { (FoundationX - 1) * 128, (FoundationY - 1) * 128 };

					pItem->Techno->SetLocation(adjust);
				}
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

void TechnoExt::ChangeAmmo(TechnoClass* pThis, int ammo)
{
	if (pThis->Ammo - ammo > pThis->GetTechnoType()->Ammo)
	{
		pThis->Ammo = pThis->GetTechnoType()->Ammo;
	}
	else if (pThis->Ammo - ammo < 0)
	{
		pThis->Ammo = 0;
	}
	else
	{
		pThis->Ammo -= ammo;
	}

	if (!pThis->ReloadTimer.HasStarted())
	{
		pThis->StartReloading();
	}
}

void TechnoExt::InfantryOnWaterFix(TechnoClass* pThis)
{
	if (pThis->WhatAmI() != AbstractType::Infantry)
		return;

	auto pCell = pThis->GetCell();
	auto pType = pThis->GetTechnoType();

	if (pCell->ContainsBridge() && pThis->IsOnBridge())
		return;

	if (pCell->Tile_Is_Water() && pThis->GetHeight() == 0)
	{
		if (pCell->OverlayTypeIndex >= 0)
		{
			if (OverlayTypeClass::Array->GetItem(pCell->OverlayTypeIndex)->LandType != LandType::Water)
				return;
		}

		if (!(pType->SpeedType == SpeedType::Hover ||
			pType->SpeedType == SpeedType::Winged ||
			pType->SpeedType == SpeedType::Float ||
			pType->SpeedType == SpeedType::Amphibious))
		{
			// TechnoExt::KillSelf(pThis, AutoDeathBehavior::Kill);

			VocClass::PlayAt(pType->DieSound.GetItem(0), pThis->GetCoords());

			int count = RulesClass::Instance->SplashList.Count - 1;
			int seed = ScenarioClass::Instance->Random.RandomRanged(0, count);

			if (const auto pAnim = GameCreate<AnimClass>(RulesClass::Instance->SplashList.GetItem(seed), pThis->GetCoords()))
			{
				pAnim->Owner = pThis->Owner;
				if (const auto pAnimExt = AnimExt::ExtMap.Find(pAnim))
				{
					pAnimExt->Invoker = pThis;
				}
			}

			TechnoExt::KillSelf(pThis, AutoDeathBehavior::Vanish);
		}
	}
}

void TechnoExt::FallRateFix(TechnoClass* pThis)
{
	if (pThis->FallRate != 0 && !pThis->IsFallingDown)
		pThis->FallRate = 0;
}

bool TechnoExt::AttachmentAI(TechnoClass* pThis)
{
	auto const pExt = TechnoExt::ExtMap.Find(pThis);
	// auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

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

	for (const auto& entry : pTypeExt->AttachmentData)
	{
		if (entry->Type->MaxCount > 0)
		{
			int count = 0;
			for (size_t j = 0; j < pExt->ChildAttachments.size(); j++)
			{
				if (pExt->ChildAttachments[j]->GetType() == entry->Type)
				{
					count++;

					if (count >= entry->Type->MaxCount)
						break;
				}
			}
			if (count >= entry->Type->MaxCount)
				continue;
		}

		pExt->ChildAttachments.push_back(std::make_unique<AttachmentClass>(entry.get(), pThis, nullptr, pThis->Owner));
		pExt->ChildAttachments.back()->Initialize();
	}
}

void TechnoExt::DestroyAttachments(TechnoClass* pThis, TechnoClass* pSource)
{
	auto const& pExt = TechnoExt::ExtMap.Find(pThis);

	for (auto const& pAttachment : pExt->ChildAttachments)
		pAttachment->Destroy(pSource);

	if (pExt->ParentAttachment && pExt->ParentAttachment->GetType()->InheritDestruction_Parent)
		pExt->ParentAttachment->DestroyParent(pSource);
}

void TechnoExt::HandleDestructionAsChild(TechnoClass* pThis)
{
	auto const& pExt = TechnoExt::ExtMap.Find(pThis);

	if (pExt->ParentAttachment)
		pExt->ParentAttachment->ChildDestroyed();

	pExt->ParentAttachment = nullptr;
}

void TechnoExt::UnlimboAttachments(TechnoClass* pThis)
{
	auto const pExt = TechnoExt::ExtMap.Find(pThis);
	for (auto const& pAttachment : pExt->ChildAttachments)
		pAttachment->Unlimbo();
}

void TechnoExt::LimboAttachments(TechnoClass* pThis)
{
	auto const pExt = TechnoExt::ExtMap.Find(pThis);
	for (auto const& pAttachment : pExt->ChildAttachments)
		pAttachment->Limbo();
}

void TechnoExt::AttachmentsAirFix(TechnoClass* pThis)
{
	auto const pExt = TechnoExt::ExtMap.Find(pThis);
	if (pExt->ChildAttachments.empty())
		return;

	if (pExt->ParentInAir == pThis->IsInAir())
		return;

	for (auto const& pAttachment : pExt->ChildAttachments)
	{
		if (!pAttachment->Child)
			continue;

		bool selected = pAttachment->Child->IsSelected;
		pAttachment->Limbo();
		pAttachment->Unlimbo();
		// if (pAttachment->Child->WhatAmI() != AbstractType::Building)
			// ChangeLocomotorTo(pAttachment->Child, LocomotionClass::CLSIDs::Hover.get());
		//pAttachment->Child->InAir = pThis->IsInAir();
		if (selected)
		{
			pAttachment->Child->Select();
		}
	}
	pExt->ParentInAir = pThis->IsInAir();
}

void TechnoExt::AttachmentsRestore(TechnoClass* pThis)
{
	auto const pExt = TechnoExt::ExtMap.Find(pThis);
	for (size_t i = 0; i < pExt->ChildAttachments.size(); i++)
	{
		if (pExt->ChildAttachments[i]->Child)
			continue;

		if (pExt->ChildAttachments[i]->GetType()->RestoreDelay < 0)
		{
			for (size_t j = 0; j < pExt->AddonAttachmentData.size(); j++)
			{
				if (pExt->ChildAttachments[i]->Data == pExt->AddonAttachmentData[j].get())
				{
					pExt->AddonAttachmentData.erase(pExt->AddonAttachmentData.begin() + j);
					break;
				}
			}
			pExt->ChildAttachments.erase(pExt->ChildAttachments.begin() + i);
			continue;
		}

		if (pExt->ChildAttachments[i]->RestoreCount > 0)
		{
			pExt->ChildAttachments[i]->RestoreCount--;
		}
		else
		{
			pExt->ChildAttachments[i]->CreateChild();
			pExt->ChildAttachments[i]->RestoreCount = pExt->ChildAttachments[i]->GetType()->RestoreDelay;
		}
	}
}

void TechnoExt::AttachSelfToTargetAttachments(TechnoClass* pThis, AbstractClass* pTarget, WeaponTypeClass* pWeapon, HouseClass* pHouse)
{
	TechnoClass* pTargetTechno = abstract_cast<TechnoClass*>(pTarget);

	if (pTargetTechno == nullptr || pTargetTechno == pThis)
		return;

	auto const pExt = TechnoExt::ExtMap.Find(pThis);
	if (pExt->ParentAttachment)
		return;

	auto const pTargetExt = TechnoExt::ExtMap.Find(pTargetTechno);
	auto const pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);

	if (pWeaponExt->AttachAttachment_Type->MaxCount > 0)
	{
		int count = 0;
		for (size_t i = 0; i < pTargetExt->ChildAttachments.size(); i++)
		{
			if (pTargetExt->ChildAttachments[i]->GetType() == pWeaponExt->AttachAttachment_Type)
			{
				count++;

				if (count >= pWeaponExt->AttachAttachment_Type->MaxCount)
					return;
			}
		}
	}

	std::unique_ptr<TechnoTypeExt::ExtData::AttachmentDataEntry> TempAttachment = nullptr;
	TempAttachment.reset(new TechnoTypeExt::ExtData::AttachmentDataEntry(pWeaponExt->AttachAttachment_Type, TechnoTypeClass::Array->GetItem(pThis->GetTechnoType()->GetArrayIndex()), pWeaponExt->AttachAttachment_FLH, pWeaponExt->AttachAttachment_IsOnTurret));
	pTargetExt->AddonAttachmentData.emplace_back(std::move(TempAttachment));
	pThis->Limbo();
	pTargetExt->ChildAttachments.push_back(std::make_unique<AttachmentClass>(pTargetExt->AddonAttachmentData.back().get(), pTargetTechno, nullptr, pHouse));
	pTargetExt->ChildAttachments.back()->RestoreCount = pTargetExt->ChildAttachments.back()->GetType()->RestoreDelay;
	pTargetExt->ChildAttachments.back()->OriginFLH = pTargetExt->ChildAttachments.back()->Data->FLH;
	pTargetExt->ChildAttachments.back()->SetFLHoffset();
	pTargetExt->ChildAttachments.back()->AttachChild(pThis);
	pTargetExt->ChildAttachments.back()->Unlimbo();
}

void TechnoExt::MoveTargetToChild(TechnoClass* pThis)
{
	if (FootClass* pFoot = abstract_cast<FootClass*>(pThis))
	{
		if (auto pTarget = abstract_cast<TechnoClass*>(pFoot->Target))
		{
			auto pTargetExt = TechnoExt::ExtMap.Find(pTarget);
			if (pTargetExt->ParentAttachment && pTargetExt->ParentAttachment->GetType()->MoveTargetToParent)
			{
				pFoot->SetTarget(pTargetExt->ParentAttachment->Parent);
			}
		}
	}
}

bool TechnoExt::IsAttached(TechnoClass* pThis)
{
	auto const& pExt = TechnoExt::ExtMap.Find(pThis);
	return pExt && pExt->ParentAttachment;
}

bool TechnoExt::IsChildOf(TechnoClass* pThis, TechnoClass* pParent, bool deep)
{
	auto const pThisExt = TechnoExt::ExtMap.Find(pThis);

	return pThis && pThisExt && pParent  // sanity check, sometimes crashes because ext is null - Kerbiter
		&& pThisExt->ParentAttachment
		&& (pThisExt->ParentAttachment->Parent == pParent
			|| (deep && TechnoExt::IsChildOf(pThisExt->ParentAttachment->Parent, pParent)));
}

// Returns this if no parent.
TechnoClass* TechnoExt::GetTopLevelParent(TechnoClass* pThis)
{
	auto const pThisExt = TechnoExt::ExtMap.Find(pThis);

	return pThis && pThisExt  // sanity check, sometimes crashes because ext is null - Kerbiter
		&& pThisExt->ParentAttachment
		? TechnoExt::GetTopLevelParent(pThisExt->ParentAttachment->Parent)
		: pThis;
}

Matrix3D TechnoExt::GetAttachmentTransform(TechnoClass* pThis, VoxelIndexKey* pKey, bool isShadow)
{
	auto const pThisExt = TechnoExt::ExtMap.Find(pThis);

	if (pThis && pThisExt && pThisExt->ParentAttachment)
		return pThisExt->ParentAttachment->GetUpdatedTransform(pKey, isShadow);

	return TechnoExt::GetTransform(pThis, pKey, isShadow);
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

void TechnoExt::DrawSelfHealPips(TechnoClass* pThis, Point2D* pLocation, RectangleStruct* pBounds)
{
	bool drawPip = false;
	bool isInfantryHeal = false;
	int selfHealFrames = 0;

	const auto pExt = TechnoExt::ExtMap.Find(pThis);

	if (!pExt || !pExt->TypeExtData)
		return;

	Nullable<Point2D> selfheal_pips;
	Nullable<Point2D> selfheal_offset;

	if (auto const pTypeExt = pExt->TypeExtData)
	{
		if (pTypeExt->SelfHealGainType.isset() && pTypeExt->SelfHealGainType.Get() == SelfHealGainType::None)
			return;

		bool hasInfantrySelfHeal = pTypeExt->SelfHealGainType.isset() && pTypeExt->SelfHealGainType.Get() == SelfHealGainType::Infantry;
		bool hasUnitSelfHeal = pTypeExt->SelfHealGainType.isset() && pTypeExt->SelfHealGainType.Get() == SelfHealGainType::Units;
		bool isOrganic = false;

		if (pThis->WhatAmI() == AbstractType::Infantry ||
			pThis->GetTechnoType()->Organic && pThis->WhatAmI() == AbstractType::Unit)
		{
			isOrganic = true;
		}

		if (pThis->Owner->InfantrySelfHeal > 0 && (hasInfantrySelfHeal || isOrganic) && !hasUnitSelfHeal)
		{
			drawPip = true;
			selfHealFrames = RulesClass::Instance->SelfHealInfantryFrames;
			isInfantryHeal = true;
		}
		else if (pThis->Owner->UnitsSelfHeal > 0 && (hasUnitSelfHeal || (pThis->WhatAmI() == AbstractType::Unit && !isOrganic)))
		{
			drawPip = true;
			selfHealFrames = RulesClass::Instance->SelfHealUnitFrames;
		}

		if (const auto pHealthBar = pTypeExt->HealthBarType.Get(TechnoExt::GetHealthBarType(pThis, false)))
		{
			selfheal_pips = pHealthBar->SelfHealPips_Frame;
			selfheal_offset = pHealthBar->SelfHealPips_Offset;
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
			const auto offset = selfheal_offset.Get(RulesExt::Global()->Pips_SelfHeal_Units_Offset.Get());
			pipFrames = selfheal_pips.Get(RulesExt::Global()->Pips_SelfHeal_Units.Get());
			xOffset = offset.X;
			yOffset = offset.Y + pThis->GetTechnoType()->PixelSelectionBracketDelta;
		}
		else if (pThis->WhatAmI() == AbstractType::Infantry)
		{
			const auto offset = selfheal_offset.Get(RulesExt::Global()->Pips_SelfHeal_Infantry_Offset.Get());
			pipFrames = selfheal_pips.Get(RulesExt::Global()->Pips_SelfHeal_Infantry.Get());
			xOffset = offset.X;
			yOffset = offset.Y + pThis->GetTechnoType()->PixelSelectionBracketDelta;
		}
		else
		{
			auto pType = abstract_cast<BuildingTypeClass*>(pThis->GetTechnoType());
			int fHeight = pType->GetFoundationHeight(false);
			int yAdjust = -Unsorted::CellHeightInPixels / 2;

			const auto offset = selfheal_offset.Get(RulesExt::Global()->Pips_SelfHeal_Buildings_Offset.Get());
			pipFrames = selfheal_pips.Get(RulesExt::Global()->Pips_SelfHeal_Buildings.Get());
			xOffset = offset.X + Unsorted::CellWidthInPixels / 2 * fHeight;
			yOffset = offset.Y + yAdjust * fHeight + pType->Height * yAdjust;
		}

		int pipFrame = isInfantryHeal ? pipFrames.Get().X : pipFrames.Get().Y;

		Point2D position = { pLocation->X + xOffset, pLocation->Y + yOffset };

		auto flags = BlitterFlags::bf_400 | BlitterFlags::Centered;

		if (isSelfHealFrame)
			flags = flags | BlitterFlags::Darken;

		DSurface::Temp->DrawSHP(FileSystem::PALETTE_PAL, FileSystem::PIPS_SHP, pipFrame, position, *pBounds, flags);
	}
}

void TechnoExt::DrawGroupID_Building(TechnoClass* pThis, HealthBarTypeClass* pHealthBar, const Point2D* pLocation)
{
	CoordStruct coords = CoordStruct::Empty;
	pThis->GetTechnoType()->Dimension2(&coords);
	Point2D posScreen = Point2D::Empty;
	CoordStruct coords2 = { -coords.X / 2, coords.Y / 2, coords.Z };
	TacticalClass::Instance->CoordsToScreen(&posScreen, &coords2);

	const Point2D& location = *pLocation;
	Point2D posDraw = Point2D::Empty;
	const Point2D& offset = pHealthBar->GroupID_Offset.Get();

	posDraw.X = location.X + offset.X;
	posDraw.Y = posScreen.Y + location.Y + posScreen.Y + 16;

	if (pThis->Group >= 0)
	{
		const COLORREF colorGroupID = Drawing::RGB_To_Int(pThis->GetOwningHouse()->Color);

		RectangleStruct rect(posDraw.X, posDraw.Y, 11, 13);

		auto bounds = DSurface::Temp()->GetRect();
		bounds.Height -= 32;

		DSurface::Composite->FillRectEx(&bounds, &rect, COLOR_BLACK);
		DSurface::Composite->DrawRectEx(&bounds, &rect, colorGroupID);

		int groupID = (pThis->Group == 9) ? 0 : (pThis->Group + 1);

		wchar_t textGroupID[0x20];
		swprintf_s(textGroupID, L"%d", groupID);

		Point2D posGroupID
		{
			posDraw.X + 3,
			posDraw.Y - 2
		};

		DSurface::Temp->DrawTextA(textGroupID, bounds, posGroupID, colorGroupID, 0, TextPrintType::NoShadow);
	}
}

void TechnoExt::DrawGroupID_Other(TechnoClass* pThis, HealthBarTypeClass* pHealthBar, const Point2D* pLocation)
{
	//const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	const Point2D& location = *pLocation;
	Point2D offset = pHealthBar->GroupID_Offset.Get();

	offset.Y += pThis->GetTechnoType()->PixelSelectionBracketDelta;

	Point2D posDraw = location;

	posDraw += offset;

	if (pThis->Group >= 0)
	{
		if (pThis->WhatAmI() == AbstractType::Infantry)
		{
			posDraw.X -= 7;
			posDraw.Y -= 37;
		}
		else
		{
			posDraw.X -= 17;
			posDraw.Y -= 38;
		}

		const COLORREF colorGroupID = Drawing::RGB_To_Int(pThis->GetOwningHouse()->Color);

		RectangleStruct rect(posDraw.X, posDraw.Y, 11, 13);

		RectangleStruct bounds = DSurface::Temp()->GetRect();
		bounds.Height -= 32;

		DSurface::Composite->FillRectEx(&bounds, &rect, COLOR_BLACK);
		DSurface::Composite->DrawRectEx(&bounds, &rect, colorGroupID);

		int groupid = (pThis->Group == 9) ? 0 : (pThis->Group + 1);

		wchar_t groupID[0x20];
		swprintf_s(groupID, L"%d", groupid);

		Point2D posGroupID
		{
			posDraw.X + 3,
			posDraw.Y - 2
		};

		DSurface::Composite->DrawTextA(groupID, bounds, posGroupID, colorGroupID, 0, TextPrintType::NoShadow);
	}
}

void TechnoExt::DrawHealthBar_Building(TechnoClass* pThis, HealthBarTypeClass* pHealthBar, int iLength, const Point2D* pLocation, const RectangleStruct* pBound)
{
	//const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	CoordStruct vCoords = { 0, 0, 0 };
	pThis->GetTechnoType()->Dimension2(&vCoords);
	Point2D vPos2 = { 0, 0 };
	CoordStruct vCoords2 = { -vCoords.X / 2, vCoords.Y / 2,vCoords.Z };
	TacticalClass::Instance->CoordsToScreen(&vPos2, &vCoords2);

	Point2D vLoc = *pLocation;
	vLoc.Y += 1;

	Point2D vPos = { 0, 0 };

	SHPStruct* PipsSHP = pHealthBar->PipsSHP.Get() ? pHealthBar->PipsSHP.Get() : FileSystem::PIPS_SHP;
	ConvertClass* PipsPAL = pHealthBar->PipsPAL.GetOrDefaultConvert(FileSystem::PALETTE_PAL);

	const int iTotal = DrawHealthBar_PipAmount(pThis, 1, iLength);
	int frame = DrawHealthBar_Pip(pThis, pHealthBar, true);
	BlitterFlags flags = BlitterFlags::Centered | BlitterFlags::bf_400 | BlitterFlags::Alpha;

	if (iTotal > 0)
	{
		int frameIdx, deltaX, deltaY;
		for (frameIdx = iTotal, deltaX = 0, deltaY = 0;
			frameIdx;
			frameIdx--, deltaX += 4, deltaY -= 2)
		{
			vPos.X = vPos2.X + vLoc.X + 4 * iLength + 3 - deltaX;
			vPos.Y = vPos2.Y + vLoc.Y - 2 * iLength + 4 - deltaY;

			DSurface::Composite->DrawSHP
			(
				PipsPAL,
				PipsSHP,
				frame,
				vPos,
				*pBound,
				flags
			);
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

			DSurface::Composite->DrawSHP
			(
				PipsPAL,
				PipsSHP,
				0,
				vPos,
				*pBound,
				flags
			);
		}
	}

}

void TechnoExt::DrawHealthBar_Other(TechnoClass* pThis, HealthBarTypeClass* pHealthBar, int iLength, const Point2D* pLocation, const RectangleStruct* pBound)
{
	//const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	Point2D vPos = { 0, 0 };
	Point2D vLoc = *pLocation;

	int frame, XOffset, YOffset;// , XOffset2;
	YOffset = pHealthBar->YOffset.Get(pThis->GetTechnoType()->PixelSelectionBracketDelta);
	vLoc.Y -= 5;

	if (pThis->Health == 0)
		return;

	vLoc.X += pHealthBar->XOffset.Get();

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

	SHPStruct* pPipsSHP = pHealthBar->PipsSHP.Get() ? pHealthBar->PipsSHP.Get() : FileSystem::PIPS_SHP;
	ConvertClass* pPipsPAL = pHealthBar->PipsPAL.GetOrDefaultConvert(FileSystem::PALETTE_PAL);
	SHPStruct* pPipBrdSHP = pHealthBar->PipBrdSHP.Get() ? pHealthBar->PipBrdSHP.Get() : FileSystem::PIPBRD_SHP;
	ConvertClass* pPipBrdPAL = pHealthBar->PipBrdPAL.GetOrDefaultConvert(FileSystem::PALETTE_PAL);

	int BldAdjust = 0;
	if (pThis->WhatAmI() == AbstractType::Building && !pHealthBar->UnitHealthBar_IgnoreBuildingHeight)
	{
		CoordStruct vCoords = { 0, 0, 0 };
		pThis->GetTechnoType()->Dimension2(&vCoords);
		Point2D vPos2 = { 0, 0 };
		CoordStruct vCoords2 = { -vCoords.X / 2, vCoords.Y / 2,vCoords.Z };
		TacticalClass::Instance->CoordsToScreen(&vPos2, &vCoords2);
		BldAdjust = vPos2.Y;
	}

	const auto pipbrd = pHealthBar->PipBrd.Get(frame);
	if (pThis->IsSelected)
	{

		Point2D vPosBrd
		{
			vPos.X + pHealthBar->PipBrdOffset.Get().X,
			vPos.Y + pHealthBar->PipBrdOffset.Get().Y + BldAdjust
		};

		DSurface::Temp->DrawSHP
		(
			pPipBrdPAL,
			pPipBrdSHP,
			pipbrd,
			vPosBrd,
			*pBound,
			BlitterFlags::Centered | BlitterFlags::bf_400 | BlitterFlags::Alpha
		);
	}

	const auto ilength = pHealthBar->Length.Get(iLength);
	const int iTotal = DrawHealthBar_PipAmount(pThis, 1, ilength);
	frame = DrawHealthBar_Pip(pThis, pHealthBar, false);

	Point2D DrawOffset = pHealthBar->DrawOffset.Get({ 2,0 });

	for (int i = 0; i < iTotal; ++i)
	{
		vPos.X = vLoc.X + XOffset + DrawOffset.X * i;
		vPos.Y = vLoc.Y + YOffset + DrawOffset.Y * i + BldAdjust;

		DSurface::Temp->DrawSHP
		(
			pPipsPAL,
			pPipsSHP,
			frame,
			vPos,
			*pBound,
			BlitterFlags::Centered | BlitterFlags::bf_400 | BlitterFlags::Alpha
		);
	}
}

void TechnoExt::DrawHealthBar_Picture(TechnoClass* pThis, HealthBarTypeClass* pHealthBar, int iLength, const Point2D* pLocation, const RectangleStruct* pBound)
{
	//const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	Point2D vPos = { 0,0 };
	Point2D vLoc = *pLocation;
	int YOffset;
	YOffset = pHealthBar->YOffset.Get(pThis->GetTechnoType()->PixelSelectionBracketDelta);
	vLoc.Y -= 5;

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

	SHPStruct* pShape = pHealthBar->PictureSHP.Get() ? pHealthBar->PictureSHP.Get() : FileSystem::PIPS_SHP;
	ConvertClass* pPalette = pHealthBar->PicturePAL.GetOrDefaultConvert(FileSystem::PALETTE_PAL);

	const auto ilength = pHealthBar->Length.Get(pShape->Frames - 1);
	const int iTotal = DrawHealthBar_PipAmount(pThis, 0, ilength);

	vPos.X += pHealthBar->XOffset.Get();

	DSurface::Composite->DrawSHP
	(
		pPalette,
		pShape,
		iTotal,
		vPos,
		*pBound,
		EnumFunctions::GetTranslucentLevel(pHealthBar->PictureTransparency.Get())
	);
}

int TechnoExt::DrawHealthBar_Pip(TechnoClass* pThis, HealthBarTypeClass* pHealthBar, const bool isBuilding)
{
	const auto strength = pThis->GetTechnoType()->Strength;

	const auto Pip = (isBuilding ? pHealthBar->Pips.Get({ 1,2,4 }) :
		pHealthBar->Pips.Get({ 16,17,18 }));

	if (pThis->Health > RulesClass::Instance->ConditionYellow * strength && Pip.X != -1)
		return Pip.X;
	else if (pThis->Health > RulesClass::Instance->ConditionRed * strength && (Pip.Y != -1 || Pip.X != -1))
		return Pip.Y == -1 ? Pip.X : Pip.Y;
	else if (Pip.Z != -1 || Pip.X != -1)
		return Pip.Z == -1 ? Pip.X : Pip.Z;

	return isBuilding ? 5 : 16;
}

int TechnoExt::DrawHealthBar_PipAmount(TechnoClass* pThis, int Minimum, int iLength)
{
	return pThis->Health > 0
		? Math::clamp((int)round(GetHealthRatio(pThis) * iLength), Minimum, iLength)
		: 0;
}

double TechnoExt::GetHealthRatio(TechnoClass* pThis)
{
	return static_cast<double>(pThis->Health) / pThis->GetTechnoType()->Strength;
}

HealthBarTypeClass* TechnoExt::GetHealthBarType(TechnoClass* pThis, bool isShield = false)
{
	const auto Default = RulesExt::Global()->HealthBar_Def;

	if (isShield)
	{
		if (pThis->WhatAmI() == AbstractType::Infantry)
			return RulesExt::Global()->ShieldBar_Infantry.Get(Default);
		if (pThis->WhatAmI() == AbstractType::Unit)
			return RulesExt::Global()->ShieldBar_Vehicle.Get(Default);
		if (pThis->WhatAmI() == AbstractType::Aircraft)
			return RulesExt::Global()->ShieldBar_Aircraft.Get(Default);
		if (pThis->WhatAmI() == AbstractType::Building)
			return RulesExt::Global()->ShieldBar_Building.Get(Default);
	}
	else
	{
		if (pThis->WhatAmI() == AbstractType::Infantry)
			return RulesExt::Global()->HealthBar_Infantry.Get(Default);
		if (pThis->WhatAmI() == AbstractType::Unit)
			return RulesExt::Global()->HealthBar_Vehicle.Get(Default);
		if (pThis->WhatAmI() == AbstractType::Aircraft)
			return RulesExt::Global()->HealthBar_Aircraft.Get(Default);
		if (pThis->WhatAmI() == AbstractType::Building)
			return RulesExt::Global()->HealthBar_Building.Get(Default);
	}

	return nullptr;
}

// Based on Ares source.
void TechnoExt::DrawInsignia(TechnoClass* pThis, const Point2D& location, const RectangleStruct& bounds)
{
	Point2D offset = location;

	SHPStruct* pShapeFile = FileSystem::PIPS_SHP;
	int defaultFrameIndex = -1;

	auto pTechnoType = pThis->GetTechnoType();
	auto pOwner = pThis->Owner;

	if (pThis->IsDisguised() && !pThis->IsClearlyVisibleTo(HouseClass::CurrentPlayer))
	{
		if (auto const pType = TechnoTypeExt::GetTechnoType(pThis->Disguise))
		{
			pTechnoType = pType;
			pOwner = pThis->DisguisedAsHouse;
		}
		else if (!pOwner->IsAlliedWith(HouseClass::CurrentPlayer) && !HouseClass::IsCurrentPlayerObserver())
		{
			return;
		}
	}

	TechnoTypeExt::ExtData* pExt = TechnoTypeExt::ExtMap.Find(pTechnoType);

	bool isVisibleToPlayer = (pOwner && pOwner->IsAlliedWith(HouseClass::CurrentPlayer))
		|| HouseClass::IsCurrentPlayerObserver()
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

		DSurface::Composite->DrawSHP
		(
			FileSystem::PALETTE_PAL,
			pShapeFile,
			frameIndex,
			offset,
			bounds,
			BlitterFlags::Centered | BlitterFlags::bf_400 | BlitterFlags::Alpha,
			0,
			-2
		);
	}

	return;
}

void TechnoExt::DrawSelectBox(TechnoClass* pThis, const Point2D& location, const RectangleStruct& bound, bool isInfantry)
{
	if (!pThis->IsSelected)
		return;

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	const auto canHouse = pTypeExt->SelectBox_CanSee.Get(RulesExt::Global()->SelectBox_CanSee);
	bool canSee = pTypeExt->SelectBox_CanObserverSee.Get(RulesExt::Global()->SelectBox_CanObserverSee) && HouseClass::IsCurrentPlayerObserver()
		|| EnumFunctions::CanTargetHouse(canHouse, pThis->Owner, HouseClass::CurrentPlayer);

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
	auto const nFlag =
		BlitterFlags::Centered
		| BlitterFlags::Nonzero
		| BlitterFlags::MultiPass
		| EnumFunctions::GetTranslucentLevel(pTypeExt->SelectBox_TranslucentLevel.Get(RulesExt::Global()->SelectBox_TranslucentLevel.Get()));

	if (selectboxFrame.X == -1)
		selectboxFrame = glbSelectboxFrame;

	if (isInfantry)
	{
		vPos.X = location.X + 1 + vOffset.X;
		vPos.Y = location.Y + 1 + pThis->GetTechnoType()->PixelSelectionBracketDelta + vOffset.Y;
	}
	else
	{
		vPos.X = location.X + 2 + vOffset.X;
		vPos.Y = location.Y + 1 + pThis->GetTechnoType()->PixelSelectionBracketDelta + vOffset.Y;
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

	if (pThis->IsGreenHP())
		frame = selectboxFrame.X;
	else if (pThis->IsYellowHP())
		frame = selectboxFrame.Y;
	else
		frame = selectboxFrame.Z;

	DSurface::Temp->DrawSHP(pPalette, pShape, frame, vPos, bound, nFlag);
}

void TechnoExt::DisplayDamageNumberString(TechnoClass* pThis, int damage, bool isShieldDamage)
{
	if (!pThis || damage == 0)
		return;

	const auto pExt = TechnoExt::ExtMap.Find(pThis);
	ColorStruct color;

	if (!isShieldDamage)
		color = damage > 0 ? ColorStruct { 255, 0, 0 } : ColorStruct { 0, 255, 0 };
	else
		color = damage > 0 ? ColorStruct { 0, 160, 255 } : ColorStruct { 0, 255, 230 };

	auto coords = pThis->GetRenderCoords();
	int maxOffset = Unsorted::CellWidthInPixels / 2;
	int width = 0, height = 0;
	wchar_t damageStr[0x20];
	swprintf_s(damageStr, L"%d", damage);

	BitFont::Instance->GetTextDimension(damageStr, &width, &height, 120);

	if (pExt->DamageNumberOffset >= maxOffset || pExt->DamageNumberOffset < -maxOffset)
		pExt->DamageNumberOffset = -maxOffset;

	FlyingStrings::Add(damageStr, coords, color, Point2D { pExt->DamageNumberOffset - (width / 2), 0 });

	pExt->DamageNumberOffset = pExt->DamageNumberOffset + width;
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
	RectangleStruct rectDraw = DSurface::Composite->GetRect();
	rectDraw.Height -= 32;
	Point2D posDraw = pConfig->HugeBar_Offset.Get() + pConfig->Anchor.OffsetPosition(rectDraw);
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
			posDrawValue.Y -= pShp_Bar->Height;
			break;

		default:
			break;
		}

		DSurface::Composite->DrawSHP(pPal_Bar, pShp_Bar, pConfig->HugeBar_Frame.Get(ratio), posDraw, rBound);

		posDraw += pConfig->HugeBar_Pips_Offset.Get();

		for (int i = 0; i < iPipNumber; i++)
		{
			DSurface::Composite->DrawSHP(pPal_Pips, pShp_Pips, iPipFrame, posDraw, rBound);

			posDraw.X += pConfig->HugeBar_Pips_Spacing;
		}
	}
	else
	{
		COLORREF color1 = Drawing::RGB_To_Int(pConfig->HugeBar_Pips_Color1.Get(ratio));
		COLORREF color2 = Drawing::RGB_To_Int(pConfig->HugeBar_Pips_Color2.Get(ratio));
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
			posDrawValue.Y -= rectWH.Y;
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

		posDraw.X -= text.length() * pConfig->Value_Shape_Spacing / 2;

		ShapeTextPrintData printData
		(
			pShp,
			pPal,
			iNumBaseFrame,
			iSignBaseFrame,
			Point2D({ pConfig->Value_Shape_Spacing, 0 })
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

		COLORREF color = Drawing::RGB_To_Int(pConfig->Value_Text_Color.Get(ratio));
		DSurface::Composite->DrawTextA(text, rBound, posDraw, color, COLOR_BLACK, TextPrintType::Center);
	}
}

void TechnoExt::ProcessBlinkWeapon(TechnoClass* pThis, AbstractClass* pTarget, WeaponTypeClass* pWeapon)
{
	TechnoClass* pTargetTechno = abstract_cast<TechnoClass*>(pTarget);

	if (pTargetTechno == nullptr || pTargetTechno == pThis)
		return;

	TechnoTypeClass* pType = pThis->GetTechnoType();
	auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);
	const CoordStruct crdSrc = pThis->Location;
	CoordStruct crdDest = pTargetTechno->GetCoords();

	if (pWeaponExt->BlinkWeapon && pThis->WhatAmI() != AbstractType::Building)
	{
		for (AnimTypeClass* pAnimType : pWeaponExt->BlinkWeapon_SelfAnim)
		{
			AnimClass* pAnim = GameCreate<AnimClass>(pAnimType, crdSrc);
			pAnim->Owner = pThis->Owner;
		}

		for (AnimTypeClass* pAnimType : pWeaponExt->BlinkWeapon_TargetAnim)
		{
			AnimClass* pAnim = GameCreate<AnimClass>(pAnimType, crdDest);
			pAnim->SetOwnerObject(pTargetTechno);
			pAnim->Owner = pThis->Owner;
		}

		CellClass* pCell = nullptr;
		CellStruct nCell;
		int iHeight = pTargetTechno->GetHeight();

		if (pWeaponExt->BlinkWeapon_Overlap.Get())
		{
			nCell = CellClass::Coord2Cell(crdDest);
			pCell = MapClass::Instance->TryGetCellAt(nCell);
		}
		else
		{
			bool allowBridges = pType->SpeedType != SpeedType::Float;
			nCell = MapClass::Instance->NearByLocation(CellClass::Coord2Cell(crdDest),
				pType->SpeedType, -1, pType->MovementZone, false, 1, 1, true,
				false, false, allowBridges, CellStruct::Empty, false, false);
			pCell = MapClass::Instance->TryGetCellAt(nCell);
		}

		if (pCell != nullptr)
			crdDest = pCell->GetCoordsWithBridge();
		else
			crdDest.Z = MapClass::Instance->GetCellFloorHeight(crdDest);

		crdDest.Z += iHeight;
		FootClass* pFoot = abstract_cast<FootClass*>(pThis);
		CellStruct cellDest = CellClass::Coord2Cell(crdDest);
		pThis->Limbo();
		ILocomotion* pLoco = pFoot->Locomotor.release();
		pFoot->Locomotor.reset(LocomotionClass::CreateInstance(pType->Locomotor).release());
		pFoot->Locomotor->Link_To_Object(pFoot);
		pLoco->Release();
		++Unsorted::IKnowWhatImDoing;
		pThis->Unlimbo(crdDest, pThis->PrimaryFacing.Current().GetDir());
		--Unsorted::IKnowWhatImDoing;

		//if (pThis->IsInAir())
		//	TechnoExt::FallenDown(pThis);

		TechnoExt::ExtMap.Find(pThis)->WasFallenDown = true;

		if (pWeaponExt->BlinkWeapon_KillTarget.Get())
			pTargetTechno->TakeDamage(pTargetTechno->Health, pThis->Owner, pThis);
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
		if (pData->ShowAnim_LastActivatedFrame < 0)
			pData->ShowAnim_LastActivatedFrame = -pReceiveDamageAnimType->ShowAnim_CoolDown;

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

		Point2D posAnim = {
			DSurface::Composite->GetWidth() / 2 - ShowAnimSHP->Width / 2,
			DSurface::Composite->GetHeight() / 2 - ShowAnimSHP->Height / 2
		};
		posAnim += pReceiveDamageAnimType->ShowAnim_Offset.Get();
		int translucentLevel = pReceiveDamageAnimType->ShowAnim_TranslucentLevel.Get();
		int frameKeep = pReceiveDamageAnimType->ShowAnim_FrameKeep;
		int loopCount = pReceiveDamageAnimType->ShowAnim_LoopCount;
		GScreenDisplay::Add(ShowAnimPAL, ShowAnimSHP, posAnim, translucentLevel, frameKeep, loopCount);
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
	const auto pHealthBar = pTypeExt->HealthBarType.Get(TechnoExt::GetHealthBarType(pThis, false));


	if (!pHealthBar || pHealthBar->DigitalDisplay_Disable.Get(pTypeExt->DigitalDisplay_Disable))
		return;

	if (pThis->Owner != HouseClass::CurrentPlayer &&
		!pThis->Owner->Allies.Contains(HouseClass::CurrentPlayer) &&
		!pHealthBar->DigitalDisplay_ShowEnemy.Get())
		return;

	TechnoExt::ExtData* pExt = TechnoExt::ExtMap.Find(pThis);
	int iLength = 17;
	ValueableVector<DigitalDisplayTypeClass*>* pDisplayTypes = nullptr;

	if (!pHealthBar->DigitalDisplayTypes.empty())
	{
		pDisplayTypes = &pHealthBar->DigitalDisplayTypes;
	}
	else if (!pTypeExt->DigitalDisplayTypes.empty())
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
		if (HouseClass::IsCurrentPlayerObserver() && !pDisplayType->VisibleToHouses_Observer)
			continue;

		if (!HouseClass::IsCurrentPlayerObserver() && !EnumFunctions::CanTargetHouse(pDisplayType->VisibleToHouses, pThis->Owner, HouseClass::CurrentPlayer))
			continue;

		int iCur = -1;
		int iMax = -1;

		GetValuesForDisplay(pThis, pDisplayType->InfoType.Get(), iCur, iMax);

		if (iCur == -1 || iMax == -1)
			continue;

		bool isBuilding = pThis->WhatAmI() == AbstractType::Building;
		bool isInifantry = pThis->WhatAmI() == AbstractType::Infantry;
		bool hasShield = pExt->Shield != nullptr && !pExt->Shield->IsBrokenAndNonRespawning();
		Point2D posDraw = (pThis->WhatAmI() == AbstractType::Building && !pDisplayType->UnitDigitalDisplay) ?
			GetBuildingSelectBracketPosition(pThis, pDisplayType->AnchorType_Building)
			: GetFootSelectBracketPosition(pThis, pDisplayType->AnchorType);
		posDraw.Y += pHealthBar->YOffset.Get(pType->PixelSelectionBracketDelta);

		if (pDisplayType->InfoType == DisplayInfoType::Shield)
			posDraw.Y += pExt->Shield->GetType()->BracketDelta;

		if (isBuilding && pDisplayType->UnitDigitalDisplay)
		{
			iLength = 17;
			if (!pDisplayType->UnitDigitalDisplay_IgnoreBuildingHeight)
			{
				CoordStruct vCoords = { 0, 0, 0 };
				pThis->GetTechnoType()->Dimension2(&vCoords);
				Point2D vPos2 = { 0, 0 };
				CoordStruct vCoords2 = { -vCoords.X / 2, vCoords.Y / 2,vCoords.Z };
				TacticalClass::Instance->CoordsToScreen(&vPos2, &vCoords2);
				posDraw.Y += vPos2.Y;
			}
		}

		pDisplayType->Draw(posDraw, iLength, iCur, iMax, isBuilding, isInifantry, hasShield);
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
		if (!pType->IsGattling && !pTypeExt->IsExtendGattling)
			return;

		iCur = (!pType->IsGattling && pTypeExt->IsExtendGattling) ? pExt->GattlingStage + 1 : pThis->CurrentGattlingStage + 1;
		iMax = pType->WeaponStages;
		break;
	}
	case DisplayInfoType::Temperature:
	{
		iCur = pExt->Temperature[0];
		iMax = pTypeExt->Temperature[0];
		break;
	}
	case DisplayInfoType::Power:
	{
		if (pTypeExt->Power == 0 && pTypeExt->ExtraPower == 0)
			return;

		int extra = 0;

		if (pThis->Passengers.NumPassengers > 0)
		{
			FootClass* pPassenger = pThis->Passengers.GetFirstPassenger();
			while (pPassenger)
			{
				if (pTypeExt->ExtraPower_BySize)
				{
					if (pTypeExt->ExtraPower > 0)
						extra += pTypeExt->ExtraPower * static_cast<int>(pPassenger->GetTechnoType()->Size);
					else
						extra += pTypeExt->ExtraPower * static_cast<int>(pPassenger->GetTechnoType()->Size);
				}
				else
				{
					if (pTypeExt->ExtraPower > 0)
						extra += pTypeExt->ExtraPower;
					else
						extra += pTypeExt->ExtraPower;
				}

				pPassenger = abstract_cast<FootClass*>(pPassenger->NextObject);
			}
		}

		iCur = abs(pTypeExt->Power + extra);

		if (pTypeExt->Power + extra >= 0)
			iMax = pThis->Owner->PowerOutput;
		else
			iMax = abs(pThis->Owner->PowerDrain);
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
		/*
		auto pTechno = static_cast<TechnoClass*>(pType->CreateObject(pThis->Owner));
		++Unsorted::IKnowWhatImDoing;
		pTechno->Unlimbo(pThis->GetCoords(), pThis->PrimaryFacing.Current().GetDir());
		--Unsorted::IKnowWhatImDoing;
		pTechno->Limbo();
		*/
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

		// pTechno->UnInit();
	}

	SidebarClass::Instance->SidebarNeedsRepaint();
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
	}

	SidebarClass::Instance->SidebarNeedsRepaint();
}

void TechnoExt::ProcessAttackedWeapon(TechnoClass* pThis, args_ReceiveDamage* args, bool bBeforeDamageCheck)
{
	if (!TechnoExt::IsReallyAlive(pThis) || pThis == args->Attacker)
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
	ValueableVector<Leptons>& vRange = pTypeExt->AttackedWeapon_Range;
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
			if (TechnoExt::IsReallyAlive(args->Attacker))
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

void TechnoExt::InitialPayloadFixed(TechnoClass* pThis)
{
	if (pThis->WhatAmI() != AbstractType::Unit && pThis->WhatAmI() != AbstractType::Aircraft)
		return;

	if (pThis->Passengers.GetFirstPassenger() != nullptr)
		return;

	const TechnoTypeClass* pType = pThis->GetTechnoType();
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
	const auto& types = pTypeExt->InitialPayload_Types;
	const auto& nums = pTypeExt->InitialPayload_Nums;

	for (size_t i = 0; i < types.size(); i++)
	{
		TechnoTypeClass* pTechnoType = types[i];
		const int num = i < nums.size() ? nums[i] : 1;

		if (pTechnoType->WhatAmI() == AbstractType::BuildingType)
			continue;

		for (int j = 0; j < num; j++)
		{
			FootClass* pPassenger = static_cast<FootClass*>(pTechnoType->CreateObject(pThis->Owner));

			pPassenger->SetLocation(pThis->GetCenterCoords());
			pThis->Passengers.AddPassenger(pPassenger);
			pPassenger->Transporter = pThis;

			if (pType->OpenTopped)
				pThis->EnteredOpenTopped(pPassenger);

			if (pThis->WhatAmI() != AbstractType::Building
				&& pType->Gunner)
			{
				static_cast<FootClass*>(pThis)->ReceiveGunner(pPassenger);
			}
				
		}
	}
}

void TechnoExt::FixManagers(TechnoClass* pThis)
{
	TechnoTypeClass* pType = pThis->GetTechnoType();
	std::vector<WeaponTypeClass*> vWeapons(TechnoTypeExt::GetAllWeapons(pType));
	bool bCaptureSet = false;
	bool bSpawnSet = false;
	bool hasCapture = false;
	bool hasTemporal = false;
	bool hasSpawn = false;
	bool hasParasite = false;
	FootClass* pFoot = abstract_cast<FootClass*>(pThis);

	for (WeaponTypeClass* pWeapon : vWeapons)
	{
		if (pWeapon->Warhead->MindControl)
		{
			hasCapture = true;

			if (!bCaptureSet)
			{
				std::vector<ControlNode*> vCaptured;

				if (pThis->CaptureManager != nullptr)
				{
					DynamicVectorClass<ControlNode*> nodes = pThis->CaptureManager->ControlNodes;

					for (int i = 0; i < nodes.Count; i++)
					{
						vCaptured.emplace_back(nodes.GetItem(i));
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

		if (pWeapon->Warhead->Temporal)
		{
			hasTemporal = true;

			if (pThis->TemporalImUsing == nullptr)
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
					TechnoClass* pSpawn = pManager->SpawnedNodes.GetItem(0)->Techno;
					pSpawn->TakeDamage(pSpawn->Health);
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

		if (pWeapon->Warhead->Airstrike)
		{
			if (pThis->Airstrike == nullptr)
			{
				pThis->Airstrike = GameCreate<AirstrikeClass>(pThis);
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

void TechnoExt::ChangeLocomotorTo(TechnoClass* pThis, const CLSID& locomotor)
{
	FootClass* pFoot = abstract_cast<FootClass*>(pThis);

	if (pFoot == nullptr)
		return;

	while(LocomotionClass::End_Piggyback(pFoot->Locomotor));

	ILocomotion* pSource = pFoot->Locomotor.release();

	pFoot->Locomotor.reset(LocomotionClass::CreateInstance(locomotor).release());
	pFoot->Locomotor->Link_To_Object(pFoot);
	pSource->Release();
	Mission curMission = pFoot->GetCurrentMission();

	if (pFoot->Target != nullptr)
	{
		AbstractClass* pTarget = pFoot->Target;

		pFoot->ForceMission(Mission::Move);
		pFoot->ReceiveCommand(pThis, RadioCommand::RequestMoveTo, pFoot->Destination);
		pFoot->ForceMission(curMission);
		pFoot->ReceiveCommand(pThis, RadioCommand::RequestAttack, pTarget);
	}
	else
	{
		pFoot->Guard();
		pFoot->ForceMission(Mission::Move);
		pFoot->ReceiveCommand(pThis, RadioCommand::RequestMoveTo, pFoot->Destination);
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

	HouseClass* pHouse = pThis->Owner;
	double healthPercentage = pThis->GetHealthPercentage();
	ExtData* pExt = ExtMap.Find(pThis);
	pThis->Ammo = Math::min(pThis->Ammo, pTargetType->Ammo);
	HouseExt::RegisterLoss(pHouse, pThis);
	int originIdx = pOriginType->GetArrayIndex();
	int targetIdx = pTargetType->GetArrayIndex();

	switch (pThis->WhatAmI())
	{
	case AbstractType::Infantry:
	{
		InfantryClass* pInf = abstract_cast<InfantryClass*>(pThis);
		InfantryTypeClass* pInfType = static_cast<InfantryTypeClass*>(pTargetType);

		if (pInf->CurrentMission != Mission::Unload && pInf->IsDeployed() && !pInfType->Deployer)
		{
			pInf->Mission_Unload();
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
		pThis->PrimaryFacing.SetROT(pTargetType->ROT);
		pThis->SecondaryFacing.SetROT(pTargetTypeExt->TurretROT.Get(pTargetType->ROT));

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
		pThis->PrimaryFacing.SetROT(pTargetType->ROT);
		pThis->SecondaryFacing.SetROT(pTargetTypeExt->TurretROT.Get(pTargetType->ROT));

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
		return;
	}break;
	default:
		return;
	}

	pExt->TypeExtData = TechnoTypeExt::ExtMap.Find(pTargetType);
	pExt->AttachEffects_Initialized = false;
	SidebarClass::Instance->SidebarNeedsRepaint();
	HouseExt::RegisterGain(pHouse, pThis);
	pThis->Health = std::max(static_cast<int>(pTargetType->Strength * healthPercentage), 1);
	pThis->EstimatedHealth = pThis->Health;
	pThis->Cloakable = pTargetType->Cloakable;
	FixManagers(pThis);
	FootClass* pFoot = abstract_cast<FootClass*>(pThis);
	TechnoTypeExt::ExtData* pTargetTypeExt = TechnoTypeExt::ExtMap.Find(pTargetType);

	if (!pTargetTypeExt->GiftBoxData.empty())
		pExt->AttachedGiftBox = std::make_unique<GiftBoxClass>(pThis);

	if (pFoot != nullptr && pOriginType->Locomotor != pTargetType->Locomotor)
		ChangeLocomotorTo(pThis, pTargetType->Locomotor);

	if (pTargetType->OpenTopped && !pOriginType->OpenTopped)
	{
		for (FootClass* pPassenger = pThis->Passengers.GetFirstPassenger();
			pPassenger->NextObject
			&& (pPassenger->NextObject->AbstractFlags & AbstractFlags::Foot) != AbstractFlags::None
			&& static_cast<FootClass*>(pPassenger->NextObject)->Transporter == pThis;
			pPassenger = static_cast<FootClass*>(pPassenger->NextObject))
		{
			pThis->EnteredOpenTopped(pPassenger);
		}
	}

	if (!pTargetType->OpenTopped && pOriginType->OpenTopped)
	{
		for (FootClass* pPassenger = pThis->Passengers.GetFirstPassenger();
			pPassenger->NextObject
			&& (pPassenger->NextObject->AbstractFlags & AbstractFlags::Foot) != AbstractFlags::None
			&& static_cast<FootClass*>(pPassenger->NextObject)->Transporter == pThis;
			pPassenger = static_cast<FootClass*>(pPassenger->NextObject))
		{
			pThis->ExitedOpenTopped(pPassenger);
			pPassenger->Guard();
			MapClass::Logics->RemoveObject(pPassenger);
		}
	}

	auto const pOriginTypeExt = TechnoTypeExt::ExtMap.Find(pOriginType);

	if (pOriginTypeExt->Power != 0 || pTargetTypeExt->Power != 0)
	{
		auto pHouseExt = HouseExt::ExtMap.Find(pThis->Owner);
		if (pOriginTypeExt->Power > 0)
			pHouseExt->PowerUnitOutPut -= pOriginTypeExt->Power;
		else
			pHouseExt->PowerUnitDrain -= pOriginTypeExt->Power;

		if (pTargetTypeExt->Power > 0)
			pHouseExt->PowerUnitOutPut += pTargetTypeExt->Power;
		else
			pHouseExt->PowerUnitDrain += pTargetTypeExt->Power;

		pThis->Owner->UpdatePower();
		if (!pTargetTypeExt->Powered && pThis->Deactivated)
			pThis->Reactivate();
	}

	if (pThis->GetTechnoType()->DisableShadowCache)
	{
		pTargetType->DisableShadowCache = true;
		pTargetType->VoxelShadowCache.Clear();
	}

	if (pThis->GetTechnoType()->DisableVoxelCache)
	{
		pTargetType->DisableVoxelCache = true;
		pTargetType->VoxelMainCache.Clear();
		pTargetType->VoxelTurretBarrelCache.Clear();
		pTargetType->VoxelTurretWeaponCache.Clear();
	}

	pHouse->RecheckTechTree = true;
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

void TechnoExt::InitialConvert(TechnoClass* pThis, TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt)
{
	if (pThis->GetTechnoType()->Passengers <= 0)
		return;

	if (pThis->WhatAmI() != AbstractType::Unit)
		return;

	if (pExt->OrignType == nullptr)
		pExt->OrignType = pThis->GetTechnoType();

	for (size_t i = 0; i < pTypeExt->Convert_Passengers.size(); i++)
	{
		auto pass = TechnoTypeClass::Array()->GetItem(pTypeExt->Convert_Passengers[i]);
		auto tech = TechnoTypeClass::Array()->GetItem(pTypeExt->Convert_Types[i]);

		pExt->Convert_Passengers.push_back(pass);
		pExt->Convert_Types.push_back(tech);
	}
}

void TechnoExt::CheckPassenger(TechnoClass* const pThis, TechnoTypeClass* const pType, TechnoExt::ExtData* const pExt, TechnoTypeExt::ExtData* const pTypeExt)
{
	if (pThis->WhatAmI() != AbstractType::Unit)
		return;

	if (pExt->Convert_Passengers.empty() || pExt->Convert_Types.empty())
		return;

	if (!pTypeExt->UseConvert.Get())
		return;

	TechnoTypeClass* PassType = abstract_cast<TechnoClass*>(pThis->Passengers.GetFirstPassenger())->GetTechnoType();

	if (!PassType)
		return;

	if (std::find(pExt->Convert_Passengers.begin(), pExt->Convert_Passengers.end(), PassType) == pExt->Convert_Passengers.end())
		return;

	Nullable<TechnoTypeClass*> ChangeType;

	for (size_t i = 0; i < pTypeExt->Convert_Passengers.size(); i++)
	{
		TechnoTypeClass* Passenger = pExt->Convert_Passengers[i];

		if (strcmp(Passenger->get_ID(), PassType->get_ID()) == 0)
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

// Compares two weapons and returns index of which one is eligible to fire against current target (0 = first, 1 = second), or -1 if neither works.
int TechnoExt::PickWeaponIndex(TechnoClass* pThis, TechnoClass* pTargetTechno, AbstractClass* pTarget, int weaponIndexOne, int weaponIndexTwo, bool allowFallback)
{
	CellClass* targetCell = nullptr;

	if (auto pObject = abstract_cast<ObjectClass*>(pTarget))
	{
		if (!TechnoExt::IsReallyAlive(pObject))
			return -1;
	}

	// Ignore target cell for airborne target technos.
	if (!pTargetTechno || !pTargetTechno->IsInAir())
	{
		if (const auto pCell = abstract_cast<CellClass*>(pTarget))
			targetCell = pCell;
		else if (const auto pObject = abstract_cast<ObjectClass*>(pTarget))
			targetCell = pObject->GetCell();
	}

	const auto pWeaponStructOne = pThis->GetWeapon(weaponIndexOne);
	const auto pWeaponStructTwo = pThis->GetWeapon(weaponIndexTwo);

	if (!pWeaponStructOne && !pWeaponStructTwo)
		return -1;
	else if (!pWeaponStructTwo || !pWeaponStructTwo->WeaponType)
		return weaponIndexOne;
	else if (!pWeaponStructOne || !pWeaponStructOne->WeaponType)
		return weaponIndexTwo;

	const auto pWeaponOne = pWeaponStructOne->WeaponType;
	const auto pWeaponTwo = pWeaponStructTwo->WeaponType;
	const auto pTargetExt = TechnoExt::ExtMap.Find(pTargetTechno);

	if (const auto pSecondExt = WeaponTypeExt::ExtMap.Find(pWeaponTwo))
	{
		if (
			targetCell && !EnumFunctions::IsCellEligible(targetCell, pSecondExt->CanTarget, true)
			|| pThis->Passengers.NumPassengers == 0 && pSecondExt->PassengerDeletion
			|| (pTargetTechno
				&& (!EnumFunctions::IsTechnoEligible(pTargetTechno, pSecondExt->CanTarget)
					|| !EnumFunctions::CanTargetHouse(pSecondExt->CanTargetHouses, pThis->Owner, pTargetTechno->Owner)
					|| !pSecondExt->HasRequiredAttachedEffects(pTargetTechno, pThis)
					|| (pSecondExt->OnlyAllowOneFirer && pTargetExt->Attacker != nullptr
						&& pThis != pTargetExt->Attacker))
				)
			)
		{
			return weaponIndexOne;
		}
		else if (const auto pFirstExt = WeaponTypeExt::ExtMap.Find(pWeaponOne))
		{
			if (!allowFallback && !TechnoExt::CanFireNoAmmoWeapon(pThis, 1))
				return weaponIndexOne;

			if (
				targetCell && !EnumFunctions::IsCellEligible(targetCell, pFirstExt->CanTarget, true)
			|| pThis->Passengers.NumPassengers == 0 && pFirstExt->PassengerDeletion
			|| (pTargetTechno
				&& (!EnumFunctions::IsTechnoEligible(pTargetTechno, pFirstExt->CanTarget)
					|| !EnumFunctions::CanTargetHouse(pFirstExt->CanTargetHouses, pThis->Owner, pTargetTechno->Owner)
					|| !pFirstExt->HasRequiredAttachedEffects(pTargetTechno, pThis)
					|| (pFirstExt->OnlyAllowOneFirer && pTargetExt->Attacker != nullptr
						&& pThis != pTargetExt->Attacker))
				)
			)
			{
				return weaponIndexTwo;
			}
		}
	}

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (pTypeExt->DeterminedByRange)
	{
		WeaponTypeClass* pCloseWeapon = pWeaponOne;
		WeaponTypeClass* pFurtherWeapon = pWeaponTwo;
		int closeIdx = weaponIndexOne;
		int furtherIdx = weaponIndexTwo;

		if (pTargetTechno != nullptr)
		{
			TechnoExt::ExtData* pTargetTechnoExt = TechnoExt::ExtMap.Find(pTargetTechno);

			double versusClose = CustomArmor::GetVersus(pCloseWeapon->Warhead, pTargetTechnoExt->GetArmorIdx(pCloseWeapon));
			double versusFurther = CustomArmor::GetVersus(pFurtherWeapon->Warhead, pTargetTechnoExt->GetArmorIdx(pFurtherWeapon));

			if (fabs(versusFurther) < 1e-6)
				return closeIdx;

			if (fabs(versusClose) < 1e-6)
				return furtherIdx;
		}

		if (pCloseWeapon->Range > pFurtherWeapon->Range)
		{
			std::swap(closeIdx, furtherIdx);
			std::swap(pCloseWeapon, pFurtherWeapon);
		}

		int ChangeRange = pFurtherWeapon->MinimumRange;
		int ChangeRangeExtra = pTypeExt->DeterminedByRange_ExtraRange * 256;
		ChangeRange += ChangeRangeExtra;

		// auto pExt = TechnoExt::ExtMap.Find(pThis);

		if (pTypeExt->DeterminedByRange_MainWeapon == 0)
		{
			if (pThis->DiskLaserTimer.Completed() && pWeaponTwo->Range >= pThis->DistanceFrom(pTarget) && pWeaponTwo->MinimumRange <= pThis->DistanceFrom(pTarget))
				return weaponIndexTwo;

			return weaponIndexOne;
		}

		if (pTypeExt->DeterminedByRange_MainWeapon == 1)
		{
			if (pThis->DiskLaserTimer.Completed() && pWeaponOne->Range >= pThis->DistanceFrom(pTarget) && pWeaponOne->MinimumRange <= pThis->DistanceFrom(pTarget))
				return weaponIndexOne;

			return weaponIndexTwo;
		}

		if (pThis->DistanceFrom(pTarget) <= ChangeRange)
			return closeIdx;
		else
			return furtherIdx;
	}

	return -1;
}

CoordStruct TechnoExt::PassengerKickOutLocation(TechnoClass* pThis, FootClass* pPassenger)
{
	if (!pThis || !pPassenger)
		return CoordStruct::Empty;

	auto pTypePassenger = pPassenger->GetTechnoType();
	CoordStruct finalLocation = CoordStruct::Empty;
	short extraDistanceX = 1;
	short extraDistanceY = 1;
	SpeedType speedType = pTypePassenger->SpeedType;
	MovementZone movementZone = pTypePassenger->MovementZone;

	if (pTypePassenger->WhatAmI() == AbstractType::AircraftType)
	{
		speedType = SpeedType::Track;
		movementZone = MovementZone::Normal;
	}

	CellStruct placeCoords = pThis->GetCell()->MapCoords - CellStruct { (short)(extraDistanceX / 2), (short)(extraDistanceY / 2) };
	placeCoords = MapClass::Instance->NearByLocation(placeCoords, speedType, -1, movementZone, false, extraDistanceX, extraDistanceY, true, false, false, false, CellStruct::Empty, false, false);

	if (auto pCell = MapClass::Instance->TryGetCellAt(placeCoords))
	{
		pPassenger->OnBridge = pCell->ContainsBridge();
		finalLocation = pCell->GetCoordsWithBridge();
	}

	return finalLocation;
}

void TechnoExt::SelectSW(TechnoClass* pThis, TechnoTypeExt::ExtData* pTypeExt)
{
	const auto pHouse = pThis->Owner;

	if (!pHouse->IsCurrentPlayer())
		return;

	auto pExt = TechnoExt::ExtMap.Find(pThis);

	if (Phobos::ToSelectSW)
	{
		const auto idxSW = pTypeExt->SuperWeapon_Quick[pExt->SWIdx];
		auto pSW = pHouse->Supers.GetItem(idxSW);
		if (pSW)
		{
			MapClass::UnselectAll();
			pSW->SetReadiness(true);
			Unsorted::CurrentSWType = idxSW;
		}

		pExt->SWIdx++;
		if (pExt->SWIdx > pTypeExt->SuperWeapon_Quick.size() - 1)
			pExt->SWIdx = 0;

		Phobos::ToSelectSW = false;
	}

	if (Unsorted::CurrentSWType == -1)
	{
		pExt->SWIdx = 0;
	}
}

bool TechnoExt::CheckCanBuildUnitType(TechnoClass* pThis, int HouseIdx)
{
	bool CanPlace = false;

	if (const auto pExt = TechnoExt::ExtMap.Find(pThis))
	{
		const auto pTypeExt = pExt->TypeExtData;
		if (!pTypeExt)
			return false;

		bool baseNormal = pTypeExt->BaseNormal.Get();
		bool eligibileForAllyBuilding = pTypeExt->EligibileForAllyBuilding.Get();

		for (const auto& pAE : pExt->AttachEffects)
		{
			if (!pAE->IsActive())
				continue;

			baseNormal = pAE->Type->BaseNormal.Get(baseNormal);
			eligibileForAllyBuilding = pAE->Type->EligibileForAllyBuilding.Get(eligibileForAllyBuilding);

			if (baseNormal)
				break;
		}

		if (baseNormal)
		{
			if (pThis->Owner->ArrayIndex == HouseIdx)
			{
				CanPlace = true;
			}
			else if (SessionClass::Instance->Config.BuildOffAlly
				&& pThis->Owner->IsAlliedWith(HouseClass::Array()->GetItem(HouseIdx))
				&& eligibileForAllyBuilding)
			{
				CanPlace = true;
			}
		}
	}

	return CanPlace;
}

void TechnoExt::SetTemporalTeam(TechnoClass* pThis, TechnoClass* pTarget, WarheadTypeExt::ExtData* pWHExt)
{
	auto pOwnerExt = TechnoExt::ExtMap.Find(pThis);
	pOwnerExt->TemporalTarget = pTarget;

	std::vector<TechnoClass*> items(std::move(Helpers::Alex::getCellSpreadItems(pTarget->GetCoords(), pWHExt->Temporal_CellSpread, true)));
	pOwnerExt->TemporalTeam.clear();

	for (auto pEachTarget : items)
	{
		if (!pEachTarget || pEachTarget->InLimbo || !pEachTarget->IsAlive || !pEachTarget->Health || pEachTarget->IsSinking || pEachTarget == pThis || pEachTarget == pTarget)
			continue;

		if (!pWHExt->CanTargetHouse(pThis->Owner, pEachTarget))
			continue;

		auto pEachTargetExt = TechnoExt::ExtMap.Find(pEachTarget);

		if (CustomArmor::GetVersus(pWHExt, pEachTargetExt->GetArmorIdx(pWHExt->OwnerObject())) == 0.0)
			continue;

		pOwnerExt->TemporalTeam.emplace_back(pEachTarget);
	}
}

void TechnoExt::FallenDown(TechnoClass* pThis)
{
	if (pThis->WhatAmI() == AbstractType::Building)
		return;

	FootClass* pFoot = abstract_cast<FootClass*>(pThis);
	const auto pExt = TechnoExt::ExtMap.Find(pFoot);

	if (pExt->AntiGravityType)
	{
		pExt->WasOnAntiGravity = false;
		pExt->OnAntiGravity = false;
		pExt->AntiGravityType = nullptr;
		pExt->CurrtenFallRate = 0;
	}

	auto const pType = pThis->GetTechnoType();
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	bool allowBridges = pThis->GetTechnoType()->SpeedType != SpeedType::Float;
	auto pCell = pThis->GetCell();
	CoordStruct location = pThis->GetCoords();

	if (allowBridges)
		location = pCell->GetCoordsWithBridge();

	if (auto const pJJLoco = locomotion_cast<JumpjetLocomotionClass*>(pFoot->Locomotor))
	{
		location.Z = Math::max(MapClass::Instance->GetCellFloorHeight(location), INT32_MIN);
		location.Z += pType->JumpjetHeight;

		if (pType->BalloonHover)
		{
			if (pTypeExt->Tnoland)
			{
				pJJLoco->Move_To(location);
			}

			else
			{
				pJJLoco->State = JumpjetLocomotionClass::State::Hovering;
				pJJLoco->IsMoving = true;
				pJJLoco->DestinationCoords = location;
			}
		}
		else
		{
			pJJLoco->Move_To(location);
		}
	}
	else
	{
		if (pCell->ContainsBridge() && pThis->Location.Z >= location.Z)
			pThis->OnBridge = true;

		pThis->IsFallingDown = true;

		if (pThis->Location.Z > location.Z)
		{
			if (const auto pFootTypeExt = TechnoTypeExt::ExtMap.Find(pFoot->GetTechnoType()))
			{
				const int parachuteHeight = pFootTypeExt->Parachute_OpenHeight.Get(
							HouseTypeExt::ExtMap.Find(pFoot->Owner->Type)->Parachute_OpenHeight.Get(RulesExt::Global()->Parachute_OpenHeight));

				if (parachuteHeight == 0)
				{
					pExt->NeedParachute_Height = INT_MAX;
				}
				else
				{
					pExt->NeedParachute_Height = parachuteHeight;
				}
			}
		}
		pExt->WasFallenDown = true;
	}
}

int TechnoExt::GetSensorSight(TechnoClass* pThis)
{
	TechnoTypeClass* pType = pThis->GetTechnoType();

	return pType->SensorsSight;
}

int TechnoExt::GetCurrentDamage(int damage, FootClass* pThis)
{
	if (!TechnoExt::IsReallyAlive(pThis))
		return 0;

	int nDamage = damage;

	if (pThis->Veterancy.Veterancy >= 1.0)
	{
		if (pThis->GetTechnoType()->VeteranAbilities.FIREPOWER)
			nDamage = Game::F2I(nDamage * RulesClass::Instance->VeteranCombat);

		if (pThis->Veterancy.IsElite() && pThis->GetTechnoType()->EliteAbilities.FIREPOWER)
			nDamage = Game::F2I(nDamage * RulesClass::Instance->VeteranCombat);
	}

	if (pThis->BunkerLinkedItem && pThis->BunkerLinkedItem->WhatAmI() == AbstractType::Building)
		nDamage = Game::F2I(nDamage * RulesClass::Instance->BunkerDamageMultiplier);

	if (pThis->InOpenToppedTransport)
		nDamage = Game::F2I(nDamage * RulesClass::Instance->OpenToppedDamageMultiplier);

	nDamage = Game::F2I(nDamage * pThis->FirepowerMultiplier);

	return nDamage;
}

CoordStruct TechnoExt::GetFLH(TechnoClass* pThis, int idxWeapon)
{
	if (idxWeapon < 0)
		return CoordStruct::Empty;

	if (!TechnoExt::IsReallyAlive(pThis))
		return CoordStruct::Empty;

	const auto pExt = TechnoExt::ExtMap.Find(pThis);
	if (!pExt)
		return CoordStruct::Empty;

	const auto pTypeExt = pExt->TypeExtData;
	if (!pTypeExt)
		return CoordStruct::Empty;

	CoordStruct coord = pThis->GetWeapon(idxWeapon)->FLH;
	if (pThis->CurrentBurstIndex % 2 != 0)
		coord.Y = -coord.Y;

	bool found = false;
	CoordStruct BurstFlh;

	BurstFlh = TechnoExt::GetBurstFLH(pThis, idxWeapon, found);
	if (found)
		coord = BurstFlh;

	return coord;
}

void TechnoExt::DeleteStrafingLaser(TechnoClass* pThis, TechnoExt::ExtData* pExt)
{
	if (!pThis)
		return;

	auto pData = pExt;
	if (!pData)
		pData = TechnoExt::ExtMap.Find(pThis);

	if (!pData->StrafingLasers.empty())
	{
		for (auto& pStrafingLaser : pData->StrafingLasers)
		{
			auto it = std::find(pData->StrafingLasers.begin(), pData->StrafingLasers.end(), pStrafingLaser);
			if (it != pData->StrafingLasers.end())
			{
				pStrafingLaser = nullptr;
				pData->StrafingLasers.erase(it);
			}
		}
	}
}

void TechnoExt::AddSensorsAt(int houseindex, int range, CellStruct cell)
{
	if (range <= 0)
		return;

	for (CellSpreadEnumerator it(range); it; ++it)
	{
		auto const pCell = MapClass::Instance->GetCellAt(*it + cell);
		pCell->Sensors_AddOfHouse(houseindex);
		for (auto pUnit = (UnitClass*)pCell->FirstObject; pUnit; pUnit = (UnitClass*)pUnit->NextObject)
		{
			if (pUnit->WhatAmI() == AbstractType::Unit || pUnit->WhatAmI() == AbstractType::Infantry || pUnit->WhatAmI() == AbstractType::Aircraft)
				pUnit->Sensed();
		}

		auto building = pCell->GetBuilding();
		if (building && building->Owner->ArrayIndex != houseindex)
		{
			if (building->VisualCharacter(0, 0) != VisualType::Normal)
				building->NeedsRedraw = true;
		}
	}
}

void TechnoExt::RemoveSensorsAt(int houseindex, int range, CellStruct cell)
{
	if (range <= 0)
		return;

	for (CellSpreadEnumerator it(range); it; ++it)
	{
		auto const pCell = MapClass::Instance->GetCellAt(*it + cell);
		if (pCell->Sensors_InclHouse(houseindex))
		{
			pCell->Sensors_RemOfHouse(houseindex);
			for (auto pUnit = (UnitClass*)pCell->FirstObject; pUnit; pUnit = (UnitClass*)pUnit->NextObject)
			{
				if (pUnit->WhatAmI() == AbstractType::Unit || pUnit->WhatAmI() == AbstractType::Infantry || pUnit->WhatAmI() == AbstractType::Aircraft)
					pUnit->Sensed();
			}

			auto building = pCell->GetBuilding();
			if (building && building->Owner->ArrayIndex != houseindex)
			{
				if (building->VisualCharacter(0, 0) != VisualType::Normal)
					building->NeedsRedraw = true;
			}
		}
	}
}

int TechnoExt::TechnoFactoryPlant(TechnoTypeClass* pThis, HouseClass* pHouse)
{
	double cost = static_cast<double>(pThis->GetCost());
	auto const pHouseExt = HouseExt::ExtMap.Find(pHouse);
	auto const pBuildingType = abstract_cast<BuildingTypeClass*>(pThis);

	for (auto pTechnoType : *TechnoTypeClass::Array)
	{
		auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pTechnoType);

		if (!pTypeExt->IsExtendFactoryPlant)
			continue;

		const auto& vTechnos = HouseExt::GetOwnedTechno(pHouse, pTechnoType);
		for (size_t i = 0; i < vTechnos.size(); i++)
		{
			if (!vTechnos[i]->IsInPlayfield)
				continue;

			if (pTypeExt->FactoryPlant_Powered)
			{
				if (pTechnoType->WhatAmI() == AbstractType::BuildingType)
				{
					if (!TechnoExt::IsActivePower(vTechnos[i]))
						continue;
				}
				else
				{
					if (!TechnoExt::IsActive(vTechnos[i]))
						continue;
				}
			}

			bool checked = false;

			switch (pThis->WhatAmI())
			{
			case AbstractType::AircraftType:
				if (!pTypeExt->FactoryPlant_AircraftCostBonus_Types.empty())
				{
					auto& vTypes = pTypeExt->FactoryPlant_AircraftCostBonus_Types;
					auto it = std::find(vTypes.begin(), vTypes.end(), pThis);

					if (it == vTypes.end())
						break;
				}

				if (!pTypeExt->FactoryPlant_AircraftCostBonus_Ignore.empty())
				{
					auto& vTypes = pTypeExt->FactoryPlant_AircraftCostBonus_Ignore;
					auto it = std::find(vTypes.begin(), vTypes.end(), pThis);

					if (it != vTypes.end())
						break;
				}

				cost = cost * pTypeExt->FactoryPlant_AircraftCostBonus;

				if (!pTypeExt->FactoryPlant_AircraftCostBonus_Cumulative)
					checked = true;

				break;
			case AbstractType::BuildingType:
				if (pBuildingType->BuildCat == BuildCat::Combat)
				{
					if (!pTypeExt->FactoryPlant_DefensesCostBonus_Types.empty())
					{
						auto& vTypes = pTypeExt->FactoryPlant_DefensesCostBonus_Types;
						auto it = std::find(vTypes.begin(), vTypes.end(), pThis);

						if (it == vTypes.end())
							break;
					}

					if (!pTypeExt->FactoryPlant_DefensesCostBonus_Ignore.empty())
					{
						auto& vTypes = pTypeExt->FactoryPlant_DefensesCostBonus_Ignore;
						auto it = std::find(vTypes.begin(), vTypes.end(), pThis);

						if (it != vTypes.end())
							break;
					}

					cost = cost * pTypeExt->FactoryPlant_DefensesCostBonus;

					if (!pTypeExt->FactoryPlant_DefensesCostBonus_Cumulative)
						checked = true;

					break;
				}
				else
				{
					if (!pTypeExt->FactoryPlant_BuildingsCostBonus_Types.empty())
					{
						auto& vTypes = pTypeExt->FactoryPlant_BuildingsCostBonus_Types;
						auto it = std::find(vTypes.begin(), vTypes.end(), pThis);

						if (it == vTypes.end())
							break;
					}

					if (!pTypeExt->FactoryPlant_BuildingsCostBonus_Ignore.empty())
					{
						auto& vTypes = pTypeExt->FactoryPlant_BuildingsCostBonus_Ignore;
						auto it = std::find(vTypes.begin(), vTypes.end(), pThis);

						if (it != vTypes.end())
							break;
					}

					cost = cost * pTypeExt->FactoryPlant_BuildingsCostBonus;

					if (!pTypeExt->FactoryPlant_BuildingsCostBonus_Cumulative)
						checked = true;

					break;
				}
			case AbstractType::InfantryType:
				if (!pTypeExt->FactoryPlant_InfantryCostBonus_Types.empty())
				{
					auto& vTypes = pTypeExt->FactoryPlant_InfantryCostBonus_Types;
					auto it = std::find(vTypes.begin(), vTypes.end(), pThis);

					if (it == vTypes.end())
						break;
				}

				if (!pTypeExt->FactoryPlant_InfantryCostBonus_Ignore.empty())
				{
					auto& vTypes = pTypeExt->FactoryPlant_InfantryCostBonus_Ignore;
					auto it = std::find(vTypes.begin(), vTypes.end(), pThis);

					if (it != vTypes.end())
						break;
				}

				cost = cost * pTypeExt->FactoryPlant_InfantryCostBonus;

				if (!pTypeExt->FactoryPlant_InfantryCostBonus_Cumulative)
					checked = true;

				break;
			case AbstractType::UnitType:
				if (pThis->Organic)
				{
					if (!pTypeExt->FactoryPlant_InfantryCostBonus_Types.empty())
					{
						auto& vTypes = pTypeExt->FactoryPlant_InfantryCostBonus_Types;
						auto it = std::find(vTypes.begin(), vTypes.end(), pThis);

						if (it == vTypes.end())
							break;
					}

					if (!pTypeExt->FactoryPlant_InfantryCostBonus_Ignore.empty())
					{
						auto& vTypes = pTypeExt->FactoryPlant_InfantryCostBonus_Ignore;
						auto it = std::find(vTypes.begin(), vTypes.end(), pThis);

						if (it != vTypes.end())
							break;
					}

					cost = cost * pTypeExt->FactoryPlant_InfantryCostBonus;

					if (!pTypeExt->FactoryPlant_InfantryCostBonus_Cumulative)
						checked = true;

					break;
				}
				else if (pThis->ConsideredAircraft)
				{
					if (!pTypeExt->FactoryPlant_AircraftCostBonus_Types.empty())
					{
						auto& vTypes = pTypeExt->FactoryPlant_AircraftCostBonus_Types;
						auto it = std::find(vTypes.begin(), vTypes.end(), pThis);

						if (it == vTypes.end())
							break;
					}

					if (!pTypeExt->FactoryPlant_AircraftCostBonus_Ignore.empty())
					{
						auto& vTypes = pTypeExt->FactoryPlant_AircraftCostBonus_Ignore;
						auto it = std::find(vTypes.begin(), vTypes.end(), pThis);

						if (it != vTypes.end())
							break;
					}

					cost = cost * pTypeExt->FactoryPlant_AircraftCostBonus;

					if (!pTypeExt->FactoryPlant_AircraftCostBonus_Cumulative)
						checked = true;

					break;
				}
				else if (pThis->Naval)
				{
					if (!pTypeExt->FactoryPlant_NavalCostBonus_Types.empty())
					{
						auto& vTypes = pTypeExt->FactoryPlant_NavalCostBonus_Types;
						auto it = std::find(vTypes.begin(), vTypes.end(), pThis);

						if (it == vTypes.end())
							break;
					}

					if (!pTypeExt->FactoryPlant_NavalCostBonus_Ignore.empty())
					{
						auto& vTypes = pTypeExt->FactoryPlant_NavalCostBonus_Ignore;
						auto it = std::find(vTypes.begin(), vTypes.end(), pThis);

						if (it != vTypes.end())
							break;
					}

					cost = cost * pTypeExt->FactoryPlant_NavalCostBonus;

					if (!pTypeExt->FactoryPlant_NavalCostBonus_Cumulative)
						checked = true;

					break;
				}
				else
				{
					if (!pTypeExt->FactoryPlant_UnitsCostBonus_Types.empty())
					{
						auto& vTypes = pTypeExt->FactoryPlant_UnitsCostBonus_Types;
						auto it = std::find(vTypes.begin(), vTypes.end(), pThis);

						if (it == vTypes.end())
							break;
					}

					if (!pTypeExt->FactoryPlant_UnitsCostBonus_Ignore.empty())
					{
						auto& vTypes = pTypeExt->FactoryPlant_UnitsCostBonus_Ignore;
						auto it = std::find(vTypes.begin(), vTypes.end(), pThis);

						if (it != vTypes.end())
							break;
					}

					cost = cost * pTypeExt->FactoryPlant_UnitsCostBonus;

					if (!pTypeExt->FactoryPlant_UnitsCostBonus_Cumulative)
						checked = true;

					break;
				}
			default:
				break;
			}

			if (checked)
				break;
		}
	}

	switch (pThis->WhatAmI())
	{
	case AbstractType::AircraftType:
		for (size_t i = 0; i < pHouseExt->AircraftCostBonusTypes.size(); i++)
		{
			if (pHouseExt->AircraftCostBonusTypes[i] == pThis)
			{
				cost = cost * pHouseExt->AircraftCostBonus[i];
				break;
			}
		}

		break;
	case AbstractType::BuildingType:
		if (pBuildingType->BuildCat == BuildCat::Combat)
		{
			for (size_t i = 0; i < pHouseExt->DefensesCostBonusTypes.size(); i++)
			{
				if (pHouseExt->DefensesCostBonusTypes[i] == pThis)
				{
					cost = cost * pHouseExt->DefensesCostBonus[i];
					break;
				}
			}

			break;
		}
		else
		{
			for (size_t i = 0; i < pHouseExt->BuildingsCostBonusTypes.size(); i++)
			{
				if (pHouseExt->BuildingsCostBonusTypes[i] == pThis)
				{
					cost = cost * pHouseExt->BuildingsCostBonus[i];
					break;
				}
			}

			break;
		}
	case AbstractType::InfantryType:
		for (size_t i = 0; i < pHouseExt->InfantryCostBonusTypes.size(); i++)
		{
			if (pHouseExt->InfantryCostBonusTypes[i] == pThis)
			{
				cost = cost * pHouseExt->InfantryCostBonus[i];
				break;
			}
		}

		break;
	case AbstractType::UnitType:
		if (pThis->Organic)
		{
			for (size_t i = 0; i < pHouseExt->InfantryCostBonusTypes.size(); i++)
			{
				if (pHouseExt->InfantryCostBonusTypes[i] == pThis)
				{
					cost = cost * pHouseExt->InfantryCostBonus[i];
					break;
				}
			}

			break;
		}
		else if (pThis->ConsideredAircraft)
		{
			for (size_t i = 0; i < pHouseExt->AircraftCostBonusTypes.size(); i++)
			{
				if (pHouseExt->AircraftCostBonusTypes[i] == pThis)
				{
					cost = cost * pHouseExt->AircraftCostBonus[i];
					break;
				}
			}

			break;
		}
		else if (pThis->Naval)
		{
			for (size_t i = 0; i < pHouseExt->NavalCostBonusTypes.size(); i++)
			{
				if (pHouseExt->NavalCostBonusTypes[i] == pThis)
				{
					cost = cost * pHouseExt->NavalCostBonus[i];
					break;
				}
			}

			break;
		}
		else
		{
			for (size_t i = 0; i < pHouseExt->UnitsCostBonusTypes.size(); i++)
			{
				if (pHouseExt->UnitsCostBonusTypes[i] == pThis)
				{
					cost = cost * pHouseExt->UnitsCostBonus[i];
					break;
				}
			}

			break;
		}
	default:
		break;
	}

	return static_cast<int>(cost);
}

PhobosAction TechnoExt::GetActionPilot(InfantryClass* pThis, TechnoClass* pTarget)
{
	if (!pThis || !pTarget || !IsReallyAlive(pThis) || !IsReallyAlive(pTarget))
	{
		return PhobosAction::None;
	}

	const auto pType = pThis->Type;
	const auto pTargetType = pTarget->GetTechnoType();
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

	if (!pTypeExt->Pilot || pThis->IsDeployed())
	{
		return PhobosAction::None;
	}

	if (pTypeExt->Pilot_IgnoreTrainable ? false : !pTargetType->Trainable)
	{
		return PhobosAction::None;
	}

	const auto absTarget = pTarget->WhatAmI();
	if (absTarget == AbstractType::Infantry ||
		(!pTypeExt->Pilot_CanEnterUnit && (absTarget == AbstractType::Unit || absTarget == AbstractType::Aircraft)) ||
		(!pTypeExt->Pilot_CanEnterBuilding && absTarget == AbstractType::Building))
	{
		return PhobosAction::None;
	}

	if (pTarget->CurrentMission == Mission::Selling || pTarget->IsBeingWarpedOut()
		|| pTargetType->IsTrain
		|| !pTarget->IsOnFloor())
	{
		return PhobosAction::None;
	}

	if (pTarget->BunkerLinkedItem)
	{
		return PhobosAction::None;
	}

	bool neturalcheck = pTypeExt->Pilot_AllowHouses_IgnoreNeturalHouse ? true : false;

	if ((neturalcheck && !pTarget->Owner->IsControlledByHuman() && (strcmp(pTarget->Owner->PlainName, "Computer") != 0)) ? false : !EnumFunctions::CanTargetHouse(pTypeExt->Pilot_AllowHouses, pThis->Owner, pTarget->Owner))
	{
		return PhobosAction::None;
	}

	if (!pTypeExt->Pilot_CanEnterTypes.empty())
	{
		if (!pTypeExt->Pilot_CanEnterTypes.Contains(pTargetType))
			return PhobosAction::None;
	}

	if (!pTypeExt->Pilot_CanNotEnterTypes.empty())
	{
		if (pTypeExt->Pilot_CanNotEnterTypes.Contains(pTargetType))
			return PhobosAction::None;
	}

	if (!pTypeExt->Pilot_IgnoreTrainable)
	{
		double Veterancy = pTypeExt->Pilot_Veterancy_UseOwned ? static_cast<double>(pThis->Veterancy.Veterancy) : pTypeExt->Pilot_Veterancy;
		if ((Veterancy > 0 && (pTarget->Veterancy.Veterancy >= 2.0)) ||
			(Veterancy < 0 && (pTarget->Veterancy.Veterancy <= 0.0)))
		{
			return PhobosAction::None;
		}
	}

	const auto pTargetTypeExt = TechnoTypeExt::ExtMap.Find(pTargetType);
	if (!pTargetTypeExt->Pilot_AllowTypes.empty())
	{
		if (!pTargetTypeExt->Pilot_AllowTypes.Contains(pType))
			return PhobosAction::None;
	}

	if (!pTargetTypeExt->Pilot_DisallowTypes.empty())
	{
		if (pTargetTypeExt->Pilot_DisallowTypes.Contains(pType))
			return PhobosAction::None;
	}

	return PhobosAction::Drive;
}

bool TechnoExt::PerformActionPilot(InfantryClass* pThis, TechnoClass* pTarget)
{
	bool enter = false;

	if (IsReallyAlive(pThis))
	{
		const auto pType = pThis->Type;
		const auto pExt = TechnoExt::ExtMap.Find(pThis);
		const auto pTypeExt = pExt->TypeExtData;

		const auto action = GetActionPilot(pThis, pTarget);

		if (action == PhobosAction::None)
			return false;
		
		if (pTarget->AttachedTag)
		{
			pTarget->AttachedTag->RaiseEvent(TriggerEvent::EnteredBy,
				pThis, CellStruct::Empty, false, nullptr);
		}

		if (!pTypeExt->Pilot_AttachEffects.empty())
		{
			for (auto pAE : pTypeExt->Pilot_AttachEffects)
				TechnoExt::AttachEffect(pTarget, pThis, pAE);
		}

		const auto controller = pThis->MindControlledBy;
		if (controller)
		{
			++Unsorted::IKnowWhatImDoing;
			controller->CaptureManager->Free(pThis);
			--Unsorted::IKnowWhatImDoing;
		}

		double Veterancy = pTypeExt->Pilot_Veterancy_UseOwned ? static_cast<double>(pThis->Veterancy.Veterancy) : pTypeExt->Pilot_Veterancy;
		VeterancyStruct* vstruct = &pTarget->Veterancy;
		vstruct->Add(Veterancy);
		if (vstruct->Veterancy > 2.0)
			vstruct->SetElite();
		else if (vstruct->Veterancy < 0.0)
			vstruct->SetRookie();

		VocClass::PlayAt(pTypeExt->Pilot_EnterSound, pTarget->Location, nullptr);

		if (pTypeExt->Pilot_CanLeave)
		{
			const auto pTargetExt = TechnoExt::ExtMap.Find(pTarget);
			pTargetExt->PilotType = pType;
			pTargetExt->PilotOwner = pThis->Owner;
			pTargetExt->PilotHealth = pThis->Health;
		}

		if (pTypeExt->Pilot_ChangeOwner)
		{

			pTarget->SetOwningHouse(pThis->Owner);

			if (controller)
			{
				++Unsorted::IKnowWhatImDoing;
				controller->CaptureManager->Capture(pTarget);
				--Unsorted::IKnowWhatImDoing;
			}

			pTarget->QueueMission(Mission::Guard, true);
		}

		enter = true;
	}

	return enter;
}

void TechnoExt::KickOutPassenger(TechnoClass* pThis, int WeaponIdx)
{
	const auto pType = pThis->GetTechnoType();
	const auto pWeapon = pThis->GetWeapon(WeaponIdx);

	if (pType->WhatAmI() != AbstractType::UnitType)
		return;

	if (!pWeapon || !pWeapon->WeaponType || !pWeapon->WeaponType->Warhead)
		return;

	if (const auto pTypeExt = WeaponTypeExt::ExtMap.Find(pWeapon->WeaponType))
	{
		if (!pTypeExt->KickOutPassenger.Get(false))
			return;

		if (pType->Passengers > 0 && pThis->Passengers.NumPassengers > 0)
		{
			bool parachute = pTypeExt->KickOutPassenger_Parachute.Get(true);
			bool allowBridges = pType->SpeedType != SpeedType::Float;
			auto flh = pWeapon->FLH;

			if (pThis->CurrentBurstIndex % 2 != 0)
				flh.Y = -flh.Y;

			auto pCell = pThis->GetCell();
			CoordStruct location = TechnoExt::GetFLHAbsoluteCoords(pThis, flh, pThis->GetTechnoType()->Turret);

			if (parachute)
			{
				auto nCell = MapClass::Instance->NearByLocation(CellClass::Coord2Cell(location),
										pType->SpeedType, -1, pType->MovementZone, false, 1, 1, true,
										false, false, allowBridges, CellStruct::Empty, false, false);

				pCell = MapClass::Instance->TryGetCellAt(nCell);
			}

			if (pCell && allowBridges)
				location = pCell->GetCoordsWithBridge();

			int z = pThis->GetCoords().Z;
			location.Z = Math::max(MapClass::Instance->GetCellFloorHeight(location), z);

			FootClass* pPassenger = pThis->Passengers.GetFirstPassenger();
			FootClass* pLastPassenger = static_cast<FootClass*>(pPassenger->NextObject);

			pPassenger->NextObject = nullptr;

			if (pLastPassenger)
				pThis->Passengers.FirstPassenger = pLastPassenger;
			else
				pThis->Passengers.FirstPassenger = nullptr;

			--pThis->Passengers.NumPassengers;

			if (pPassenger)
			{
				if (pType->OpenTopped)
				{
					pThis->ExitedOpenTopped(pPassenger);
				}

				if (pType->Gunner)
				{
					if (pThis->Passengers.NumPassengers == 0)
						abstract_cast<FootClass*>(pThis)->RemoveGunner(pPassenger);
				}

				pPassenger->Transporter = nullptr;

				Unsorted::IKnowWhatImDoing++;
				pPassenger->Unlimbo(location, DirType(64));
				pPassenger->SetLocation(location);
				Unsorted::IKnowWhatImDoing--;

				const auto pPassengerType = pPassenger->GetTechnoType();
				pPassenger->OnBridge = (pPassengerType->SpeedType == SpeedType::Float || pPassengerType->SpeedType == SpeedType::FloatBeach) ?
					false : pCell->ContainsBridge();

				if (pThis->IsInAir())
				{
					if (const auto pTechno = abstract_cast<TechnoClass*>(pPassenger))
					{
						pPassenger->InAir = true;
						TechnoExt::KickOutPassenger_SetHight(pPassenger, location, 64, pTypeExt->KickOutPassenger_Parachute.Get());
						VocClass::PlayAt(pThis->GetTechnoType()->LeaveTransportSound, pThis->GetCoords());

						if (const auto pExt = TechnoExt::ExtMap.Find(pTechno))
						{
							pExt->RopeConnection = true;

							if (!parachute)
							{
								pExt->RopeConnection_Vehicle = abstract_cast<UnitClass*>(pThis);
							}
						}
					}

					pPassenger->Scatter(pPassenger->GetCoords(), true, false);
				}
				else
				{
					pPassenger->Scatter(location, true, false);
					VocClass::PlayAt(pThis->GetTechnoType()->LeaveTransportSound, pThis->GetCoords());
				}

				if (pPassenger->Owner->IsControlledByHuman())
					pPassenger->QueueMission(Mission::Guard, false);
				else
					pPassenger->QueueMission(Mission::Hunt, false);
			}
		}
	}
}

void TechnoExt::KickOutPassenger_SetHight(FootClass* pThis, CoordStruct location, short facing, bool parachute)
{
	if (auto const pJJLoco = locomotion_cast<JumpjetLocomotionClass*>(pThis->Locomotor))
	{
		auto const pType = pThis->GetTechnoType();
		const auto dir = static_cast<unsigned short>(static_cast<unsigned char>(static_cast<DirType>(facing)) * 256);

		pJJLoco->LocomotionFacing.SetCurrent(DirStruct(dir));

		if (pType->BalloonHover)
		{
			// Makes the jumpjet think it is hovering without actually moving.
			pJJLoco->State = JumpjetLocomotionClass::State::Hovering;
			pJJLoco->IsMoving = true;
			pJJLoco->DestinationCoords = location;
			pJJLoco->CurrentHeight = pType->JumpjetHeight;
		}
		else
		{
			// Order non-BalloonHover jumpjets to land.
			pJJLoco->Move_To(location);
		}
	}
	else
	{
		if (pThis->GetTechnoType()->WhatAmI() == AbstractType::InfantryType)
		{
			if (const auto pInf = abstract_cast<InfantryClass*>(pThis))
			{
				pInf->PlayAnim(Sequence::Paradrop, true);
			}
		}

		if (parachute)
		{
			if (const auto pExt = TechnoExt::ExtMap.Find(pThis))
			{
				if (const auto pTypeExt = pExt->TypeExtData)
				{
					const int height = pThis->GetCoords().Z - MapClass::Instance->GetCellFloorHeight(pThis->GetCoords());
					const int parachuteHeight = pTypeExt->Parachute_OpenHeight.Get(
									RulesExt::Global()->Parachute_OpenHeight);

					if (parachuteHeight > 0 && height > parachuteHeight)
					{
						pThis->HasParachute = false;
						pExt->NeedParachute_Height = parachuteHeight;
					}
					else
					{
						auto coords = pThis->GetCoords();
						coords.Z += 75;

						const auto parachuteAnimType = pTypeExt->Parachute_Anim.isset() ?
							pTypeExt->Parachute_Anim.Get() : RulesClass::Instance->Parachute;

						if (const auto parachuteAnim = GameCreate<AnimClass>(parachuteAnimType, coords))
						{
							VocClass::PlayAt(RulesClass::Instance->ChuteSound, coords, nullptr);
							pThis->Parachute = parachuteAnim;
							parachuteAnim->SetOwnerObject(pThis);
							parachuteAnim->LightConvert = pThis->GetRemapColour();
							parachuteAnim->TintColor = pThis->GetCell()->Intensity_Normal;
							pThis->HasParachute = true;
						}
					}
				}
			}
		}

		pThis->FallRate = -1;
		pThis->IsFallingDown = true;
	}
}

// =============================
// load / save

template <typename T>
void TechnoExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->TypeExtData)
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

		.Process(this->AutoDeathTimer_Passenger)

		.Process(this->ParentAttachment)
		.Process(this->ChildAttachments)
		.Process(this->ParentInAir)

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
		.Process(this->BeamCannon_Center)
		.Process(this->BeamCannon_ROF)
		.Process(this->BeamCannon_LengthIncrease)
		.Process(this->SendPassenger)
		.Process(this->SendPassengerData)
		.Process(this->SendPassengerMove)
		.Process(this->SendPassengerMoveHouse)
		.Process(this->SendPassengerMoveHouse_IgnoreNeturalHouse)
		.Process(this->SendPassengerOverlap)
		.Process(this->SendPassengerSelect)
		.Process(this->SendPassengerUseParachute)
		.Process(this->AllowPassengerToFire)
		.Process(this->AllowFireCount)
		.Process(this->SpawneLoseTarget)

		.Process(this->ConvertsCounts)
		.Process(this->ConvertsOriginalType)
		.Process(this->ConvertsAnim)
		.Process(this->Convert_FromTypes)
		.Process(this->Convert_DetachedBuildLimit)
		.Process(this->ConvertsTargetType)
		.Process(this->ProcessingConvertsAnim)

		.Process(this->DisableTurnCount)
		.Process(this->FacingInitialized)
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

		.Process(this->IsChargeROF)
		.Process(this->GattlingCount)
		.Process(this->GattlingStage)
		.Process(this->GattlingWeaponIndex)
		.Process(this->MaxGattlingCount)
		.Process(this->IsCharging)
		.Process(this->HasCharged)
		.Process(this->AttackTarget)
		.Process(this->AttackWeapon)
		.Process(this->GattlingWeapons)
		.Process(this->GattlingStages)
		.Process(this->FireUpTimer)
		.Process(this->PrimaryWeapon)
		.Process(this->SecondaryWeapon)
		.Process(this->WeaponFLHs)
		.Process(this->NeedConvertWhenLanding)
		.Process(this->JJ_Landed)
		.Process(this->FloatingType)
		.Process(this->LandingType)

		.Process(this->Build_As)
		.Process(this->Build_As_OnlyOne)

		.Process(this->AttackedWeapon_Timer)

		.Process(this->IsSharingWeaponRange)
		.Process(this->ShareWeaponRangeTarget)
		.Process(this->ShareWeaponRangeFacing)

		.Process(this->IFVTurrets)
		.Process(this->IFVMode)
		.Process(this->CurrtenWeapon)

		.Process(this->BuildingROFFix)

		.Process(this->Attacker)
		.Process(this->Attacker_Count)
		.Process(this->Attacker_Weapon)

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
		.Process(this->AttachEffects_ReceivedCounter)
		.Process(this->AttachWeapon_Timers)

		.Process(this->DeployAttachEffectsCount)

		.Process(this->FireSelf_Timers)

		.Process(this->LosePower)
		.Process(this->InLosePower)
		.Process(this->LosePowerAnim)
		.Process(this->LosePowerParticleCount)

		.Process(this->Temperature)
		.Process(this->Temperature_HeatUpTimer)
		.Process(this->Temperature_HeatUpDelayTimer)
		.Process(this->Temperature_WeaponTimer)

		.Process(this->ConvertPassenger)
		.Process(this->Convert_Passengers)
		.Process(this->Convert_Types)
		.Process(this->IsConverted)
		.Process(this->OrignType)

		.Process(this->CurrentRank)
		.Process(this->ReplacedArmorIdx)

		.Process(this->AcademyReset)

		.Process(this->PassengerProduct_Timer)

		.Process(this->ReceiveDamageMultiplier)
		.Process(this->NeedParachute_Height)

		.Process(this->Crate_Cloakable)

		.Process(this->DeployFireTimer)

		.Process(this->DelayedFire_Anim)
		.Process(this->DelayedFire_Anim_LoopCount)
		.Process(this->DelayedFire_DurationTimer)

		.Process(this->TargetType)

		.Process(this->MoneyReturn_Sold)

		.Process(this->HasChangeLocomotor)
		.Process(this->ChangeLocomotorTarget)
		.Process(this->IsTypeLocomotor)

		.Process(this->NextAttachEffects)
		.Process(this->NextAttachEffectsOwner)

		.Process(this->UnitDeathAnim)
		.Process(this->UnitDeathAnimOwner)

		.Process(this->LastOwner)
		.Process(this->LastTarget)

		.Process(this->Warp_Count)
		.Process(this->WarpOut_Count)

		.Process(this->TemporalTarget)
		.Process(this->TemporalTeam)
		.Process(this->TemporalStand)
		.Process(this->IsTemporalTarget)
		.Process(this->TemporalOwner)

		.Process(this->TemporalStandTarget)
		.Process(this->TemporalStandFirer)
		.Process(this->TemporalStandOwner)

		.Process(this->AddonAttachmentData)

		.Process(this->ExtraBurstTargets)
		.Process(this->ExtraBurstIndex)
		.Process(this->ExtraBurstTargetIndex)

		.Process(this->SyncDeathOwner)

		.Process(this->WasFallenDown)

		.Process(this->OnAntiGravity)
		.Process(this->WasOnAntiGravity)
		.Process(this->AntiGravityType)
		.Process(this->AntiGravityOwner)

		.Process(this->CurrtenFallRate)

		.Process(this->UnitFallWeapon)
		.Process(this->UnitFallDestory)
		.Process(this->UnitFallDestoryHeight)

		.Process(this->Landed)

		.Process(this->Deployed)

		.Process(this->CurrentFiringSW)
		.Process(this->FinishSW)
		.Process(this->EMPulseBall)

		.Process(this->CurrentTarget)
		.Process(this->isAreaProtecting)
		.Process(this->isAreaGuardReloading)
		.Process(this->areaProtectTo)
		.Process(this->areaGuardTargetCheckRof)
		.Process(this->currentAreaProtectedIndex)
		.Process(this->areaGuardCoords)
		.Process(this->AreaROF)

		.Process(this->BackwarpTimer)
		.Process(this->BackwarpColdDown)
		.Process(this->BackwarpWarpOutTimer)
		.Process(this->BackwarpLocation)
		.Process(this->BackwarpHealth)

		.Process(this->StrafingLasers)

		.Process(this->LastPassengerCheck)

		.Process(this->PilotType)
		.Process(this->PilotOwner)
		.Process(this->PilotHealth)

		.Process(this->SensorCell)
		.Process(this->EnterTarget)

		.Process(this->RopeConnection)
		.Process(this->RopeConnection_Vehicle)
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

	auto pExt = TechnoExt::ExtMap.FindOrAllocate(pItem);
	pExt->TypeExtData = TechnoTypeExt::ExtMap.Find(pItem->GetTechnoType());

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
