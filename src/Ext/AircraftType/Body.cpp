#include "Body.h"

#include <Helpers/Macro.h>

#include <Utilities/TemplateDef.h>

AircraftTypeExt::ExtContainer AircraftTypeExt::ExtMap;

void AircraftTypeExt::ExtData::LoadFromINIFile(CCINIClass* const pINI)
{
	auto pThis = this->OwnerObject();
	const char* pSection = pThis->ID;
	//const char* pArtSection = pThis->ImageFile;
	auto pArtINI = &CCINIClass::INI_Art();

	INI_EX exINI(pINI);
	INI_EX exArtINI(pArtINI);

	//by 俊哥
	this->Attack_OnUnit.Read(exINI, pSection, "Attack.OnUnit");

	this->Fighter_AreaGuard.Read(exINI, pSection, "Fighter.AreaGuard");
	if (this->Fighter_AreaGuard.Get())
	{
		this->Fighter_GuardRange.Read(exINI, pSection, "Fighter.GuardRange");
		this->Fighter_AutoFire.Read(exINI, pSection, "Fighter.AutoFire");
		this->Fighter_Ammo.Read(exINI, pSection, "Fighter.Ammo");
		this->Fighter_GuardRadius.Read(exINI, pSection, "Fighter.GuardRadius");
		this->Fighter_FindRangeAroundSelf.Read(exINI, pSection, "Fighter.FindRangeAroundSelf");
		this->Fighter_ChaseRange.Read(exINI, pSection, "Fighter.ChaseRange");
		this->Fighter_CanAirToAir.Read(exINI, pSection, "Fighter.CanAirToAir");
	}
}

template <typename T>
void AircraftTypeExt::ExtData::Serialize(T& Stm)
{
	Stm
		//by 俊哥
		.Process(this->Attack_OnUnit)
		.Process(this->Fighter_AreaGuard)
		.Process(this->Fighter_GuardRange)
		.Process(this->Fighter_AutoFire)
		.Process(this->Fighter_Ammo)
		.Process(this->Fighter_GuardRadius)
		.Process(this->Fighter_FindRangeAroundSelf)
		.Process(this->Fighter_ChaseRange)
		.Process(this->Fighter_CanAirToAir)
		;
}

void AircraftTypeExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<AircraftTypeClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void AircraftTypeExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<AircraftTypeClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool AircraftTypeExt::ExtContainer::Load(AircraftTypeClass* pThis, IStream* pStm)
{
	AircraftTypeExt::ExtData* pData = this->LoadKey(pThis, pStm);

	return pData != nullptr;
};

bool AircraftTypeExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm.Success();
}

bool AircraftTypeExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm.Success();
}

// =============================
// container

AircraftTypeExt::ExtContainer::ExtContainer() : Container("AircraftTypeClass") { }

AircraftTypeExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x41C9E1, AircraftTypeClass_CTOR, 0x7)
{
	GET(AircraftTypeClass*, pItem, ESI);

	AircraftTypeExt::ExtMap.TryAllocate(pItem);
	return 0;
}

DEFINE_HOOK(0x41D006, AircraftTypeClass_DTOR, 0x6)
{
	GET(AircraftTypeClass*, pItem, ESI);

	AircraftTypeExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x41CE20, AircraftTypeClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x41CE90, AircraftTypeClass_SaveLoad_Prefix, 0x8)
{
	GET_STACK(AircraftTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	AircraftTypeExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x41CE80, AircraftTypeClass_Load_Suffix, 0x5)
{
	AircraftTypeExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x41CEAA, AircraftTypeClass_Save_Suffix, 0x6)
{
	AircraftTypeExt::ExtMap.SaveStatic();
	return 0;
}

DEFINE_HOOK_AGAIN(0x41CD8A, AircraftTypeClass_LoadFromINI, 0xA)
DEFINE_HOOK(0x41CD97, AircraftTypeClass_LoadFromINI, 0xA)
{
	GET(AircraftTypeClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, 0x0);

	AircraftTypeExt::ExtMap.LoadFromINI(pItem, pINI);
	return 0;
}
