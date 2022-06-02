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
	if (!pThis) return 0;

	int result = 0;
	for (auto techno : *TechnoClass::Array) {
		if (auto pTechnoExt = TechnoTypeExt::ExtMap.Find(techno->GetTechnoType())) {
			if (pTechnoExt->IsCountedAsHarvester() && techno->Owner == pThis)
			{
				if (auto pTechno = TechnoExt::ExtMap.Find(techno))
				{
					result += TechnoExt::IsHarvesting(techno);
				}
			}
		}
	}

	return result;
}

int HouseExt::TotalHarvesterCount(HouseClass* pThis)
{
	if (!pThis)	return 0;

	int result = 0;
	for (auto techno : *TechnoTypeClass::Array) {
		if (auto pTechnoExt = TechnoTypeExt::ExtMap.Find(techno)) {
			if (pTechnoExt->IsCountedAsHarvester()) {
				result += pThis->CountOwnedAndPresent(techno);
			}
		}
	}

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
	switch (kind) {
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

void HouseExt::ForceOnlyTargetHouseEnemy(HouseClass* pThis, int mode = -1)
{
	if (!pThis)
	{
		return;
	}

	auto pHouseExt = HouseExt::ExtMap.Find(pThis);
	if (!pHouseExt)
	{
		return;
	}

	if (mode < 0 || mode > 2)
		mode = -1;

	pHouseExt->ForceOnlyTargetHouseEnemyMode = mode;
	/*
	Modes:
		0  -> Force "False"
		1  -> Force "True"
		2  -> Force "Random boolean"
		-1 -> Use default value in Team->OnlyTargetHouseEnemy tag
		Note: only works for new Actions that use Team->OnlyTargetHouseEnemyMode, not vanilla YR actions
	*/
	switch (mode)
	{
	case 0:
		pHouseExt->ForceOnlyTargetHouseEnemy = false;
		break;

	case 1:
		pHouseExt->ForceOnlyTargetHouseEnemy = true;
		break;

	case 2:
		pHouseExt->ForceOnlyTargetHouseEnemy = (bool)ScenarioClass::Instance->Random.RandomRanged(0, 1);;
		break;

	default:
		pHouseExt->ForceOnlyTargetHouseEnemyMode = -1;
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

HouseExt::BuildLimitStatus HouseExt::BuildLimitGroupCheck(HouseClass* pThis, TechnoTypeClass* pItem, BuildLimitStatus Origin)
{
	auto pItemExt = TechnoTypeExt::ExtMap.Find(pItem);
	if (Origin != BuildLimitStatus::NotReached
		|| pItemExt->BuildLimit_Group_Types.empty())
		return Origin;
	if (pItemExt->BuildLimit_Group_Any.Get())
	{
		for (size_t i = 0;
			i < std::min(
				pItemExt->BuildLimit_Group_Types.size(),
				pItemExt->BuildLimit_Group_Limits.size())
			; i++)
		{
			TechnoTypeClass* pType = TechnoTypeClass::Array->GetItem(pItemExt->BuildLimit_Group_Types[i]);
			if (pThis->CountOwnedAndPresent(pType) >= pItemExt->BuildLimit_Group_Limits[i])
				return BuildLimitStatus::ReachedPermanently;
		}
		return BuildLimitStatus::NotReached;
	}
	else
	{
		if (pItemExt->BuildLimit_Group_Limits.size() == 1U)
		{
			int sum = 0;
			for (auto& pTypeIdx : pItemExt->BuildLimit_Group_Types)
			{
				TechnoTypeClass* pType = TechnoTypeClass::Array->GetItem(pTypeIdx);
				sum += pThis->CountOwnedAndPresent(pType);
			}
			if (sum >= pItemExt->BuildLimit_Group_Limits[0])
				return BuildLimitStatus::ReachedPermanently;
			return BuildLimitStatus::NotReached;
		}
		else
		{
			for (size_t i = 0;
			i < std::min(
				pItemExt->BuildLimit_Group_Types.size(),
				pItemExt->BuildLimit_Group_Limits.size())
			; i++)
			{
				TechnoTypeClass* pType = TechnoTypeClass::Array->GetItem(pItemExt->BuildLimit_Group_Limits[i]);
				if (pThis->CountOwnedAndPresent(pType) < pItemExt->BuildLimit_Group_Limits[i])
					return BuildLimitStatus::NotReached;
			}
			return BuildLimitStatus::ReachedPermanently;
		}
	}
}

// =============================
// load / save

template <typename T>
void HouseExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->BuildingCounter)
		.Process(this->ForceOnlyTargetHouseEnemy)
		.Process(this->ForceOnlyTargetHouseEnemyMode)
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

HouseExt::ExtContainer::ExtContainer() : Container("HouseClass") {
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
