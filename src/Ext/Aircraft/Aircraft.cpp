#include "Aircraft.h"

#include <Ext/Scenario/Body.h>
#include <Ext/WeaponType/Body.h>

#include <Utilities/TemplateDef.h>

// TODO: Implement proper extended AircraftClass.

template<> const DWORD Extension<AircraftClass>::Canary = 0x3939618A;
AircraftExt::ExtContainer AircraftExt::ExtMap;

void AircraftExt::FireBurst(AircraftClass* pThis, AbstractClass* pTarget, int shotNumber = 0)
{
	if (!pThis)
		return;

	int weaponIndex = pThis->SelectWeapon(pTarget);

	if (!pThis->GetWeapon(weaponIndex))
		return;

	auto weaponType = pThis->GetWeapon(weaponIndex)->WeaponType;
	if (!weaponType)
		return;

	auto pWeaponTypeExt = WeaponTypeExt::ExtMap.Find(weaponType);
	if (!pWeaponTypeExt)
		return;

	if (weaponType->Burst > 0)
	{
		for (int i = 0; i < weaponType->Burst; i++)
		{
			if (weaponType->Burst < 2 && pWeaponTypeExt->Strafing_SimulateBurst)
				pThis->CurrentBurstIndex = shotNumber;

			pThis->Fire(pThis->Target, weaponIndex);
		}
	}
}

// =============================
// load / save

template <typename T>
void AircraftExt::ExtData::Serialize(T& Stm)
{
	Stm
		;
}

void AircraftExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<AircraftClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void AircraftExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<AircraftClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool AircraftExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Success();
}

bool AircraftExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Success();
}

// =============================
// container

AircraftExt::ExtContainer::ExtContainer() : Container("AircraftClass") { }

AircraftExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

//DEFINE_HOOK(0x413F6A, AircraftClass_CTOR, 0x7)
//{
//	GET(AircraftClass*, pItem, ESI);
//
//	AircraftExt::ExtMap.FindOrAllocate(pItem);
//
//	return 0;
//}
//
//DEFINE_HOOK(0x41426F, AircraftClass_DTOR, 0x7)
//{
//	GET(AircraftClass*, pItem, EDI);
//
//	AircraftExt::ExtMap.Remove(pItem);
//
//	return 0;
//}
//
//DEFINE_HOOK_AGAIN(0x41B430, AircraftClass_SaveLoad_Prefix, 0x6)
//DEFINE_HOOK(0x41B5C0, AircraftClass_SaveLoad_Prefix, 0x8)
//{
//	GET_STACK(AircraftClass*, pItem, 0x4);
//	GET_STACK(IStream*, pStm, 0x8);
//
//	AircraftExt::ExtMap.PrepareStream(pItem, pStm);
//
//	return 0;
//}
//
//DEFINE_HOOK(0x41B5B5, AircraftClass_Load_Suffix, 0x6)
//{
//	AircraftExt::ExtMap.LoadStatic();
//
//	return 0;
//}
//
//DEFINE_HOOK(0x41B5D4, AircraftClass_Save_Suffix, 0x5)
//{
//	AircraftExt::ExtMap.SaveStatic();
//
//	return 0;
//}
