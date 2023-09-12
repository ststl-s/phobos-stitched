#include "KillMsgDisplay.h"

#include <MessageListClass.h>

#include <Utilities/GeneralUtils.h>

const char* KillMsgDisplayCommandClass::GetName() const
{
	return "Display Killing Message";
}

const wchar_t* KillMsgDisplayCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_DISPLAY_KILL", L"Display Kill Message");
}

const wchar_t* KillMsgDisplayCommandClass::GetUICategory() const
{
	return CATEGORY_DEVELOPMENT;
}

const wchar_t* KillMsgDisplayCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_DISPLAY_KILL_DESC", L"Display killing messages at the top left of the screen.");
}

void KillMsgDisplayCommandClass::Execute(WWKey eInput) const
{
	if (this->CheckDebugDeactivated())
		return;

	Phobos::Debug_DisplayKillMsg = !Phobos::Debug_DisplayKillMsg;
}
