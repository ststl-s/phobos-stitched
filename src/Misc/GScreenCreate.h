#pragma once
#include <vector>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

#include <BuildingClass.h>
#include <HouseClass.h>

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
	static CoordStruct ScreenToCoords(Point2D client);
};
