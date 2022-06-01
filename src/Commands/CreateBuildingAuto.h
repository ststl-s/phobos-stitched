#pragma once
#include "Commands.h"

#include <Utilities/Debug.h>
#include <Utilities/GeneralUtils.h>

// Display animation command
class CreateBuildingAutoCommandClass : public PhobosCommandClass
{
public:
	virtual const char* GetName() const override
	{
		return "Create Building Automatically";
	}

	virtual const wchar_t* GetUIName() const override
	{
		return GeneralUtils::LoadStringUnlessMissing("TXT_CREATE_BUILDING_AUTO", L"Create Building Automatically");
	}

	virtual const wchar_t* GetUICategory() const override
	{
		return GeneralUtils::LoadStringUnlessMissing("TXT_DEVELOPMENT", L"Development");
	}

	virtual const wchar_t* GetUIDescription() const override
	{
		return GeneralUtils::LoadStringUnlessMissing("TXT_CREATE_BUILDING_AUTO_DESC", L"Create 0x0 building in the center of current screen automatically.");
	}

	virtual void Execute(WWKey eInput) const override
	{
		if (this->CheckDebugDeactivated())
			return;

		Phobos::CreateBuildingFire = !Phobos::CreateBuildingFire; // 切换是否在屏幕中心不断自动创建建筑
	}
};