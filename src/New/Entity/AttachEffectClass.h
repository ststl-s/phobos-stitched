#pragma once
#include <AnimClass.h>

#include <New/Type/AttachEffectTypeClass.h>

class AttachEffectClass
{
	struct UninitAnim
	{
		void operator() (AnimClass* const pAnim) const
		{
			pAnim->SetOwnerObject(nullptr);
			pAnim->UnInit();
		}
	};

public:

	AttachEffectTypeClass* Type;
	TechnoClass* Owner;
	TechnoClass* AttachOwner;
	CDTimerClass Timer;
	RateTimer Loop_Timer;
	RateTimer Delay_Timer;
	std::unique_ptr<AnimClass, UninitAnim> Anim;
	std::vector<RateTimer> WeaponTimers;
	std::vector<RateTimer> AttackedWeaponTimers;
	int Duration;
	bool Initialized;
	bool InLoopDelay;
	bool InCloak;

	AttachEffectClass(AttachEffectClass& other) = delete;
	AttachEffectClass() = default;
	AttachEffectClass(AttachEffectTypeClass* pType, TechnoClass* pOwner, TechnoClass* pTarget, int duration, int delay);

	~AttachEffectClass();

	void Init();
	void Update();
	void AttachOwnerAttackedBy(TechnoClass* pAttacker);
	void CreateAnim();
	void KillAnim();

	bool Load(PhobosStreamReader& stm);
	bool Save(PhobosStreamWriter& stm);

private:

	template <typename T>
	bool Serialize(T& stm);
};
