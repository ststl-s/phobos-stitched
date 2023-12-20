#include "LastSWBarPage.h"

#include <Utilities/GeneralUtils.h>

const char* LastSWBarPageCommandClass::GetName() const
{
	return "Last SWBar Page";
}

const wchar_t* LastSWBarPageCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_LAST_SWBAR_PAGE", L"Last SWBar Page");
}

const wchar_t* LastSWBarPageCommandClass::GetUICategory() const
{
	return CATEGORY_INTERFACE;
}

const wchar_t* LastSWBarPageCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_LAST_SWBAR_PAGE_DESC", L"Turn SWBar to the last page.");
}

void LastSWBarPageCommandClass::Execute(WWKey eInput) const
{
	Phobos::Config::SWBarPage--;

	if (Phobos::Config::SWBarPage < 1)
		Phobos::Config::SWBarPage = 1;
}
