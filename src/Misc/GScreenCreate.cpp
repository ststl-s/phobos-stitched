#include "GScreenCreate.h"
#include <Drawing.h>
#include <Utilities/EnumFunctions.h>
#include <TacticalClass.h>

std::vector<GScreenCreate::Item> GScreenCreate::Data;

void GScreenCreate::Add(BuildingTypeClass* buildingType, HouseClass* pHouse, int duration, int reload)
{
	Item item {};

	item.BuildingType = buildingType;
    item.pHouse = pHouse;
    item.Duration = duration;
    item.Reload = reload;

    item.FramesCount = 0; // 计数达到Duration则彻底禁止投建筑，默认是超武释放1500帧后禁止投建筑
    item.ReloadCount = 0; // 贤者模式下按帧自加，达到Reload之后解除，重新允许投建筑
    item.Reloading = false; // 是否处于投放一次建筑之后的贤者时间，从而禁止频繁投建筑

	Data.push_back(item);
}

void GScreenCreate::UpdateAll()
{
	if (Data.empty()) // 若每次empty时Phobos::CreateBuildingAllowed=false，可能比较吃资源
		return;

    bool successCreate = false;

	for (int i = Data.size() - 1; i >= 0; --i)
	{
		auto& dataItem = Data[i];

        dataItem.FramesCount++; // 每个超武对应的实际游戏帧数计数器无论如何都要加1

        if (dataItem.Reloading) // 正处于贤者模式，投不出建筑
        {
            if (++dataItem.ReloadCount > dataItem.Reload) // 装弹ing
            {
                dataItem.Reloading = false; // false即封印解除，允许投建筑
                dataItem.ReloadCount = 0;
            }
        }

        if (Phobos::CreateBuildingAllowed && !dataItem.Reloading) // 必须按过了快捷键，且必须装填完毕
		{
            // 获取屏幕中心点的坐标，作为预定目标
            Point2D posCenter = { DSurface::Composite->GetWidth() / 2, DSurface::Composite->GetHeight() / 2 };
            
            CoordStruct location = TacticalClass::Instance->ClientToCoords(posCenter);
            
            auto pCell = MapClass::Instance->TryGetCellAt(location);

            if (pCell)
                location = pCell->GetCoordsWithBridge();
            else
                location.Z = MapClass::Instance->GetCellFloorHeight(location);

            if (location.Z > 0)
            {
                location.X += 512;
                location.Y += 512;
            }

            auto building = dataItem.BuildingType;
            auto decidedOwner = dataItem.pHouse;

            BuildingClass* pBuilding = abstract_cast<BuildingClass*>(building->CreateObject(decidedOwner));

            ++Unsorted::IKnowWhatImDoing();
            successCreate = pBuilding->Unlimbo(location, Direction::E);
            --Unsorted::IKnowWhatImDoing();
            pBuilding->Location = location;

            // All of these are mandatory
            pBuilding->InLimbo = false;
            pBuilding->IsAlive = true;
            pBuilding->IsOnMap = true;
            decidedOwner->RegisterGain(pBuilding, false);
            decidedOwner->UpdatePower();
            decidedOwner->RecheckTechTree = true;
            decidedOwner->RecheckPower = true;
            decidedOwner->RecheckRadar = true;
            decidedOwner->Buildings.AddItem(pBuilding);            
            
            if (successCreate)
            {
                // Debug::Log("DEBUG: successCreate!\n");
                pBuilding->DiscoveredBy(decidedOwner);
                dataItem.Reloading = true;
            }
            else
            {
                Debug::Log("DEBUG: createFailed!\n");
            }

		}

		if (dataItem.FramesCount == dataItem.Duration)
			Data.erase(Data.begin() + i);

	}

    if (successCreate)
        Phobos::CreateBuildingAllowed = false; // 关闭开关，禁止创建建筑

}
