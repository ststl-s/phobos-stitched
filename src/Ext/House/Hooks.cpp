#include "Body.h"
#include <GameOptionsClass.h>
#include <HouseClass.h>
#include <Utilities/Macro.h>
#include <Utilities/Enum.h>
#include <Ext/House/Body.h>
#include <Ext/HouseType/Body.h>

#include <Misc/PhobosGlobal.h>

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

	auto pExt = HouseExt::ExtMap.Find(pThis);
	if (pExt->BuildingCheckCount == 0)
	{
		pThis->PowerOutput += pExt->PowerUnitOutPut;
		pThis->PowerDrain -= pExt->PowerUnitDrain;
		pExt->BuildingCheckCount = pExt->BuildingCount - 1;
	}
	else
		pExt->BuildingCheckCount--;

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
				pExt->AlreadyGranted.push_back(0);
			}
			pExt->ScoreVectorInited = true;
		}
		for (auto& entry : pTypeExt->ScoreSuperWeaponData)
		{
			if (pThis->SiloMoney >= entry.Score)
			{
				if (pExt->AlreadyGranted[entry.Index] < 1)
				{
					HouseExt::GrantScoreSuperPower(pThis, entry.IdxType);
					pExt->AlreadyGranted[entry.Index] = 1;
				}
			}
		}
	}

	HouseExt::TechnoDeactivate(pThis);
	HouseExt::SetWarpTechnos(pThis);
	HouseExt::SetWarpOutTechnos(pThis);
	HouseExt::TemporalStandsCheck(pThis);
	HouseExt::TechnoVeterancyInit(pThis);
	HouseExt::SpySuperWeaponCount(pThis);
	HouseExt::CheckSuperWeaponCumulativeMax(pThis);
	HouseExt::SuperWeaponCumulative(pThis);
	HouseExt::UnitFallActivate(pThis);
	HouseExt::GapRadar(pThis);
	HouseExt::RevealRadarSight(pThis);

	PhobosGlobal::Global()->CheckFallUnitQueued();
	PhobosGlobal::Global()->CheckSuperQueued();

	if (!pExt->Checked)
	{
		HouseExt::CheckTechnoType(pThis);
	}

	return 0;
}

DEFINE_HOOK(0x4FF550, HouseClass_SubCounters_OwnedNow, 0x6)
{
	GET(HouseClass*, pThis, ECX);
	GET_STACK(TechnoClass*, pTechno, 0x4);

	auto pExt = HouseExt::ExtMap.Find(pThis);
	if (pTechno->WhatAmI() == AbstractType::Building)
		pExt->BuildingCount--;
	else
	{
		auto pTypeExt = TechnoTypeExt::ExtMap.Find(pTechno->GetTechnoType());
		if (pTypeExt->Power != 0)
		{
			if (pTypeExt->Power > 0)
				pExt->PowerUnitOutPut -= pTypeExt->Power;
			else
				pExt->PowerUnitDrain -= pTypeExt->Power;
			pThis->UpdatePower();
		}
	}

	if (!TechnoExt::IsReallyAlive(pTechno))
	{
		if (!(TechnoExt::ExtMap.Find(pTechno)->MoneyReturn_Sold))
			TechnoExt::ReturnMoney(pTechno, pThis, pTechno->Location);
	}

	TechnoExt::DeleteTheBuild(pTechno);
	TechnoExt::RegisterLoss_ClearConvertFromTypesCounter(pTechno);
	HouseExt::RegisterLoss(pThis, pTechno);

	return 0;
}

DEFINE_HOOK(0x4FF700, HouseClass_AddCounters_OwnedNow, 0x6)
{
	GET(HouseClass*, pThis, ECX);
	GET_STACK(TechnoClass*, pTechno, 0x4);

	auto pExt = HouseExt::ExtMap.Find(pThis);
	if (pTechno->WhatAmI() == AbstractType::Building)
		pExt->BuildingCount++;
	else
	{
		auto pTypeExt = TechnoTypeExt::ExtMap.Find(pTechno->GetTechnoType());
		if (pTypeExt->Power != 0)
		{
			if (pTypeExt->Power > 0)
				pExt->PowerUnitOutPut += pTypeExt->Power;
			else
				pExt->PowerUnitDrain += pTypeExt->Power;
			pThis->UpdatePower();
		}
	}

	auto pTechnoExt = TechnoExt::ExtMap.Find(pTechno);
	if (!pTechnoExt->Build_As_OnlyOne)
		TechnoExt::InitializeBuild(pTechno, pTechnoExt, TechnoTypeExt::ExtMap.Find(pTechno->GetTechnoType()));

	HouseExt::RegisterGain(pThis, pTechno);

	return 0;
}

//Maybe in BuildingClass_CreateFromINIList but seems too dangerous for me
DEFINE_HOOK(0x440B4F, BuildingClass_Unlimbo_SetShouldRebuild, 0x5)
{
	enum { ContinueCheck = 0x440B58, SkipCheck = 0x440B81 };

	GET(BuildingClass* const, pThis, ESI);

	if (SessionClass::IsCampaign())
	{
		if (!pThis->BeingProduced ||
			!HouseExt::ExtMap.Find(pThis->Owner)->RepairBaseNodes[GameOptionsClass::Instance->Difficulty])
			return SkipCheck;
	}

	return ContinueCheck;
}
