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
	CDTimerClass Loop_Timer;
	CDTimerClass Delay_Timer;
	std::unique_ptr<AnimClass, UninitAnim> Anim;
	std::vector<CDTimerClass> WeaponTimers;
	std::vector<CDTimerClass> AttackedWeaponTimers;
	int Duration;
	bool Initialized;
	bool InLoopDelay;
	bool InCloak;
	bool Inlimbo;

	AttachEffectClass(AttachEffectClass& other) = delete;
	AttachEffectClass() = default;
	AttachEffectClass(AttachEffectTypeClass* pType, TechnoClass* pOwner, TechnoClass* pTarget, int duration, int delay);

	~AttachEffectClass();

	void Init();
	void Update();
	void AddAllTimers(int frames = 1);
	void AttachOwnerAttackedBy(TechnoClass* pAttacker);
	void CreateAnim();
	void KillAnim();

	bool Load(PhobosStreamReader& stm);
	bool Save(PhobosStreamWriter& stm);

private:

	template <typename T>
	bool Serialize(T& stm);
};
