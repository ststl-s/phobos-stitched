#include "InterceptorTypeClass.h"

#include <Utilities/TemplateDef.h>

InterceptorTypeClass::InterceptorTypeClass(TechnoTypeClass* pOwnedType)
	: OwnedType(pOwnedType)
	, CanTargetHouses { AffectedHouse::Enemies }
	, Rookie {}
	, Veteran {}
	, Elite {}
	, GuardRange {}
	, MinimumGuardRange {}
	, Weapon { 0 }
	, WeaponType { nullptr }
	, UseStageWeapon { false }
	, DeleteOnIntercept {}
	, WeaponOverride {}
	, WeaponReplaceProjectile { false }
	, WeaponCumulativeDamage { false }
	, KeepIntact { false }
	, Success { 100 }
	, RookieSuccess { -1 }
	, VeteranSuccess { -1 }
	, EliteSuccess { -1 }
	, InterfereOnIntercept {}
	, InterfereToSource { false }
	, InterfereToSelf { false }
{ }

void InterceptorTypeClass::LoadFromINI(CCINIClass* pINI, const char* pSection)
{
	INI_EX exINI(pINI);

	this->Rookie.Read(exINI, pSection, "Interceptor.Rookie");
	this->Veteran.Read(exINI, pSection, "Interceptor.Veteran");
	this->Elite.Read(exINI, pSection, "Interceptor.Elite");
	this->CanTargetHouses.Read(exINI, pSection, "Interceptor.CanTargetHouses");
	this->GuardRange.Read(exINI, pSection, "Interceptor.%sGuardRange");
	this->MinimumGuardRange.Read(exINI, pSection, "Interceptor.%sMinimumGuardRange");
	this->Weapon.Read(exINI, pSection, "Interceptor.Weapon");
	this->UseStageWeapon.Read(exINI, pSection, "Interceptor.UseStageWeapon");
	this->DeleteOnIntercept.Read(exINI, pSection, "Interceptor.DeleteOnIntercept");
	this->WeaponOverride.Read(exINI, pSection, "Interceptor.WeaponOverride");
	this->WeaponReplaceProjectile.Read(exINI, pSection, "Interceptor.WeaponReplaceProjectile");
	this->WeaponCumulativeDamage.Read(exINI, pSection, "Interceptor.WeaponCumulativeDamage");
	this->KeepIntact.Read(exINI, pSection, "Interceptor.KeepIntact");
	this->Success.Read(exINI, pSection, "Interceptor.Success");
	this->RookieSuccess.Read(exINI, pSection, "Interceptor.RookieSuccess");
	this->VeteranSuccess.Read(exINI, pSection, "Interceptor.VeteranSuccess");
	this->EliteSuccess.Read(exINI, pSection, "Interceptor.EliteSuccess");
	this->InterfereOnIntercept.Read(exINI, pSection, "Interceptor.InterfereOnIntercept");
	this->InterfereToSource.Read(exINI, pSection, "Interceptor.InterfereToSource");
	this->InterfereToSelf.Read(exINI, pSection, "Interceptor.InterfereToSelf");

	{
		Nullable<WeaponTypeClass*> weapon;
		weapon.Read(exINI, pSection, "Interceptor.WeaponType", true);

		if (weapon.isset())
			this->WeaponType.SetAll(WeaponStruct(weapon));
	}
}

#pragma region(save/load)

template <class T>
bool InterceptorTypeClass::Serialize(T& stm)
{
	return stm
		.Process(this->OwnedType)
		.Process(this->CanTargetHouses)
		.Process(this->Rookie)
		.Process(this->Veteran)
		.Process(this->Elite)
		.Process(this->GuardRange)
		.Process(this->MinimumGuardRange)
		.Process(this->Weapon)
		.Process(this->WeaponType)
		.Process(this->UseStageWeapon)
		.Process(this->DeleteOnIntercept)
		.Process(this->WeaponOverride)
		.Process(this->WeaponReplaceProjectile)
		.Process(this->WeaponCumulativeDamage)
		.Process(this->KeepIntact)
		.Process(this->Success)
		.Process(this->RookieSuccess)
		.Process(this->VeteranSuccess)
		.Process(this->EliteSuccess)
		.Process(this->InterfereOnIntercept)
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
