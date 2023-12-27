#pragma once

#include <WarheadTypeClass.h>
#include <SuperWeaponTypeClass.h>

#include <Ext/Bullet/Body.h>
#include <Ext/WeaponType/Body.h>

#include <Utilities/Container.h>

class ShieldTypeClass;

class WarheadTypeExt
{
public:
	using base_type = WarheadTypeClass;

	static constexpr DWORD Canary = 0x22222222;
	static constexpr size_t ExtPointerOffset = 0x18;

	class ExtData final : public Extension<WarheadTypeClass>
	{
	public:

		Valueable<int> Reveal;
		Valueable<bool> BigGap;
		Valueable<bool> SpySat;
		Valueable<int> TransactMoney;
		Valueable<bool> TransactMoney_Display;
		Valueable<AffectedHouse> TransactMoney_Display_Houses;
		Valueable<bool> TransactMoney_Display_AtFirer;
		Valueable<Point2D> TransactMoney_Display_Offset;
		ValueableVector<AnimTypeClass*> SplashList;
		Valueable<bool> SplashList_PickRandom;
		Valueable<bool> RemoveDisguise;
		Valueable<bool> RemoveMindControl;
		Valueable<bool> AnimList_PickByDirection;
		Valueable<bool> AnimList_PickRandom;
		Nullable<bool> DecloakDamagedTargets;
		Valueable<bool> ShakeIsLocal;

		Valueable<bool> Transact;
		Nullable<int> Transact_Experience_Value;
		Nullable<double> Transact_Experience_Percent;
		Nullable<int> Transact_Experience_Veterancy;
		Valueable<int> Transact_Experience_Source_Flat;
		Valueable<double> Transact_Experience_Source_Percent;
		Valueable<bool> Transact_Experience_Source_Percent_CalcFromTarget;
		Valueable<int> Transact_Experience_Target_Flat;
		Valueable<double> Transact_Experience_Target_Percent;
		Valueable<bool> Transact_Experience_Target_Percent_CalcFromSource;
		Valueable<bool> Transact_SpreadAmongTargets;

		Valueable<int> Transact_Money_Source_Flat;
		Valueable<double> Transact_Money_Source_Percent;
		Valueable<bool> Transact_Money_Source_Percent_CalcFromTarget;
		Valueable<bool> Transact_Money_Source_Display;
		Valueable<AffectedHouse> Transact_Money_Source_Display_Houses;
		Valueable<Point2D> Transact_Money_Source_Display_Offset;
		Valueable<WeaponTypeClass*> Transact_Money_Source_Weapon;
		Valueable<bool> Transact_Money_Source_CheckMoney;
		Valueable<int> Transact_Money_Target_Flat;
		Valueable<double> Transact_Money_Target_Percent;
		Valueable<bool> Transact_Money_Target_Percent_CalcFromSource;
		Valueable<bool> Transact_Money_Target_Display;
		Valueable<AffectedHouse> Transact_Money_Target_Display_Houses;
		Valueable<Point2D> Transact_Money_Target_Display_Offset;
		Valueable<WeaponTypeClass*> Transact_Money_Target_Weapon;
		Valueable<bool> Transact_Money_Target_CheckMoney;
		Valueable<bool> Transact_Money_WeaponOnlyCheckOneSide;

		Valueable<int> Transact_Health_Source_Flat;
		Valueable<double> Transact_Health_Source_Percent;
		Valueable<bool> Transact_Health_Source_Percent_UseCurrentHealth;
		Valueable<bool> Transact_Health_Source_Percent_CalcFromTarget;
		Valueable<int> Transact_Health_Target_Flat;
		Valueable<double> Transact_Health_Target_Percent;
		Valueable<bool> Transact_Health_Target_Percent_UseCurrentHealth;
		Valueable<bool> Transact_Health_Target_Percent_CalcFromSource;

		Valueable<int> GattlingStage;
		Valueable<int> GattlingRateUp;
		Valueable<int> ReloadAmmo;

		Valueable<bool> Crit_RandomPick;
		ValueableVector<double> Crit_Chance;
		ValueableVector<bool> Crit_ApplyChancePerTarget;
		ValueableVector<int> Crit_ExtraDamage;
		NullableVector<WarheadTypeClass*> Crit_Warhead;
		ValueableVector<AffectedTarget> Crit_Affects;
		ValueableVector<AffectedHouse> Crit_AffectsHouses;
		ValueableVector<ValueableVector<AnimTypeClass*>> Crit_AnimList;
		NullableVector<bool> Crit_AnimList_PickByDirection;
		NullableVector<bool> Crit_AnimList_PickRandom;
		ValueableVector<bool> Crit_AnimOnAffectedTargets;
		ValueableVector<double> Crit_AffectBelowPercent;
		ValueableVector<bool> Crit_SuppressWhenIntercepted;

		Nullable<AnimTypeClass*> MindControl_Anim;

		Valueable<bool> Shield_Penetrate;
		Valueable<bool> Shield_Break;
		Nullable<AnimTypeClass*> Shield_BreakAnim;
		Nullable<WeaponTypeClass*> Shield_BreakWeapon;
		ValueableVector<AnimTypeClass*> Shield_HitAnim;
		Valueable<bool> Shield_HitAnim_PickByDirection;
		Valueable<bool> Shield_HitAnim_PickRandom;

		Valueable<bool> Shield_Steal;
		Valueable<double> Shield_Assimilate_Rate;
		Valueable<bool> Shield_StealTargetType;
		Valueable<double> Shield_StealTargetType_InitShieldHealthRate;

		Valueable<bool> Converts;
		ValueableVector<TechnoTypeClass*> Converts_From;
		ValueableVector<TechnoTypeClass*> Converts_To;
		Valueable<int> Converts_Duration;
		Nullable<AnimTypeClass*> Converts_Anim;
		Nullable<AnimTypeClass*> Converts_RecoverAnim;
		Valueable<bool> Converts_DetachedBuildLimit;

		Nullable<double> Shield_AbsorbPercent;
		Nullable<double> Shield_PassPercent;

		Valueable<int> Shield_Respawn_Duration;
		Valueable<double> Shield_Respawn_Amount;
		Valueable<int> Shield_Respawn_Rate;
		Valueable<bool> Shield_Respawn_ResetTimer;
		Valueable<int> Shield_SelfHealing_Duration;
		Nullable<double> Shield_SelfHealing_Amount;
		Valueable<int> Shield_SelfHealing_Rate;
		Valueable<bool> Shield_SelfHealing_ResetTimer;

		ValueableVector<ShieldTypeClass*> Shield_AttachTypes;
		ValueableVector<ShieldTypeClass*> Shield_RemoveTypes;
		Valueable<bool> Shield_ReplaceOnly;
		Valueable<bool> Shield_ReplaceNonRespawning;
		Valueable<bool> Shield_InheritStateOnReplace;
		Valueable<int> Shield_MinimumReplaceDelay;
		ValueableVector<ShieldTypeClass*> Shield_AffectTypes;

		ValueableIdxVector<SuperWeaponTypeClass> LaunchSW;
		Valueable<bool> LaunchSW_RealLaunch;
		Valueable<bool> LaunchSW_IgnoreInhibitors;
		Valueable<bool> LaunchSW_IgnoreDesignators;

		NullableVector<AnimTypeClass*> DebrisAnims;
		Valueable<bool> Debris_Conventional;

		Valueable<double> MindControl_Threshold;
		Valueable<bool> MindControl_Threshold_Inverse;
		Nullable<int> MindControl_AlternateDamage;
		Nullable<WarheadTypeClass*> MindControl_AlternateWarhead;
		Valueable<bool> MindControl_CanKill;

		Valueable<int> NotHuman_DeathSequence;
		Valueable<bool> AllowDamageOnSelf;

		Valueable<bool> ClearPassengers;
		Valueable<bool> ReleasePassengers;
		Valueable<bool> DamagePassengers;
		Valueable<bool> DamagePassengers_AffectAllPassengers;

		Valueable<int> DisableTurn_Duration;

		Valueable<ColorStruct> PaintBall_Color;
		Valueable<int> PaintBall_Duration;
		Valueable<bool> PaintBall_IsDiscoColor;
		ValueableVector<ColorStruct> PaintBall_Colors;
		Valueable<int> PaintBall_TransitionDuration;
		Valueable<bool> PaintBall_IgnoreTintStatus;

		Valueable<bool> AttackedWeapon_ForceNoResponse;
		ValueableVector<TechnoTypeClass*> AttackedWeapon_ResponseTechno;
		ValueableVector<TechnoTypeClass*> AttackedWeapon_NoResponseTechno;

		Nullable<bool> CanBeDodge;

		Valueable<bool> DistanceDamage;
		Valueable<int> DistanceDamage_Add;
		Valueable<double> DistanceDamage_Add_Factor;
		Valueable<double> DistanceDamage_Multiply;
		Valueable<double> DistanceDamage_Multiply_Factor;
		Valueable<int> DistanceDamage_Max;
		Valueable<int> DistanceDamage_Min;
		Valueable<bool> DistanceDamage_PreventChangeSign;

		Valueable<bool> IgnoreArmorMultiplier;
		Valueable<bool> IgnoreDefense;
		Valueable<bool> IgnoreIronCurtain;
		Valueable<bool> IgnoreWarping;

		Valueable<bool> ChangeOwner;
		Valueable<bool> ChangeOwner_EffectToPsionics;
		Nullable<int> ChangeOwner_CountryIndex;
		NullableVector<TechnoTypeClass*> ChangeOwner_Types;
		NullableVector<TechnoTypeClass*> ChangeOwner_Ignore;

		PhobosFixedString<32U> Theme;
		Valueable<bool> Theme_Queue;
		Valueable<bool> Theme_Global;

		PhobosFixedString<32U> AttachTag;
		Valueable<bool> AttachTag_Imposed;
		NullableVector<TechnoTypeClass*> AttachTag_Types;
		NullableVector<TechnoTypeClass*> AttachTag_Ignore;

		Valueable<bool> IgnoreDamageLimit;

		Valueable<double> AbsorbPercent;
		Valueable<int> AbsorbMax;

		Valueable<bool> DetonateOnAllMapObjects;
		Valueable<bool> DetonateOnAllMapObjects_RequireVerses;
		Valueable<AffectedTarget> DetonateOnAllMapObjects_AffectTargets;
		Valueable<AffectedHouse> DetonateOnAllMapObjects_AffectHouses;
		ValueableVector<TechnoTypeClass*> DetonateOnAllMapObjects_AffectTypes;
		ValueableVector<TechnoTypeClass*> DetonateOnAllMapObjects_IgnoreTypes;

		std::unordered_map<size_t, int> Temperature;
		std::unordered_map<size_t, bool> Temperature_IgnoreVersus;
		std::unordered_map<size_t, bool> Temperature_IgnoreIronCurtain;

		ValueableVector<AttachEffectTypeClass*> AttachEffects;
		ValueableVector<int> AttachEffects_Duration;
		ValueableVector<bool> AttachEffects_RandomDuration;
		std::unordered_map<int, Vector2D<int>> AttachEffects_RandomDuration_Interval;
		ValueableVector<int> AttachEffects_Delay;
		ValueableVector<bool> AttachEffects_IfExist_ResetTimer;
		ValueableVector<bool> AttachEffects_IfExist_ResetAnim;
		ValueableVector<int> AttachEffects_IfExist_AddTimer;
		ValueableVector<int> AttachEffects_IfExist_AddTimer_Cap;
		ValueableVector<AttachEffectTypeClass*> DestroyAttachEffects;
		ValueableVector<AttachEffectTypeClass*> DelayAttachEffects;
		ValueableVector<int> DelayAttachEffects_Time;

		Valueable<AnimTypeClass*> UnitDeathAnim;

		Nullable<bool> Directional;
		Nullable<double> Directional_Multiplier;

		Valueable<bool> ReduceSWTimer;
		Valueable<bool> ReduceSWTimer_NeedAffectSWBuilding;
		Valueable<int> ReduceSWTimer_Second;
		Valueable<double> ReduceSWTimer_Percent;
		ValueableIdxVector<SuperWeaponTypeClass> ReduceSWTimer_SWTypes;
		Valueable<int> ReduceSWTimer_MaxAffect;
		Valueable<bool> ReduceSWTimer_ForceSet;

		Nullable<Mission> SetMission;

		Valueable<bool> DetachAttachment_Parent;
		Valueable<bool> DetachAttachment_Child;

		ValueableVector<AttachmentTypeClass*> AttachAttachment_Types;
		ValueableVector<TechnoTypeClass*> AttachAttachment_TechnoTypes;
		std::vector<CoordStruct> AttachAttachment_FLHs;
		std::vector<bool> AttachAttachment_IsOnTurrets;

		Valueable<int> Warp_Duration;
		Valueable<int> Warp_Cap;

		Valueable<int> WarpOut_Duration;

		Valueable<float> Temporal_CellSpread;

		Valueable<bool> ReleaseMindControl;
		Valueable<bool> ReleaseMindControl_Kill;

		Valueable<bool> AntiGravity;
		Valueable<int> AntiGravity_Height;
		Valueable<bool> AntiGravity_Destory;
		Valueable<int> AntiGravity_FallDamage;
		Valueable<double> AntiGravity_FallDamage_Factor;
		Nullable<WarheadTypeClass*> AntiGravity_FallDamage_Warhead;
		Valueable<AnimTypeClass*> AntiGravity_Anim;
		Nullable<int> AntiGravity_RiseRate;
		Nullable<int> AntiGravity_RiseRateMax;
		Nullable<int> AntiGravity_FallRate;
		Nullable<int> AntiGravity_FallRateMax;

		ValueableIdxVector<SuperWeaponTypeClass> AntiGravity_ConnectSW;
		Valueable<int> AntiGravity_ConnectSW_Deferment;
		Valueable<int> AntiGravity_ConnectSW_DefermentRandomMax;
		Valueable<int> AntiGravity_ConnectSW_Height;
		Valueable<bool> AntiGravity_ConnectSW_UseParachute;
		Valueable<OwnerHouseKind> AntiGravity_ConnectSW_Owner;
		Valueable<WeaponTypeClass*> AntiGravity_ConnectSW_Weapon;
		Valueable<AnimTypeClass*> AntiGravity_ConnectSW_Anim;
		Valueable<unsigned short> AntiGravity_ConnectSW_Facing;
		Valueable<bool>AntiGravity_ConnectSW_RandomFacing;
		Valueable<Mission> AntiGravity_ConnectSW_Mission;
		Valueable<bool> AntiGravity_ConnectSW_Destory;
		Valueable<int> AntiGravity_ConnectSW_DestoryHeight;
		Valueable<bool> AntiGravity_ConnectSW_AlwaysFall;

		Valueable<int> RadarEvent;

		// Ares tags
		// http://ares-developers.github.io/Ares-docs/new/warheads/general.html
		ValueableVector<double> Verses;
		Valueable<bool> AffectsEnemies;
		Nullable<bool> AffectsOwner;
		Valueable<bool> IsDetachedRailgun;
		Valueable<bool> MindControl_Permanent;
		std::unordered_map<int, double> Versus;
		std::unordered_map<int, bool> Versus_HasValue;
		std::unordered_map<int, bool> Versus_PassiveAcquire;
		std::unordered_map<int, bool> Versus_Retaliate;
		Nullable<double> AlliesDamageMulti;

		double RandomBuffer;
		bool HasCrit;
		bool WasDetonatedOnAllMapObjects;
		int HitDir;

	private:
		Valueable<double> Shield_Respawn_Rate_InMinutes;
		Valueable<double> Shield_SelfHealing_Rate_InMinutes;

	public:
		ExtData(WarheadTypeClass* OwnerObject) : Extension<WarheadTypeClass>(OwnerObject)
			, Reveal { 0 }
			, BigGap { false }
			, SpySat { false }
			, TransactMoney { 0 }
			, TransactMoney_Display { false }
			, TransactMoney_Display_Houses { AffectedHouse::All }
			, TransactMoney_Display_AtFirer { false }
			, TransactMoney_Display_Offset { { 0, 0 } }
			, SplashList {}
			, SplashList_PickRandom { false }
			, RemoveDisguise { false }
			, RemoveMindControl { false }
			, AnimList_PickByDirection { false }
			, AnimList_PickRandom { false }
			, DecloakDamagedTargets {}
			, ShakeIsLocal { false }

			, Crit_RandomPick { false }
			, Crit_Chance {}
			, Crit_ApplyChancePerTarget {}
			, Crit_ExtraDamage {}
			, Crit_Warhead {}
			, Crit_Affects {}
			, Crit_AffectsHouses {}
			, Crit_AnimList {}
			, Crit_AnimList_PickByDirection {}
			, Crit_AnimList_PickRandom {}
			, Crit_AnimOnAffectedTargets {}
			, Crit_AffectBelowPercent {}
			, Crit_SuppressWhenIntercepted {}

			, Transact { false }
			, Transact_Experience_Value { 0 }
			, Transact_Experience_Percent {}
			, Transact_Experience_Veterancy {}
			, Transact_Experience_Source_Flat { 0 }
			, Transact_Experience_Source_Percent { 0.0 }
			, Transact_Experience_Source_Percent_CalcFromTarget { false }
			, Transact_Experience_Target_Flat { 0 }
			, Transact_Experience_Target_Percent { 0.0 }
			, Transact_Experience_Target_Percent_CalcFromSource { false }
			, Transact_SpreadAmongTargets { false }

			, Transact_Money_Source_Flat { 0 }
			, Transact_Money_Source_Percent { 0.0 }
			, Transact_Money_Source_Percent_CalcFromTarget { false }
			, Transact_Money_Source_Display { false }
			, Transact_Money_Source_Display_Houses { AffectedHouse::All }
			, Transact_Money_Source_Display_Offset { { 0, 0 } }
			, Transact_Money_Source_Weapon {}
			, Transact_Money_Source_CheckMoney { false }
			, Transact_Money_Target_Flat { 0 }
			, Transact_Money_Target_Percent { 0.0 }
			, Transact_Money_Target_Percent_CalcFromSource { false }
			, Transact_Money_Target_Display { false }
			, Transact_Money_Target_Display_Houses { AffectedHouse::All }
			, Transact_Money_Target_Display_Offset { { 0, 0 } }
			, Transact_Money_Target_Weapon {}
			, Transact_Money_Target_CheckMoney { false }
			, Transact_Money_WeaponOnlyCheckOneSide { false }

			, Transact_Health_Source_Flat { 0 }
			, Transact_Health_Source_Percent { 0.0 }
			, Transact_Health_Source_Percent_UseCurrentHealth { false }
			, Transact_Health_Source_Percent_CalcFromTarget { false }
			, Transact_Health_Target_Flat { 0 }
			, Transact_Health_Target_Percent { 0.0 }
			, Transact_Health_Target_Percent_UseCurrentHealth { false }
			, Transact_Health_Target_Percent_CalcFromSource { false }

			, GattlingStage { 0 }
			, GattlingRateUp { 0 }
			, ReloadAmmo { 0 }

			, MindControl_Anim {}

			, Shield_Penetrate { false }
			, Shield_Break { false }
			, Shield_BreakAnim {}
			, Shield_BreakWeapon {}
			, Shield_HitAnim {}
			, Shield_HitAnim_PickByDirection { false }
			, Shield_HitAnim_PickRandom { false }
			, Shield_AbsorbPercent {}
			, Shield_PassPercent {}

			, Shield_Steal { false }
			, Shield_Assimilate_Rate { 1.0 }
			, Shield_StealTargetType { false }
			, Shield_StealTargetType_InitShieldHealthRate { 0.0 }

			, Shield_Respawn_Duration { 0 }
			, Shield_Respawn_Amount { 0.0 }
			, Shield_Respawn_Rate { -1 }
			, Shield_Respawn_Rate_InMinutes { -1.0 }
			, Shield_Respawn_ResetTimer { false }
			, Shield_SelfHealing_Duration { 0 }
			, Shield_SelfHealing_Amount { }
			, Shield_SelfHealing_Rate { -1 }
			, Shield_SelfHealing_Rate_InMinutes { -1.0 }
			, Shield_SelfHealing_ResetTimer { false }
			, Shield_AttachTypes {}
			, Shield_RemoveTypes {}
			, Shield_ReplaceOnly { false }
			, Shield_ReplaceNonRespawning { false }
			, Shield_InheritStateOnReplace { false }
			, Shield_MinimumReplaceDelay { 0 }
			, Shield_AffectTypes {}

			, NotHuman_DeathSequence { -1 }
			, AllowDamageOnSelf { false }
			, DebrisAnims {}
			, Debris_Conventional { false }

			, MindControl_Threshold { 1.0 }
			, MindControl_Threshold_Inverse { false }
			, MindControl_AlternateDamage {}
			, MindControl_AlternateWarhead {}
			, MindControl_CanKill { false }

			, Converts {}
			, Converts_From {}
			, Converts_To {}
			, Converts_Duration { -1 }
			, Converts_Anim { nullptr }
			, Converts_RecoverAnim { nullptr }
			, Converts_DetachedBuildLimit { false }

			, ClearPassengers { false }
			, ReleasePassengers { false }
			, DamagePassengers { false }
			, DamagePassengers_AffectAllPassengers { false }

			, DisableTurn_Duration { 0 }

			, LaunchSW {}
			, LaunchSW_RealLaunch { true }
			, LaunchSW_IgnoreInhibitors { false }
			, LaunchSW_IgnoreDesignators { true }

			, PaintBall_Color { { 255, 0, 0 } }
			, PaintBall_Duration { 0 }
			, PaintBall_IsDiscoColor { false }
			, PaintBall_Colors { }
			, PaintBall_TransitionDuration { 60 }
			, PaintBall_IgnoreTintStatus { false }

			, AttackedWeapon_ForceNoResponse { false }
			, AttackedWeapon_ResponseTechno {}
			, AttackedWeapon_NoResponseTechno {}

			, CanBeDodge {}

			, DistanceDamage { false }
			, DistanceDamage_Add { 0 }
			, DistanceDamage_Add_Factor { 1.0 }
			, DistanceDamage_Multiply { 1.0 }
			, DistanceDamage_Multiply_Factor { 1.0 }
			, DistanceDamage_Max { INT_MAX }
			, DistanceDamage_Min { -INT_MAX }
			, DistanceDamage_PreventChangeSign { true }

			, IgnoreArmorMultiplier { false }
			, IgnoreDefense { false }
			, IgnoreIronCurtain { false }
			, IgnoreWarping { false }

			, ChangeOwner { false }
			, ChangeOwner_EffectToPsionics { false }
			, ChangeOwner_CountryIndex { -1 }
			, ChangeOwner_Types {}
			, ChangeOwner_Ignore {}

			, Theme { nullptr }
			, Theme_Queue { true }
			, Theme_Global { false }

			, AttachTag { nullptr }
			, AttachTag_Imposed { false }
			, AttachTag_Types {}
			, AttachTag_Ignore {}

			, IgnoreDamageLimit { false }

			, AbsorbPercent { 0.0 }
			, AbsorbMax { -1 }

			, DetonateOnAllMapObjects { false }
			, DetonateOnAllMapObjects_RequireVerses { false }
			, DetonateOnAllMapObjects_AffectTargets { AffectedTarget::All }
			, DetonateOnAllMapObjects_AffectHouses { AffectedHouse::All }
			, DetonateOnAllMapObjects_AffectTypes {}
			, DetonateOnAllMapObjects_IgnoreTypes {}

			, Temperature { 0 }
			, Temperature_IgnoreVersus { true }
			, Temperature_IgnoreIronCurtain { false }

			, AttachEffects {}
			, AttachEffects_Duration {}
			, AttachEffects_Delay {}
			, AttachEffects_IfExist_ResetTimer {}
			, AttachEffects_IfExist_ResetAnim {}
			, AttachEffects_IfExist_AddTimer {}
			, AttachEffects_IfExist_AddTimer_Cap {}
			, AttachEffects_RandomDuration {}
			, AttachEffects_RandomDuration_Interval {}
			, DestroyAttachEffects {}
			, DelayAttachEffects {}
			, DelayAttachEffects_Time {}

			, Directional {}
			, Directional_Multiplier {}

			, ReduceSWTimer { false }
			, ReduceSWTimer_Second { 0 }
			, ReduceSWTimer_Percent { 0.0 }
			, ReduceSWTimer_SWTypes {}
			, ReduceSWTimer_NeedAffectSWBuilding { true }
			, ReduceSWTimer_MaxAffect { 1 }
			, ReduceSWTimer_ForceSet { false }

			, SetMission { }

			, DetachAttachment_Parent { false }
			, DetachAttachment_Child { false }

			, AttachAttachment_Types {}
			, AttachAttachment_TechnoTypes {}
			, AttachAttachment_FLHs {}
			, AttachAttachment_IsOnTurrets {}

			, Warp_Duration { 0 }
			, Warp_Cap { 0 }

			, WarpOut_Duration { 0 }

			, Temporal_CellSpread { 0.0 }

			, ReleaseMindControl { false }
			, ReleaseMindControl_Kill { false }

			, AntiGravity { false }
			, AntiGravity_Height { 0 }
			, AntiGravity_Destory { false }
			, AntiGravity_FallDamage { 0 }
			, AntiGravity_FallDamage_Factor { 0.0 }
			, AntiGravity_FallDamage_Warhead {}
			, AntiGravity_Anim { nullptr }
			, AntiGravity_RiseRate {}
			, AntiGravity_RiseRateMax {}
			, AntiGravity_FallRate {}
			, AntiGravity_FallRateMax {}

			, AntiGravity_ConnectSW {}
			, AntiGravity_ConnectSW_Deferment { 0 }
			, AntiGravity_ConnectSW_DefermentRandomMax { 0 }
			, AntiGravity_ConnectSW_Height { RulesClass::Instance->FlightLevel }
			, AntiGravity_ConnectSW_UseParachute { false }
			, AntiGravity_ConnectSW_Owner { OwnerHouseKind::Default }
			, AntiGravity_ConnectSW_Weapon { nullptr }
			, AntiGravity_ConnectSW_Anim { nullptr }
			, AntiGravity_ConnectSW_Facing { 0 }
			, AntiGravity_ConnectSW_RandomFacing { false }
			, AntiGravity_ConnectSW_Mission { Mission::Guard }
			, AntiGravity_ConnectSW_Destory { false }
			, AntiGravity_ConnectSW_DestoryHeight { -1 }
			, AntiGravity_ConnectSW_AlwaysFall { false }

			, Verses(11, 1.0)
			, Versus {}
			, Versus_HasValue {}
			, Versus_Retaliate {}
			, Versus_PassiveAcquire {}
			, IsDetachedRailgun { false }
			, AffectsEnemies { true }
			, AffectsOwner {}
			, MindControl_Permanent { false }

			, RandomBuffer { 0.0 }
			, HasCrit { false }
			, WasDetonatedOnAllMapObjects { false }
			, HitDir { -1 }

			, RadarEvent { -1 }

			, UnitDeathAnim { nullptr }

			, AlliesDamageMulti {}
		{
			this->PaintBall_Colors.push_back({ 255, 0, 0 });
		}

	private:
		void DetonateOnOneUnit(HouseClass* pHouse, TechnoClass* pTarget, TechnoClass* pOwner, BulletClass* pBullet, bool bulletWasIntercepted = false);
		void DetonateOnCell(HouseClass* pHouse, CellClass* pTarget, TechnoClass* pOwner = nullptr);
		void DetonateOnAllUnits(HouseClass* pHouse, const CoordStruct coords, const float cellSpread, TechnoClass* pOwner, BulletClass* pBullet, bool bulletWasIntercepted = false);
		void TransactOnOneUnit(TechnoClass* pTarget, TechnoClass* pOwner, int targets);
		void TransactOnAllUnits(HouseClass* pHouse, const CoordStruct coords, const float cellSpread, TechnoClass* pOwner, WarheadTypeExt::ExtData* pWHTypeExt);
		int TransactGetValue(TechnoClass* pTarget, TechnoClass* pOwner, int flat, double percent, boolean calcFromTarget, int targetValue, int ownerValue);
		std::vector<std::vector<int>> TransactGetSourceAndTarget(TechnoClass* pTarget, TechnoTypeClass* pTargetType, TechnoClass* pOwner, TechnoTypeClass* pOwnerType, int targets);
		int TransactOneValue(TechnoClass* pTechno, TechnoTypeClass* pTechnoType, int transactValue, TransactValueType valueType);
		int TransactMoneyOnOneUnit(TechnoClass* pTarget, TechnoClass* pOwner, int targets);

		void ApplyRemoveDisguiseToInf(HouseClass* pHouse, TechnoClass* pTarget);
		void ApplyRemoveMindControl(HouseClass* pHouse, TechnoClass* pTarget);
		void ApplyCrit(HouseClass* pHouse, AbstractClass* pTarget, TechnoClass* Owner, int idx);
		void ApplyShieldModifiers(TechnoClass* pTarget);
		void ApplyGattlingStage(TechnoClass* pTarget, int Stage);
		void ApplyGattlingRateUp(TechnoClass* pTarget, int RateUp);
		void ApplyReloadAmmo(TechnoClass* pTarget, int ReloadAmount);
		void ApplyUpgrade(HouseClass* pHouse, TechnoClass* pTarget);
		void ApplyInvBlink(TechnoClass* pOwner, HouseClass* pHouse, const std::vector<TechnoClass*>& vTargets, const WeaponTypeExt::ExtData* pWeaponExt);
		void ApplyAttachTargetToSelfAttachments(TechnoClass* pOwner, HouseClass* pHouse, const std::vector<TechnoClass*>& vTargets, const WeaponTypeExt::ExtData* pWeaponExt);
		void ApplyPaintBall(TechnoClass* pTarget);
		void ApplyDisableTurn(TechnoClass* pTarget);
		void ApplyAffectPassenger(TechnoClass* pTarget, WeaponTypeClass* pWeapon, BulletClass* pBullet);
		void ApplyChangeOwner(HouseClass* pHouse, TechnoClass* pTarget);
		void ApplyAttachTag(TechnoClass* pTarget);
		void ApplyAttachEffects(TechnoClass* pOwner, TechnoClass* pTarget);
		void ApplyTemperature(TechnoClass* pTarget);
		void ApplyDirectional(BulletClass* pBullet, TechnoClass* pTarget);
		void ApplyReduceSWTimer(HouseClass* pHouse);
		bool ApplyReduceSWTimer(TechnoClass* pTarget);
		void ApplyUnitDeathAnim(HouseClass* pHouse, TechnoClass* pTarget);
		void ApplyForceMission(TechnoClass* pTarget);
		void ApplyDetachParent(TechnoClass* pTarget);
		void ApplyDetachChild(TechnoClass* pTarget);
		void ApplyAttachAttachment(TechnoClass* pTarget, HouseClass* pHouse);
		void ApplyWarp(TechnoClass* pTarget);
		void ApplyWarpOut(TechnoClass* pTarget);
		void ApplyCellSpreadMindControl(TechnoClass* pOwner, TechnoClass* pTarget);
		void ApplyReleaseMindControl(TechnoClass* pOwner, TechnoClass* pTarget);
		void ApplyPermanentMindControl(TechnoClass* pOwner, HouseClass* pHouse, TechnoClass* pTarget);
		void ApplyAntiGravity(TechnoClass* pTarget, HouseClass* pHouse);

		double GetCritChance(TechnoClass* pFirer, int idx);
	public:
		void Detonate(TechnoClass* pOwner, HouseClass* pHouse, BulletExt::ExtData* pBullet, CoordStruct coords);
		bool CanTargetHouse(HouseClass* pHouse, TechnoClass* pTechno);
		void InterceptBullets(TechnoClass* pOwner, WeaponTypeClass* pWeapon, CoordStruct coords);
		bool EligibleForFullMapDetonation(TechnoClass* pTechno, HouseClass* pOwner);

		virtual ~ExtData() = default;
		virtual void LoadFromINIFile(CCINIClass* pINI) override;
		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }
		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<WarheadTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override;
	};

	static ExtContainer ExtMap;
	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static void DetonateAt(WarheadTypeClass* pThis, ObjectClass* pTarget, TechnoClass* pOwner, int damage, HouseClass* pHouse = nullptr);
	static void DetonateAt(WarheadTypeClass* pThis, const CoordStruct& coords, TechnoClass* pOwner, int damage, HouseClass* pHouse = nullptr);
};
