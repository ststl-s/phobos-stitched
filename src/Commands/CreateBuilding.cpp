#include "CreateBuilding.h"

#include <Utilities/GeneralUtils.h>

#include <TacticalClass.h>
#include <SessionClass.h>

#include <Ext/Network/Body.h>
#include <Ext/House/Body.h>

#include <Misc/GScreenCreate.h>

const char* CreateBuildingCommandClass::GetName() const
{
	return "Create Building";
}

const wchar_t* CreateBuildingCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_CREATE_BUILDING", L"Create Building");
}

const wchar_t* CreateBuildingCommandClass::GetUICategory() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_DEVELOPMENT", L"Development");
}

const wchar_t* CreateBuildingCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_CREATE_BUILDING_DESC", L"Create 0x0 building in the center of current screen.");
}

void CreateBuildingCommandClass::Execute(WWKey eInput) const
{
	auto pHouse = HouseClass::CurrentPlayer.get();
	if (pHouse->Defeated || pHouse->IsObserver())
		return;

	Point2D posCenter = { DSurface::Composite->GetWidth() / 2, DSurface::Composite->GetHeight() / 2 };
	CoordStruct coord = GScreenCreate::ScreenToCoords(posCenter);

	if (SessionClass::IsSingleplayer())
	{
		if (const auto pHouseExt = HouseExt::ExtMap.Find(pHouse))
		{
			// pHouseExt->CreateBuildingAllowed = true;
			// pHouseExt->ScreenSWAllowed = true;
			pHouseExt->AutoFireCoords = coord;
			GScreenCreate::Active(pHouse, pHouseExt->AutoFireCoords);
		}
	}
	else
	{
		ExtraPhobosNetEvent::Handlers::RaiseCreateBuilding(pHouse, coord);
	}
}
