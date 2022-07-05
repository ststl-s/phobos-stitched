#pragma once
#include <WarheadTypeClass.h>

#include <SuperWeaponTypeClass.h>
#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>
#include <New/Type/ShieldTypeClass.h>
#include <Ext/WeaponType/Body.h>

class WarheadTypeExt
{
public:
	using base_type = WarheadTypeClass;

	class ExtData final : public Extension<WarheadTypeClass>
	{
	public:

		Valueable<bool> SpySat;
		Valueable<bool> BigGap;
		Valueable<int> TransactMoney;
		Valueable<bool> TransactMoney_Display;
		Valueable<AffectedHouse> TransactMoney_Display_Houses;
		Valueable<bool> TransactMoney_Display_AtFirer;
		Valueable<Point2D> TransactMoney_Display_Offset;
		ValueableVector<AnimTypeClass*> SplashList;
		Valueable<bool> SplashList_PickRandom;
		Valueable<bool> RemoveDisguise;
		Valueable<bool> RemoveMindControl;
		Valueable<bool> AnimList_PickRandom;
		Nullable<bool> DecloakDamagedTargets;
		Valueable<bool> ShakeIsLocal;

		Valueable<bool> Transact;
		Nullable<int> Transact_Experience_Value;
		Nullable<int> Transact_Experience_Veterancy;
		Valueable<int> Transact_Experience_Source_Flat;
		Valueable<double> Transact_Experience_Source_Percent;
		Valueable<bool> Transact_Experience_Source_Percent_CalcFromTarget;
		Valueable<int> Transact_Experience_Target_Flat;
		Valueable<double> Transact_Experience_Target_Percent;
		Valueable<bool> Transact_Experience_Target_Percent_CalcFromSource;
		Valueable<bool> Transact_SpreadAmongTargets;

		Valueable<int> GattlingStage;
		Valueable<int> GattlingRateUp;
		Valueable<int> ReloadAmmo;

		Valueable<double> Crit_Chance;
		Valueable<bool> Crit_ApplyChancePerTarget;
		Valueable<int> Crit_ExtraDamage;
		Nullable<WarheadTypeClass*> Crit_Warhead;
		Valueable<AffectedTarget> Crit_Affects;
		ValueableVector<AnimTypeClass*> Crit_AnimList;
		Nullable<bool> Crit_AnimList_PickRandom;
		Valueable<bool> Crit_AnimOnAffectedTargets;
		Valueable<double> Crit_AffectBelowPercent;
		Valueable<bool> Crit_SuppressWhenIntercepted;

		Nullable<AnimTypeClass*> MindControl_Anim;

		Valueable<bool> Shield_Penetrate;
		Valueable<bool> Shield_Break;
		Nullable<AnimTypeClass*> Shield_BreakAnim;
		Nullable<AnimTypeClass*> Shield_HitAnim;
		Nullable<WeaponTypeClass*> Shield_BreakWeapon;

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

		ValueableVector<SuperWeaponTypeClass*> LaunchSW;
		Valueable<bool> LaunchSW_RealLaunch;
		Valueable<bool> LaunchSW_IgnoreInhibitors;

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

		Valueable<int> DisableTurn_Duration;

		Valueable<ColorStruct> PaintBall_Color;
		Valueable<int> PaintBall_Duration;
		Valueable<bool> PaintBall_IsDiscoColor;
		ValueableVector<ColorStruct> PaintBall_Colors;
		Valueable<int> PaintBall_TransitionDuration;

		Valueable<bool> AttackedWeapon_ForceNoResponse;
		ValueableVector<TechnoTypeClass*> AttackedWeapon_ResponseTechno;
		ValueableVector<TechnoTypeClass*> AttackedWeapon_NoResponseTechno;

		Nullable<bool> CanBeDodge;
		Valueable<int> DodgeAttach_Duration;
		Valueable<AffectedHouse> DodgeAttach_Houses;
		Valueable<double> DodgeAttach_MaxHealthPercent;
		Valueable<double> DodgeAttach_MinHealthPercent;
		Valueable<double> DodgeAttach_Chance;
		Nullable<AnimTypeClass*> DodgeAttach_Anim;

		Valueable<int> MoveDamageAttach_Damage;
		Valueable<int> MoveDamageAttach_Duration;
		Nullable<WarheadTypeClass*> MoveDamageAttach_Warhead;
		Valueable<int> MoveDamageAttach_Delay;

		Valueable<int> StopDamageAttach_Damage;
		Valueable<int> StopDamageAttach_Duration;
		Nullable<WarheadTypeClass*> StopDamageAttach_Warhead;
		Valueable<int> StopDamageAttach_Delay;

		Valueable<bool> IgnoreArmorMultiplier;
		Valueable<bool> IgnoreDefense;

		Valueable<bool> ChangeOwner;
		Valueable<bool> ChangeOwner_EffectToPsionics;
		Nullable<int> ChangeOwner_CountryIndex;
		NullableVector<TechnoTypeClass*> ChangeOwner_Types;
		NullableVector<TechnoTypeClass*> ChangeOwner_Ignore;

		PhobosFixedString<32U> Theme;
		Valueable<bool> Theme_Queue;

		PhobosFixedString<32U> AttachTag;
		Valueable<bool> AttachTag_Imposed;
		NullableVector<TechnoTypeClass*> AttachTag_Types;
		NullableVector<TechnoTypeClass*> AttachTag_Ignore;

		// Ares tags
		// http://ares-developers.github.io/Ares-docs/new/warheads/general.html
		Valueable<bool> AffectsEnemies;
		Nullable<bool> AffectsOwner;
		Valueable<bool> IsDetachedRailgun;

		double RandomBuffer;
		bool HasCrit;

	private:
		Valueable<double> Shield_Respawn_Rate_InMinutes;
		Valueable<double> Shield_SelfHealing_Rate_InMinutes;

	public:
		ExtData(WarheadTypeClass* OwnerObject) : Extension<WarheadTypeClass>(OwnerObject)
			, SpySat { false }
			, BigGap { false }
			, TransactMoney { 0 }
			, TransactMoney_Display { false }
			, TransactMoney_Display_Houses { AffectedHouse::All }
			, TransactMoney_Display_AtFirer { false }
			, TransactMoney_Display_Offset { { 0, 0 } }
			, SplashList {}
			, SplashList_PickRandom { false }
			, RemoveDisguise { false }
			, RemoveMindControl { false }
			, AnimList_PickRandom { false }
			, DecloakDamagedTargets {}
			, ShakeIsLocal { false }

			, Crit_Chance { 0.0 }
			, Crit_ApplyChancePerTarget { false }
			, Crit_ExtraDamage { 0 }
			, Crit_Warhead {}
			, Crit_Affects { AffectedTarget::All }
			, Crit_AnimList {}
			, Crit_AnimList_PickRandom {}
			, Crit_AnimOnAffectedTargets { false }
			, Crit_AffectBelowPercent { 1.0 }
			, Crit_SuppressWhenIntercepted { false }
			, RandomBuffer { 0.0 }
			, HasCrit { false }

			, Transact { false }
			, Transact_Experience_Value { 1 }
			, Transact_Experience_Source_Flat { 0 }
			, Transact_Experience_Source_Percent { 0.0 }
			, Transact_Experience_Source_Percent_CalcFromTarget { false }
			, Transact_Experience_Target_Flat { 0 }
			, Transact_Experience_Target_Percent { 0.0 }
			, Transact_Experience_Target_Percent_CalcFromSource { false }
			, Transact_SpreadAmongTargets { false }

			, GattlingStage { 0 }
			, GattlingRateUp { 0 }
			, ReloadAmmo { 0 }

			, MindControl_Anim {}

			, Shield_Penetrate { false }
			, Shield_Break { false }
			, Shield_BreakAnim {}
			, Shield_HitAnim {}
			, Shield_BreakWeapon {}
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

			, ClearPassengers{ false }
			, ReleasePassengers { false }
			, DamagePassengers { false }

			, DisableTurn_Duration { 0 }

			, AffectsEnemies { true }
			, AffectsOwner {}
			, LaunchSW {}
			, LaunchSW_RealLaunch { true }
			, LaunchSW_IgnoreInhibitors { false }

			, PaintBall_Color { { 255, 0, 0 } }
			, PaintBall_Duration { 0 }
			, PaintBall_IsDiscoColor { false }
			, PaintBall_Colors { }
			, PaintBall_TransitionDuration { 60 }

			, AttackedWeapon_ForceNoResponse { false }
			, AttackedWeapon_ResponseTechno {}
			, AttackedWeapon_NoResponseTechno {}

			, CanBeDodge {}
			, DodgeAttach_Duration { 0 }
			, DodgeAttach_Houses { AffectedHouse::All }
			, DodgeAttach_MaxHealthPercent { 1.0 }
			, DodgeAttach_MinHealthPercent { 0.0 }
			, DodgeAttach_Chance { 0.0 }
			, DodgeAttach_Anim {}

			, MoveDamageAttach_Damage { 0 }
			, MoveDamageAttach_Duration { 0 }
			, MoveDamageAttach_Warhead {}
			, MoveDamageAttach_Delay { 0 }

			, StopDamageAttach_Damage { 0 }
			, StopDamageAttach_Duration { 0 }
			, StopDamageAttach_Warhead {}
			, StopDamageAttach_Delay { 0 }

			, IgnoreArmorMultiplier { false }
			, IgnoreDefense { false }

			, ChangeOwner { false }
			, ChangeOwner_EffectToPsionics { false }
			, ChangeOwner_CountryIndex { -1 }
			, ChangeOwner_Types {}
			, ChangeOwner_Ignore {}

			, Theme { nullptr }
			, Theme_Queue { true }

			, AttachTag { nullptr }
			, AttachTag_Imposed { false }
			, AttachTag_Types {}
			, AttachTag_Ignore {}

			, IsDetachedRailgun { false }
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

		void ApplyRemoveDisguiseToInf(HouseClass* pHouse, TechnoClass* pTarget);
		void ApplyRemoveMindControl(HouseClass* pHouse, TechnoClass* pTarget);
		void ApplyCrit(HouseClass* pHouse, AbstractClass* pTarget, TechnoClass* Owner);
		void ApplyShieldModifiers(TechnoClass* pTarget);
		void ApplyGattlingStage(TechnoClass* pTarget, int Stage);
		void ApplyGattlingRateUp(TechnoClass* pTarget, int RateUp);
		void ApplyReloadAmmo(TechnoClass* pTarget, int ReloadAmount);
		void ApplyUpgrade(HouseClass* pHouse, TechnoClass* pTarget);
		void ApplyInvBlink(TechnoClass* pOwner, TechnoClass* pTarget, WeaponTypeExt::ExtData* pWeaponExt);
		void ApplyPaintBall(TechnoClass* pTarget);
		void ApplyDisableTurn(TechnoClass* pTarget);
		void ApplyAffectPassenger(TechnoClass* pTarget, WeaponTypeClass* pWeapon, BulletClass* pBullet);
		void ApplyCanDodge(TechnoClass* pTarget);
		void ApplyMoveDamage(TechnoClass* pTarget);
		void ApplyStopDamage(TechnoClass* pTarget);

		void ApplyChangeOwner(HouseClass* pHouse, TechnoClass* pTarget);
		void ApplyAttachTag(TechnoClass* pTarget);

	public:
		void Detonate(TechnoClass* pOwner, HouseClass* pHouse, BulletClass* pBullet, CoordStruct coords);
		bool CanTargetHouse(HouseClass* pHouse, TechnoClass* pTechno);
		void InterceptBullets(TechnoClass* pOwner, WeaponTypeClass* pWeapon, CoordStruct coords);

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

	static void DetonateAt(WarheadTypeClass* pThis, ObjectClass* pTarget, TechnoClass* pOwner, int damage);
	static void DetonateAt(WarheadTypeClass* pThis, const CoordStruct& coords, TechnoClass* pOwner, int damage);
};
