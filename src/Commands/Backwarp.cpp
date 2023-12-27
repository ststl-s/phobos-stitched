#include "Backwarp.h"

#include <Ext/Techno/Body.h>
#include <Ext/Network/Body.h>

const char* BackwarpCommandClass::GetName() const
{
	return "Back Warp";
}

const wchar_t* BackwarpCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_BACK_WARP", L"Back Warp");
}

const wchar_t* BackwarpCommandClass::GetUICategory() const
{
	return CATEGORY_CONTROL;
}

const wchar_t* BackwarpCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_BACK_WARP_DESC", L"Let selected units to use back warp ability if they can.");
}

void BackwarpCommandClass::Execute(WWKey eInput) const
{
	auto pObjectToSelect = MapClass::Instance->NextObject(
		ObjectClass::CurrentObjects->Count ? ObjectClass::CurrentObjects->GetItem(0) : nullptr);

	auto pNextObject = pObjectToSelect;

	do
	{
		if (auto pTechno = abstract_cast<TechnoClass*>(pNextObject))
		{
			if (pTechno->IsSelected && pTechno->WhatAmI() != AbstractType::Building && TechnoExt::IsActive(pTechno))
			{
				if (SessionClass::Instance->IsCampaign())
				{
					if (const auto pExt = TechnoExt::ExtMap.Find(pTechno))
					{
						pExt->BackwarpActive();
					}
				}
				else
				{
					ExtraPhobosNetEvent::Handlers::RaiseBackwarp(pTechno);
				}
			}
		}

		pNextObject = MapClass::Instance->NextObject(pNextObject);
	}
	while (pNextObject != pObjectToSelect);
}
