#pragma once
#include "Commands.h"

#include <Utilities/Debug.h>
#include <Utilities/GeneralUtils.h>

// Display animation command
class CreateBuildingCommandClass : public PhobosCommandClass
{
public:
	virtual const char* GetName() const override
	{
		return "Create Building";
	}

	virtual const wchar_t* GetUIName() const override
	{
		return GeneralUtils::LoadStringUnlessMissing("TXT_CREATE_BUILDING", L"Create Building");
	}

	virtual const wchar_t* GetUICategory() const override
	{
		return GeneralUtils::LoadStringUnlessMissing("TXT_DEVELOPMENT", L"Development");
	}

	virtual const wchar_t* GetUIDescription() const override
	{
		return GeneralUtils::LoadStringUnlessMissing("TXT_CREATE_BUILDING_DESC", L"Create 0x0 building in the center of current screen.");
	}

	virtual void Execute(WWKey eInput) const override
	{
		if (this->CheckDebugDeactivated())
			return;

		Phobos::CreateBuildingAllowed = true; // 允许在屏幕中心创建建筑
	}
};