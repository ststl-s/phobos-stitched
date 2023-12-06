#include "CallTroops.h"

#include <map>

#include <Ext/Foot/FootExt.h>
#include <Ext/House/Body.h>
#include <Ext/Techno/Body.h>

#include <Utilities/EnumFunctions.h>
#include <Utilities/TemplateDef.h>

const char* CallTroops::GetTypeID()
{
	return "CallTroops";
}

void CallTroops::LoadFromINI(SWTypeExt::ExtData* pData, SuperWeaponTypeClass* pSW, CCINIClass* pINI)
{
	const char* section = pData->OwnerObject()->get_ID();
	INI_EX exINI(pINI);

	pData->CallTroops_Flag.Read(exINI, section, "CallTroops.Flag");
	pData->CallTroops_Types.Read(exINI, section, "CallTroops.Types");
	pData->CallTroops_Nums.Read(exINI, section, "CallTroops.Nums");
	pData->CallTroops_ForceMission.Read(exINI, section, "CallTroops.ForceMission");
	pData->CallTroops_AffectedHouse.Read(exINI, section, "CallTroops.AffectedHouse");
	pData->CallTroops_CreateTechno.Read(exINI, section, "CallTroops.CreateTechno");
	pData->CallTroops_CreateTechno_NeedsMoney.Read(exINI, section, "CallTroops.CreateTechno.NeedsMoney");
	pData->CallTroops_Overlap.Read(exINI, section, "CallTroops.Overlap");
}

bool CallTroops::Activate(SuperClass* pSW, const CellStruct& cell, bool isPlayer)
{
	const SuperWeaponTypeClass* pSWType = pSW->Type;
	const auto pSWTypeExt = SWTypeExt::ExtMap.Find(pSWType);
	const CallTroopsFlags flag = pSWTypeExt->CallTroops_Flag;
	const Mission mission = pSWTypeExt->CallTroops_ForceMission;
	const auto& targetTypes = pSWTypeExt->CallTroops_Types;
	const auto& targetNums = pSWTypeExt->CallTroops_Nums;
	HouseClass* pOwner = pSW->Owner;
	std::map<const TechnoTypeClass*, std::vector<TechnoClass*>> targets;
	std::map<const TechnoTypeClass*, int> hasNums;
	CoordStruct coords = CellClass::Cell2Coord(cell);

	std::for_each(HouseClass::Array->begin(), HouseClass::Array->end(),
		[&](const HouseClass* pHouse)
		{
			if (!EnumFunctions::CanTargetHouse(pSWTypeExt->CallTroops_AffectedHouse, pOwner, pHouse))
				return;

			for (const TechnoTypeClass* pType : targetTypes)
			{
				if (pType->WhatAmI() == AbstractType::BuildingType)
					continue;

				const auto& technos = HouseExt::GetOwnedTechno(pHouse, pType);
				targets[pType].insert(targets[pType].end(), technos.cbegin(), technos.cend());
				hasNums[pType] += static_cast<int>(technos.size());
			}
		}
	);

	for (const TechnoTypeClass* pType : targetTypes)
	{
		auto& technos = targets[pType];
		technos.erase(std::remove_if(technos.begin(), technos.end(),
			[flag](const TechnoClass* pTechno)
			{
				return !TechnoExt::IsActive(pTechno)
					|| (flag == CallTroopsFlags::MaxHealth
						&& pTechno->Health < pTechno->GetTechnoType()->Strength);
			}), technos.end());
	}

	if (pSWTypeExt->CallTroops_CreateTechno)
	{
		for (size_t i = 0; i < targetTypes.size(); i++)
		{
			TechnoTypeClass* pType = targetTypes[i];

			if (pType->WhatAmI() == AbstractType::BuildingType)
				continue;

			if (hasNums[pType] < targetNums[i])
			{
				auto& targetTechnos = targets[pType];

				if (pSWTypeExt->CallTroops_CreateTechno_NeedsMoney)
				{
					const int cost = pType->GetActualCost(pOwner);

					while (static_cast<int>(targetTechnos.size()) < targetNums[i] && pOwner->CanTransactMoney(cost))
					{
						pOwner->TransactMoney(cost);
						TechnoClass* pTechno = static_cast<TechnoClass*>(pType->CreateObject(pOwner));
						targetTechnos.emplace_back(pTechno);
					}
				}
				else
				{
					while (static_cast<int>(targetTechnos.size()) < targetNums[i])
					{
						TechnoClass* pTechno = static_cast<TechnoClass*>(pType->CreateObject(pOwner));
						targetTechnos.emplace_back(pTechno);
					}
				}
			}
		}
	}

	for (size_t i = 0; i < targetTypes.size(); i++)
	{
		const TechnoTypeClass* pType = targetTypes[i];

		if (pType->WhatAmI() == AbstractType::BuildingType)
			continue;

		auto& technos = targets[pType];

		if (flag != CallTroopsFlags::None)
		{
			std::sort(technos.begin(), technos.end(),
				[pType, &coords, flag](const TechnoClass* pTechno1, const TechnoClass* pTechno2)
				{
					switch (flag)
					{
					case CallTroopsFlags::Farthest:
						return pTechno1->GetCoords().DistanceFrom(coords) > pTechno2->GetCoords().DistanceFrom(coords);
					case CallTroopsFlags::Nearlest:
						return pTechno1->GetCoords().DistanceFrom(coords) < pTechno2->GetCoords().DistanceFrom(coords);
					case CallTroopsFlags::MaxHealth:
						return pTechno1->Health >= pType->Strength && pTechno2->Health < pType->Strength;
					case CallTroopsFlags::LessHealth:
						return pTechno1->Health < pTechno2->Health;
					case CallTroopsFlags::MoreHealth:
						return pTechno1->Health > pTechno2->Health;
					default:
						return false;
					}
				}
			);
		}

		if (static_cast<int>(technos.size()) > targetNums[i])
			technos.resize(targetNums[i]);

		FootExt::PutFootsTo(reinterpret_cast<std::vector<FootClass*>&>(technos), coords, pSWTypeExt->CallTroops_Overlap);

		if (mission != Mission::None)
		{
			std::for_each(technos.begin(), technos.end(),
				[mission](TechnoClass* pTechno)
				{
					pTechno->ForceMission(mission);
				});
		}
	}

	return true;
}
