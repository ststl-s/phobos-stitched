#include "Body.h"

#include <SuperClass.h>
#include <BuildingClass.h>
#include <HouseClass.h>
#include <ScenarioClass.h>

#include <Utilities/EnumFunctions.h>
#include <Utilities/GeneralUtils.h>
// #include "Ext/Building/Body.h"
#include "Ext/House/Body.h"

#include <New/Type/GScreenAnimTypeClass.h>
#include <Misc/GScreenDisplay.h>
#include <Misc/GScreenCreate.h>

void SWTypeExt::ExtData::FireSuperWeaponAnim(SuperClass* pSW, HouseClass* pHouse)
{
	// Debug::Log("[SWShowAnim] FireSuperWeaponAnimActivated!\n");

	if (this->CreateBuilding.Get())
	{
		// Debug::Log("[CreateBuilding] this->CreateBuilding.Get()\n");

		BuildingTypeClass* buildingType = nullptr;

		buildingType = this->CreateBuilding_Type.Get();

		if (buildingType)
		{
			// Debug::Log("[CreateBuilding] buildingType true\n");

			Phobos::CreateBuildingAllowed = false; // 关闭开关，禁止创建建筑

			GScreenCreate::Add(buildingType, pHouse, this->CreateBuilding_Duration.Get(), this->CreateBuilding_Reload, this->CreateBuilding_AutoCreate.Get());
		}
	}

	GScreenAnimTypeClass* pSWAnimType = nullptr;

	pSWAnimType = this->GScreenAnimType.Get();

	if (pSWAnimType)
	{
		// Debug::Log("[SWShowAnim] this->GScreenAnimType.Get() Successfully!\n");

		SHPStruct* ShowAnimSHP = pSWAnimType->SHP_ShowAnim;
		ConvertClass* ShowAnimPAL = pSWAnimType->PAL_ShowAnim;

		if (ShowAnimSHP == nullptr)
		{
			Debug::Log("[SWShowAnim::Error] SHP file not found!\n");
			return;
		}
		if (ShowAnimPAL == nullptr)
		{
			Debug::Log("[SWShowAnim::Error] PAL file not found!\n");
			return;
		}

		// 左上角坐标，默认将SHP文件放置到屏幕中央
		Point2D posAnim = {
			DSurface::Composite->GetWidth() / 2 - ShowAnimSHP->Width / 2,
			DSurface::Composite->GetHeight() / 2 - ShowAnimSHP->Height / 2
		};
		posAnim += pSWAnimType->ShowAnim_Offset.Get();

		// 透明度
		int translucentLevel = pSWAnimType->ShowAnim_TranslucentLevel.Get();

		// 每帧shp文件实际重复播放几帧
		int frameKeep = pSWAnimType->ShowAnim_FrameKeep;

		// shp文件循环次数
		int loopCount = pSWAnimType->ShowAnim_LoopCount;

		// 信息加入vector
		GScreenDisplay::Add(ShowAnimPAL, ShowAnimSHP, posAnim, translucentLevel, frameKeep, loopCount);

	}

}
