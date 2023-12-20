#include <HouseClass.h>
#include <UnitClass.h>

#include <Ext/TechnoType/Body.h>

#include <Utilities/AresFunctions.h>
#include <Utilities/Macro.h>

DEFINE_HOOK(0x73B0B0, UnitClass_DrawIfVisible, 0xA)
{
	GET(UnitClass*, pThis, ECX);
	GET_STACK(RectangleStruct*, pBounds, 0x4);
	GET_STACK(bool, evenIfCloaked, 0x8);

	bool result = !pThis->unknown_bool_418;
	TechnoClass* pNthLink = pThis->GetNthLink();

	if (pNthLink != nullptr)
	{
		result |= pNthLink->WhatAmI() != AbstractType::Building;
		result |= pNthLink->GetCurrentMission() != Mission::Unload && pNthLink->QueuedMission != Mission::Unload;
		result |= !pNthLink->UnloadTimer.AreStates11()
			&& !pNthLink->UnloadTimer.AreStates10()
			&& !pNthLink->UnloadTimer.AreStates01()
			&& !pNthLink->UnloadTimer.AreStates00();
	}

	result &= pThis->ObjectClass::DrawIfVisible(pBounds, evenIfCloaked, 0);

	R->EAX(result);

	return 0x73B139;
}

DEFINE_HOOK(0x73DE78, UnitClass_Mi_Unload_Deploy, 0x5)
{
	GET(UnitClass*, pThis, ESI);

	if (pThis->Deployed)
		pThis->Undeploy();
	else
		pThis->Deploy();

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);

	if (pThis->Deployed
		&& pTypeExt->Convert_Deploy != nullptr
		&& (pTypeExt->Deploy_Cost <= 0 || pThis->Owner->CanTransactMoney(pTypeExt->Deploy_Cost))
		&& AresFunctions::ConvertTypeTo(pThis, pTypeExt->Convert_Deploy))
	{
		if (pTypeExt->Deploy_Cost != 0)
			pThis->Owner->TransactMoney(pTypeExt->Deploy_Cost);

		pThis->Deployed = false;
	}

	const auto& pLoco = pThis->Locomotor;

	if (!pLoco)
		Game::RaiseError(-2147467261);

	if (pLoco->Is_Moving_Now())
		return 0x73E5B1;

	R->AL(pThis->Deploying);
	return 0x73DE96;
}
