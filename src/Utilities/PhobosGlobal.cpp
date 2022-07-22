#include "PhobosGlobal.h"
#include <Ext/WarheadType/Body.h>

//GlobalObject initial
PhobosGlobal PhobosGlobal::GlobalObject;

PhobosGlobal* PhobosGlobal::Global()
{
	return &GlobalObject;
}

void PhobosGlobal::Clear()
{
	GlobalObject.Reset();
}

void PhobosGlobal::Reset()
{
	Techno_HugeBar.clear();
	RandomTriggerPool.clear();
	GenericStand = nullptr;
	MultipleSWFirer_Queued.clear();
}

void PhobosGlobal::CheckSuperQueued()
{
	std::vector<std::set<QueuedSW>::iterator> needRemove;
	for (auto it = MultipleSWFirer_Queued.begin(); it != MultipleSWFirer_Queued.end(); ++it)
	{
		if (it->Timer.Completed() && it->Super->Granted && it->Super->CanFire())
		{
			it->Super->Launch(it->MapCoords, it->IsPlayer);
		}
		else
		{
			needRemove.emplace_back(it);
		}
	}

	for (const auto& it : needRemove)
	{
		MultipleSWFirer_Queued.erase(it);
	}
}

//Save/Load
#pragma region save/load

template <typename T>
bool PhobosGlobal::Serialize(T& stm)
{
	return stm
		.Process(this->Techno_HugeBar)
		.Process(this->RandomTriggerPool)
		.Process(this->GenericStand)
		.Process(this->MultipleSWFirer_Queued)
		.Success();
}

bool PhobosGlobal::Save(PhobosStreamWriter& stm)
{
	return Serialize(stm);
}

bool PhobosGlobal::Load(PhobosStreamReader& stm)
{
	return Serialize(stm);
}

bool PhobosGlobal::SaveGlobals(PhobosStreamWriter& stm)
{
	GlobalObject.Save(stm);
	return stm.Success();
}

bool PhobosGlobal::LoadGlobals(PhobosStreamReader& stm)
{
	GlobalObject.Load(stm);
	return stm.Success();
}

#pragma endregion save/load

TechnoClass* PhobosGlobal::GetGenericStand()
{
	if (GenericStand != nullptr)
		return GenericStand;

	HouseClass* pHouse = HouseClass::FindSpecial();

	if (pHouse == nullptr)
		pHouse = HouseClass::FindNeutral();

	if (pHouse == nullptr && HouseClass::Array->Count > 0)
		pHouse = HouseClass::Array->GetItem(0);

	if (pHouse == nullptr)
		Debug::FatalErrorAndExit("House is empty!\n");

	if (UnitTypeClass::Array->Count > 0)
	{
		GenericStand = abstract_cast<TechnoClass*>(UnitTypeClass::Array->GetItem(0)->CreateObject(pHouse));
	}
	else
	{
		Debug::Log("[Warning] UnitTypes Is Empty\n");
		if (InfantryTypeClass::Array->Count > 0)
		{
			GenericStand = abstract_cast<TechnoClass*>(InfantryTypeClass::Array->GetItem(0)->CreateObject(pHouse));
		}
		else
		{
			Debug::Log("[Warning] InfantryTypes Is Empty\n");
			if (BuildingTypeClass::Array->Count > 0)
			{
				GenericStand = abstract_cast<TechnoClass*>(BuildingTypeClass::Array->GetItem(0)->CreateObject(pHouse));
			}
			else
			{
				Debug::Log("[Warning] BuildingTypes Is Empty\n");
				Debug::FatalErrorAndExit("[Error] Generic Stand Create Fail\n");
			}
		}
	}

	GenericStand->Limbo();

	return GenericStand;
}
