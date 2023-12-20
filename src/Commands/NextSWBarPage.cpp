#include "NextSWBarPage.h"

#include <Utilities/GeneralUtils.h>

const char* NextSWBarPageCommandClass::GetName() const
{
	return "Next SWBar Page";
}

const wchar_t* NextSWBarPageCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_NEXT_SWBAR_PAGE", L"Next SWBar Page");
}

const wchar_t* NextSWBarPageCommandClass::GetUICategory() const
{
	return CATEGORY_INTERFACE;
}

const wchar_t* NextSWBarPageCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_NEXT_SWBAR_PAGE_DESC", L"Turn SWBar to the next page.");
}

void NextSWBarPageCommandClass::Execute(WWKey eInput) const
{
	Phobos::Config::SWBarPage++;
}
