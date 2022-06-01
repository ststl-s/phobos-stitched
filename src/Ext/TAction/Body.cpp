#include "Body.h"

#include <SessionClass.h>
#include <MessageListClass.h>
#include <HouseClass.h>
#include <CRT.h>
#include <BuildingClass.h>
#include <RadSiteClass.h>
#include <LightSourceClass.h>

#include <SuperWeaponTypeClass.h>
#include <SuperClass.h>
#include <TacticalClass.h>
#include <Ext/SWType/Body.h>
#include <Utilities/SavegameDef.h>
#include <New/Entity/BannerClass.h>
#include <New/Entity/ExternVariableClass.h>
#include <Utilities/PhobosGlobal.h>
#include <TriggerClass.h>
#include <TriggerTypeClass.h>

#include <Ext/Scenario/Body.h>

//Static init
template<> const DWORD Extension<TActionClass>::Canary = 0x91919191;
TActionExt::ExtContainer TActionExt::ExtMap;

// =============================
// load / save

template <typename T>
void TActionExt::ExtData::Serialize(T& Stm)
{
	//Stm;
}

void TActionExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<TActionClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void TActionExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<TActionClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

void TActionExt::RecreateLightSources()
{
	for (auto pBld : *BuildingClass::Array)
	{
		if (pBld->LightSource)
		{
			bool activated = pBld->LightSource->Activated;

			GameDelete(pBld->LightSource);
			if (pBld->Type->LightIntensity)
			{
				TintStruct color { pBld->Type->LightRedTint, pBld->Type->LightGreenTint, pBld->Type->LightBlueTint };

				pBld->LightSource = GameCreate<LightSourceClass>(pBld->GetCoords(),
					pBld->Type->LightVisibility, pBld->Type->LightIntensity, color);

				if (activated)
					pBld->LightSource->Activate();
				else
					pBld->LightSource->Deactivate();
			}
		}
	}

	for (auto pRadSite : *RadSiteClass::Array)
	{
		if (pRadSite->LightSource)
		{
			auto coord = pRadSite->LightSource->Location;
			auto color = pRadSite->LightSource->LightTint;
			auto intensity = pRadSite->LightSource->LightIntensity;
			auto visibility = pRadSite->LightSource->LightVisibility;
			bool activated = pRadSite->LightSource->Activated;

			GameDelete(pRadSite->LightSource);

			pRadSite->LightSource = GameCreate<LightSourceClass>(coord,
				visibility, intensity, color);

			if (activated)
				pRadSite->LightSource->Activate();
			else
				pRadSite->LightSource->Deactivate();
		}
	}
}

bool TActionExt::Execute(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject,
	TriggerClass* pTrigger, CellStruct const& location, bool& bHandled)
{
	bHandled = true;

	// Vanilla
	switch (pThis->ActionKind)
	{
	case TriggerAction::PlaySoundEffectRandom:
		return TActionExt::PlayAudioAtRandomWP(pThis, pHouse, pObject, pTrigger, location);
	default:
		break;
	};

	// Phobos
	switch (static_cast<PhobosTriggerAction>(pThis->ActionKind))
	{
	case PhobosTriggerAction::SaveGame:
		return TActionExt::SaveGame(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::EditVariable:
		return TActionExt::EditVariable(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::GenerateRandomNumber:
		return TActionExt::GenerateRandomNumber(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::PrintVariableValue:
		return TActionExt::PrintVariableValue(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::BinaryOperation:
		return TActionExt::BinaryOperation(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::AdjustLighting:
		return TActionExt::AdjustLighting(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::RunSuperWeaponAtLocation:
		return TActionExt::RunSuperWeaponAtLocation(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::RunSuperWeaponAtWaypoint:
		return TActionExt::RunSuperWeaponAtWaypoint(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::CreateBannerGlobal:
		return TActionExt::CreateBannerGlobal(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::CreateBannerLocal:
		return TActionExt::CreateBannerLocal(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::DeleteBanner:
		return TActionExt::DeleteBanner(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::LoadExternVarToLocalVar:
		return TActionExt::LoadExternVarToLocalVar(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::LoadExternVarToGlobalVar:
		return TActionExt::LoadExternVarToGlobalVar(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::SaveLocalVarToExternVar:
		return TActionExt::SaveLocalVarToExternVar(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::SaveGlobalVarToExternVar:
		return TActionExt::SaveGlobalVarToExternVar(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::MessageForSpecifiedHouse:
		return TActionExt::MessageForSpecifiedHouse(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::RandomTriggerPut:
		return TActionExt::RandomTriggerPut(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::RandomTriggerEnable:
		return TActionExt::RandomTriggerEnable(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::RandomTriggerRemove:
		return TActionExt::RandomTriggerRemove(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::ScoreCampaignText:
		return TActionExt::ScoreCampaignText(pThis, pHouse, pObject, pTrigger, location);
	case PhobosTriggerAction::ScoreCampaignTheme:
		return TActionExt::ScoreCampaignTheme(pThis, pHouse, pObject, pTrigger, location);
	default:
		bHandled = false;
		return true;
	}
}

bool TActionExt::PlayAudioAtRandomWP(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	std::vector<int> waypoints;
	waypoints.reserve(ScenarioExt::Global()->Waypoints.size());

	auto const pScen = ScenarioClass::Instance();

	for (auto pair : ScenarioExt::Global()->Waypoints)
		if (pScen->IsDefinedWaypoint(pair.first))
			waypoints.push_back(pair.first);

	if (waypoints.size() > 0)
	{
		auto const index = pScen->Random.RandomRanged(0, waypoints.size() - 1);
		auto const luckyWP = waypoints[index];
		auto const cell = pScen->GetWaypointCoords(luckyWP);
		auto const coords = CellClass::Cell2Coord(cell);
		VocClass::PlayIndexAtPos(pThis->Value, coords);
	}

	return true;
}

bool TActionExt::SaveGame(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	if (SessionClass::Instance->GameMode == GameMode::Campaign || SessionClass::Instance->GameMode == GameMode::Skirmish)
	{
		auto PrintMessage = [](const wchar_t* pMessage)
		{
			MessageListClass::Instance->PrintMessage(
				pMessage,
				RulesClass::Instance->MessageDelay,
				HouseClass::Player->ColorSchemeIndex,
				true
			);
		};

		char fName[0x80];

		SYSTEMTIME time;
		GetLocalTime(&time);

		_snprintf_s(fName, 0x7F, "Map.%04u%02u%02u-%02u%02u%02u-%05u.sav",
			time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, time.wMilliseconds);

		PrintMessage(StringTable::LoadString("TXT_SAVING_GAME"));

		wchar_t fDescription[0x80] = { 0 };
		wcscpy_s(fDescription, ScenarioClass::Instance->UINameLoaded);
		wcscat_s(fDescription, L" - ");
		wcscat_s(fDescription, StringTable::LoadString(pThis->Text));

		if (ScenarioClass::Instance->SaveGame(fName, fDescription))
			PrintMessage(StringTable::LoadString("TXT_GAME_WAS_SAVED"));
		else
			PrintMessage(StringTable::LoadString("TXT_ERROR_SAVING_GAME"));
	}

	return true;
}

bool TActionExt::EditVariable(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	// Variable Index
	// holds by pThis->Value

	// Operations:
	// 0 : set value - operator=
	// 1 : add value - operator+
	// 2 : minus value - operator-
	// 3 : multiply value - operator*
	// 4 : divide value - operator/
	// 5 : mod value - operator%
	// 6 : <<
	// 7 : >>
	// 8 : ~ (no second param being used)
	// 9 : ^
	// 10 : |
	// 11 : &
	// holds by pThis->Param3

	// Params:
	// The second value
	// holds by pThis->Param4

	// Global Variable or Local
	// 0 for local and 1 for global
	// holds by pThis->Param5

	// uses !pThis->Param5 to ensure Param5 is 0 or 1
	auto& variables = ScenarioExt::Global()->Variables[pThis->Param5 != 0];
	auto itr = variables.find(pThis->Value);
	if (itr != variables.end())
	{
		auto& nCurrentValue = itr->second.Value;
		// variable being found
		switch (pThis->Param3)
		{
		case 0: { nCurrentValue = pThis->Param4; break; }
		case 1: { nCurrentValue += pThis->Param4; break; }
		case 2: { nCurrentValue -= pThis->Param4; break; }
		case 3: { nCurrentValue *= pThis->Param4; break; }
		case 4: { nCurrentValue /= pThis->Param4; break; }
		case 5: { nCurrentValue %= pThis->Param4; break; }
		case 6: { nCurrentValue <<= pThis->Param4; break; }
		case 7: { nCurrentValue >>= pThis->Param4; break; }
		case 8: { nCurrentValue = ~nCurrentValue; break; }
		case 9: { nCurrentValue ^= pThis->Param4; break; }
		case 10: { nCurrentValue |= pThis->Param4; break; }
		case 11: { nCurrentValue &= pThis->Param4; break; }
		default:
			return true;
		}

		if (!pThis->Param5)
			TagClass::NotifyLocalChanged(pThis->Value);
		else
			TagClass::NotifyGlobalChanged(pThis->Value);
	}
	return true;
}

bool TActionExt::GenerateRandomNumber(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	auto& variables = ScenarioExt::Global()->Variables[pThis->Param5 != 0];
	auto itr = variables.find(pThis->Value);
	if (itr != variables.end())
	{
		itr->second.Value = ScenarioClass::Instance->Random.RandomRanged(pThis->Param3, pThis->Param4);
		if (!pThis->Param5)
			TagClass::NotifyLocalChanged(pThis->Value);
		else
			TagClass::NotifyGlobalChanged(pThis->Value);
	}

	return true;
}

bool TActionExt::PrintVariableValue(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	auto& variables = ScenarioExt::Global()->Variables[pThis->Param3 != 0];
	auto itr = variables.find(pThis->Value);
	if (itr != variables.end())
	{
		CRT::swprintf(Phobos::wideBuffer, L"%d", itr->second.Value);
		MessageListClass::Instance->PrintMessage(Phobos::wideBuffer);
	}

	return true;
}

bool TActionExt::BinaryOperation(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	auto& variables1 = ScenarioExt::Global()->Variables[pThis->Param5 != 0];
	auto itr1 = variables1.find(pThis->Value);
	auto& variables2 = ScenarioExt::Global()->Variables[pThis->Param6 != 0];
	auto itr2 = variables2.find(pThis->Param4);

	if (itr1 != variables1.end() && itr2 != variables2.end())
	{
		auto& nCurrentValue = itr1->second.Value;
		auto& nOptValue = itr2->second.Value;
		switch (pThis->Param3)
		{
		case 0: { nCurrentValue = nOptValue; break; }
		case 1: { nCurrentValue += nOptValue; break; }
		case 2: { nCurrentValue -= nOptValue; break; }
		case 3: { nCurrentValue *= nOptValue; break; }
		case 4: { nCurrentValue /= nOptValue; break; }
		case 5: { nCurrentValue %= nOptValue; break; }
		case 6: { nCurrentValue <<= nOptValue; break; }
		case 7: { nCurrentValue >>= nOptValue; break; }
		case 8: { nCurrentValue = nOptValue; break; }
		case 9: { nCurrentValue ^= nOptValue; break; }
		case 10: { nCurrentValue |= nOptValue; break; }
		case 11: { nCurrentValue &= nOptValue; break; }
		default:
			return true;
		}

		if (!pThis->Param5)
			TagClass::NotifyLocalChanged(pThis->Value);
		else
			TagClass::NotifyGlobalChanged(pThis->Value);
	}
	return true;
}

bool TActionExt::RunSuperWeaponAtLocation(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	if (!pThis)
		return true;

	TActionExt::RunSuperWeaponAt(pThis, pThis->Param5, pThis->Param6);

	return true;
}

bool TActionExt::RunSuperWeaponAtWaypoint(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	if (!pThis)
		return true;

	auto& waypoints = ScenarioExt::Global()->Waypoints;
	int nWaypoint = pThis->Param5;

	// Check if is a valid Waypoint
	if (nWaypoint >= 0 && waypoints.find(nWaypoint) != waypoints.end() && waypoints[nWaypoint].X && waypoints[nWaypoint].Y)
	{
		auto const selectedWP = waypoints[nWaypoint];
		TActionExt::RunSuperWeaponAt(pThis, selectedWP.X, selectedWP.Y);
	}

	return true;
}

bool TActionExt::RunSuperWeaponAt(TActionClass* pThis, int X, int Y)
{
	if (SuperWeaponTypeClass::Array->Count > 0)
	{
		SuperWeaponTypeClass* pSWType = SuperWeaponTypeClass::Find(pThis->Text);
		int houseIdx = -1;
		std::vector<int> housesListIdx;
		CellStruct targetLocation = { (short)X, (short)Y };

		if (pSWType == nullptr)
			return true;

		do
		{
			if (X < 0)
				targetLocation.X = (short)ScenarioClass::Instance->Random.RandomRanged(0, MapClass::Instance->MapCoordBounds.Right);

			if (Y < 0)
				targetLocation.Y = (short)ScenarioClass::Instance->Random.RandomRanged(0, MapClass::Instance->MapCoordBounds.Bottom);
		}
		while (!MapClass::Instance->IsWithinUsableArea(targetLocation, false));

		HouseClass* pHouse = nullptr;

		switch (pThis->Param4)
		{
		case -1:
			for (auto pTmp : *HouseClass::Array)
			{
				if (!pTmp->Defeated
					&& !pTmp->IsObserver()
					&& !pTmp->Type->MultiplayPassive)
				{
					housesListIdx.push_back(pTmp->ArrayIndex);
				}
			}

			if (housesListIdx.size() > 0)
				houseIdx = housesListIdx.at(ScenarioClass::Instance->Random.RandomRanged(0, housesListIdx.size() - 1));
			else
				return true;
			pHouse = HouseClass::FindByIndex(houseIdx);
			break;

		case -2:
			// Find first Neutral
			for (auto pHouseNeutral : *HouseClass::Array)
			{
				if (pHouseNeutral->IsNeutral())
				{
					houseIdx = pHouseNeutral->ArrayIndex;
					break;
				}
			}

			if (houseIdx < 0)
				return true;
			pHouse = HouseClass::FindByIndex(houseIdx);
			break;

		case -3:
			// Random Human Player
			for (auto ptmpHouse : *HouseClass::Array)
			{
				if (ptmpHouse->ControlledByHuman()
					&& !ptmpHouse->Defeated
					&& !ptmpHouse->IsObserver())
				{
					housesListIdx.push_back(ptmpHouse->ArrayIndex);
				}
			}

			if (housesListIdx.size() > 0)
				houseIdx = housesListIdx.at(ScenarioClass::Instance->Random.RandomRanged(0, housesListIdx.size() - 1));
			else
				return true;
			pHouse = HouseClass::FindByIndex(houseIdx);
			break;

		default:
			if (pThis->Param4 >= 0)
			{
				if (HouseClass::Index_IsMP(pThis->Param4))
					pHouse = HouseClass::FindByIndex(pThis->Param4);
				else
					pHouse = HouseClass::FindByCountryIndex(pThis->Param4);
			}
			else
				return true;
			break;
		}

		//HouseClass* pHouse = HouseClass::Array->GetItem(houseIdx);
		if (pHouse == nullptr) 
			return true;
		
		SuperClass* pSuper = GameCreate<SuperClass>(pSWType, pHouse);
		auto const pSWExt = SWTypeExt::ExtMap.Find(pSWType);
		if (pSWExt != nullptr)
		{
			pSuper->SetReadiness(true);
			pSuper->Launch(targetLocation, false);
		}
	}
	return true;
}

bool TActionExt::AdjustLighting(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	if (pThis->Param3 != -1)
		ScenarioClass::Instance->NormalLighting.Tint.Red = pThis->Param3;
	if (pThis->Param4 != -1)
		ScenarioClass::Instance->NormalLighting.Tint.Green = pThis->Param4;
	if (pThis->Param5 != -1)
		ScenarioClass::Instance->NormalLighting.Tint.Blue = pThis->Param5;

	const int r = ScenarioClass::Instance->NormalLighting.Tint.Red * 10;
	const int g = ScenarioClass::Instance->NormalLighting.Tint.Green * 10;
	const int b = ScenarioClass::Instance->NormalLighting.Tint.Blue * 10;

	if (pThis->Value & 0b001) // Update Tiles
	{
		for (auto& pLightConvert : *LightConvertClass::Array)
			pLightConvert->UpdateColors(r, g, b, false);
		ScenarioExt::Global()->CurrentTint_Tiles = ScenarioClass::Instance->NormalLighting.Tint;
	}

	if (pThis->Value & 0b010) // Update Units & Buildings
	{
		for (auto& pScheme : *ColorScheme::Array)
			pScheme->LightConvert->UpdateColors(r, g, b, false);
		ScenarioExt::Global()->CurrentTint_Schemes = ScenarioClass::Instance->NormalLighting.Tint;
	}

	if (pThis->Value & 0b100) // Update CustomPalettes (vanilla YR LightConvertClass one, not the Ares ConvertClass only one)
	{
		ScenarioClass::UpdateHashPalLighting(r, g, b, false);
		ScenarioExt::Global()->CurrentTint_Hashes = ScenarioClass::Instance->NormalLighting.Tint;
	}

	ScenarioClass::UpdateCellLighting();
	MapClass::Instance->RedrawSidebar(1); // GScreenClass::Flag_To_Redraw

	// #issue 429
	TActionExt::RecreateLightSources();

	return true;
}

void CreateOrReplaceBanner(TActionClass* pTAction, bool isGlobal)
{
	BannerTypeClass* pBannerType = BannerTypeClass::Array[pTAction->Param3].get();

	bool found = false;
	for (int i = 0; i < BannerClass::Array.Count; i++)
	{
		if (BannerClass::Array[i]->Id == pTAction->Value)
		{
			BannerClass::Array[i]->Type = pBannerType;
			BannerClass::Array[i]->Position = CoordStruct { pTAction->Param4, pTAction->Param5, 0 };
			BannerClass::Array[i]->Variable = pTAction->Param6;
			BannerClass::Array[i]->IsGlobalVariable = isGlobal;
			found = true;
			break;
		}
	}
	if (!found)
		new BannerClass(pBannerType, pTAction->Value, CoordStruct { pTAction->Param4, pTAction->Param5, 0 }, pTAction->Param6, isGlobal);
}

void SortBanners(int start, int end)
{
	// just a quicksort
	if (start >= end)
		return;

	int pivotId = BannerClass::Array[start]->Id;

	int count = 0;
	for (int i = start + 1; i <= end; i++)
	{
		if (BannerClass::Array[i]->Id <= pivotId)
			count++;
	}

	int pivot = start + count;
	std::swap(BannerClass::Array[pivot], BannerClass::Array[start]);

	int i = start, j = end;
	while (i < pivot && j > pivot)
	{
		while (BannerClass::Array[i]->Id <= pivotId)
			i++;

		while (BannerClass::Array[j]->Id > pivotId)
			j--;

		if (i < pivot && j > pivot)
			std::swap(BannerClass::Array[i++], BannerClass::Array[j--]);
	}

	SortBanners(start, pivot - 1);
	SortBanners(pivot + 1, end);
}

bool TActionExt::CreateBannerGlobal(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	CreateOrReplaceBanner(pThis, true);
	SortBanners(0, BannerClass::Array.Count - 1);
	return true;
}

bool TActionExt::CreateBannerLocal(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	CreateOrReplaceBanner(pThis, false);
	SortBanners(0, BannerClass::Array.Count - 1);
	return true;
}

bool TActionExt::DeleteBanner(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	int j = -1;
	for (int i = 0; i < BannerClass::Array.Count; i++)
	{
		if (BannerClass::Array[i]->Id == pThis->Value)
		{
			j = i;
			break;
		}
	}
	if (j != -1)
	{
		auto pBanner = BannerClass::Array[j];
		BannerClass::Array.RemoveItem(j);
		delete pBanner;
	}

	return true;
}

bool TActionExt::LoadExternVarToLocalVar(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	ExternVariableClass* pExtVar = ExternVariableClass::GetExternVariable(pThis->Text);
	auto it = ScenarioExt::Global()->Variables[0].find(pThis->Param4);
	if (it != ScenarioExt::Global()->Variables[0].end() && pExtVar != nullptr)
		it->second.Value = pExtVar->intValue;
	return true;
}

bool TActionExt::LoadExternVarToGlobalVar(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	ExternVariableClass* pExtVar = ExternVariableClass::GetExternVariable(pThis->Text);
	auto it = ScenarioExt::Global()->Variables[1].find(pThis->Param4);
	if (it != ScenarioExt::Global()->Variables[1].end() && pExtVar != nullptr)
		it->second.Value = pExtVar->intValue;
	return true;
}

bool TActionExt::SaveLocalVarToExternVar(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	ExternVariableClass* pExtVar = ExternVariableClass::GetExternVariable(pThis->Text);
	auto it = ScenarioExt::Global()->Variables[0].find(pThis->Param4);
	if (it != ScenarioExt::Global()->Variables[0].end() && pExtVar != nullptr)
	{
		pExtVar->intValue = it->second.Value;
		ExternVariableClass::SaveVariableToFile(*pExtVar);
	}
	return true;
}

bool TActionExt::SaveGlobalVarToExternVar(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	ExternVariableClass* pExtVar = ExternVariableClass::GetExternVariable(pThis->Text);
	auto it = ScenarioExt::Global()->Variables[1].find(pThis->Param4);
	if (it != ScenarioExt::Global()->Variables[1].end() && pExtVar != nullptr)
	{
		pExtVar->intValue = it->second.Value;
		ExternVariableClass::SaveVariableToFile(*pExtVar);
	}
	return true;
}

bool TActionExt::MessageForSpecifiedHouse(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	int houseIdx = 0;
	if (pThis->Param3 == -3)
	{
		// Random Human Player
		std::vector<int> housesListIdx;
		for (auto ptmpHouse : *HouseClass::Array)
		{
			if (ptmpHouse->ControlledByHuman()
				&& !ptmpHouse->Defeated
				&& !ptmpHouse->IsObserver())
			{
				housesListIdx.push_back(ptmpHouse->ArrayIndex);
			}
		}

		if (housesListIdx.size() > 0)
			houseIdx = housesListIdx.at(ScenarioClass::Instance->Random.RandomRanged(0, housesListIdx.size() - 1));
		else
			return true;
	}
	else
	{
		houseIdx = pThis->Param3;
	}
	
	HouseClass* pTargetHouse = nullptr;

	if (HouseClass::Index_IsMP(houseIdx))
		pTargetHouse = HouseClass::FindByIndex(houseIdx);
	else
		pTargetHouse = HouseClass::FindByCountryIndex(houseIdx);

	if (pTargetHouse == nullptr)
		return true;

	for (int i = 0; i < HouseClass::Array->Count; i++)
	{
		auto pTmpHouse = HouseClass::Array->GetItem(i);
		if (pTmpHouse->ControlledByPlayer() && pTmpHouse == pTargetHouse)
		{
			MessageListClass::Instance->PrintMessage(StringTable::LoadStringA(pThis->Text), RulesClass::Instance->MessageDelay, pTmpHouse->ColorSchemeIndex);
		}
	}
	return true;
}

bool TActionExt::RandomTriggerPut(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	TriggerTypeClass* pTargetType = pThis->TriggerType;
	
	if (pTargetType == nullptr)
		return true;

	TriggerClass* pTarget = TriggerClass::GetInstance(pTargetType);
	int PoolID = pThis->Param3;

	if (pTarget != nullptr)
		PhobosGlobal::Global()->RandomTriggerPool[PoolID].emplace(pTarget);
	
	return true;
}

bool TActionExt::RandomTriggerEnable(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	int PoolID = pThis->Param3;
	bool TakeOff = pThis->Param4;
	
	if (!PhobosGlobal::Global()->RandomTriggerPool.count(PoolID))
		return true;
	
	auto& Pool = PhobosGlobal::Global()->RandomTriggerPool[PoolID];
	
	if (Pool.empty())
		return true;
	
	int Pos = ScenarioClass::Instance->Random.RandomRanged(0, Pool.size() - 1);
	auto it = Pool.begin();

	while (Pos > 0 && it != Pool.end())
	{
		it++;
		Pos--;
	}
	TriggerClass* pTarget = *it;

	pTarget->Enable();
	
	if (TakeOff)
	{
		Pool.erase(pTarget);
		if (Pool.empty())
			PhobosGlobal::Global()->RandomTriggerPool.erase(PoolID);
	}
	return true;
}

bool TActionExt::RandomTriggerRemove(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	int PoolID = pThis->Param3;
	TriggerTypeClass* pTriggerType = pThis->TriggerType;
	TriggerClass* pTarget = TriggerClass::GetInstance(pTriggerType);
	auto& Poolmap = PhobosGlobal::Global()->RandomTriggerPool;
	if (!Poolmap.count(PoolID))
		return true;
	auto& Pool = Poolmap[PoolID];
	if (!Pool.count(pTarget))
		return true;
	Pool.erase(pTarget);
	return true;
}

bool TActionExt::ScoreCampaignText(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	if (pThis->Param3 == 0)
		ScenarioExt::Global()->ParTitle = pThis->Text;
	else
		ScenarioExt::Global()->ParMessage = pThis->Text;
	return true;
}

bool TActionExt::ScoreCampaignTheme(TActionClass* pThis, HouseClass* pHouse, ObjectClass* pObject, TriggerClass* pTrigger, CellStruct const& location)
{
	ScenarioExt::Global()->ScoreCampaignTheme = pThis->Text;
	return true;
}


// =============================
// container

TActionExt::ExtContainer::ExtContainer() : Container("TActionClass") { }

TActionExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

#ifdef MAKE_GAME_SLOWER_FOR_NO_REASON
DEFINE_HOOK(0x6DD176, TActionClass_CTOR, 0x5)
{
	GET(TActionClass*, pItem, ESI);

	TActionExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(0x6E4761, TActionClass_SDDTOR, 0x6)
{
	GET(TActionClass*, pItem, ESI);

	TActionExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x6E3E30, TActionClass_SaveLoad_Prefix, 0x8)
DEFINE_HOOK(0x6E3DB0, TActionClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(TActionClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	TActionExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x6E3E29, TActionClass_Load_Suffix, 0x4)
{
	TActionExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x6E3E4A, TActionClass_Save_Suffix, 0x3)
{
	TActionExt::ExtMap.SaveStatic();
	return 0;
}
#endif
