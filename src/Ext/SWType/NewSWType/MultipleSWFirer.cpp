#include "MultipleSWFirer.h"

#include <Misc/PhobosGlobal.h>

const char* MultipleSWFirer::GetTypeID()
{
	return "MultipleSWFirer";
}

void MultipleSWFirer::LoadFromINI(SWTypeExt::ExtData* pData, SuperWeaponTypeClass* pSW, CCINIClass* pINI)
{
	const char* pSection = pSW->get_ID();

	INI_EX exINI(pINI);

	pData->MultipleSWFirer_RandomPick.Read(exINI, pSection, "MultipleSWFirer.RandomPick");

	for (int i = 0;; i++)
	{
		Nullable<SuperWeaponTypeClass*> swType;
		char keyName[0x30];
		sprintf_s(keyName, "FireSW%d.Type", i);
		swType.Read(exINI, pSection, keyName, true);

		if (!swType.isset())
			break;

		Valueable<int> deferment;
		sprintf_s(keyName, "FireSW%d.Deferment", i);
		deferment.Read(exINI, pSection, keyName);

		Valueable<bool> realLaunch;
		sprintf_s(keyName, "FireSW%d.RealLaunch", i);
		realLaunch.Read(exINI, pSection, keyName);

		pData->MultipleSWFirer_FireSW_Types.emplace_back(swType.Get());
		pData->MultipleSWFirer_FireSW_Deferments.emplace_back(deferment.Get());
		pData->MultipleSWFirer_RealLaunch.emplace_back(realLaunch.Get());
	}
}

bool MultipleSWFirer::Activate(SuperClass* pSW, const CellStruct& cell, bool isPlayer)
{
	SWTypeExt::ExtData* pSWTypeExt = SWTypeExt::ExtMap.Find(pSW->Type);
	HouseClass* pHouse = pSW->Owner;
	const std::vector<SuperWeaponTypeClass*>& vTypes = pSWTypeExt->MultipleSWFirer_FireSW_Types;
	const std::vector<int>& vDeferments = pSWTypeExt->MultipleSWFirer_FireSW_Deferments;

	if (vTypes.empty())
		return true;

	if (pSWTypeExt->MultipleSWFirer_RandomPick)
	{
		int idx = ScenarioClass::Instance->Random.RandomRanged(0, static_cast<int>(vTypes.size()) - 1);
		SuperWeaponTypeClass* pSWType = vTypes[idx];
		SuperClass* pSuper = pHouse->Supers.GetItem(pSWType->ArrayIndex);
		int iDeferment = vDeferments[idx];

		if (iDeferment <= 0)
		{
			if (!pSWTypeExt->MultipleSWFirer_RealLaunch[idx] || pSuper->Granted && pSuper->IsCharged && pHouse->CanTransactMoney(pSWTypeExt->Money_Amount))
			{
				if (!pSWTypeExt->HasInhibitor(pSuper->Owner, cell))
				{
					pSuper->SetReadiness(true);
					pSuper->Launch(cell, true);
					pSuper->Reset();
				}
			}
		}
		else
		{
			PhobosGlobal::Global()->MultipleSWFirer_Queued.emplace_back(cell, iDeferment, pSW, isPlayer, pSWTypeExt->MultipleSWFirer_RealLaunch[idx]);
		}
	}
	else
	{
		for (size_t i = 0; i < vTypes.size(); i++)
		{
			SuperWeaponTypeClass* pSWType = vTypes[i];
			SuperClass* pSuper = pHouse->Supers.GetItem(pSWType->ArrayIndex);
			int iDeferment = vDeferments[i];

			if (iDeferment <= 0)
			{
				if (!pSWTypeExt->MultipleSWFirer_RealLaunch[i] || pSuper->Granted && pSuper->IsCharged && pHouse->CanTransactMoney(pSWTypeExt->Money_Amount))
				{
					if (!pSWTypeExt->HasInhibitor(pHouse, cell))
					{
						pSuper->SetReadiness(true);
						pSuper->Launch(cell, true);
						pSuper->Reset();
					}
				}
			}
			else
			{
				PhobosGlobal::Global()->MultipleSWFirer_Queued.emplace_back(cell, iDeferment, pSW, isPlayer, pSWTypeExt->MultipleSWFirer_RealLaunch[i]);
			}
		}
	}

	if (isPlayer && pHouse->IsControlledByCurrentPlayer())
		Unsorted::CurrentSWType = -1;

	return true;
}
