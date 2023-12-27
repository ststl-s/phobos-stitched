#include "SpreadAttack.h"

#include <Ext/Techno/Body.h>
#include <Ext/Network/Body.h>

const char* SpreadAttackCommandClass::GetName() const
{
	return "Spread Attack";
}

const wchar_t* SpreadAttackCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_SPREAD_ATTACK", L"Spread Attack");
}

const wchar_t* SpreadAttackCommandClass::GetUICategory() const
{
	return CATEGORY_CONTROL;
}

const wchar_t* SpreadAttackCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_SPREAD_ATTACK_DESC", L"Let selected units to attack adjecant target around their original target.");
}

void SpreadAttackCommandClass::Execute(WWKey eInput) const
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
						pExt->SpreadAttackCommand();
					}
				}
				else
				{
					ExtraPhobosNetEvent::Handlers::RaiseSpreadAttack(pTechno);
				}
			}
		}

		pNextObject = MapClass::Instance->NextObject(pNextObject);
	}
	while (pNextObject != pObjectToSelect);
}
