#pragma once
#include <TechnoClass.h>
#include <AnimClass.h>

#include <Helpers/Macro.h>

#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

#include <Ext/WarheadType/Body.h>

#include <New/Entity/ShieldClass.h>
#include <New/Entity/LaserTrailClass.h>
#include <New/Entity/GiftBoxClass.h>
#include <New/Entity/AttachmentClass.h>
#include <New/Entity/AttachEffectClass.h>

class BulletClass;

class TechnoExt
{
public:
	using base_type = TechnoClass;

	class ExtData final : public Extension<TechnoClass>
	{
	public:
		TechnoTypeExt::ExtData* TypeExtData;
		std::unique_ptr<ShieldClass> Shield;
		std::vector<std::unique_ptr<LaserTrailClass>> LaserTrails;
		bool ReceiveDamage;
		bool LastKillWasTeamTarget;
		CDTimerClass PassengerDeletionTimer;
		int PassengerDeletionCountDown;
		ShieldTypeClass* CurrentShieldType;
		int LastWarpDistance;
		CDTimerClass AutoDeathTimer;
		AnimTypeClass* MindControlRingAnimType;
		bool IsLeggedCyborg;
		OptionalStruct<int, false> DamageNumberOffset;
		OptionalStruct<int, true> CurrentLaserWeaponIndex;

		// Used for Passengers.SyncOwner.RevertOnExit instead of TechnoClass::InitialOwner / OriginallyOwnedByHouse,
		// as neither is guaranteed to point to the house the TechnoClass had prior to entering transport and cannot be safely overridden.
		HouseClass* OriginalPassengerOwner;

		std::vector<CDTimerClass> FireSelf_Timers;

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

		WeaponTypeClass* setIonCannonWeapon;
		IonCannonTypeClass* setIonCannonType;
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

		WeaponTypeClass* setBeamCannon;
		bool BeamCannon_setLength;
		int BeamCannon_Length;
		bool BeamCannon_Stop;
		CoordStruct BeamCannon_Target;
		CoordStruct BeamCannon_Self;
		int BeamCannon_ROF;
		int BeamCannon_LengthIncrease;

		int PassengerNumber;
		FootClass* PassengerList[sizeof(PassengerNumber)];
		CoordStruct PassengerlocationList[sizeof(PassengerNumber)];
		bool AllowCreatPassenger;
		bool AllowChangePassenger;

		bool AllowPassengerToFire;
		int AllowFireCount;

		bool SpawneLoseTarget;

		int ShowAnim_LastActivatedFrame;

		int ConvertsCounts;
		TechnoTypeClass* ConvertsOriginalType;
		AnimTypeClass* ConvertsAnim;
		std::vector<TechnoTypeClass*> Convert_FromTypes;
		bool Convert_DetachedBuildLimit;

		int DisableTurnCount;
		bool FacingInitialized;
		DirStruct LastSelfFacing;
		DirStruct LastTurretFacing;

		std::unique_ptr<GiftBoxClass> AttachedGiftBox;

		AttachmentClass* ParentAttachment;
		std::vector<std::unique_ptr<AttachmentClass>> ChildAttachments;

		bool AllowToPaint;
		ColorStruct ColorToPaint;
		int Paint_Count;
		bool Paint_IsDiscoColor;
		std::vector<ColorStruct> Paint_Colors;
		int Paint_TransitionDuration;
		int Paint_FramesPassed;
		bool Paint_IgnoreTintStatus;

		bool InROF;
		int ROFCount;
		bool IsChargeROF;
		int GattlingCount;
		int GattlingStage;
		int GattlingWeaponIndex;
		int MaxGattlingCount;
		bool IsCharging;
		bool HasCharged;
		AbstractClass* AttackTarget;
		std::vector<DynamicVectorClass<WeaponTypeClass*>> GattlingWeapons;
		std::vector<DynamicVectorClass<int>> GattlingStages;

		WeaponTypeClass* PrimaryWeapon;
		WeaponTypeClass* SecondaryWeapon;
		std::vector<DynamicVectorClass<CoordStruct>> WeaponFLHs;

		bool NeedConvertWhenLanding;
		bool JJ_Landed;
		UnitTypeClass* FloatingType;
		UnitTypeClass* LandingType;

		std::vector<TechnoTypeClass*> Build_As;
		bool Build_As_OnlyOne;

		std::vector<int> AttackedWeapon_Timer;

		bool CanDodge;
		int DodgeDuration;
		AffectedHouse Dodge_Houses;
		double Dodge_MaxHealthPercent;
		double Dodge_MinHealthPercent;
		double Dodge_Chance;
		AnimTypeClass* Dodge_Anim;
		bool Dodge_OnlyDodgePositiveDamage;

		CoordStruct LastLocation;
		int MoveDamage_Duration;
		int MoveDamage_Count;
		int MoveDamage_Delay;
		int MoveDamage;
		AnimTypeClass* MoveDamage_Anim;
		WarheadTypeClass* MoveDamage_Warhead;
		int StopDamage_Duration;
		int StopDamage_Count;
		int StopDamage_Delay;
		int StopDamage;
		WarheadTypeClass* StopDamage_Warhead;
		AnimTypeClass* StopDamage_Anim;

		bool IsSharingWeaponRange;
		bool BeSharedWeaponRange;
		bool ShareWeaponFire;

		bool InitialPayload;

		std::vector<DynamicVectorClass<WeaponTypeClass*>> IFVWeapons;
		std::vector<DynamicVectorClass<int>> IFVTurrets;

		int BuildingROFFix;

		TechnoClass* Attacker;
		int Attacker_Count;

		bool LimitDamage;
		int LimitDamageDuration;
		Vector2D<int> AllowMaxDamage;
		Vector2D<int> AllowMinDamage;

		int TeamAffectCount;
		bool TeamAffectActive;
		int TeamAffectLoseEfficacyCount;
		AnimClass* TeamAffectAnim;
		ValueableVector<TechnoClass*> TeamAffectUnits;

		bool LosePower;
		AnimClass* LosePowerAnim;
		int LosePowerParticleCount;

		CDTimerClass PassengerHealTimer;
		int PassengerHealCountDown;

		std::vector<std::unique_ptr<AttachEffectClass>> AttachEffects;
		std::map<int, std::vector<CDTimerClass>> AttachWeapon_Timers;
		bool AttachEffects_Initialized;

		std::unordered_map<size_t, int> Temperature;
		std::unordered_map<size_t, CDTimerClass> Temperature_HeatUpTimer;
		std::unordered_map<size_t, CDTimerClass> Temperature_HeatUpDelayTimer;
		std::unordered_map<size_t, CDTimerClass> Temperature_WeaponTimer;

		TechnoTypeClass* OrignType;
		FootClass* ConvertPassenger;
		bool IsConverted;
		std::vector<TechnoTypeClass*> Convert_Passengers;
		std::vector<TechnoTypeClass*> Convert_Types;

		Rank CurrentRank;

		int ReplacedArmorIdx;
		bool ArmorReplaced;

		int PassengerProduct_Timer;

		double ReceiveDamageMultiplier;
		int NeedParachute_Height;

		CDTimerClass DeployFireTimer;

		AnimClass* DelayedFire_Anim;
		int DelayedFire_Anim_LoopCount;
		int DelayedFire_DurationTimer;

		bool Crate_Cloakable;

		bool IsInTunnel;

		int TargetType;

		ExtData(TechnoClass* OwnerObject) : Extension<TechnoClass>(OwnerObject)
			, TypeExtData { nullptr }
			, Shield {}
			, LaserTrails {}
			, ReceiveDamage { false }
			, AttachedGiftBox {}
			, LastKillWasTeamTarget { false }
			, PassengerDeletionTimer {}
			, PassengerDeletionCountDown { -1 }
			, CurrentShieldType { nullptr }
			, LastWarpDistance {}
			, AutoDeathTimer {}
			, ParentAttachment { nullptr }
			, ChildAttachments {}
			, MindControlRingAnimType { nullptr }
			, DamageNumberOffset {}
			, IsLeggedCyborg { false }
			, OriginalPassengerOwner {}
			, DeployFireTimer {}

			, IonCannon_setRadius { true }
			, IonCannon_Radius { -1 }
			, IonCannon_StartAngle { -180 }
			, IonCannon_Stop { false }
			, IonCannon_Rate { -1 }
			, IonCannon_ROF { 0 }
			, IonCannon_RadiusReduce { 0 }
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

			, PassengerNumber { 0 }
			, PassengerList { nullptr }
			, PassengerlocationList {}
			, AllowCreatPassenger { true }
			, AllowChangePassenger { false }

			, AllowPassengerToFire { false }
			, AllowFireCount { 0 }

			, SpawneLoseTarget { true }

			, FireSelf_Timers {}

			, ShowAnim_LastActivatedFrame { -1 }

			, ConvertsCounts { -1 }
			, ConvertsOriginalType {}
			, ConvertsAnim { nullptr }
			, Convert_FromTypes {}
			, Convert_DetachedBuildLimit { false }

			, DisableTurnCount { -1 }
			, LastSelfFacing { -1 }
			, LastTurretFacing { -1 }

			, AllowToPaint { false }
			, ColorToPaint { 255, 0, 0 }
			, Paint_Count { 0 }
			, Paint_IsDiscoColor { false }
			, Paint_Colors {}
			, Paint_TransitionDuration { 60 }
			, Paint_FramesPassed { 0 }
			, Paint_IgnoreTintStatus { false }

			, InROF { false }
			, ROFCount { -1 }
			, IsChargeROF { false }
			, GattlingCount { 0 }
			, GattlingStage { 0 }
			, GattlingWeaponIndex { 0 }
			, MaxGattlingCount { 0 }
			, IsCharging { false }
			, HasCharged { false }
			, AttackTarget { nullptr }
			, GattlingWeapons {}
			, GattlingStages {}

			, PrimaryWeapon {}
			, SecondaryWeapon {}
			, WeaponFLHs {}

			, NeedConvertWhenLanding { false }
			, JJ_Landed { false }
			, FloatingType {}
			, LandingType {}

			, Build_As {}
			, Build_As_OnlyOne { false }
			, AttackedWeapon_Timer {}

			, CanDodge { false }
			, DodgeDuration { 0 }
			, Dodge_Houses { AffectedHouse::All }
			, Dodge_MaxHealthPercent { 1.0 }
			, Dodge_MinHealthPercent { 0.0 }
			, Dodge_Chance { 0.0 }
			, Dodge_Anim {}
			, Dodge_OnlyDodgePositiveDamage { true }

			, LastLocation {}
			, MoveDamage_Duration { 0 }
			, MoveDamage_Count { 0 }
			, MoveDamage_Delay { 0 }
			, MoveDamage { 0 }
			, MoveDamage_Warhead {}
			, MoveDamage_Anim {}
			, StopDamage_Duration { 0 }
			, StopDamage_Count { 0 }
			, StopDamage_Delay { 0 }
			, StopDamage { 0 }
			, StopDamage_Warhead {}
			, StopDamage_Anim {}

			, IsSharingWeaponRange { false }
			, BeSharedWeaponRange { false }
			, ShareWeaponFire { false }

			, InitialPayload { false }

			, IFVWeapons {}
			, IFVTurrets {}

			, BuildingROFFix { -1 }

			, Attacker { nullptr }
			, Attacker_Count { 0 }

			, LimitDamage { false }
			, LimitDamageDuration { 0 }
			, AllowMaxDamage { MAX(int), MIN(int) }
			, AllowMinDamage { MIN(int), MAX(int) }

			, TeamAffectCount { -1 }
			, TeamAffectActive { false }
			, TeamAffectLoseEfficacyCount { -1 }
			, TeamAffectAnim { nullptr }
			, TeamAffectUnits {}

			, AttachEffects{}
			, AttachEffects_Initialized { false }
			, AttachWeapon_Timers {}

			, LosePower { false }
			, LosePowerAnim { nullptr }
			, LosePowerParticleCount { 0 }

			, PassengerHealCountDown {}
			, PassengerHealTimer { -1 }

			, Temperature{}
			, Temperature_HeatUpTimer {}
			, Temperature_WeaponTimer {}

			, OrignType { nullptr }
			, ConvertPassenger { nullptr }
			, IsConverted { false }
			, Convert_Passengers {}
			, Convert_Types {}

			, CurrentRank { Rank::Invalid }
			, ReplacedArmorIdx { 0 }
			, ArmorReplaced { false }
			, PassengerProduct_Timer {}

			, ReceiveDamageMultiplier { 1.0 }
			, NeedParachute_Height { 0 }

			, Crate_Cloakable { false }

			, IsInTunnel { false }

			, DelayedFire_Anim { nullptr }
			, DelayedFire_Anim_LoopCount { 0 }
			, DelayedFire_DurationTimer { -1 }

			, TargetType { 0 }
		{ }

		void ApplyInterceptor();
		bool CheckDeathConditions();
		void EatPassengers();
		void UpdateShield();
		void ApplySpawnLimitRange();
		void ApplySpawnSameLoseTarget();
		void CheckAttachEffects();
		void UpdateAttackedWeaponTimer();
		void IsInROF();
		void CheckPaintConditions();
		void InfantryConverts();
		void CheckIonCannonConditions();
		void RunIonCannonWeapon();
		void RunBeamCannon();
		void SilentPassenger();
		void ConvertsRecover();
		void RecalculateROT();
		void ChangePassengersList();
		void CheckJJConvertConditions();
		void TechnoUpgradeAnim();
		void OccupantsWeaponChange();
		void OccupantsVeteranWeapon();
		void ProcessFireSelf();
		void ShieldPowered();
		void UpdateDodge();
		void ProcessMoveDamage();
		void ProcessStopDamage();
		void ForgetFirer();
		void TeamAffect();
		void PoweredUnit();
		void PoweredUnitDown();
		void UpdateDamageLimit();
		void ApplyMobileRefinery();
		void ShareWeaponRangeRecover();
		void ShareWeaponRangeFire();
		void PassengerProduct();
		int __fastcall GetArmorIdx(const WeaponTypeClass* pWeapon) const;
		int __fastcall GetArmorIdx(const WarheadTypeClass* pWH) const;
		int GetArmorIdxWithoutShield() const;
		void CheckParachuted();
		void UpdateOnTunnelEnter();
		void __fastcall UpdateTypeData(const TechnoTypeClass* currentType);

		virtual ~ExtData() = default;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override
		{
			for (auto& pAE : AttachEffects)
			{
				pAE->InvalidatePointer(ptr, bRemoved);
			}

			if (bRemoved)
			{
				for (auto const& pAttachment : ChildAttachments)
					pAttachment->InvalidatePointer(ptr);
			}
		}

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<TechnoExt>
	{
	public:
		ExtContainer();
		~ExtContainer();

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override;
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	//In TechnoClass_AI-------------------------------------------

	static void ApplyMindControlRangeLimit(TechnoClass* pThis, TechnoTypeExt::ExtData* pTypeExt);
	static void MovePassengerToSpawn(TechnoClass* pThis, TechnoTypeExt::ExtData* pTypeExt);
	static void WeaponFacingTarget(TechnoClass* pThis);
	static void SelectGattlingWeapon(TechnoClass* pThis, TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt);
	static void TechnoGattlingCount(TechnoClass* pThis, TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt);
	static void ResetGattlingCount(TechnoClass* pThis, TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt);
	static void SetWeaponIndex(TechnoClass* pThis, TechnoExt::ExtData* pExt);
	static void SelectIFVWeapon(TechnoClass* pThis, TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt);
	static void BuildingPassengerFix(TechnoClass* pThis);
	static void BuildingSpawnFix(TechnoClass* pThis);
	static void CheckTemperature(TechnoClass* pThis);
	static void ReceiveShareDamage(TechnoClass* pThis, args_ReceiveDamage* args, std::vector<TechnoClass*>& teamTechnos);
	static void CurePassengers(TechnoClass* pThis, TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt);
	static void CheckPassenger(TechnoClass* pThis, TechnoTypeClass* pType, TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt);
	//------------------------------------------------------------

	static bool __fastcall IsReallyAlive(ObjectClass* const pThis);
	static bool __fastcall IsActive(TechnoClass* const pThis);
	// building needs IsPowerOnline() == true
	static bool __fastcall IsActivePower(TechnoClass* const pThis);
	static bool __fastcall CanICloakByDefault(TechnoClass* pThis);

	static int __fastcall GetSizeLeft(FootClass* const pThis);

	static void __fastcall Stop(TechnoClass* pThis, Mission eMission = Mission::Guard);

	static bool IsHarvesting(TechnoClass* pThis);
	static bool HasAvailableDock(TechnoClass* pThis);

	static void InitializeLaserTrails(TechnoClass* pThis);
	static void InitializeShield(TechnoClass* pThis);

	static Matrix3D GetTransform(TechnoClass* pThis, VoxelIndexKey* pKey = nullptr, bool isShadow = false);
	static Matrix3D GetFLHMatrix(TechnoClass* pThis, CoordStruct flh, bool isOnTurret, double factor = 1.0, bool isShadow = false);
	static Matrix3D TransformFLHForTurret(TechnoClass* pThis, Matrix3D mtx, bool isOnTurret, double factor = 1.0);
	static CoordStruct GetFLHAbsoluteCoords(TechnoClass* pThis, CoordStruct flh, bool isOnTurret = false);

	static CoordStruct GetBurstFLH(TechnoClass* pThis, int weaponIndex, bool& FLHFound);
	static CoordStruct GetSimpleFLH(InfantryClass* pThis, int weaponIndex, bool& FLHFound);

	static bool AttachmentAI(TechnoClass* pThis);
	static bool AttachTo(TechnoClass* pThis, TechnoClass* pParent);
	static bool DetachFromParent(TechnoClass* pThis, bool force = false);

	static void InitializeAttachments(TechnoClass* pThis);
	static void DestroyAttachments(TechnoClass* pThis, TechnoClass* pSource);
	static void HandleDestructionAsChild(TechnoClass* pThis);
	static void UnlimboAttachments(TechnoClass* pThis);
	static void LimboAttachments(TechnoClass* pThis);

	static bool IsAttached(TechnoClass* pThis);
	static bool IsParentOf(TechnoClass* pThis, TechnoClass* pOtherTechno);
	static bool IsChildOf(TechnoClass* pThis, TechnoClass* pParent, bool deep = true);
	static TechnoClass* GetTopLevelParent(TechnoClass* pThis);
	static Matrix3D GetAttachmentTransform(TechnoClass* pThis, VoxelIndexKey* pKey = nullptr, bool isShadow = false);

	static void FireWeaponAtSelf(TechnoClass* pThis, WeaponTypeClass* pWeaponType);
	static void KillSelf(TechnoClass* pThis, AutoDeathBehavior deathOption);

	static void UpdateSharedAmmo(TechnoClass* pThis);
	static void TransferMindControlOnDeploy(TechnoClass* pTechnoFrom, TechnoClass* pTechnoTo);
	static void ObjectKilledBy(TechnoClass* pThis, TechnoClass* pKiller);
	static double GetCurrentSpeedMultiplier(FootClass* pThis);
	static bool CanFireNoAmmoWeapon(TechnoClass* pThis, int weaponIndex);
	static void DrawSelfHealPips(TechnoClass* pThis, Point2D* pLocation, RectangleStruct* pBounds);
	static void DrawInsignia(TechnoClass* pThis, Point2D* pLocation, RectangleStruct* pBounds);
	static void ApplyGainedSelfHeal(TechnoClass* pThis);
	static void SyncIronCurtainStatus(TechnoClass* pFrom, TechnoClass* pTo);
	static void UpdateMindControlAnim(TechnoClass* pThis);

	static void DrawGroupID_Building(TechnoClass* pThis, TechnoTypeExt::ExtData* pTypeExt, Point2D* pLocation);
	static void DrawGroupID_Other(TechnoClass* pThis, TechnoTypeExt::ExtData* pTypeExt, Point2D* pLocation);

	static void DrawHealthBar_Building(TechnoClass* pThis, TechnoTypeExt::ExtData* pTypeExt, int iLength, Point2D* pLocation, RectangleStruct* pBound);
	static void DrawHealthBar_Other(TechnoClass* pThis, TechnoTypeExt::ExtData* pTypeExt, int iLength, Point2D* pLocation, RectangleStruct* pBound);
	static void DrawHealthBar_Picture(TechnoClass* pThis, TechnoTypeExt::ExtData* pTypeExt, int iLength, Point2D* pLocation, RectangleStruct* pBound);

	static int DrawHealthBar_Pip(TechnoClass* pThis, TechnoTypeExt::ExtData* pTypeExt, const bool isBuilding);
	static int DrawHealthBar_PipAmount(TechnoClass* pThis, TechnoTypeExt::ExtData* pTypeExt, int iLength);
	static double GetHealthRatio(TechnoClass* pThis);

	static void InitializeJJConvert(TechnoClass* pThis);
	static void InitializeHugeBar(TechnoClass* pThis);
	static void RemoveHugeBar(TechnoClass* pThis);
	static void ProcessHugeBar();
	static void DrawHugeBar(RulesExt::ExtData::HugeBarData* pConfig, int iCurrent, int iMax);
	static void HugeBar_DrawValue(RulesExt::ExtData::HugeBarData* pConfig, Point2D& posDraw, int iCurrent, int iMax);

	static void ProcessBlinkWeapon(TechnoClass* pThis, AbstractClass* pTarget, WeaponTypeClass* pWeapon);
	static void IonCannonWeapon(TechnoClass* pThis, AbstractClass* pTarget, WeaponTypeClass* pWeapon);
	static void BeamCannon(TechnoClass* pThis, AbstractClass* pTarget, WeaponTypeClass* pWeapon);
	static void DrawSelectBox(TechnoClass* pThis, TechnoTypeExt::ExtData* pTypeExt, Point2D* pLocation, RectangleStruct* pBound, bool isInfantry);
	static void DisplayDamageNumberString(TechnoClass* pThis, int damage, bool isShieldDamage);
	static void FirePassenger(TechnoClass* pThis, AbstractClass* pTarget, WeaponTypeClass* pWeapon);
	static void AllowPassengerToFire(TechnoClass* pThis, AbstractClass* pTarget, WeaponTypeClass* pWeapon);
	static void SpawneLoseTarget(TechnoClass* pThis);
	static void SetWeaponROF(TechnoClass* pThis, WeaponTypeClass* pWeapon);
	static void SetGattlingCount(TechnoClass* pThis, AbstractClass* pTarget, WeaponTypeClass* pWeapon);
	static void ShareWeaponRange(TechnoClass* pThis, AbstractClass* pTarget, WeaponTypeClass* pWeapon);
	static void RememeberFirer(TechnoClass* pThis, AbstractClass* pTarget, WeaponTypeClass* pWeapon);

	static void ReceiveDamageAnim(TechnoClass* pThis, int damage);

	static Point2D GetScreenLocation(TechnoClass* pThis);
	static Point2D GetFootSelectBracketPosition(TechnoClass* pThis, Anchor anchor);
	static Point2D GetBuildingSelectBracketPosition(TechnoClass* pThis, BuildingSelectBracketPosition ePos);
	static void ProcessDigitalDisplays(TechnoClass* pThis);
	static void GetValuesForDisplay(TechnoClass* pThis, DisplayInfoType infoType, int& iCur, int& iMax);

	static void InitializeBuild(TechnoClass* pThis, TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt);
	static void DeleteTheBuild(TechnoClass* pThis);

	static void ProcessAttackedWeapon(TechnoClass* pThis, args_ReceiveDamage* args, bool bBeforeDamageCheck);

	static void PassengerFixed(TechnoClass* pThis);
	static void InitialPayloadFixed(TechnoClass* pThis, TechnoTypeExt::ExtData* pTypeExt);
	static void FixManagers(TechnoClass* pThis);
	static void ChangeLocomotorTo(TechnoClass* pThis, const _GUID& locomotor);

	//Force fire on target
	static BulletClass* SimulatedFire(TechnoClass* pThis, const WeaponStruct& weaponStruct, AbstractClass* pTarget);

	static void AttachEffect(TechnoClass* pThis, TechnoClass* pInvoker, AttachEffectTypeClass* pAEType);
	static void AttachEffect(TechnoClass* pThis, TechnoClass* pInvoker, WarheadTypeExt::ExtData* pWHExt);

	//Building is not supported
	static void Convert(TechnoClass* pThis, TechnoTypeClass* pTargetType, bool bDetachedBuildLimit = false);
	static void RegisterLoss_ClearConvertFromTypesCounter(TechnoClass* pThis);

	static void InitialConvert(TechnoClass* pThis, TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt);
	static void UnitConvert(TechnoClass* pThis, TechnoTypeClass* pTargetType, FootClass* pFirstPassenger);

	static int PickWeaponIndex(TechnoClass* pThis, TechnoClass* pTargetTechno, AbstractClass* pTarget, int weaponIndexOne, int weaponIndexTwo, bool allowFallback);
	static CoordStruct PassengerKickOutLocation(TechnoClass* pThis, FootClass* pPassenger);
};
