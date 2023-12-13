#include "ObjectExt.h"

#include <FootClass.h>

#include <Helpers/Macro.h>

ObjectExt::ExtContainer ObjectExt::ExtMap;

bool ObjectClass::IsReallyAlive() const
{
	return ObjectExt::IsReallyAlive(this);
}

bool ObjectExt::IsReallyAlive(const ObjectClass* const pObject)
{
	return pObject
		&& pObject->IsAlive
		&& pObject->Health > 0
		&& ObjectExt::ExtMap.Find(pObject) != nullptr
		;
}

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

	ObjectExt::ExtMap.TryAllocate(pItem);

	return 0;
}

DEFINE_HOOK(0x5F3B80, ObjectClass_DTOR, 0x7)
{
	GET(ObjectClass*, pItem, ECX);

	ObjectExt::ExtMap.Remove(pItem);

	return 0;
}
