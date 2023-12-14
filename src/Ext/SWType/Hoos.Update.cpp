#include "Body.h"

#include <HouseClass.h>
#include <SuperClass.h>

#include <Ext/Building/BuildingExt.h>
#include <Ext/Techno/Body.h>
#include <Ext/House/Body.h>

#include <Utilities/Macro.h>

struct SWStatus
{
	bool Available = false;
	bool PowerSourced = false;
	bool Charging = false;
};

std::vector<SWStatus> GetSuperWeaponStatus(HouseClass* pHouse)
{
	std::vector<SWStatus> status(pHouse->Supers.Count);

	if (pHouse->Defeated || pHouse->IsObserver())
		return status;

	for (BuildingClass* pBld : pHouse->Buildings)
	{
		auto pExt = TechnoExt::ExtMap.Find(pBld);

		auto UpdateStatus =
			[pBld, pExt, &status](int swIdx)
			{
				if (swIdx < 0)
					return;

				SWStatus& state = status[swIdx];
				state.Available = true;

				if (!state.Charging
					&& TechnoExt::IsActivePower(pBld))
				{
					state.PowerSourced = true;

					if (!pBld->IsBeingWarpedOut()
						&& (pBld->CurrentMission != Mission::Construction)
						&& (pBld->CurrentMission != Mission::Selling)
						&& (pBld->QueuedMission != Mission::Construction)
						&& (pBld->QueuedMission != Mission::Selling))
					{
						state.Charging = true;
					}
				}
			};

		for (const BuildingTypeClass* pUpgrade : pBld->Upgrades)
		{
			if (const auto pUpgradeExt = BuildingTypeExt::ExtMap.Find(pUpgrade))
			{
				const int swNum = pUpgradeExt->GetSuperWeaponCount();

				for (int i = 0; i < swNum; i++)
				{
					UpdateStatus(pUpgradeExt->GetSuperWeaponIndex(i, pHouse));
				}
			}
		}

		const auto pBldTypeExt = BuildingTypeExt::ExtMap.Find(pBld->Type);
		const int swNum = pBldTypeExt->GetSuperWeaponCount();

		for (int i = 0; i < swNum; i++)
		{
			UpdateStatus(pBldTypeExt->GetSuperWeaponIndex(i, pHouse));
		}
	}

	for (SuperClass* pSW : pHouse->Supers)
	{
		int swIdx = pSW->Type->GetArrayIndex();
		SWStatus& state = status[swIdx];

		const auto pSWTypeExt = SWTypeExt::ExtMap.Find(pSW->Type);
		const HouseExt::ExtData* pHouseExt = HouseExt::ExtMap.Find(pHouse);

		if ((pHouseExt->SW_Permanents.contains(swIdx) && pHouseExt->SW_Permanents.at(swIdx)
				|| pSWTypeExt->SW_AlwaysGranted)
			&& pSWTypeExt->IsAvailable(pHouse))
			state.Available = state.PowerSourced = state.Charging = true;

		if (!pSWTypeExt->SW_AllowAI && !pHouse->IsControlledByHuman())
			state.Available = false;

		if (!pSWTypeExt->SW_AllowPlayer && pHouse->IsControlledByHuman())
			state.Available = false;

		if (SessionClass::Instance->GameMode != GameMode::Campaign
			&& !Unsorted::SWAllowed)
		{
			if (pSW->Type->DisableableFromShell)
				state.Available = false;
		}

		if (pSW->IsPowered())
			state.PowerSourced &= pHouse->HasFullPower();
	}

	return status;
}

void __fastcall HouseClass_UpdateSuperWeaponOwned(HouseClass* pThis)
{
	std::vector<SWStatus> status = GetSuperWeaponStatus(pThis);

	for (auto pSW : pThis->Supers)
	{
		if (!pSW->Granted)
			continue;

		bool isCreateGoody = (!pSW->CanHold || pSW->OneTime);

		if (isCreateGoody && !pThis->Defeated)
			continue;

		const SuperWeaponTypeClass* pSWType = pSW->Type;
		const int swIdx = pSWType->GetArrayIndex();
		const SWStatus& state = status[swIdx];
		bool update = false;

		if (!state.Available || pThis->Defeated)
			update = (pSW->Lose() && HouseClass::CurrentPlayer);
		else if (state.Charging && !pSW->IsPowered())
			update = pSW->IsOnHold && pSW->SetOnHold(false);
		else if (!state.Charging && !pSW->IsPowered())
			update = !pSW->IsOnHold && pSW->SetOnHold(true);
		else if (!state.PowerSourced)
			update = pSW->IsPowered() && pSW->SetOnHold(true);
		else
			update = state.PowerSourced && pSW->SetOnHold(false);

		if (update)
		{
			if (pThis->IsCurrentPlayer())
			{
				if (Unsorted::CurrentSWType == swIdx)
				{
					Unsorted::CurrentSWType = -1;
				}
				int idxTab = SidebarClass::GetObjectTabIdx(SuperClass::AbsID, swIdx, 0);
				MouseClass::Instance->RepaintSidebar(idxTab);
			}
			pThis->RecheckTechTree = true;
		}
	}
}

DEFINE_JUMP(CALL, 0x43BEF0, GET_OFFSET(HouseClass_UpdateSuperWeaponOwned));
DEFINE_JUMP(CALL, 0x451700, GET_OFFSET(HouseClass_UpdateSuperWeaponOwned));
DEFINE_JUMP(CALL, 0x451739, GET_OFFSET(HouseClass_UpdateSuperWeaponOwned));
DEFINE_JUMP(CALL, 0x4F92F6, GET_OFFSET(HouseClass_UpdateSuperWeaponOwned));
DEFINE_JUMP(CALL, 0x508DDB, GET_OFFSET(HouseClass_UpdateSuperWeaponOwned));

void __fastcall HouseClass_UpdateSuperWeaponUnavailable(HouseClass* pThis)
{
	//pThis->UpdateSuperWeaponUnavailable();

	if (pThis->Defeated)
		return;

	std::vector<SWStatus> status = GetSuperWeaponStatus(pThis);

	for (SuperClass* pSW : pThis->Supers)
	{
		if (pSW->Granted && !pSW->OneTime)
			continue;

		const int swIdx = pSW->Type->GetArrayIndex();
		const SWStatus& state = status[swIdx];

		if (!state.Available)
			continue;

		pSW->Grant(false, pThis->IsCurrentPlayer(), !state.PowerSourced);

		if (pThis->IsCurrentPlayer())
		{
			auto pSWTypeExt = SWTypeExt::ExtMap.Find(pSW->Type);
			if (pSWTypeExt->SW_ShowCameo || !pSWTypeExt->SW_AutoFire)
			{
				MouseClass::Instance->AddCameo(AbstractType::Special, swIdx);
				int idxTab = SidebarClass::GetObjectTabIdx(SuperClass::AbsID, swIdx, 0);
				MouseClass::Instance->RepaintSidebar(idxTab);
			}
		}
	}
}

DEFINE_JUMP(CALL, 0x4409EF, GET_OFFSET(HouseClass_UpdateSuperWeaponUnavailable));
DEFINE_JUMP(CALL, 0x4F92FD, GET_OFFSET(HouseClass_UpdateSuperWeaponUnavailable));
