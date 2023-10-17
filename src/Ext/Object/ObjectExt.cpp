#include "ObjectExt.h"

#include <FootClass.h>

#include <Helpers/Macro.h>

template<> const DWORD Extension<ObjectClass>::Canary = 0x23332333;
ObjectExt::ExtContainer ObjectExt::ExtMap;

template <typename T>
void ObjectExt::ExtData::Serialize(T& stm)
{

}

void ObjectExt::ExtData::LoadFromStream(PhobosStreamReader& stm)
{
	Extension<ObjectClass>::LoadFromStream(stm);
	this->Serialize(stm);
}

void ObjectExt::ExtData::SaveToStream(PhobosStreamWriter& stm)
{
	Extension<ObjectClass>::SaveToStream(stm);
	this->Serialize(stm);
}

bool ObjectExt::LoadGlobals(PhobosStreamReader& stm)
{
	return stm
		.Success();
}

bool ObjectExt::SaveGlobals(PhobosStreamWriter& stm)
{
	return stm
		.Success();
}

// =============================
// container

ObjectExt::ExtContainer::ExtContainer() : Container("ObjectClass") { }

ObjectExt::ExtContainer::~ExtContainer() = default;

DEFINE_HOOK(0x5F3B3C, ObjectClass_CTOR, 0x5)
{
	GET(ObjectClass*, pItem, ESI);

	ObjectExt::ExtMap.FindOrAllocate(pItem);

	return 0;
}

DEFINE_HOOK(0x5F3B80, ObjectClass_DTOR, 0x7)
{
	GET(ObjectClass*, pItem, ECX);

	ObjectExt::ExtMap.Remove(pItem);

	return 0;
}