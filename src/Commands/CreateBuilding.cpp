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
	auto PrintMessage = [](const wchar_t* pMessage)
		{
			MessageListClass::Instance->PrintMessage(
				pMessage,
				RulesClass::Instance->MessageDelay,
				HouseClass::CurrentPlayer->ColorSchemeIndex,
				true
			);
		};

	if (SessionClass::IsSingleplayer())
	{
		auto pHouse = HouseClass::CurrentPlayer.get();
		if (pHouse->Defeated)
			return;

		if (SessionClass::Instance->IsCampaign())
		{
			if (const auto pHouseExt = HouseExt::ExtMap.Find(pHouse))
			{
				// pHouseExt->CreateBuildingAllowed = true;
				// pHouseExt->ScreenSWAllowed = true;
				Point2D posCenter = { DSurface::Composite->GetWidth() / 2, DSurface::Composite->GetHeight() / 2 };
				pHouseExt->AutoFireCoords = GScreenCreate::ScreenToCoords(posCenter);
				GScreenCreate::Active(pHouse, pHouseExt->AutoFireCoords);
			}
		}
		else
		{
			for (auto pTechno : *TechnoClass::Array)
			{
				if (pTechno->Owner == pHouse)
				{
					ExtraPhobosNetEvent::Handlers::RaiseCreateBuilding(pTechno);
					break;
				}
			}
		}
	}
	else
	{
		PrintMessage(StringTable::LoadString("MSG:NotAvailableInMultiplayer"));
	}
}
