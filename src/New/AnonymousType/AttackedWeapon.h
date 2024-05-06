#pragma once

#include <SpecificStructures.h>

#include <Utilities/Constructs.h>
#include <Utilities/Enum.h>
#include <Utilities/Template.h>

class AttackedWeaponTypeClass
{
public:
	ValueableVector<WeaponTypeClass*> AttackedWeapon;
	ValueableVector<WeaponTypeClass*> AttackedWeapon_Veteran;
	ValueableVector<WeaponTypeClass*> AttackedWeapon_Elite;
	ValueableVector<int> AttackedWeapon_ROF;
	ValueableVector<bool> AttackedWeapon_FireToAttacker;
	ValueableVector<bool> AttackedWeapon_IgnoreROF;
	ValueableVector<bool> AttackedWeapon_IgnoreRange;
	ValueableVector<Leptons> AttackedWeapon_Range;
	ValueableVector<WarheadTypeClass*> AttackedWeapon_ResponseWarhead;
	ValueableVector<WarheadTypeClass*> AttackedWeapon_NoResponseWarhead;
	ValueableVector<bool> AttackedWeapon_ResponseZeroDamage;
	std::vector<AffectedHouse> AttackedWeapon_ResponseHouse;
	ValueableVector<int> AttackedWeapon_ActiveMaxHealth;
	ValueableVector<int> AttackedWeapon_ActiveMinHealth;
	std::vector<CoordStruct> AttackedWeapon_FLHs;

	AttackedWeaponTypeClass();

	void LoadFromINI(CCINIClass* pINI, const char* pSection);
	bool ValidAttackedWeapon() const;

	bool Load(PhobosStreamReader& stm, bool registerForChange);
	bool Save(PhobosStreamWriter& stm) const;

	void ProcessAttackedWeapon(TechnoClass* pTechno, args_ReceiveDamage* args, bool beforeDamage) const;

private:
	template <typename T>
	bool Serialize(T& Stm);
};
