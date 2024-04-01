#include "Body.h"
#include <Helpers/Macro.h>

#include <WWMouseClass.h>
#include <InfantryClass.h>
#include <UnitClass.h>

DEFINE_HOOK(0x51E451, InfantryClass_MouseOverObject_SetNewAction, 0x5)
{
	if (WWMouseClass::Instance->IsRefCountNegative() ||
		DisplayClass::Instance->RepairMode ||
		DisplayClass::Instance->SellMode ||
		DisplayClass::Instance->PowerToggleMode ||
		DisplayClass::Instance->PlanningMode ||
		DisplayClass::Instance->PlaceBeaconMode)
		return 0;

	GET(Action, pAction, EAX);
	if (pAction == Action::ToggleSelect)
		return 0;

	GET(InfantryClass*, pThis, EDI);
	GET(ObjectClass*, pObject, ESI);
	enum { SetNewAction = 0x51E458 };

	if (!TechnoExt::IsActive(pThis) ||
		!pThis->Owner ||
		!pThis->Owner->IsControlledByCurrentPlayer() ||
		pThis->Owner->IsObserver() ||
		pThis->Owner->Defeated)
		return 0;

	Action action;

	auto const pTechno = abstract_cast<TechnoClass*>(pObject);
	auto const pBuilding = abstract_cast<BuildingClass*>(pObject);

	if (pThis->Type->Engineer)
	{
		if (TechnoExt::EngineerAllowAttack(pThis, pTechno, action))
		{
			R->EAX(action);
			return SetNewAction;
		}

		if (TechnoExt::EngineerAllowEnterBuilding(pThis, pBuilding, action))
		{
			R->EAX(action);
			return SetNewAction;
		}
	}

	return 0;
}
