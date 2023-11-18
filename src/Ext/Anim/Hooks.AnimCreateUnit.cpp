// Anim-to--Unit
// Author: Otamaa, revisions by Starkku

#include "Body.h"

#include <Helpers/Macro.h>

#include <BulletClass.h>
#include <HouseClass.h>
#include <JumpjetLocomotionClass.h>
#include <ScenarioClass.h>

#include <Ext/Bullet/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/AnimType/Body.h>

DEFINE_HOOK(0x737F6D, UnitClass_TakeDamage_Destroy, 0x7)
{
	GET(UnitClass* const, pThis, ESI);
	REF_STACK(args_ReceiveDamage const, Receivedamageargs, STACK_OFFSET(0x44, 0x4));

	R->ECX(R->ESI());
	TechnoExt::ExtMap.Find(pThis)->ReceiveDamage = true;
	AnimTypeExt::ProcessDestroyAnims(pThis, Receivedamageargs.Attacker);
	pThis->Destroy();

	return 0x737F74;
}

DEFINE_HOOK(0x738807, UnitClass_Destroy_DestroyAnim, 0x8)
{
	GET(UnitClass* const, pThis, ESI);

	auto const Extension = TechnoExt::ExtMap.Find(pThis);

	if (!Extension->ReceiveDamage)
		AnimTypeExt::ProcessDestroyAnims(pThis);

	return 0x73887E;
}

// Performance tweak, mark once instead of every frame.
// DEFINE_HOOK(0x423BC8, AnimClass_AI_CreateUnit_MarkOccupationBits, 0x6)
DEFINE_HOOK(0x4226F0, AnimClass_CTOR_CreateUnit_MarkOccupationBits, 0x6)
{
	GET(AnimClass* const, pThis, ESI);

	auto const pTypeExt = AnimTypeExt::ExtMap.Find(pThis->Type);

	if (pTypeExt->CreateUnit.Get())
	{
		auto location = pThis->GetCoords();

		if (auto pCell = pThis->GetCell())
			location = pCell->GetCoordsWithBridge();
		else
			location.Z = MapClass::Instance->GetCellFloorHeight(location);

		pThis->MarkAllOccupationBits(location);
	}

	return 0; //return (pThis->Type->MakeInfantry != -1) ? 0x423BD6 : 0x423C03;
}

DEFINE_HOOK(0x424932, AnimClass_AI_CreateUnit_ActualAffects, 0x6)
{
	GET(AnimClass* const, pThis, ESI);

	auto const pTypeExt = AnimTypeExt::ExtMap.Find(pThis->Type);

	if (auto unit = pTypeExt->CreateUnit.Get())
	{
		HouseClass* decidedOwner = pThis->Owner && !pThis->Owner->Defeated
			? pThis->Owner : HouseClass::FindCivilianSide();

		auto pCell = pThis->GetCell();
		CoordStruct location = pThis->GetCoords();

		pThis->UnmarkAllOccupationBits(location);

		bool allowBridges = GroundType::Array[static_cast<int>(LandType::Clear)].Cost[static_cast<int>(unit->SpeedType)] > 0.0;
		bool isBridge = allowBridges && pCell->ContainsBridge();

		if (pTypeExt->CreateUnit_ConsiderPathfinding && (!pCell || !pCell->IsClearToMove(unit->SpeedType, false, false, -1, unit->MovementZone, -1, isBridge)))
		{
			auto nCell = MapClass::Instance->NearByLocation(CellClass::Coord2Cell(location),
				unit->SpeedType, -1, unit->MovementZone, isBridge, 1, 1, true,
				false, false, isBridge, CellStruct::Empty, false, false);

			pCell = MapClass::Instance->TryGetCellAt(nCell);
			location = pCell->GetCoords();
		}

		if (pCell)
		{
			isBridge = allowBridges && pCell->ContainsBridge();
			int bridgeZ = isBridge ? CellClass::BridgeHeight : 0;
			int z = pTypeExt->CreateUnit_AlwaysSpawnOnGround ? INT32_MIN : pThis->GetCoords().Z;
			location.Z = Math::max(MapClass::Instance->GetCellFloorHeight(location) + bridgeZ, z);

			if (auto pTechno = static_cast<FootClass*>(unit->CreateObject(decidedOwner)))
			{
				bool success = false;
				auto const pExt = AnimExt::ExtMap.Find(pThis);
				TechnoClass* pInvoker = pExt->Invoker;
				HouseClass* pInvokerHouse = pExt->InvokerHouse;

				DirType facing = pTypeExt->CreateUnit_RandomFacing
					? static_cast<DirType>(ScenarioClass::Instance->Random.RandomRanged(0, 255)) : pTypeExt->CreateUnit_Facing;

				auto resultingFacing = (pTypeExt->CreateUnit_InheritDeathFacings && pExt->FromDeathUnit) ? DirType(pExt->DeathUnitFacing) : facing;
				pTechno->OnBridge = isBridge;

				if (!pCell->GetBuilding())
				{
					++Unsorted::IKnowWhatImDoing;
					success = pTechno->Unlimbo(location, resultingFacing);
					--Unsorted::IKnowWhatImDoing;
				}
				else
				{
					success = pTechno->Unlimbo(location, resultingFacing);
				}

				if (success)
				{
					auto const loc = pTechno->Location;

					if (pTypeExt->CreateUnit_SpawnAnim.isset())
					{
						const auto pAnimType = pTypeExt->CreateUnit_SpawnAnim.Get();

						if (auto const pAnim = GameCreate<AnimClass>(pAnimType, location))
						{
							AnimExt::SetAnimOwnerHouseKind(pAnim, pInvokerHouse, nullptr, false, true);

							if (auto const pAnimExt = AnimExt::ExtMap.Find(pAnim))
								pAnimExt->SetInvoker(pInvoker, pInvokerHouse);
						}
					}

					if (pTechno->HasTurret() && pExt->FromDeathUnit && pExt->DeathUnitHasTurret && pTypeExt->CreateUnit_InheritTurretFacings)
					{
						pTechno->SecondaryFacing.SetCurrent(pExt->DeathUnitTurretFacing);
						Debug::Log("CreateUnit: Using stored turret facing %d\n", pExt->DeathUnitTurretFacing.GetFacing<256>());
					}

					if (!pTechno->InLimbo)
					{
						if (pThis->IsInAir() && !pTypeExt->CreateUnit_AlwaysSpawnOnGround)
						{
							if (auto const pJJLoco = locomotion_cast<JumpjetLocomotionClass*>(pTechno->Locomotor))
							{
								auto const pType = pTechno->GetTechnoType();
								pJJLoco->LocomotionFacing.SetCurrent(DirStruct(static_cast<DirType>(resultingFacing)));

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
								if (pTypeExt->CreateUnit_UseParachute)
								{
									TechnoExt::FallenDown(pTechno);
								}
								else
								{
									pTechno->IsFallingDown = true;
									TechnoExt::ExtMap.Find(pTechno)->WasFallenDown = true;
								}
							}
						}

						pTechno->QueueMission(pTypeExt->CreateUnit_Mission.Get(), false);
					}

					if (!decidedOwner->Type->MultiplayPassive)
						decidedOwner->RecheckTechTree = true;
				}
				else
				{
					if (pTechno)
						pTechno->UnInit();
				}
			}
		}
	}

	return (pThis->Type->MakeInfantry != -1) ? 0x42493E : 0x424B31;
}

DEFINE_HOOK(0x424A3D, AnimClass_Update_MakeInfantry_ConsiderPathfinding,0x5)
{
	GET(AnimClass*, pThis, ESI);

	const auto pTypeExt = AnimTypeExt::ExtMap.Find(pThis->Type);

	if (!pTypeExt->MakeInfantry_ConsiderPathfinding)
		return 0;

	GET(InfantryClass*, pInf, EDI);
	REF_STACK(CoordStruct, coords, STACK_OFFSET(0x8C, -0x4C));

	const InfantryTypeClass* pInfType = pInf->Type;
	CellClass* pCell = MapClass::Instance->TryGetCellAt(coords);
	bool allowBridges = GroundType::Array[static_cast<int>(LandType::Clear)].Cost[static_cast<int>(pInfType->SpeedType)] > 0.0;
	bool isBridge = allowBridges && pCell != nullptr && pCell->ContainsBridge();

	if (pCell == nullptr
		|| !pCell->IsClearToMove(pInfType->SpeedType, false, false, -1, pInfType->MovementZone, -1, isBridge))
	{
		CellStruct cell = MapClass::Instance->NearByLocation
		(
			CellClass::Coord2Cell(coords),
			pInfType->SpeedType,
			-1,
			pInfType->MovementZone,
			isBridge,
			1,
			1,
			true,
			false,
			false,
			isBridge,
			CellStruct::Empty,
			false,
			false
		);

		CellClass* pTargetCell = MapClass::Instance->TryGetCellAt(cell);

		if (pTargetCell != nullptr)
		{
			coords = pTargetCell->GetCoords();
			coords.Z = Math::max(coords.Z, pThis->GetCoords().Z);
		}
	}

	return 0;
}

DEFINE_HOOK(0x424AEC, AnimClass_Update_MakeInfantry_Fall, 0x6)
{
	GET(AnimClass*, pThis, ESI);
	GET(InfantryClass*, pInf, EDI);

	if (pInf->IsInAir())
	{
		if (const auto pLoco = locomotion_cast<JumpjetLocomotionClass*>(pInf->Locomotor))
		{
			const auto pType = pInf->Type;
			pLoco->LocomotionFacing.SetCurrent(DirStruct(DirType::SouthEast));

			if (pType->BalloonHover)
			{
				pLoco->State = JumpjetLocomotionClass::State::Hovering;
				pLoco->IsMoving = true;
				pLoco->DestinationCoords = pInf->GetCoords();
				pLoco->CurrentHeight = pType->JumpjetHeight;
			}
			else
			{
				pLoco->Move_To(pInf->GetCoords());
			}
		}
		else
		{
			if (AnimTypeExt::ExtMap.Find(pThis->Type)->MakeInfantry_UseParachute)
			{
				TechnoExt::FallenDown(pInf);
			}
			else
			{
				pInf->IsFallingDown = true;
				TechnoExt::ExtMap.Find(pInf)->WasFallenDown = true;
			}
		}
	}

	return 0;
}
