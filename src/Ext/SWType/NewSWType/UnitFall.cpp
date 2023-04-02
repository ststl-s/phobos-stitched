#include "UnitFall.h"

#include <JumpjetLocomotionClass.h>

#include <Misc/PhobosGlobal.h>

#include <Ext/House/Body.h>

const char* UnitFall::GetTypeID()
{
	return "UnitFall";
}

void UnitFall::LoadFromINI(SWTypeExt::ExtData* pData, SuperWeaponTypeClass* pSW, CCINIClass* pINI)
{
	const char* pSection = pSW->get_ID();

	INI_EX exINI(pINI);

	pData->UnitFall_RandomPick.Read(exINI, pSection, "UnitFall.RandomPick");
	pData->UnitFall_RandomInRange.Read(exINI, pSection, "UnitFall.RandomInRange");

	for (int i = 0;; i++)
	{
		NullableIdx<TechnoTypeClass> unitType;
		char keyName[0x30];
		sprintf_s(keyName, "UnitFall%d.Type", i);
		unitType.Read(exINI, pSection, keyName);

		if (!unitType.isset())
			break;

		Nullable<int> deferment;
		sprintf_s(keyName, "UnitFall%d.Deferment", i);
		deferment.Read(exINI, pSection, keyName);

		if (!deferment.isset())
			deferment = 0;

		Nullable<int> height;
		sprintf_s(keyName, "UnitFall%d.Height", i);
		height.Read(exINI, pSection, keyName);

		if (!height.isset())
			height = RulesClass::Instance->FlightLevel;

		Nullable<bool> parachute;
		sprintf_s(keyName, "UnitFall%d.UseParachute", i);
		parachute.Read(exINI, pSection, keyName);

		if (!parachute.isset())
			parachute = false;

		Nullable<OwnerHouseKind> owner;
		sprintf_s(keyName, "UnitFall%d.Owner", i);
		owner.Read(exINI, pSection, keyName);

		if (!owner.isset())
			owner = OwnerHouseKind::Default;

		Nullable<WeaponTypeClass*> weapon;
		sprintf_s(keyName, "UnitFall%d.Weapon", i);
		weapon.Read(exINI, pSection, keyName);

		if (!weapon.isset())
			weapon = nullptr;

		Nullable<AnimTypeClass*> anim;
		sprintf_s(keyName, "UnitFall%d.Anim", i);
		anim.Read(exINI, pSection, keyName);

		if (!anim.isset())
			anim = nullptr;

		Nullable<double> weight;
		sprintf_s(keyName, "UnitFall%d.RandomPickWeight", i);
		weight.Read(exINI, pSection, keyName);

		if (!weight.isset())
			weight = 0.5;

		Nullable<unsigned short> facing;
		sprintf_s(keyName, "UnitFall%d.Facing", i);
		facing.Read(exINI, pSection, keyName);

		if (!facing.isset())
			facing = (unsigned short)0;

		Nullable<bool> randomfacing;
		sprintf_s(keyName, "UnitFall%d.RandomFacing", i);
		randomfacing.Read(exINI, pSection, keyName);

		if (!randomfacing.isset())
			randomfacing = false;

		Nullable<int> health;
		sprintf_s(keyName, "UnitFall%d.Health", i);
		health.Read(exINI, pSection, keyName);

		if (!health.isset())
			health = -1;

		Nullable<Mission> mission;
		sprintf_s(keyName, "UnitFall%d.Mission", i);
		mission.Read(exINI, pSection, keyName);

		if (!mission.isset())
			mission = Mission::Guard;

		Nullable<double> veterancy;
		sprintf_s(keyName, "UnitFall%d.Veterancy", i);
		veterancy.Read(exINI, pSection, keyName);

		if (!veterancy.isset())
			veterancy = 0.0;

		Nullable<bool> destroy;
		sprintf_s(keyName, "UnitFall%d.Destory", i);
		destroy.Read(exINI, pSection, keyName);

		if (!destroy.isset())
			destroy = false;

		Nullable<int> destoryheight;
		sprintf_s(keyName, "UnitFall%d.DestoryHeight", i);
		destoryheight.Read(exINI, pSection, keyName);

		if (!destoryheight.isset())
			destoryheight = -1;

		Nullable<bool> always;
		sprintf_s(keyName, "UnitFall%d.AlwaysFall", i);
		always.Read(exINI, pSection, keyName);

		if (!always.isset())
			always = false;

		auto pTechnoType = TechnoTypeClass::Array->GetItem(unitType.Get());
		pData->UnitFall_Types.emplace_back(pTechnoType);
		pData->UnitFall_Deferments.emplace_back(deferment.Get());
		pData->UnitFall_Heights.emplace_back(height.Get());
		pData->UnitFall_UseParachutes.emplace_back(parachute.Get());
		pData->UnitFall_Owners.emplace_back(owner.Get());
		pData->UnitFall_Weapons.emplace_back(weapon.Get());
		pData->UnitFall_Anims.emplace_back(anim.Get());
		pData->UnitFall_RandomPickWeights.emplace_back(weight.Get());
		pData->UnitFall_Facings.emplace_back(facing.Get());
		pData->UnitFall_RandomFacings.emplace_back(randomfacing.Get());
		pData->UnitFall_Healths.emplace_back(health.Get());
		pData->UnitFall_Missions.emplace_back(mission.Get());
		pData->UnitFall_Veterancys.emplace_back(veterancy.Get());
		pData->UnitFall_Destorys.emplace_back(destroy.Get());
		pData->UnitFall_DestoryHeights.emplace_back(destoryheight.Get());
		pData->UnitFall_AlwaysFalls.emplace_back(always.Get());
	}
}

bool UnitFall::Activate(SuperClass* pSW, const CellStruct& cell, bool isPlayer)
{
	SWTypeExt::ExtData* pSWTypeExt = SWTypeExt::ExtMap.Find(pSW->Type);
	HouseClass* pHouse = pSW->Owner;
	const std::vector<TechnoTypeClass*>& vTypes = pSWTypeExt->UnitFall_Types;
	const std::vector<int>& vDeferments = pSWTypeExt->UnitFall_Deferments;

	if (vTypes.empty())
	{
		Unsorted::CurrentSWType = -1;
		return true;
	}

	for (size_t i = 0; i < vTypes.size(); i++)
	{
		if (pSWTypeExt->UnitFall_RandomPick)
		{
			double dice = ScenarioClass::Instance->Random.RandomDouble();
			if (pSWTypeExt->UnitFall_RandomPickWeights[i] < dice)
				continue;
		}

		TechnoTypeClass* pTechnoType = vTypes[i];
		int iDeferment = vDeferments[i];

		if (iDeferment <= 0)
		{
			auto newOwner = HouseExt::GetHouseKind(pSWTypeExt->UnitFall_Owners[i], true, pHouse, pHouse, pHouse);

			HouseClass* decidedOwner = newOwner && !newOwner->Defeated
				? newOwner : HouseClass::FindCivilianSide();

			bool allowBridges = pTechnoType->SpeedType != SpeedType::Float;
			auto pCell = MapClass::Instance->TryGetCellAt(cell);
			CoordStruct location = pCell->GetCoords();

			if (pSWTypeExt->UnitFall_RandomInRange)
			{
				int range = static_cast<int>(pSW->Type->Range * 256);
				double random = ScenarioClass::Instance()->Random.RandomRanged(0, range);
				double theta = ScenarioClass::Instance()->Random.RandomDouble() * Math::TwoPi;

				CoordStruct offset
				{
					static_cast<int>(random * Math::cos(theta)),
					static_cast<int>(random * Math::sin(theta)),
					0
				};
				location += offset;

				auto NewCell = CellClass::Coord2Cell(location);
				pCell = MapClass::Instance->TryGetCellAt(NewCell);
			}

			location.Z = pSWTypeExt->UnitFall_Heights[i] > 0 ? pSWTypeExt->UnitFall_Heights[i] : 0;

			if (pCell && allowBridges)
				location = pCell->GetCoordsWithBridge();

			auto nCell = MapClass::Instance->NearByLocation(CellClass::Coord2Cell(location),
					pTechnoType->SpeedType, -1, pTechnoType->MovementZone, false, 1, 1, false,
					false, false, allowBridges, CellStruct::Empty, false, false);

			pCell = MapClass::Instance->TryGetCellAt(nCell);

			if (pCell && allowBridges)
				location = pCell->GetCoordsWithBridge();

			location.Z = pSWTypeExt->UnitFall_Heights[i];

			if (auto pTechno = static_cast<FootClass*>(pTechnoType->CreateObject(decidedOwner)))
			{
				bool success = false;

				auto aFacing = pSWTypeExt->UnitFall_RandomFacings[i]
					? static_cast<unsigned short>(ScenarioClass::Instance->Random.RandomRanged(0, 255)) : pSWTypeExt->UnitFall_Facings[i];

				if (pCell && allowBridges)
					pTechno->OnBridge = pCell->ContainsBridge();

				BuildingClass* pBuilding = pCell ? pCell->GetBuilding() : MapClass::Instance->TryGetCellAt(location)->GetBuilding();

				if (!pBuilding)
				{
					++Unsorted::IKnowWhatImDoing;
					success = pTechno->Unlimbo(location, static_cast<DirType>(aFacing));
					--Unsorted::IKnowWhatImDoing;
				}
				else
				{
					success = pTechno->Unlimbo(location, static_cast<DirType>(aFacing));
				}

				if (success)
				{
					if (!pTechno->InLimbo)
					{
						if (pTechno->IsInAir())
						{
							const auto pTechnoExt = TechnoExt::ExtMap.Find(pTechno);
							if (auto const pJJLoco = locomotion_cast<JumpjetLocomotionClass*>(pTechno->Locomotor))
							{
								if (!pSWTypeExt->UnitFall_AlwaysFalls[i])
								{
									pTechno->IsFallingDown = false;
									pTechnoExt->WasFallenDown = false;
									pTechnoExt->CurrtenFallRate = 0;
									pTechno->FallRate = pTechnoExt->CurrtenFallRate;

									pJJLoco->LocomotionFacing.SetCurrent(DirStruct(static_cast<DirType>(aFacing)));

									if (pTechnoType->BalloonHover)
									{
										// Makes the jumpjet think it is hovering without actually moving.
										pJJLoco->State = JumpjetLocomotionClass::State::Hovering;
										pJJLoco->IsMoving = true;
										pJJLoco->DestinationCoords = location;
										pJJLoco->CurrentHeight = pTechnoType->JumpjetHeight;
									}
									else
									{
										// Order non-BalloonHover jumpjets to land.
										pJJLoco->Move_To(location);
									}

									pTechnoExt->UnitFallWeapon = nullptr;
									pTechnoExt->UnitFallDestory = false;
									pTechnoExt->UnitFallDestoryHeight = -1;
								}
								else
								{
									if (pSWTypeExt->UnitFall_UseParachutes[i])
										TechnoExt::FallenDown(pTechno);
									else
									{
										pTechno->IsFallingDown = true;
										pTechnoExt->WasFallenDown = true;
									}

									pTechnoExt->UnitFallWeapon = pSWTypeExt->UnitFall_Weapons[i];
									pTechnoExt->UnitFallDestory = pSWTypeExt->UnitFall_Destorys[i];
									pTechnoExt->UnitFallDestoryHeight = pSWTypeExt->UnitFall_DestoryHeights[i];
								}
							}
							else
							{
								if (pSWTypeExt->UnitFall_UseParachutes[i])
									TechnoExt::FallenDown(pTechno);
								else
								{
									pTechno->IsFallingDown = true;
									pTechnoExt->WasFallenDown = true;
								}

								pTechnoExt->UnitFallWeapon = pSWTypeExt->UnitFall_Weapons[i];
								pTechnoExt->UnitFallDestory = pSWTypeExt->UnitFall_Destorys[i];
								pTechnoExt->UnitFallDestoryHeight = pSWTypeExt->UnitFall_DestoryHeights[i];
							}
						}
						pTechno->QueueMission(pSWTypeExt->UnitFall_Missions[i], false);

						if (pSWTypeExt->UnitFall_Healths[i] > 0)
							pTechno->Health = pSWTypeExt->UnitFall_Healths[i];

						if (pSWTypeExt->UnitFall_Veterancys[i] > 0)
						{
							VeterancyStruct* vstruct = &pTechno->Veterancy;
							vstruct->Add(pSWTypeExt->UnitFall_Veterancys[i]);
							if (vstruct->IsElite())
								vstruct->SetElite();
						}
						
						if (pSWTypeExt->UnitFall_Anims[i] != nullptr)
						{
							auto pAnim = GameCreate<AnimClass>(pSWTypeExt->UnitFall_Anims[i], pTechno->Location);
							pAnim->Owner = pTechno->Owner;
						}
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
		else
		{
			PhobosGlobal::Global()->FallUnit_Queued.emplace_back(pSW, iDeferment, cell, i);
		}

		HouseExt::UnitFallCheck(pHouse, pSW, cell);
	}

	Unsorted::CurrentSWType = -1;

	return true;
}
