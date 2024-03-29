#include "EnumFunctions.h"

bool EnumFunctions::CanTargetHouse(AffectedHouse flags, const HouseClass* const ownerHouse, const HouseClass* const targetHouse)
{
	if (ownerHouse == targetHouse)
		return (flags & AffectedHouse::Owner) != AffectedHouse::None;
	if (ownerHouse->IsAlliedWith(targetHouse))
		return (flags & AffectedHouse::Allies) != AffectedHouse::None;
	return (flags & AffectedHouse::Enemies) != AffectedHouse::None;
}

bool EnumFunctions::IsCellEligible(CellClass* const pCell, AffectedTarget allowed, bool explicitEmptyCells)
{
	if (explicitEmptyCells)
	{
		auto pTechno = pCell->FirstObject ? abstract_cast<TechnoClass*>(pCell->FirstObject) : nullptr;

		if (!pTechno && !(allowed & AffectedTarget::NoContent))
			return false;
	}

	if (allowed & AffectedTarget::AllCells)
	{
		if (pCell->LandType == LandType::Water) // check whether it supports water
			return (allowed & AffectedTarget::Water) != AffectedTarget::None;
		else                                    // check whether it supports non-water
			return (allowed & AffectedTarget::Land) != AffectedTarget::None;
	}

	return allowed != AffectedTarget::None ? true : false;
}

bool EnumFunctions::IsTechnoEligible(TechnoClass* const pTechno, AffectedTarget allowed, bool considerAircraftSeparately)
{
	if (allowed & AffectedTarget::AllContents)
	{
		if (pTechno)
		{
			switch (pTechno->WhatAmI())
			{
			case AbstractType::Infantry:
				return (allowed & AffectedTarget::Infantry) != AffectedTarget::None;
			case AbstractType::Unit:
				return (allowed & AffectedTarget::Unit) != AffectedTarget::None;
			case AbstractType::Aircraft:
				if (!considerAircraftSeparately)
					return (allowed & AffectedTarget::Unit) != AffectedTarget::None;
				else
					return (allowed & AffectedTarget::Aircraft) != AffectedTarget::None;
			case AbstractType::Building:
				return (allowed & AffectedTarget::Building) != AffectedTarget::None;
			}
		}
		else
		{
			// is the target cell allowed to be empty?
			return (allowed & AffectedTarget::NoContent) != AffectedTarget::None;
		}
	}

	return allowed != AffectedTarget::None ? true : false;
}

bool EnumFunctions::AreCellAndObjectsEligible(CellClass* const pCell, AffectedTarget allowed, AffectedHouse allowedHouses, HouseClass* owner, bool explicitEmptyCells, bool considerAircraftSeparately)
{
	if (!pCell)
		return false;

	auto object = pCell->FirstObject;
	bool eligible = EnumFunctions::IsCellEligible(pCell, allowed, explicitEmptyCells);

	while (true)
	{
		if (!object || !eligible)
			break;

		if (auto pTechno = abstract_cast<TechnoClass*>(object))
		{
			if (owner)
			{
				eligible = EnumFunctions::CanTargetHouse(allowedHouses, owner, pTechno->Owner);

				if (!eligible)
					break;
			}

			eligible = EnumFunctions::IsTechnoEligible(pTechno, allowed, considerAircraftSeparately);
		}

		object = object->NextObject;
	}

	return eligible;
}

BlitterFlags EnumFunctions::GetTranslucentLevel(int nInt)
{
	switch (nInt)
	{
	default:
	case 0:
		return BlitterFlags::None;
	case 25:
		return BlitterFlags::TransLucent25;
	case 50:
		return BlitterFlags::TransLucent50;
	case 75:
		return BlitterFlags::TransLucent75;
	}
}

bool EnumFunctions::IsWeaponDisabled(const TechnoClass* const pTechno, DisableWeaponCate disabled, int weaponIdx)
{
	TechnoTypeClass* pType = pTechno->GetTechnoType();

	if (!pType->Gunner && !pType->IsGattling)
	{
		if (weaponIdx == 0 && (disabled & DisableWeaponCate::Primary))
			return true;

		if (weaponIdx == 1 && (disabled & DisableWeaponCate::Secondary))
			return true;
	}
	else if (pType->Gunner)
	{
		if (disabled & DisableWeaponCate::Primary)
			return true;
	}
	else
	{
		if ((weaponIdx & 1) && (disabled & DisableWeaponCate::GattlingOdd))
			return true;

		if (!(weaponIdx & 1) && (disabled & DisableWeaponCate::GattlingEven))
			return true;
	}

	return false;
}
