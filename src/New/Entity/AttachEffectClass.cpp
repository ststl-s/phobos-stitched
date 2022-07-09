#include "AttachEffectClass.h"

template <typename T>
bool AttachEffectClass::Serialize(T& stm)
{
	stm
		.Process(this->Type)
		.Process(this->Timer)
		;

	return stm.Success();
}

bool AttachEffectClass::Load(PhobosStreamReader& stm)
{
	return Serialize(stm);
}

bool AttachEffectClass::Save(PhobosStreamWriter& stm)
{
	return Serialize(stm);
}
