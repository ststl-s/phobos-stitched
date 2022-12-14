#include "Body.h"

#include <Ext/TechnoType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/SWType/Body.h>

#include <ScenarioClass.h>
#include <SuperClass.h>
#include <TechnoTypeClass.h>

//Static init

template<> const DWORD Extension<HouseClass>::Canary = 0x11111111;
HouseExt::ExtContainer HouseExt::ExtMap;


int HouseExt::ActiveHarvesterCount(HouseClass* pThis)
{
	int result = 0;
	for (auto pTechno : *TechnoClass::Array)
	{
		if (pTechno->Owner == pThis)
		{
			auto pTypeExt = TechnoTypeExt::ExtMap.Find(pTechno->GetTechnoType());
			result += pTypeExt->Harvester_Counted && TechnoExt::IsHarvesting(pTechno);
		}
	}
	return result;
}

int HouseExt::TotalHarvesterCount(HouseClass* pThis)
{
	int result = 0;

	for (auto pType : RulesExt::Global()->HarvesterTypes)
		result += pThis->CountOwnedAndPresent(pType);

	return result;
}

int HouseExt::CountOwnedLimbo(HouseClass* pThis, BuildingTypeClass const* const pItem)
{
	auto pHouseExt = HouseExt::ExtMap.Find(pThis);
	return pHouseExt->OwnedLimboBuildingTypes.GetItemCount(pItem->ArrayIndex);
}

// Ares
HouseClass* HouseExt::GetHouseKind(OwnerHouseKind const kind, bool const allowRandom, HouseClass* const pDefault, HouseClass* const pInvoker, HouseClass* const pVictim)
{
	switch (kind)
	{
	case OwnerHouseKind::Invoker:
	case OwnerHouseKind::Killer:
		return pInvoker ? pInvoker : pDefault;
	case OwnerHouseKind::Victim:
		return pVictim ? pVictim : pDefault;
	case OwnerHouseKind::Civilian:
		return HouseClass::FindCivilianSide();
	case OwnerHouseKind::Special:
		return HouseClass::FindSpecial();
	case OwnerHouseKind::Neutral:
		return HouseClass::FindNeutral();
	case OwnerHouseKind::Random:
		if (allowRandom)
		{
			auto& Random = ScenarioClass::Instance->Random;
			return HouseClass::Array->GetItem(
				Random.RandomRanged(0, HouseClass::Array->Count - 1));
		}
		else
		{
			return pDefault;
		}
	case OwnerHouseKind::Default:
	default:
		return pDefault;
	}
}

void HouseExt::ExtData::LoadFromINIFile(CCINIClass* const pINI)
{
	const char* pSection = this->OwnerObject()->PlainName;

	INI_EX exINI(pINI);

	ValueableVector<bool> readBaseNodeRepairInfo;
	readBaseNodeRepairInfo.Read(exINI, pSection, "RepairBaseNodes");
	size_t nWritten = readBaseNodeRepairInfo.size();
	if (nWritten > 0)
	{
		for (size_t i = 0; i < 3; i++)
			this->RepairBaseNodes[i] = readBaseNodeRepairInfo[i < nWritten ? i : nWritten - 1];
	}

}

void HouseExt::ForceOnlyTargetHouseEnemy(HouseClass* pThis, int mode = -1)
{
	if (!pThis)
		return;

	auto pHouseExt = HouseExt::ExtMap.Find(pThis);

	if (!pHouseExt)
		return;

	if (mode < 0 || mode > 2)
		mode = -1;

	enum { ForceFalse = 0, ForceTrue = 1, ForceRandom = 2, UseDefault = -1 };

	pHouseExt->ForceOnlyTargetHouseEnemyMode = mode;

	switch (mode)
	{
	case ForceFalse:
		pHouseExt->ForceOnlyTargetHouseEnemy = false;
		break;

	case ForceTrue:
		pHouseExt->ForceOnlyTargetHouseEnemy = true;
		break;

	case ForceRandom:
		pHouseExt->ForceOnlyTargetHouseEnemy = (bool)ScenarioClass::Instance->Random.RandomRanged(0, 1);;
		break;

	default:
		pHouseExt->ForceOnlyTargetHouseEnemy = false;
		break;
	}
}

void HouseExt::GrantScoreSuperPower(HouseClass* pThis, int SWIDX)
{
	SuperClass* pSuper = pThis->Supers[SWIDX];
	bool NotObserver = !pThis->IsObserver() || !pThis->IsCurrentPlayerObserver();
	bool granted;
	granted = pSuper->Grant(true, NotObserver, false);
	if (granted && NotObserver && pThis == HouseClass::CurrentPlayer)
	{
		if (MouseClass::Instance->AddCameo(AbstractType::Special, SWIDX))
		{
			MouseClass::Instance->RepaintSidebar(1);
		}
	}
}

int HouseExt::GetHouseIndex(int param, TeamClass* pTeam = nullptr, TActionClass* pTAction = nullptr)
{
	if ((pTeam && pTAction) || (param == 8997 && !pTeam && !pTAction))
		return -1;

	int houseIdx = -1;
	std::vector<int> housesListIdx;

	// Transtale the Multiplayer index into a valid index for the HouseClass array
	if (param >= HouseClass::PlayerAtA && param <= HouseClass::PlayerAtH)
	{
		switch (param)
		{
		case HouseClass::PlayerAtA:
			houseIdx = 0;
			break;

		case HouseClass::PlayerAtB:
			houseIdx = 1;
			break;

		case HouseClass::PlayerAtC:
			houseIdx = 2;
			break;

		case HouseClass::PlayerAtD:
			houseIdx = 3;
			break;

		case HouseClass::PlayerAtE:
			houseIdx = 4;
			break;

		case HouseClass::PlayerAtF:
			houseIdx = 5;
			break;

		case HouseClass::PlayerAtG:
			houseIdx = 6;
			break;

		case HouseClass::PlayerAtH:
			houseIdx = 7;
			break;

		default:
			break;
		}

		if (houseIdx >= 0)
		{
			HouseClass* pHouse = HouseClass::Array->GetItem(houseIdx);

			if (!pHouse->Defeated
				&& !pHouse->IsObserver()
				&& !pHouse->Type->MultiplayPassive)
			{
				return houseIdx;
			}
		}

		return -1;
	}

	// Special case that returns the house index of the TeamClass object or the Trigger Action
	if (param == 8997)
	{
		return (pTeam ? pTeam->Owner->ArrayIndex : pTAction->TeamType->Owner->ArrayIndex);
	}

	// Positive index values check. Includes any kind of House
	if (param >= 0)
	{
		if (param < HouseClass::Array->Count)
		{
			HouseClass* pHouse = HouseClass::Array->GetItem(param);

			if (!pHouse->Defeated
				&& !pHouse->IsObserver())
			{
				return houseIdx;
			}
		}

		return -1;
	}

	// Special cases
	switch (param)
	{
	case -1:
		// Random non-neutral
		for (auto pHouse : *HouseClass::Array)
		{
			if (!pHouse->Defeated
				&& !pHouse->IsObserver()
				&& !pHouse->Type->MultiplayPassive)
			{
				housesListIdx.push_back(pHouse->ArrayIndex);
			}
		}

		if (housesListIdx.size() > 0)
			houseIdx = housesListIdx.at(ScenarioClass::Instance->Random.RandomRanged(0, housesListIdx.size() - 1));
		else
			return -1;

		break;

	case -2:
		// Find first Neutral house
		for (auto pHouseNeutral : *HouseClass::Array)
		{
			if (pHouseNeutral->IsNeutral())
			{
				houseIdx = pHouseNeutral->ArrayIndex;
				break;
			}
		}

		break;

	case -3:
		// Random Human Player
		for (auto pHouse : *HouseClass::Array)
		{
			if (pHouse->IsControlledByHuman()
				&& !pHouse->Defeated
				&& !pHouse->IsObserver())
			{
				housesListIdx.push_back(pHouse->ArrayIndex);
			}
		}

		if (housesListIdx.size() > 0)
			houseIdx = housesListIdx.at(ScenarioClass::Instance->Random.RandomRanged(0, housesListIdx.size() - 1));
		else
			return -1;

		break;

	default:
		break;
	}

	return houseIdx;
}

int HouseExt::CountOwnedIncludeDeploy(const HouseClass* pThis, const TechnoTypeClass* pItem)
{
	return pThis->CountOwnedNow(pItem) +
		(pItem->DeploysInto == nullptr ? 0 : pThis->CountOwnedNow(pItem->DeploysInto)) +
		(pItem->UndeploysInto == nullptr ? 0 : pThis->CountOwnedNow(pItem->UndeploysInto));
}

CanBuildResult HouseExt::BuildLimitGroupCheck(const HouseClass* pThis, const TechnoTypeClass* pItem, bool buildLimitOnly, bool includeQueued)
{
	auto pItemExt = TechnoTypeExt::ExtMap.Find(pItem);

	if (pItemExt->BuildLimit_Group_Types.empty())
		return CanBuildResult::Buildable;

	if (pItemExt->BuildLimit_Group_Any.Get())
	{
		bool reachedLimit = false;
		for (size_t i = 0;
			i < std::min(
				pItemExt->BuildLimit_Group_Types.size(),
				pItemExt->BuildLimit_Group_Limits.size())
			; i++)
		{
			TechnoTypeClass* pType = pItemExt->BuildLimit_Group_Types[i];
			int ownedNow = CountOwnedIncludeDeploy(pThis, pType);
			if (ownedNow >= pItemExt->BuildLimit_Group_Limits[i])
			{
				reachedLimit |= (includeQueued && FactoryClass::FindByOwnerAndProduct(pThis, pType))
					? false : true;
			}
		}
		return reachedLimit ? CanBuildResult::TemporarilyUnbuildable : CanBuildResult::Buildable;
	}
	else
	{
		if (pItemExt->BuildLimit_Group_Limits.size() == 1U)
		{
			int sum = 0;
			bool reachedLimit = false;
			for (auto& pType : pItemExt->BuildLimit_Group_Types)
			{
				sum += CountOwnedIncludeDeploy(pThis, pType);
			}
			if (sum >= pItemExt->BuildLimit_Group_Limits[0])
			{
				for (auto& pType : pItemExt->BuildLimit_Group_Types)
				{
					reachedLimit |= (includeQueued && FactoryClass::FindByOwnerAndProduct(pThis, pType))
						? false : true;
				}
			}
			return reachedLimit ? CanBuildResult::TemporarilyUnbuildable : CanBuildResult::Buildable;
		}
		else
		{
			for (size_t i = 0;
			i < std::min(
				pItemExt->BuildLimit_Group_Types.size(),
				pItemExt->BuildLimit_Group_Limits.size())
			; i++)
			{
				TechnoTypeClass* pType = pItemExt->BuildLimit_Group_Types[i];
				int ownedNow = CountOwnedIncludeDeploy(pThis, pType);
				if (ownedNow < pItemExt->BuildLimit_Group_Limits[i]
				|| includeQueued && FactoryClass::FindByOwnerAndProduct(pThis, pType))
					return CanBuildResult::Buildable;
			}
			return CanBuildResult::TemporarilyUnbuildable;
		}
	}
}

HouseExt::FactoryState HouseExt::HasFactory_Ares(const HouseClass* pThis, const TechnoTypeClass* pItem, bool requirePower)
{
	auto pExt = TechnoTypeExt::ExtMap.Find(pItem);
	auto bitsOwners = pItem->GetOwners();
	auto isNaval = pItem->Naval;
	auto thisAbsType = pItem->WhatAmI();

	auto ret = FactoryState::NoFactory;

	for (auto const& pBld : pThis->Buildings)
	{
		if (pBld->InLimbo
			|| pBld->GetCurrentMission() == Mission::Selling
			|| pBld->QueuedMission == Mission::Selling)
		{
			continue;
		}

		auto const pType = pBld->Type;

		if (pType->Factory != thisAbsType
			|| (thisAbsType == AbstractType::UnitType && pType->Naval != isNaval)
			|| !pExt->CanBeBuiltAt_Ares(pType)
			|| !pType->InOwners(bitsOwners))
		{
			continue;
		}

		if (!requirePower || pBld->HasPower)
			return FactoryState::Available;

		ret = FactoryState::Unpowered;
	}

	return ret;
}

void HouseExt::RegisterGain(HouseClass* pThis, TechnoClass* pTechno)
{
	int idxType = pTechno->GetTechnoType()->GetArrayIndex();
	ExtData* pExt = ExtMap.Find(pThis);

	switch (pTechno->WhatAmI())
	{
	case AbstractType::Aircraft:
	{
		auto& vOwned = pExt->OwnedAircraft[idxType];
		auto it = std::find(vOwned.begin(), vOwned.end(), pTechno);

		if (it == vOwned.end())
			pExt->OwnedAircraft[idxType].emplace_back(static_cast<AircraftClass*>(pTechno));
	}break;
	case AbstractType::Building:
	{
		auto& vOwned = pExt->OwnedBuilding[idxType];
		auto it = std::find(vOwned.begin(), vOwned.end(), pTechno);

		if (it == vOwned.end())
			pExt->OwnedBuilding[idxType].emplace_back(static_cast<BuildingClass*>(pTechno));
	}break;
	case AbstractType::InfantryType:
	{
		auto& vOwned = pExt->OwnedInfantry[idxType];
		auto it = std::find(vOwned.begin(), vOwned.end(), pTechno);

		if (it == vOwned.end())
			pExt->OwnedInfantry[idxType].emplace_back(static_cast<InfantryClass*>(pTechno));
	}break;
	case AbstractType::Unit:
	{
		auto& vOwned = pExt->OwnedUnit[idxType];
		auto it = std::find(vOwned.begin(), vOwned.end(), pTechno);

		if (it == vOwned.end())
			pExt->OwnedUnit[idxType].emplace_back(static_cast<UnitClass*>(pTechno));
	}break;
	default:
		break;
	}
}

void HouseExt::RegisterLoss(HouseClass* pThis, TechnoClass* pTechno)
{
	int idxType = pTechno->GetTechnoType()->GetArrayIndex();
	ExtData* pExt = ExtMap.Find(pThis);

	switch (pTechno->WhatAmI())
	{
	case AbstractType::Aircraft:
	{
		auto& vOwned = pExt->OwnedAircraft[idxType];
		auto it = std::find(vOwned.begin(), vOwned.end(), pTechno);

		if (it != vOwned.end())
			pExt->OwnedAircraft[idxType].erase(it);
	}break;
	case AbstractType::Building:
	{
		auto& vOwned = pExt->OwnedBuilding[idxType];
		auto it = std::find(vOwned.begin(), vOwned.end(), pTechno);

		if (it != vOwned.end())
			pExt->OwnedBuilding[idxType].erase(it);
	}break;
	case AbstractType::Infantry:
	{
		auto& vOwned = pExt->OwnedInfantry[idxType];
		auto it = std::find(vOwned.begin(), vOwned.end(), pTechno);

		if (it != vOwned.end())
			pExt->OwnedInfantry[idxType].erase(it);
	}break;
	case AbstractType::Unit:
	{
		auto& vOwned = pExt->OwnedUnit[idxType];
		auto it = std::find(vOwned.begin(), vOwned.end(), pTechno);

		if (it != vOwned.end())
			pExt->OwnedUnit[idxType].erase(it);
	}break;
	default:
		break;
	}
}

const std::vector<TechnoClass*>& HouseExt::GetOwnedTechno(HouseClass* pThis, TechnoTypeClass* pType)
{
	ExtData* pExt = ExtMap.Find(pThis);
	int arrayIdx = pType->GetArrayIndex();

	switch (pType->WhatAmI())
	{
	case AbstractType::InfantryType:
		return reinterpret_cast<const std::vector<TechnoClass*>&>(pExt->OwnedInfantry[arrayIdx]);
	case AbstractType::UnitType:
		return reinterpret_cast<const std::vector<TechnoClass*>&>(pExt->OwnedUnit[arrayIdx]);
	case AbstractType::AircraftType:
		return reinterpret_cast<const std::vector<TechnoClass*>&>(pExt->OwnedAircraft[arrayIdx]);
	default:
		return reinterpret_cast<const std::vector<TechnoClass*>&>(pExt->OwnedBuilding[arrayIdx]);
	}
}

void HouseExt::TechnoDeactivate(HouseClass* pThis)
{
	ExtData* pExt = ExtMap.Find(pThis);
	for (auto pTechno : *TechnoClass::Array)
	{
		if (pTechno->Owner == pThis)
		{
			auto const pBuilding = abstract_cast<BuildingClass*>(pTechno);
			switch (pTechno->WhatAmI())
			{
			case AbstractType::Infantry:
				for (size_t i = 0; i < pExt->DeactivateInfantry_Duration.size(); i++)
				{
					if (pExt->DeactivateInfantry_Duration[i] > 0)
					{
						bool deactivate = false;
						if (!pExt->DeactivateInfantry_Types[i].empty())
						{
							for (TechnoTypeClass* pType : pExt->DeactivateInfantry_Types[i])
							{
								if (pTechno->GetTechnoType() == pType)
								{
									deactivate = true;
									break;
								}
							}
						}
						else
							deactivate = true;

						if (!pExt->DeactivateInfantry_Ignore[i].empty())
						{
							for (TechnoTypeClass* pType : pExt->DeactivateInfantry_Ignore[i])
							{
								if (pTechno->GetTechnoType() == pType)
								{
									deactivate = false;
									break;
								}
							}
						}

						if (deactivate)
						{
							if (!pTechno->Deactivated)
								pTechno->Deactivate();
						}
					}
					else
					{
						bool reactivate = false;
						if (!pExt->DeactivateInfantry_Types[i].empty())
						{
							for (TechnoTypeClass* pType : pExt->DeactivateInfantry_Types[i])
							{
								if (pTechno->GetTechnoType() == pType)
								{
									reactivate = true;
									break;
								}
							}
						}
						else
							reactivate = true;

						if (!pExt->DeactivateInfantry_Ignore[i].empty())
						{
							for (TechnoTypeClass* pType : pExt->DeactivateInfantry_Ignore[i])
							{
								if (pTechno->GetTechnoType() == pType)
								{
									reactivate = false;
									break;
								}
							}
						}

						if (reactivate)
						{
							if (pTechno->Deactivated)
								pTechno->Reactivate();
						}
					}
				}
				break;
			case AbstractType::Unit:
				if (pTechno->GetTechnoType()->Organic)
				{
					for (size_t i = 0; i < pExt->DeactivateInfantry_Duration.size(); i++)
					{
						if (pExt->DeactivateInfantry_Duration[i] > 0)
						{
							bool deactivate = false;
							if (!pExt->DeactivateInfantry_Types[i].empty())
							{
								for (TechnoTypeClass* pType : pExt->DeactivateInfantry_Types[i])
								{
									if (pTechno->GetTechnoType() == pType)
									{
										deactivate = true;
										break;
									}
								}
							}
							else
								deactivate = true;

							if (!pExt->DeactivateInfantry_Ignore[i].empty())
							{
								for (TechnoTypeClass* pType : pExt->DeactivateInfantry_Ignore[i])
								{
									if (pTechno->GetTechnoType() == pType)
									{
										deactivate = false;
										break;
									}
								}
							}

							if (deactivate)
							{
								if (!pTechno->Deactivated)
									pTechno->Deactivate();
							}
						}
						else
						{
							bool reactivate = false;
							if (!pExt->DeactivateInfantry_Types[i].empty())
							{
								for (TechnoTypeClass* pType : pExt->DeactivateInfantry_Types[i])
								{
									if (pTechno->GetTechnoType() == pType)
									{
										reactivate = true;
										break;
									}
								}
							}
							else
								reactivate = true;

							if (!pExt->DeactivateInfantry_Ignore[i].empty())
							{
								for (TechnoTypeClass* pType : pExt->DeactivateInfantry_Ignore[i])
								{
									if (pTechno->GetTechnoType() == pType)
									{
										reactivate = false;
										break;
									}
								}
							}

							if (reactivate)
							{
								if (pTechno->Deactivated)
									pTechno->Reactivate();
							}
						}
					}
				}
				else if (pTechno->GetTechnoType()->ConsideredAircraft)
				{
					for (size_t i = 0; i < pExt->DeactivateAircraft_Duration.size(); i++)
					{
						if (pExt->DeactivateAircraft_Duration[i] > 0)
						{
							bool deactivate = false;
							if (!pExt->DeactivateAircraft_Types[i].empty())
							{
								for (TechnoTypeClass* pType : pExt->DeactivateAircraft_Types[i])
								{
									if (pTechno->GetTechnoType() == pType)
									{
										deactivate = true;
										break;
									}
								}
							}
							else
								deactivate = true;

							if (!pExt->DeactivateAircraft_Ignore[i].empty())
							{
								for (TechnoTypeClass* pType : pExt->DeactivateAircraft_Ignore[i])
								{
									if (pTechno->GetTechnoType() == pType)
									{
										deactivate = false;
										break;
									}
								}
							}

							if (deactivate)
							{
								if (!pTechno->Deactivated)
									pTechno->Deactivate();
							}
						}
						else
						{
							bool reactivate = false;
							if (!pExt->DeactivateAircraft_Types[i].empty())
							{
								for (TechnoTypeClass* pType : pExt->DeactivateAircraft_Types[i])
								{
									if (pTechno->GetTechnoType() == pType)
									{
										reactivate = true;
										break;
									}
								}
							}
							else
								reactivate = true;

							if (!pExt->DeactivateAircraft_Ignore[i].empty())
							{
								for (TechnoTypeClass* pType : pExt->DeactivateAircraft_Ignore[i])
								{
									if (pTechno->GetTechnoType() == pType)
									{
										reactivate = false;
										break;
									}
								}
							}

							if (reactivate)
							{
								if (pTechno->Deactivated)
									pTechno->Reactivate();
							}
						}
					}
				}
				else if (pTechno->GetTechnoType()->Naval)
				{
					for (size_t i = 0; i < pExt->DeactivateNaval_Duration.size(); i++)
					{
						if (pExt->DeactivateNaval_Duration[i] > 0)
						{
							bool deactivate = false;
							if (!pExt->DeactivateNaval_Types[i].empty())
							{
								for (TechnoTypeClass* pType : pExt->DeactivateNaval_Types[i])
								{
									if (pTechno->GetTechnoType() == pType)
									{
										deactivate = true;
										break;
									}
								}
							}
							else
								deactivate = true;

							if (!pExt->DeactivateNaval_Ignore[i].empty())
							{
								for (TechnoTypeClass* pType : pExt->DeactivateNaval_Ignore[i])
								{
									if (pTechno->GetTechnoType() == pType)
									{
										deactivate = false;
										break;
									}
								}
							}

							if (deactivate)
							{
								if (!pTechno->Deactivated)
									pTechno->Deactivate();
							}
						}
						else
						{
							bool reactivate = false;
							if (!pExt->DeactivateNaval_Types[i].empty())
							{
								for (TechnoTypeClass* pType : pExt->DeactivateNaval_Types[i])
								{
									if (pTechno->GetTechnoType() == pType)
									{
										reactivate = true;
										break;
									}
								}
							}
							else
								reactivate = true;

							if (!pExt->DeactivateNaval_Ignore[i].empty())
							{
								for (TechnoTypeClass* pType : pExt->DeactivateNaval_Ignore[i])
								{
									if (pTechno->GetTechnoType() == pType)
									{
										reactivate = false;
										break;
									}
								}
							}

							if (reactivate)
							{
								if (pTechno->Deactivated)
									pTechno->Reactivate();
							}
						}
					}
				}
				else
				{
					for (size_t i = 0; i < pExt->DeactivateVehicle_Duration.size(); i++)
					{
						if (pExt->DeactivateVehicle_Duration[i] > 0)
						{
							bool deactivate = false;
							if (!pExt->DeactivateVehicle_Types[i].empty())
							{
								for (TechnoTypeClass* pType : pExt->DeactivateVehicle_Types[i])
								{
									if (pTechno->GetTechnoType() == pType)
									{
										deactivate = true;
										break;
									}
								}
							}
							else
								deactivate = true;

							if (!pExt->DeactivateVehicle_Ignore[i].empty())
							{
								for (TechnoTypeClass* pType : pExt->DeactivateVehicle_Ignore[i])
								{
									if (pTechno->GetTechnoType() == pType)
									{
										deactivate = false;
										break;
									}
								}
							}

							if (deactivate)
							{
								if (!pTechno->Deactivated)
									pTechno->Deactivate();
							}
						}
						else
						{
							bool reactivate = false;
							if (!pExt->DeactivateVehicle_Types[i].empty())
							{
								for (TechnoTypeClass* pType : pExt->DeactivateVehicle_Types[i])
								{
									if (pTechno->GetTechnoType() == pType)
									{
										reactivate = true;
										break;
									}
								}
							}
							else
								reactivate = true;

							if (!pExt->DeactivateVehicle_Ignore[i].empty())
							{
								for (TechnoTypeClass* pType : pExt->DeactivateVehicle_Ignore[i])
								{
									if (pTechno->GetTechnoType() == pType)
									{
										reactivate = false;
										break;
									}
								}
							}

							if (reactivate)
							{
								if (pTechno->Deactivated)
									pTechno->Reactivate();
							}
						}
					}
				}
				break;
			case AbstractType::Aircraft:
				for (size_t i = 0; i < pExt->DeactivateAircraft_Duration.size(); i++)
				{
					if (pExt->DeactivateAircraft_Duration[i] > 0)
					{
						bool deactivate = false;
						if (!pExt->DeactivateAircraft_Types[i].empty())
						{
							for (TechnoTypeClass* pType : pExt->DeactivateAircraft_Types[i])
							{
								if (pTechno->GetTechnoType() == pType)
								{
									deactivate = true;
									break;
								}
							}
						}
						else
							deactivate = true;

						if (!pExt->DeactivateAircraft_Ignore[i].empty())
						{
							for (TechnoTypeClass* pType : pExt->DeactivateAircraft_Ignore[i])
							{
								if (pTechno->GetTechnoType() == pType)
								{
									deactivate = false;
									break;
								}
							}
						}

						if (deactivate)
						{
							if (!pTechno->Deactivated)
								pTechno->Deactivate();
						}
					}
					else
					{
						bool reactivate = false;
						if (!pExt->DeactivateAircraft_Types[i].empty())
						{
							for (TechnoTypeClass* pType : pExt->DeactivateAircraft_Types[i])
							{
								if (pTechno->GetTechnoType() == pType)
								{
									reactivate = true;
									break;
								}
							}
						}
						else
							reactivate = true;

						if (!pExt->DeactivateAircraft_Ignore[i].empty())
						{
							for (TechnoTypeClass* pType : pExt->DeactivateAircraft_Ignore[i])
							{
								if (pTechno->GetTechnoType() == pType)
								{
									reactivate = false;
									break;
								}
							}
						}

						if (reactivate)
						{
							if (pTechno->Deactivated)
								pTechno->Reactivate();
						}
					}
				}
				break;
			case AbstractType::Building:
				if (pBuilding->Type->BuildCat == BuildCat::Combat)
				{
					for (size_t i = 0; i < pExt->DeactivateDefense_Duration.size(); i++)
					{
						if (pExt->DeactivateDefense_Duration[i] > 0)
						{
							bool deactivate = false;
							if (!pExt->DeactivateDefense_Types[i].empty())
							{
								for (TechnoTypeClass* pType : pExt->DeactivateDefense_Types[i])
								{
									if (pTechno->GetTechnoType() == pType)
									{
										deactivate = true;
										break;
									}
								}
							}
							else
								deactivate = true;

							if (!pExt->DeactivateDefense_Ignore[i].empty())
							{
								for (TechnoTypeClass* pType : pExt->DeactivateDefense_Ignore[i])
								{
									if (pTechno->GetTechnoType() == pType)
									{
										deactivate = false;
										break;
									}
								}
							}

							if (deactivate)
							{
								if (pBuilding->IsPowerOnline())
									pBuilding->GoOffline();
							}
						}
						else
						{
							bool reactivate = false;
							if (!pExt->DeactivateDefense_Types[i].empty())
							{
								for (TechnoTypeClass* pType : pExt->DeactivateDefense_Types[i])
								{
									if (pTechno->GetTechnoType() == pType)
									{
										reactivate = true;
										break;
									}
								}
							}
							else
								reactivate = true;

							if (!pExt->DeactivateDefense_Ignore[i].empty())
							{
								for (TechnoTypeClass* pType : pExt->DeactivateDefense_Ignore[i])
								{
									if (pTechno->GetTechnoType() == pType)
									{
										reactivate = false;
										break;
									}
								}
							}

							if (reactivate)
							{
								if (!pBuilding->IsPowerOnline())
									pBuilding->GoOnline();
							}
						}
					}
				}
				else
				{
					for (size_t i = 0; i < pExt->DeactivateBuilding_Duration.size(); i++)
					{
						if (pExt->DeactivateBuilding_Duration[i] > 0)
						{
							bool deactivate = false;
							if (!pExt->DeactivateBuilding_Types[i].empty())
							{
								for (TechnoTypeClass* pType : pExt->DeactivateBuilding_Types[i])
								{
									if (pTechno->GetTechnoType() == pType)
									{
										deactivate = true;
										break;
									}
								}
							}
							else
								deactivate = true;

							if (!pExt->DeactivateBuilding_Ignore[i].empty())
							{
								for (TechnoTypeClass* pType : pExt->DeactivateBuilding_Ignore[i])
								{
									if (pTechno->GetTechnoType() == pType)
									{
										deactivate = false;
										break;
									}
								}
							}

							if (deactivate)
							{
								if (pBuilding->IsPowerOnline())
									pBuilding->GoOffline();
							}
						}
						else
						{
							bool reactivate = false;
							if (!pExt->DeactivateBuilding_Types[i].empty())
							{
								for (TechnoTypeClass* pType : pExt->DeactivateBuilding_Types[i])
								{
									if (pTechno->GetTechnoType() == pType)
									{
										reactivate = true;
										break;
									}
								}
							}
							else
								reactivate = true;

							if (!pExt->DeactivateBuilding_Ignore[i].empty())
							{
								for (TechnoTypeClass* pType : pExt->DeactivateBuilding_Ignore[i])
								{
									if (pTechno->GetTechnoType() == pType)
									{
										reactivate = false;
										break;
									}
								}
							}

							if (reactivate)
							{
								if (!pBuilding->IsPowerOnline())
									pBuilding->GoOnline();
							}
						}
					}
				}
				break;
			default:
				break;
			}
		}
	}

	std::vector<int> Temp_Duration;
	std::vector<ValueableVector<TechnoTypeClass*>> Temp_Types;
	std::vector<ValueableVector<TechnoTypeClass*>> Temp_Ignore;

	for (size_t i = 0; i < pExt->DeactivateInfantry_Duration.size(); i++)
	{
		if (pExt->DeactivateInfantry_Duration[i] > 0)
		{
			pExt->DeactivateInfantry_Duration[i]--;
			Temp_Duration.emplace_back(pExt->DeactivateInfantry_Duration[i]);
			Temp_Types.emplace_back(pExt->DeactivateInfantry_Types[i]);
			Temp_Ignore.emplace_back(pExt->DeactivateInfantry_Ignore[i]);

		}
	}
	pExt->DeactivateInfantry_Duration = Temp_Duration;
	Temp_Duration.clear();
	pExt->DeactivateInfantry_Types = Temp_Types;
	Temp_Types.clear();
	pExt->DeactivateInfantry_Ignore = Temp_Ignore;
	Temp_Ignore.clear();

	for (size_t i = 0; i < pExt->DeactivateVehicle_Duration.size(); i++)
	{
		if (pExt->DeactivateVehicle_Duration[i] > 0)
		{
			pExt->DeactivateVehicle_Duration[i]--;
			Temp_Duration.emplace_back(pExt->DeactivateVehicle_Duration[i]);
			Temp_Types.emplace_back(pExt->DeactivateVehicle_Types[i]);
			Temp_Ignore.emplace_back(pExt->DeactivateVehicle_Ignore[i]);

		}
	}
	pExt->DeactivateVehicle_Duration = Temp_Duration;
	Temp_Duration.clear();
	pExt->DeactivateVehicle_Types = Temp_Types;
	Temp_Types.clear();
	pExt->DeactivateVehicle_Ignore = Temp_Ignore;
	Temp_Ignore.clear();

	for (size_t i = 0; i < pExt->DeactivateNaval_Duration.size(); i++)
	{
		if (pExt->DeactivateNaval_Duration[i] > 0)
		{
			pExt->DeactivateNaval_Duration[i]--;
			Temp_Duration.emplace_back(pExt->DeactivateNaval_Duration[i]);
			Temp_Types.emplace_back(pExt->DeactivateNaval_Types[i]);
			Temp_Ignore.emplace_back(pExt->DeactivateNaval_Ignore[i]);

		}
	}
	pExt->DeactivateNaval_Duration = Temp_Duration;
	Temp_Duration.clear();
	pExt->DeactivateNaval_Types = Temp_Types;
	Temp_Types.clear();
	pExt->DeactivateNaval_Ignore = Temp_Ignore;
	Temp_Ignore.clear();

	for (size_t i = 0; i < pExt->DeactivateAircraft_Duration.size(); i++)
	{
		if (pExt->DeactivateAircraft_Duration[i] > 0)
		{
			pExt->DeactivateAircraft_Duration[i]--;
			Temp_Duration.emplace_back(pExt->DeactivateAircraft_Duration[i]);
			Temp_Types.emplace_back(pExt->DeactivateAircraft_Types[i]);
			Temp_Ignore.emplace_back(pExt->DeactivateAircraft_Ignore[i]);

		}
	}
	pExt->DeactivateAircraft_Duration = Temp_Duration;
	Temp_Duration.clear();
	pExt->DeactivateAircraft_Types = Temp_Types;
	Temp_Types.clear();
	pExt->DeactivateAircraft_Ignore = Temp_Ignore;
	Temp_Ignore.clear();

	for (size_t i = 0; i < pExt->DeactivateBuilding_Duration.size(); i++)
	{
		if (pExt->DeactivateBuilding_Duration[i] > 0)
		{
			pExt->DeactivateBuilding_Duration[i]--;
			Temp_Duration.emplace_back(pExt->DeactivateBuilding_Duration[i]);
			Temp_Types.emplace_back(pExt->DeactivateBuilding_Types[i]);
			Temp_Ignore.emplace_back(pExt->DeactivateBuilding_Ignore[i]);

		}
	}
	pExt->DeactivateBuilding_Duration = Temp_Duration;
	Temp_Duration.clear();
	pExt->DeactivateBuilding_Types = Temp_Types;
	Temp_Types.clear();
	pExt->DeactivateBuilding_Ignore = Temp_Ignore;
	Temp_Ignore.clear();

	for (size_t i = 0; i < pExt->DeactivateDefense_Duration.size(); i++)
	{
		if (pExt->DeactivateDefense_Duration[i] > 0)
		{
			pExt->DeactivateDefense_Duration[i]--;
			Temp_Duration.emplace_back(pExt->DeactivateDefense_Duration[i]);
			Temp_Types.emplace_back(pExt->DeactivateDefense_Types[i]);
			Temp_Ignore.emplace_back(pExt->DeactivateDefense_Ignore[i]);

		}
	}
	pExt->DeactivateDefense_Duration = Temp_Duration;
	Temp_Duration.clear();
	pExt->DeactivateDefense_Types = Temp_Types;
	Temp_Types.clear();
	pExt->DeactivateDefense_Ignore = Temp_Ignore;
	Temp_Ignore.clear();
}

void HouseExt::TechnoVeterancyInit(HouseClass* pThis)
{
	ExtData* pExt = ExtMap.Find(pThis);
	if (!pExt->VeterancyInit)
	{
		for (auto pType : *TechnoTypeClass::Array)
		{
			if (pType->Trainable)
			{
				switch (pType->WhatAmI())
				{
				case AbstractType::AircraftType:
					pExt->AircraftVeterancyTypes.emplace_back(pType);
					pExt->AircraftVeterancy.emplace_back(0.0);
					break;
				case AbstractType::BuildingType:
					pExt->BuildingVeterancyTypes.emplace_back(pType);
					pExt->BuildingVeterancy.emplace_back(0.0);
					break;
				case AbstractType::InfantryType:
					pExt->InfantryVeterancyTypes.emplace_back(pType);
					pExt->InfantryVeterancy.emplace_back(0.0);
				case AbstractType::UnitType:
					if (pType->Organic)
					{
						pExt->InfantryVeterancyTypes.emplace_back(pType);
						pExt->InfantryVeterancy.emplace_back(0.0);
					}
					else if (pType->ConsideredAircraft)
					{
						pExt->AircraftVeterancyTypes.emplace_back(pType);
						pExt->AircraftVeterancy.emplace_back(0.0);
					}
					else if (pType->Naval)
					{
						pExt->NavalVeterancyTypes.emplace_back(pType);
						pExt->NavalVeterancy.emplace_back(0.0);
					}
					else
					{
						pExt->VehicleVeterancyTypes.emplace_back(pType);
						pExt->VehicleVeterancy.emplace_back(0.0);
					}
					break;
				default:
					break;
				}
			}
		}
		pExt->VeterancyInit = true;
	}
}

void HouseExt::TechnoUpgrade(HouseClass* pThis, double veterancy, ValueableVector<TechnoTypeClass*> types, ValueableVector<TechnoTypeClass*> ignore, AbstractType whatamI, bool naval, bool cumulative)
{
	ExtData* pExt = ExtMap.Find(pThis);
	switch (whatamI)
	{
	case AbstractType::AircraftType:
		for (auto pType : *AircraftTypeClass::Array)
		{
			if (pType->Trainable)
			{
				for (size_t i = 0; i < pExt->AircraftVeterancyTypes.size(); i++)
				{
					if (pType == pExt->AircraftVeterancyTypes[i])
					{
						bool UpgradeAllow = false;
						if (!types.empty())
						{
							for (TechnoTypeClass* pAffectType : types)
							{
								if (pType == pAffectType)
								{
									UpgradeAllow = true;
									break;
								}
							}
						}
						else
							UpgradeAllow = true;

						if (!ignore.empty())
						{
							for (TechnoTypeClass* pAffectType : ignore)
							{
								if (pType == pAffectType)
								{
									UpgradeAllow = false;
									break;
								}
							}
						}

						if (UpgradeAllow)
						{
							if (cumulative)
								pExt->AircraftVeterancy[i] += veterancy;
							else
							{
								if (veterancy == 0)
									pExt->AircraftVeterancy[i] = 0;
								else
								{
									if (pExt->AircraftVeterancy[i] * veterancy < 0)
										pExt->AircraftVeterancy[i] = veterancy;
									else
									{
										if (abs(pExt->AircraftVeterancy[i]) < abs(veterancy))
											pExt->AircraftVeterancy[i] = veterancy;
									}
								}
							}
						}
					}
				}
			}
		}
		for (auto pType : *UnitTypeClass::Array)
		{
			if (pType->Trainable && pType->ConsideredAircraft && !pType->Organic)
			{
				for (size_t i = 0; i < pExt->AircraftVeterancyTypes.size(); i++)
				{
					if (pType == pExt->AircraftVeterancyTypes[i])
					{
						bool UpgradeAllow = false;
						if (!types.empty())
						{
							for (TechnoTypeClass* pAffectType : types)
							{
								if (pType == pAffectType)
								{
									UpgradeAllow = true;
									break;
								}
							}
						}
						else
							UpgradeAllow = true;

						if (!ignore.empty())
						{
							for (TechnoTypeClass* pAffectType : ignore)
							{
								if (pType == pAffectType)
								{
									UpgradeAllow = false;
									break;
								}
							}
						}

						if (UpgradeAllow)
						{
							if (cumulative)
								pExt->AircraftVeterancy[i] += veterancy;
							else
							{
								if (veterancy == 0)
									pExt->AircraftVeterancy[i] = 0;
								else
								{
									if (pExt->AircraftVeterancy[i] * veterancy < 0)
										pExt->AircraftVeterancy[i] = veterancy;
									else
									{
										if (abs(pExt->AircraftVeterancy[i]) < abs(veterancy))
											pExt->AircraftVeterancy[i] = veterancy;
									}
								}
							}
						}
					}
				}
			}
		}
		break;
	case AbstractType::BuildingType:
		for (auto pType : *BuildingTypeClass::Array)
		{
			if (pType->Trainable)
			{
				for (size_t i = 0; i < pExt->BuildingVeterancyTypes.size(); i++)
				{
					if (pType == pExt->BuildingVeterancyTypes[i])
					{
						bool UpgradeAllow = false;
						if (!types.empty())
						{
							for (TechnoTypeClass* pAffectType : types)
							{
								if (pType == pAffectType)
								{
									UpgradeAllow = true;
									break;
								}
							}
						}
						else
							UpgradeAllow = true;

						if (!ignore.empty())
						{
							for (TechnoTypeClass* pAffectType : ignore)
							{
								if (pType == pAffectType)
								{
									UpgradeAllow = false;
									break;
								}
							}
						}

						if (UpgradeAllow)
						{
							if (cumulative)
								pExt->BuildingVeterancy[i] += veterancy;
							else
							{
								if (veterancy == 0)
									pExt->BuildingVeterancy[i] = 0;
								else
								{
									if (pExt->BuildingVeterancy[i] * veterancy < 0)
										pExt->BuildingVeterancy[i] = veterancy;
									else
									{
										if (abs(pExt->BuildingVeterancy[i]) < abs(veterancy))
											pExt->BuildingVeterancy[i] = veterancy;
									}
								}
							}
						}
					}
				}
			}
		}
		break;
	case AbstractType::InfantryType:
		for (auto pType : *InfantryTypeClass::Array)
		{
			if (pType->Trainable)
			{
				for (size_t i = 0; i < pExt->InfantryVeterancyTypes.size(); i++)
				{
					if (pType == pExt->InfantryVeterancyTypes[i])
					{
						bool UpgradeAllow = false;
						if (!types.empty())
						{
							for (TechnoTypeClass* pAffectType : types)
							{
								if (pType == pAffectType)
								{
									UpgradeAllow = true;
									break;
								}
							}
						}
						else
							UpgradeAllow = true;

						if (!ignore.empty())
						{
							for (TechnoTypeClass* pAffectType : ignore)
							{
								if (pType == pAffectType)
								{
									UpgradeAllow = false;
									break;
								}
							}
						}

						if (UpgradeAllow)
						{
							if (cumulative)
								pExt->InfantryVeterancy[i] += veterancy;
							else
							{
								if (veterancy == 0)
									pExt->InfantryVeterancy[i] = 0;
								else
								{
									if (pExt->InfantryVeterancy[i] * veterancy < 0)
										pExt->InfantryVeterancy[i] = veterancy;
									else
									{
										if (abs(pExt->InfantryVeterancy[i]) < abs(veterancy))
											pExt->InfantryVeterancy[i] = veterancy;
									}
								}
							}
						}
					}
				}
			}
		}
		for (auto pType : *UnitTypeClass::Array)
		{
			if (pType->Trainable && pType->Organic)
			{
				for (size_t i = 0; i < pExt->InfantryVeterancyTypes.size(); i++)
				{
					if (pType == pExt->InfantryVeterancyTypes[i])
					{
						bool UpgradeAllow = false;
						if (!types.empty())
						{
							for (TechnoTypeClass* pAffectType : types)
							{
								if (pType == pAffectType)
								{
									UpgradeAllow = true;
									break;
								}
							}
						}
						else
							UpgradeAllow = true;

						if (!ignore.empty())
						{
							for (TechnoTypeClass* pAffectType : ignore)
							{
								if (pType == pAffectType)
								{
									UpgradeAllow = false;
									break;
								}
							}
						}

						if (UpgradeAllow)
						{
							if (cumulative)
								pExt->InfantryVeterancy[i] += veterancy;
							else
							{
								if (veterancy == 0)
									pExt->InfantryVeterancy[i] = 0;
								else
								{
									if (pExt->InfantryVeterancy[i] * veterancy < 0)
										pExt->InfantryVeterancy[i] = veterancy;
									else
									{
										if (abs(pExt->InfantryVeterancy[i]) < abs(veterancy))
											pExt->InfantryVeterancy[i] = veterancy;
									}
								}
							}
						}
					}
				}
			}
		}
		break;
	case AbstractType::UnitType:
		for (auto pType : *UnitTypeClass::Array)
		{
			if (naval)
			{
				if (pType->Trainable && pType->Naval && !pType->ConsideredAircraft && !pType->Organic)
				{
					for (size_t i = 0; i < pExt->NavalVeterancyTypes.size(); i++)
					{
						if (pType == pExt->NavalVeterancyTypes[i])
						{
							bool UpgradeAllow = false;
							if (!types.empty())
							{
								for (TechnoTypeClass* pAffectType : types)
								{
									if (pType == pAffectType)
									{
										UpgradeAllow = true;
										break;
									}
								}
							}
							else
								UpgradeAllow = true;

							if (!ignore.empty())
							{
								for (TechnoTypeClass* pAffectType : ignore)
								{
									if (pType == pAffectType)
									{
										UpgradeAllow = false;
										break;
									}
								}
							}

							if (UpgradeAllow)
							{
								if (cumulative)
									pExt->NavalVeterancy[i] += veterancy;
								else
								{
									if (veterancy == 0)
										pExt->NavalVeterancy[i] = 0;
									else
									{
										if (pExt->NavalVeterancy[i] * veterancy < 0)
											pExt->NavalVeterancy[i] = veterancy;
										else
										{
											if (abs(pExt->NavalVeterancy[i]) < abs(veterancy))
												pExt->NavalVeterancy[i] = veterancy;
										}
									}
								}
							}
						}
					}
				}
			}
			else
			{
				if (pType->Trainable && !pType->Naval && !pType->ConsideredAircraft && !pType->Organic)
				{
					for (size_t i = 0; i < pExt->VehicleVeterancyTypes.size(); i++)
					{
						if (pType == pExt->VehicleVeterancyTypes[i])
						{
							bool UpgradeAllow = false;
							if (!types.empty())
							{
								for (TechnoTypeClass* pAffectType : types)
								{
									if (pType == pAffectType)
									{
										UpgradeAllow = true;
										break;
									}
								}
							}
							else
								UpgradeAllow = true;

							if (!ignore.empty())
							{
								for (TechnoTypeClass* pAffectType : ignore)
								{
									if (pType == pAffectType)
									{
										UpgradeAllow = false;
										break;
									}
								}
							}

							if (UpgradeAllow)
							{
								if (cumulative)
									pExt->VehicleVeterancy[i] += veterancy;
								else
								{
									if (veterancy == 0)
										pExt->VehicleVeterancy[i] = 0;
									else
									{
										if (pExt->VehicleVeterancy[i] * veterancy < 0)
											pExt->VehicleVeterancy[i] = veterancy;
										else
										{
											if (abs(pExt->VehicleVeterancy[i]) < abs(veterancy))
												pExt->VehicleVeterancy[i] = veterancy;
										}
									}
								}
							}
						}
					}
				}
			}
		}
		break;
	default:
		break;
	}
}

void HouseExt::SpySuperWeaponCount(HouseClass* pThis)
{
	ExtData* pExt = ExtMap.Find(pThis);
	for (size_t i = 0; i < pExt->SpySuperWeaponTypes.size(); i++)
	{
		if (pExt->SpySuperWeaponDelay[i] > 0)
			pExt->SpySuperWeaponDelay[i]--;
	}
}

void HouseExt::CheckSuperWeaponCumulativeMax(HouseClass* pThis)
{
	ExtData* pExt = ExtMap.Find(pThis);
	if (pExt->SuperWeaponCumulativeCharge.empty())
	{
		for (int i = 0; i < pThis->Supers.Count; i++)
		{
			SuperClass* pSuper = pThis->Supers[i];
			pExt->SuperWeaponCumulativeCharge.emplace_back(pSuper->Type->RechargeTime);
			pExt->SuperWeaponCumulativeCount.emplace_back(0);
			pExt->SuperWeaponCumulativeMaxCount.emplace_back(0);
			pExt->SuperWeaponCumulativeInherit.emplace_back(false);
			pExt->SuperWeaponCumulativeSupplement.emplace_back(false);
		}
	}

	for (int i = 0; i < pThis->Supers.Count; i++)
	{
		SuperClass* pSuper = pThis->Supers[i];
		const auto pSWExt = SWTypeExt::ExtMap.Find(pSuper->Type);
		if (pSuper->Granted && pSWExt->SW_Cumulative)
		{
			int count = pSWExt->SW_Cumulative_InitialCount;
			for (size_t j = 0; j < pSWExt->SW_Cumulative_AdditionTypes.size(); j++)
			{
				for (auto pTechno : *TechnoClass::Array)
				{
					if (pTechno->GetTechnoType() == pSWExt->SW_Cumulative_AdditionTypes[j] && pTechno->Owner == pThis)
					{
						if (pSWExt->SW_Cumulative_AdditionCounts.size() >= j)
							count += pSWExt->SW_Cumulative_AdditionCounts[j];
					}

					if (count >= pSWExt->SW_Cumulative_MaxCount && pSWExt->SW_Cumulative_MaxCount > 0)
					{
						count = pSWExt->SW_Cumulative_MaxCount;
						break;
					}
				}

				if (count >= pSWExt->SW_Cumulative_MaxCount && pSWExt->SW_Cumulative_MaxCount > 0)
				{
					count = pSWExt->SW_Cumulative_MaxCount;
					break;
				}
			}
			pExt->SuperWeaponCumulativeMaxCount[i] = count;
		}
	}
}

void HouseExt::SuperWeaponCumulative(HouseClass* pThis)
{
	ExtData* pExt = ExtMap.Find(pThis);

	for (int i = 0; i < pThis->Supers.Count; i++)
	{
		SuperClass* pSuper = pThis->Supers[i];
		const auto pSWExt = SWTypeExt::ExtMap.Find(pSuper->Type);
		if (pSuper->Granted && pSWExt->SW_Cumulative)
		{
			bool allowcount = true;
			if (pExt->SuperWeaponCumulativeCount[i] >= pExt->SuperWeaponCumulativeMaxCount[i])
			{
				if (pExt->SuperWeaponCumulativeMaxCount[i] > 0)
					pExt->SuperWeaponCumulativeCount[i] = pExt->SuperWeaponCumulativeMaxCount[i];
				else
					pExt->SuperWeaponCumulativeCount[i] = 0;
				allowcount = false;
			}

			if (allowcount)
			{
				if (pSuper->RechargeTimer.GetTimeLeft() <= 0 && !pSuper->IsOnHold)
					pExt->SuperWeaponCumulativeCharge[i]--;

				if (pExt->SuperWeaponCumulativeCharge[i] <= 0)
				{
					pExt->SuperWeaponCumulativeCharge[i] = pSuper->Type->RechargeTime;
					pExt->SuperWeaponCumulativeCount[i]++;
				}
			}

			if (pExt->SuperWeaponCumulativeSupplement[i])
			{
				bool hold = pSuper->IsOnHold;
				pSuper->RechargeTimer.Stop();
				pSuper->RechargeTimer.Start(0);
				pSuper->IsOnHold = hold;
				pExt->SuperWeaponCumulativeCount[i]--;
				pExt->SuperWeaponCumulativeSupplement[i] = false;
			}

			if (pExt->SuperWeaponCumulativeInherit[i])
			{
				bool hold = pSuper->IsOnHold;
				pSuper->RechargeTimer.Stop();
				pSuper->RechargeTimer.Start(pExt->SuperWeaponCumulativeCharge[i]);
				pSuper->IsOnHold = hold;
				pExt->SuperWeaponCumulativeCharge[i] = pSuper->Type->RechargeTime;
				pExt->SuperWeaponCumulativeInherit[i] = false;
			}
		}
		else if (!pSuper->Granted && pSWExt->SW_Cumulative)
		{
			pExt->SuperWeaponCumulativeCharge[i] = pSuper->Type->RechargeTime;
			pExt->SuperWeaponCumulativeCount[i] = 0;
		}
	}
}

void HouseExt::SuperWeaponCumulativeReset(HouseClass* pThis, SuperClass* pSW)
{
	for (int i = 0; i < pThis->Supers.Count; i++)
	{
		if (pThis->Supers[i] == pSW)
		{
			ExtData* pExt = ExtMap.Find(pThis);
			if (pExt->SuperWeaponCumulativeCount[i] == 0)
				pExt->SuperWeaponCumulativeInherit[i] = true;
			else if (pExt->SuperWeaponCumulativeCount[i] > 0)
				pExt->SuperWeaponCumulativeSupplement[i] = true;
			break;
		}
	}
}

// =============================
// load / save

template <typename T>
void HouseExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Factory_BuildingType)
		.Process(this->Factory_InfantryType)
		.Process(this->Factory_VehicleType)
		.Process(this->Factory_NavyType)
		.Process(this->Factory_AircraftType)
		.Process(this->RepairBaseNodes)
		.Process(this->ForceOnlyTargetHouseEnemy)
		.Process(this->ForceOnlyTargetHouseEnemyMode)
		.Process(this->AlreadyGranted)
		.Process(this->ScoreVectorInited)
		.Process(this->OwnedAircraft)
		.Process(this->OwnedBuilding)
		.Process(this->OwnedInfantry)
		.Process(this->OwnedUnit)
		.Process(this->DeactivateInfantry_Types)
		.Process(this->DeactivateInfantry_Ignore)
		.Process(this->DeactivateInfantry_Duration)
		.Process(this->DeactivateVehicle_Types)
		.Process(this->DeactivateVehicle_Ignore)
		.Process(this->DeactivateVehicle_Duration)
		.Process(this->DeactivateNaval_Types)
		.Process(this->DeactivateNaval_Ignore)
		.Process(this->DeactivateNaval_Duration)
		.Process(this->DeactivateAircraft_Types)
		.Process(this->DeactivateAircraft_Ignore)
		.Process(this->DeactivateAircraft_Duration)
		.Process(this->DeactivateBuilding_Types)
		.Process(this->DeactivateBuilding_Ignore)
		.Process(this->DeactivateBuilding_Duration)
		.Process(this->DeactivateDefense_Types)
		.Process(this->DeactivateDefense_Ignore)
		.Process(this->DeactivateDefense_Duration)
		.Process(this->InfantryVeterancyTypes)
		.Process(this->InfantryVeterancy)
		.Process(this->VehicleVeterancyTypes)
		.Process(this->VehicleVeterancy)
		.Process(this->NavalVeterancyTypes)
		.Process(this->NavalVeterancy)
		.Process(this->AircraftVeterancyTypes)
		.Process(this->AircraftVeterancy)
		.Process(this->BuildingVeterancyTypes)
		.Process(this->BuildingVeterancy)
		.Process(this->SpySuperWeaponTypes)
		.Process(this->SpySuperWeaponDelay)
		.Process(this->SuperWeaponCumulativeCharge)
		.Process(this->SuperWeaponCumulativeCount)
		.Process(this->SuperWeaponCumulativeMaxCount)
		.Process(this->SuperWeaponCumulativeInherit)
		.Process(this->SuperWeaponCumulativeSupplement)
		.Process(this->PowerUnitOutPut)
		.Process(this->PowerUnitDrain)
		.Process(this->BuildingCount)
		.Process(this->BuildingCheckCount)
		;
}

void HouseExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<HouseClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void HouseExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<HouseClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool HouseExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Success();
}

bool HouseExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Success();
}

// =============================
// container

HouseExt::ExtContainer::ExtContainer() : Container("HouseClass")
{
}

HouseExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x4F6532, HouseClass_CTOR, 0x5)
{
	GET(HouseClass*, pItem, EAX);

	HouseExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(0x4F7371, HouseClass_DTOR, 0x6)
{
	GET(HouseClass*, pItem, ESI);

	HouseExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x504080, HouseClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x503040, HouseClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(HouseClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	HouseExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x504069, HouseClass_Load_Suffix, 0x7)
{
	HouseExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x5046DE, HouseClass_Save_Suffix, 0x7)
{
	HouseExt::ExtMap.SaveStatic();
	return 0;
}

DEFINE_HOOK(0x50114D, HouseClass_InitFromINI, 0x5)
{
	GET(HouseClass* const, pThis, EBX);
	GET(CCINIClass* const, pINI, ESI);

	HouseExt::ExtMap.LoadFromINI(pThis, pINI);

	return 0;
}
