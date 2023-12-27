#include "CreateBuildingAuto.h"

#include <Utilities/GeneralUtils.h>

#include <SessionClass.h>

#include <Ext/Network/Body.h>
#include <Ext/House/Body.h>

const char* CreateBuildingAutoCommandClass::GetName() const
{
	return "Create Building Automatically";
}

const wchar_t* CreateBuildingAutoCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_CREATE_BUILDING_AUTO", L"Create Building Automatically");
}

const wchar_t* CreateBuildingAutoCommandClass::GetUICategory() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_DEVELOPMENT", L"Development");
}

const wchar_t* CreateBuildingAutoCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_CREATE_BUILDING_AUTO_DESC", L"Create 0x0 building in the center of current screen automatically.");
}

void CreateBuildingAutoCommandClass::Execute(WWKey eInput) const
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
				// pHouseExt->CreateBuildingFire = !pHouseExt->CreateBuildingFire;
				// pHouseExt->ScreenSWFire = !pHouseExt->ScreenSWFire;
				pHouseExt->AutoFire = !pHouseExt->AutoFire;
			}
		}
		else
		{
			for (auto pTechno : *TechnoClass::Array)
			{
				if (pTechno->Owner == pHouse)
				{
					ExtraPhobosNetEvent::Handlers::RaiseCreateBuildingAuto(pTechno);
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
