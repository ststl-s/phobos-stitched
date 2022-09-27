#include "EBolt.h"

#include <Helpers/Macro.h>

#include <Utilities/TemplateDef.h>
#include <Utilities/SavegameDef.h>

template<> const DWORD Extension<EBoltExt>::Canary = 0xBCBCBCBC;
EBoltExt::ExtContainer EBoltExt::ExtMap;

template <typename T>
void EBoltExt::ExtData::Serialize(T& stm)
{
	stm
		.Process(this->Color1)
		.Process(this->Color2)
		.Process(this->Color3)
		.Process(this->Disable)
		;
}

void EBoltExt::ExtData::LoadFromStream(PhobosStreamReader& stm)
{
	Extension<EBolt>::LoadFromStream(stm);
	this->Serialize(stm);
}

void EBoltExt::ExtData::SaveToStream(PhobosStreamWriter& stm)
{
	Extension<EBolt>::SaveToStream(stm);
	this->Serialize(stm);
}

EBoltExt::ExtContainer::ExtContainer() : Container("EBolt") { }
EBoltExt::ExtContainer::~ExtContainer() = default;

DEFINE_HOOK(0x4C1E45, EBolt_CTOR, 0x5)
{
	GET(EBolt*, pItem, EAX);

	EBoltExt::ExtMap.FindOrAllocate(pItem);

	return 0;
}

DEFINE_HOOK(0x4C2C10, EBolt_DTOR, 0x5)
{
	GET(EBolt*, pItem, ECX);

	EBoltExt::ExtMap.Remove(pItem);
}

