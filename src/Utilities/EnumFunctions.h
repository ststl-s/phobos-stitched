#pragma once

#include <HouseClass.h>
#include <CellClass.h>

#include "Enum.h"

class EnumFunctions
{
public:
	static bool CanTargetHouse(AffectedHouse flags, const HouseClass* const ownerHouse, const HouseClass* const targetHouse);
	static bool IsCellEligible(CellClass* const pCell, AffectedTarget allowed, bool explicitEmptyCells = false);
	static bool IsTechnoEligible(TechnoClass* const pTechno, AffectedTarget allowed, bool considerAircraftSeparately = false);
	static bool AreCellAndObjectsEligible(CellClass* const pCell, AffectedTarget allowed, AffectedHouse allowedHouses, HouseClass* owner, bool explicitEmptyCells = false, bool considerAircraftSeparately = false);
	static BlitterFlags GetTranslucentLevel(int nInt);
	static bool IsWeaponDisabled(const TechnoClass* const pTechno, DisableWeaponCate disabled, int weaponIdx);
};
