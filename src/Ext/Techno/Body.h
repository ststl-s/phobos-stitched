#pragma once
#include <TechnoClass.h>
#include <AnimClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

#include <New/Entity/ShieldClass.h>
#include <New/Entity/LaserTrailClass.h>
#include <New/Entity/GiftBoxClass.h>
#include <New/Entity/AttachmentClass.h>
#include <New/Entity/FireScriptClass.h>

class BulletClass;

class TechnoExt
{
public:
	using base_type = TechnoClass;

	class ExtData final : public Extension<TechnoClass>
	{
	public:
		std::unique_ptr<ShieldClass> Shield;
		std::vector<std::unique_ptr<LaserTrailClass>> LaserTrails;
		bool ReceiveDamage;
		bool LastKillWasTeamTarget;
		TimerStruct	PassengerDeletionTimer;
		int PassengerDeletionCountDown;
		ShieldTypeClass* CurrentShieldType;
		int LastWarpDistance;
		int Death_Countdown;
		AnimTypeClass* MindControlRingAnimType;
		int DamageNumberOffset;
		Valueable<bool> IsLeggedCyborg;

		// Used for Passengers.SyncOwner.RevertOnExit instead of TechnoClass::InitialOwner / OriginallyOwnedByHouse,
		// as neither is guaranteed to point to the house the TechnoClass had prior to entering transport and cannot be safely overridden.
		HouseClass* OriginalPassengerOwner;

		ValueableVector<int> FireSelf_Count;
		ValueableVector<WeaponTypeClass*> FireSelf_Weapon;
		ValueableVector<int> FireSelf_ROF;
		ValueableVector<FireScriptClass*>Processing_Scripts;

		bool IonCannon_setRadius;
		int IonCannon_Radius;
		int IonCannon_StartAngle;
		bool IonCannon_Stop;
		int IonCannon_Rate;
		int IonCannon_ROF;
		int IonCannon_RadiusReduce;
		int IonCannon_Angle;
		int IonCannon_Scatter_Max;
		int IonCannon_Scatter_Min;
		int IonCannon_Duration;

		Valueable<WeaponTypeClass*> setIonCannonWeapon;
		Nullable<IonCannonTypeClass*> setIonCannonType;
		bool IonCannonWeapon_setRadius;
		int IonCannonWeapon_Radius;
		int IonCannonWeapon_StartAngle;
		bool IonCannonWeapon_Stop;
		CoordStruct IonCannonWeapon_Target;
		int IonCannonWeapon_ROF;
		int IonCannonWeapon_RadiusReduce;
		int IonCannonWeapon_Angle;
		int IonCannonWeapon_Scatter_Max;
		int IonCannonWeapon_Scatter_Min;
		int IonCannonWeapon_Duration;

		Valueable<WeaponTypeClass*> setBeamCannon;
		bool BeamCannon_setLength;
		int BeamCannon_Length;
		bool BeamCannon_Stop;
		CoordStruct BeamCannon_Target;
		CoordStruct BeamCannon_Self;
		int BeamCannon_ROF;
		int BeamCannon_LengthIncrease;

		FootClass* PassengerList[32];
		CoordStruct PassengerlocationList[32];
		bool AllowCreatPassenger;
		bool AllowChangePassenger;

		bool AllowPassengerToFire;
		int AllowFireCount;

		bool SpawneLoseTarget;

        int ShowAnim_LastActivatedFrame;

		std::unique_ptr<GiftBoxClass> AttachedGiftBox;

		AttachmentClass* ParentAttachment;
		ValueableVector<std::unique_ptr<AttachmentClass>> ChildAttachments;

		ExtData(TechnoClass* OwnerObject) : Extension<TechnoClass>(OwnerObject)
			, Shield {}
			, LaserTrails {}
			, ReceiveDamage { false }
			, AttachedGiftBox {}
			, LastKillWasTeamTarget { false }
			, PassengerDeletionTimer {}
			, PassengerDeletionCountDown { -1 }
			, CurrentShieldType { nullptr }
			, LastWarpDistance {}
			, Death_Countdown { -1 }
			, ParentAttachment { nullptr }
			, ChildAttachments {}
			, MindControlRingAnimType { nullptr }
			, IsLeggedCyborg { false }
			, DamageNumberOffset { INT32_MIN }
			, OriginalPassengerOwner {}

			, IonCannon_setRadius { true }
			, IonCannon_Radius { -1 }
			, IonCannon_StartAngle { -180 }
			, IonCannon_Stop { false }
			, IonCannon_Rate { -1 }
			, IonCannon_ROF { 0 }
			, IonCannon_RadiusReduce{ 0 }
			, IonCannon_Angle { 0 }
			, IonCannon_Scatter_Max { 0 }
			, IonCannon_Scatter_Min { 0 }
			, IonCannon_Duration { 0 }

			, setIonCannonWeapon {}
			, setIonCannonType {}
			, IonCannonWeapon_setRadius { true }
			, IonCannonWeapon_Radius { -1 }
			, IonCannonWeapon_StartAngle { -180 }
			, IonCannonWeapon_Stop { true }
			, IonCannonWeapon_Target {}
			, IonCannonWeapon_ROF { 0 }
			, IonCannonWeapon_RadiusReduce { 0 }
			, IonCannonWeapon_Angle { 0 }
			, IonCannonWeapon_Scatter_Max { 0 }
			, IonCannonWeapon_Scatter_Min { 0 }
			, IonCannonWeapon_Duration { 0 }

			, setBeamCannon {}
			, BeamCannon_setLength { true }
			, BeamCannon_Length { -1 }
			, BeamCannon_Stop { true }
			, BeamCannon_Target {}
			, BeamCannon_Self {}
			, BeamCannon_ROF { 0 }
			, BeamCannon_LengthIncrease { 0 }

			, PassengerList { nullptr }
			, PassengerlocationList {}
			, AllowCreatPassenger { true }
			, AllowChangePassenger { false }

			, AllowPassengerToFire { false }
			, AllowFireCount { 0 }

			, SpawneLoseTarget { true }

			, FireSelf_Count {}
			, FireSelf_Weapon {}
			, FireSelf_ROF {}
			, Processing_Scripts {}

			, ShowAnim_LastActivatedFrame { -1 }
		{ }

		virtual ~ExtData() = default;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override
		{
			if (auto const pShield = this->Shield.get())
				pShield->InvalidatePointer(ptr);
		}

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	enum HealthState
	{
		Green = 1,
		Yellow = 2,
		Red = 3
	};

	class ExtContainer final : public Container<TechnoExt>
	{
	public:
		ExtContainer();
		~ExtContainer();

		virtual bool InvalidateExtDataIgnorable(void* const ptr) const override
		{
			auto const abs = static_cast<AbstractClass*>(ptr)->WhatAmI();
			switch (abs)
			{
			case AbstractType::Anim:
			case AbstractType::Bullet:
				return false;
			default:
				return true;
			}
		}

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override;
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	//In TechnoClass_AI-------------------------------------------
	//Phobos and PR
	static void ApplyMindControlRangeLimit(TechnoClass* pThis);
	static void ApplyInterceptor(TechnoClass* pThis);
	static void ApplyPowered_KillSpawns(TechnoClass* pThis);
	static void ApplySpawn_LimitRange(TechnoClass* pThis);
	static void CheckDeathConditions(TechnoClass* pThis);
	static void EatPassengers(TechnoClass* pThis);
	static void UpdateMindControlAnim(TechnoClass* pThis);

	//stitched
	static void CheckIonCannonConditions(TechnoClass* pThis);
	static void RunIonCannonWeapon(TechnoClass* pThis, TechnoExt::ExtData* pExt);
	static void RunBeamCannon(TechnoClass* pThis, TechnoExt::ExtData* pExt);
	static void ChangePassengersList(TechnoClass* pThis, TechnoExt::ExtData* pExt);
	static void MovePassengerToSpawn(TechnoClass* pThis, TechnoTypeExt::ExtData* pTypeExt);
	static void SilentPassenger(TechnoClass* pThis, TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt);
	static void Spawner_SameLoseTarget(TechnoClass* pThis, TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt);
	static void RunFireSelf(TechnoClass* pThis, TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt);
	static void UpdateFireScript(TechnoClass* pThis, TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt);
	//------------------------------------------------------------

	//static bool IsActive(TechnoClass* pThis);
	static bool IsReallyAlive(TechnoClass* const pThis);
	static bool IsActive(TechnoClass* const pThis);

	static int GetSizeLeft(FootClass* const pThis);

	static void Stop(TechnoClass* pThis, Mission eMission = Mission::Guard);

	static bool IsHarvesting(TechnoClass* pThis);
	static bool HasAvailableDock(TechnoClass* pThis);

	static void InitializeLaserTrails(TechnoClass* pThis);
	static void InitializeShield(TechnoClass* pThis);
	static CoordStruct GetFLHAbsoluteCoords(TechnoClass* pThis, CoordStruct flh, bool turretFLH = false);

	static CoordStruct GetBurstFLH(TechnoClass* pThis, int weaponIndex, bool& FLHFound);
	static CoordStruct GetSimpleFLH(InfantryClass* pThis, int weaponIndex, bool& FLHFound);

	static bool AttachmentAI(TechnoClass* pThis);
	static bool AttachTo(TechnoClass* pThis, TechnoClass* pParent);
	static bool DetachFromParent(TechnoClass* pThis, bool force = false);

	static void InitializeAttachments(TechnoClass* pThis);
	static void HandleHostDestruction(TechnoClass* pThis);
	static void UnlimboAttachments(TechnoClass* pThis);
	static void LimboAttachments(TechnoClass* pThis);

	static bool IsParentOf(TechnoClass* pThis, TechnoClass* pOtherTechno);

	static void FireWeaponAtSelf(TechnoClass* pThis, WeaponTypeClass* pWeaponType);
	static void inline KillSelf(TechnoClass* pThis, bool isPeaceful);
	static void UpdateSharedAmmo(TechnoClass* pThis);
	static void TransferMindControlOnDeploy(TechnoClass* pTechnoFrom, TechnoClass* pTechnoTo);
	static void ObjectKilledBy(TechnoClass* pThis, TechnoClass* pKiller);
	static double GetCurrentSpeedMultiplier(FootClass* pThis);
	static bool CanFireNoAmmoWeapon(TechnoClass* pThis, int weaponIndex);
	static void DrawSelfHealPips(TechnoClass* pThis, Point2D* pLocation, RectangleStruct* pBounds);
	static void DrawInsignia(TechnoClass* pThis, Point2D* pLocation, RectangleStruct* pBounds);
	static void ApplyGainedSelfHeal(TechnoClass* pThis);

	static void DrawGroupID_Building(TechnoClass* pThis, TechnoTypeExt::ExtData* pTypeExt, Point2D* pLocation);
	static void DrawGroupID_Other(TechnoClass* pThis, TechnoTypeExt::ExtData* pTypeExt, Point2D* pLocation);

	static void DrawHealthBar_Building(TechnoClass* pThis, TechnoTypeExt::ExtData* pTypeExt, int iLength, Point2D* pLocation, RectangleStruct* pBound);
	static void DrawHealthBar_Other(TechnoClass* pThis, TechnoTypeExt::ExtData* pTypeExt, int iLength, Point2D* pLocation, RectangleStruct* pBound);
	static void DrawHealthBar_Picture(TechnoClass* pThis, TechnoTypeExt::ExtData* pTypeExt, int iLength, Point2D* pLocation, RectangleStruct* pBound);

	static int DrawHealthBar_Pip(TechnoClass* pThis, TechnoTypeExt::ExtData* pTypeExt, const bool isBuilding);
	static int DrawHealthBar_PipAmount(TechnoClass* pThis, TechnoTypeExt::ExtData* pTypeExt, int iLength);
	static double GetHealthRatio(TechnoClass* pThis);

	static void InitialShowHugeHP(TechnoClass* pThis);
	static void RunHugeHP();
	static void EraseHugeHP(TechnoClass* pThis, TechnoTypeExt::ExtData* pTypeExt);

	static void UpdateHugeHP(TechnoClass* pThis);
	static void DrawHugeHP(TechnoClass* pThis);
	static void DrawHugeSP(TechnoClass* pThis);
	static void DrawHugeHPValue_Text(int CurrentValue, int MaxValue, HealthState State);
	static void DrawHugeHPValue_SHP(int CurrentValue, int MaxValue, HealthState State);
	static void DrawHugeSPValue_Text(int CurrentValue, int MaxValue, HealthState State);
	static void DrawHugeSPValue_SHP(int CurrentValue, int MaxValue, HealthState State);

	static void AddFireScript(TechnoClass* pThis);
	static void RunBlinkWeapon(TechnoClass* pThis, AbstractClass* pTarget, WeaponTypeClass* pWeapon);
	static void IonCannonWeapon(TechnoClass* pThis, AbstractClass* pTarget, WeaponTypeClass* pWeapon);
	static void BeamCannon(TechnoClass* pThis, AbstractClass* pTarget, WeaponTypeClass* pWeapon);
	static void Destoryed_EraseAttachment(TechnoClass* pThis);
	static void DrawSelectBrd(TechnoClass* pThis, TechnoTypeExt::ExtData* pTypeExt, int iLength, Point2D* pLocation, RectangleStruct* pBound, bool isInfantry);
	static void DisplayDamageNumberString(TechnoClass* pThis, int damage, bool isShieldDamage);
	static void FirePassenger(TechnoClass* pThis, AbstractClass* pTarget, WeaponTypeClass* pWeapon);
	static void AllowPassengerToFire(TechnoClass* pThis, AbstractClass* pTarget, WeaponTypeClass* pWeapon);
	static void SpawneLoseTarget(TechnoClass* pThis);

	static void ReceiveDamageAnim(TechnoClass* pThis, int damage);

	static Point2D GetScreenLocation(TechnoClass* pThis);
	static Point2D GetHealthBarPosition(TechnoClass* pThis, bool Shield = false, HealthBarAnchors Anchor = HealthBarAnchors::TopLeft);
};
