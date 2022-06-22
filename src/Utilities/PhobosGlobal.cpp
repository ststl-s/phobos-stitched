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
}

//Save/Load
#pragma region save/load

template <typename T>
bool PhobosGlobal::Serialize(T& stm)
{
	return stm
		.Process(this->Techno_HugeBar)
		.Process(this->RandomTriggerPool)
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
	stm.
		Process(pExt->AttachmentData)
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
	//for (int i = 0; i < TechnoClass::Array->Count; i++)
	//{
	//	TechnoClass* pItem = TechnoClass::Array->GetItem(i);
	//	TechnoExt::ExtData* pExt = TechnoExt::ExtMap.Find(pItem);
	//	{// Process region
	//		stm
	//			//.Process(pExt->ParentAttachment)
	//			//.Process(pExt->ChildAttachments)
	//			;
	//	}
	//}
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