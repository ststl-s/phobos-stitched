#include "HideSWBar.h"

#include <Utilities/GeneralUtils.h>

const char* HideSWBarCommandClass::GetName() const
{
	return "Hide SWBar";
}

const wchar_t* HideSWBarCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_HIDE_SWBAR", L"Hide SWBar");
}

const wchar_t* HideSWBarCommandClass::GetUICategory() const
{
	return CATEGORY_INTERFACE;
}

const wchar_t* HideSWBarCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_HIDE_SWBAR_DESC", L"Show/hide SWBar.");
}

void HideSWBarCommandClass::Execute(WWKey eInput) const
{
	Phobos::Config::HideSWBar = !Phobos::Config::HideSWBar;
}
