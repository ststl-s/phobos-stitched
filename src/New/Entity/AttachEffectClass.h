#pragma once
#include <New/Type/AttachEffectTypeClass.h>

class AttachEffectClass
{
public:

	AttachEffectTypeClass* Type;
	TimerStruct Timer;
	RepeatableTimerStruct Loop_Timer;
	RepeatableTimerStruct Delay_Timer;
	std::vector<AnimClass*> Anims;
	std::vector<RepeatableTimerStruct> WeaponTimers;
	std::vector<RepeatableTimerStruct> AttackedWeaponTimers;

	AttachEffectClass(AttachEffectClass& other) = delete;
	AttachEffectClass() = default;
	AttachEffectClass(AttachEffectTypeClass* pType, int duration, int delay, CoordStruct& crdLoc) :Type(pType), Timer(duration), Delay_Timer(delay)
	{
		Init(crdLoc);
	}

	~AttachEffectClass() = default;

	void Init(const CoordStruct& crdLoc);
	void Update(const CoordStruct& crdLoc);

	bool Load(PhobosStreamReader& stm);
	bool Save(PhobosStreamWriter& stm);

private:

	template <typename T>
	bool Serialize(T& stm);
};
