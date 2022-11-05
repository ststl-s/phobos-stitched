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

	Nullable<bool> Interceptor_Rookie;
	Nullable<bool> Interceptor_Veteran;
	Nullable<bool> Interceptor_Elite;
	Valueable<AffectedHouse> Interceptor_CanTargetHouses;
	Promotable<Leptons> Interceptor_GuardRange;
	Promotable<Leptons> Interceptor_MinimumGuardRange;
	Valueable<int> Interceptor_Weapon;
	Nullable<bool> Interceptor_DeleteOnIntercept;
	Nullable<WeaponTypeClass*> Interceptor_WeaponOverride;
	Valueable<bool> Interceptor_WeaponReplaceProjectile;
	Valueable<bool> Interceptor_WeaponCumulativeDamage;
	Valueable<bool> Interceptor_KeepIntact;
	Valueable<int> Interceptor_Success;
	Valueable<int> Interceptor_RookieSuccess;
	Valueable<int> Interceptor_VeteranSuccess;
	Valueable<int> Interceptor_EliteSuccess;

	void LoadFromINI(CCINIClass* pINI, const char* pSection);
	bool Load(PhobosStreamReader& stm, bool registerForChange);
	bool Save(PhobosStreamWriter& stm) const;

private:

	template <typename T>
	bool Serialize(T& stm);
};
