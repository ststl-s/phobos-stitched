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

        int FramesCount;
	};

	static std::vector<Item> Data;

public:
	static void Add(BuildingTypeClass* buildingType, HouseClass* pHouse, int duration);
	static void UpdateAll();
};