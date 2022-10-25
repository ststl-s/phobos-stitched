#include "Body.h"

#include <SuperClass.h>
#include <BuildingClass.h>
#include <HouseClass.h>
#include <ScenarioClass.h>

#include <Utilities/EnumFunctions.h>
#include <Utilities/GeneralUtils.h>
#include "Ext/House/Body.h"


void SWTypeExt::ExtData::FireNextSuperWeapon(SuperClass* pSW, HouseClass* pHouse)
{
	if (this->NextSuperWeapon.isset())
	{
		int idxNextSW = SuperWeaponTypeClass::Array->FindItemIndex(this->NextSuperWeapon.Get());
		auto pNextSW = pHouse->Supers.GetItem(idxNextSW);
		pNextSW->SetReadiness(true);
		Unsorted::CurrentSWType = idxNextSW;
	}
}
