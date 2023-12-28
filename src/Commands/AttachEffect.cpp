#include "AttachEffect.h"

#include <Utilities/GeneralUtils.h>
#include <Ext/Techno/Body.h>
#include <Ext/Network/Body.h>

const char* AttachEffectCommandClass::GetName() const
{
	return "Attach Effect";
}

const wchar_t* AttachEffectCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_ATTACH_EFFECT", L"Attach Effect");
}

const wchar_t* AttachEffectCommandClass::GetUICategory() const
{
	return CATEGORY_CONTROL;
}

const wchar_t* AttachEffectCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_ATTACH_EFFECT_DESC", L"Attach selected units effects.");
}

void AttachEffectCommandClass::Execute(WWKey eInput) const
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
						pExt->AttachEffectCommand();
					}
				}
				else
				{
					ExtraPhobosNetEvent::Handlers::RaiseAttachEffect(pTechno);
				}
			}
		}

		pNextObject = MapClass::Instance->NextObject(pNextObject);
	}
	while (pNextObject != pObjectToSelect);
}
