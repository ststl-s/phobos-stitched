#include "Body.h"

#include <sstream>

#include <HouseClass.h>
#include <TagTypeClass.h>
#include <TriggerClass.h>

#include <Helpers/Macro.h>

#include <Ext/Scenario/Body.h>

DEFINE_HOOK(0x6DD8B0, TActionClass_Execute, 0x6)
{
	GET(TActionClass*, pThis, ECX);
	GET_STACK(HouseClass*, pHouse, 0x4);
	GET_STACK(ObjectClass*, pObject, 0x8);
	GET_STACK(TriggerClass*, pTrigger, 0xC);
	GET_STACK(CellStruct const*, pLocation, 0x10);

	bool handled;

	R->AL(TActionExt::Execute(pThis, pHouse, pObject, pTrigger, *pLocation, handled));

	return handled ? 0x6DD910 : 0;
}

namespace TActionClass_SwitchAttachedObjectToHouse_Context
{
	TActionClass* pThis;
};

DEFINE_HOOK(0x6E0AA0, TActionClass_SwitchAttachedObjectToHouse_SetContext, 0x7)
{
	GET(TActionClass*, pThis, ECX);

	TActionClass_SwitchAttachedObjectToHouse_Context::pThis = pThis;

	return 0;
}

DEFINE_HOOK(0x6E0B31, TActionClass_SwitchAttachedObjectsToHouse_Passengers, 0x5)
{
	GET(TechnoClass*, pTechno, ESI);
	GET(HouseClass*, pHouse, EDI);

	TActionClass* pThis = TActionClass_SwitchAttachedObjectToHouse_Context::pThis;

	if (pThis->Param3 != 0 && pTechno->Passengers.FirstPassenger != nullptr)
	{
		FootClass* pPassenger = pTechno->Passengers.FirstPassenger;

		do
		{
			pPassenger->SetOwningHouse(pHouse, false);
			pPassenger = abstract_cast<FootClass*>(pPassenger->NextObject);
		}
		while (pPassenger != nullptr && pPassenger->Transporter == pTechno);
	}

	return 0;
}

DEFINE_HOOK(0x6E0B60, TActionClass_SwitchAllObjectsToHouse, 0x9)
{
	GET(TActionClass*, pThis, ECX);
	GET_STACK(HouseClass*, pSourceHouse, 0x4);
	GET_STACK(TriggerClass*, pTrigger, 0xC);

	bool res = false;

	if (pTrigger != nullptr)
	{
		int houseIdx = pThis->Value;
		HouseClass* pTargetHouse = nullptr;

		if (houseIdx == 8997)
		{
			pTargetHouse = pTrigger->GetHouse();
		}
		else
		{
			if (HouseClass::Index_IsMP(houseIdx))
				pTargetHouse = HouseClass::FindByIndex(houseIdx);
			else
				pTargetHouse = HouseClass::FindByCountryIndex(houseIdx);
		}

		if (pTargetHouse != nullptr)
		{
			for (TechnoClass* pTechno : *TechnoClass::Array)
			{
				if (pTechno->Owner != pSourceHouse)
					continue;

				if (pThis->Param3 && pTechno->Passengers.FirstPassenger != nullptr)
				{
					FootClass* pPassenger = pTechno->Passengers.FirstPassenger;

					do
					{
						pPassenger->SetOwningHouse(pTargetHouse, false);
						pPassenger = abstract_cast<FootClass*>(pPassenger->NextObject);
					}
					while (pPassenger != nullptr && pPassenger->Transporter == pTechno);
				}

				pTechno->SetOwningHouse(pTargetHouse, false);

				if (BuildingClass* pBuilding = abstract_cast<BuildingClass*>(pTechno))
				{
					BuildingTypeClass* pBuildingType = pBuilding->Type;

					if (pBuildingType->Powered || pBuildingType->PoweredSpecial)
					{
						pBuilding->UpdateAnim_Powered();
					}
				}

				res = true;
			}
		}
	}

	R->EAX(res);

	return 0x6E0C91;
}

// TODO: Refactor may be needed to fix buildup anim: Buildup anims plays while the building image is already there in faster gamespeed.
// Bugfix: TAction 125 Build At could neither display the buildups nor be AI-repairable in singleplayer mode
DEFINE_HOOK(0x6E427D, TActionClass_CreateBuildingAt, 0x9)
{
	GET(TActionClass*, pThis, ESI);
	GET(BuildingTypeClass*, pBldType, ECX);
	GET(HouseClass*, pHouse, EDI);
	REF_STACK(CoordStruct, coord, STACK_OFFSET(0x24, -0x18));

	auto pTaction = TActionExt::ExtMap.Find(pThis);
	auto pTagType = TagTypeClass::FindOrAllocate(pTaction->Parm4.data());

	if (pThis->Param5)
	{
		if (HouseClass::Index_IsMP(pThis->Param6))
			pHouse = HouseClass::FindByIndex(pThis->Param6);
		else
			pHouse = HouseClass::FindByCountryIndex(pThis->Param6);
	}

	bool bPlayBuildUp = pThis->Param3;

	bool bCreated = false;

	if (auto pBld = abstract_cast<BuildingClass*>(pBldType->CreateObject(pHouse)))
	{
		if (pTagType)
		{
			auto pTag = TagClass::GetInstance(pTagType);
			pBld->AttachTrigger(pTag);
		}

		if (bPlayBuildUp)
		{
			pBld->BeginMode(BStateType::Construction);
			pBld->QueueMission(Mission::Construction, false);
		}
		else
		{
			pBld->BeginMode(BStateType::Idle);
			pBld->QueueMission(Mission::Guard, false);
		}

		if (!pBld->ForceCreate(coord))
		{
			pBld->UnInit();
		}
		else
		{
			if (!bPlayBuildUp)
				pBld->Place(false);

			pBld->IsReadyToCommence = true;

			// not so sure what makes Campaign AI different than Skirmish ones, limiting to this case temporarily
			if (pThis->Param4 && SessionClass::Instance->GameMode == GameMode::Campaign && !pHouse->IsControlledByHuman())
				pBld->ShouldRebuild = true;

			bCreated = true;

			if (SessionClass::IsCampaign() && !pHouse->IsInPlayerControl)
				pBld->ShouldRebuild = true;
		}

	}

	R->AL(bCreated);
	return 0x6E42C1;
}

/*
// Bugfix: TAction 125 Build At do not display the buildups
// Author: secsome
DEFINE_HOOK(0x6E427D, TActionClass_CreateBuildingAt, 0x9)
{
	GET(TActionClass*, pThis, ESI);
	GET(BuildingTypeClass*, pBldType, ECX);
	GET(HouseClass*, pHouse, EDI);
	REF_STACK(CoordStruct, coord, STACK_OFFSET(0x24, -0x18));

	bool bPlayBuildUp = pThis->Param3;

	bool bCreated = false;
	if (auto pBld = static_cast<BuildingClass*>(pBldType->CreateObject(pHouse)))
	{
		if (bPlayBuildUp)
		{
			pBld->BeginMode(BStateType::Construction);
			pBld->QueueMission(Mission::Construction, false);
		}
		else
		{
			pBld->BeginMode(BStateType::Idle);
			pBld->QueueMission(Mission::Guard, false);
		}

		if (!pBld->ForceCreate(coord))
		{
			pBld->UnInit();
		}
		else
		{
			if (!bPlayBuildUp)
				pBld->Place(false);

			pBld->IsReadyToCommence = true;
			bCreated = true;
		}
	}

	R->AL(bCreated);
	return 0x6E42C1;
}
*/
DEFINE_HOOK_AGAIN(0x6E2F47, TActionClass_Retint_LightSourceFix, 0x3) // Blue
DEFINE_HOOK_AGAIN(0x6E2EF7, TActionClass_Retint_LightSourceFix, 0x3) // Green
DEFINE_HOOK(0x6E2EA7, TActionClass_Retint_LightSourceFix, 0x3) // Red
{
	TActionExt::RecreateLightSources();

	ScenarioExt::Global()->CurrentTint_Tiles =
		ScenarioExt::Global()->CurrentTint_Schemes =
		ScenarioExt::Global()->CurrentTint_Hashes = ScenarioClass::Instance->NormalLighting.Tint;

	return 0;
}

namespace ActionsString
{
	std::string ActionsString;
	std::deque<std::string> SubStrings;
}

DEFINE_HOOK(0x727544, TriggerClass_LoadFromINI_Actions, 0x5)
{
	GET(const char*, pString, EDX);
	ActionsString::ActionsString = pString;
	//Debug::Log("pString[%s]\n", ActionsString::ActionsString.c_str());
	std::stringstream sin(ActionsString::ActionsString);
	std::deque<std::string>& substrs = ActionsString::SubStrings;
	substrs.clear();
	std::string tmp;
	while (std::getline(sin, tmp, ','))
	{
		substrs.emplace_back(tmp);
	}
	if (!ActionsString::SubStrings.empty())
		ActionsString::SubStrings.pop_front();
	return 0;
}

DEFINE_HOOK(0x6DD5B0, TActionClass_LoadFromINI_Parm, 0x5)
{
	GET(TActionClass*, pThis, ECX);
	auto pExt = TActionExt::ExtMap.Find(pThis);
	std::deque<std::string>& substrs = ActionsString::SubStrings;

	if (substrs.empty())
		return 0;

	substrs.pop_front();

	if (substrs.empty())
		return 0;

	pExt->Value1 = substrs.front();
	substrs.pop_front();

	if (substrs.empty())
		return 0;

	pExt->Value2 = substrs.front();
	substrs.pop_front();

	if (substrs.empty())
		return 0;

	pExt->Parm3 = substrs.front();
	substrs.pop_front();

	if (substrs.empty())
		return 0;

	pExt->Parm4 = substrs.front();
	substrs.pop_front();

	if (substrs.empty())
		return 0;

	pExt->Parm5 = substrs.front();
	substrs.pop_front();

	if (substrs.empty())
		return 0;

	pExt->Parm6 = substrs.front();
	substrs.pop_front();

	if (substrs.empty())
		return 0;

	substrs.pop_front();
	/*Debug::Log("[TAction] Kind[%d],Value1[%s],Value2[%s],Parm[%s,%s,%s,%s]\n",
		ActionKind, pExt->Value1.c_str(), pExt->Value2.c_str(), pExt->Parm3.c_str(),
		pExt->Parm4.c_str(), pExt->Parm5.c_str(), pExt->Parm6.c_str());*/
	return 0;
}
