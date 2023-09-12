#include "AutoRepair.h"

#include <MessageListClass.h>
#include <Utilities/GeneralUtils.h>

const char* AutoRepairCommandClass::GetName() const
{
	return "Auto Repair";
}

const wchar_t* AutoRepairCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_AUTO_REPAIR", L"Auto Repair");
}

const wchar_t* AutoRepairCommandClass::GetUICategory() const
{
	return CATEGORY_DEVELOPMENT;
}

const wchar_t* AutoRepairCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_AUTO_REPAIR_DESC", L"Automatically repairs damaged buildings.");
}

void AutoRepairCommandClass::Execute(WWKey eInput) const
{
	if (this->CheckDebugDeactivated())
		return;

	Phobos::AutoRepair = !Phobos::AutoRepair;
}
