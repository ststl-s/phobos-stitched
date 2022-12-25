#include "Body.h"

#include <Utilities/EnumFunctions.h>

#include <New/Armor/Armor.h>

#include <New/Type/TemperatureTypeClass.h>

template<> const DWORD Extension<WarheadTypeClass>::Canary = 0x22222222;
WarheadTypeExt::ExtContainer WarheadTypeExt::ExtMap;

bool WarheadTypeExt::ExtData::CanTargetHouse(HouseClass* pHouse, TechnoClass* pTarget)
{
	if (pHouse && pTarget)
	{
		if (this->AffectsOwner.Get(this->OwnerObject()->AffectsAllies) && pTarget->Owner == pHouse)
			return true;

		bool isAllies = pHouse->IsAlliedWith(pTarget);

		if (this->OwnerObject()->AffectsAllies && isAllies)
			return pTarget->Owner == pHouse ? false : true;

		if (this->AffectsEnemies && !isAllies)
			return true;

		return false;
	}

	return true;
}

void WarheadTypeExt::DetonateAt(WarheadTypeClass* pThis, ObjectClass* pTarget, TechnoClass* pOwner, int damage)
{
	BulletTypeClass* pType = BulletTypeExt::GetDefaultBulletType();

	if (BulletClass* pBullet = pType->CreateBullet(pTarget, pOwner,
		damage, pThis, 0, pThis->Bright))
	{
		const CoordStruct& coords = pTarget->GetCoords();

		pBullet->Limbo();
		pBullet->SetLocation(coords);
		pBullet->Explode(true);
		pBullet->UnInit();
	}
}

void WarheadTypeExt::DetonateAt(WarheadTypeClass* pThis, const CoordStruct& coords, TechnoClass* pOwner, int damage)
{
	BulletTypeClass* pType = BulletTypeExt::GetDefaultBulletType();

	if (BulletClass* pBullet = pType->CreateBullet(nullptr, pOwner,
		damage, pThis, 0, pThis->Bright))
	{
		pBullet->Limbo();
		pBullet->SetLocation(coords);
		pBullet->Explode(true);
		pBullet->UnInit();
	}
}

bool WarheadTypeExt::ExtData::EligibleForFullMapDetonation(TechnoClass* pTechno, HouseClass* pOwner)
{
	if (!pTechno || !pTechno->IsOnMap || !pTechno->IsAlive || pTechno->InLimbo || pTechno->IsSinking)
		return false;

	if (pOwner && !EnumFunctions::CanTargetHouse(this->DetonateOnAllMapObjects_AffectHouses, pOwner, pTechno->Owner))
		return false;

	if ((this->DetonateOnAllMapObjects_AffectTypes.size() > 0 &&
		!this->DetonateOnAllMapObjects_AffectTypes.Contains(pTechno->GetTechnoType())) ||
		this->DetonateOnAllMapObjects_IgnoreTypes.Contains(pTechno->GetTechnoType()))
	{
		return false;
	}

	if (this->DetonateOnAllMapObjects_RequireVerses &&
		GeneralUtils::GetWarheadVersusArmor(this->OwnerObject(), pTechno->GetTechnoType()->Armor) == 0.0)
	{
		return false;
	}

	return true;
}

// =============================
// load / save

void WarheadTypeExt::ExtData::LoadFromINIFile(CCINIClass* const pINI)
{
	auto pThis = this->OwnerObject();
	const char* pSection = pThis->ID;

	if (!pINI->GetSection(pSection))
		return;

	INI_EX exINI(pINI);

	// Miscs
	this->SpySat.Read(exINI, pSection, "SpySat");
	this->BigGap.Read(exINI, pSection, "BigGap");
	this->TransactMoney.Read(exINI, pSection, "TransactMoney");
	this->TransactMoney_Display.Read(exINI, pSection, "TransactMoney.Display");
	this->TransactMoney_Display_Houses.Read(exINI, pSection, "TransactMoney.Display.Houses");
	this->TransactMoney_Display_AtFirer.Read(exINI, pSection, "TransactMoney.Display.AtFirer");
	this->TransactMoney_Display_Offset.Read(exINI, pSection, "TransactMoney.Display.Offset");
	this->SplashList.Read(exINI, pSection, "SplashList");
	this->SplashList_PickRandom.Read(exINI, pSection, "SplashList.PickRandom");
	this->RemoveDisguise.Read(exINI, pSection, "RemoveDisguise");
	this->RemoveMindControl.Read(exINI, pSection, "RemoveMindControl");
	this->AnimList_PickByDirection.Read(exINI, pSection, "AnimList.PickByDirection");
	this->AnimList_PickRandom.Read(exINI, pSection, "AnimList.PickRandom");
	this->DecloakDamagedTargets.Read(exINI, pSection, "DecloakDamagedTargets");
	this->GattlingStage.Read(exINI, pSection, "TargetGattlingStage");
	this->GattlingRateUp.Read(exINI, pSection, "TargetGattlingRateUp");
	this->ReloadAmmo.Read(exINI, pSection, "TargetReloadAmmo");
	this->ShakeIsLocal.Read(exINI, pSection, "ShakeIsLocal");

	// Transact
	this->Transact.Read(exINI, pSection, "Transact");
	this->Transact_SpreadAmongTargets.Read(exINI, pSection, "Transact.SpreadAmongTargets");
	this->Transact_Experience_Value.Read(exINI, pSection, "Transact.Experience.Value");
	this->Transact_Experience_Veterancy.Read(exINI, pSection, "Transact.Experience.Veterncy");
	this->Transact_Experience_Source_Flat.Read(exINI, pSection, "Transact.Experience.Source.Flat");
	this->Transact_Experience_Source_Percent.Read(exINI, pSection, "Transact.Experience.Source.Percent");
	this->Transact_Experience_Source_Percent_CalcFromTarget.Read(exINI, pSection, "Transact.Experience.Source.Percent.CalcFromTarget");
	this->Transact_Experience_Target_Flat.Read(exINI, pSection, "Transact.Experience.Target.Flat");
	this->Transact_Experience_Target_Percent.Read(exINI, pSection, "Transact.Experience.Target.Percent");
	this->Transact_Experience_Target_Percent_CalcFromSource.Read(exINI, pSection, "Transact.Experience.Target.Percent.CalcFromSource");

	this->Transact_Money_Source_Flat.Read(exINI, pSection, "Transact.Money.Source.Flat");
	this->Transact_Money_Source_Percent.Read(exINI, pSection, "Transact.Money.Source.Percent");
	this->Transact_Money_Source_Percent_CalcFromTarget.Read(exINI, pSection, "Transact.Money.Source.Percent.CalcFromTarget");
	this->Transact_Money_Source_Display.Read(exINI, pSection, "Transact.Money.Source.Display");
	this->Transact_Money_Source_Display_Houses.Read(exINI, pSection, "Transact.Money.Source.Display.Houses");
	this->Transact_Money_Source_Display_Offset.Read(exINI, pSection, "Transact.Money.Source.Display.Offset");
	this->Transact_Money_Source_Weapon.Read(exINI, pSection, "Transact.Money.Source.Weapon");
	this->Transact_Money_Target_Flat.Read(exINI, pSection, "Transact.Money.Target.Flat");
	this->Transact_Money_Target_Percent.Read(exINI, pSection, "Transact.Money.Target.Percent");
	this->Transact_Money_Target_Percent_CalcFromSource.Read(exINI, pSection, "Transact.Money.Target.Percent.CalcFromSource");
	this->Transact_Money_Target_Display.Read(exINI, pSection, "Transact.Money.Target.Display");
	this->Transact_Money_Target_Display_Houses.Read(exINI, pSection, "Transact.Money.Target.Display.Houses");
	this->Transact_Money_Target_Display_Offset.Read(exINI, pSection, "Transact.Money.Target.Display.Offset");
	this->Transact_Money_Target_Weapon.Read(exINI, pSection, "Transact.Money.Target.Weapon");

	this->Transact_Health_Source_Flat.Read(exINI, pSection, "Transact.Health.Source.Flat");
	this->Transact_Health_Source_Percent.Read(exINI, pSection, "Transact.Health.Source.Percent");
	this->Transact_Health_Source_Percent_UseCurrentHealth.Read(exINI, pSection, "Transact.Health.Source.Percent.UseCurrentHealth");
	this->Transact_Health_Source_Percent_CalcFromTarget.Read(exINI, pSection, "Transact.Health.Source.Percent.CalcFromTarget");
	this->Transact_Health_Target_Flat.Read(exINI, pSection, "Transact.Health.Target.Flat");
	this->Transact_Health_Target_Percent.Read(exINI, pSection, "Transact.Health.Target.Percent");
	this->Transact_Health_Target_Percent_UseCurrentHealth.Read(exINI, pSection, "Transact.Health.Target.Percent.UseCurrentHealth");
	this->Transact_Health_Target_Percent_CalcFromSource.Read(exINI, pSection, "Transact.Health.Target.Percent.CalcFromSource");

	// Crits
	this->Crit_Chance.Read(exINI, pSection, "Crit.Chance");
	this->Crit_ApplyChancePerTarget.Read(exINI, pSection, "Crit.ApplyChancePerTarget");
	this->Crit_ExtraDamage.Read(exINI, pSection, "Crit.ExtraDamage");
	this->Crit_Warhead.Read(exINI, pSection, "Crit.Warhead");
	this->Crit_Affects.Read(exINI, pSection, "Crit.Affects");
	this->Crit_AffectsHouses.Read(exINI, pSection, "Crit.AffectsHouse");
	this->Crit_AnimList.Read(exINI, pSection, "Crit.AnimList");
	this->Crit_AnimList_PickByDirection.Read(exINI, pSection, "Crit.AnimList.PickByDirection");
	this->Crit_AnimList_PickRandom.Read(exINI, pSection, "Crit.AnimList.PickRandom");
	this->Crit_AnimOnAffectedTargets.Read(exINI, pSection, "Crit.AnimOnAffectedTargets");
	this->Crit_AffectBelowPercent.Read(exINI, pSection, "Crit.AffectBelowPercent");
	this->Crit_SuppressWhenIntercepted.Read(exINI, pSection, "Crit.SuppressWhenIntercepted");

	this->MindControl_Anim.Read(exINI, pSection, "MindControl.Anim");

	// Shields
	this->Shield_Penetrate.Read(exINI, pSection, "Shield.Penetrate");
	this->Shield_Break.Read(exINI, pSection, "Shield.Break");
	this->Shield_BreakAnim.Read(exINI, pSection, "Shield.BreakAnim");
	this->Shield_BreakWeapon.Read(exINI, pSection, "Shield.BreakWeapon", true);
	this->Shield_HitAnim.Read(exINI, pSection, "Shield.HitAnim");
	this->Shield_HitAnim_PickRandom.Read(exINI, pSection, "Shield.HitAnim.PickRandom");
	this->Shield_HitAnim_PickByDirection.Read(exINI, pSection, "Shield.HitAnim.PickByDirection");
	this->Shield_AbsorbPercent.Read(exINI, pSection, "Shield.AbsorbPercent");
	this->Shield_PassPercent.Read(exINI, pSection, "Shield.PassPercent");
	this->Shield_Respawn_Duration.Read(exINI, pSection, "Shield.Respawn.Duration");
	this->Shield_Respawn_Amount.Read(exINI, pSection, "Shield.Respawn.Amount");
	this->Shield_Respawn_Rate_InMinutes.Read(exINI, pSection, "Shield.Respawn.Rate");
	this->Shield_Respawn_Rate = (int)(this->Shield_Respawn_Rate_InMinutes * 900);
	this->Shield_Respawn_ResetTimer.Read(exINI, pSection, "Shield.Respawn.RestartTimer");
	this->Shield_SelfHealing_Duration.Read(exINI, pSection, "Shield.SelfHealing.Duration");
	this->Shield_SelfHealing_Amount.Read(exINI, pSection, "Shield.SelfHealing.Amount");
	this->Shield_SelfHealing_Rate_InMinutes.Read(exINI, pSection, "Shield.SelfHealing.Rate");
	this->Shield_SelfHealing_Rate = (int)(this->Shield_SelfHealing_Rate_InMinutes * 900);
	this->Shield_SelfHealing_ResetTimer.Read(exINI, pSection, "Shield.SelfHealing.RestartTimer");
	this->Shield_AttachTypes.Read(exINI, pSection, "Shield.AttachTypes");
	this->Shield_RemoveTypes.Read(exINI, pSection, "Shield.RemoveTypes");
	this->Shield_ReplaceOnly.Read(exINI, pSection, "Shield.ReplaceOnly");
	this->Shield_ReplaceNonRespawning.Read(exINI, pSection, "Shield.ReplaceNonRespawning");
	this->Shield_InheritStateOnReplace.Read(exINI, pSection, "Shield.InheritStateOnReplace");
	this->Shield_MinimumReplaceDelay.Read(exINI, pSection, "Shield.MinimumReplaceDelay");
	this->Shield_AffectTypes.Read(exINI, pSection, "Shield.AffectTypes");
	this->Shield_Steal.Read(exINI, pSection, "Shield.Steal");
	this->Shield_Assimilate_Rate.Read(exINI, pSection, "Shield.Assimilate.Rate");
	this->Shield_StealTargetType.Read(exINI, pSection, "Shield.StealTargetType");
	this->Shield_StealTargetType_InitShieldHealthRate.Read(exINI, pSection, "Shield.StealTargetType.InitShieldHealthRate");

	this->NotHuman_DeathSequence.Read(exINI, pSection, "NotHuman.DeathSequence");
	this->AllowDamageOnSelf.Read(exINI, pSection, "AllowDamageOnSelf");

	this->LaunchSW.Read(exINI, pSection, "LaunchSW");
	this->LaunchSW_RealLaunch.Read(exINI, pSection, "LaunchSW.RealLaunch");
	this->LaunchSW_IgnoreInhibitors.Read(exINI, pSection, "LaunchSW.IgnoreInhibitors");
	this->LaunchSW_IgnoreDesignators.Read(exINI, pSection, "LaunchSW.IgnoreDesignators");

	this->DebrisAnims.Read(exINI, pSection, "DebrisAnims");
	this->Debris_Conventional.Read(exINI, pSection, "Debris.Conventional");

	this->MindControl_Threshold.Read(exINI, pSection, "MindControl.Threshold");
	this->MindControl_Threshold_Inverse.Read(exINI, pSection, "MindControl.Threshold.Inverse");
	this->MindControl_AlternateDamage.Read(exINI, pSection, "MindControl.AlternateDamage");
	this->MindControl_AlternateWarhead.Read(exINI, pSection, "MindControl.AlternateWarhead", true);
	this->MindControl_CanKill.Read(exINI, pSection, "MindControl.CanKill");

	this->Converts.Read(exINI, pSection, "Converts");
	this->Converts_From.Read(exINI, pSection, "Converts.From");
	this->Converts_To.Read(exINI, pSection, "Converts.To");
	this->Converts_Duration.Read(exINI, pSection, "Converts.Duration");
	this->Converts_Anim.Read(exINI, pSection, "Converts.Anim");
	this->Converts_RecoverAnim.Read(exINI, pSection, "Converts.RecoverAnim");
	this->Converts_DetachedBuildLimit.Read(exINI, pSection, "Converts.DetachedBuildLimit");

	this->ClearPassengers.Read(exINI, pSection, "ClearPassengers");
	this->ReleasePassengers.Read(exINI, pSection, "ReleasePassengers");
	this->DamagePassengers.Read(exINI, pSection, "DamagePassengers");
	this->DamagePassengers_AffectAllPassengers.Read(exINI, pSection, "DamagePassengers.AffectAllPassengers");

	this->DisableTurn_Duration.Read(exINI, pSection, "DisableTurn.Duration");

	this->PaintBall_Color.Read(exINI, pSection, "PaintBall.Color");
	this->PaintBall_Duration.Read(exINI, pSection, "PaintBall.Duration");
	this->PaintBall_IsDiscoColor.Read(exINI, pSection, "PaintBall.IsDiscoColor");

	for (size_t i = 0; i <= this->PaintBall_Colors.size(); ++i)
	{
		Nullable<ColorStruct> color;
		_snprintf_s(Phobos::readBuffer, Phobos::readLength, "PaintBall.Color%d", i);
		color.Read(exINI, pSection, Phobos::readBuffer);

		if (i == this->PaintBall_Colors.size() && !color.isset())
			break;
		else if (!color.isset())
			continue;

		if (i == this->PaintBall_Colors.size())
			this->PaintBall_Colors.push_back(color);
		else
			this->PaintBall_Colors[i] = color;
	}

	this->PaintBall_TransitionDuration.Read(exINI, pSection, "PaintBall.TransitionDuration");
	this->PaintBall_IgnoreTintStatus.Read(exINI, pSection, "PaintBall.IgnoreTintStatus");

	this->AttackedWeapon_ForceNoResponse.Read(exINI, pSection, "AttackedWeapon.ForceNoResponse");
	this->AttackedWeapon_ResponseTechno.Read(exINI, pSection, "AttackedWeapon.ResponseTechno");
	this->AttackedWeapon_NoResponseTechno.Read(exINI, pSection, "AttackedWeapon.NoResponseTechno");

	this->CanBeDodge.Read(exINI, pSection, "CanBeDodge");
	this->DodgeAttach_Houses.Read(exINI, pSection, "DodgeAttach.Houses");
	this->DodgeAttach_Duration.Read(exINI, pSection, "DodgeAttach.Duration");
	this->DodgeAttach_MaxHealthPercent.Read(exINI, pSection, "DodgeAttach.MaxHealthPercent");
	this->DodgeAttach_MinHealthPercent.Read(exINI, pSection, "DodgeAttach.MinHealthPercent");
	this->DodgeAttach_Chance.Read(exINI, pSection, "DodgeAttach.Chance");
	this->DodgeAttach_Anim.Read(exINI, pSection, "DodgeAttach.Anim");
	this->DodgeAttach_OnlyDodgePositiveDamage.Read(exINI, pSection, "DodgeAttach.OnlyDodgePositiveDamage");

	this->DistanceDamage.Read(exINI, pSection, "DistanceDamage");
	this->DistanceDamage_Add.Read(exINI, pSection, "DistanceDamage.Add");
	this->DistanceDamage_Multiply.Read(exINI, pSection, "DistanceDamage.Multiply");
	this->DistanceDamage_Add_Factor.Read(exINI, pSection, "DistanceDamage.Add.Factor");
	this->DistanceDamage_Multiply_Factor.Read(exINI, pSection, "DistanceDamage.Multiply.Factor");
	this->DistanceDamage_Max.Read(exINI, pSection, "DistanceDamage.Max");
	this->DistanceDamage_Min.Read(exINI, pSection, "DistanceDamage.Min");

	this->MoveDamageAttach_Damage.Read(exINI, pSection, "MoveDamageAttach.Damage");
	this->MoveDamageAttach_Duration.Read(exINI, pSection, "MoveDamageAttach.Duration");
	this->MoveDamageAttach_Warhead.Read(exINI, pSection, "MoveDamageAttach.Warhead");
	this->MoveDamageAttach_Delay.Read(exINI, pSection, "MoveDamageAttach.Delay");
	this->MoveDamageAttach_Anim.Read(exINI, pSection, "MoveDamageAttach.Anim");

	this->StopDamageAttach_Damage.Read(exINI, pSection, "StopDamageAttach.Damage");
	this->StopDamageAttach_Duration.Read(exINI, pSection, "StopDamageAttach.Duration");
	this->StopDamageAttach_Warhead.Read(exINI, pSection, "StopDamageAttach.Warhead");
	this->StopDamageAttach_Delay.Read(exINI, pSection, "StopDamageAttach.Delay");
	this->StopDamageAttach_Anim.Read(exINI, pSection, "StopDamageAttach.Anim");

	this->IgnoreArmorMultiplier.Read(exINI, pSection, "IgnoreArmorMultiplier");
	this->IgnoreDefense.Read(exINI, pSection, "IgnoreDefense");

	this->DetonateOnAllMapObjects.Read(exINI, pSection, "DetonateOnAllMapObjects");
	this->DetonateOnAllMapObjects_RequireVerses.Read(exINI, pSection, "DetonateOnAllMapObjects.RequireVerses");
	this->DetonateOnAllMapObjects_AffectTargets.Read(exINI, pSection, "DetonateOnAllMapObjects.AffectTargets");
	this->DetonateOnAllMapObjects_AffectHouses.Read(exINI, pSection, "DetonateOnAllMapObjects.AffectHouses");
	this->DetonateOnAllMapObjects_AffectTypes.Read(exINI, pSection, "DetonateOnAllMapObjects.AffectTypes");
	this->DetonateOnAllMapObjects_IgnoreTypes.Read(exINI, pSection, "DetonateOnAllMapObjects.IgnoreTypes");

	for (size_t i = 0; i < TemperatureTypeClass::Array.size(); i++)
	{
		const char* pName = TemperatureTypeClass::Array[i]->Name;
		Nullable<int> temperature;
		Nullable<bool> ignoreVersus;
		Nullable<bool> ignoreIronCurtain;

		const char* baseFlag = "Temperature.%s.%s";
		char key[0x50];
		_snprintf_s(key, _TRUNCATE, baseFlag, pName, "Addend");
		temperature.Read(exINI, pSection, key);
		_snprintf_s(key, _TRUNCATE, baseFlag, pName, "IgnoreVersus");
		ignoreVersus.Read(exINI, pSection, key);
		_snprintf_s(key, _TRUNCATE, baseFlag, pName, "IgnoreIronCurtain");
		ignoreIronCurtain.Read(exINI, pSection, key);

		if (temperature.isset())
		{
			Temperature.emplace(i, temperature);
			Temperature_IgnoreVersus.emplace(i, ignoreVersus.Get(true));
			Temperature_IgnoreIronCurtain.emplace(i, ignoreIronCurtain.Get(false));
		}
	}

	this->ChangeOwner.Read(exINI, pSection, "ChangeOwner");
	this->ChangeOwner_EffectToPsionics.Read(exINI, pSection, "ChangeOwner.EffectToPsionics");
	this->ChangeOwner_CountryIndex.Read(exINI, pSection, "ChangeOwner.CountryIndex");
	this->ChangeOwner_Types.Read(exINI, pSection, "ChangeOwner.Types");
	this->ChangeOwner_Ignore.Read(exINI, pSection, "ChangeOwner.Ignore");

	this->Theme.Read(pINI, pSection, "Theme");
	this->Theme_Queue.Read(exINI, pSection, "Theme.Queue");

	this->AttachTag.Read(pINI, pSection, "AttachTag");
	this->AttachTag_Imposed.Read(exINI, pSection, "AttachTag.Imposed");
	this->AttachTag_Types.Read(exINI, pSection, "AttachTag.Types");
	this->AttachTag_Ignore.Read(exINI, pSection, "AttachTag.Ignore");

	this->IgnoreDamageLimit.Read(exINI, pSection, "IgnoreDamageLimit");
	this->DamageLimitAttach_Duration.Read(exINI, pSection, "DamageLimitAttach.Duration");
	this->DamageLimitAttach_AllowMaxDamage.Read(exINI, pSection, "DamageLimitAttach.AllowMaxDamage");
	this->DamageLimitAttach_AllowMinDamage.Read(exINI, pSection, "DamageLimitAttach.AllowMinDamage");

	this->AbsorbPercent.Read(exINI, pSection, "AbsorbPercent");
	this->AbsorbMax.Read(exINI, pSection, "AbsorbMax");

	this->AttachEffects.Read(exINI, pSection, "AttachEffects");
	this->AttachEffects_Duration.Read(exINI, pSection, "AttachEffects.Duration");
	this->AttachEffects_Delay.Read(exINI, pSection, "AttachEffects.Delay");
	this->AttachEffects_IfExist_ResetTimer.Read(exINI, pSection, "AttachEffects.IfExist.ResetTimer");
	this->AttachEffects_IfExist_ResetAnim.Read(exINI, pSection, "AttachEffects.IfExist.ResetAnim");
	this->AttachEffects_IfExist_AddTimer.Read(exINI, pSection, "AttachEffects.IfExist.AddTimer");
	this->AttachEffects_IfExist_AddTimer_Cap.Read(exINI, pSection, "AttachEffects.IfExist.AddTimer.Cap");
	this->AttachEffects_RandomDuration.Read(exINI, pSection, "AttachEffects.RandomDuration");

	for (size_t i = 0; i < AttachEffects.size(); i++)
	{
		char key[0x40];
		Nullable<Vector2D<int>> interval;

		sprintf_s(key, "AttachEffect%d.RandomDuration.Interval", i);
		interval.Read(exINI, pSection, key);

		if (interval.isset())
			this->AttachEffects_RandomDuration_Interval[i] = interval;
	}

	this->Directional.Read(exINI, pSection, "Directional");
	this->Directional_Multiplier.Read(exINI, pSection, "Directional.Multiplier");

	this->ReduceSWTimer.Read(exINI, pSection, "ReduceSWTimer");
	this->ReduceSWTimer_Second.Read(exINI, pSection, "ReduceSWTimer.Second");
	this->ReduceSWTimer_Percent.Read(exINI, pSection, "ReduceSWTimer.Percent");
	this->ReduceSWTimer_SWTypes.Read(exINI, pSection, "ReduceSWTimer.SWTypes");
	this->ReduceSWTimer_MaxAffect.Read(exINI, pSection, "ReduceSWTimer.MaxAffect");
	this->ReduceSWTimer_NeedAffectSWBuilding.Read(exINI, pSection, "ReduceSWTimer.NeedAffectSWBuilding");

	// Ares tags
	// http://ares-developers.github.io/Ares-docs/new/warheads/general.html
	this->AffectsEnemies.Read(exINI, pSection, "AffectsEnemies");
	this->AffectsOwner.Read(exINI, pSection, "AffectsOwner");
	this->IsDetachedRailgun.Read(exINI, pSection, "IsDetachedRailgun");

	{
		this->Verses.Read(exINI, pSection, "Verses");

		while (static_cast<int>(Verses.size()) < CustomArmor::BaseArmorNumber)
			Verses.emplace_back(0.0);

		char key[0x30];

		for (const auto& pArmor : CustomArmor::Array)
		{
			Nullable<double> versus;
			sprintf_s(key, "Versus.%s", pArmor->Name);
			versus.Read(exINI, pSection, key);

			if (versus.isset())
			{
				Versus.emplace(pArmor->ArrayIndex, versus);
			}

			Nullable<bool> passiveAcquire;
			sprintf_s(key, "Versus.%s.PassiveAcquire", pArmor->Name);
			passiveAcquire.Read(exINI, pSection, key);

			if (passiveAcquire.isset())
			{
				Versus_PassiveAcquire.emplace(pArmor->ArrayIndex + CustomArmor::BaseArmorNumber, passiveAcquire);
			}

			Nullable<bool> retaliate;
			sprintf_s(key, "Versus.%s.Retaliate", pArmor->Name);
			retaliate.Read(exINI, pSection, key);

			if (retaliate.isset())
			{
				Versus_Retaliate.emplace(pArmor->ArrayIndex + CustomArmor::BaseArmorNumber, retaliate);
			}
		}

		for (int i = 0; i < CustomArmor::BaseArmorNumber; i++)
		{
			Nullable<bool> passiveAcquire;
			sprintf_s(key, "Versus.%s.PassiveAcquire", CustomArmor::BaseArmorName[i]);
			passiveAcquire.Read(exINI, pSection, key);

			if (passiveAcquire.isset())
			{
				Versus_PassiveAcquire.emplace(i, passiveAcquire);
			}

			Nullable<bool> retaliate;
			sprintf_s(key, "Versus.%s.Retaliate", CustomArmor::BaseArmorName[i]);
			retaliate.Read(exINI, pSection, key);

			if (retaliate.isset())
			{
				Versus_Retaliate.emplace(i, passiveAcquire);
			}
		}
	}
}

template <typename T>
void WarheadTypeExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->SpySat)
		.Process(this->BigGap)

		.Process(this->TransactMoney)
		.Process(this->TransactMoney_Display)
		.Process(this->TransactMoney_Display_Houses)
		.Process(this->TransactMoney_Display_AtFirer)
		.Process(this->TransactMoney_Display_Offset)

		.Process(this->SplashList)
		.Process(this->SplashList_PickRandom)
		.Process(this->RemoveDisguise)
		.Process(this->RemoveMindControl)
		.Process(this->AnimList_PickByDirection)
		.Process(this->AnimList_PickRandom)
		.Process(this->DecloakDamagedTargets)
		.Process(this->ShakeIsLocal)

		.Process(this->GattlingStage)
		.Process(this->GattlingRateUp)
		.Process(this->ReloadAmmo)

		.Process(this->Crit_Chance)
		.Process(this->Crit_ApplyChancePerTarget)
		.Process(this->Crit_ExtraDamage)
		.Process(this->Crit_Warhead)
		.Process(this->Crit_Affects)
		.Process(this->Crit_AffectsHouses)
		.Process(this->Crit_AnimList)
		.Process(this->Crit_AnimList_PickByDirection)
		.Process(this->Crit_AnimList_PickRandom)
		.Process(this->Crit_AnimOnAffectedTargets)
		.Process(this->Crit_AffectBelowPercent)
		.Process(this->Crit_SuppressWhenIntercepted)

		.Process(this->Transact)
		.Process(this->Transact_SpreadAmongTargets)
		.Process(this->Transact_Experience_Value)
		.Process(this->Transact_Experience_Veterancy)
		.Process(this->Transact_Experience_Source_Flat)
		.Process(this->Transact_Experience_Source_Percent)
		.Process(this->Transact_Experience_Source_Percent_CalcFromTarget)
		.Process(this->Transact_Experience_Target_Flat)
		.Process(this->Transact_Experience_Target_Percent)
		.Process(this->Transact_Experience_Target_Percent_CalcFromSource)

		.Process(this->Transact_Money_Source_Flat)
		.Process(this->Transact_Money_Source_Percent)
		.Process(this->Transact_Money_Source_Percent_CalcFromTarget)
		.Process(this->Transact_Money_Source_Display)
		.Process(this->Transact_Money_Source_Display_Houses)
		.Process(this->Transact_Money_Source_Display_Offset)
		.Process(this->Transact_Money_Source_Weapon)
		.Process(this->Transact_Money_Target_Flat)
		.Process(this->Transact_Money_Target_Percent)
		.Process(this->Transact_Money_Target_Percent_CalcFromSource)
		.Process(this->Transact_Money_Target_Display)
		.Process(this->Transact_Money_Target_Display_Houses)
		.Process(this->Transact_Money_Target_Display_Offset)
		.Process(this->Transact_Money_Target_Weapon)

		.Process(this->Transact_Health_Source_Flat)
		.Process(this->Transact_Health_Source_Percent)
		.Process(this->Transact_Health_Source_Percent_UseCurrentHealth)
		.Process(this->Transact_Health_Source_Percent_CalcFromTarget)
		.Process(this->Transact_Health_Target_Flat)
		.Process(this->Transact_Health_Target_Percent)
		.Process(this->Transact_Health_Target_Percent_UseCurrentHealth)
		.Process(this->Transact_Health_Target_Percent_CalcFromSource)

		.Process(this->MindControl_Anim)

		.Process(this->Shield_Penetrate)
		.Process(this->Shield_Break)
		.Process(this->Shield_BreakAnim)
		.Process(this->Shield_BreakWeapon)
		.Process(this->Shield_HitAnim)
		.Process(this->Shield_HitAnim_PickRandom)
		.Process(this->Shield_HitAnim_PickByDirection)
		.Process(this->Shield_AbsorbPercent)
		.Process(this->Shield_PassPercent)

		.Process(this->Shield_Steal)
		.Process(this->Shield_Assimilate_Rate)
		.Process(this->Shield_StealTargetType)
		.Process(this->Shield_StealTargetType_InitShieldHealthRate)

		.Process(this->Shield_Respawn_Duration)
		.Process(this->Shield_Respawn_Amount)
		.Process(this->Shield_Respawn_Rate)
		.Process(this->Shield_Respawn_ResetTimer)
		.Process(this->Shield_SelfHealing_Duration)
		.Process(this->Shield_SelfHealing_Amount)
		.Process(this->Shield_SelfHealing_Rate)
		.Process(this->Shield_SelfHealing_ResetTimer)
		.Process(this->Shield_AttachTypes)
		.Process(this->Shield_RemoveTypes)
		.Process(this->Shield_ReplaceOnly)
		.Process(this->Shield_ReplaceNonRespawning)
		.Process(this->Shield_InheritStateOnReplace)
		.Process(this->Shield_MinimumReplaceDelay)
		.Process(this->Shield_AffectTypes)

		.Process(this->NotHuman_DeathSequence)
		.Process(this->AllowDamageOnSelf)

		.Process(this->LaunchSW)
		.Process(this->LaunchSW_RealLaunch)
		.Process(this->LaunchSW_IgnoreInhibitors)
		.Process(this->LaunchSW_IgnoreDesignators)

		.Process(this->DebrisAnims)
		.Process(this->Debris_Conventional)

		.Process(this->MindControl_Threshold)
		.Process(this->MindControl_Threshold_Inverse)
		.Process(this->MindControl_AlternateDamage)
		.Process(this->MindControl_AlternateWarhead)
		.Process(this->MindControl_CanKill)

		.Process(this->Converts)
		.Process(this->Converts_From)
		.Process(this->Converts_To)
		.Process(this->Converts_Duration)
		.Process(this->Converts_Anim)
		.Process(this->Converts_RecoverAnim)
		.Process(this->Converts_DetachedBuildLimit)

		.Process(this->ClearPassengers)
		.Process(this->ReleasePassengers)
		.Process(this->DamagePassengers)
		.Process(this->DamagePassengers_AffectAllPassengers)

		.Process(this->DisableTurn_Duration)

		.Process(this->PaintBall_Color)
		.Process(this->PaintBall_Duration)
		.Process(this->PaintBall_IsDiscoColor)
		.Process(this->PaintBall_Colors)
		.Process(this->PaintBall_TransitionDuration)
		.Process(this->PaintBall_IgnoreTintStatus)

		.Process(this->AttackedWeapon_ForceNoResponse)
		.Process(this->AttackedWeapon_ResponseTechno)
		.Process(this->AttackedWeapon_NoResponseTechno)

		.Process(this->CanBeDodge)
		.Process(this->DodgeAttach_Duration)
		.Process(this->DodgeAttach_Houses)
		.Process(this->DodgeAttach_MaxHealthPercent)
		.Process(this->DodgeAttach_MinHealthPercent)
		.Process(this->DodgeAttach_Chance)
		.Process(this->DodgeAttach_Anim)
		.Process(this->DodgeAttach_OnlyDodgePositiveDamage)

		.Process(this->DistanceDamage)
		.Process(this->DistanceDamage_Add)
		.Process(this->DistanceDamage_Add_Factor)
		.Process(this->DistanceDamage_Multiply)
		.Process(this->DistanceDamage_Multiply_Factor)
		.Process(this->DistanceDamage_Max)
		.Process(this->DistanceDamage_Min)

		.Process(this->MoveDamageAttach_Damage)
		.Process(this->MoveDamageAttach_Duration)
		.Process(this->MoveDamageAttach_Warhead)
		.Process(this->MoveDamageAttach_Delay)
		.Process(this->MoveDamageAttach_Anim)

		.Process(this->StopDamageAttach_Damage)
		.Process(this->StopDamageAttach_Duration)
		.Process(this->StopDamageAttach_Warhead)
		.Process(this->StopDamageAttach_Delay)
		.Process(this->StopDamageAttach_Anim)

		.Process(this->Theme)
		.Process(this->Theme_Queue)
		.Process(this->AttachTag)
		.Process(this->AttachTag_Types)
		.Process(this->AttachTag_Ignore)
		.Process(this->AttachTag_Imposed)

		.Process(this->ChangeOwner)
		.Process(this->ChangeOwner_CountryIndex)
		.Process(this->ChangeOwner_EffectToPsionics)
		.Process(this->ChangeOwner_Ignore)
		.Process(this->ChangeOwner_Types)

		.Process(this->IgnoreDamageLimit)
		.Process(this->DamageLimitAttach_Duration)
		.Process(this->DamageLimitAttach_AllowMaxDamage)
		.Process(this->DamageLimitAttach_AllowMinDamage)

		.Process(this->AbsorbPercent)
		.Process(this->AbsorbMax)

		.Process(this->IgnoreArmorMultiplier)
		.Process(this->IgnoreDefense)

		.Process(this->DetonateOnAllMapObjects)
		.Process(this->DetonateOnAllMapObjects_RequireVerses)
		.Process(this->DetonateOnAllMapObjects_AffectTargets)
		.Process(this->DetonateOnAllMapObjects_AffectHouses)
		.Process(this->DetonateOnAllMapObjects_AffectTypes)
		.Process(this->DetonateOnAllMapObjects_IgnoreTypes)
		.Process(this->WasDetonatedOnAllMapObjects)

		.Process(this->HitDir)

		.Process(this->AttachEffects)
		.Process(this->AttachEffects_Duration)
		.Process(this->AttachEffects_Delay)
		.Process(this->AttachEffects_IfExist_ResetTimer)
		.Process(this->AttachEffects_IfExist_ResetAnim)
		.Process(this->AttachEffects_IfExist_AddTimer)
		.Process(this->AttachEffects_IfExist_AddTimer_Cap)
		.Process(this->AttachEffects_RandomDuration)
		.Process(this->AttachEffects_RandomDuration_Interval)

		.Process(this->Directional)
		.Process(this->Directional_Multiplier)

		.Process(this->Temperature)
		.Process(this->Temperature_IgnoreVersus)
		.Process(this->Temperature_IgnoreIronCurtain)

		.Process(this->ReduceSWTimer)
		.Process(this->ReduceSWTimer_Second)
		.Process(this->ReduceSWTimer_Percent)
		.Process(this->ReduceSWTimer_SWTypes)
		.Process(this->ReduceSWTimer_NeedAffectSWBuilding)
		.Process(this->ReduceSWTimer_MaxAffect)

		// Ares tags
		.Process(this->Verses)
		.Process(this->AffectsEnemies)
		.Process(this->AffectsOwner)
		.Process(this->IsDetachedRailgun)
		.Process(this->Versus)
		.Process(this->Versus_PassiveAcquire)
		.Process(this->Versus_Retaliate)
		;
}

void WarheadTypeExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<WarheadTypeClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void WarheadTypeExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<WarheadTypeClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool WarheadTypeExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm.Success();
}

bool WarheadTypeExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm.Success();
}

// =============================
// container

WarheadTypeExt::ExtContainer::ExtContainer() : Container("WarheadTypeClass") { }

WarheadTypeExt::ExtContainer::~ExtContainer() = default;

void WarheadTypeExt::ExtContainer::InvalidatePointer(void* ptr, bool bRemoved)
{

}

// =============================
// container hooks

DEFINE_HOOK(0x75D1A9, WarheadTypeClass_CTOR, 0x7)
{
	GET(WarheadTypeClass*, pItem, EBP);

	WarheadTypeExt::ExtMap.FindOrAllocate(pItem);

	return 0;
}

DEFINE_HOOK(0x75E5C8, WarheadTypeClass_SDDTOR, 0x6)
{
	GET(WarheadTypeClass*, pItem, ESI);

	WarheadTypeExt::ExtMap.Remove(pItem);

	return 0;
}

DEFINE_HOOK_AGAIN(0x75E2C0, WarheadTypeClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x75E0C0, WarheadTypeClass_SaveLoad_Prefix, 0x8)
{
	GET_STACK(WarheadTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	WarheadTypeExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x75E2AE, WarheadTypeClass_Load_Suffix, 0x7)
{
	WarheadTypeExt::ExtMap.LoadStatic();

	return 0;
}

DEFINE_HOOK(0x75E39C, WarheadTypeClass_Save_Suffix, 0x5)
{
	WarheadTypeExt::ExtMap.SaveStatic();

	return 0;
}

DEFINE_HOOK_AGAIN(0x75DEAF, WarheadTypeClass_LoadFromINI, 0x5)
DEFINE_HOOK(0x75DEA0, WarheadTypeClass_LoadFromINI, 0x5)
{
	GET(WarheadTypeClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, 0x150);

	WarheadTypeExt::ExtMap.LoadFromINI(pItem, pINI);

	return 0;
}
