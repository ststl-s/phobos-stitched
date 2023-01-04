#pragma once

#include <Utilities/Constructs.h>
#include <Utilities/Enum.h>
#include <Utilities/Template.h>

class InterceptorTypeClass
{
public:

	InterceptorTypeClass() = default;

	InterceptorTypeClass(TechnoTypeClass* pOwnedType);

	TechnoTypeClass* OwnedType;

	Nullable<bool> Rookie;
	Nullable<bool> Veteran;
	Nullable<bool> Elite;
	Valueable<AffectedHouse> CanTargetHouses;
	Promotable<Leptons> GuardRange;
	Promotable<Leptons> MinimumGuardRange;
	Valueable<int> Weapon;
	Nullable<bool> DeleteOnIntercept;
	Nullable<WeaponTypeClass*> WeaponOverride;
	Valueable<bool> WeaponReplaceProjectile;
	Valueable<bool> WeaponCumulativeDamage;
	Valueable<bool> KeepIntact;
	Valueable<int> Success;
	Valueable<int> RookieSuccess;
	Valueable<int> VeteranSuccess;
	Valueable<int> EliteSuccess;
	Nullable<bool> InterfereOnIntercept;

	void LoadFromINI(CCINIClass* pINI, const char* pSection);
	bool Load(PhobosStreamReader& stm, bool registerForChange);
	bool Save(PhobosStreamWriter& stm) const;

private:

	template <typename T>
	bool Serialize(T& stm);
};
