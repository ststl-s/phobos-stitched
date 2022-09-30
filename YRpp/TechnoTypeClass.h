/*
	TechnoTypes are initialized by INI files.
*/

#pragma once

#include <ObjectTypeClass.h>
#include <RulesClass.h>
#include <WeaponTypeClass.h>

//forward declarations
class AircraftTypeClass;
class AnimTypeClass;
class BuildingTypeClass;
class InfantryTypeClass;
class ParticleSystemTypeClass;
class VoxelAnimTypeClass;
class UnitTypeClass;

struct AbilitiesStruct
{
	bool FASTER; //0x00
	bool STRONGER; //0x01
	bool FIREPOWER; //0x02
	bool SCATTER; //0x03
	bool ROF; //0x04
	bool SIGHT; //0x05
	bool CLOAK; //0x06
	bool TIBERIUM_PROOF; //0x07
	bool VEIN_PROOF; //0x08
	bool SELF_HEAL; //0x09
	bool EXPLODES; //0x0A
	bool RADAR_INVISIBLE; //0x0B
	bool SENSORS; //0x0C
	bool FEARLESS; //0x0D
	bool C4; //0x0E
	bool TIBERIUM_HEAL; //0x0F
	bool GUARD_AREA; //0x10
	bool CRUSHER; //0x11
};

struct TurretControl
{
	int Travel;
	int CompressFrames;
	int RecoverFrames;
	int HoldFrames;
};

struct WeaponStruct
{
	WeaponTypeClass*  WeaponType;
	CoordStruct       FLH;
	int               BarrelLength;
	int               BarrelThickness;
	bool              TurretLocked;

	WeaponStruct() : WeaponType(nullptr),
		FLH(CoordStruct::Empty),
		BarrelLength(0),
		BarrelThickness(0),
		TurretLocked(false)
	{ }

	WeaponStruct
	(
		WeaponTypeClass* pType,
		const CoordStruct& flh = CoordStruct::Empty,
		int barrelLength = 0,
		int barrelThickness = 0,
		bool turretLocked = false
	) : WeaponType(pType)
		, FLH(flh)
		, BarrelLength(barrelLength)
		, BarrelThickness(barrelThickness)
		, TurretLocked(turretLocked)
	{ }

	bool operator == (const WeaponStruct& pWeap) const
		{ return false; }
};

class NOVTABLE TechnoTypeClass : public ObjectTypeClass
{
public:
	static constexpr constant_ptr<DynamicVectorClass<TechnoTypeClass*>, 0xA8EB00u> const Array{};

	static __declspec(noinline) TechnoTypeClass* __fastcall Find(const char* pID)
	{
		for(auto pItem : *Array) {
			if(!_strcmpi(pItem->ID, pID)) {
				return pItem;
			}
		}
		return nullptr;
	}

	static __declspec(noinline) int __fastcall FindIndex(const char* pID)
	{
		for(int i = 0; i < Array->Count; ++i) {
			if(!_strcmpi(Array->Items[i]->get_ID(), pID)) {
				return i;
			}
		}
		return -1;
	}

	static auto const MaxWeapons = 18;

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x7162F0);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override JMP_STD(0x616DC0);
	virtual HRESULT __stdcall GetSizeMax(ULARGE_INTEGER* pcbSize) override JMP_STD(0x7170A0);

	//Destructor
	virtual ~TechnoTypeClass() override JMP_THIS(0x7179A0);

	//AbstractClass
	virtual void ComputeCRC(CRCEngine& crc) const override JMP_THIS(0x7171A0);

	//AbstractTypeClass
	virtual bool LoadFromINI(CCINIClass* pINI) override JMP_THIS(0x712170);

	//ObjectTypeClass
	virtual DWORD GetOwners() const override JMP_THIS(0x711EC0);
	virtual int GetPipMax() const override JMP_THIS(0x716290);
	virtual int GetActualCost(HouseClass* pHouse) const override JMP_THIS(0x711F00);
	virtual int GetBuildSpeed() const override JMP_THIS(0x711EE0);
	virtual SHPStruct* GetCameo() const override JMP_THIS(0x712040);

	//TechnoTypeClass
	virtual bool vt_entry_A0() { return true; }
	virtual bool CanAttackMove() const { return this->Weapon[0].WeaponType && !this->PreventAttackMove; }//JMP_THIS(0x711E90)
	virtual bool CanCreateHere(const CellStruct& mapCoords, HouseClass* pOwner) const JMP_THIS(0x716150);
	virtual int GetCost() const { return this->Cost; }//JMP_THIS(0x711EB0)
	virtual int GetRepairStepCost() const JMP_THIS(0x7120D0);
	virtual int GetRepairStep() const JMP_THIS(0x712120);
	virtual int GetRefund(HouseClass* pHouse, bool bUnk) const JMP_THIS(0x711F60);
	virtual int GetFlightLevel() const JMP_THIS(0x717800);

	// non-virtual
	static TechnoTypeClass* __fastcall GetByTypeAndIndex(AbstractType abs, int index)
		{ JMP_STD(0x48DCD0); }

	static int sub_717840()
		{ JMP_STD(0x717840); }
	
	bool HasMultipleTurrets() const
		{ return this->TurretCount > 0; }

	//I don't know what that is
	void SetTurretWeapon(int index, int unknown) //JMP_THIS(0x717890)
		{ this->TurretWeapon[index] = unknown; }

	int GetTurretWeapon(int index)
		{ return this->TurretWeapon[index]; }

	bool sub_712130() const
		{ JMP_THIS(0x712130); }

	DynamicVectorClass<ColorScheme*>* sub_717820()
		{ JMP_THIS(0x717820); }

	CoordStruct* GetParticleSysOffset(CoordStruct* pBuffer) const
		{ JMP_THIS(0x7178C0); }

	CoordStruct GetParticleSysOffset() const
	{
		CoordStruct buffer;
		GetParticleSysOffset(&buffer);
		return buffer;
	}

	bool InOwners(DWORD const bitHouseType) const
	{
		return 0u != (this->GetOwners() & bitHouseType);
	}

	bool InRequiredHouses(DWORD const bitHouseType) const
	{
		auto const test = this->RequiredHouses;
		if(static_cast<int>(test) == -1) {
			return true;
		}
		return 0u != (test & bitHouseType);
	}

	bool InForbiddenHouses(DWORD const bitHouseType) const
	{
		auto const test = this->ForbiddenHouses;
		if(static_cast<int>(test) == -1) {
			return false;
		}
		return 0u != (test & bitHouseType);
	}

	// weapon related
	WeaponStruct& GetWeapon(int index)
	{
		JMP_THIS(0x7177C0);
		//return this->Weapon[index];
	}

	WeaponStruct& GetEliteWeapon(int index)
	{
		JMP_THIS(0x7177E0);
		//return this->EliteWeapon[index];
	}

	WeaponStruct& GetWeapon(size_t const index, bool const elite)
	{
		return elite ? this->EliteWeapon[index] : this->Weapon[index];
	}

	WeaponStruct const& GetWeapon(size_t const index, bool const elite) const
	{
		return elite ? this->EliteWeapon[index] : this->Weapon[index];
	}

	//Constructor
	TechnoTypeClass(const char* pID, SpeedType speedtype) noexcept
		: TechnoTypeClass(noinit_t())
	{ JMP_THIS(0x710AF0); }

	TechnoTypeClass(IStream* pStm) noexcept
		: TechnoTypeClass(noinit_t())
	{ JMP_THIS(0x711840); }

protected:
	explicit __forceinline TechnoTypeClass(noinit_t) noexcept
		: ObjectTypeClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	int             WalkRate;
	int             IdleRate;
	AbilitiesStruct VeteranAbilities;
	AbilitiesStruct EliteAbilities;
	double          SpecialThreatValue;
	double          MyEffectivenessCoefficient;
	double          TargetEffectivenessCoefficient;
	double          TargetSpecialThreatCoefficient;
	double          TargetStrengthCoefficient;
	double          TargetDistanceCoefficient;
	double          ThreatAvoidanceCoefficient;
	int             SlowdownDistance;
	PROTECTED_PROPERTY(BYTE, align_2FC[4]);
	double          unknown_double_300;
	double          AccelerationFactor;
	int             CloakingSpeed;
	TypeList<VoxelAnimTypeClass*> DebrisTypes;
	TypeList<int> DebrisMaximums;
	_GUID           Locomotor;
	PROTECTED_PROPERTY(BYTE, align_35C[4]);
	double          VoxelScaleX;
	double          VoxelScaleY;
	double          Weight;
	double          PhysicalSize;
	double          Size;
	double          SizeLimit;
	bool            HoverAttack;
	PROTECTED_PROPERTY(BYTE, align_391[3]);
	int             VHPScan;
	int             unknown_int_398;
	PROTECTED_PROPERTY(BYTE, align_39C[4]);
	double          RollAngle;
	double          PitchSpeed;
	double          PitchAngle;
	int             BuildLimit;
	Category        Category;
	DWORD           unknown_3C0;
	PROTECTED_PROPERTY(BYTE, align_3C4[4]);
	double          DeployTime;
	int             FireAngle;
	PipScale        PipScale;
	bool            PipsDrawForAll;
	PROTECTED_PROPERTY(BYTE, align_3D9[3]);
	int             LeptonMindControlOffset;
	int             PixelSelectionBracketDelta;
	int             PipWrap;
	TypeList<BuildingTypeClass*> Dock;
	BuildingTypeClass* DeploysInto;
	UnitTypeClass*  UndeploysInto;
	UnitTypeClass*  PowersUnit;
	bool            PoweredUnit;
	PROTECTED_PROPERTY(BYTE, align_411[3]);
	TypeList<int> VoiceSelect;
	TypeList<int> VoiceSelectEnslaved;
	TypeList<int> VoiceSelectDeactivated;
	TypeList<int> VoiceMove;
	TypeList<int> VoiceAttack;
	TypeList<int> VoiceSpecialAttack;
	TypeList<int> VoiceDie;
	TypeList<int> VoiceFeedback;
	TypeList<int> MoveSound;
	TypeList<int> DieSound;
	int             AuxSound1;
	int             AuxSound2;
	int             CreateSound;
	int             DamageSound;
	int             ImpactWaterSound;
	int             ImpactLandSound;
	int             CrashingSound;
	int             SinkingSound;
	int             VoiceFalling;
	int             VoiceCrashing;
	int             VoiceSinking;
	int             VoiceEnter;
	int             VoiceCapture;
	int             TurretRotateSound;
	int             EnterTransportSound;
	int             LeaveTransportSound;
	int             DeploySound;
	int             UndeploySound;
	int             ChronoInSound;
	int             ChronoOutSound;
	int             VoiceHarvest;
	int             VoicePrimaryWeaponAttack;
	int             VoicePrimaryEliteWeaponAttack;
	int             VoiceSecondaryWeaponAttack;
	int             VoiceSecondaryEliteWeaponAttack;
	int             VoiceDeploy;
	int             VoiceUndeploy;
	int             EnterGrinderSound;
	int             LeaveGrinderSound;
	int             EnterBioReactorSound;
	int             LeaveBioReactorSound;
	int             ActivateSound;
	int             DeactivateSound;
	int             MindClearedSound;
	MovementZone    MovementZone;
	int             GuardRange;
	int             MinDebris;
	int             MaxDebris;
	TypeList<AnimTypeClass*> DebrisAnims;
	int             Passengers;
	bool            OpenTopped;
	PROTECTED_PROPERTY(BYTE, align_5E5[3]);
	int             Sight;
	bool            ResourceGatherer;
	bool            ResourceDestination;
	bool            RevealToAll;
	bool            Drainable;
	int             SensorsSight;
	int             DetectDisguiseRange;
	int             BombSight;
	int             LeadershipRating;
	NavalTargetingType NavalTargeting;
	LandTargetingType LandTargeting;
	float           BuildTimeMultiplier;
	int             MindControlRingOffset;
	int             Cost;
	int             Soylent;
	int             FlightLevel;
	int             AirstrikeTeam;
	int             EliteAirstrikeTeam;
	AircraftTypeClass* AirstrikeTeamType;
	AircraftTypeClass* EliteAirstrikeTeamType;
	int             AirstrikeRechargeTime;
	int             EliteAirstrikeRechargeTime;
	int             TechLevel;
	TypeList<int> Prerequisite;
	TypeList<int> PrerequisiteOverride;
	int             ThreatPosed;
	int             Points;
	int             Speed;
	SpeedType       SpeedType;
	int             InitialAmmo;
	int             Ammo;
	int             IFVMode;
	int             AirRangeBonus;
	bool            BerserkFriendly;
	bool            SprayAttack;
	bool            Pushy;
	bool            Natural;
	bool            Unnatural;
	bool            CloseRange;
	PROTECTED_PROPERTY(BYTE, align_696[2]);
	int             Reload;
	int             EmptyReload;
	int             ReloadIncrement;
	int             RadialFireSegments;
	int             DeployFireWeapon;
	bool            DeployFire;
	bool            DeployToLand;
	bool            MobileFire;
	bool            OpportunityFire;
	bool            DistributedFire;
	bool            DamageReducesReadiness;
	PROTECTED_PROPERTY(BYTE, align_6B2[2]);
	int             ReadinessReductionMultiplier;
	UnitTypeClass*  UnloadingClass;
	AnimTypeClass*  DeployingAnim;
	bool            AttackFriendlies;
	bool            AttackCursorOnFriendlies;
	PROTECTED_PROPERTY(BYTE, align_6C2[2]);
	int             UndeployDelay;
	bool            PreventAttackMove;
	PROTECTED_PROPERTY(BYTE, align_6C9[3]);
	DWORD           OwnerFlags;
	int             AIBasePlanningSide;
	bool            StupidHunt;
	bool            AllowedToStartInMultiplayer;
	char            CameoFile[0x19];
	PROTECTED_PROPERTY(BYTE,  align_6EF);
	SHPStruct*      Cameo;
	bool            CameoAllocated;
	char            AltCameoFile[0x19];
	PROTECTED_PROPERTY(BYTE,  align_70E[2]);
	SHPStruct*      AltCameo;
	bool            AltCameoAllocated;
	PROTECTED_PROPERTY(BYTE, align_715[3]);
	int             RotCount;
	int             ROT;
	int             TurretOffset;
	bool            CanBeHidden;
	PROTECTED_PROPERTY(BYTE, align_725[3]);
	int             Points2; //twice
	TypeList<AnimTypeClass*> Explosion;
	TypeList<AnimTypeClass*> DestroyAnim;
	ParticleSystemTypeClass* NaturalParticleSystem;
	CoordStruct NaturalParticleSystemLocation;
	ParticleSystemTypeClass* RefinerySmokeParticleSystem;
	TypeList<ParticleSystemTypeClass*> DamageParticleSystems;
	TypeList<ParticleSystemTypeClass*> DestroyParticleSystems;
	CoordStruct DamageSmokeOffset;
	bool            DamSmkOffScrnRel;
	PROTECTED_PROPERTY(BYTE, align_7BD[3]);
	CoordStruct DestroySmokeOffset;
	CoordStruct RefinerySmokeOffsetOne;
	CoordStruct RefinerySmokeOffsetTwo;
	CoordStruct RefinerySmokeOffsetThree;
	CoordStruct RefinerySmokeOffsetFour;
	int             ShadowIndex;
	int             Storage;
	bool            TurretNotExportedOnGround;
	bool            Gunner;
	bool            HasTurretTooltips;
	PROTECTED_PROPERTY(BYTE, align_807);
	int             TurretCount;
	int             WeaponCount;
	bool            IsChargeTurret;
	PROTECTED_PROPERTY(BYTE, align_811[3]);
	int             TurretWeapon[0x21];
	WeaponStruct	Weapon[MaxWeapons];
	bool            ClearAllWeapons;
	PROTECTED_PROPERTY(BYTE, align_A91[3]);
	WeaponStruct	EliteWeapon[MaxWeapons];
	bool            TypeImmune;
	bool            MoveToShroud;
	bool            Trainable;
	bool            DamageSparks; //enabled for Cyborg InfantryTypes
	bool            TargetLaser;
	bool            ImmuneToVeins;
	bool            TiberiumHeal;
	bool            CloakStop;
	bool            IsTrain;
	bool            IsDropship;
	bool            ToProtect;
	bool            Disableable;
	bool            Unbuildable; //always false, if true it cannot be built from sidebar
	bool            DoubleOwned;
	bool            Invisible;
	bool            RadarVisible;
	bool            HasPrimary; //not loaded from the INIs
	bool            Sensors;
	bool            Nominal;
	bool            DontScore;
	bool            DamageSelf;
	bool            Turret;
	bool            TurretRecoil;
	PROTECTED_PROPERTY(BYTE, align_CA3);
	TurretControl   TurretAnimData;
	bool            unknown_bool_CB4; //always false?
	PROTECTED_PROPERTY(BYTE, align_CB5[3]);
	TurretControl   BarrelAnimData;
	bool            unknown_bool_CC8; //always false?
	PROTECTED_PROPERTY(BYTE, align_CC9[3]);
	bool            Repairable;
	bool            Crewed;
	bool            Naval;
	bool            Remapable;
	bool            Cloakable;
	bool            GapGenerator;
	char            GapRadiusInCells;
	char            SuperGapRadiusInCells;
	bool            Teleporter;
	bool            IsGattling;
	PROTECTED_PROPERTY(BYTE, align_CD6[2]);
	int             WeaponStages;
	int WeaponStage [6];
	int EliteStage [6];
	int             RateUp;
	int             RateDown;
	bool            SelfHealing;
	bool            Explodes;
	PROTECTED_PROPERTY(BYTE, align_D16[2]);
	WeaponTypeClass* DeathWeapon;
	float           DeathWeaponDamageModifier;
	bool            NoAutoFire;
	bool            TurretSpins;
	bool            TiltCrashJumpjet;
	bool            Normalized;
	bool            ManualReload;
	bool            VisibleLoad;
	bool            LightningRod;
	bool            HunterSeeker;
	bool            Crusher;
	bool            OmniCrusher;
	bool            OmniCrushResistant;
	bool            TiltsWhenCrushes;
	bool            IsSubterranean;
	bool            AutoCrush;
	bool            Bunkerable;
	bool            CanDisguise;
	bool            PermaDisguise;
	bool            DetectDisguise;
	bool            DisguiseWhenStill;
	bool            CanApproachTarget;
	bool            CanRecalcApproachTarget;
	bool            ImmuneToPsionics;
	bool            ImmuneToPsionicWeapons;
	bool            ImmuneToRadiation;
	bool            Parasiteable;
	bool            DefaultToGuardArea;
	bool            Warpable;
	bool            ImmuneToPoison;
	bool            ReselectIfLimboed;
	bool            RejoinTeamIfLimboed;
	bool            Slaved;
	PROTECTED_PROPERTY(BYTE, align_D3F);
	InfantryTypeClass* Enslaves;
	int             SlavesNumber;
	int             SlaveRegenRate;
	int             SlaveReloadRate;
	int             OpenTransportWeapon;
	bool            Spawned;
	PROTECTED_PROPERTY(BYTE, align_D55[3]);
	AircraftTypeClass* Spawns;
	int             SpawnsNumber;
	int             SpawnRegenRate;
	int             SpawnReloadRate;
	bool            MissileSpawn;
	bool            Underwater;
	bool            BalloonHover;
	PROTECTED_PROPERTY(BYTE, align_D6B);
	int             SuppressionThreshold;
	int             JumpjetTurnRate;
	int             JumpjetSpeed;
	float           JumpjetClimb;
	float           JumpjetCrash;
	int             JumpjetHeight;
	float           JumpjetAccel;
	float           JumpjetWobbles;
	bool            JumpjetNoWobbles;
	PROTECTED_PROPERTY(BYTE, align_D8D[3]);
	int             JumpjetDeviation;
	bool            JumpJet;
	bool            Crashable;
	bool            ConsideredAircraft;
	bool            Organic;
	bool            NoShadow;
	bool            CanPassiveAquire;
	bool            CanRetaliate;
	bool            RequiresStolenThirdTech;
	bool            RequiresStolenSovietTech;
	bool            RequiresStolenAlliedTech;
	PROTECTED_PROPERTY(BYTE, align_D9E[2]);
	DWORD           RequiredHouses;
	DWORD           ForbiddenHouses;
	DWORD           SecretHouses;
	bool            UseBuffer;
	PROTECTED_PROPERTY(BYTE, align_DAD[3]);
	CoordStruct SecondSpawnOffset;
	bool            IsSelectableCombatant;
	bool            Accelerates;
	bool            DisableVoxelCache;
	bool            DisableShadowCache;
	int             ZFudgeCliff;
	int             ZFudgeColumn;
	int             ZFudgeTunnel;
	int             ZFudgeBridge;
	char            PaletteFile[0x20];
	DynamicVectorClass<ColorScheme*>*           Palette; //no... idea....
	PROTECTED_PROPERTY(BYTE, align_DF4[4]);
};
