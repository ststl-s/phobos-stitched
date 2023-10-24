#include "EBoltExt.h"

#include <Helpers/Macro.h>

#include <Ext/WeaponType/Body.h>

#include <Utilities/TemplateDef.h>

const DWORD Extension<EBolt>::Canary = 0x7f5f4f3f;
EBoltExt::ExtContainer EBoltExt::ExtMap;

EBoltExt::ExtData::ExtData(EBolt* ownerObject)
	: Extension<EBolt>(ownerObject)
	, Weapon(nullptr)
{ }

void EBoltExt::ExtData::SetWeapon(WeaponTypeClass* pWeapon)
{
	Weapon.WeaponType = pWeapon;
}

void EBoltExt::ExtData::SetWeapon(const WeaponStruct& weapon)
{
	Weapon = weapon;
}

const WeaponStruct& EBoltExt::ExtData::GetWeapon() const
{
	return this->Weapon;
}

void EBoltExt::ExtData::LoadFromStream(PhobosStreamReader& stm)
{
	stm.Process(this->Weapon);
}

void EBoltExt::ExtData::SaveToStream(PhobosStreamWriter& stm)
{
	stm.Process(this->Weapon);
}


void EBoltExt::ExtData::InvalidatePointer(void* ptr, bool removed) { }
EBoltExt::ExtContainer::ExtContainer() : Container("EBolt") { }
EBoltExt::ExtContainer::~ExtContainer() = default;
void EBoltExt::ExtContainer::InvalidatePointer(void* ptr, bool removed) { }

DEFINE_HOOK(0x4C1E45, EBolt_CTOR, 0x5)
{
	GET(EBolt*, pItem, EAX);

	EBoltExt::ExtMap.FindOrAllocate(pItem);

	return 0;
}

DEFINE_HOOK(0x4C2951, EBolt_DTOR, 0x5)
{
	GET(EBolt*, pItem, ECX);

	EBoltExt::ExtMap.Remove(pItem);

	return 0;
}

namespace EBoltContext
{
	WeaponTypeExt::ExtData* WeaponExtData = nullptr;
}

DEFINE_HOOK(0x6FD480, TechnoClass_FireBolt_SetWeapon, 0x6)
{
	GET(EBolt*, pBolt, EAX);
	GET_STACK(WeaponTypeClass*, pWeapon, STACK_OFFSET(0x30, 0x8));

	EBoltExt::ExtData* pBoltExt = EBoltExt::ExtMap.Find(pBolt);
	pBoltExt->SetWeapon(pWeapon);

	return 0;
}

DEFINE_HOOK(0x4C1F20, EBolt_Draw_SetContext, 0x6)
{
	GET(EBolt*, pThis, ECX);

	const auto pExt = EBoltExt::ExtMap.Find(pThis);
	EBoltContext::WeaponExtData = WeaponTypeExt::ExtMap.Find(pExt->GetWeapon().WeaponType);

	return 0;
}

DEFINE_HOOK(0x4C24E4, Ebolt_Draw_Disable1, 0x8)
{
	return (EBoltContext::WeaponExtData->Bolt_Disable1) ? 0x4C2515 : 0;
}

DEFINE_HOOK(0x4C25FD, Ebolt_Draw_Disable2, 0xA)
{
	return (EBoltContext::WeaponExtData->Bolt_Disable2) ? 0x4C262A : 0;
}

DEFINE_HOOK(0x4C26EE, Ebolt_Draw_Disable3, 0x8)
{
	return (EBoltContext::WeaponExtData->Bolt_Disable3) ? 0x4C2710 : 0;
}

DEFINE_HOOK(0x4C24FC, EBolt_Draw_Color1, 0x8)
{
	R->EAX(Drawing::RGB_To_Int(
		EBoltContext::WeaponExtData->EBolt_Color1.Get(Drawing::Int_To_RGB(R->EAX()))
	));

	return 0;
}

DEFINE_HOOK(0x4C2611, EBolt_Draw_Color2, 0x8)
{
	R->EDX(Drawing::RGB_To_Int(
		EBoltContext::WeaponExtData->EBolt_Color1.Get(Drawing::Int_To_RGB(R->EDX()))
	));

	return 0;
}

DEFINE_HOOK(0x4C26FA, EBolt_Draw_Color3, 0x8)
{
	R->EAX(Drawing::RGB_To_Int(
		EBoltContext::WeaponExtData->EBolt_Color3.Get(Drawing::Int_To_RGB(R->EAX()))
	));

	return 0;
}
