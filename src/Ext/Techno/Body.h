#pragma once

#include <map>

#include <TechnoClass.h>

#include <Ext/Rules/Body.h>
#include <Ext/WarheadType/Body.h>

#include <New/Entity/AttachEffectClass.h>
#include <New/Entity/AttachmentClass.h>
#include <New/Entity/GiftBoxClass.h>
#include <New/Entity/LaserTrailClass.h>
#include <New/Entity/ShieldClass.h>
#include <New/Entity/StrafingLaserClass.h>

#include <Utilities/Container.h>

class BulletClass;

class TechnoExt
{
public:
	using base_type = TechnoClass;

	static constexpr DWORD Canary = 0x55555555;
	static constexpr size_t ExtPointerOffset = 0x34C;
	static constexpr uintptr_t VirtualTableAddresses[] =
	{
		AircraftClass::VirtualTableAddress,
		BuildingClass::VirtualTableAddress,
		InfantryClass::VirtualTableAddress,
		UnitClass::VirtualTableAddress
	};

	class ExtData final : public Extension<TechnoClass>
	{
	public:
		TechnoTypeExt::ExtData* TypeExtData = nullptr;
		std::unique_ptr<ShieldClass> Shield = nullptr;
		std::vector<std::unique_ptr<LaserTrailClass>> LaserTrails = {};
		bool ReceiveDamage = false;
		bool LastKillWasTeamTarget = false;
		CDTimerClass PassengerDeletionTimer = {};
		int PassengerDeletionCountDown = -1;
		ShieldTypeClass* CurrentShieldType = nullptr;
		int LastWarpDistance = 0;
		CDTimerClass AutoDeathTimer = {};
		AnimTypeClass* MindControlRingAnimType = nullptr;
		bool IsLeggedCyborg = false;
		OptionalStruct<int, false> DamageNumberOffset = {};
		OptionalStruct<int, true> CurrentLaserWeaponIndex = {};

		//目前没有指针和包含指针的结构体，不需要存档
		struct AEBuff
		{
			double FirepowerMul = 1.0;
			double ROFMul = 1.0;
			double ArmorMul = 1.0;
			double SpeedMul = 1.0;
			double ROTMul = 1.0;
			double RangeMul = 1.0;
			double WeightMul = 1.0;
			int Firepower = 0;
			int ROF = 0;
			int Armor = 0;
			int Speed = 0;
			int ROT = 0;
			int Range = 0;
			double Weight = 0;
			Nullable<bool> DecloakToFire;
			DisableWeaponCate DisableWeapon = DisableWeaponCate::None;
			int LimitMaxPostiveDamage = INT_MAX;
			int LimitMaxNegtiveDamage = INT_MIN;
			int LimitMinPostiveDamage = INT_MIN;
			int LimitMinNegtiveDamage = INT_MAX;
			bool Cloakable = false;
			bool Decloak = false;
			bool DisableTurn = false;
			bool DisableSelect = false;
			bool ImmuneMindControl = false;
			bool HideImage = false;
		}AEBuffs;

		// Used for Passengers.SyncOwner.RevertOnExit instead of TechnoClass::InitialOwner / OriginallyOwnedByHouse,
		// as neither is guaranteed to point to the house the TechnoClass had prior to entering transport and cannot be safely overridden.
		HouseClass* OriginalPassengerOwner = nullptr;

		CDTimerClass AutoDeathTimer_Passenger = {};

		std::vector<CDTimerClass> FireSelf_Timers = {};

		bool IonCannon_setRadius = true;
		int IonCannon_Radius = -1;
		int IonCannon_StartAngle = -180;
		bool IonCannon_Stop = false;
		int IonCannon_Rate = -1;
		int IonCannon_ROF = 0;
		int IonCannon_RadiusReduce = 0;
		int IonCannon_Angle = 0;
		int IonCannon_Scatter_Max = 0;
		int IonCannon_Scatter_Min = 0;
		int IonCannon_Duration = 0;

		WeaponTypeClass* setIonCannonWeapon = nullptr;
		IonCannonTypeClass* setIonCannonType = nullptr;
		bool IonCannonWeapon_setRadius = true;
		int IonCannonWeapon_Radius = -1;
		int IonCannonWeapon_StartAngle = -180;
		bool IonCannonWeapon_Stop = false;
		CoordStruct IonCannonWeapon_Target;
		int IonCannonWeapon_ROF = 0;
		int IonCannonWeapon_RadiusReduce = 0;
		int IonCannonWeapon_Angle = 0;
		int IonCannonWeapon_Scatter_Max = 0;
		int IonCannonWeapon_Scatter_Min = 0;
		int IonCannonWeapon_Duration = 0;

		WeaponTypeClass* setBeamCannon = nullptr;
		bool BeamCannon_setLength = true;
		int BeamCannon_Length = -1;
		bool BeamCannon_Stop = true;
		CoordStruct BeamCannon_Target = CoordStruct::Empty;
		std::vector<CoordStruct> BeamCannon_Center = {};
		int BeamCannon_ROF = 0;
		int BeamCannon_LengthIncrease = 0;

		FootClass* SendPassenger = nullptr;
		bool SendPassengerData = false;
		bool SendPassengerMove = false;
		AffectedHouse SendPassengerMoveHouse = AffectedHouse::Team;
		bool SendPassengerMoveHouse_IgnoreNeturalHouse = true;
		bool SendPassengerOverlap = false;
		bool SendPassengerSelect = false;
		bool SendPassengerUseParachute = true;

		bool AllowPassengerToFire = false;
		int AllowFireCount = 0;

		bool SpawneLoseTarget = true;

		int ShowAnim_LastActivatedFrame = -1;

		int ConvertsCounts = -1;
		TechnoTypeClass* ConvertsOriginalType = nullptr;
		AnimTypeClass* ConvertsAnim = nullptr;
		AnimClass* ProcessingConvertsAnim = nullptr;
		TechnoTypeClass* ConvertsTargetType = nullptr;
		std::vector<TechnoTypeClass*> Convert_FromTypes = {};
		bool Convert_DetachedBuildLimit = false;

		std::unique_ptr<GiftBoxClass> AttachedGiftBox = {};

		AttachmentClass* ParentAttachment = nullptr;
		std::vector<std::unique_ptr<AttachmentClass>> ChildAttachments = {};
		bool ParentInAir = false;

		bool AllowToPaint = false;
		ColorStruct ColorToPaint = { 255,0,0 };
		int Paint_Count = 0;
		bool Paint_IsDiscoColor = false;
		std::vector<ColorStruct> Paint_Colors = {};
		int Paint_TransitionDuration = 0;
		int Paint_FramesPassed = 0;
		bool Paint_IgnoreTintStatus = false;

		bool IsChargeROF = false;
		int GattlingCount = 0;
		int GattlingStage = 0;
		int GattlingWeaponIndex = 0;
		int MaxGattlingCount = 0;
		bool IsCharging = false;
		bool HasCharged = false;
		AbstractClass* AttackTarget = nullptr;
		WeaponTypeClass* AttackWeapon = nullptr;
		std::vector<DynamicVectorClass<WeaponTypeClass*>> GattlingWeapons = {};
		std::vector<DynamicVectorClass<int>> GattlingStages = {};
		CDTimerClass FireUpTimer;

		WeaponTypeClass* PrimaryWeapon = nullptr;
		WeaponTypeClass* SecondaryWeapon = nullptr;
		std::vector<DynamicVectorClass<CoordStruct>> WeaponFLHs = {};

		bool NeedConvertWhenLanding = false;
		bool JJ_Landed = false;
		UnitTypeClass* FloatingType = nullptr;
		UnitTypeClass* LandingType = nullptr;

		std::vector<TechnoTypeClass*> Build_As = {};
		bool Build_As_OnlyOne = false;

		std::vector<int> AttackedWeapon_Timer = {};

		bool IsSharingWeaponRange = false;
		AbstractClass* ShareWeaponRangeTarget = nullptr;
		DirStruct ShareWeaponRangeFacing = DirStruct();

		bool InitialPayload = false;

		TechnoClass* Attacker = nullptr;
		int Attacker_Count = 0;
		WeaponTypeClass* Attacker_Weapon = nullptr;

		int TeamAffectCount = -1;
		bool TeamAffectActive = false;
		int TeamAffectLoseEfficacyCount = -1;
		AnimClass* TeamAffectAnim = nullptr;
		ValueableVector<TechnoClass*> TeamAffectUnits = {};

		bool LosePower = false;
		bool InLosePower = false;
		AnimClass* LosePowerAnim = nullptr;
		int LosePowerParticleCount = 0;

		CDTimerClass PassengerHealTimer = {};
		int PassengerHealCountDown = 0;

		std::vector<std::unique_ptr<AttachEffectClass>> AttachEffects = {};
		std::unordered_map<int, int> AttachEffects_ReceivedCounter = {};

		int DeployAttachEffectsCount = -1;

		std::map<int, std::vector<CDTimerClass>> AttachWeapon_Timers = {};
		bool AttachEffects_Initialized = false;

		std::unordered_map<size_t, int> Temperature = {};
		std::unordered_map<size_t, CDTimerClass> Temperature_HeatUpTimer = {};
		std::unordered_map<size_t, CDTimerClass> Temperature_HeatUpDelayTimer = {};
		std::unordered_map<size_t, CDTimerClass> Temperature_WeaponTimer = {};

		TechnoTypeClass* OrignType = nullptr;
		FootClass* ConvertPassenger = nullptr;
		bool IsConverted = false;
		std::vector<TechnoTypeClass*> Convert_Passengers = {};
		std::vector<TechnoTypeClass*> Convert_Types = {};

		Rank CurrentRank = Rank::Invalid;

		bool AcademyReset = false;

		int ReplacedArmorIdx = 0;
		bool ArmorReplaced = false;

		int PassengerProduct_Timer = 0;

		double ReceiveDamageMultiplier = 1.0;
		int NeedParachute_Height = 0;

		CDTimerClass DeployFireTimer = {};

		AnimClass* DelayedFire_Anim = nullptr;
		int DelayedFire_Anim_LoopCount = 0;
		int DelayedFire_DurationTimer = -1;

		bool Crate_Cloakable = false;

		bool IsInTunnel = false;

		int TargetType = 0;

		bool MoneyReturn_Sold = false;

		size_t SWIdx = 0;

		bool HasChangeLocomotor = false;
		AbstractClass* ChangeLocomotorTarget = nullptr;
		bool IsTypeLocomotor = true;

		std::vector<AttachEffectTypeClass*> NextAttachEffects = {};
		TechnoClass* NextAttachEffectsOwner = nullptr;

		AnimTypeClass* UnitDeathAnim = nullptr;
		HouseClass* UnitDeathAnimOwner = nullptr;
		int UnitFallDestoryHeight = -1;

		HouseClass* LastOwner = nullptr;
		AbstractClass* LastTarget = nullptr;

		int Warp_Count = 0;
		int WarpOut_Count = 0;

		std::vector<std::unique_ptr<TechnoTypeExt::ExtData::AttachmentDataEntry>> AddonAttachmentData = {};

		std::vector<TechnoClass*> ExtraBurstTargets = {};
		int ExtraBurstIndex = 0;
		size_t ExtraBurstTargetIndex = 0;

		TechnoClass* TemporalTarget = nullptr;
		std::vector<TechnoClass*> TemporalTeam = {};
		std::vector<TechnoClass*> TemporalStand = {};
		bool IsTemporalTarget = false;
		std::vector<TechnoClass*> TemporalOwner = {};

		TechnoClass* TemporalStandTarget = nullptr;
		TechnoClass* TemporalStandFirer = nullptr;
		TechnoClass* TemporalStandOwner = nullptr;

		TechnoClass* SyncDeathOwner = nullptr;

		bool WasFallenDown = false;

		bool OnAntiGravity = false;
		bool WasOnAntiGravity = false;
		WarheadTypeClass* AntiGravityType = nullptr;
		HouseClass* AntiGravityOwner = nullptr;

		int CurrtenFallRate = 0;

		WeaponTypeClass* UnitFallWeapon = nullptr;
		bool UnitFallDestory = false;

		bool Landed = true;

		bool Deployed = false;

		SuperClass* CurrentFiringSW = nullptr;
		bool FinishSW = false;
		AnimClass* EMPulseBall = nullptr;

		CDTimerClass BackwarpTimer = {};
		CDTimerClass BackwarpColdDown = {};
		CDTimerClass BackwarpWarpOutTimer = {};
		CoordStruct BackwarpLocation = CoordStruct::Empty;
		int BackwarpHealth = -1;

		std::vector<std::unique_ptr<StrafingLaserClass>> StrafingLasers = {};

		int DisableTurnCount = -1;
		bool FacingInitialized = false;
		DirStruct LastSelfFacing = DirStruct();
		DirStruct LastTurretFacing = DirStruct();

		int LastPassengerCheck = 0;

		InfantryTypeClass* PilotType = nullptr;
		HouseClass* PilotOwner = nullptr;
		int PilotHealth = 0;

		CellStruct SensorCell = CellStruct::Empty;

		TechnoClass* EnterTarget = nullptr;

		bool RopeConnection = false;
		UnitClass* RopeConnection_Vehicle = nullptr;

		AnimClass* PreFireAnim = nullptr;
		bool PreFireFinish = false;

		ExtData(TechnoClass* OwnerObject) : Extension<TechnoClass>(OwnerObject)
		{ }

		void ApplyInterceptor();
		bool CheckDeathConditions();
		void EatPassengers();
		void UpdateShield();
		void ApplySpawnLimitRange();
		void ApplySpawnSameLoseTarget();
		void CheckAttachEffects();
		void UpdateAttackedWeaponTimer();
		void CheckPaintConditions();
		void InfantryConverts();
		void CheckIonCannonConditions();
		void RunIonCannonWeapon();
		void RunBeamCannon();
		void BeamCannonLockFacing();
		void SilentPassenger();
		void ConvertsRecover();
		void RecalculateROT();
		void CheckJJConvertConditions();
		void TechnoUpgradeAnim();
		void TechnoAcademy();
		void TechnoAcademyReset();
		void ProcessFireSelf();
		void ShieldPowered();
		void ForgetFirer();
		void TeamAffect();
		void PoweredUnit();
		void PoweredUnitDown();
		void PoweredUnitDeactivate();
		void ApplyMobileRefinery();
		void PassengerProduct();
		int __fastcall GetArmorIdx(const WeaponTypeClass* pWeapon) const;
		int __fastcall GetArmorIdx(const WarheadTypeClass* pWH) const;
		int GetArmorIdxWithoutShield() const;
		void CheckParachuted();
		void UpdateOnTunnelEnter();
		void __fastcall UpdateTypeData(const TechnoTypeClass* currentType);
		void ControlConverts();
		void MoveConverts();
		void DeployAttachEffect();
		void MoveChangeLocomotor();
		void AttachEffectNext();
		void ShareWeaponRangeTurn();
		void DisableBeSelect();
		void KeepGuard();
		void TemporalTeamCheck();
		void SetSyncDeathOwner();
		void DeathWithSyncDeathOwner();
		void ShouldSinking();
		void AntiGravity();
		void PlayLandAnim();
		bool IsDeployed();
		bool HasAttachedEffects(std::vector<AttachEffectTypeClass*> attachEffectTypes, bool requireAll, bool ignoreSameSource, TechnoClass* pInvoker, AbstractClass* pSource);
		void BackwarpUpdate();
		void Backwarp();
		void UpdateStrafingLaser();
		void SetNeedConvert(TechnoTypeClass* pTargetType, bool detachedBuildLimit, AnimTypeClass* pAnimType = nullptr);
		void ApplySpawnsTiberium();
		void AttachmentsAirFix();
		void CheckPassenger();
		void SelectSW();

		std::vector<AttachEffectClass*> GetActiveAE() const;

		inline void ResetAEBuffs()
		{
			this->AEBuffs = AEBuff();
		}

		void CheckRopeConnection();
		void CheckRopeConnection_Alive();

		virtual ~ExtData() = default;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override;

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
	static void __fastcall WeaponFacingTarget(TechnoClass* pThis);
	static void SelectGattlingWeapon(TechnoClass* pThis, TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt);
	static void TechnoGattlingCount(TechnoClass* pThis, TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt);
	static void ResetGattlingCount(TechnoClass* pThis, TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt);
	static void SetWeaponIndex(TechnoClass* pThis, TechnoExt::ExtData* pExt);
	static void __fastcall CheckTemperature(TechnoClass* pThis);
	static void ReceiveShareDamage(TechnoClass* pThis, args_ReceiveDamage* args, std::vector<TechnoClass*>& teamTechnos);
	static void CurePassengers(TechnoClass* pThis, TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt);
	static void ReturnMoney(TechnoClass* pThis, HouseClass* pHouse, CoordStruct pLocation);
	static void ShareWeaponRangeFire(TechnoClass* pThis, AbstractClass* pTarget);
	//------------------------------------------------------------

	static bool __fastcall IsReallyAlive(const ObjectClass* const pThis);
	static bool __fastcall IsActive(const TechnoClass* const pThis);
	// building needs IsPowerOnline() == true
	static bool __fastcall IsActivePower(const TechnoClass* const pThis);
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
	static CoordStruct GetFLHAbsoluteCoords(TechnoClass* pThis, CoordStruct flh, CoordStruct oriflh, bool isOnTurret);

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
	static void AttachmentsRestore(TechnoClass* pThis);
	static void AttachSelfToTargetAttachments(TechnoClass* pThis, AbstractClass* pTarget, WeaponTypeClass* pWeapon, HouseClass* pHouse);
	static void __fastcall MoveTargetToChild(TechnoClass* pThis);

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
	static void DrawInsignia(TechnoClass* pThis, const Point2D& pLocation, const RectangleStruct& pBounds);
	static void ApplyGainedSelfHeal(TechnoClass* pThis);
	static void SyncIronCurtainStatus(TechnoClass* pFrom, TechnoClass* pTo);
	static void UpdateMindControlAnim(TechnoClass* pThis);

	static void InitializeJJConvert(TechnoClass* pThis);
	static void InitializeHugeBar(TechnoClass* pThis);
	static void RemoveHugeBar(TechnoClass* pThis);
	static void ProcessHugeBar();
	static void DrawHugeBar(RulesExt::ExtData::HugeBarData* pConfig, int iCurrent, int iMax);
	static void HugeBar_DrawValue(RulesExt::ExtData::HugeBarData* pConfig, Point2D& posDraw, int iCurrent, int iMax);

	static void ProcessBlinkWeapon(TechnoClass* pThis, AbstractClass* pTarget, WeaponTypeClass* pWeapon);
	static void IonCannonWeapon(TechnoClass* pThis, AbstractClass* pTarget, WeaponTypeClass* pWeapon);
	static void BeamCannon(TechnoClass* pThis, AbstractClass* pTarget, WeaponTypeClass* pWeapon);
	static void DrawSelectBox(TechnoClass* pThis, const Point2D& location, const RectangleStruct& bound, bool isInfantry);
	static void DisplayDamageNumberString(TechnoClass* pThis, int damage, bool isShieldDamage);
	static void FirePassenger(TechnoClass* pThis, WeaponTypeExt::ExtData* pWeaponExt);
	static void FireSelf(TechnoClass* pThis, WeaponTypeExt::ExtData* pWeaponExt);
	static void AllowPassengerToFire(TechnoClass* pThis, AbstractClass* pTarget, WeaponTypeClass* pWeapon);
	static void SpawneLoseTarget(TechnoClass* pThis);
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
	static void InitialPayloadFixed(TechnoClass* pThis);
	static void FixManagers(TechnoClass* pThis);
	static void ChangeLocomotorTo(TechnoClass* pThis, const CLSID& locomotor);

	//Force fire on target
	static BulletClass* SimulatedFire(TechnoClass* pThis, const WeaponStruct& weaponStruct, AbstractClass* pTarget);
	static BulletClass* SimulatedFireWithoutStand(TechnoClass* pThis, const WeaponStruct& weaponStruct, AbstractClass* pTarget);
	static BulletClass* SimulatedFire(TechnoClass* pThis, WeaponTypeClass* pWeapon, const CoordStruct& sourceCoords, AbstractClass* pTarget);
	
	static void AttachEffect(TechnoClass* pThis, TechnoClass* pInvoker, AttachEffectTypeClass* pAEType);
	static void AttachEffect(TechnoClass* pThis, TechnoClass* pInvoker, WarheadTypeExt::ExtData* pWHExt);

	//Building is not supported
	static void Convert(TechnoClass* pThis, TechnoTypeClass* pTargetType, bool bDetachedBuildLimit = false);
	static void RegisterLoss_ClearConvertFromTypesCounter(TechnoClass* pThis);

	static void InitialConvert(TechnoClass* pThis, TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt);
	static void UnitConvert(TechnoClass* pThis, TechnoTypeClass* pTargetType, FootClass* pFirstPassenger);

	static int PickWeaponIndex(TechnoClass* pThis, TechnoClass* pTargetTechno, AbstractClass* pTarget, int weaponIndexOne, int weaponIndexTwo, bool allowFallback);
	static CoordStruct PassengerKickOutLocation(TechnoClass* pThis, FootClass* pPassenger);

	static bool CheckCanBuildUnitType(TechnoClass* pThis, int HouseIdx);

	static void DrawGroupID_Building(TechnoClass* pThis, HealthBarTypeClass* pHealthBar,const Point2D* pLocation);
	static void DrawGroupID_Other(TechnoClass* pThis, HealthBarTypeClass* pHealthBar,const Point2D* pLocation);
	static void DrawHealthBar_Building(TechnoClass* pThis, HealthBarTypeClass* pHealthBar, int iLength,const Point2D* pLocation,const RectangleStruct* pBound);
	static void DrawHealthBar_Other(TechnoClass* pThis, HealthBarTypeClass* pHealthBar, int iLength,const Point2D* pLocation,const RectangleStruct* pBound);
	static void DrawHealthBar_Picture(TechnoClass* pThis, HealthBarTypeClass* pHealthBar, int iLength,const Point2D* pLocation,const RectangleStruct* pBound);
	static int DrawHealthBar_Pip(TechnoClass* pThis, HealthBarTypeClass* pHealthBar, const bool isBuilding);
	static int DrawHealthBar_PipAmount(TechnoClass* pThis, int Minimum, int iLength);
	static double GetHealthRatio(TechnoClass* pThis);
	static HealthBarTypeClass* GetHealthBarType(TechnoClass* pThis, bool isShield);
	static void ChangeAmmo(TechnoClass* pThis, int ammo);
	static void SetTemporalTeam(TechnoClass* pThis, TechnoClass* pTarget, WarheadTypeExt::ExtData* pWHExt);
	static void FallenDown(TechnoClass* pThis);
	static void InfantryOnWaterFix(TechnoClass* pThis);
	static void __fastcall FallRateFix(TechnoClass* pThis);
	static int GetSensorSight(TechnoClass* pThis);

	static int GetCurrentDamage(int damage, FootClass* pThis);
	static CoordStruct GetFLH(TechnoClass* pThis, int idxWeapon);
	static void DeleteStrafingLaser(TechnoClass* pThis, TechnoExt::ExtData* pExt);
	static void AddSensorsAt(int houseindex, int range, CellStruct cell);
	static void RemoveSensorsAt(int houseindex, int range, CellStruct cell);
	static int TechnoFactoryPlant(TechnoTypeClass* pThis, HouseClass* pHouse);
	static PhobosAction GetActionPilot(InfantryClass* pThis, TechnoClass* pTarget);
	static bool PerformActionPilot(InfantryClass* pThis, TechnoClass* pTarget);

	static void KickOutPassenger(TechnoClass* pThis, int WeaponIdx);
	static void KickOutPassenger_SetHight(FootClass* pThis, CoordStruct location, short facing, bool parachute);
};
