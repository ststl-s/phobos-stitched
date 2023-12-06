#include "Body.h"

#include <SuperClass.h>
#include <BuildingClass.h>
#include <HouseClass.h>
#include <ScenarioClass.h>

#include <Utilities/EnumFunctions.h>
#include <Utilities/GeneralUtils.h>
#include "Ext/Building/Body.h"
#include "Ext/House/Body.h"
#include "Ext/WarheadType/Body.h"
#include "Ext/WeaponType/Body.h"

inline void LimboCreate(BuildingTypeClass* pType, HouseClass* pOwner, int ID)
{
	auto pOwnerExt = HouseExt::ExtMap.Find(pOwner);

	// BuildLimit check goes before creation
	if (pType->BuildLimit > 0)
	{
		int sum = pOwner->CountOwnedNow(pType);

		// copy Ares' deployable units x build limit fix
		if (auto const pUndeploy = pType->UndeploysInto)
			sum += pOwner->CountOwnedNow(pUndeploy);

		if (sum >= pType->BuildLimit)
			return;
	}

	if (auto const pBuilding = static_cast<BuildingClass*>(pType->CreateObject(pOwner)))
	{
		// All of these are mandatory
		pBuilding->InLimbo = false;
		pBuilding->IsAlive = true;
		pBuilding->IsOnMap = true;
		pOwner->RegisterGain(pBuilding, false);
		pOwner->UpdatePower();
		pOwner->RecheckTechTree = true;
		pOwner->RecheckPower = true;
		pOwner->RecheckRadar = true;
		pOwner->Buildings.AddItem(pBuilding);

		// increment limbo build count
		if (pOwnerExt)
			pOwnerExt->OwnedLimboBuildingTypes.Increment(pType->ArrayIndex);

		// Different types of building logics
		if (pType->ConstructionYard)
			pOwner->ConYards.AddItem(pBuilding); // why would you do that????

		if (pType->SecretLab)
			pOwner->SecretLabs.AddItem(pBuilding);

		if (pType->FactoryPlant)
		{
			pOwner->FactoryPlants.AddItem(pBuilding);
			pOwner->CalculateCostMultipliers();
		}

		if (pType->OrePurifier)
			pOwner->NumOrePurifiers++;

		// BuildingClass::Place is where Ares hooks secret lab expansion
		// pTechnoBuilding->Place(false);
		// even with it no bueno yet, plus new issues
		// probably should just port it from Ares 0.A and be done

		// LimboKill init
		auto const pBuildingExt = BuildingExt::ExtMap.Find(pBuilding);
		if (pBuildingExt && ID != -1)
			pBuildingExt->LimboID = ID;
	}
}

inline void LimboDelete(BuildingClass* pBuilding, HouseClass* pTargetHouse)
{
	BuildingTypeClass* pType = pBuilding->Type;

	// Mandatory
	pBuilding->InLimbo = true;
	pBuilding->IsAlive = false;
	pBuilding->IsOnMap = false;
	pTargetHouse->RegisterLoss(pBuilding, false);
	pTargetHouse->UpdatePower();
	pTargetHouse->RecheckTechTree = true;
	pTargetHouse->RecheckPower = true;
	pTargetHouse->RecheckRadar = true;
	pTargetHouse->Buildings.Remove(pBuilding);

	// Building logics
	if (pType->ConstructionYard)
		pTargetHouse->ConYards.Remove(pBuilding);

	if (pType->SecretLab)
		pTargetHouse->SecretLabs.Remove(pBuilding);

	if (pType->FactoryPlant)
	{
		pTargetHouse->FactoryPlants.Remove(pBuilding);
		pTargetHouse->CalculateCostMultipliers();
	}

	if (pType->OrePurifier)
		pTargetHouse->NumOrePurifiers--;

	// Remove completely
	pBuilding->UnInit();
}

void SWTypeExt::FireSuperWeaponExt(SuperClass* pSW, const CellStruct& cell)
{
	if (auto const pTypeExt = SWTypeExt::ExtMap.Find(pSW->Type))
	{
		if (pTypeExt->LimboDelivery_Types.size() > 0)
			pTypeExt->ApplyLimboDelivery(pSW->Owner);

		if (pTypeExt->LimboKill_IDs.size() > 0)
			pTypeExt->ApplyLimboKill(pSW->Owner);

		if (pTypeExt->Detonate_Warhead.isset() || pTypeExt->Detonate_Weapon.isset())
			pTypeExt->ApplyDetonation(pSW->Owner, cell);

		if (!pTypeExt->Convert_Pairs.empty())
			pTypeExt->ApplyTypeConversion(pSW);

		if (pTypeExt->SW_Next.size() > 0)
			pTypeExt->ApplySWNext(pSW, cell);

		pTypeExt->FireSuperWeaponAnim(pSW, pSW->Owner);
		pTypeExt->FireNextSuperWeapon(pSW, pSW->Owner);

		if (pTypeExt->SW_Cumulative)
			HouseExt::SuperWeaponCumulativeReset(pSW->Owner, pSW);

		if (pTypeExt->SW_Shots > 0)
		{
			const auto pHouseExt = HouseExt::ExtMap.Find(pSW->Owner);
			++pHouseExt->SW_Shots[pSW->Type->ArrayIndex];
		}
	}
}

// Universal handler of the rolls-weights system
std::vector<int> SWTypeExt::ExtData::WeightedRollsHandler(ValueableVector<float>* rolls, ValueableVector<ValueableVector<int>>* weights, size_t size)
{
	bool rollOnce = false;
	size_t rollsSize = rolls->size();
	size_t weightsSize = weights->size();
	int index;
	std::vector<int> indices;

	// if no RollChances are supplied, do only one roll
	if (rollsSize == 0)
	{
		rollsSize = 1;
		rollOnce = true;
	}

	for (size_t i = 0; i < rollsSize; i++)
	{
		this->RandomBuffer = ScenarioClass::Instance->Random.RandomDouble();
		if (!rollOnce && this->RandomBuffer > (*rolls)[i])
			continue;

		// If there are more rolls than weight lists, use the last weight list
		size_t j = std::min(weightsSize - 1, i);
		index = GeneralUtils::ChooseOneWeighted(this->RandomBuffer, &(*weights)[j]);

		// If modder provides more weights than there are objects and we hit one of these, ignore it
		// otherwise add
		if (size_t(index) < size)
			indices.push_back(index);
	}
	return indices;
}

// SW.Next proper launching mechanic
inline void LaunchTheSW(HouseClass* pHouse, SWTypeExt::ExtData* pLauncherTypeExt, SuperWeaponTypeClass* pLaunchedType, const CellStruct& cell)
{
	const auto pSuper = pHouse->Supers.GetItem(SuperWeaponTypeClass::Array->FindItemIndex(pLaunchedType));

	if (!pSuper)
		return;

	const auto pSuperTypeExt = SWTypeExt::ExtMap.Find(pLaunchedType);
	if (!pLauncherTypeExt->SW_Next_RealLaunch || (pSuperTypeExt && pSuper->IsCharged && pHouse->CanTransactMoney(pSuperTypeExt->Money_Amount)))
	{

		if (pLauncherTypeExt->SW_Next_IgnoreInhibitors || !pSuperTypeExt->HasInhibitor(pHouse, cell)
			&& (pLauncherTypeExt->SW_Next_IgnoreDesignators || pSuperTypeExt->HasDesignator(pHouse, cell)))
		{
			pSuper->SetReadiness(true);
			pSuper->Launch(cell, true);
			pSuper->Reset();
		}

	}
}

void SWTypeExt::ExtData::ApplyLimboDelivery(HouseClass* pHouse)
{
	// random mode
	if (this->LimboDelivery_RandomWeightsData.size())
	{
		int id = -1;
		size_t idsSize = this->LimboDelivery_IDs.size();
		auto results = this->WeightedRollsHandler(&this->LimboDelivery_RollChances, &this->LimboDelivery_RandomWeightsData, this->LimboDelivery_Types.size());
		for (size_t result : results)
		{
			if (result < idsSize)
				id = this->LimboDelivery_IDs[result];

			LimboCreate(this->LimboDelivery_Types[result], pHouse, id);
		}
	}
	// no randomness mode
	else
	{
		int id = -1;
		size_t idsSize = this->LimboDelivery_IDs.size();
		for (size_t i = 0; i < this->LimboDelivery_Types.size(); i++)
		{
			if (i < idsSize)
				id = this->LimboDelivery_IDs[i];

			LimboCreate(this->LimboDelivery_Types[i], pHouse, id);
		}
	}
}

void SWTypeExt::ExtData::ApplyLimboKill(HouseClass* pHouse)
{
	for (int limboKillID : this->LimboKill_IDs)
	{
		for (HouseClass* pTargetHouse : *HouseClass::Array)
		{
			if (EnumFunctions::CanTargetHouse(this->LimboKill_Affected, pHouse, pTargetHouse))
			{
				for (const auto pBuilding : pTargetHouse->Buildings)
				{
					const auto pBuildingExt = BuildingExt::ExtMap.Find(pBuilding);
					if (pBuildingExt->LimboID == limboKillID)
						LimboDelete(pBuilding, pTargetHouse);
				}
			}
		}
	}
}

void SWTypeExt::ExtData::ApplyDetonation(HouseClass* pHouse, const CellStruct& cell)
{
	const auto coords = MapClass::Instance->GetCellAt(cell)->GetCoords();
	BuildingClass* pFirer = nullptr;

	for (auto const& pBld : pHouse->Buildings)
	{
		if (this->IsLaunchSiteEligible(cell, pBld, false))
		{
			pFirer = pBld;
			break;
		}
	}

	const auto pWeapon = this->Detonate_Weapon.isset() ? this->Detonate_Weapon.Get() : nullptr;

	if (pWeapon)
		WeaponTypeExt::DetonateAt(pWeapon, coords, pFirer, this->Detonate_Damage.Get(pWeapon->Damage));
	else
		WarheadTypeExt::DetonateAt(this->Detonate_Warhead.Get(), coords, pFirer, this->Detonate_Damage.Get(0));
}

void SWTypeExt::ExtData::ApplySWNext(SuperClass* pSW, const CellStruct& cell)
{
	// random mode
	if (this->SW_Next_RandomWeightsData.size())
	{
		auto results = this->WeightedRollsHandler(&this->SW_Next_RollChances, &this->SW_Next_RandomWeightsData, this->SW_Next.size());
		for (int result : results)
			LaunchTheSW(pSW->Owner, this, this->SW_Next[result], cell);
	}
	// no randomness mode
	else
	{
		for (const auto pSWType : this->SW_Next)
			LaunchTheSW(pSW->Owner, this, pSWType, cell);
	}
}

void SWTypeExt::ExtData::ApplyTypeConversion(SuperClass* pSW)
{
	if (this->Convert_Pairs.size() == 0)
		return;

	for (const auto pTargetFoot : *FootClass::Array)
		TypeConvertHelper::Convert(pTargetFoot, this->Convert_Pairs, pSW->Owner);
}

// =============================
// Ares 0.A helpers
// Inhibitors check
bool SWTypeExt::ExtData::IsInhibitor(HouseClass* pOwner, TechnoClass* pTechno) const
{
	if (pTechno->IsAlive && pTechno->Health && !pTechno->InLimbo && !pTechno->Deactivated)
	{
		if (!pOwner->IsAlliedWith(pTechno))
		{
			if (auto pBld = abstract_cast<BuildingClass*>(pTechno))
			{
				if (!pBld->IsPowerOnline())
					return false;
			}

			return this->SW_AnyInhibitor || this->SW_Inhibitors.Contains(pTechno->GetTechnoType());
		}
	}
	return false;
}

bool SWTypeExt::ExtData::IsInhibitorEligible(HouseClass* pOwner, const CellStruct& coords, TechnoClass* pTechno) const
{
	if (this->IsInhibitor(pOwner, pTechno))
	{
		const auto pType = pTechno->GetTechnoType();
		const auto pExt = TechnoTypeExt::ExtMap.Find(pType);

		// get the inhibitor's center
		auto center = pTechno->GetCenterCoords();

		// has to be closer than the inhibitor range (which defaults to Sight)
		return coords.DistanceFrom(CellClass::Coord2Cell(center)) <= pExt->InhibitorRange.Get(pType->Sight);
	}

	return false;
}

bool SWTypeExt::ExtData::HasInhibitor(HouseClass* pOwner, const CellStruct& coords) const
{
	// does not allow inhibitors
	if (this->SW_Inhibitors.empty() && !this->SW_AnyInhibitor)
		return false;

	// a single inhibitor in range suffices
	return std::any_of(TechnoClass::Array->begin(), TechnoClass::Array->end(), [=, &coords](TechnoClass* pTechno)
		{ return this->IsInhibitorEligible(pOwner, coords, pTechno); }
	);
}

// Designators check
bool SWTypeExt::ExtData::IsDesignator(HouseClass* pOwner, TechnoClass* pTechno) const
{
	if (pTechno->Owner == pOwner && pTechno->IsAlive && pTechno->Health && !pTechno->InLimbo && !pTechno->Deactivated)
		return this->SW_AnyDesignator || this->SW_Designators.Contains(pTechno->GetTechnoType());

	return false;
}

bool SWTypeExt::ExtData::IsDesignatorEligible(HouseClass* pOwner, const CellStruct& coords, TechnoClass* pTechno) const
{
	if (this->IsDesignator(pOwner, pTechno))
	{
		const auto pType = pTechno->GetTechnoType();
		const auto pExt = TechnoTypeExt::ExtMap.Find(pType);

		// get the designator's center
		auto center = pTechno->GetCenterCoords();

		// has to be closer than the designator range (which defaults to Sight)
		return coords.DistanceFrom(CellClass::Coord2Cell(center)) <= pExt->DesignatorRange.Get(pType->Sight);
	}

	return false;
}

bool SWTypeExt::ExtData::HasDesignator(HouseClass* pOwner, const CellStruct& coords) const
{
	// does not require designators
	if (this->SW_Designators.empty() && !this->SW_AnyDesignator)
		return true;

	// a single designator in range suffices
	return std::any_of(TechnoClass::Array->begin(), TechnoClass::Array->end(), [=, &coords](TechnoClass* pTechno)
		{ return this->IsDesignatorEligible(pOwner, coords, pTechno); });
}

bool SWTypeExt::ExtData::IsLaunchSiteEligible(const CellStruct& Coords, BuildingClass* pBuilding, bool ignoreRange) const
{
	if (!this->IsLaunchSite(pBuilding))
		return false;

	if (ignoreRange)
		return true;

	// get the range for this building
	auto range = this->GetLaunchSiteRange(pBuilding);
	const auto& minRange = range.first;
	const auto& maxRange = range.second;

	CoordStruct coords = pBuilding->GetCenterCoords();
	const auto center = CellClass::Coord2Cell(coords);
	const auto distance = Coords.DistanceFrom(center);

	// negative range values just pass the test
	return (minRange < 0.0 || distance >= minRange)
		&& (maxRange < 0.0 || distance <= maxRange);
}

bool SWTypeExt::ExtData::IsLaunchSite(BuildingClass* pBuilding) const
{
	if (pBuilding->IsAlive && pBuilding->Health && !pBuilding->InLimbo && pBuilding->IsPowerOnline())
	{
		auto const pExt = BuildingExt::ExtMap.Find(pBuilding);
		return pExt->HasSuperWeapon(this->OwnerObject()->ArrayIndex, true);
	}

	return false;
}

std::pair<double, double> SWTypeExt::ExtData::GetLaunchSiteRange(BuildingClass* pBuilding) const
{
	return std::make_pair(this->SW_RangeMinimum.Get(), this->SW_RangeMaximum.Get());
}

bool SWTypeExt::ExtData::IsAvailable(HouseClass* pHouse) const
{
	const auto pThis = this->OwnerObject();
	const int arrayIndex = pThis->ArrayIndex;

	// check whether the optional aux building exists
	if (pThis->AuxBuilding && pHouse->CountOwnedAndPresent(pThis->AuxBuilding) <= 0)
		return false;

	// allow only certain houses, disallow forbidden houses
	const auto OwnerBits = 1u << pHouse->Type->ArrayIndex;

	if (!(this->SW_RequiredHouses & OwnerBits) || (this->SW_ForbiddenHouses & OwnerBits))
		return false;

	// check that any aux building exist and no neg building
	auto IsPresent = [pHouse](const TechnoTypeClass* pType)
	{
		return pHouse->CountOwnedAndPresent(pType) > 0;
	};

	const auto& auxBuildings = this->SW_AuxBuildings;

	if (!auxBuildings.empty()
		&& std::none_of(auxBuildings.cbegin(), auxBuildings.cend(), IsPresent))
		return false;

	const auto& negBuildings = this->SW_NegBuildings;

	if (std::any_of(negBuildings.cbegin(), negBuildings.cend(), IsPresent))
		return false;

	const auto& auxTechnos = this->SW_AuxTechnos;

	if (this->SW_AuxTechnos_Any)
	{
		if (!auxTechnos.empty()
			&& std::none_of(auxTechnos.cbegin(), auxTechnos.cend(), IsPresent))
			return false;
	}
	else
	{
		if (!auxTechnos.empty()
			&& !std::all_of(auxTechnos.cbegin(), auxTechnos.cend(), IsPresent))
			return false;
	}

	const auto& negTechnos = this->SW_NegTechnos;

	if (this->SW_NegTechnos_Any)
	{
		if (std::any_of(negTechnos.cbegin(), negTechnos.cend(), IsPresent))
			return false;
	}
	else
	{
		if (std::all_of(negTechnos.cbegin(), negTechnos.cend(), IsPresent))
			return false;
	}

	// check counts
	if (this->SW_Shots >= 0)
	{
		const HouseExt::ExtData* pHouseExt = HouseExt::ExtMap.Find(pHouse);
		if (pHouseExt->SW_Shots.contains(arrayIndex)
			&& pHouseExt->SW_Shots.at(arrayIndex) >= this->SW_Shots)
			return false;
	}

	return true;
}

void SWTypeExt::ExtData::PrintMessage(const CSFText& message, HouseClass* pFirer)
{
	if (message.empty())
		return;

	int color = ColorScheme::FindIndex("Gold");
	if (this->Message_FirerColor)
	{
		// firer color
		if (pFirer)
		{
			color = pFirer->ColorSchemeIndex;
		}
	}
	else
	{
		if (this->Message_ColorScheme > -1)
		{
			// user defined color
			color = this->Message_ColorScheme;
		}
		else if (const auto pCurrent = HouseClass::CurrentPlayer())
		{
			// default way: the current player's color
			color = pCurrent->ColorSchemeIndex;
		}
	}

	// print the message
	MessageListClass::Instance->PrintMessage(message, RulesClass::Instance->MessageDelay, color);
}
