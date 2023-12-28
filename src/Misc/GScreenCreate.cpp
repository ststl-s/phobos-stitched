#include "GScreenCreate.h"
#include <Drawing.h>
#include <Utilities/EnumFunctions.h>
#include <TacticalClass.h>
#include <SuperClass.h>
#include <Ext/SWType/Body.h>
#include <Ext/House/Body.h>

std::vector<GScreenCreate::Item> GScreenCreate::Data;
std::vector<GScreenCreate::ItemSW> GScreenCreate::DataSW;

void GScreenCreate::Add(BuildingTypeClass* buildingType, HouseClass* pHouse, int duration, int reload, bool autoCreate)
{
	Item item {};

	item.BuildingType = buildingType;
	item.pHouse = pHouse;
	item.Duration = duration;
	item.Reload = reload;
	item.AutoCreate = autoCreate;

	item.FramesCount = 0; // 计数达到Duration则彻底禁止投建筑，默认是超武释放1500帧后禁止投建筑
	item.ReloadCount = 0; // 贤者模式下按帧自加，达到Reload之后解除，重新允许投建筑
	item.Reloading = false; // 是否处于投放一次建筑之后的贤者时间，从而禁止频繁投建筑

	Data.push_back(item);
}

void GScreenCreate::Add(int swIdx, HouseClass* pHouse, Point2D posLaunch, int duration, int reload, bool autoLaunch)
{
	ItemSW itemSW{};

	itemSW.Index = swIdx;
	itemSW.pHouse = pHouse;
	itemSW.PosLaunch = posLaunch;
	itemSW.Duration = duration;
	itemSW.Reload = reload;
	itemSW.AutoLaunch = autoLaunch;

	itemSW.FramesCount = 0;
	itemSW.ReloadCount = 0;
	itemSW.Reloading = false;

	DataSW.push_back(itemSW);
}

void GScreenCreate::UpdateAll()
{
	if (Data.empty() && DataSW.empty()) // 若每次empty时Phobos::CreateBuildingAllowed=false，可能比较吃资源
		return;

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

		if (dataItem.FramesCount == dataItem.Duration)
			Data.erase(Data.begin() + i);
	}

	// -------------------------------------------------------------------------------------------------------------

	// HousesList.clear();

	for (int k = DataSW.size() - 1; k >= 0; --k)
	{
		auto& dataItem = DataSW[k];

		dataItem.FramesCount++; // 每个超武对应的实际游戏帧数计数器无论如何都要加1

		if (dataItem.Reloading) // 正处于贤者模式，禁止砸超武
		{
			if (++dataItem.ReloadCount > dataItem.Reload) // 装弹ing
			{
				dataItem.Reloading = false; // false即封印解除，允许砸超武
				dataItem.ReloadCount = 0;
			}
		}

		if (dataItem.FramesCount == dataItem.Duration)
			DataSW.erase(DataSW.begin() + k);
	}
}

CoordStruct GScreenCreate::ScreenToCoords(Point2D client)
{
	CoordStruct location = TacticalClass::Instance->ClientToCoords(client);

	auto pCell = MapClass::Instance->TryGetCellAt(location);

	if (pCell)
		location.Z = pCell->GetCoordsWithBridge().Z;
	else
		location.Z = MapClass::Instance->GetCellFloorHeight(location);

	int layerFix = location.Z / 104 * 128;
	location.X += layerFix;
	location.Y += layerFix;

	auto pCell2 = MapClass::Instance->TryGetCellAt(location);

	if (pCell2)
		location.Z = pCell2->GetCoordsWithBridge().Z;
	else
		location.Z = MapClass::Instance->GetCellFloorHeight(location);

	CoordStruct locTemp = location;
	locTemp.Z = 0;
	Point2D posTemp;
	for (int j = 0; j < 8; j++) // 向SE方向扫描8次，以应对屏幕中心地形很高而获取到的格子地形很低
	{
		locTemp.X += 128;
		locTemp.Y += 128;

		auto pCellTemp = MapClass::Instance->TryGetCellAt(locTemp);

		if (pCellTemp)
			locTemp.Z = pCellTemp->GetCoordsWithBridge().Z;
		else
			locTemp.Z = MapClass::Instance->GetCellFloorHeight(locTemp);

		TacticalClass::Instance->CoordsToScreen(&posTemp, &locTemp);
		posTemp -= TacticalClass::Instance->TacticalPos;

		if (pow((posTemp.X - client.X), 2) + pow((posTemp.Y - client.Y), 2) < 400)
		{
			location = locTemp;
		}
	}

	return location;
}

void GScreenCreate::Active(CoordStruct coords, bool fireauto)
{
	if (Data.empty() && DataSW.empty())
		return;

	for (int i = Data.size() - 1; i >= 0; --i)
	{
		auto& dataItem = Data[i];

		if (fireauto && !dataItem.AutoCreate)
			continue;

		// 必须装填完毕，且要么自动投且按下了快捷键切换为连续开火状态而非停火状态、要么不自动投且刚按过快捷键
		if (!dataItem.Reloading)
		{
			// 获取屏幕中心点的坐标，作为预定目标
			
			CoordStruct location = coords;

			const auto cell = CellClass::Coord2Cell(location);
			auto pCell = MapClass::Instance->TryGetCellAt(cell);
			location = pCell->GetCoords();

			auto building = dataItem.BuildingType;
			auto decidedOwner = dataItem.pHouse;

			dataItem.Reloading = true;

			BuildingClass* pBuilding = abstract_cast<BuildingClass*>(building->CreateObject(decidedOwner));

			++Unsorted::IKnowWhatImDoing();
			pBuilding->Unlimbo(location, DirType::East);
			--Unsorted::IKnowWhatImDoing();
			pBuilding->Location = location;
		}
	}

	// -------------------------------------------------------------------------------------------------------------

	for (int k = DataSW.size() - 1; k >= 0; --k)
	{
		auto& dataItem = DataSW[k];

		if (fireauto && !dataItem.AutoLaunch)
			continue;

		// 必须装填完毕，且要么自动投且按下了快捷键切换为连续开火状态而非停火状态、要么不自动投且刚按过快捷键
		if (!dataItem.Reloading)
		{
			auto coords1 = GScreenCreate::ScreenToCoords(dataItem.PosLaunch);
			Point2D posCenter = { DSurface::Composite->GetWidth() / 2, DSurface::Composite->GetHeight() / 2 };
			auto coords2 = GScreenCreate::ScreenToCoords(posCenter);

			CoordStruct location = coords + (coords1 - coords2);

			if (const auto pSuper = dataItem.pHouse->Supers.GetItem(dataItem.Index))
			{
				const auto cell = CellClass::Coord2Cell(location);
				pSuper->IsCharged = true;
				pSuper->Launch(cell, true);
				pSuper->IsCharged = false;
			}

			dataItem.Reloading = true;
		}
	}
}
