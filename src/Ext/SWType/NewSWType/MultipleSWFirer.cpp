#include "MultipleSWFirer.h"

#include <Utilities/PhobosGlobal.h>

const char* MultipleSWFirer::GetTypeID()
{
	return "MultipleSWFirer";
}

void MultipleSWFirer::LoadFromINI(SWTypeExt::ExtData* pData, SuperWeaponTypeClass* pSW, CCINIClass* pINI)
{
	const char* pSection = pSW->get_ID();

	if (!pINI->GetSection(pSection))
		return;

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

		pData->MultipleSWFirer_FireSW_Types.emplace_back(swType.Get());
		pData->MultipleSWFirer_FireSW_Deferments.emplace_back(deferment.Get());
	}
}

bool MultipleSWFirer::Activate(SuperClass* pSW, const CellStruct& Coords, bool isPlayer)
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
			if (pSuper->IsCharged && pHouse->CanTransactMoney(pSWTypeExt->Money_Amount))
			{
				if (!SWTypeExt::HasInhibitor(pSWTypeExt, pHouse, Coords))
				{
					pSuper->SetReadiness(true);
					pSuper->Launch(Coords, true);
					pSuper->Reset();
				}
			}
		}
		else
		{
			PhobosGlobal::Global()->MultipleSWFirer_Queued.emplace(Coords, iDeferment, pSW, isPlayer);
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
				if (pSuper->IsCharged && pHouse->CanTransactMoney(pSWTypeExt->Money_Amount))
				{
					if (!SWTypeExt::HasInhibitor(pSWTypeExt, pHouse, Coords))
					{
						pSuper->SetReadiness(true);
						pSuper->Launch(Coords, true);
						pSuper->Reset();
					}
				}
			}
			else
			{
				PhobosGlobal::Global()->MultipleSWFirer_Queued.emplace(Coords, iDeferment, pSW, isPlayer);
			}
		}
	}

	if (isPlayer && pHouse->ControlledByPlayer())
		Unsorted::CurrentSWType = -1;

	return true;
}
