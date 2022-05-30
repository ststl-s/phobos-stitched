#pragma once
#include <TechnoTypeClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

#include <New/Type/ShieldTypeClass.h>
#include <New/Type/LaserTrailTypeClass.h>
#include <New/Entity/GiftBoxClass.h>
#include <New/Type/AttachmentTypeClass.h>
#include <New/Type/DigitalDisplayTypeClass.h>
#include <New/Type/FireScriptTypeClass.h>
#include <New/Type/IonCannonTypeClass.h>
#include <New/Type/GScreenAnimTypeClass.h>

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
		Valueable<int> InhibitorRange;
		Valueable<Leptons> MindControlRangeLimit;
		Valueable<bool> Interceptor;
		Nullable<bool> Interceptor_Rookie;
		Nullable<bool> Interceptor_Veteran;
		Nullable<bool> Interceptor_Elite;
		Valueable<Leptons> Interceptor_GuardRange;
		Valueable<Leptons> Interceptor_MinimumGuardRange;
		Valueable<Leptons> Interceptor_EliteGuardRange;
		Valueable<Leptons> Interceptor_EliteMinimumGuardRange;
		Valueable<int> Interceptor_Success;
		Valueable<int> Interceptor_RookieSuccess;
		Valueable<int> Interceptor_VeteranSuccess;
		Valueable<int> Interceptor_EliteSuccess;
		Valueable<CoordStruct> TurretOffset;
		Valueable<bool> Powered_KillSpawns;
		Valueable<bool> Spawner_LimitRange;
		Valueable<int> Spawner_ExtraLimitRange;
		Nullable<int> Spawner_DelayFrames;
		Nullable<bool> Harvester_Counted;
		Valueable<bool> Promote_IncludeSpawns;
		Valueable<bool> ImmuneToCrit;
		Valueable<bool> MultiMindControl_ReleaseVictim;
		Valueable<int> CameoPriority;
		Valueable<bool> NoManualMove;
		Nullable<int> InitialStrength;
		Valueable<bool> PassengerDeletion_Soylent;
		Valueable<bool> PassengerDeletion_SoylentFriendlies;
		Valueable<int> PassengerDeletion_Rate;
		NullableIdx<VocClass> PassengerDeletion_ReportSound;
		Valueable<bool> PassengerDeletion_Rate_SizeMultiply;
		Nullable<AnimTypeClass*> PassengerDeletion_Anim;
		Valueable<bool> Death_NoAmmo;
		Valueable<int> Death_Countdown;
		Valueable<bool> Death_Peaceful;
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
		std::vector<DynamicVectorClass<CoordStruct>> EliteWeaponBurstFLHs;

		Valueable<bool> DestroyAnim_Random;
		Valueable<bool> NotHuman_RandomDeathSequence;

		Nullable<InfantryTypeClass*> DefaultDisguise;

		Nullable<int> OpenTopped_RangeBonus;
		Nullable<float> OpenTopped_DamageMultiplier;
		Nullable<int> OpenTopped_WarpDistance;

		struct AttachmentDataEntry
		{
			ValueableIdx<AttachmentTypeClass> Type;
			ValueableVector<TechnoTypeClass*> TechnoType;
			Valueable<CoordStruct> FLH;
			Valueable<bool> IsOnTurret;

			AttachmentDataEntry() = default;

			AttachmentDataEntry(int AttachmentTypeIdx, TechnoTypeClass* TechnoType, CoordStruct FLH, bool IsOnTurret)
				:Type(AttachmentTypeIdx), FLH(FLH), IsOnTurret(IsOnTurret)
			{
				this->TechnoType.push_back(TechnoType);
			}

			~AttachmentDataEntry() = default;

			bool Load(PhobosStreamReader& stm, bool registerForChange);
			bool Save(PhobosStreamWriter& stm) const;

		private:
			template <typename T>
			bool Serialize(T& stm);
		};

		ValueableVector<AttachmentDataEntry> AttachmentData;

		Valueable<bool> AutoFire;
		Valueable<bool> AutoFire_TargetSelf;

		Valueable<bool> NoSecondaryWeaponFallback;

		Valueable<int> NoAmmoWeapon;
		Valueable<int> NoAmmoAmount;

		Nullable<bool> JumpjetAllowLayerDeviation;

		Valueable<bool> DeployingAnim_AllowAnyDirection;
		Valueable<bool> DeployingAnim_KeepUnitVisible;
		Valueable<bool> DeployingAnim_ReverseForUndeploy;
		Valueable<bool> DeployingAnim_UseUnitDrawer;

		Valueable<int> ForceWeapon_Naval_Decloaked;

		Valueable<bool> Ammo_Shared;
		Valueable<int> Ammo_Shared_Group;
		Nullable<bool> JumpjetTurnToTarget;

		Valueable<bool> Passengers_ChangeOwnerWithTransport;

		Nullable<SelfHealGainType> SelfHealGainType;

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
		Valueable<CSFText> EnemyUIName;
		GiftBoxData GiftBoxData;

		SHPStruct* SHP_SelectBrdSHP;
		ConvertClass* SHP_SelectBrdPAL;

		Nullable<bool> UseCustomSelectBrd;
		PhobosFixedString<32U> SelectBrd_SHP;
		PhobosFixedString<32U> SelectBrd_PAL;
		Nullable<Vector3D<int>> SelectBrd_Frame;
		Nullable<Vector2D<int>> SelectBrd_DrawOffset;
		Nullable<int> SelectBrd_TranslucentLevel;
		Nullable<bool> SelectBrd_ShowEnemy;

		Valueable<bool> CanRepairCyborgLegs;

		Nullable<CoordStruct> PronePrimaryFireFLH;
		Nullable<CoordStruct> ProneSecondaryFireFLH;
		Nullable<CoordStruct> DeployedPrimaryFireFLH;
		Nullable<CoordStruct> DeployedSecondaryFireFLH;
		std::vector<DynamicVectorClass<CoordStruct>> CrouchedWeaponBurstFLHs;
		std::vector<DynamicVectorClass<CoordStruct>> EliteCrouchedWeaponBurstFLHs;
		std::vector<DynamicVectorClass<CoordStruct>> DeployedWeaponBurstFLHs;
		std::vector<DynamicVectorClass<CoordStruct>> EliteDeployedWeaponBurstFLHs;

		NullableVector<int> Overload_Count;
		NullableVector<int> Overload_Damage;
		NullableVector<int> Overload_Frames;
		NullableIdx<VocClass> Overload_DeathSound;
		Nullable<ParticleSystemTypeClass*> Overload_ParticleSys;
		Nullable<int> Overload_ParticleSysCount;
		Valueable<bool> Draw_MindControlLink;

		Nullable<DigitalDisplayTypeClass*> DigitalDisplayType;
		Nullable<DigitalDisplayTypeClass*> DigitalDisplayType_Shield;

		ValueableIdxVector<TechnoTypeClass> RandomProduct;

		Valueable<bool> HugeHP_Show;
		Valueable<int> HugeHP_Priority;

		PhobosFixedString<0x20> Script_Fire;
		Valueable<bool> Script_Fire_SelfCenter;

		ValueableVector<WeaponTypeClass*> FireSelf_Weapon;
		ValueableVector<int> FireSelf_ROF;
		ValueableVector<WeaponTypeClass*> FireSelf_Weapon_GreenHeath;
		ValueableVector<int> FireSelf_ROF_GreenHeath;
		ValueableVector<WeaponTypeClass*> FireSelf_Weapon_YellowHeath;
		ValueableVector<int> FireSelf_ROF_YellowHeath;
		ValueableVector<WeaponTypeClass*> FireSelf_Weapon_RedHeath;
		ValueableVector<int> FireSelf_ROF_RedHeath;

		FireScriptTypeClass* FireScriptType;

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

		Nullable<GScreenAnimTypeClass*> GScreenAnimType;

		Valueable<bool> MovePassengerToSpawn;
		Valueable<bool> SilentPassenger;
		Valueable<bool> Spawner_SameLoseTarget;

		Valueable<bool> DeterminedByRange;
		Valueable<int> DeterminedByRange_ExtraRange;

		ValueableIdxVector<TechnoTypeClass> BuildLimit_Group_Types;
		Valueable<bool> BuildLimit_Group_Any;
		ValueableVector<int> BuildLimit_Group_Limits;

		Valueable<bool> VehicleImmuneToMindControl;

		ExtData(TechnoTypeClass* OwnerObject) : Extension<TechnoTypeClass>(OwnerObject)
			, HealthBar_Hide { false }
			, UIDescription {}
			, LowSelectionPriority { false }
			, GroupAs { NONE_STR }
			, RadarJamRadius { 0 }
			, InhibitorRange { 0 }
			, MindControlRangeLimit {}
			, Interceptor { false }
			, Interceptor_Rookie {}
			, Interceptor_Veteran {}
			, Interceptor_Elite {}
			, Interceptor_GuardRange {}
			, Interceptor_MinimumGuardRange {}
			, Interceptor_EliteGuardRange {}
			, Interceptor_EliteMinimumGuardRange {}
			, Interceptor_Success { 100 }
			, Interceptor_RookieSuccess { -1 }
			, Interceptor_VeteranSuccess { -1 }
			, Interceptor_EliteSuccess { -1 }
			, TurretOffset { { 0, 0, 0 } }
			, Powered_KillSpawns { false }
			, Spawner_LimitRange { false }
			, Spawner_ExtraLimitRange { 0 }
			, Spawner_DelayFrames {}
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
			, PassengerDeletion_Soylent { false }
			, PassengerDeletion_SoylentFriendlies { false }
			, PassengerDeletion_Rate { 0 }
			, PassengerDeletion_ReportSound {}
			, PassengerDeletion_Rate_SizeMultiply { true }
			, PassengerDeletion_Anim {}
			, DefaultDisguise {}
			, OpenTopped_RangeBonus {}
			, OpenTopped_DamageMultiplier {}
			, OpenTopped_WarpDistance {}
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
			, EnemyUIName {}
			, Death_NoAmmo { false }
			, Death_Countdown { 0 }
			, Death_Peaceful { false }
			, ForceWeapon_Naval_Decloaked { -1 }
			, Ammo_Shared { false }
			, Ammo_Shared_Group { -1 }
			, Passengers_ChangeOwnerWithTransport { false }
			, SHP_SelectBrdSHP { nullptr }
			, SHP_SelectBrdPAL { nullptr }
			, UseCustomSelectBrd {}
			, SelectBrd_SHP {}
			, SelectBrd_PAL {}
			, SelectBrd_Frame { {-1,-1,-1} }
			, SelectBrd_DrawOffset {}
			, SelectBrd_TranslucentLevel {}
			, SelectBrd_ShowEnemy {}
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
			, DigitalDisplayType {}
			, DigitalDisplayType_Shield {}
			, HugeHP_Show { false }
			, HugeHP_Priority { -1 }
			, IonCannonType {}
			, FireSelf_Weapon {}
			, FireSelf_ROF {}
			, FireSelf_Weapon_GreenHeath {}
			, FireSelf_ROF_GreenHeath {}
			, FireSelf_Weapon_YellowHeath {}
			, FireSelf_ROF_YellowHeath {}
			, FireSelf_Weapon_RedHeath {}
			, FireSelf_ROF_RedHeath {}
			, Script_Fire{ "" }
			, Script_Fire_SelfCenter{ false }
			, FireScriptType{ nullptr }
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
			, HealthBar_XOffset { 0 }
			, GroupID_Offset { {0,0} }
			, SelfHealPips_Offset { {0,0} }
			, UseCustomHealthBar { false }
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
			, BuildLimit_Group_Types {}
			, BuildLimit_Group_Any { false }
			, BuildLimit_Group_Limits {}
			, VehicleImmuneToMindControl { false }
		{ }

		virtual ~ExtData() = default;
		virtual void LoadFromINIFile(CCINIClass* pINI) override;
		virtual void Initialize() override;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

		void ApplyTurretOffset(Matrix3D* mtx, double factor = 1.0);
		bool IsCountedAsHarvester();

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
	static void GetBurstFLHs(TechnoTypeClass* pThis, INI_EX& exArtINI, const char* pArtSection, std::vector<DynamicVectorClass<CoordStruct>>& nFLH, std::vector<DynamicVectorClass<CoordStruct>>& nEFlh, const char* pPrefixTag);

	// Ares 0.A
	static const char* GetSelectionGroupID(ObjectTypeClass* pType);
	static bool HasSelectionGroupID(ObjectTypeClass* pType, const char* pID);
};