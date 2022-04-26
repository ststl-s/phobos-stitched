#include "GScreenCreate.h"
#include <Drawing.h>
#include <Utilities/EnumFunctions.h>
#include <TacticalClass.h>

std::vector<GScreenCreate::Item> GScreenCreate::Data;

void GScreenCreate::Add(BuildingTypeClass* buildingType, HouseClass* pHouse, int duration)
{
	Item item {};

	item.BuildingType = buildingType;
    item.pHouse = pHouse;
    item.Duration = duration;

    item.FramesCount = 0;

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

		if (Phobos::CreateBuildingAllowed)
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
