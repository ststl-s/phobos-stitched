#include "AutoRepair.h"

#include <Utilities/GeneralUtils.h>

#include <Ext/Network/Body.h>
#include <Ext/House/Body.h>

const char* AutoRepairCommandClass::GetName() const
{
	return "Auto Repair";
}

const wchar_t* AutoRepairCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_AUTO_REPAIR", L"Auto Repair");
}

const wchar_t* AutoRepairCommandClass::GetUICategory() const
{
	return CATEGORY_DEVELOPMENT;
}

const wchar_t* AutoRepairCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_AUTO_REPAIR_DESC", L"Automatically repairs damaged buildings.");
}

void AutoRepairCommandClass::Execute(WWKey eInput) const
{
	auto pHouse = HouseClass::CurrentPlayer.get();
	if (pHouse->Defeated || pHouse->IsObserver() || !pHouse->IsControlledByHuman())
		return;

	if (SessionClass::Instance->IsSingleplayer())
	{
		if (const auto pHouseExt = HouseExt::ExtMap.Find(pHouse))
		{
			pHouseExt->AutoRepair = !pHouseExt->AutoRepair;
		}
	}
	else
	{
		ExtraPhobosNetEvent::Handlers::RaiseAutoRepair(pHouse);
	}
}
