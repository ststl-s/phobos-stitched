#include "Body.h"

template<> const DWORD Extension<AircraftTypeClass>::Canary = 0x39396B1A;
AircraftTypeExt::ExtContainer AircraftTypeExt::ExtMap;

void AircraftTypeExt::ExtData::LoadFromINIFile(CCINIClass* const pINI)
{
	auto pThis = this->OwnerObject();
	const char* pSection = pThis->ID;
	const char* pArtSection = pThis->ImageFile;
	auto pArtINI = &CCINIClass::INI_Art();

	if (!pINI->GetSection(pSection))
		return;

	INI_EX exINI(pINI);
	INI_EX exArtINI(pArtINI);
	UNREFERENCED_PARAMETER(pArtSection);
	
	this->Fire_KickOutPassenger.Read(exINI, pSection, "Fire.KickOutPassenger");
}

template <typename T>
void AircraftTypeExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Fire_KickOutPassenger)
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

	return Stm
		.Success();
}

bool AircraftTypeExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Success();
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

	AircraftTypeExt::ExtMap.FindOrAllocate(pItem);
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
