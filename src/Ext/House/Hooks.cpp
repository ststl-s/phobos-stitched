#include "Body.h"

#include <HouseClass.h>
#include <Utilities/Macro.h>
#include <Utilities/Enum.h>
#include <Ext/House/Body.h>
#include <Ext/HouseType/Body.h>

#include "../Techno/Body.h"
#include "../Building/Body.h"
#include <unordered_map>

DEFINE_HOOK(0x508C30, HouseClass_UpdatePower_UpdateCounter, 0x5)
{
	GET(HouseClass*, pThis, ECX);
	auto pHouseExt = HouseExt::ExtMap.Find(pThis);

	pHouseExt->BuildingCounter.clear();

	// This pre-iterating ensure our process to be done in O(NM) instead of O(N^2),
	// as M should be much less than N, this will be a great improvement. - secsome
	for (auto& pBld : pThis->Buildings)
	{
		if (pBld && !pBld->InLimbo && pBld->IsOnMap)
		{
			const auto pExt = BuildingTypeExt::ExtMap.Find(pBld->Type);
			if (pExt->PowerPlantEnhancer_Buildings.size() &&
				(pExt->PowerPlantEnhancer_Amount != 0 || pExt->PowerPlantEnhancer_Factor != 1.0f))
			{
				++pHouseExt->BuildingCounter[pExt];
			}
		}
	}

	return 0;
}

// Power Plant Enhancer #131
DEFINE_HOOK(0x508CF2, HouseClass_UpdatePower_PowerOutput, 0x7)
{
	GET(HouseClass*, pThis, ESI);
	GET(BuildingClass*, pBld, EDI);

	pThis->PowerOutput += BuildingTypeExt::GetEnhancedPower(pBld, pThis);

	return 0x508D07;
}

DEFINE_HOOK(0x73E474, UnitClass_Unload_Storage, 0x6)
{
	GET(BuildingClass* const, pBuilding, EDI);
	GET(int const, idxTiberium, EBP);
	REF_STACK(float, amount, 0x1C);

	auto pTypeExt = BuildingTypeExt::ExtMap.Find(pBuilding->Type);
	if (!pTypeExt)
		return 0;

	if (!pBuilding->Owner)
		return 0;

	auto storageTiberiumIndex = RulesExt::Global()->Storage_TiberiumIndex;

	if (pTypeExt->Refinery_UseStorage && storageTiberiumIndex >= 0)
	{
		BuildingExt::StoreTiberium(pBuilding, amount, idxTiberium, storageTiberiumIndex);
		amount = 0.0f;
	}

	return 0;
}

DEFINE_HOOK(0x4F8440, HouseClass_AI_ScoreCheck, 0x5)
{
	GET(HouseClass* const, pThis, ECX);

	auto pExt = HouseExt::ExtMap.Find(pThis);
	auto pTypeExt = HouseTypeExt::ExtMap.Find(pThis->Type);

	if (!pExt || !pTypeExt)
		return 0;

	if (pTypeExt->ScoreSuperWeapon_OnlyOnce)
	{
		for (auto& entry : pTypeExt->ScoreSuperWeaponData)
		{
			if (!entry.AlreadyGranted && pThis->SiloMoney >= entry.Score)
			{
				HouseExt::GrantScoreSuperPower(pThis, entry.IdxType);
				entry.AlreadyGranted = true;
			}
		}
	}
	else
	{
		if (!pExt->ScoreVectorInited)
		{
			//for (auto& entry : pTypeExt->ScoreSuperWeaponData)
			for (unsigned int i = 0; i < pTypeExt->ScoreSuperWeaponData.size(); i++)
			{
				pExt->vAlreadyGranted.push_back(0);
			}
			pExt->ScoreVectorInited = true;
		}
		for (auto& entry : pTypeExt->ScoreSuperWeaponData)
		{
			if (pThis->SiloMoney >= entry.Score)
			{
				if (pExt->vAlreadyGranted[entry.Index] < 1)
				{
					HouseExt::GrantScoreSuperPower(pThis, entry.IdxType);
					pExt->vAlreadyGranted[entry.Index] = 1;
				}
			}
		}
	}

	return 0;
}

DEFINE_HOOK(0x4F8361, HouseClass_CanBuild, 0x18)
{
	// int (TechnoTypeClass *item, bool BuildLimitOnly, bool includeQueued)
	/* return
		 1 - cameo shown
		 0 - cameo not shown
		-1 - cameo greyed out
	 */

	GET(HouseClass*, pThis, ECX);
	GET_STACK(TechnoTypeClass*, pItem, 0x4);
	GET_STACK(bool, buildLimitOnly, 0x8);
	GET_STACK(bool, includeQueued, 0xC);

	//if(buildLimitOnly)
	/*if (std::string(pItem->get_ID()) == "E1")
		Debug::Log("[BuildLimit] OriginCheck[%d]\n", R->EAX());*/

	HouseExt::BuildLimitStatus Origin = static_cast<HouseExt::BuildLimitStatus>(R->EAX());
	HouseExt::BuildLimitStatus newStatus = HouseExt::BuildLimitGroupCheck(pThis, pItem, buildLimitOnly, includeQueued, Origin);
	R->EAX(static_cast<DWORD>(newStatus));

	pThis->RecheckTechTree = true;
	return 0;
}