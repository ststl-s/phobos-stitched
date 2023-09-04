#include "RepeatLastCombat.h"

#include <HouseClass.h>

#include <Utilities/GeneralUtils.h>

int RepeatLastCombatCommandClass::LastBuildingID;

const char* RepeatLastCombatCommandClass::GetName() const
{
	return "RepeatLastCombat";
}

const wchar_t* RepeatLastCombatCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_REPEAT_LAST_COMBAT", L"Repeat Last Combat");
}

const wchar_t* RepeatLastCombatCommandClass::GetUICategory() const
{
	return StringTable::LoadString("TXT_INTERFACE");
}

const wchar_t* RepeatLastCombatCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_REPEAT_LAST_COMBAT_DESC", L"Repeat last combat you built if it is buildable.");
}

void RepeatLastCombatCommandClass::Execute(WWKey eInput) const
{
	if (LastBuildingID != -1)
	{
		GeneralUtils::ProduceBuilding(HouseClass::CurrentPlayer, LastBuildingID);
	}
}
