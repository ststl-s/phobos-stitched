//This file initializes static constant values.

#include <YRPP.h>

void SlaveManagerClass::ZeroOutSlaves()
{
	for (const auto& pNode : this->SlaveNodes)
	{
		if (auto pSlave = pNode->Slave)
		{
			pSlave->SlaveOwner = nullptr;
		}
		pNode->Slave = nullptr;
		pNode->State = SlaveControlStatus::Dead;
		pNode->RespawnTimer.Start(this->RegenRate);
	}
}

int HouseClass::CountOwnedNow(const TechnoTypeClass* const pItem) const
{
	switch (pItem->WhatAmI())
	{
	case AbstractType::BuildingType:
		return this->CountOwnedNow(
			static_cast<BuildingTypeClass const*>(pItem));

	case AbstractType::UnitType:
		return this->CountOwnedNow(
			static_cast<UnitTypeClass const*>(pItem));

	case AbstractType::InfantryType:
		return this->CountOwnedNow(
			static_cast<InfantryTypeClass const*>(pItem));

	case AbstractType::AircraftType:
		return this->CountOwnedNow(
			static_cast<AircraftTypeClass const*>(pItem));

	default:
		__assume(0);
	}
}

int HouseClass::CountOwnedAndPresent(const TechnoTypeClass* const pItem) const
{
	switch (pItem->WhatAmI())
	{
	case AbstractType::BuildingType:
		return this->CountOwnedAndPresent(
			static_cast<BuildingTypeClass const*>(pItem));

	case AbstractType::UnitType:
		return this->CountOwnedAndPresent(
			static_cast<UnitTypeClass const*>(pItem));

	case AbstractType::InfantryType:
		return this->CountOwnedAndPresent(
			static_cast<InfantryTypeClass const*>(pItem));

	case AbstractType::AircraftType:
		return this->CountOwnedAndPresent(
			static_cast<AircraftTypeClass const*>(pItem));

	default:
		__assume(0);
	}
}

int HouseClass::CountOwnedEver(TechnoTypeClass const* const pItem) const
{
	switch (pItem->WhatAmI())
	{
	case AbstractType::BuildingType:
		return this->CountOwnedEver(
			static_cast<BuildingTypeClass const*>(pItem));

	case AbstractType::UnitType:
		return this->CountOwnedEver(
			static_cast<UnitTypeClass const*>(pItem));

	case AbstractType::InfantryType:
		return this->CountOwnedEver(
			static_cast<InfantryTypeClass const*>(pItem));

	case AbstractType::AircraftType:
		return this->CountOwnedEver(
			static_cast<AircraftTypeClass const*>(pItem));

	default:
		__assume(0);
	}
}

bool HouseClass::CanExpectToBuild(const TechnoTypeClass* const pItem) const
{
	auto const parentOwnerMask = this->Type->FindParentCountryIndex();
	return this->CanExpectToBuild(pItem, parentOwnerMask);
}

bool HouseClass::CanExpectToBuild(const TechnoTypeClass* const pItem, int const idxParent) const
{
	auto const parentOwnerMask = 1u << idxParent;
	if (pItem->InOwners(parentOwnerMask))
	{
		if (this->InRequiredHouses(pItem))
		{
			if (!this->InForbiddenHouses(pItem))
			{
				auto const BaseSide = pItem->AIBasePlanningSide;
				if (BaseSide == -1 || BaseSide == this->Type->SideIndex)
				{
					return true;
				}
			}
		}
	}
	return false;
}

int HouseClass::FindSuperWeaponIndex(SuperWeaponType const type) const
{
	for (int i = 0; i < this->Supers.Count; ++i)
	{
		if (this->Supers.Items[i]->Type->Type == type)
		{
			return i;
		}
	}
	return -1;
}

SuperClass* HouseClass::FindSuperWeapon(SuperWeaponType const type) const
{
	auto index = this->FindSuperWeaponIndex(type);
	return this->Supers.GetItemOrDefault(index);
}

bool HouseClass::IsIonCannonEligibleTarget(const TechnoClass* const pTechno) const
{
	if (pTechno->InWhichLayer() == Layer::Ground && pTechno->IsAlive && !pTechno->InLimbo)
	{
		return true;
	}

	// hard difficulty shoots the tank in the factory
	if (this->AIDifficulty == AIDifficulty::Hard)
	{
		for (const auto* pFactory : *FactoryClass::Array)
		{
			if (pFactory->Object == pTechno
				&& pFactory->Production.Rate
				&& !pFactory->IsSuspended)
			{
				return true;
			}
		}
	}

	return false;
}

int TechnoClass::GetIonCannonValue(AIDifficulty const difficulty) const
{
	const auto& rules = *RulesClass::Instance;

	const TypeList<int>* pValues = nullptr;
	int value = 1;

	if (auto pUnit = abstract_cast<const UnitClass*>(this))
	{
		auto pType = pUnit->Type;

		if (pType->Harvester)
		{
			pValues = &rules.AIIonCannonHarvesterValue;

		}
		else if (rules.BuildConst.FindItemIndex(pType->DeploysInto) != -1)
		{
			pValues = &rules.AIIonCannonMCVValue;

		}
		else if (pType->Passengers > 0)
		{
			pValues = &rules.AIIonCannonAPCValue;

		}
		else
		{
			value = 2;
		}

	}
	else if (auto pBuilding = abstract_cast<const BuildingClass*>(this))
	{
		auto pType = pBuilding->Type;

		if (pType->Factory == AbstractType::BuildingType)
		{
			pValues = &rules.AIIonCannonConYardValue;

		}
		else if (pType->Factory == AbstractType::UnitType && !pType->Naval)
		{
			pValues = &rules.AIIonCannonWarFactoryValue;

		}
		else if (pType->PowerBonus > pType->PowerDrain)
		{
			pValues = &rules.AIIonCannonPowerValue;

		}
		else if (pType->IsBaseDefense)
		{
			pValues = &rules.AIIonCannonBaseDefenseValue;

		}
		else if (pType->IsPlug)
		{
			pValues = &rules.AIIonCannonPlugValue;

		}
		else if (pType->IsTemple)
		{
			pValues = &rules.AIIonCannonTempleValue;

		}
		else if (pType->HoverPad)
		{
			pValues = &rules.AIIonCannonHelipadValue;

		}
		else if (rules.BuildConst.FindItemIndex(pType) != -1)
		{
			pValues = &rules.AIIonCannonTechCenterValue;

		}
		else
		{
			value = 4;
		}

	}
	else if (auto pInfantry = abstract_cast<const InfantryClass*>(this))
	{
		auto pType = pInfantry->Type;

		if (pType->Engineer)
		{
			pValues = &rules.AIIonCannonEngineerValue;

		}
		else if (pType->VehicleThief)
		{
			pValues = &rules.AIIonCannonThiefValue;

		}
		else
		{
			value = 2;
		}
	}

	if (pValues)
	{
		value = pValues->GetItemOrDefault(static_cast<int>(difficulty), value);
	}

	return value;
}

TechnoTypeClass* BuildingClass::GetSecretProduction() const
{
	auto const pType = this->Type;

	if (pType->SecretInfantry)
	{
		return pType->SecretInfantry;
	}
	if (pType->SecretUnit)
	{
		return pType->SecretUnit;
	}
	if (pType->SecretBuilding)
	{
		return pType->SecretBuilding;
	}
	return this->SecretProduction;
}
