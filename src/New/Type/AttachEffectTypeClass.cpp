#include "AttachEffectTypeClass.h"

#include <Utilities/TemplateDef.h>

Enumerable<AttachEffectTypeClass>::container_t Enumerable<AttachEffectTypeClass>::Array;

const char* Enumerable<AttachEffectTypeClass>::GetMainSection()
{
	return "PhobosAttachEffectTypes";
}

void AttachEffectTypeClass::LoadFromINI(CCINIClass* pINI)
{
	const char* pSection = Name.data();

	INI_EX exINI(pINI);

	this->FirePower.Read(exINI, pSection, "FirePower");
	this->ROF.Read(exINI, pSection, "ROF");
	this->Armor.Read(exINI, pSection, "Armor");
	this->Speed.Read(exINI, pSection, "Speed");
	this->ROT.Read(exINI, pSection, "ROT");
	this->Range.Read(exINI, pSection, "Range");
	this->Range.Read(exINI, pSection, "WeaponRange.ExtraRange");
	this->Weight.Read(exINI, pSection, "Weight");
	this->FirePower_Multiplier.Read(exINI, pSection, "FirePower.Multiplier");
	this->FirePower_Multiplier.Read(exINI, pSection, "FirepowerMultiplier");
	this->ROF_Multiplier.Read(exINI, pSection, "ROF.Multiplier");
	this->ROF_Multiplier.Read(exINI, pSection, "ROFMultiplier");
	this->Armor_Multiplier.Read(exINI, pSection, "Armor.Multiplier");
	this->Armor_Multiplier.Read(exINI, pSection, "ArmorMultiplier");
	this->Speed_Multiplier.Read(exINI, pSection, "Speed.Multiplier");
	this->Speed_Multiplier.Read(exINI, pSection, "SpeedMultilpier");
	this->ROT_Multiplier.Read(exINI, pSection, "ROT.Multiplier");
	this->Range_Multiplier.Read(exINI, pSection, "Range.Multiplier");
	this->Range_Multiplier.Read(exINI, pSection, "WeaponRange.Multiplier");
	this->Weight_Multiplier.Read(exINI, pSection, "Weight.Multiplier");
	this->Scale.Read(exINI, pSection, "Scale");
	this->DisableWeapon.Read(exINI, pSection, "DisableWeapon");
	this->DisableWeapon_Category.Read(exINI, pSection, "DisableWeapon.Category");
	this->DecloakToFire.Read(exINI, pSection, "DecloakToFire");
	this->Cloak.Read(exINI, pSection, "Cloak");
	this->Cloak.Read(exINI, pSection, "Cloakable");
	this->Decloak.Read(exINI, pSection, "Decloak");
	this->Decloak.Read(exINI, pSection, "ForceDecloak");
	this->Sensor.Read(exINI, pSection, "Sensor");

	this->Anim.Read(exINI, pSection, "Anim");
	this->Anim.Read(exINI, pSection, "Animation");
	this->EndedAnim.Read(exINI, pSection, "EndedAnim");
	this->Anim_FLH.Read(exINI, pSection, "Anim.FLH");

	for (size_t i = 0;; i++)
	{
		Nullable<CoordStruct> flh;
		char key[0x20];

		sprintf_s(key, "Anim%d.FLH", i);
		flh.Read(exINI, pSection, key);

		if (!flh.isset())
			break;

		Anim_FLHs.emplace_back(flh);
	}

	this->Anim_RandomPick.Read(exINI, pSection, "Anim.RandomPick");
	this->EndedAnim_RandomPick.Read(exINI, pSection, "EndedAnim.RandomPick");

	this->WeaponList.Read(exINI, pSection, "WeaponList");
	this->WeaponList_FireOnAttach.Read(exINI, pSection, "WeaponList.FireOnAttach");
	this->AttackedWeaponList.Read(exINI, pSection, "AttackedWeaponList");
	this->ReplaceArmor.Read(exINI, pSection, "ReplaceArmor");
	this->ReplaceArmor_Shield.Read(exINI, pSection, "ReplaceArmor.Shield");
	this->ReplaceWeapon.Read(exINI, pSection, "ReplaceWeapon");
	this->ReplacePrimary.Read(exINI, pSection, "ReplacePrimary.%s");
	this->ReplaceSecondary.Read(exINI, pSection, "ReplaceSecondary.%s");
	this->ReplaceGattlingWeapon.Read(exINI, pSection, "ReplaceGattlingWeapon.%s");
	this->ReplaceDeathWeapon.Read(exINI, pSection, "ReplaceDeathWeapon");
	this->ForceExplode.Read(exINI, pSection, "ForceExplode");
	this->PenetratesIronCurtain.Read(exINI, pSection, "PenetratesIronCurtain");
	this->DiscardOnEntry.Read(exINI, pSection, "DiscardOnEntry");
	this->Cumulative.Read(exINI, pSection, "Cumulative");
	this->Cumulative_Maximum.Read(exINI, pSection, "Cumulative.Maximum");
	this->Cumulative_Maximum.Read(exINI, pSection, "Cumulative.MaxCount");
	this->IfExist_IgnoreOwner.Read(exINI, pSection, "IfExist.IgnoreOwner");
	this->IfExist_AddTimer.Read(exINI, pSection, "IfExist.AddTimer");
	this->IfExist_AddTimer_Cap.Read(exINI, pSection, "IfExist.AddTimer.Cap");
	this->IfExist_ResetTimer.Read(exINI, pSection, "IfExist.ResetTimer");
	this->IfExist_ResetAnim.Read(exINI, pSection, "IfExist.ResetAnim");
	this->IfExist_ResetAnim.Read(exINI, pSection, "Animation.ResetOnReapply");
	this->ShowAnim_Cloaked.Read(exINI, pSection, "ShowAnim.Cloaked");
	this->Loop_Delay.Read(exINI, pSection, "Loop.Delay");
	this->Loop_Duration.Read(exINI, pSection, "Loop.Duration");
	this->Duration.Read(exINI, pSection, "Duration");
	this->RandomDuration.Read(exINI, pSection, "RandomDuration");
	this->RandomDuration_Interval.Read(exINI, pSection, "RandomDuration.Interval");
	this->Delay.Read(exINI, pSection, "Delay");
	this->Coexist_Maximum.Read(exINI, pSection, "Coexist.Maximum");
	this->HideImage.Read(exINI, pSection, "HideImage");
	this->ImmuneMindControl.Read(exINI, pSection, "ImmuneMindControl");
	this->AllowMinHealth.Read(exINI, pSection, "AllowMinHealth");
	this->InfDeathAnim.Read(exINI, pSection, "InfDeathAnim");
	this->NextAttachEffects.Read(exINI, pSection, "NextAttachEffects");
	this->DiscardAfterShoots.Read(exINI, pSection, "DiscardAfterShoots");
	this->DiscardAfterHits.Read(exINI, pSection, "DiscardAfterHits");
	this->MaxReceive.Read(exINI, pSection, "MaxReceive");
	this->AuxTechnos.Read(exINI, pSection, "AuxTechnos");
	this->NegTechnos.Read(exINI, pSection, "NegTechnos");
	this->OwnerFireOn.Read(exINI, pSection, "OwnerFireOn");
	this->FireOnOwner.Read(exINI, pSection, "FireOnOwner");
	this->DisableBeSelect.Read(exINI, pSection, "DisableBeSelect");
	this->DisableBeTarget.Read(exINI, pSection, "DisableBeTarget");
	this->RevengeWeapon.Read(exINI, pSection, "RevengeWeapon");
	this->RevengeWeapon_AffectsHouses.Read(exINI, pSection, "RevengeWeapon.AffectsHouses");
	this->RevengeWeaponAttach.Read(exINI, pSection, "RevengeWeaponAttach");
	this->RevengeWeaponAttach_AffectsHouses.Read(exINI, pSection, "RevengeWeaponAttach.AffectsHouses");
	this->ForbiddenSelfHeal.Read(exINI, pSection, "ForbiddenSelfHeal");
	this->EMP.Read(exINI, pSection, "EMP");
	this->Psychedelic.Read(exINI, pSection, "Psychedelic");
	this->SensorsSight.Read(exINI, pSection, "SensorsSight");
	this->RevealSight.Read(exINI, pSection, "RevealSight");

	this->Dodge_Chance.Read(exINI, pSection, "Dodge.Chance");
	this->Dodge_Houses.Read(exINI, pSection, "Dodge.Houses");
	this->Dodge_MaxHealthPercent.Read(exINI, pSection, "Dodge.MaxHealthPercent");
	this->Dodge_MinHealthPercent.Read(exINI, pSection, "Dodge.MinHealthPercent");
	this->Dodge_Anim.Read(exINI, pSection, "Dodge.Anim");
	this->Dodge_OnlyDodgePositiveDamage.Read(exINI, pSection, "Dodge.OnlyDodgePositiveDamage");

	this->MoveDamage.Read(exINI, pSection, "MoveDamage");
	this->MoveDamage_Delay.Read(exINI, pSection, "MoveDamage.Delay");
	this->MoveDamage_Warhead.Read(exINI, pSection, "MoveDamage.Warhead");
	this->MoveDamage_Anim.Read(exINI, pSection, "MoveDamage.Anim");

	this->StopDamage.Read(exINI, pSection, "StopDamage");
	this->StopDamage_Delay.Read(exINI, pSection, "StopDamage.Delay");
	this->StopDamage_Warhead.Read(exINI, pSection, "StopDamage.Warhead");
	this->StopDamage_Anim.Read(exINI, pSection, "StopDamage.Anim");

	this->Blackhole_Range.Read(exINI, pSection, "Blackhole.Range");
	this->Blackhole_MinRange.Read(exINI, pSection, "Blackhole.MinRange");
	this->Blackhole_Destory.Read(exINI, pSection, "Blackhole.Destory");
	this->Blackhole_Destory_TakeDamage.Read(exINI, pSection, "Blackhole.Destory.TakeDamage");
	this->Blackhole_Destory_TakeDamageMultiplier.Read(exINI, pSection, "Blackhole.Destory.TakeDamageMultiplier");
	this->Blackhole_AffectedHouse.Read(exINI, pSection, "Blackhole.AffectedHouse");

	this->Crit_Multiplier.Read(exINI, pSection, "Crit.Multiplier");
	this->Crit_ExtraChance.Read(exINI, pSection, "Crit.ExtraChance");
	this->Crit_AllowWarheads.Read(exINI, pSection, "Crit.AllowWarheads");
	this->Crit_DisallowWarheads.Read(exINI, pSection, "Crit.DisallowWarheads");

	this->BaseNormal.Read(exINI, pSection, "BaseNormal");
	this->EligibileForAllyBuilding.Read(exINI, pSection, "EligibileForAllyBuilding");

	for (size_t i = 0; i <= this->Tint_Colors.size(); ++i)
	{
		Nullable<ColorStruct> color;
		_snprintf_s(Phobos::readBuffer, Phobos::readLength, "Tint.Color%d", i);
		color.Read(exINI, pSection, Phobos::readBuffer);

		if (!color.isset())
			continue;

		if (i == this->Tint_Colors.size())
			this->Tint_Colors.emplace_back(color);
		else
			this->Tint_Colors[i] = color;
	}

	this->Tint_TransitionDuration.Read(exINI, pSection, "Tint.TransitionDuration");
}

template <typename T>
void AttachEffectTypeClass::Serialize(T& stm)
{
	stm
		.Process(this->FirePower)
		.Process(this->ROF)
		.Process(this->Armor)
		.Process(this->Speed)
		.Process(this->ROT)
		.Process(this->Range)
		.Process(this->Weight)
		.Process(this->FirePower_Multiplier)
		.Process(this->ROF_Multiplier)
		.Process(this->Armor_Multiplier)
		.Process(this->Speed_Multiplier)
		.Process(this->ROT_Multiplier)
		.Process(this->Range_Multiplier)
		.Process(this->Weight_Multiplier)
		.Process(this->Scale)
		.Process(this->DisableWeapon)
		.Process(this->DisableWeapon_Category)
		.Process(this->DecloakToFire)
		.Process(this->Cloak)
		.Process(this->Decloak)
		.Process(this->Sensor)
		.Process(this->Anim)
		.Process(this->EndedAnim)
		.Process(this->Anim_FLH)
		.Process(this->Anim_FLHs)
		.Process(this->Anim_RandomPick)
		.Process(this->EndedAnim_RandomPick)
		.Process(this->WeaponList)
		.Process(this->WeaponList_FireOnAttach)
		.Process(this->AttackedWeaponList)
		.Process(this->PenetratesIronCurtain)
		.Process(this->DiscardOnEntry)
		.Process(this->Cumulative)
		.Process(this->Cumulative_Maximum)
		.Process(this->Duration)
		.Process(this->Delay)
		.Process(this->Loop_Delay)
		.Process(this->Loop_Duration)
		.Process(this->ShowAnim_Cloaked)
		.Process(this->IfExist_IgnoreOwner)
		.Process(this->IfExist_AddTimer)
		.Process(this->IfExist_AddTimer_Cap)
		.Process(this->IfExist_ResetTimer)
		.Process(this->IfExist_ResetAnim)
		.Process(this->ReplaceArmor)
		.Process(this->ReplaceArmor_Shield)
		.Process(this->ReplaceWeapon)
		.Process(this->ReplacePrimary)
		.Process(this->ReplaceSecondary)
		.Process(this->ReplaceGattlingWeapon)
		.Process(this->ReplaceDeathWeapon)
		.Process(this->ForceExplode)
		.Process(this->Coexist_Maximum)
		.Process(this->HideImage)
		.Process(this->ArrayIndex)
		.Process(this->AllowMinHealth)
		.Process(this->RandomDuration)
		.Process(this->RandomDuration_Interval)
		.Process(this->ImmuneMindControl)
		.Process(this->InfDeathAnim)
		.Process(this->Tint_Colors)
		.Process(this->Tint_TransitionDuration)
		.Process(this->NextAttachEffects)
		.Process(this->DiscardAfterShoots)
		.Process(this->DiscardAfterHits)
		.Process(this->MaxReceive)
		.Process(this->AuxTechnos)
		.Process(this->NegTechnos)
		.Process(this->OwnerFireOn)
		.Process(this->FireOnOwner)
		.Process(this->DisableBeSelect)
		.Process(this->DisableBeTarget)
		.Process(this->RevengeWeapon)
		.Process(this->RevengeWeapon_AffectsHouses)
		.Process(this->RevengeWeaponAttach)
		.Process(this->RevengeWeaponAttach_AffectsHouses)
		.Process(this->ForbiddenSelfHeal)
		.Process(this->EMP)
		.Process(this->Psychedelic)
		.Process(this->SensorsSight)
		.Process(this->RevealSight)

		.Process(this->Dodge_Chance)
		.Process(this->Dodge_Houses)
		.Process(this->Dodge_MaxHealthPercent)
		.Process(this->Dodge_MinHealthPercent)
		.Process(this->Dodge_Anim)
		.Process(this->Dodge_OnlyDodgePositiveDamage)

		.Process(this->MoveDamage)
		.Process(this->MoveDamage_Delay)
		.Process(this->MoveDamage_Warhead)
		.Process(this->MoveDamage_Anim)

		.Process(this->StopDamage)
		.Process(this->StopDamage_Delay)
		.Process(this->StopDamage_Warhead)
		.Process(this->StopDamage_Anim)

		.Process(this->Blackhole_Range)
		.Process(this->Blackhole_MinRange)
		.Process(this->Blackhole_Destory)
		.Process(this->Blackhole_Destory_TakeDamage)
		.Process(this->Blackhole_Destory_TakeDamageMultiplier)
		.Process(this->Blackhole_AffectedHouse)

		.Process(this->Crit_Multiplier)
		.Process(this->Crit_ExtraChance)
		.Process(this->Crit_AllowWarheads)
		.Process(this->Crit_DisallowWarheads)

		.Process(this->BaseNormal)
		.Process(this->EligibileForAllyBuilding)
		;
}

void AttachEffectTypeClass::LoadFromStream(PhobosStreamReader& stm)
{
	this->Serialize(stm);
}

void AttachEffectTypeClass::SaveToStream(PhobosStreamWriter& stm)
{
	this->Serialize(stm);
}
