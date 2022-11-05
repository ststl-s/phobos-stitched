#include "InterceptorTypeClass.h"

#include <Utilities/SavegameDef.h>
#include <Utilities/TemplateDef.h>

InterceptorTypeClass::InterceptorTypeClass(TechnoTypeClass* pOwnedType)
	: OwnedType(pOwnedType)
	, Interceptor_CanTargetHouses { AffectedHouse::Enemies }
	, Interceptor_Rookie {}
	, Interceptor_Veteran {}
	, Interceptor_Elite {}
	, Interceptor_GuardRange {}
	, Interceptor_MinimumGuardRange {}
	, Interceptor_Weapon { 0 }
	, Interceptor_DeleteOnIntercept {}
	, Interceptor_WeaponOverride {}
	, Interceptor_WeaponReplaceProjectile { false }
	, Interceptor_WeaponCumulativeDamage { false }
	, Interceptor_KeepIntact { false }
	, Interceptor_Success { 100 }
	, Interceptor_RookieSuccess { -1 }
	, Interceptor_VeteranSuccess { -1 }
	, Interceptor_EliteSuccess { -1 }
{ }

void InterceptorTypeClass::LoadFromINI(CCINIClass* pINI, const char* pSection)
{
	INI_EX exINI(pINI);

	this->Interceptor_Rookie.Read(exINI, pSection, "Interceptor.Rookie");
	this->Interceptor_Veteran.Read(exINI, pSection, "Interceptor.Veteran");
	this->Interceptor_Elite.Read(exINI, pSection, "Interceptor.Elite");
	this->Interceptor_CanTargetHouses.Read(exINI, pSection, "Interceptor.CanTargetHouses");
	this->Interceptor_GuardRange.Read(exINI, pSection, "Interceptor.%sGuardRange");
	this->Interceptor_MinimumGuardRange.Read(exINI, pSection, "Interceptor.%sMinimumGuardRange");
	this->Interceptor_Weapon.Read(exINI, pSection, "Interceptor.Weapon");
	this->Interceptor_DeleteOnIntercept.Read(exINI, pSection, "Interceptor.DeleteOnIntercept");
	this->Interceptor_WeaponOverride.Read(exINI, pSection, "Interceptor.WeaponOverride");
	this->Interceptor_WeaponReplaceProjectile.Read(exINI, pSection, "Interceptor.WeaponReplaceProjectile");
	this->Interceptor_WeaponCumulativeDamage.Read(exINI, pSection, "Interceptor.WeaponCumulativeDamage");
	this->Interceptor_KeepIntact.Read(exINI, pSection, "Interceptor.KeepIntact");
	this->Interceptor_Success.Read(exINI, pSection, "Interceptor.Success");
	this->Interceptor_RookieSuccess.Read(exINI, pSection, "Interceptor.RookieSuccess");
	this->Interceptor_VeteranSuccess.Read(exINI, pSection, "Interceptor.VeteranSuccess");
	this->Interceptor_EliteSuccess.Read(exINI, pSection, "Interceptor.EliteSuccess");
}

#pragma region(save/load)

template <class T>
bool InterceptorTypeClass::Serialize(T& stm)
{
	return stm
		.Process(this->OwnedType)
		.Process(this->Interceptor_CanTargetHouses)
		.Process(this->Interceptor_Rookie)
		.Process(this->Interceptor_Veteran)
		.Process(this->Interceptor_Elite)
		.Process(this->Interceptor_GuardRange)
		.Process(this->Interceptor_MinimumGuardRange)
		.Process(this->Interceptor_Weapon)
		.Process(this->Interceptor_DeleteOnIntercept)
		.Process(this->Interceptor_WeaponOverride)
		.Process(this->Interceptor_WeaponReplaceProjectile)
		.Process(this->Interceptor_WeaponCumulativeDamage)
		.Process(this->Interceptor_KeepIntact)
		.Process(this->Interceptor_Success)
		.Process(this->Interceptor_RookieSuccess)
		.Process(this->Interceptor_VeteranSuccess)
		.Process(this->Interceptor_EliteSuccess)
		.Success();
}

bool InterceptorTypeClass::Load(PhobosStreamReader& stm, bool registerForChange)
{
	return this->Serialize(stm);
}

bool InterceptorTypeClass::Save(PhobosStreamWriter& stm) const
{
	return const_cast<InterceptorTypeClass*>(this)->Serialize(stm);
}

#pragma endregion(save/load)
