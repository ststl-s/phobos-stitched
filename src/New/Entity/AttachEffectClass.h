#pragma once
#include <New/Type/AttachEffectTypeClass.h>

class AttachEffectClass
{
public:

	AttachEffectTypeClass* Type;
	TimerStruct Timer;

	AttachEffectClass(AttachEffectClass& other) = delete;
	AttachEffectClass() :Type(nullptr), Timer(0) { }
	AttachEffectClass(AttachEffectTypeClass* type, int duration) :Type(type), Timer(duration)
	{ }

	bool Load(PhobosStreamReader& stm);
	bool Save(PhobosStreamWriter& stm);

private:

	template <typename T>
	bool Serialize(T& stm);
};
