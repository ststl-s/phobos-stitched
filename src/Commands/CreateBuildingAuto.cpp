#include "CreateBuildingAuto.h"

#include <Utilities/GeneralUtils.h>

const char* CreateBuildingAutoCommandClass::GetName() const
{
	return "Create Building Automatically";
}

const wchar_t* CreateBuildingAutoCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_CREATE_BUILDING_AUTO", L"Create Building Automatically");
}

const wchar_t* CreateBuildingAutoCommandClass::GetUICategory() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_DEVELOPMENT", L"Development");
}

const wchar_t* CreateBuildingAutoCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_CREATE_BUILDING_AUTO_DESC", L"Create 0x0 building in the center of current screen automatically.");
}

void CreateBuildingAutoCommandClass::Execute(WWKey eInput) const
{
	if (this->CheckDebugDeactivated())
		return;

	Phobos::CreateBuildingFire = !Phobos::CreateBuildingFire; // 切换是否在屏幕中心不断自动创建建筑
	Phobos::ScreenSWFire = !Phobos::ScreenSWFire; // 切换是否在屏幕指定位置不断自动发射超武
}
