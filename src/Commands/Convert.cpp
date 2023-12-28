#include "Convert.h"

#include <Utilities/GeneralUtils.h>
#include <Ext/Techno/Body.h>
#include <Ext/Network/Body.h>

const char* ConvertCommandClass::GetName() const
{
	return "Convert";
}

const wchar_t* ConvertCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_CONVERT", L"Convert");
}

const wchar_t* ConvertCommandClass::GetUICategory() const
{
	return CATEGORY_CONTROL;
}

const wchar_t* ConvertCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_CONVERT_DESC", L"Let selected units convert if they can.");
}

void ConvertCommandClass::Execute(WWKey eInput) const
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
				if (SessionClass::IsSingleplayer())
				{
					if (const auto pExt = TechnoExt::ExtMap.Find(pTechno))
					{
						pExt->ConvertCommand();
					}
				}
				else
				{
					ExtraPhobosNetEvent::Handlers::RaiseConvert(pTechno);
				}
			}
		}

		pNextObject = MapClass::Instance->NextObject(pNextObject);
	}
	while (pNextObject != pObjectToSelect);
}
