#include "GScreenCreate.h"
#include <Drawing.h>
#include <Utilities/EnumFunctions.h>
#include <TacticalClass.h>
#include <SuperClass.h>
#include <Ext/SWType/Body.h>

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

	bool successCreate = false;

	for (int i = Data.size() - 1; i >= 0; --i)
	{
		auto& dataItem = Data[i];

		dataItem.FramesCount++; // 每个超武对应的实际游戏帧数计数器无论如何都要加1

		if (dataItem.Reloading) // 正处于贤者模式，投不出建筑
		{
			Phobos::CreateBuildingAllowed = false; // 禁止投建筑，防止时间一到就擅自投，副作用是一个超武投建筑在CD则其他所有超武都无法投建筑
			if (++dataItem.ReloadCount > dataItem.Reload) // 装弹ing
			{
				dataItem.Reloading = false; // false即封印解除，允许投建筑
				dataItem.ReloadCount = 0;
			}
		}

		// 必须装填完毕，且要么自动投且按下了快捷键切换为连续开火状态而非停火状态、要么不自动投且刚按过快捷键
		if (!dataItem.Reloading && ((dataItem.AutoCreate && Phobos::CreateBuildingFire) || Phobos::CreateBuildingAllowed))
		{
			// 获取屏幕中心点的坐标，作为预定目标
			Point2D posCenter = { DSurface::Composite->GetWidth() / 2, DSurface::Composite->GetHeight() / 2 };

			CoordStruct location = GScreenCreate::ScreenToCoords(posCenter);

			auto building = dataItem.BuildingType;
			auto decidedOwner = dataItem.pHouse;

			BuildingClass* pBuilding = abstract_cast<BuildingClass*>(building->CreateObject(decidedOwner));

			++Unsorted::IKnowWhatImDoing();
			successCreate = pBuilding->Unlimbo(location, DirType::East);
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

	// -------------------------------------------------------------------------------------------------------------

	for (int k = DataSW.size() - 1; k >= 0; --k)
	{
		auto& dataItem = DataSW[k];

		dataItem.FramesCount++; // 每个超武对应的实际游戏帧数计数器无论如何都要加1

		if (dataItem.Reloading) // 正处于贤者模式，投不出建筑
		{
			Phobos::ScreenSWAllowed = false; // 禁止砸超武，防止时间一到就擅自砸，副作用是一个超武砸超武在CD则其他所有超武都无法砸超武
			if (++dataItem.ReloadCount > dataItem.Reload) // 装弹ing
			{
				dataItem.Reloading = false; // false即封印解除，允许砸超武
				dataItem.ReloadCount = 0;
			}
		}

		// 必须装填完毕，且要么自动投且按下了快捷键切换为连续开火状态而非停火状态、要么不自动投且刚按过快捷键
		if (!dataItem.Reloading && ((dataItem.AutoLaunch && Phobos::ScreenSWFire) || Phobos::ScreenSWAllowed))
		{
			CoordStruct location = GScreenCreate::ScreenToCoords(dataItem.PosLaunch);

			if (const auto pSuper = dataItem.pHouse->Supers.GetItem(dataItem.Index))
			{
				const auto cell = CellClass::Coord2Cell(location);
				pSuper->IsCharged = true;
				pSuper->Launch(cell, true);
				pSuper->IsCharged = false;
			}
			dataItem.Reloading = true;
		}
		if (dataItem.FramesCount == dataItem.Duration)
			DataSW.erase(DataSW.begin() + k);
	}
	Phobos::ScreenSWAllowed = false; // 关闭开关，禁止砸超武

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
