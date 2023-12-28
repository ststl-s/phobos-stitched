#pragma once

#include <vector>

#include <GeneralStructures.h>

class BuildingTypeClass;
class HouseClass;

class GScreenCreate
{
private:

	struct Item
	{
		BuildingTypeClass* BuildingType;
		HouseClass* pHouse;
		int Duration;
		int Reload;
		bool AutoCreate;

		int FramesCount;
		int ReloadCount;
		bool Reloading;
	};

	static std::vector<Item> Data;

	struct ItemSW
	{
		int Index;
		HouseClass* pHouse;
		Point2D PosLaunch;
		int Duration;
		int Reload;
		bool AutoLaunch;

		int FramesCount;
		int ReloadCount;
		bool Reloading;
	};

	static std::vector<ItemSW> DataSW;

public:
	static void Add(BuildingTypeClass* buildingType, HouseClass* pHouse, int duration, int reload, bool autoCreate);
	static void Add(int swIdx, HouseClass* pHouse, Point2D posLaunch, int duration, int reload, bool autoLaunch);
	static void UpdateAll();
	static void Active(CoordStruct coords, bool fireauto = false);
	static CoordStruct ScreenToCoords(Point2D client);
};
