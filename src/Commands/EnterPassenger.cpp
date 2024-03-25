#include "EnterPassenger.h"

#include <Ext/Techno/Body.h>
#include <Ext/Network/Body.h>

const char* EnterPassengerCommandClass::GetName() const
{
	return "Enter Passenger";
}

const wchar_t* EnterPassengerCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_ENTER_PASSENGER", L"EnterPassenger");
}

const wchar_t* EnterPassengerCommandClass::GetUICategory() const
{
	return CATEGORY_CONTROL;
}

const wchar_t* EnterPassengerCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_ENTER_PASSENGER_DESC", L"Let selected units to load nearby units as passenger.");
}

void EnterPassengerCommandClass::Execute(WWKey eInput) const
{
	auto pObjectToSelect = MapClass::Instance->NextObject(
		ObjectClass::CurrentObjects->Count ? ObjectClass::CurrentObjects->GetItem(0) : nullptr);

	auto pNextObject = pObjectToSelect;

	do
	{
		if (auto pTechno = abstract_cast<TechnoClass*>(pNextObject))
		{
			if (pTechno->IsSelected)
			{
				if (SessionClass::Instance->IsCampaign())
				{
					if (const auto pExt = TechnoExt::ExtMap.Find(pTechno))
					{
						pExt->EnterPassengerCommand();
					}
				}
				else
				{
					ExtraPhobosNetEvent::Handlers::RaiseEnterPassenger(pTechno);
				}
			}
		}

		pNextObject = MapClass::Instance->NextObject(pNextObject);
	}
	while (pNextObject != pObjectToSelect);
}
