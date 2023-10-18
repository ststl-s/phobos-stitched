#include <UnitClass.h>

#include <Helpers/Macro.h>

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
