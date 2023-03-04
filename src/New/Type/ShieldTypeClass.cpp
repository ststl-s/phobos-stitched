#include "ShieldTypeClass.h"

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

	this->Directional.Read(exINI, pSection, "Directional");
	this->Directional_FrontMultiplier.Read(exINI, pSection, "Directional.FrontMultiplier");
	this->Directional_SideMultiplier.Read(exINI, pSection, "Directional.SideMultiplier");
	this->Directional_BackMultiplier.Read(exINI, pSection, "Directional.BackMultiplier");
	this->Directional_FrontField.Read(exINI, pSection, "Directional.FrontField");
	this->Directional_BackField.Read(exINI, pSection, "Directional.BackField");

	this->Directional_FrontField = Math::min(this->Directional_FrontField, 1.0f);
	this->Directional_FrontField = Math::max(this->Directional_FrontField, 0.0f);
	this->Directional_BackField = Math::min(this->Directional_BackField, 1.0f);
	this->Directional_BackField = Math::max(this->Directional_BackField, 0.0f);

	this->AbsorbOverDamage.Read(exINI, pSection, "AbsorbOverDamage");
	this->BracketDelta.Read(exINI, pSection, "BracketDelta");

	this->IdleAnim_OfflineAction.Read(exINI, pSection, "IdleAnim.OfflineAction");
	this->IdleAnim_TemporalAction.Read(exINI, pSection, "IdleAnim.TemporalAction");

	this->IdleAnim.Read(exINI, pSection, "IdleAnim.%s");
	this->IdleAnimDamaged.Read(exINI, pSection, "IdleAnimDamaged.%s");

	this->BreakAnim.Read(exINI, pSection, "BreakAnim");
	this->BreakWeapon.Read(exINI, pSection, "BreakWeapon", true);
	this->HitAnim.Read(exINI, pSection, "HitAnim");
	this->HitAnim_PickRandom.Read(exINI, pSection, "HitAnim.PickRandom");
	this->HitAnim_PickByDirection.Read(exINI, pSection, "HitAnim.PickByDirection");

	this->AbsorbPercent.Read(exINI, pSection, "AbsorbPercent");
	this->PassPercent.Read(exINI, pSection, "PassPercent");

	this->AllowTransfer.Read(exINI, pSection, "AllowTransfer");

	this->CanBeStolen.Read(exINI, pSection, "CanBeStolen");
	this->CanBeStolenType.Read(exINI, pSection, "CanBeStolenType");

	this->Pips.Read(exINI, pSection, "Pips");
	this->Pips_Background.Read(exINI, pSection, "PipBrd.SHP");
	this->Pips_Building.Read(exINI, pSection, "Pips.Building");
	this->Pips_Building_Empty.Read(exINI, pSection, "Pips.Building.Empty");

	this->ImmuneToBerserk.Read(exINI, pSection, "ImmuneToBerserk");

	this->PoweredTechnos.Read(exINI, pSection, "PoweredTechnos");
	this->PoweredTechnos_Any.Read(exINI, pSection, "PoweredTechnos.Any");

	HealthBarTypeClass::LoadFromINIList(pINI, pSection, "ShieldBarType");
	this->ShieldBar.Read(exINI, pSection, "ShieldBarType");
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
		.Process(this->Directional)
		.Process(this->Directional_FrontMultiplier)
		.Process(this->Directional_SideMultiplier)
		.Process(this->Directional_BackMultiplier)
		.Process(this->Directional_FrontField)
		.Process(this->Directional_BackField)
		.Process(this->AbsorbOverDamage)
		.Process(this->BracketDelta)
		.Process(this->IdleAnim_OfflineAction)
		.Process(this->IdleAnim_TemporalAction)
		.Process(this->IdleAnim)
		.Process(this->BreakAnim)
		.Process(this->BreakWeapon)
		.Process(this->HitAnim)
		.Process(this->HitAnim_PickRandom)
		.Process(this->HitAnim_PickByDirection)
		.Process(this->AbsorbPercent)
		.Process(this->PassPercent)
		.Process(this->AllowTransfer)
		.Process(this->Pips)
		.Process(this->Pips_Background)
		.Process(this->Pips_Building)
		.Process(this->Pips_Building_Empty)
		.Process(this->ImmuneToBerserk)
		.Process(this->CanBeStolen)
		.Process(this->CanBeStolenType)
		.Process(this->PoweredTechnos)
		.Process(this->PoweredTechnos_Any)

		.Process(this->ShieldBar)
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
