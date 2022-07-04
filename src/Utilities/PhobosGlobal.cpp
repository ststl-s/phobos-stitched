#include "PhobosGlobal.h"
#include <Ext/WarheadType/Body.h>
#include <Ext/SWType/Body.h>

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
		.Success();
}

template <typename T>
bool PhobosGlobal::SerializeGlobal(T& stm)
{
	ProcessTechnoType(stm);
	ProcessTechno(stm);
	ProcessWarhead(stm);
	return stm.Success();
}

template <typename T>
bool Process(T& stm, TechnoTypeClass* pItem)
{
	TechnoTypeExt::ExtData* pExt = TechnoTypeExt::ExtMap.Find(pItem);
	stm
		.Process(pExt->AttachmentData)
		.Process(pExt->GiftBoxData.GiftBox_Types)
		.Process(pExt->WeaponRangeShare_Techno)
		.Process(pExt->BuildLimit_Group_Types)
		.Process(pExt->BuildLimit_As)
		;
	return stm.Success();
}

template <typename T>
bool Process(T& stm, TechnoClass* pItem)
{
	TechnoExt::ExtData* pExt = TechnoExt::ExtMap.Find(pItem);
	stm
		.Process(pExt->AttachedGiftBox)
		;
	return stm.Success();
}

template <typename T>
bool PhobosGlobal::ProcessTechnoType(T& stm)
{
	for (int i = 0; i < UnitTypeClass::Array->Count; i++)
	{
		Process(stm, UnitTypeClass::Array->GetItem(i));
	}
	for (int i = 0; i < InfantryTypeClass::Array->Count; i++)
	{
		Process(stm, InfantryTypeClass::Array->GetItem(i));
	}
	for (int i = 0; i < AircraftTypeClass::Array->Count; i++)
	{
		Process(stm, AircraftTypeClass::Array->GetItem(i));
	}
	for (int i = 0; i < BuildingTypeClass::Array->Count; i++)
	{
		Process(stm, BuildingTypeClass::Array->GetItem(i));
	}
	return stm.Success();
}

template <typename T>
bool PhobosGlobal::ProcessTechno(T& stm)
{
	for (int i = 0; i < UnitClass::Array->Count; i++)
	{
		Process(stm, UnitClass::Array->GetItem(i));
	}
	for (int i = 0; i < InfantryClass::Array->Count; i++)
	{
		Process(stm, InfantryClass::Array->GetItem(i));
	}
	for (int i = 0; i < AircraftClass::Array->Count; i++)
	{
		Process(stm, AircraftClass::Array->GetItem(i));
	}
	for (int i = 0; i < BuildingClass::Array->Count; i++)
	{
		Process(stm, BuildingClass::Array->GetItem(i));
	}
	return stm.Success();
}

template <typename T>
bool PhobosGlobal::ProcessWarhead(T& stm)
{
	for (int i = 0; i < WarheadTypeClass::Array->Count; i++)
	{
		WarheadTypeClass* pItem = WarheadTypeClass::Array->GetItem(i);
		WarheadTypeExt::ExtData* pExt = WarheadTypeExt::ExtMap.Find(pItem);

		stm
			.Process(pExt->AttackedWeapon_ResponseTechno)
			.Process(pExt->AttackedWeapon_NoResponseTechno)
			.Process(pExt->ChangeOwner_Types)
			.Process(pExt->ChangeOwner_Ignore)
			.Process(pExt->AttachTag_Types)
			.Process(pExt->AttachTag_Ignore)
			;
	}
	return stm.Success();
}

template <typename T>
bool PhobosGlobal::ProcessSWType(T& stm)
{
	for (int i = 0; i < SuperWeaponTypeClass::Array->Count; i++)
	{
		SuperWeaponTypeClass* pItem = SuperWeaponTypeClass::Array->GetItem(i);
		SWTypeExt::ExtData* pExt = SWTypeExt::ExtMap.Find(pItem);

		stm
			.Process(pExt->SW_AuxTechno)
			.Process(pExt->SW_NegTechno)
			;
	}

	return stm.Success();
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
	SerializeGlobal(stm);
	GlobalObject.Save(stm);
	return stm.Success();
}

bool PhobosGlobal::LoadGlobals(PhobosStreamReader& stm)
{
	SerializeGlobal(stm);
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
