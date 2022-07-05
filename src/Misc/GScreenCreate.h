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

public:
	static void Add(BuildingTypeClass* buildingType, HouseClass* pHouse, int duration, int reload, bool autoCreate);
	static void UpdateAll();
};