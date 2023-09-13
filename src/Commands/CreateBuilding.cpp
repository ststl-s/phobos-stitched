#include "CreateBuilding.h"

#include <Utilities/GeneralUtils.h>

const char* CreateBuildingCommandClass::GetName() const
{
	return "Create Building";
}

const wchar_t* CreateBuildingCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_CREATE_BUILDING", L"Create Building");
}

const wchar_t* CreateBuildingCommandClass::GetUICategory() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_DEVELOPMENT", L"Development");
}

const wchar_t* CreateBuildingCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_CREATE_BUILDING_DESC", L"Create 0x0 building in the center of current screen.");
}

void CreateBuildingCommandClass::Execute(WWKey eInput) const
{
	if (this->CheckDebugDeactivated())
		return;

	Phobos::CreateBuildingAllowed = true; // 允许在屏幕中心创建建筑
	Phobos::ScreenSWAllowed = true; // 允许在屏幕指定位置发射超武
}
