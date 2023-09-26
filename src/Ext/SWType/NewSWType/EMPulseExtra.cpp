#include "EMPulseExtra.h"

#include <AircraftClass.h>
#include <BuildingClass.h>
#include <InfantryClass.h>
#include <UnitClass.h>

#include <Ext/House/Body.h>
#include <Ext/Techno/Body.h>

#include <Utilities/TemplateDef.h>


const char* EMPulseExtra::GetTypeID()
{
	return "EMPulseExtra";
}

void EMPulseExtra::LoadFromINI(SWTypeExt::ExtData* pData, SuperWeaponTypeClass* pSW, CCINIClass* pINI)
{
	INI_EX exINI(pINI);
	const char* pSection = pSW->get_ID();

	pData->EMPulse_Cannons.Read(exINI, pSection, "EMPulse.Cannons");
	pData->EMPulse_Linked.Read(exINI, pSection, "EMPulse.Linked");
	pData->EMPulse_TargetSelf.Read(exINI, pSection, "EMPulse.TargetSelf");
}

template <CanBeTechno T>
void EMPulseExtra::ProcessEMPulseCannon(const std::vector<T*>& technos, SuperClass* pSW, const CellStruct& cell)
{
	for (TechnoClass* pTechno : technos)
	{
		FireEMPulse(pTechno, pSW, cell);
	}
}

bool EMPulseExtra::Activate(SuperClass* pSW, const CellStruct& cell, bool isPlayer)
{
	HouseClass* pHouse = pSW->Owner;
	HouseExt::ExtData* pHouseExt = HouseExt::ExtMap.Find(pHouse);
	SWTypeExt::ExtData* pSWTypeExt = SWTypeExt::ExtMap.Find(pSW->Type);

	for (TechnoTypeClass* pType : pSWTypeExt->EMPulse_Cannons)
	{
		int arrayIndex = pType->GetArrayIndex();

		switch (pType->WhatAmI())
		{
		case AbstractType::AircraftType:
			ProcessEMPulseCannon<AircraftClass>(pHouseExt->OwnedAircraft[arrayIndex], pSW, cell);
			break;
		case AbstractType::BuildingType:
			ProcessEMPulseCannon(pHouseExt->OwnedBuilding[arrayIndex], pSW, cell);
			break;
		case AbstractType::InfantryType:
			ProcessEMPulseCannon(pHouseExt->OwnedInfantry[arrayIndex], pSW, cell);
			break;
		case AbstractType::UnitType:
			ProcessEMPulseCannon(pHouseExt->OwnedUnit[arrayIndex], pSW, cell);
		}
	}

	return true;
}

inline void EMPulseExtra::FireEMPulse(TechnoClass* pFirer, SuperClass* pSW, const CellStruct& cell)
{
	if (!TechnoExt::IsActivePower(pFirer))
		return;

	if (pFirer->GetCurrentMission() != Mission::Guard)
		return;

	TechnoExt::ExtData* pFirerExt = TechnoExt::ExtMap.Find(pFirer);

	if (pFirerExt->CurrentFiringSW != nullptr)
		return;

	pFirerExt->CurrentFiringSW = pSW;

	WeaponStruct* pWeapon = pFirer->GetWeapon(0);

	if (pWeapon == nullptr || pWeapon->WeaponType == nullptr)
		return;

	SWTypeExt::ExtData* pSWTypeExt = SWTypeExt::ExtMap.Find(pSW->Type);
	WeaponTypeClass* pWeaponType = pWeapon->WeaponType;

	if (pSWTypeExt->EMPulse_TargetSelf)
	{
		WeaponTypeExt::DetonateAt(pWeaponType, pFirer->GetCenterCoords(), pFirer);

		return;
	}
	
	CellClass* pCell = MapClass::Instance->GetCellAt(cell);
	int distance = pFirer->DistanceFrom(pFirer);

	if (distance < pWeaponType->MinimumRange || distance > pWeaponType->Range)
		return;

	pFirer->Fire(pCell, 0);

	pFirerExt->CurrentFiringSW = nullptr;
}
