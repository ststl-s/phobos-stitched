#include "RepeatLastBuilding.h"

#include <HouseClass.h>

#include <Utilities/GeneralUtils.h>

int RepeatLastBuildingCommandClass::LastBuildingID;

const char* RepeatLastBuildingCommandClass::GetName() const
{
	return "RepeatLastBuilding";
}

const wchar_t* RepeatLastBuildingCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_REPEAT_LAST_BUILDING", L"Repeat Last Building");
}

const wchar_t* RepeatLastBuildingCommandClass::GetUICategory() const
{
	return StringTable::LoadString("TXT_INTERFACE");
}

const wchar_t* RepeatLastBuildingCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_REPEAT_LAST_BUILDING_DESC", L"Repeat last building you built if it is buildable.");
}

void RepeatLastBuildingCommandClass::Execute(WWKey eInput) const
{
	if (LastBuildingID != -1)
	{
		GeneralUtils::ProduceBuilding(HouseClass::CurrentPlayer, LastBuildingID);
	}
}
