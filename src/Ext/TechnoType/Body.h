#pragma once
#include <TechnoTypeClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>
#include <Utilities/Enum.h>

#include <New/Type/ShieldTypeClass.h>
#include <New/Type/LaserTrailTypeClass.h>
#include <New/Type/AttachmentTypeClass.h>
#include <New/Type/DigitalDisplayTypeClass.h>
#include <New/Type/IonCannonTypeClass.h>
#include <New/Type/GScreenAnimTypeClass.h>
#include <New/Type/AttachEffectTypeClass.h>

class Matrix3D;
class ParticleSystemTypeClass;

class TechnoTypeExt
{
public:
	using base_type = TechnoTypeClass;

	class ExtData final : public Extension<TechnoTypeClass>
	{
	public:
		Valueable<bool> HealthBar_Hide;
		Valueable<CSFText> UIDescription;
		Valueable<bool> LowSelectionPriority;
		PhobosFixedString<0x20> GroupAs;
		Valueable<int> RadarJamRadius;
		Nullable<int> InhibitorRange;
		Valueable<Leptons> MindControlRangeLimit;
		Valueable<bool> Interceptor;
		Nullable<bool> Interceptor_Rookie;
		Nullable<bool> Interceptor_Veteran;
		Nullable<bool> Interceptor_Elite;
		Valueable<AffectedHouse> Interceptor_CanTargetHouses;
		Promotable<Leptons> Interceptor_GuardRange;
		Promotable<Leptons> Interceptor_MinimumGuardRange;
		Valueable<int> Interceptor_Weapon;
		Nullable<bool> Interceptor_DeleteOnIntercept;
		Nullable<WeaponTypeClass*> Interceptor_WeaponOverride;
		Valueable<bool> Interceptor_WeaponReplaceProjectile;
		Valueable<bool> Interceptor_WeaponCumulativeDamage;
		Valueable<bool> Interceptor_KeepIntact;
		Valueable<int> Interceptor_Success;
		Valueable<int> Interceptor_RookieSuccess;
		Valueable<int> Interceptor_VeteranSuccess;
		Valueable<int> Interceptor_EliteSuccess;
		Valueable<CoordStruct> TurretOffset;
		Valueable<bool> Powered_KillSpawns;

		Valueable<bool> Spawner_LimitRange;
		Valueable<int> Spawner_ExtraLimitRange;
		Nullable<int> Spawner_DelayFrames;
		ValueableVector<int> Spawner_DelayFrams_PerSpawn;
		ValueableVector<AircraftTypeClass*> Spawn_Types;
		ValueableVector<int> Spawn_Nums;
		ValueableVector<int> Spawn_RegenRate;
		ValueableVector<int> Spawn_ReloadRate;

		Nullable<bool> Harvester_Counted;
		Valueable<bool> Promote_IncludeSpawns;
		Valueable<bool> ImmuneToCrit;
		Valueable<bool> MultiMindControl_ReleaseVictim;
		Valueable<int> CameoPriority;
		Valueable<bool> NoManualMove;
		Nullable<int> InitialStrength;
		Valueable<int> PassengerDeletion_Rate;
		Valueable<bool> PassengerDeletion_Rate_SizeMultiply;
		Valueable<bool> PassengerDeletion_UseCostAsRate;
		Valueable<double> PassengerDeletion_CostMultiplier;
		Valueable<bool> PassengerDeletion_Soylent;
		Valueable<bool> PassengerDeletion_SoylentFriendlies;
		Valueable<bool> PassengerDeletion_DisplaySoylent;
		Valueable<double> PassengerDeletion_SoylentMultiplier;
		Valueable<AffectedHouse> PassengerDeletion_DisplaySoylentToHouses;
		Valueable<Point2D> PassengerDeletion_DisplaySoylentOffset;
		NullableIdx<VocClass> PassengerDeletion_ReportSound;
		Nullable<AnimTypeClass*> PassengerDeletion_Anim;

		Valueable<bool> AutoDeath_OnAmmoDepletion;
		Valueable<int> AutoDeath_AfterDelay;
		Nullable<AutoDeathBehavior> AutoDeath_Behavior;
		ValueableVector<TechnoTypeClass*> AutoDeath_Nonexist;
		Valueable<AffectedHouse> AutoDeath_Nonexist_House;
		ValueableVector<TechnoTypeClass*> AutoDeath_Exist;
		Valueable<AffectedHouse> AutoDeath_Exist_House;

		Valueable<SlaveChangeOwnerType> Slaved_OwnerWhenMasterKilled;
		NullableIdx<VocClass> SellSound;
		NullableIdx<VoxClass> EVA_Sold;

		Valueable<ShieldTypeClass*> ShieldType;

		ValueableVector<AnimTypeClass*> WarpOut;
		ValueableVector<AnimTypeClass*> WarpIn;
		ValueableVector<AnimTypeClass*> WarpAway;
		Nullable<bool> ChronoTrigger;
		Nullable<int> ChronoDistanceFactor;
		Nullable<int> ChronoMinimumDelay;
		Nullable<int> ChronoRangeMinimum;
		Nullable<int> ChronoDelay;

		Nullable<WeaponTypeClass*> WarpInWeapon;
		Nullable<WeaponTypeClass*> WarpInMinRangeWeapon;
		Nullable<WeaponTypeClass*> WarpOutWeapon;
		Valueable<bool> WarpInWeapon_UseDistanceAsDamage;

		ValueableVector<AnimTypeClass*> OreGathering_Anims;
		ValueableVector<int> OreGathering_Tiberiums;
		ValueableVector<int> OreGathering_FramesPerDir;

		std::vector<DynamicVectorClass<CoordStruct>> WeaponBurstFLHs;
		std::vector<DynamicVectorClass<CoordStruct>> VeteranWeaponBurstFLHs;
		std::vector<DynamicVectorClass<CoordStruct>> EliteWeaponBurstFLHs;

		Valueable<bool> DestroyAnim_Random;
		Valueable<bool> NotHuman_RandomDeathSequence;

		Nullable<InfantryTypeClass*> DefaultDisguise;
		Valueable<bool> UseDisguiseMovementSpeed;

		Nullable<int> OpenTopped_RangeBonus;
		Nullable<float> OpenTopped_DamageMultiplier;
		Nullable<int> OpenTopped_WarpDistance;
		Valueable<bool> OpenTopped_IgnoreRangefinding;
		Valueable<bool> OpenTopped_AllowFiringIfDeactivated;

		Valueable<bool> AllowPlanningMode;

		struct AttachmentDataEntry
		{
			AttachmentTypeClass* Type;
			TechnoTypeClass* TechnoType;
			CoordStruct FLH;
			bool IsOnTurret;

			AttachmentDataEntry() = default;

			AttachmentDataEntry(AttachmentTypeClass* pAttachmentType, TechnoTypeClass* pTechnoType, CoordStruct crdFLH, bool isOnTurret)
				:Type(pAttachmentType), TechnoType(pTechnoType), FLH(crdFLH), IsOnTurret(isOnTurret)
			{ }

			~AttachmentDataEntry() = default;

			bool Load(PhobosStreamReader& stm, bool registerForChange);
			bool Save(PhobosStreamWriter& stm) const;

		private:
			template <typename T>
			bool Serialize(T& stm);
		};

		std::vector<std::unique_ptr<AttachmentDataEntry>> AttachmentData;

		Valueable<bool> AutoFire;
		Valueable<bool> AutoFire_TargetSelf;

		Valueable<bool> NoSecondaryWeaponFallback;

		Valueable<int> NoAmmoWeapon;
		Valueable<int> NoAmmoAmount;

		Nullable<bool> JumpjetAllowLayerDeviation;
		Nullable<bool> JumpjetTurnToTarget;

		Valueable<bool> DeployingAnim_AllowAnyDirection;
		Valueable<bool> DeployingAnim_KeepUnitVisible;
		Valueable<bool> DeployingAnim_ReverseForUndeploy;
		Valueable<bool> DeployingAnim_UseUnitDrawer;

		Valueable<CSFText> EnemyUIName;
		Valueable<int> ForceWeapon_Naval_Decloaked;

		Valueable<bool> Ammo_Shared;
		Valueable<int> Ammo_Shared_Group;

		Valueable<bool> Passengers_ChangeOwnerWithTransport;

		Nullable<SelfHealGainType> SelfHealGainType;
		Valueable<bool> Passengers_SyncOwner;
		Valueable<bool> Passengers_SyncOwner_RevertOnExit;

		Promotable<SHPStruct*> Insignia;
		Valueable<Vector3D<int>> InsigniaFrames;
		Promotable<int> InsigniaFrame;
		Nullable<bool> Insignia_ShowEnemy;

		Valueable<Vector2D<double>> InitialStrength_Cloning;

		struct LaserTrailDataEntry
		{
			ValueableIdx<LaserTrailTypeClass> Type;
			Valueable<CoordStruct> FLH;
			Valueable<bool> IsOnTurret;

			bool Load(PhobosStreamReader& stm, bool registerForChange);
			bool Save(PhobosStreamWriter& stm) const;

		private:
			template <typename T>
			bool Serialize(T& stm);
		};

		ValueableVector<LaserTrailDataEntry> LaserTrailData;

		struct GiftBoxDataEntry
		{
			ValueableVector<TechnoTypeClass*> GiftBox_Types;
			ValueableVector<int> GiftBox_Nums;
			Valueable<bool> GiftBox_Remove;
			Valueable<bool> GiftBox_Destroy;
			Valueable<int> GiftBox_Delay;
			Valueable<Point2D> GiftBox_DelayMinMax;
			Valueable<int> GiftBox_CellRandomRange;
			Valueable<bool> GiftBox_EmptyCell;
			Valueable<bool> GiftBox_RandomType;

			GiftBoxDataEntry() :
				GiftBox_Types {}
				, GiftBox_Nums {}
				, GiftBox_Remove { true }
				, GiftBox_Destroy { false }
				, GiftBox_Delay { 0 }
				, GiftBox_DelayMinMax { { 0,0 } }
				, GiftBox_CellRandomRange { 0 }
				, GiftBox_EmptyCell { false }
				, GiftBox_RandomType { true }
			{ }

			operator bool() const
			{
				return !GiftBox_Types.empty() && !GiftBox_Nums.empty();
			}

			bool Load(PhobosStreamReader& stm, bool registerForChange);
			bool Save(PhobosStreamWriter& stm) const;

		private:
			template <typename T>
			bool Serialize(T& stm);
		};

		GiftBoxDataEntry GiftBoxData;

		Nullable<SHPStruct*> SelectBox_Shape;
		CustomPalette SelectBox_Palette;
		Valueable<Vector3D<int>> SelectBox_Frame;
		Nullable<Vector2D<int>> SelectBox_DrawOffset;
		Nullable<int> SelectBox_TranslucentLevel;
		Nullable<AffectedHouse> SelectBox_CanSee;
		Nullable<bool> SelectBox_CanObserverSee;

		Valueable<bool> CanRepairCyborgLegs;

		Nullable<CoordStruct> PronePrimaryFireFLH;
		Nullable<CoordStruct> ProneSecondaryFireFLH;
		Nullable<CoordStruct> DeployedPrimaryFireFLH;
		Nullable<CoordStruct> DeployedSecondaryFireFLH;
		std::vector<DynamicVectorClass<CoordStruct>> CrouchedWeaponBurstFLHs;
		std::vector<DynamicVectorClass<CoordStruct>> VeteranCrouchedWeaponBurstFLHs;
		std::vector<DynamicVectorClass<CoordStruct>> EliteCrouchedWeaponBurstFLHs;
		std::vector<DynamicVectorClass<CoordStruct>> DeployedWeaponBurstFLHs;
		std::vector<DynamicVectorClass<CoordStruct>> VeteranDeployedWeaponBurstFLHs;
		std::vector<DynamicVectorClass<CoordStruct>> EliteDeployedWeaponBurstFLHs;

		Valueable<bool> MobileRefinery;
		Valueable<int> MobileRefinery_TransRate;
		Valueable<float> MobileRefinery_CashMultiplier;
		Valueable<int> MobileRefinery_AmountPerCell;
		ValueableVector<double> MobileRefinery_FrontOffset;
		ValueableVector<double> MobileRefinery_LeftOffset;
		Valueable<bool> MobileRefinery_Display;
		Valueable<ColorStruct> MobileRefinery_DisplayColor;
		ValueableVector<AnimTypeClass*> MobileRefinery_Anims;
		Valueable<bool> MobileRefinery_AnimMove;

		NullableVector<int> Overload_Count;
		NullableVector<int> Overload_Damage;
		NullableVector<int> Overload_Frames;
		NullableIdx<VocClass> Overload_DeathSound;
		Nullable<ParticleSystemTypeClass*> Overload_ParticleSys;
		Nullable<int> Overload_ParticleSysCount;
		Valueable<bool> Draw_MindControlLink;

		ValueableVector<DigitalDisplayTypeClass*> DigitalDisplayTypes;
		Valueable<bool> DigitalDisplay_Disable;

		ValueableVector<TechnoTypeClass*> RandomProduct;

		Valueable<bool> HugeBar;
		Valueable<int> HugeBar_Priority;

		DamageableVector<WeaponTypeClass*> FireSelf_Weapon;
		DamageableVector<int> FireSelf_ROF;
		Damageable<bool> FireSelf_Immediately;

		Nullable<IonCannonTypeClass*> IonCannonType;

		ConvertClass* SHP_PipsPAL;
		SHPStruct* SHP_PipsSHP;
		ConvertClass* SHP_PipBrdPAL;
		SHPStruct* SHP_PipBrdSHP;
		Nullable<Vector3D<int>> HealthBar_Pips;
		Nullable<Vector2D<int>> HealthBar_Pips_DrawOffset;
		Nullable<int> HealthBar_PipsLength;
		PhobosFixedString<32U> HealthBar_PipsSHP;
		PhobosFixedString<32U> HealthBar_PipsPAL;
		Nullable<int> HealthBar_PipBrd;
		PhobosFixedString<32U> HealthBar_PipBrdSHP;
		PhobosFixedString<32U> HealthBar_PipBrdPAL;
		Nullable<Vector2D<int>> HealthBar_PipBrdOffset;
		Nullable<int> HealthBar_XOffset;
		Valueable<bool> UseNewHealthBar;
		PhobosFixedString<32U> HealthBar_PictureSHP;
		PhobosFixedString<32U> HealthBar_PicturePAL;
		Valueable<int> HealthBar_PictureTransparency;
		SHPStruct* SHP_PictureSHP;
		ConvertClass* SHP_PicturePAL;

		Nullable<Vector2D<int>> GroupID_Offset;
		Nullable<Vector2D<int>> SelfHealPips_Offset;
		Valueable<bool> UseCustomHealthBar;
		Valueable<bool> UseUnitHealthBar;

		Nullable<GScreenAnimTypeClass*> GScreenAnimType;

		Valueable<bool> MovePassengerToSpawn;
		Valueable<bool> SilentPassenger;
		Valueable<bool> Spawner_SameLoseTarget;

		Valueable<bool> DeterminedByRange;
		Valueable<int> DeterminedByRange_ExtraRange;
		Valueable<int> DeterminedByRange_MainWeapon;

		ValueableVector<TechnoTypeClass*> BuildLimit_Group_Types;
		Valueable<bool> BuildLimit_Group_Any;
		ValueableVector<int> BuildLimit_Group_Limits;

		ValueableVector<TechnoTypeClass*> BuildLimit_As;

		Valueable<bool> VehicleImmuneToMindControl;

		TechnoTypeClass* Convert_Deploy;
		Valueable<AnimTypeClass*> Convert_DeployAnim;

		Valueable<bool> Gattling_SelectWeaponByVersus;
		Valueable<bool> IsExtendGattling;
		Valueable<bool> Gattling_Cycle;
		Valueable<bool> Gattling_Charge;

		PromotableVector<WeaponStruct> Weapons;

		std::vector<DynamicVectorClass<int>> Stages;
		std::vector<DynamicVectorClass<int>> VeteranStages;
		std::vector<DynamicVectorClass<int>> EliteStages;
		std::vector<DynamicVectorClass<CoordStruct>> WeaponFLHs;
		std::vector<DynamicVectorClass<CoordStruct>> VeteranWeaponFLHs;
		std::vector<DynamicVectorClass<CoordStruct>> EliteWeaponFLHs;

		Valueable<WeaponTypeClass*> OccupyWeapon;
		Valueable<WeaponTypeClass*> VeteranOccupyWeapon;
		Valueable<WeaponTypeClass*> EliteOccupyWeapon;

		Valueable<UnitTypeClass*> JJConvert_Unload;

		Nullable<bool> IronCurtain_KeptOnDeploy;
		Nullable<IronCurtainAffects> IronCurtain_Affect;
		Nullable<WarheadTypeClass*> IronCuratin_KillWarhead;

		ValueableVector<WeaponTypeClass*> AttackedWeapon;
		ValueableVector<WeaponTypeClass*> AttackedWeapon_Veteran;
		ValueableVector<WeaponTypeClass*> AttackedWeapon_Elite;
		ValueableVector<int> AttackedWeapon_ROF;
		ValueableVector<int> AttackedWeapon_FireToAttacker;
		ValueableVector<int> AttackedWeapon_IgnoreROF;
		ValueableVector<int> AttackedWeapon_IgnoreRange;
		ValueableVector<int> AttackedWeapon_Range;
		ValueableVector<WarheadTypeClass*> AttackedWeapon_ResponseWarhead;
		ValueableVector<WarheadTypeClass*> AttackedWeapon_NoResponseWarhead;
		ValueableVector<int> AttackedWeapon_ResponseZeroDamage;
		std::vector<AffectedHouse> AttackedWeapon_ResponseHouse;
		ValueableVector<int> AttackedWeapon_ActiveMaxHealth;
		ValueableVector<int> AttackedWeapon_ActiveMinHealth;
		std::vector<CoordStruct> AttackedWeapon_FLHs;

		Promotable<WeaponTypeClass*> WeaponInTransport;

		Valueable<bool> ProtectPassengers;
		Valueable<bool> ProtectPassengers_Clear;
		Valueable<bool> ProtectPassengers_Release;
		Valueable<bool> ProtectPassengers_Damage;

		Valueable<AffectedHouse> Dodge_Houses;
		Valueable<double> Dodge_MaxHealthPercent;
		Valueable<double> Dodge_MinHealthPercent;
		Valueable<double> Dodge_Chance;
		Nullable<AnimTypeClass*> Dodge_Anim;
		Valueable<bool> Dodge_OnlyDodgePositiveDamage;

		Valueable<int> MoveDamage;
		Valueable<int> MoveDamage_Delay;
		Nullable<WarheadTypeClass*> MoveDamage_Warhead;
		Nullable<AnimTypeClass*> MoveDamage_Anim;
		Valueable<int> StopDamage;
		Valueable<int> StopDamage_Delay;
		Nullable<WarheadTypeClass*> StopDamage_Warhead;
		Valueable<AnimTypeClass*> StopDamage_Anim;

		ValueableVector<TechnoTypeClass*> WeaponRangeShare_Technos;
		Valueable<double> WeaponRangeShare_Range;
		Valueable<bool> WeaponRangeShare_ForceAttack;
		Valueable<int> WeaponRangeShare_UseWeapon;

		Nullable<int> AllowMinHealth;

		Valueable<Vector2D<int>> AllowMaxDamage;
		Valueable<Vector2D<int>> AllowMinDamage;

		Valueable<bool> ImmuneToAbsorb;

		ValueableVector<AttachEffectTypeClass*> AttachEffects;
		ValueableVector<int> AttachEffects_Duration;
		ValueableVector<int> AttachEffects_Delay;
		ValueableVector<AttachEffectTypeClass*> AttachEffects_Immune;
		NullableVector<AttachEffectTypeClass*> AttachEffects_OnlyAccept;

		Valueable<bool> TeamAffect;
		Valueable<double> TeamAffect_Range;
		ValueableVector<TechnoTypeClass*> TeamAffect_Technos;
		Valueable<AffectedHouse> TeamAffect_Houses;
		Valueable<int> TeamAffect_Number;
		Valueable<WeaponTypeClass*> TeamAffect_Weapon;
		Nullable<int> TeamAffect_ROF;
		Valueable<WeaponTypeClass*> TeamAffect_LoseEfficacyWeapon;
		Nullable<int> TeamAffect_LoseEfficacyROF;
		Nullable<AnimTypeClass*> TeamAffect_Anim;
		Valueable<bool> TeamAffect_ShareDamage;
		Valueable<int> TeamAffect_MaxNumber;

		ValueableVector<TechnoTypeClass*> PoweredUnitBy;
		Valueable<bool> PoweredUnitBy_Any;
		Nullable<AnimTypeClass*> PoweredUnitBy_Sparkles;
		Nullable<ParticleSystemTypeClass*> PoweredUnitBy_ParticleSystem;
		Valueable<Vector2D<int>> PoweredUnitBy_ParticleSystemXOffset;
		Valueable<Vector2D<int>> PoweredUnitBy_ParticleSystemYOffset;
		Valueable<int> PoweredUnitBy_ParticleSystemSpawnDelay;

		Valueable<int> PassengerHeal_Rate;
		Valueable<bool> PassengerHeal_HealAll;
		Valueable<int> PassengerHeal_Amount;
		Valueable<AffectedHouse> PassengerHeal_Houses;
		NullableIdx<VocClass> PassengerHeal_Sound;
		Nullable<AnimTypeClass*> PassengerHeal_Anim;

		std::unordered_map<size_t, int> Temperature;
		std::unordered_map<size_t, int> Temperatrue_Disable;
		std::unordered_map<size_t, int> Temperature_HeatUpFrame;
		std::unordered_map<size_t, int> Temperature_HeatUpAmount;
		std::unordered_map<size_t, int> Temperature_HeatUpDelay;

		std::vector<DynamicVectorClass<int>> Turrets;

		Valueable<bool> UseConvert;
		ValueableIdxVector<TechnoTypeClass> Convert_Passengers;
		ValueableIdxVector<TechnoTypeClass> Convert_Types;

		Valueable<AnimTypeClass*> VeteranAnim;
		Valueable<AnimTypeClass*> EliteAnim;

		Valueable<bool> PassengerProduct;
		ValueableVector<TechnoTypeClass*> PassengerProduct_Type;
		Valueable<int> PassengerProduct_Rate;
		Valueable<int> PassengerProduct_Amount;
		Valueable<bool> PassengerProduct_RandomPick;

		//Ares
		ValueableVector<TechnoTypeClass*> InitialPayload_Types;
		ValueableVector<int> InitialPayload_Nums;
		ValueableVector<BuildingTypeClass*> BuiltAt;
		Nullable<int> TurretROT;
		Valueable<bool> Passengers_BySize;

		/*
		EatPassengers
		MovePassengerToSpawn
		IonCannon
		AutoDeath
		AttackedWeapon
		PoweredShield
		PassengerHeal
		*/
		bool Subset_1_Used() const;

		/*
		EatPassengers
		MovePassengerToSpawn
		IonCannon
		AutoDeath
		AttackedWeapon
		PoweredShield
		PassengerHeal
		*/
		bool Subset_1 = false;

		/*
		SilentPassenger
		Spawner_SameLoseTarget
		Powered_KillSpawns
		Spawn_LimitRange
		MindControlRange
		Veteran/Elite Anim
		MobileRefinery
		*/
		bool Subset_2_Used() const;
		/*
		SilentPassenger
		Spawner_SameLoseTarget
		Powered_KillSpawns
		Spawn_LimitRange
		MindControlRange
		Veteran/Elite Anim
		MobileRefinery
		*/
		bool Subset_2 = false;

		/*
		LaserTrails
		ExtendGattling
		FireSelf
		VeteranWeapon
		TeamAffect
		PoweredUnit
		*/
		bool Subset_3_Used() const;
		/*
		LaserTrails
		ExtendGattling
		FireSelf
		VeteranWeapon
		TeamAffect
		PoweredUnit
		*/
		bool Subset_3 = false;

		bool CanBeBuiltAt_Ares(BuildingTypeClass* pFactoryType);

		ExtData(TechnoTypeClass* OwnerObject) : Extension<TechnoTypeClass>(OwnerObject)
			, HealthBar_Hide { false }
			, UIDescription {}
			, LowSelectionPriority { false }
			, GroupAs { NONE_STR }
			, RadarJamRadius { 0 }
			, InhibitorRange { }
			, MindControlRangeLimit {}

			, Interceptor { false }
			, Interceptor_CanTargetHouses { AffectedHouse::Enemies }
			, Interceptor_Rookie {}
			, Interceptor_Veteran {}
			, Interceptor_Elite {}
			, Interceptor_GuardRange {}
			, Interceptor_MinimumGuardRange {}
			, Interceptor_Weapon { 0 }
			, Interceptor_DeleteOnIntercept {}
			, Interceptor_WeaponOverride {}
			, Interceptor_WeaponReplaceProjectile { false }
			, Interceptor_WeaponCumulativeDamage { false }
			, Interceptor_KeepIntact { false }
			, Interceptor_Success { 100 }
			, Interceptor_RookieSuccess { -1 }
			, Interceptor_VeteranSuccess { -1 }
			, Interceptor_EliteSuccess { -1 }

			, TurretOffset { { 0, 0, 0 } }

			, Powered_KillSpawns { false }
			, Spawner_LimitRange { false }
			, Spawner_ExtraLimitRange { 0 }
			, Spawner_DelayFrames {}
			, Spawner_DelayFrams_PerSpawn {}
			, Spawn_Types {}
			, Spawn_Nums {}
			, Spawn_RegenRate {}
			, Spawn_ReloadRate {}

			, Harvester_Counted {}
			, Promote_IncludeSpawns { false }
			, ImmuneToCrit { false }
			, MultiMindControl_ReleaseVictim { false }
			, CameoPriority { 0 }
			, NoManualMove { false }
			, InitialStrength {}

			, ShieldType {}

			, WarpOut {}
			, WarpIn {}
			, WarpAway {}
			, ChronoTrigger {}
			, ChronoDistanceFactor {}
			, ChronoMinimumDelay {}
			, ChronoRangeMinimum {}
			, ChronoDelay {}
			, WarpInWeapon {}
			, WarpInMinRangeWeapon {}
			, WarpOutWeapon {}
			, WarpInWeapon_UseDistanceAsDamage { false }

			, OreGathering_Anims {}
			, OreGathering_Tiberiums {}
			, OreGathering_FramesPerDir {}

			, LaserTrailData {}
			, DestroyAnim_Random { true }
			, NotHuman_RandomDeathSequence { false }
			, GiftBoxData {}

			, PassengerDeletion_Rate { 0 }
			, PassengerDeletion_Rate_SizeMultiply { true }
			, PassengerDeletion_UseCostAsRate { false }
			, PassengerDeletion_CostMultiplier { 1.0 }
			, PassengerDeletion_Soylent { false }
			, PassengerDeletion_SoylentMultiplier { 1.0 }
			, PassengerDeletion_SoylentFriendlies { false }
			, PassengerDeletion_DisplaySoylent { false }
			, PassengerDeletion_DisplaySoylentToHouses { AffectedHouse::All }
			, PassengerDeletion_DisplaySoylentOffset { { 0, 0 } }
			, PassengerDeletion_ReportSound {}
			, PassengerDeletion_Anim {}

			, DefaultDisguise {}
			, UseDisguiseMovementSpeed {}

			, OpenTopped_RangeBonus {}
			, OpenTopped_DamageMultiplier {}
			, OpenTopped_WarpDistance {}
			, OpenTopped_IgnoreRangefinding { false }
			, OpenTopped_AllowFiringIfDeactivated { true }

			, AllowPlanningMode { true }

			, AttachmentData {}
			, AutoFire { false }
			, AutoFire_TargetSelf { false }
			, NoSecondaryWeaponFallback { false }
			, NoAmmoWeapon { -1 }
			, NoAmmoAmount { 0 }
			, JumpjetAllowLayerDeviation {}
			, JumpjetTurnToTarget {}
			, DeployingAnim_AllowAnyDirection { false }
			, DeployingAnim_KeepUnitVisible { false }
			, DeployingAnim_ReverseForUndeploy { true }
			, DeployingAnim_UseUnitDrawer { true }

			, AutoDeath_Behavior { }
			, AutoDeath_OnAmmoDepletion { false }
			, AutoDeath_AfterDelay { 0 }
			, AutoDeath_Nonexist {}
			, AutoDeath_Nonexist_House { AffectedHouse::Owner }
			, AutoDeath_Exist {}
			, AutoDeath_Exist_House{ AffectedHouse::Owner }

			, Slaved_OwnerWhenMasterKilled { SlaveChangeOwnerType::Killer }
			, SellSound { }
			, EVA_Sold { }
			, EnemyUIName {}
			, ForceWeapon_Naval_Decloaked { -1 }
			, Ammo_Shared { false }
			, Ammo_Shared_Group { -1 }
			, Passengers_ChangeOwnerWithTransport { false }

			, SelectBox_Shape {}
			, SelectBox_Palette {}
			, SelectBox_Frame { { -1, -1, -1 } }
			, SelectBox_DrawOffset {}
			, SelectBox_TranslucentLevel {}
			, SelectBox_CanSee {}
			, SelectBox_CanObserverSee {}

			, PronePrimaryFireFLH { }
			, ProneSecondaryFireFLH { }
			, DeployedPrimaryFireFLH { }
			, DeployedSecondaryFireFLH { }
			, CanRepairCyborgLegs { false }
			, Draw_MindControlLink { true }
			, Overload_Count {}
			, Overload_Damage {}
			, Overload_Frames {}
			, Overload_DeathSound {}
			, Overload_ParticleSys {}
			, Overload_ParticleSysCount {}
			, SelfHealGainType {}
			, Passengers_SyncOwner { false }
			, Passengers_SyncOwner_RevertOnExit { true }
			, Insignia {}
			, InsigniaFrame { -1 }
			, InsigniaFrames { { -1, -1, -1 } }
			, Insignia_ShowEnemy {}
			, InitialStrength_Cloning { { 1.0, 0.0 } }

			, MobileRefinery { false }
			, MobileRefinery_TransRate { 30 }
			, MobileRefinery_CashMultiplier { 1.0 }
			, MobileRefinery_AmountPerCell { 0 }
			, MobileRefinery_FrontOffset { }
			, MobileRefinery_LeftOffset { }
			, MobileRefinery_Display { true }
			, MobileRefinery_DisplayColor { { 57,197,187 } }
			, MobileRefinery_Anims { }
			, MobileRefinery_AnimMove { true }

			, DigitalDisplayTypes {}
			, DigitalDisplay_Disable { false }
			, HugeBar { false }
			, HugeBar_Priority { -1 }

			, IonCannonType {}
			, FireSelf_Weapon {}
			, FireSelf_ROF {}
			, FireSelf_Immediately { false }

			, SHP_PipsPAL { nullptr }
			, SHP_PipsSHP { nullptr }
			, SHP_PipBrdPAL { nullptr }
			, SHP_PipBrdSHP { nullptr }
			, HealthBar_Pips {}
			, HealthBar_Pips_DrawOffset {}
			, HealthBar_PipsLength {}
			, HealthBar_PipsSHP { "" }
			, HealthBar_PipsPAL { "" }
			, HealthBar_PipBrd {}
			, HealthBar_PipBrdSHP { "" }
			, HealthBar_PipBrdPAL { "" }
			, HealthBar_PipBrdOffset { }
			, HealthBar_XOffset { 0 }
			, GroupID_Offset { {0,0} }
			, SelfHealPips_Offset { {0,0} }
			, UseCustomHealthBar { false }
			, UseUnitHealthBar { false }
			, UseNewHealthBar { false }
			, HealthBar_PictureSHP { "" }
			, HealthBar_PicturePAL { "" }
			, HealthBar_PictureTransparency { 0 }
			, SHP_PictureSHP {}
			, SHP_PicturePAL {}

			, GScreenAnimType {}
			, MovePassengerToSpawn { false }
			, SilentPassenger { false }
			, Spawner_SameLoseTarget { false }
			, DeterminedByRange { false }
			, DeterminedByRange_ExtraRange { 0 }
			, DeterminedByRange_MainWeapon { -1 }
			, BuildLimit_Group_Types {}
			, BuildLimit_Group_Any { false }
			, BuildLimit_Group_Limits {}
			, VehicleImmuneToMindControl { false }
			, Convert_Deploy {}
			, Convert_DeployAnim {}
			, Gattling_SelectWeaponByVersus { false }
			, IsExtendGattling { false }
			, Gattling_Cycle { false }
			, Gattling_Charge { false }
			, Weapons {}
			, Stages {}
			, VeteranStages {}
			, EliteStages {}
			, WeaponFLHs {}
			, VeteranWeaponFLHs {}
			, EliteWeaponFLHs {}
			, OccupyWeapon {}
			, VeteranOccupyWeapon {}
			, EliteOccupyWeapon {}
			, JJConvert_Unload {}

			, IronCurtain_KeptOnDeploy {}
			, IronCurtain_Affect {}
			, IronCuratin_KillWarhead {}

			, BuildLimit_As {}

			, AttackedWeapon {}
			, AttackedWeapon_FireToAttacker {}
			, AttackedWeapon_ROF {}
			, AttackedWeapon_IgnoreROF {}
			, AttackedWeapon_IgnoreRange {}
			, AttackedWeapon_Range {}
			, AttackedWeapon_ResponseWarhead {}
			, AttackedWeapon_NoResponseWarhead {}
			, AttackedWeapon_ResponseZeroDamage {}
			, AttackedWeapon_ResponseHouse {}
			, AttackedWeapon_ActiveMaxHealth {}
			, AttackedWeapon_ActiveMinHealth {}
			, AttackedWeapon_FLHs {}

			, WeaponInTransport {}
			
			, ProtectPassengers { false }
			, ProtectPassengers_Clear { false }
			, ProtectPassengers_Release { false }
			, ProtectPassengers_Damage { false }

			, Dodge_Houses { AffectedHouse::All }
			, Dodge_MaxHealthPercent { 1.0 }
			, Dodge_MinHealthPercent { 0.0 }
			, Dodge_Chance { 0.0 }
			, Dodge_Anim {}
			, Dodge_OnlyDodgePositiveDamage { true }

			, MoveDamage { 0 }
			, MoveDamage_Delay { 0 }
			, MoveDamage_Warhead {}
			, MoveDamage_Anim {}
			, StopDamage { 0 }
			, StopDamage_Delay { 0 }
			, StopDamage_Warhead {}
			, StopDamage_Anim {}

			, WeaponRangeShare_Technos {}
			, WeaponRangeShare_Range { 0.0 }
			, WeaponRangeShare_ForceAttack { false }
			, WeaponRangeShare_UseWeapon { 0 }

			, AllowMinHealth {}
			, Turrets {}
			, AllowMaxDamage { { INT_MAX, -INT_MAX } }
			, AllowMinDamage { { -INT_MAX, INT_MAX } }
			, ImmuneToAbsorb { false }

			, TeamAffect { false }
			, TeamAffect_Range { 0.0 }
			, TeamAffect_Technos {}
			, TeamAffect_Houses { AffectedHouse::Owner }
			, TeamAffect_Number { 0 }
			, TeamAffect_Weapon {}
			, TeamAffect_ROF {}
			, TeamAffect_LoseEfficacyWeapon {}
			, TeamAffect_LoseEfficacyROF {}
			, TeamAffect_Anim {}
			, TeamAffect_ShareDamage { false }
			, TeamAffect_MaxNumber { 0 }

			, AttachEffects {}
			, AttachEffects_Duration {}
			, AttachEffects_Delay {}
			, AttachEffects_Immune {}
			, AttachEffects_OnlyAccept {}

			, PoweredUnitBy {}
			, PoweredUnitBy_Any { true }
			, PoweredUnitBy_Sparkles {}
			, PoweredUnitBy_ParticleSystem {}
			, PoweredUnitBy_ParticleSystemXOffset { { -128, 128 } }
			, PoweredUnitBy_ParticleSystemYOffset { { -128, 128 } }
			, PoweredUnitBy_ParticleSystemSpawnDelay { 30 }

			, PassengerHeal_Rate { 0 }
			, PassengerHeal_HealAll { false }
			, PassengerHeal_Amount { 0 }
			, PassengerHeal_Houses { AffectedHouse::Team }
			, PassengerHeal_Sound {}
			, PassengerHeal_Anim {}

			, Temperature {}
			, Temperature_HeatUpFrame {}
			, Temperature_HeatUpAmount {}
			, Temperature_HeatUpDelay {}
			, Temperatrue_Disable {}

			, PassengerProduct { false }
			, PassengerProduct_Type {}
			, PassengerProduct_Rate { -1 }
			, PassengerProduct_Amount { 1 }
			, PassengerProduct_RandomPick { false }

			, UseConvert { false }
			, Convert_Passengers {}
			, Convert_Types {}

			, VeteranAnim { nullptr }
			, EliteAnim { nullptr }

			, TurretROT {}
			, InitialPayload_Types {}
			, InitialPayload_Nums {}

		{ }

		virtual ~ExtData() = default;
		virtual void LoadFromINIFile(CCINIClass* pINI) override;
		virtual void Initialize() override;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

		void ApplyTurretOffset(Matrix3D* mtx, double factor = 1.0);
		bool IsCountedAsHarvester();
		void ReadWeapons();

		// Ares 0.A
		const char* GetSelectionGroupID() const;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<TechnoTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static void ApplyTurretOffset(TechnoTypeClass* pType, Matrix3D* mtx, double factor = 1.0);
	static void GetBurstFLHs(TechnoTypeClass* pThis, INI_EX& exArtINI, const char* pArtSection, std::vector<DynamicVectorClass<CoordStruct>>& nFLH, std::vector<DynamicVectorClass<CoordStruct>>& nVFlh, std::vector<DynamicVectorClass<CoordStruct>>& nEFlh, const char* pPrefixTag);
	static TechnoTypeClass* GetTechnoType(ObjectTypeClass* pType);
	static void GetWeaponCounts(TechnoTypeClass* pThis, INI_EX& exINI, const char* pSection, std::vector<DynamicVectorClass<WeaponTypeClass*>>& n, std::vector<DynamicVectorClass<WeaponTypeClass*>>& nV, std::vector<DynamicVectorClass<WeaponTypeClass*>>& nE);
	static void GetWeaponStages(TechnoTypeClass* pThis, INI_EX& exINI, const char* pSection, std::vector<DynamicVectorClass<int>>& nStage, std::vector<DynamicVectorClass<int>>& nStageV, std::vector<DynamicVectorClass<int>>& nStageE);
	static void GetWeaponFLHs(TechnoTypeClass* pThis, INI_EX& exArtINI, const char* pArtSection, std::vector<DynamicVectorClass<CoordStruct>>& nFLH, std::vector<DynamicVectorClass<CoordStruct>>& nVFlh, std::vector<DynamicVectorClass<CoordStruct>>& nEFlh);
	static void GetIFVTurrets(TechnoTypeClass* pThis, INI_EX& exINI, const char* pSection, std::vector<DynamicVectorClass<int>>& nturret);
	static std::vector<WeaponTypeClass*> GetAllWeapons(TechnoTypeClass* pThis);

	// Ares 0.A
	static const char* GetSelectionGroupID(ObjectTypeClass* pType);
	static bool HasSelectionGroupID(ObjectTypeClass* pType, const char* pID);
};
