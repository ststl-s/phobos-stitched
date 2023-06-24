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

	AttachEffectTypeClass* Type = nullptr;
	TechnoClass* Owner = nullptr;
	TechnoClass* AttachOwner = nullptr;
	HouseClass* OwnerHouse = nullptr;
	CDTimerClass Timer;
	CDTimerClass Loop_Timer;
	CDTimerClass Delay_Timer;
	AnimClass* Anim = nullptr;
	int AnimIndex = -1;
	std::vector<CDTimerClass> WeaponTimers;
	std::vector<CDTimerClass> AttackedWeaponTimers;
	int Duration = 0;
	int AttachOwnerAttackedCounter = 0;
	bool Initialized = false;
	bool InLoopDelay = false;
	bool InCloak = false;
	bool Inlimbo = false;
	std::unordered_map<int, WeaponStruct> ReplaceWeapons_Rookie;
	std::unordered_map<int, WeaponStruct> ReplaceWeapons_Veteran;
	std::unordered_map<int, WeaponStruct> ReplaceWeapons_Elite;
	bool IsInvalid = false;
	bool IsGranted = false;
	std::vector<CDTimerClass> FireOnOwner_Timers;
	std::vector<CDTimerClass> OwnerFireOn_Timers;
	AbstractClass* Source = nullptr;

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
	bool IsFromSource(TechnoClass* pInvoker, AbstractClass* pSource) const;

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
