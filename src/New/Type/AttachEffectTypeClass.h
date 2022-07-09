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
	Valueable<double> FirePower_Multiplier;
	Valueable<double> ROF_Multiplier;
	Valueable<double> Armor_Multiplier;
	Valueable<double> Speed_Multiplier;
	Valueable<bool> DisableWeapon;
	Valueable<bool> DisableTurn;
	ValueableVector<AnimTypeClass*> AnimList;
	ValueableVector<WeaponTypeClass*> WeaponList;
	ValueableVector<WeaponTypeClass*> AttackedWeaponList;

	AttachEffectTypeClass(const char* pTitle = NONE_STR) : Enumerable<AttachEffectTypeClass>(pTitle)
	{ }

	virtual ~AttachEffectTypeClass() = default;

	virtual void LoadFromINI(CCINIClass* pINI);
	virtual void LoadFromStream(PhobosStreamReader& stm);
	virtual void SaveToStream(PhobosStreamWriter& stm);

private:

	template <typename T>
	void Serialize(T& stm);
};
