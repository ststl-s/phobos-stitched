#pragma once
#include <BulletClass.h>
#include <WeaponTypeClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

#include <New/Type/RadTypeClass.h>
#include <New/Type/IonCannonTypeClass.h>

class WeaponTypeExt
{
public:
	using base_type = WeaponTypeClass;

	class ExtData final : public Extension<WeaponTypeClass>
	{
	public:

		Valueable<double> DiskLaser_Radius;
		Valueable<int> DiskLaser_Circumference;
		Valueable<RadTypeClass*> RadType;
		Valueable<bool> Bolt_Disable1;
		Valueable<bool> Bolt_Disable2;
		Valueable<bool> Bolt_Disable3;
		Valueable<int> Strafing_Shots;
		Valueable<bool> Strafing_SimulateBurst;
		Valueable<AffectedTarget> CanTarget;
		Valueable<AffectedHouse> CanTargetHouses;
		ValueableVector<int> Burst_Delays;
		Valueable<AreaFireTarget> AreaFire_Target;
		Nullable<WeaponTypeClass*> FeedbackWeapon;
		Valueable<bool> Laser_IsSingleColor;
		Valueable<double> Trajectory_Speed;
		Valueable<bool> BlinkWeapon;
		Valueable<bool> InvBlinkWeapon;
		Valueable<bool> BlinkWeapon_Overlap;
		Valueable<bool> BlinkWeapon_KillTarget;
		ValueableVector<AnimTypeClass*> BlinkWeapon_TargetAnim;
		ValueableVector<AnimTypeClass*> BlinkWeapon_SelfAnim;
		Nullable<IonCannonTypeClass*> IonCannonType;
		Valueable<bool> IsBeamCannon;
		Valueable<int> BeamCannon_Length;
		Valueable<int> BeamCannon_Length_StartOffset;
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
		Valueable<int> BeamCannon_DrawFromSelf_HeightOffset;
		Valueable<int> BeamCannon_ROF;
		Valueable<bool> PassengerDeletion;
		Valueable<bool> PassengerTransport;
		Valueable<bool> PassengerTransport_MoveToTarget;
		Valueable<AffectedHouse> PassengerTransport_MoveToTargetAllowHouses;
		Valueable<bool> FacingTarget;
		Valueable<bool> KickOutPassenger;
		Valueable<bool> OnlyAllowOneFirer;

		ValueableVector<WeaponTypeClass*> AttachWeapons;

		ExtData(WeaponTypeClass* OwnerObject) : Extension<WeaponTypeClass>(OwnerObject)
			, DiskLaser_Radius { 38.2 }
			, DiskLaser_Circumference { 240 }
			, RadType {}
			, Bolt_Disable1 { false }
			, Bolt_Disable2 { false }
			, Bolt_Disable3 { false }
			, Strafing_Shots { 5 }
			, Strafing_SimulateBurst { false }
			, CanTarget { AffectedTarget::All }
			, CanTargetHouses { AffectedHouse::All }
			, Burst_Delays {}
			, AreaFire_Target { AreaFireTarget::Base }
			, FeedbackWeapon {}
			, Laser_IsSingleColor { false }
			, Trajectory_Speed { 100.0 }
			, BlinkWeapon { false }
			, InvBlinkWeapon { false }
			, BlinkWeapon_Overlap { false }
			, BlinkWeapon_KillTarget { false }
			, BlinkWeapon_TargetAnim {}
			, BlinkWeapon_SelfAnim {}
			, IonCannonType {}
			, IsBeamCannon { false }
			, BeamCannon_Length { 2560 }
			, BeamCannon_Length_StartOffset { 0 }
			, BeamCannon_LengthIncrease { 20 }
			, BeamCannon_LengthIncreaseAcceleration { 0 }
			, BeamCannon_LengthIncreaseMax { 0 }
			, BeamCannon_LengthIncreaseMin { 0 }
			, BeamCannon_DrawEBolt { false }
			, BeamCannon_EleHeight { 4096 }
			, BeamCannon_InnerColor { {255,0,0} }
			, BeamCannon_OuterColor { {255,0,0} }
			, BeamCannon_OuterSpread { {255,0,0} }
			, BeamCannon_Duration { 3 }
			, BeamCannon_Thickness { 10 }
			, BeamCannonWeapon {}
			, BeamCannon_DrawLaser { false }
			, BeamCannon_LaserHeight { 20000 }
			, BeamCannon_DrawFromSelf { false }
			, BeamCannon_DrawFromSelf_HeightOffset { 0 }
			, BeamCannon_ROF { 0 }
			, PassengerDeletion { false }
			, PassengerTransport { false }
			, PassengerTransport_MoveToTarget { false }
			, PassengerTransport_MoveToTargetAllowHouses { AffectedHouse::Team }
			, FacingTarget { false }
			, KickOutPassenger { false }
			, AttachWeapons {}
			, OnlyAllowOneFirer { false }
		{ }

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

	// no projectile or no warhead while call Debug::FatalErrorAndExist
	// pThis == nullptr is allowed
	static void AssertValid(WeaponTypeClass* pThis);

	static void DetonateAt(WeaponTypeClass* pThis, ObjectClass* pTarget, TechnoClass* pOwner);
	static void DetonateAt(WeaponTypeClass* pThis, ObjectClass* pTarget, TechnoClass* pOwner, int damage);
	static void DetonateAt(WeaponTypeClass* pThis, const CoordStruct& coords, TechnoClass* pOwner);
	static void DetonateAt(WeaponTypeClass* pThis, const CoordStruct& coords, TechnoClass* pOwner, int damage);
	static void ProcessAttachWeapons(WeaponTypeClass* pThis, TechnoClass* pOwner, AbstractClass* pTarget);
};
