#pragma once

#include <BulletClass.h>
#include <WeaponTypeClass.h>

#include <Utilities/Container.h>
#include <Utilities/Enum.h>
#include <Utilities/Template.h>

#include <New/Type/StrafingLaserTypeClass.h>

class AttachEffectTypeClass;
class AttachmentTypeClass;
class IonCannonTypeClass;
class RadTypeClass;

class WeaponTypeExt
{
public:
	using base_type = WeaponTypeClass;

	static constexpr DWORD Canary = 0x22222222;
	static constexpr size_t ExtPointerOffset = 0x18;

	class ExtData final : public Extension<WeaponTypeClass>
	{
	public:

		Valueable<double> DiskLaser_Radius;
		Valueable<int> DiskLaser_Circumference;
		Valueable<int> LaserThickness;
		Valueable<RadTypeClass*> RadType;
		Valueable<bool> Bolt_Disable1;
		Valueable<bool> Bolt_Disable2;
		Valueable<bool> Bolt_Disable3;
		Nullable<int> Strafing_Shots;
		Valueable<bool> Strafing_SimulateBurst;
		Valueable<AffectedTarget> CanTarget;
		Valueable<AffectedHouse> CanTargetHouses;
		ValueableVector<int> Burst_Delays;
		Valueable<AreaFireTarget> AreaFire_Target;
		Nullable<WeaponTypeClass*> FeedbackWeapon;
		Valueable<bool> Laser_IsSingleColor;
		Nullable<PartialVector2D<int>> ROF_RandomDelay;
		ValueableVector<AttachEffectTypeClass*> AttachEffect_RequiredTypes;
		ValueableVector<AttachEffectTypeClass*> AttachEffect_DisallowedTypes;
		Valueable<bool> AttachEffect_IgnoreFromSameSource;
		Valueable<bool> BlinkWeapon;
		Valueable<bool> InvBlinkWeapon;
		Valueable<bool> BlinkWeapon_Overlap;
		Valueable<bool> BlinkWeapon_KillTarget;
		ValueableVector<AnimTypeClass*> BlinkWeapon_TargetAnim;
		ValueableVector<AnimTypeClass*> BlinkWeapon_SelfAnim;
		Nullable<IonCannonTypeClass*> IonCannonType;
		Valueable<bool> IsBeamCannon;
		Valueable<int> BeamCannon_Length;
		Valueable<CoordStruct> BeamCannon_Length_Start;
		Valueable<int> BeamCannon_LengthIncrease;
		Valueable<int> BeamCannon_LengthIncreaseAcceleration;
		Valueable<int> BeamCannon_LengthIncreaseMax;
		Valueable<int> BeamCannon_LengthIncreaseMin;
		Valueable<bool> BeamCannon_DrawEBolt;
		Valueable<int> BeamCannon_EleHeight;
		Nullable<ColorStruct> BeamCannon_InnerColor;
		Nullable<ColorStruct> BeamCannon_OuterColor;
		Nullable<ColorStruct> BeamCannon_OuterSpread;
		Valueable<int> BeamCannon_Duration;
		Valueable<int> BeamCannon_Thickness;
		Nullable<WeaponTypeClass*> BeamCannonWeapon;
		Valueable<bool> BeamCannon_DrawLaser;
		Valueable<int> BeamCannon_LaserHeight;
		Valueable<bool> BeamCannon_DrawFromSelf;
		Valueable<CoordStruct> BeamCannon_DrawFromSelf_FLH;
		Valueable<int> BeamCannon_ROF;
		Valueable<int> BeamCannon_Burst;
		std::vector<CoordStruct> BeamCannon_FLH;
		std::vector<CoordStruct> BeamCannon_Start;
		Valueable<bool> PassengerDeletion;
		Valueable<bool> PassengerTransport;
		Valueable<bool> PassengerTransport_UsePassengerData;
		Valueable<bool> PassengerTransport_Overlap;
		Valueable<bool> PassengerTransport_MoveToTarget;
		Valueable<AffectedHouse> PassengerTransport_MoveToTargetAllowHouses;
		Valueable<bool> PassengerTransport_UseParachute;
		Valueable<bool> PassengerTransport_MoveToTargetAllowHouses_IgnoreNeturalHouse;
		Valueable<bool> SelfTransport;
		Valueable<bool> SelfTransport_UseData;
		Valueable<bool> SelfTransport_Overlap;
		Valueable<AnimTypeClass*> SelfTransport_Anim;
		Valueable<bool> SelfTransport_MoveToTarget;
		Valueable<AffectedHouse> SelfTransport_MoveToTargetAllowHouses;
		Valueable<bool> SelfTransport_MoveToTargetAllowHouses_IgnoreNeturalHouse;
		Valueable<bool> SelfTransport_UseParachute;
		Valueable<bool> FacingTarget;

		Nullable<bool> KickOutPassenger;
		Nullable<bool> KickOutPassenger_Parachute;

		Valueable<bool> OnlyAllowOneFirer;
		Valueable<int> OnlyAllowOneFirer_Count;
		ValueableVector<WeaponTypeClass*> OnlyAllowOneFirer_OtherWeapons;
		ValueableVector<WeaponTypeClass*> OnlyAllowOneFirer_IgnoreWeapons;
		Valueable<bool> OnlyAllowOneFirer_ResetImmediately;

		ValueableVector<WeaponTypeClass*> AttachWeapons;
		Valueable<bool> AttachWeapons_DetachedROF;
		Valueable<bool> AttachWeapons_Burst_InvertL;
		std::vector<CoordStruct> AttachWeapons_FLH;
		Valueable<bool> AttachWeapons_UseAmmo;
		ValueableVector<bool> AttachWeapons_DetachedFire;

		Valueable<bool> ElectricLaser;
		Valueable<int> ElectricLaser_Count;
		Nullable<int> ElectricLaser_Length;
		Valueable<int> ElectricLaser_Timer;
		ValueableVector<ColorStruct> ElectricLaser_Color;
		ValueableVector<double> ElectricLaser_Amplitude;
		ValueableVector<int> ElectricLaser_Duration;
		ValueableVector<int> ElectricLaser_Thickness;
		ValueableVector<bool> ElectricLaser_IsSupported;

		Nullable<AnimTypeClass*> DelayedFire_Anim;
		Valueable<int> DelayedFire_Anim_LoopCount;
		Valueable<bool> DelayedFire_Anim_UseFLH;
		Valueable<int> DelayedFire_DurationTimer;

		Valueable<bool> AttachAttachment_SelfToTarget;
		Valueable<bool> AttachAttachment_TargetToSelf;
		Valueable<AttachmentTypeClass*> AttachAttachment_Type;
		Valueable<CoordStruct> AttachAttachment_FLH;
		Valueable<bool> AttachAttachment_IsOnTurret;

		Valueable<int> Ammo;

		Valueable<int> ExtraBurst;
		Valueable<WeaponTypeClass*> ExtraBurst_Weapon;
		Valueable<AffectedHouse> ExtraBurst_Houses;
		Valueable<bool> ExtraBurst_AlwaysFire;
		std::vector<CoordStruct> ExtraBurst_FLH;
		Valueable<unsigned short> ExtraBurst_FacingRange;
		Valueable<bool> ExtraBurst_InvertL;
		Valueable<bool> ExtraBurst_Spread;
		Valueable<bool> ExtraBurst_UseAmmo;
		Valueable<bool> ExtraBurst_SkipNeutralTarget;

		ValueableVector<WarheadTypeClass*> ExtraWarheads;
		ValueableVector<int> ExtraWarheads_DamageOverrides;

		Valueable<int> Laser_Thickness;
		Valueable<bool> IsTrackingLaser;

		NullableVector<StrafingLaserTypeClass*> StrafingLasers;
		Valueable<bool> StrafingLasers_Random;

		Nullable<bool> DecloakToFire;

		Nullable<ColorStruct> EBolt_Color1;
		Nullable<ColorStruct> EBolt_Color2;
		Nullable<ColorStruct> EBolt_Color3;

		Nullable<ColorStruct> Beam_Color;
		Nullable<int> Beam_Duration;
		Valueable<bool> Beam_IsHouseColor;
		Valueable<double> Beam_Amplitude;

		Valueable<AnimTypeClass*> PreFireAnim;
		ValueableIdx<VocClass> PreFireReport;

		Valueable<Leptons> ProjectileRange;

		ExtData(WeaponTypeClass* OwnerObject) : Extension<WeaponTypeClass>(OwnerObject)
			, DiskLaser_Radius { 38.2 }
			, DiskLaser_Circumference { 240 }
			, LaserThickness { 1 }
			, RadType {}
			, Bolt_Disable1 { false }
			, Bolt_Disable2 { false }
			, Bolt_Disable3 { false }
			, Strafing_Shots { }
			, Strafing_SimulateBurst { false }
			, CanTarget { AffectedTarget::All }
			, CanTargetHouses { AffectedHouse::All }
			, Burst_Delays {}
			, AreaFire_Target { AreaFireTarget::Base }
			, FeedbackWeapon {}
			, Laser_IsSingleColor { false }
			, ROF_RandomDelay {}
			, AttachEffect_RequiredTypes {}
			, AttachEffect_DisallowedTypes {}
			, AttachEffect_IgnoreFromSameSource { false }
			, BlinkWeapon { false }
			, InvBlinkWeapon { false }
			, BlinkWeapon_Overlap { false }
			, BlinkWeapon_KillTarget { false }
			, BlinkWeapon_TargetAnim {}
			, BlinkWeapon_SelfAnim {}
			, IonCannonType {}
			, IsBeamCannon { false }
			, BeamCannon_Length { 2560 }
			, BeamCannon_Length_Start { { 0, 0, 0 } }
			, BeamCannon_LengthIncrease { 20 }
			, BeamCannon_LengthIncreaseAcceleration { 0 }
			, BeamCannon_LengthIncreaseMax { 0 }
			, BeamCannon_LengthIncreaseMin { 0 }
			, BeamCannon_DrawEBolt { false }
			, BeamCannon_EleHeight { 4096 }
			, BeamCannon_InnerColor { { 255, 0, 0 } }
			, BeamCannon_OuterColor { { 255, 0, 0 } }
			, BeamCannon_OuterSpread { { 255, 0, 0 } }
			, BeamCannon_Duration { 3 }
			, BeamCannon_Thickness { 10 }
			, BeamCannonWeapon {}
			, BeamCannon_DrawLaser { false }
			, BeamCannon_LaserHeight { 20000 }
			, BeamCannon_DrawFromSelf { false }
			, BeamCannon_DrawFromSelf_FLH { { 0, 0, 0 } }
			, BeamCannon_ROF { 0 }
			, BeamCannon_Burst { 1 }
			, BeamCannon_FLH {}
			, BeamCannon_Start {}
			, PassengerDeletion { false }
			, PassengerTransport { false }
			, PassengerTransport_UsePassengerData { false }
			, PassengerTransport_Overlap { false }
			, PassengerTransport_MoveToTarget { false }
			, PassengerTransport_MoveToTargetAllowHouses { AffectedHouse::Team }
			, PassengerTransport_UseParachute { true }
			, PassengerTransport_MoveToTargetAllowHouses_IgnoreNeturalHouse { true }
			, SelfTransport { false }
			, SelfTransport_UseData { false }
			, SelfTransport_Overlap { false }
			, SelfTransport_Anim { nullptr }
			, SelfTransport_MoveToTarget { false }
			, SelfTransport_MoveToTargetAllowHouses { AffectedHouse::Team }
			, SelfTransport_MoveToTargetAllowHouses_IgnoreNeturalHouse { true }
			, SelfTransport_UseParachute { true }
			, FacingTarget { false }

			, KickOutPassenger {}
			, KickOutPassenger_Parachute {}

			, AttachWeapons {}
			, AttachWeapons_Burst_InvertL { true }
			, AttachWeapons_DetachedROF { false }
			, AttachWeapons_FLH {}
			, AttachWeapons_UseAmmo { false }
			, AttachWeapons_DetachedFire {}
			, OnlyAllowOneFirer { false }
			, OnlyAllowOneFirer_Count { 0 }
			, OnlyAllowOneFirer_OtherWeapons {}
			, OnlyAllowOneFirer_IgnoreWeapons {}
			, OnlyAllowOneFirer_ResetImmediately { true }

			, ElectricLaser { false }
			, ElectricLaser_Count { 3 }
			, ElectricLaser_Length { 5 }
			, ElectricLaser_Timer { 15 }
			, ElectricLaser_Color {}
			, ElectricLaser_Amplitude {}
			, ElectricLaser_Duration {}
			, ElectricLaser_Thickness {}
			, ElectricLaser_IsSupported {}

			, DelayedFire_Anim { }
			, DelayedFire_Anim_LoopCount { 1 }
			, DelayedFire_Anim_UseFLH { true }
			, DelayedFire_DurationTimer { 0 }

			, AttachAttachment_SelfToTarget{ false }
			, AttachAttachment_TargetToSelf { false }
			, AttachAttachment_Type {}
			, AttachAttachment_FLH { { 0, 0, 0 } }
			, AttachAttachment_IsOnTurret { false }

			, Ammo { 1 }

			, ExtraBurst { 0 }
			, ExtraBurst_Weapon { }
			, ExtraBurst_Houses { AffectedHouse::Enemies }
			, ExtraBurst_AlwaysFire { true }
			, ExtraBurst_FLH { }
			, ExtraBurst_FacingRange { unsigned short(128) }
			, ExtraBurst_InvertL { true }
			, ExtraBurst_Spread { false }
			, ExtraBurst_UseAmmo { false }
			, ExtraBurst_SkipNeutralTarget { false }

			, ExtraWarheads {}
			, ExtraWarheads_DamageOverrides {}

			, Laser_Thickness { -1 }
			, IsTrackingLaser { false }

			, StrafingLasers {}
			, StrafingLasers_Random { false }

			, DecloakToFire {}

			, EBolt_Color1 {}
			, EBolt_Color2 {}
			, EBolt_Color3 {}

			, Beam_Color {}
			, Beam_IsHouseColor { false }
			, Beam_Duration {}
			, Beam_Amplitude { 40 }

			, PreFireAnim { nullptr }
			, PreFireReport { -1 }

			, ProjectileRange { Leptons(INT_MAX) }
		{ }

		bool HasRequiredAttachedEffects(TechnoClass* pTechno, TechnoClass* pFirer);

		void AddStrafingLaser(TechnoClass* pThis, AbstractClass* pTarget, int IdxWeapon);
		void AddStrafingLaser(TechnoClass* pThis, AbstractClass* pTarget, CoordStruct coord);

		virtual ~ExtData() = default;

		virtual void LoadFromINIFile(CCINIClass* pINI) override;
		virtual void Initialize() override;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;

		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<WeaponTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static int nOldCircumference;

	static AnimTypeClass* GetFireAnim(WeaponTypeClass* pThis, TechnoClass* pFirer);

	static void DetonateAt(const WeaponTypeClass* pThis, AbstractClass* pTarget, TechnoClass* pOwner, HouseClass* pFiringHouse = nullptr);
	static void DetonateAt(const WeaponTypeClass* pThis, AbstractClass* pTarget, TechnoClass* pOwner, int damage, HouseClass* pFiringHouse = nullptr);
	static void DetonateAt(const WeaponTypeClass* pThis, const CoordStruct& coords, TechnoClass* pOwner, HouseClass* pFiringHouse = nullptr);
	static void DetonateAt(const WeaponTypeClass* pThis, const CoordStruct& coords, TechnoClass* pOwner, int damage, HouseClass* pFiringHouse = nullptr);
	static void ProcessAttachWeapons(WeaponTypeClass* pThis, TechnoClass* pOwner, AbstractClass* pTarget);
	static void ProcessExtraBrust(WeaponTypeClass* pThis, TechnoClass* pOwner, AbstractClass* pTarget);
	static void ProcessExtraBrustSpread(WeaponTypeClass* pThis, TechnoClass* pOwner, AbstractClass* pTarget);
	static int GetWeaponRange(WeaponTypeClass* pWeapon, TechnoClass* pTechno);
	static int GetWeaponRange(int weaponrange, TechnoClass* pTechno);
};
