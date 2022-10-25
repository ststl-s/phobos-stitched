#pragma once
#include "Commands.h"

#include <Utilities/Debug.h>
#include <Utilities/GeneralUtils.h>

// Display animation command
class AnimDisplayCommandClass : public PhobosCommandClass
{
public:
	virtual const char* GetName() const override
	{
		return "Display Animation";
	}

	virtual const wchar_t* GetUIName() const override
	{
		return GeneralUtils::LoadStringUnlessMissing("TXT_DISPLAY_ANIM", L"Display Animation");
	}

	virtual const wchar_t* GetUICategory() const override
	{
		return GeneralUtils::LoadStringUnlessMissing("TXT_DEVELOPMENT", L"Development");
	}

	virtual const wchar_t* GetUIDescription() const override
	{
		return GeneralUtils::LoadStringUnlessMissing("TXT_DISPLAY_ANIM_DESC", L"Display animation attached to screen.");
	}

	virtual void Execute(WWKey eInput) const override
	{
		if (this->CheckDebugDeactivated())
			return;

		Phobos::Debug_DisplayAnimation = !Phobos::Debug_DisplayAnimation;
	}
};
