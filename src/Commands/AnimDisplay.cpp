#include "AnimDisplay.h"

#include <Utilities/GeneralUtils.h>

const char* AnimDisplayCommandClass::GetName() const
{
	return "Display Animation";
}

const wchar_t* AnimDisplayCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_DISPLAY_ANIM", L"Display Animation");
}

const wchar_t* AnimDisplayCommandClass::GetUICategory() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_DEVELOPMENT", L"Development");
}

const wchar_t* AnimDisplayCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_DISPLAY_ANIM_DESC", L"Display animation attached to screen.");
}

void AnimDisplayCommandClass::Execute(WWKey eInput) const
{
	if (this->CheckDebugDeactivated())
		return;

	Phobos::Debug_DisplayAnimation = !Phobos::Debug_DisplayAnimation;
}
