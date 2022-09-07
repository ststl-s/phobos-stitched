#pragma once

#include <TechnoTypeClass.h>
class OverlayTypeClass;

struct BuildingAnimStruct
{
	char Anim[0x10];
	char Damaged[0x10];
	char Garrisoned[0x10];
	Point2D Position;
	int ZAdjust;
	int YSort;
	bool Powered;
	bool PoweredLight;
	bool PoweredEffect;
	bool PoweredSpecial;
};

struct BuildingAnimFrameStruct
{
	DWORD dwUnknown;
	int FrameCount;
	int FrameDuration;
};

class NOVTABLE BuildingTypeClass : public TechnoTypeClass
{
public:
	static const AbstractType AbsID = AbstractType::BuildingType;

	//Array
	ABSTRACTTYPE_ARRAY(BuildingTypeClass, 0xA83C68u);

	//static
	static bool LoadBUILDINZ_SHP() JMP_STD(0x45E924);
	static void LoadSomething_0(DWORD dwUnk) JMP_STD(0x45E970);
	static int LoadSomething_1() JMP_STD(0x465450);
	static int sub_465CC0() JMP_STD(0x465CC0);
	static bool sub_465D30() { return false; }

	//static int __fastcall FindIndex(const char* pID) JMP_STD(0x45E7B0);
	//static BuildingTypeClass* __fastcall FindOrAllocate(const char* pID) JMP_STD(0x4653C0);

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) JMP_STD(0x465380);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) JMP_STD(0x465010);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) JMP_STD(0x465300);

	//Destructor
	virtual ~BuildingTypeClass() JMP_THIS(0x465DC0);

	//AbstractClass
	virtual AbstractType WhatAmI() const { return AbstractType::BuildingType; }
	virtual int Size() const { return 0x1798; }
	virtual void CalculateChecksum(Checksummer& checksum) const JMP_THIS(0x464B30);
	virtual int GetArrayIndex() const { return this->ArrayIndex; }

	//AbstractTypeClass
	virtual bool LoadFromINI(CCINIClass* pINI) JMP_THIS(0x45FE50);

	//ObjectTypeClass
	virtual CoordStruct* vt_entry_6C(CoordStruct* pDest, CoordStruct* pSrc) const JMP_THIS(0x464A70);
	virtual int GetPipMax() const JMP_THIS(0x45ECE0); 
	virtual void vt_entry_78(DWORD dwUnk) const JMP_THIS(0x45EBD0);
	virtual void Dimension2(CoordStruct* pDest) JMP_THIS(0x464AF0);
	virtual bool SpawnAtMapCoords(CellStruct* pMapCoords, HouseClass* pOwner) JMP_THIS(0x45E800);
	virtual int GetActualCost(HouseClass* pHouse) const JMP_THIS(0x45EDD0);
	virtual ObjectClass* CreateObject(HouseClass* pOwner) JMP_THIS(0x45E880);
	virtual CellStruct* GetFoundationData(bool IncludeBib) const JMP_THIS(0x45EC20);
	virtual SHPStruct* GetImage() const JMP_THIS(0x45F040);

	//TechnoTypeClass
	virtual bool vt_entry_A0() { return false; }
	virtual bool CanAttackMove() const { return false; }
	virtual bool CanCreateHere(const CellStruct& mapCoords, HouseClass* pOwner) const JMP_THIS(0x464AC0);
	virtual int GetCost() const JMP_THIS(0x45ED50);

	//BuildingTypeClass
	virtual SHPStruct* LoadBuildup() JMP_THIS(0x465960);

	//non-virtual
	bool IsVehicle() const
		{ JMP_THIS(0x465D40); }

	short GetFoundationWidth() const
		{ JMP_THIS(0x45EC90); }

	short GetFoundationHeight(bool bIncludeBib) const
		{ JMP_THIS(0x45ECA0); }

	int GetDeployFacint() const
		{ return this->DeployFacing; }

	bool CanPlaceHere(CellStruct* cell, HouseClass* owner) const
		{ JMP_THIS(0x464AC0); } //CanCreateHere

	bool IsUndeployable() const
		{ JMP_THIS(0x465D40); }

	void LoadVoxel()
		{ JMP_THIS(0x45FA90); }

	void LoadArt()
		{ JMP_THIS(0x5F9070); }

	bool sub_45EE70(CellStruct* pCell, HouseClass* pHouse)
		{ JMP_THIS(0x45EE70); }

	//is somewhat alike to sub_45F1D0
	bool sub_45F160(DWORD dwUnk1, DWORD dwUnk2)
		{ JMP_THIS(0x45F160); }

	//is somewhat alike to sub_45F160
	bool sub_45F1D0(DWORD dwUnk1, DWORD dwUnk2)
		{ JMP_THIS(0x45F1D0); }

	void sub_45F230(DWORD dwUnk)
		{ JMP_THIS(0x45F230); }

	int sub_465570(DWORD dwUnk)
		{ JMP_THIS(0x465570); }

	void sub_465AF0()
	{
		JMP_THIS(0x465AF0);
		/*if (this->FreeBuildup)
		{
			if (this->Buildup)
			{
				delete this->Buildup;
				this->Buildup = nullptr;
				this->BuildupLoaded = false;
			}
		}*/
	}

	// helpers
	bool HasSuperWeapon(int index) const {
		return (this->SuperWeapon == index || this->SuperWeapon2 == index);
	}

	bool HasSuperWeapon() const {
		return (this->SuperWeapon != -1 || this->SuperWeapon2 != -1);
	}

	bool CanTogglePower() const {
		return this->TogglePower && (this->PowerDrain > 0 || this->Powered);
	}

	BuildingAnimStruct& GetBuildingAnim(BuildingAnimSlot slot) {
		return this->BuildingAnim[static_cast<int>(slot)];
	}

	const BuildingAnimStruct& GetBuildingAnim(BuildingAnimSlot slot) const {
		return this->BuildingAnim[static_cast<int>(slot)];
	}

	//Constructor
	BuildingTypeClass(const char* pID) noexcept
		: BuildingTypeClass(noinit_t())
	{ JMP_THIS(0x45DD90); }

	BuildingTypeClass(IStream* pStm) noexcept
		: BuildingTypeClass(noinit_t())
	{ JMP_THIS(0x45E520); }

protected:
	explicit __forceinline BuildingTypeClass(noinit_t) noexcept
		: TechnoTypeClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	int ArrayIndex;
	CellStruct* FoundationData;
	SHPStruct* Buildup;
	bool BuildupLoaded;
	PROTECTED_PROPERTY(BYTE, align_E05[3]);
	BuildCat BuildCat;
	CoordStruct HalfDamageSmokeLocation1;
	CoordStruct HalfDamageSmokeLocation2;
	PROTECTED_PROPERTY(BYTE, align_E24[4]);
	double GateCloseDelay;
	int LightVisibility;
	int LightIntensity;
	int LightRedTint;
	int LightGreenTint;
	int LightBlueTint;
	Point2D PrimaryFirePixelOffset;
	Point2D SecondaryFirePixelOffset;
	OverlayTypeClass *ToOverlay;
	int ToTile;
	char BuildupFile [0x10];
	int BuildupSound;
	int PackupSound;
	int CreateUnitSound;
	int UnitEnterSound;
	int UnitExitSound;
	int WorkingSound;
	int NotWorkingSound;
	char PowersUpBuilding [0x18];
	UnitTypeClass* FreeUnit;
	InfantryTypeClass* SecretInfantry;
	UnitTypeClass* SecretUnit;
	BuildingTypeClass* SecretBuilding;
	int field_EB0;
	int Adjacent;
	AbstractType Factory;
	CoordStruct TargetCoordOffset;
	CoordStruct ExitCoord;
	CellStruct* FoundationOutside;
	int field_ED8;
	int DeployFacing;
	int PowerBonus;
	int PowerDrain;
	int ExtraPowerBonus;
	int ExtraPowerDrain;
	eFoundation Foundation;
	int Height;
	int OccupyHeight;
	int MidPoint;
	int DoorStages;

	BuildingAnimFrameStruct BuildingAnimFrame[6];

	BuildingAnimStruct BuildingAnim[0x15];

	int Upgrades;
	SHPStruct* DeployingAnim;
	bool DeployingAnimLoaded;
	PROTECTED_PROPERTY(BYTE, align_14E9[3]);
	SHPStruct* UnderDoorAnim;
	bool UnderDoorAnimLoaded;
	PROTECTED_PROPERTY(BYTE, align_14F1[3]);
	SHPStruct* Rubble;
	bool RubbleLoaded;
	PROTECTED_PROPERTY(BYTE, align_14F9[3]);
	SHPStruct* RoofDeployingAnim;
	bool RoofDeployingAnimLoaded;
	PROTECTED_PROPERTY(BYTE, align_1501[3]);
	SHPStruct* UnderRoofDoorAnim;
	bool UnderRoofDoorAnimLoaded;
	PROTECTED_PROPERTY(BYTE, align_1509[3]);
	SHPStruct* DoorAnim;
	SHPStruct* SpecialZOverlay;
	int SpecialZOverlayZAdjust;
	SHPStruct* BibShape;
	bool BibShapeLoaded;
	PROTECTED_PROPERTY(BYTE, align_151D[3]);
	int NormalZAdjust;
	int AntiAirValue;
	int AntiArmorValue;
	int AntiInfantryValue;
	Point2D ZShapePointMove;
	int unknown_1538;
	int unknown_153C;
	int unknown_1540;
	int unknown_1544;
	WORD ExtraLight;
	bool TogglePower;
	bool HasSpotlight;
	bool IsTemple;
	bool IsPlug;
	bool HoverPad;
	bool BaseNormal;
	bool EligibileForAllyBuilding;
	bool EligibleForDelayKill;
	bool NeedsEngineer;
	PROTECTED_PROPERTY(BYTE, align_1553);
	int CaptureEvaEvent;
	int ProduceCashStartup;
	int ProduceCashAmount;
	int ProduceCashDelay;
	int InfantryGainSelfHeal;
	int UnitsGainSelfHeal;
	int RefinerySmokeFrames;
	bool Bib;
	bool Wall;
	bool Capturable;
	bool Powered;
	bool PoweredSpecial;
	bool Overpowerable;
	bool Spyable;
	bool CanC4;
	bool WantsExtraSpace;
	bool Unsellable;
	bool ClickRepairable;
	bool CanBeOccupied;
	bool CanOccupyFire;
	PROTECTED_PROPERTY(BYTE, align_157D[3]);
	int MaxNumberOccupants;
	bool ShowOccupantPips;
	PROTECTED_PROPERTY(BYTE, align_1585[3]);

	Point2D MuzzleFlash[0xA];

	Point2D DamageFireOffset[8];

	Point2D QueueingCell;
	int NumberImpassableRows;

	Point2D RemoveOccupy[8];

	Point2D AddOccupy[8];

	bool Radar;
	bool SpySat;
	bool ChargeAnim;
	bool IsAnimDelayedFire;
	bool SiloDamage;
	bool UnitRepair;
	bool UnitReload;
	bool Bunker;
	bool Cloning;
	bool Grinding;
	bool UnitAbsorb;
	bool InfantryAbsorb;
	bool SecretLab;
	bool DoubleThick;
	bool Flat;
	bool DockUnload;
	bool Recoilless;
	bool HasStupidGuardMode;
	bool BridgeRepairHut;
	bool Gate;
	bool SAM;
	bool ConstructionYard;
	bool NukeSilo;
	bool Refinery;
	bool Weeder;
	bool WeaponsFactory;
	bool LaserFencePost;
	bool LaserFence;
	bool FirestormWall;
	bool Hospital;
	bool Armory;
	bool EMPulseCannon;
	bool TickTank;
	bool TurretAnimIsVoxel;
	bool BarrelAnimIsVoxel;
	bool CloakGenerator;
	bool SensorArray;
	bool ICBMLauncher;
	bool Artillary;
	bool Helipad;
	bool OrePurifier;
	bool FactoryPlant;
	PROTECTED_PROPERTY(BYTE, align_16CE[2]);
	float InfantryCostBonus;
	float UnitsCostBonus;
	float AircraftCostBonus;
	float BuildingsCostBonus;
	float DefensesCostBonus;
	bool GDIBarracks;
	bool NODBarracks;
	bool YuriBarracks;
	PROTECTED_PROPERTY(BYTE, align_16E7[1]);
	float ChargedAnimTime;
	int DelayedFireDelay;
	int SuperWeapon;
	int SuperWeapon2;
	int GateStages;
	int PowersUpToLevel;
	bool DamagedDoor;
	bool InvisibleInGame;
	bool TerrainPalette;
	bool PlaceAnywhere;
	bool ExtraDamageStage;
	bool AIBuildThis;
	bool IsBaseDefense;
	BYTE CloakRadiusInCells;
	bool ConcentricRadialIndicator;
	PROTECTED_PROPERTY(BYTE, align_1709[3]);
	int PsychicDetectionRadius;
	int BarrelStartPitch;
	char VoxelBarrelFile [0x1C];
	CoordStruct VoxelBarrelOffsetToPitchPivotPoint;
	CoordStruct VoxelBarrelOffsetToRotatePivotPoint;
	CoordStruct VoxelBarrelOffsetToBuildingPivotPoint;
	CoordStruct VoxelBarrelOffsetToBarrelEnd;
	bool DemandLoad;
	bool DemandLoadBuildup;
	bool FreeBuildup;
	bool IsThreatRatingNode;
	bool PrimaryFireDualOffset;
	bool ProtectWithWall;
	bool CanHideThings;
	bool CrateBeneath;
	bool LeaveRubble;
	bool CrateBeneathIsMoney;
	char TheaterSpecificID [0x13];
	PROTECTED_PROPERTY(BYTE, align_176B[3]);
	int NumberOfDocks;
	VectorClass<CoordStruct> DockingOffsets;
	PROTECTED_PROPERTY(BYTE, align_1794[4]);
};
