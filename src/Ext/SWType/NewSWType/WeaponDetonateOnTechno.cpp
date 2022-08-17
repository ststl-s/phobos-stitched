#include "WeaponDetonateOnTechno.h"

#include <Utilities/EnumFunctions.h>

#include <Ext/House/Body.h>
#include <Ext/WeaponType/Body.h>

const char* WeaponDetonateOnTechno::GetTypeID()
{
	return "WeaponDetonateOnTechno";
}

void WeaponDetonateOnTechno::LoadFromINI(SWTypeExt::ExtData* pData, SuperWeaponTypeClass* pSW, CCINIClass* pINI)
{
	INI_EX exINI(pINI);
	const char* pSection = pSW->get_ID();

	pData->WeaponDetonate_Weapons.Read(exINI, pSection, "WeaponDetonate.Weapons");
	pData->WeaponDetonate_TechnoTypes.Read(exINI, pSection, "WeaponDetonate.TechnoTypes");
	pData->WeaponDetonate_TechnoTypes_Ignore.Read(exINI, pSection, "WeaponDetonate.TechnoTypes.Ignore");
	pData->WeaponDetonate_RandomPick_Weapon.Read(exINI, pSection, "WeaponDetonate.RandomPick.Weapon");
	pData->WeaponDetonate_RandomPick_Weapon_Weights.Read(exINI, pSection, "WeaponDetonate.RandomPick.Weapon.Weights");
	pData->WeaponDetonate_RandomPick_TechnoType.Read(exINI, pSection, "WeaponDetonate.RandomPick.TechnoType");
	pData->WeaponDetonate_RandomPick_TechnoType_Weights.Read(exINI, pSection, "WeaponDetonate.RandomPick.TechnoType.Weights");

	if (pData->WeaponDetonate_PerSum_WeaponWeights.empty())
	{
		ValueableVector<int>& vWeights = pData->WeaponDetonate_RandomPick_Weapon_Weights;
		int sum = 0;

		for (size_t i = 0; i < vWeights.size(); i++)
		{
			sum += vWeights[i];
			pData->WeaponDetonate_PerSum_WeaponWeights.emplace(sum, i);
		}
	}

	if (pData->WeaponDetonate_PerSum_TechnoTypeWeights.empty())
	{
		ValueableVector<int>& vWeights = pData->WeaponDetonate_RandomPick_TechnoType_Weights;
		int sum = 0;

		for (size_t i = 0; i < vWeights.size(); i++)
		{
			sum += vWeights[i];
			pData->WeaponDetonate_PerSum_TechnoTypeWeights.emplace(sum, i);
		}
	}
}

bool WeaponDetonateOnTechno::Activate(SuperClass* pSW, const CellStruct& cell, bool isPlayer)
{
	auto pSWTypeExt = SWTypeExt::ExtMap.Find(pSW->Type);

	for (HouseClass* pHouse : *HouseClass::Array)
	{
		if (!EnumFunctions::CanTargetHouse(pSWTypeExt->SW_AffectsHouse, pSW->Owner, pHouse))
			continue;

		auto pHouseExt = HouseExt::ExtMap.Find(pHouse);

		if (pSWTypeExt->WeaponDetonate_TechnoTypes.empty())
		{
			ProcessSW(pSWTypeExt, pHouseExt->OwnedBuilding);
			ProcessSW(pSWTypeExt, pHouseExt->OwnedUnit);
			ProcessSW(pSWTypeExt, pHouseExt->OwnedInfantry);
			ProcessSW(pSWTypeExt, pHouseExt->OwnedAircraft);
		}
		else
		{
			if (pSWTypeExt->WeaponDetonate_RandomPick_TechnoType)
			{
				int randNum = ScenarioClass::Instance->Random.RandomRanged(0, pSWTypeExt->WeaponDetonate_PerSum_TechnoTypeWeights.rbegin()->first - 1);
				int idx = pSWTypeExt->WeaponDetonate_PerSum_TechnoTypeWeights.upper_bound(randNum)->second;
				TechnoTypeClass* pTechnoType = pSWTypeExt->WeaponDetonate_TechnoTypes[idx];
				int arrayIdx = pTechnoType->GetArrayIndex();

				switch (pTechnoType->WhatAmI())
				{
				case AbstractType::BuildingType:
					ProcessSW(pSWTypeExt, pHouseExt->OwnedBuilding[arrayIdx]);
					break;
				case AbstractType::UnitType:
					ProcessSW(pSWTypeExt, pHouseExt->OwnedUnit[arrayIdx]);
					break;
				case AbstractType::InfantryType:
					ProcessSW(pSWTypeExt, pHouseExt->OwnedInfantry[arrayIdx]);
					break;
				case AbstractType::AircraftType:
					ProcessSW(pSWTypeExt, pHouseExt->OwnedAircraft[arrayIdx]);
					break;
				default:
					break;
				}
			}
			else
			{
				for (TechnoTypeClass* pTechnoType : pSWTypeExt->WeaponDetonate_TechnoTypes)
				{
					int arrayIdx = pTechnoType->GetArrayIndex();

					switch (pTechnoType->WhatAmI())
					{
					case AbstractType::BuildingType:
						ProcessSW(pSWTypeExt, pHouseExt->OwnedBuilding[arrayIdx]);
						break;
					case AbstractType::UnitType:
						ProcessSW(pSWTypeExt, pHouseExt->OwnedUnit[arrayIdx]);
						break;
					case AbstractType::InfantryType:
						ProcessSW(pSWTypeExt, pHouseExt->OwnedInfantry[arrayIdx]);
						break;
					case AbstractType::AircraftType:
						ProcessSW(pSWTypeExt, pHouseExt->OwnedAircraft[arrayIdx]);
						break;
					default:
						break;
					}
				}
			}
		}
	}

	Unsorted::CurrentSWType = -1;

	return true;
}

template <class Entity>
void WeaponDetonateOnTechno::ProcessSW(SWTypeExt::ExtData* pSWTypeExt, const std::map<int, std::vector<Entity*>>& ownedTechnos)
{
	for (auto& item : ownedTechnos)
	{
		Entity::type_class* pType = Entity::type_class::Array->GetItem(item.first);

		if (pSWTypeExt->WeaponDetonate_TechnoTypes_Ignore.Contains(pType))
			continue;

		ProcessSW(pSWTypeExt, item.second);
	}
}

template <class Entity>
void WeaponDetonateOnTechno::ProcessSW(SWTypeExt::ExtData* pSWTypeExt, const std::vector<Entity*>& vTechnos)
{
	std::vector<WeaponTypeClass*> vWeapons;

	if (pSWTypeExt->WeaponDetonate_RandomPick_Weapon)
	{
		int randNum = ScenarioClass::Instance->Random.RandomRanged(0, pSWTypeExt->WeaponDetonate_PerSum_WeaponWeights.rbegin()->first - 1);
		int idx = pSWTypeExt->WeaponDetonate_PerSum_WeaponWeights.upper_bound(randNum)->second;
		vWeapons = std::move(std::vector<WeaponTypeClass*>(1, pSWTypeExt->WeaponDetonate_Weapons[idx]));
	}
	else
	{
		vWeapons = pSWTypeExt->WeaponDetonate_Weapons;
	}

	for (TechnoClass* pTechno : vTechnos)
	{
		for (WeaponTypeClass* pWeapon : vWeapons)
		{
			WeaponTypeExt::DetonateAt(pWeapon, pTechno, pTechno);
		}
	}
}
