#include "Body.h"

#include <Ext/TechnoType/Body.h>
#include <Ext/Techno/Body.h>

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
	for (size_t idx = 0; idx < readBaseNodeRepairInfo.size(); idx++)
		this->RepairBaseNodes[idx] = readBaseNodeRepairInfo[idx];
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
	bool NotObserver = !pThis->IsObserver() || !pThis->IsPlayerObserver();
	bool granted;
	granted = pSuper->Grant(true, NotObserver, false);
	if (granted && NotObserver && pThis == HouseClass::Player)
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
			if (pHouse->ControlledByHuman()
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

int HouseExt::CountOwnedIncludeDeploy(HouseClass* pThis, TechnoTypeClass* pItem)
{
	return pThis->CountOwnedNow(pItem) +
		(pItem->DeploysInto == nullptr ? 0 : pThis->CountOwnedNow(pItem->DeploysInto)) +
		(pItem->UndeploysInto == nullptr ? 0 : pThis->CountOwnedNow(pItem->UndeploysInto));
}

HouseExt::BuildLimitStatus HouseExt::BuildLimitGroupCheck(HouseClass* pThis, TechnoTypeClass* pItem, bool buildLimitOnly, bool includeQueued, BuildLimitStatus Origin)
{
	if (!buildLimitOnly)
	{
		if (Origin == BuildLimitStatus::ReachedTemporarily)
			return Origin;

		if (!pThis->ControlledByHuman())
		{
			if (Phobos::Config::AllowBypassBuildLimit[pThis->GetAIDifficultyIndex()])
				return BuildLimitStatus::NotReached;

			return BuildLimitGroupValidate(pThis, pItem, includeQueued, Origin);
		}
	}

	FactoryState state = HouseExt::HasFactory_Ares(pThis, pItem, true);

	if (state != FactoryState::Available)
		return BuildLimitStatus::ReachedTemporarily;

	return BuildLimitGroupValidate(pThis, pItem, includeQueued, Origin);
}

HouseExt::BuildLimitStatus HouseExt::BuildLimitGroupValidate(HouseClass* pThis, TechnoTypeClass* pItem, bool includeQueued, BuildLimitStatus Origin)
{
	auto pItemExt = TechnoTypeExt::ExtMap.Find(pItem);

	if (pItemExt->BuildLimit_Group_Types.empty())
		return Origin;

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
		return reachedLimit ? BuildLimitStatus::ReachedPermanently : BuildLimitStatus::NotReached;
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
					;
					reachedLimit |= (includeQueued && FactoryClass::FindByOwnerAndProduct(pThis, pType))
						? false : true;
				}
			}
			return reachedLimit ? BuildLimitStatus::ReachedPermanently : BuildLimitStatus::NotReached;
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
					return BuildLimitStatus::NotReached;
			}
			return BuildLimitStatus::ReachedPermanently;
		}
	}
}

HouseExt::FactoryState HouseExt::HasFactory_Ares(HouseClass* pThis, TechnoTypeClass* pItem, bool requirePower)
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
