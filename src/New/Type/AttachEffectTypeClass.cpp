#include "AttachEffectTypeClass.h"

#include <Utilities/TemplateDef.h>
#include <Utilities/SavegameDef.h>

Enumerable<AttachEffectTypeClass>::container_t Enumerable<AttachEffectTypeClass>::Array;

const char* Enumerable<AttachEffectTypeClass>::GetMainSection()
{
	return "AttachEffectTypes";
}

void AttachEffectTypeClass::LoadFromINI(CCINIClass* pINI)
{
	const char* pSection = Name.data();

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
	this->Cloak.Read(exINI, pSection, "Cloak");
	this->Decloak.Read(exINI, pSection, "Decloak");
	this->Anim.Read(exINI, pSection, "Anim");
	this->WeaponList.Read(exINI, pSection, "WeaponList");
	this->AttackedWeaponList.Read(exINI, pSection, "AttackedWeaponList");
	this->PenetratesIronCurtain.Read(exINI, pSection, "PenetratesIronCurtain");
	this->DiscardOnEntry.Read(exINI, pSection, "DiscardOnEntry");
	this->Cumulative.Read(exINI, pSection, "Cumulative");
	this->ResetIfExist_Timer.Read(exINI, pSection, "ResetIfExist.Timer");
	this->ResetIfExist_Anim.Read(exINI, pSection, "ResetIfExist.Anim");
	this->ShowAnim_Cloaked.Read(exINI, pSection, "ShowAnim.Cloaked");
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
		.Process(this->Cloak)
		.Process(this->Decloak)
		.Process(this->Anim)
		.Process(this->WeaponList)
		.Process(this->AttackedWeaponList)
		.Process(this->PenetratesIronCurtain)
		.Process(this->DiscardOnEntry)
		.Process(this->Cumulative)
		.Process(this->Loop_Delay)
		.Process(this->Loop_Duration)
		.Process(this->ShowAnim_Cloaked)
		.Process(this->ResetIfExist_Timer)
		.Process(this->ResetIfExist_Anim)
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
