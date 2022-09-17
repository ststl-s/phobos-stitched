#pragma once
#include <AnimClass.h>

#include <New/Type/AttachEffectTypeClass.h>

class AttachEffectClass
{
	struct UninitAnim
	{
		void operator () (AnimClass* const pAnim) const
		{
			if (pAnim != nullptr)
			{
				pAnim->DetachFromObject(pAnim->OwnerObject, false);
				pAnim->UnInit();
			}
		}
	};

	static std::unordered_map<int, int> AttachEffect_Exist;

public:

	AttachEffectTypeClass* Type;
	TechnoClass* Owner;
	TechnoClass* AttachOwner;
	HouseClass* OwnerHouse;
	CDTimerClass Timer;
	CDTimerClass Loop_Timer;
	CDTimerClass Delay_Timer;
	Handle<AnimClass*, UninitAnim> Anim;
	int AnimIndex;
	std::vector<CDTimerClass> WeaponTimers;
	std::vector<CDTimerClass> AttackedWeaponTimers;
	int Duration;
	bool Initialized;
	bool InLoopDelay;
	bool InCloak;
	bool Inlimbo;
	std::unordered_map<int, WeaponStruct> ReplaceWeapons_Rookie;
	std::unordered_map<int, WeaponStruct> ReplaceWeapons_Veteran;
	std::unordered_map<int, WeaponStruct> ReplaceWeapons_Elite;

	AttachEffectClass(AttachEffectClass& other) = delete;
	AttachEffectClass() = default;
	AttachEffectClass(AttachEffectTypeClass* pType, TechnoClass* pOwner, TechnoClass* pTarget, int duration, int delay);

	~AttachEffectClass();

	void Init();
	void Update();
	void AddAllTimers(int frames = 1);
	void AttachOwnerAttackedBy(TechnoClass* pAttacker);
	void ResetAnim();
	void CreateAnim();
	void KillAnim();
	bool IsActive() const;
	const WeaponStruct* GetReplaceWeapon(int weaponIdx) const;
	int GetCurrentTintColor();

	static bool CanExist(AttachEffectTypeClass* pType);

	bool Load(PhobosStreamReader& stm, bool registerForChange);
	bool Save(PhobosStreamWriter& stm) const;
	void InvalidatePointer(void* ptr, bool removed);
	static void Clear();
	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
	
private:

	template <typename T>
	bool Serialize(T& stm);
};
