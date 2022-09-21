#include "PhobosGlobal.h"
#include <Utilities/TemplateDef.h>

#include <Ext/SWType/Body.h>

//GlobalObject initial
PhobosGlobal PhobosGlobal::GlobalObject;

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
	TriggerType_HouseMultiplayer.clear();
}

void PhobosGlobal::PointerGotInvalid(void* ptr, bool removed)
{
	GlobalObject.InvalidatePointer(ptr, removed);
}

void PhobosGlobal::InvalidatePointer(void* ptr, bool removed)
{
	if (removed)
	{
		for (auto it = Techno_HugeBar.begin(); it != Techno_HugeBar.end(); ++it)
		{
			if (it->second == ptr)
			{
				Techno_HugeBar.erase(it);

				break;
			}
		}
	}
}

void PhobosGlobal::CheckSuperQueued()
{
	for (auto& item : MultipleSWFirer_Queued)
	{
		if (item.Timer.Completed())
		{
			SuperClass* pSuper = item.Super;
			const auto pSWTypeExt = SWTypeExt::ExtMap.Find(item.Super->Type);

			if (!item.RealLaunch || pSuper->Granted && pSuper->IsCharged && pSuper->Owner->CanTransactMoney(pSWTypeExt->Money_Amount))
			{
				if (!pSWTypeExt->HasInhibitor(pSuper->Owner, item.MapCoords))
				{
					pSuper->SetReadiness(true);
					pSuper->Launch(item.MapCoords, item.IsPlayer);
					pSuper->Reset();
				}
			}
		}
	}

	MultipleSWFirer_Queued.erase
	(
		std::remove_if(MultipleSWFirer_Queued.begin(), MultipleSWFirer_Queued.end(),
			[](QueuedSW& item)
			{
				return item.Timer.Expired();
			}),
		MultipleSWFirer_Queued.end()
	);
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
		.Process(this->TriggerType_HouseMultiplayer)
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

	HouseClass* pHouse = HouseClass::FindNeutral();

	if (pHouse == nullptr)
		pHouse = HouseClass::FindSpecial();

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
