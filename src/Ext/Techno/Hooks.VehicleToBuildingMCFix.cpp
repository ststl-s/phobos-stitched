#include "Body.h"

#include <Helpers/Macro.h>

#include <Ext/BuildingType/Body.h>

#include <Misc/CaptureManager.h>

namespace MindControlFixTemp
{
	bool isMindControlBeingTransferred = false;
}

void TechnoExt::TransferMindControlOnDeploy(TechnoClass* pTechnoFrom, TechnoClass* pTechnoTo)
{
	if (auto Controller = pTechnoFrom->MindControlledBy)
	{
		if (auto Manager = Controller->CaptureManager)
		{
			MindControlFixTemp::isMindControlBeingTransferred = true;

			CaptureManager::FreeUnit(Manager, pTechnoFrom, true);
			CaptureManager::CaptureUnit(Manager, pTechnoTo, true);

			if (pTechnoTo->WhatAmI() == AbstractType::Building)
			{
				pTechnoTo->QueueMission(Mission::Construction, 0);
				pTechnoTo->Mission_Construction();
			}

			MindControlFixTemp::isMindControlBeingTransferred = false;
		}
	}
	else if (auto MCHouse = pTechnoFrom->MindControlledByHouse)
	{
		pTechnoTo->MindControlledByHouse = MCHouse;
		pTechnoFrom->MindControlledByHouse = NULL;
	}

	if (auto Anim = pTechnoFrom->MindControlRingAnim)
	{
		auto ToAnim = &pTechnoTo->MindControlRingAnim;

		if (*ToAnim)
			(*ToAnim)->TimeToDie = 1;

		*ToAnim = Anim;
		Anim->SetOwnerObject(pTechnoTo);
	}
}

DEFINE_HOOK(0x739956, UnitClass_Deploy_TransferMindControl, 0x6)
{
	GET(UnitClass*, pUnit, EBP);
	GET(BuildingClass*, pStructure, EBX);
	if (pUnit->AttachedTag != nullptr)
		pStructure->AttachTrigger(pUnit->AttachedTag);
	TechnoExt::TransferMindControlOnDeploy(pUnit, pStructure);

	auto pExt = TechnoExt::ExtMap.Find(pUnit);
	for (size_t i = 0; i < pExt->AttachEffects.size(); i++)
	{
		pExt->AttachEffects[i]->Clear();
		pExt->AttachEffects.erase(pExt->AttachEffects.begin() + i);
	}

	return 0;
}

DEFINE_HOOK(0x44A03C, BuildingClass_Mi_Selling_TransferMindControl, 0x6)
{
	GET(BuildingClass*, pStructure, EBP);
	GET(UnitClass*, pUnit, EBX);

	TechnoExt::TransferMindControlOnDeploy(pStructure, pUnit);

	if (pStructure->AttachedTag != nullptr)
		pUnit->AttachTrigger(pStructure->AttachedTag);

	pUnit->QueueMission(Mission::Hunt, true);

	auto pExt = TechnoExt::ExtMap.Find(pStructure);
	for (size_t i = 0; i < pExt->AttachEffects.size(); i++)
	{
		pExt->AttachEffects[i]->Clear();
		pExt->AttachEffects.erase(pExt->AttachEffects.begin() + i);
	}

	return 0;
}

DEFINE_HOOK(0x449E2E, BuildingClass_Mi_Selling_CreateUnit, 0x6)
{
	GET(BuildingClass*, pStructure, EBP);
	R->ECX<HouseClass*>(pStructure->GetOriginalOwner());

	return 0x449E34;
}

DEFINE_HOOK(0x7396AD, UnitClass_Deploy_CreateBuilding, 0x6)
{
	GET(UnitClass*, pUnit, EBP);
	R->EDX<HouseClass*>(pUnit->GetOriginalOwner());

	return 0x7396B3;
}

DEFINE_HOOK(0x448460, BuildingClass_Captured_MuteSound, 0x6)
{
	return MindControlFixTemp::isMindControlBeingTransferred ?
		0x44848F : 0;
}
