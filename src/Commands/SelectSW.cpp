#include "SelectSW.h"

#include <MessageListClass.h>
#include <Utilities/GeneralUtils.h>

const char* SelectSWCommandClass::GetName() const
{
	return "Select SuperWeapon";
}

const wchar_t* SelectSWCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_SELECT_SW", L"Select SW");
}

const wchar_t* SelectSWCommandClass::GetUICategory() const
{
	return CATEGORY_DEVELOPMENT;
}

const wchar_t* SelectSWCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_SELECT_SW_DESC", L"Select SuperWeapon for techno currently selected with hotkey.");
}

void SelectSWCommandClass::Execute(WWKey eInput) const
{
	if (this->CheckDebugDeactivated())
		return;

	Phobos::ToSelectSW = true;
}
