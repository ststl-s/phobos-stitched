#pragma once

#include <Utilities/Template.h>
#include <Utilities/Enumerable.h>

class AttachEffectTypeClass : public Enumerable<AttachEffectTypeClass>
{
public:

	Valueable<int> FirePower;
	Valueable<int> ROF;
	Valueable<int> Armor;
	Valueable<int> Speed;
	Valueable<int> ROT;
	Valueable<double> FirePower_Multiplier;
	Valueable<double> ROF_Multiplier;
	Valueable<double> Armor_Multiplier;
	Valueable<double> Speed_Multiplier;
	Valueable<double> ROT_Multiplier;
	Valueable<bool> DisableWeapon;
	Valueable<bool> Cloak;
	Valueable<bool> Decloak;
	Nullable<AnimTypeClass*> Anim;
	Nullable<AnimTypeClass*> EndedAnim;
	ValueableVector<WeaponTypeClass*> WeaponList;
	ValueableVector<WeaponTypeClass*> AttackedWeaponList;
	Valueable<int> Loop_Delay;
	Nullable<int> Loop_Duration;
	Valueable<bool> WeaponList_FireOnAttach;
	Valueable<bool> PenetratesIronCurtain;
	Valueable<bool> DiscardOnEntry;
	Valueable<bool> Cumulative;
	Valueable<bool> ResetIfExist_Timer;
	Valueable<bool> ResetIfExist_Anim;
	Valueable<bool> ShowAnim_Cloaked;

	AttachEffectTypeClass(const char* pTitle = NONE_STR) : Enumerable<AttachEffectTypeClass>(pTitle)
		, FirePower(0)
		, ROF(0)
		, Armor(0)
		, Speed(0)
		, ROT(0)
		, FirePower_Multiplier(1.0)
		, ROF_Multiplier(1.0)
		, Armor_Multiplier(1.0)
		, Speed_Multiplier(1.0)
		, ROT_Multiplier(1.0)
		, DisableWeapon(false)
		, Cloak(false)
		, Decloak(false)
		, Anim()
		, EndedAnim()
		, WeaponList()
		, WeaponList_FireOnAttach(false)
		, AttackedWeaponList()
		, PenetratesIronCurtain(false)
		, DiscardOnEntry(false)
		, Cumulative(false)
		, ResetIfExist_Timer(true)
		, ResetIfExist_Anim(false)
		, Loop_Delay(0)
		, Loop_Duration()
		, ShowAnim_Cloaked(false)
	{ }

	virtual ~AttachEffectTypeClass() = default;

	virtual void LoadFromINI(CCINIClass* pINI);
	virtual void LoadFromStream(PhobosStreamReader& stm);
	virtual void SaveToStream(PhobosStreamWriter& stm);

private:

	template <typename T>
	void Serialize(T& stm);
};
