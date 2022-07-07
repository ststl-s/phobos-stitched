#include "MultipleSWFirer.h"

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

		pData->FireSW_Types.emplace_back(swType.Get());
		pData->FireSW_Deferments.emplace_back(deferment.Get());
	}
}

bool MultipleSWFirer::Activate(SuperClass* pSW, const CellStruct& Coords, bool isPlayer)
{
	SWTypeExt::ExtData* pSWTypeExt = SWTypeExt::ExtMap.Find(pSW->Type);
	HouseClass* pHouse = pSW->Owner;
	const std::vector<SuperWeaponTypeClass*>& vTypes = pSWTypeExt->FireSW_Types;
	const std::vector<int>& vDeferments = pSWTypeExt->FireSW_Deferments;
	UNREFERENCED_PARAMETER(vDeferments);

	for (size_t i = 0; i < vTypes.size(); i++)
	{
		SuperWeaponTypeClass* pSWType = vTypes[i];
		SuperClass* pSuper = pHouse->Supers.GetItem(pSWType->ArrayIndex);

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

	if (isPlayer && pHouse->ControlledByPlayer())
		Unsorted::CurrentSWType = -1;

	return true;
}
