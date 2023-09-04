#include "WeaponDetonateOnTechno.h"

#include <AircraftClass.h>
#include <BuildingClass.h>
#include <InfantryClass.h>
#include <UnitClass.h>

#include <Ext/House/Body.h>
#include <Ext/WeaponType/Body.h>

#include <Utilities/EnumFunctions.h>
#include <Utilities/TemplateDef.h>

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
			if (pSWTypeExt->SW_AffectsTarget & AffectedTarget::Building)
				ProcessSW(pSWTypeExt, pHouseExt->OwnedBuilding);

			if (pSWTypeExt->SW_AffectsTarget & AffectedTarget::Unit)
				ProcessSW(pSWTypeExt, pHouseExt->OwnedUnit);

			if (pSWTypeExt->SW_AffectsTarget & AffectedTarget::Infantry)
				ProcessSW(pSWTypeExt, pHouseExt->OwnedInfantry);

			if (pSWTypeExt->SW_AffectsTarget & AffectedTarget::Aircraft)
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
					if (pSWTypeExt->SW_AffectsTarget & AffectedTarget::Building)
						ProcessSW(pSWTypeExt, pHouseExt->OwnedBuilding[arrayIdx]);
					break;
				case AbstractType::UnitType:
					if (pSWTypeExt->SW_AffectsTarget & AffectedTarget::Unit)
						ProcessSW(pSWTypeExt, pHouseExt->OwnedUnit[arrayIdx]);
					break;
				case AbstractType::InfantryType:
					if (pSWTypeExt->SW_AffectsTarget & AffectedTarget::Infantry)
						ProcessSW(pSWTypeExt, pHouseExt->OwnedInfantry[arrayIdx]);
					break;
				case AbstractType::AircraftType:
					if (pSWTypeExt->SW_AffectsTarget & AffectedTarget::Aircraft)
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
						if (pSWTypeExt->SW_AffectsTarget & AffectedTarget::Building)
							ProcessSW(pSWTypeExt, pHouseExt->OwnedBuilding[arrayIdx]);
						break;
					case AbstractType::UnitType:
						if (pSWTypeExt->SW_AffectsTarget & AffectedTarget::Unit)
							ProcessSW(pSWTypeExt, pHouseExt->OwnedUnit[arrayIdx]);
						break;
					case AbstractType::InfantryType:
						if (pSWTypeExt->SW_AffectsTarget & AffectedTarget::Infantry)
							ProcessSW(pSWTypeExt, pHouseExt->OwnedInfantry[arrayIdx]);
						break;
					case AbstractType::AircraftType:
						if (pSWTypeExt->SW_AffectsTarget & AffectedTarget::Aircraft)
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

template <typename T>
concept IsTechnoType = std::is_base_of<TechnoTypeClass, T>::value;

template <HasTypeClass Entity>
void WeaponDetonateOnTechno::ProcessSW(SWTypeExt::ExtData* pSWTypeExt, const std::map<int, std::vector<Entity*>>& ownedTechnos)
{

	for (auto& item : ownedTechnos)
	{
		decltype(item.second[0]) pType = pType->Array->GetItem(item.first);

		if constexpr (IsTechnoType<Entity>)
			if (pSWTypeExt->WeaponDetonate_TechnoTypes_Ignore.Contains(pType))
				continue;

		ProcessSW(pSWTypeExt, item.second);
	}
}

template <HasTypeClass Entity>
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
