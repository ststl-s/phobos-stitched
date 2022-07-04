#include "ShieldTypeClass.h"
#include <New/Type/DigitalDisplayTypeClass.h>

Enumerable<ShieldTypeClass>::container_t Enumerable<ShieldTypeClass>::Array;

const char* Enumerable<ShieldTypeClass>::GetMainSection()
{
	return "ShieldTypes";
}

AnimTypeClass* ShieldTypeClass::GetIdleAnimType(bool isDamaged, double healthRatio)
{
	auto damagedAnim = this->IdleAnimDamaged.Get(healthRatio);

	if (isDamaged && damagedAnim)
		return damagedAnim;
	else
		return this->IdleAnim.Get(healthRatio);
}

void ShieldTypeClass::LoadFromINI(CCINIClass* pINI)
{
	const char* pSection = this->Name;
	if (strcmp(pSection, NONE_STR) == 0)
		return;

	INI_EX exINI(pINI);

	this->Strength.Read(exINI, pSection, "Strength");
	this->InitialStrength.Read(exINI, pSection, "InitialStrength");
	this->Armor.Read(exINI, pSection, "Armor");
	this->Powered.Read(exINI, pSection, "Powered");

	this->Respawn.Read(exINI, pSection, "Respawn");
	this->Respawn_Rate__InMinutes.Read(exINI, pSection, "Respawn.Rate");
	this->Respawn_Rate = (int)(this->Respawn_Rate__InMinutes * 900);

	this->SelfHealing.Read(exINI, pSection, "SelfHealing");
	this->SelfHealing_Rate__InMinutes.Read(exINI, pSection, "SelfHealing.Rate");
	this->SelfHealing_Rate = (int)(this->SelfHealing_Rate__InMinutes * 900);

	this->AbsorbOverDamage.Read(exINI, pSection, "AbsorbOverDamage");
	this->BracketDelta.Read(exINI, pSection, "BracketDelta");

	this->IdleAnim_OfflineAction.Read(exINI, pSection, "IdleAnim.OfflineAction");
	this->IdleAnim_TemporalAction.Read(exINI, pSection, "IdleAnim.TemporalAction");

	this->IdleAnim.Read(exINI, pSection, "IdleAnim.%s");
	this->IdleAnimDamaged.Read(exINI, pSection, "IdleAnimDamaged.%s");

	this->BreakAnim.Read(exINI, pSection, "BreakAnim");
	this->HitAnim.Read(exINI, pSection, "HitAnim");
	this->BreakWeapon.Read(exINI, pSection, "BreakWeapon", true);

	this->AbsorbPercent.Read(exINI, pSection, "AbsorbPercent");
	this->PassPercent.Read(exINI, pSection, "PassPercent");

	this->AllowTransfer.Read(exINI, pSection, "AllowTransfer");

	this->CanBeStolen.Read(exINI, pSection, "CanBeStolen");
	this->CanBeStolenType.Read(exINI, pSection, "CanBeStolenType");

	this->Pips.Read(exINI, pSection, "Pips");
	this->Pips_Filename.Read(pINI, pSection, "Pips.SHP");
	this->Pips_Building.Read(exINI, pSection, "Pips.Building");
	this->Pips_Building_Empty.Read(exINI, pSection, "Pips.Building.Empty");
	this->Pips_PALFilename.Read(pINI, pSection, "Pips.PAL");
	this->PipBrd.Read(exINI, pSection, "PipBrd");
	this->Pips_Background.Read(exINI, pSection, "PipBrd.SHP");
	this->Pips_Background_PALFilename.Read(pINI, pSection, "PipBrd.PAL");
	this->Pips_Length.Read(exINI, pSection, "Pips.Length");
	this->Pips_XOffset.Read(exINI, pSection, "Pips.XOffset");
	this->Pips_DrawOffset.Read(exINI, pSection, "Pips.DrawOffset");
	this->PipBrd_Offset.Read(exINI, pSection, "PipBrd.Offset");
}

template <typename T>
void ShieldTypeClass::Serialize(T& Stm)
{
	Stm
		.Process(this->Strength)
		.Process(this->InitialStrength)
		.Process(this->Armor)
		.Process(this->Powered)
		.Process(this->Respawn)
		.Process(this->Respawn_Rate)
		.Process(this->SelfHealing)
		.Process(this->SelfHealing_Rate)
		.Process(this->AbsorbOverDamage)
		.Process(this->BracketDelta)
		.Process(this->IdleAnim_OfflineAction)
		.Process(this->IdleAnim_TemporalAction)
		.Process(this->IdleAnim)
		.Process(this->BreakAnim)
		.Process(this->HitAnim)
		.Process(this->BreakWeapon)
		.Process(this->AbsorbPercent)
		.Process(this->PassPercent)
		.Process(this->AllowTransfer)
		.Process(this->Pips)
		.Process(this->Pips_Filename)
		.Process(this->Pips_PALFilename)
		.Process(this->Pips_Background)
		.Process(this->Pips_Background_PALFilename)
		.Process(this->Pips_SHP)
		.Process(this->Pips_Building)
		.Process(this->Pips_Building_Empty)
		.Process(this->Pips_Length)
		.Process(this->Pips_XOffset)
		.Process(this->Pips_DrawOffset)
		.Process(this->PipBrd)
		.Process(this->PipBrd_Offset)
		.Process(this->CanBeStolen)
		.Process(this->CanBeStolenType)
		;
}

void ShieldTypeClass::LoadFromStream(PhobosStreamReader& Stm)
{
	this->Serialize(Stm);
}

void ShieldTypeClass::SaveToStream(PhobosStreamWriter& Stm)
{
	this->Serialize(Stm);
}
