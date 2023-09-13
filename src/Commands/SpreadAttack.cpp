#include "SpreadAttack.h"

#include <MessageListClass.h>
#include <Utilities/GeneralUtils.h>
#include <Utilities/Helpers.Alex.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <New/Armor/Armor.h>

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
			if (pTechno->IsSelected && pTechno->Target)
			{
				if (auto pTypeExt = TechnoTypeExt::ExtMap.Find(pTechno->GetTechnoType()))
				{
					auto range = pTypeExt->SpreadAttackRange.Get(RulesExt::Global()->SpreadAttackRange);
					if (range > 0)
					{
						ValueableVector<TechnoClass*> TargetList;
						for (auto pChangeTarget : Helpers::Alex::getCellSpreadItems(pTechno->Target->GetCoords(), range, true))
						{
							if (pChangeTarget == pTechno)
								continue;

							auto weaponidx = pTechno->SelectWeapon(pChangeTarget);
							auto weapontype = pTechno->GetWeapon(weaponidx)->WeaponType;
							auto wh = weapontype->Warhead;
							auto damage = weapontype->Damage *
								CustomArmor::GetVersus(WarheadTypeExt::ExtMap.Find(wh), TechnoExt::ExtMap.Find(pChangeTarget)->GetArmorIdx(wh));

							if (damage >= 0)
							{
								if (pChangeTarget->Owner == pTechno->Owner || pTechno->Owner->IsAlliedWith(pChangeTarget))
									continue;
							}
							else
							{
								if (pChangeTarget->Owner != pTechno->Owner && !pTechno->Owner->IsAlliedWith(pChangeTarget))
									continue;
							}

							if (pTechno->GetFireErrorWithoutRange(pChangeTarget, pTechno->SelectWeapon(pChangeTarget)) != FireError::ILLEGAL)
								TargetList.emplace_back(pChangeTarget);
						}

						if (!TargetList.empty())
						{
							auto idx = ScenarioClass::Instance->Random.RandomRanged(0, TargetList.size() - 1);
							pTechno->SetTarget(TargetList[idx]);
						}
					}
				}
			}
		}

		pNextObject = MapClass::Instance->NextObject(pNextObject);
	}
	while (pNextObject != pObjectToSelect);
}
