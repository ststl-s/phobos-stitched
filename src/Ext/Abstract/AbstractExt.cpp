#include "AbstractExt.h"

#include <FootClass.h>

#include <Helpers/Macro.h>

AbstractExt::ExtContainer AbstractExt::ExtMap;

template <typename T>
void AbstractExt::ExtData::Serialize(T& stm)
{

}

void AbstractExt::ExtData::LoadFromStream(PhobosStreamReader& stm)
{
	Extension<AbstractClass>::LoadFromStream(stm);
	this->Serialize(stm);
}

void AbstractExt::ExtData::SaveToStream(PhobosStreamWriter& stm)
{
	Extension<AbstractClass>::SaveToStream(stm);
	this->Serialize(stm);
}

bool AbstractExt::LoadGlobals(PhobosStreamReader& stm)
{
	return stm
		.Success();
}

bool AbstractExt::SaveGlobals(PhobosStreamWriter& stm)
{
	return stm
		.Success();
}

// =============================
// container

AbstractExt::ExtContainer::ExtContainer() : Container("AbstractClass") { }

AbstractExt::ExtContainer::~ExtContainer() = default;

DEFINE_HOOK(0x4101B6, AbstractClass_CTOR, 0x5)
{
	GET(AbstractClass*, pItem, EAX);

	AbstractExt::ExtMap.ForceFindOrAllocate(pItem);

	return 0;
}

DEFINE_HOOK(0x4101F0, AbstractClass_DTOR, 0x6)
{
	GET(AbstractClass*, pItem, ECX);

	AbstractExt::ExtMap.Remove(pItem);

	return 0;
}
