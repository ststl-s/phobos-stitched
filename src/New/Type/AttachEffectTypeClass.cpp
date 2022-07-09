#include "AttachEffectTypeClass.h"

#include <Utilities/TemplateDef.h>

Enumerable<AttachEffectTypeClass>::container_t Enumerable<AttachEffectTypeClass>::Array;

const char* AttachEffectTypeClass::GetMainSection()
{
	return "AttachEffectTypes";
}

void AttachEffectTypeClass::LoadFromINI(CCINIClass* pINI)
{
	const char* pSection = Name.data();

	if (pINI->GetSection(pSection) == nullptr)
		return;

	INI_EX exINI(pINI);

	this->FirePower.Read(exINI, pSection, "FirePower");
	this->ROF.Read(exINI, pSection, "ROF");
	this->Armor.Read(exINI, pSection, "Armor");
	this->Speed.Read(exINI, pSection, "Speed");
	this->FirePower_Multiplier.Read(exINI, pSection, "FirePower.Multiplier");
	this->ROF_Multiplier.Read(exINI, pSection, "ROF.Multiplier");
	this->Armor_Multiplier.Read(exINI, pSection, "Armor.Multiplier");
	this->Speed_Multiplier.Read(exINI, pSection, "Speed.Multiplier");
	this->DisableWeapon.Read(exINI, pSection, "DisableWeapon");
	this->DisableTurn.Read(exINI, pSection, "DisableTurn");
	this->AnimList.Read(exINI, pSection, "AnimList");
	this->WeaponList.Read(exINI, pSection, "WeaponList");
	this->AttackedWeaponList.Read(exINI, pSection, "AttackedWeaponList");
	this->CanBeMultiplie.Read(exINI, pSection, "CanBeMultiplie");
	this->ResetIfExist.Read(exINI, pSection, "ResetIfExist");
	this->DelayOnAttach.Read(exINI, pSection, "DelayOnAttach");
	this->Loop_Delay.Read(exINI, pSection, "Loop.Delay");
	this->Loop_Duration.Read(exINI, pSection, "Loop.Duration");
}

template <typename T>
void AttachEffectTypeClass::Serialize(T& stm)
{
	stm
		.Process(this->FirePower)
		.Process(this->ROF)
		.Process(this->Armor)
		.Process(this->Speed)
		.Process(this->FirePower_Multiplier)
		.Process(this->ROF_Multiplier)
		.Process(this->Armor_Multiplier)
		.Process(this->Speed_Multiplier)
		.Process(this->DisableWeapon)
		.Process(this->DisableTurn)
		.Process(this->AnimList)
		.Process(this->WeaponList)
		.Process(this->AttackedWeaponList)
		.Process(this->CanBeMultiplie)
		.Process(this->ResetIfExist)
		.Process(this->DelayOnAttach)
		.Process(this->Loop_Delay)
		.Process(this->Loop_Duration)
		;
}

void AttachEffectTypeClass::LoadFromStream(PhobosStreamReader& stm)
{
	Serialize(stm);
}

void AttachEffectTypeClass::SaveToStream(PhobosStreamWriter& stm)
{
	Serialize(stm);
}
