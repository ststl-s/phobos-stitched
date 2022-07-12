#pragma once

#include <Utilities/Template.h>
#include <Utilities/Enumerable.h>

class AttachEffectTypeClass : public Enumerable<AttachEffectTypeClass>
{
public:

	static const char* GetMainSection();

	Valueable<int> FirePower;
	Valueable<int> ROF;
	Valueable<int> Armor;
	Valueable<int> Speed;
	Valueable<double> FirePower_Multiplier;
	Valueable<double> ROF_Multiplier;
	Valueable<double> Armor_Multiplier;
	Valueable<double> Speed_Multiplier;
	Valueable<bool> DisableWeapon;
	Valueable<bool> DisableTurn;
	Nullable<AnimTypeClass*> Anim;
	ValueableVector<WeaponTypeClass*> WeaponList;
	ValueableVector<WeaponTypeClass*> AttackedWeaponList;
	Valueable<bool> Cumulative;
	Valueable<bool> ResetIfExist;
	Valueable<int> Loop_Delay;
	Nullable<int> Loop_Duration;

	AttachEffectTypeClass(const char* pTitle = NONE_STR) : Enumerable<AttachEffectTypeClass>(pTitle)
		, FirePower(0)
		, ROF(0)
		, Armor(0)
		, Speed(0)
		, FirePower_Multiplier(1)
		, ROF_Multiplier(1)
		, Armor_Multiplier(1)
		, Speed_Multiplier(1)
		, DisableWeapon(false)
		, DisableTurn(false)
		, Anim()
		, WeaponList()
		, AttackedWeaponList()
		, Cumulative(false)
		, ResetIfExist(true)
		, Loop_Delay(0)
		, Loop_Duration()
	{ }

	virtual ~AttachEffectTypeClass() = default;

	virtual void LoadFromINI(CCINIClass* pINI);
	virtual void LoadFromStream(PhobosStreamReader& stm);
	virtual void SaveToStream(PhobosStreamWriter& stm);

private:

	template <typename T>
	void Serialize(T& stm);
};
