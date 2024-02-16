#include "SelectSW.h"

#include <Utilities/GeneralUtils.h>

#include <Ext/Network/Body.h>
#include <Ext/House/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>

const char* SelectSWCommandClass::GetName() const
{
	return "Select SuperWeapon";
}

const wchar_t* SelectSWCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_SELECT_SW", L"Select SW");
}

const wchar_t* SelectSWCommandClass::GetUICategory() const
{
	return CATEGORY_CONTROL;
}

const wchar_t* SelectSWCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_SELECT_SW_DESC", L"Select SuperWeapon for techno currently selected with hotkey.");
}

void SelectSWCommandClass::Execute(WWKey eInput) const
{
	auto pHouse = HouseClass::CurrentPlayer.get();
	if (pHouse->Defeated || pHouse->IsObserver() || !pHouse->IsControlledByHuman())
		return;

	if (SessionClass::Instance->IsSingleplayer())
	{
		if (const auto pHouseExt = HouseExt::ExtMap.Find(pHouse))
		{
			HouseExt::SetSelectSWList(pHouse);
			pHouseExt->ToSelectSW = true;
		}
	}
	else
	{
		ExtraPhobosNetEvent::Handlers::RaiseToSelectSW(pHouse);
	}
}
