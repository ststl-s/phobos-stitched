/*
	ObjectTypes are initialized by INI files.
*/

#pragma once

#include <AbstractTypeClass.h>
#include <FileSystem.h>

#include <Drawing.h>
#include <IndexClass.h>

//forward declarations
class TechnoTypeClass;
class HouseTypeClass;
class ObjectClass;
class BuildingClass;

class NOVTABLE ObjectTypeClass : public AbstractTypeClass
{
public:

	static constexpr reference<DynamicVectorClass<ObjectTypeClass*>, 0xAC1418u> const Array {};

	//static
	static int __fastcall FindIndex(const char* pID) JMP_STD(0x5F9990);
	static void LoadPips() JMP_STD(0x5F76B0);
	static void sub_5F77F0() JMP_STD(0x5F77F0);
	static int ReleaseAllVoxelCaches() JMP_STD(0x5F99E0);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x5F9720);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override JMP_STD(0x5F9950);
	virtual HRESULT __stdcall GetSizeMax(ULARGE_INTEGER* pcbSize) override JMP_STD(0x5F9970);

	//Destructor
	virtual ~ObjectTypeClass() override RX;// JMP_THIS(0x5F9AE0);

	//AbstractTypeClass
	virtual bool LoadFromINI(CCINIClass* pINI) override JMP_THIS(0x5F92D0);

	//ObjectTypeClass
	virtual CoordStruct* vt_entry_6C(CoordStruct* pDest, CoordStruct* pSrc) const JMP_THIS(0x41CF80);
	virtual DWORD GetOwners() const { return ULONG_MAX; }
	virtual int GetPipMax() const { return 0; }
	virtual void vt_entry_78(DWORD dwUnk) const { CoordStruct* pCrd = reinterpret_cast<CoordStruct*>(dwUnk); *pCrd = { 10,10,10 }; }	//guess dwUnk is CoordStruct
	virtual void Dimension2(CoordStruct* pDest) JMP_THIS(0x5F75E0);
	virtual bool SpawnAtMapCoords(CellStruct* pMapCoords, HouseClass* pOwner) = 0;
	virtual int GetActualCost(HouseClass* pHouse) const { return 0; }
	virtual int GetBuildSpeed() const { return 0; }
	virtual ObjectClass* CreateObject(HouseClass* pOwner) = 0;
	virtual CellStruct * GetFoundationData(bool includeBib) const JMP_THIS(0x5F7640);
	virtual BuildingClass* FindFactory(bool allowOccupied, bool requirePower, bool requireCanBuild, HouseClass const* pHouse) const JMP_THIS(0x5F7900);
	virtual SHPStruct* GetCameo() const { return nullptr; }
	virtual SHPStruct* GetImage() const { return this->Image; }

	static bool __fastcall IsBuildCat5(AbstractType abstractID, int idx)
		{ JMP_STD(0x5004E0); }

	static TechnoTypeClass * __fastcall GetTechnoType(AbstractType abstractID, int idx)
		{ JMP_STD(0x48DCD0); }

	void sub_5F8080()
		{ JMP_THIS(0x5F8080); }

	void LoadVoxel()
		{ JMP_THIS(0x5F8110); }

	bool LoadTurret(const char* pID, int TurretIndex)
		{ JMP_THIS(0x5F7A90); }

	bool LoadBARL(const char* pID, int BARLIndex)
	{ JMP_THIS(0x5F7DB0); }

	bool LoadVehicleImage() //Only UnitType
		{ JMP_THIS(0x5F8CE0); }

	//Constructor
	ObjectTypeClass(const char* pID) noexcept
		: ObjectTypeClass(noinit_t())
	{ JMP_THIS(0x5F7090); }

	ObjectTypeClass(IStream* pStm) noexcept
		: ObjectTypeClass(noinit_t())
	{ JMP_THIS(0x5F7320); }

protected:
	explicit __forceinline ObjectTypeClass(noinit_t) noexcept
		: AbstractTypeClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	ColorStruct RadialColor;
	BYTE          unused_9B;
	Armor         Armor;
	int           Strength;
	SHPStruct*    Image;
	bool          ImageAllocated;
	PROTECTED_PROPERTY(BYTE, align_A9[3]);
	SHPStruct*    AlphaImage;
	VoxelStruct MainVoxel;
	VoxelStruct TurretVoxel; //also used for WO voxels
	VoxelStruct BarrelVoxel;

	VoxelStruct ChargerTurrets [0x12];
	VoxelStruct ChargerBarrels [0x12];

	bool          NoSpawnAlt;
	PROTECTED_PROPERTY(BYTE, align_1E9[3]);
	int           MaxDimension;
	int           CrushSound; //index
	int           AmbientSound; //index

	char ImageFile [0x19];

	bool           AlternateArcticArt;
	bool           ArcticArtInUse; //not read from ini

	char AlphaImageFile [0x19];

	bool           Theater;
	bool           Crushable;
	bool           Bombable;
	bool           RadarInvisible;
	bool           Selectable;
	bool           LegalTarget;
	bool           Insignificant;
	bool           Immune;
	bool           IsLogic; // add objects to the logic vector
	bool           AllowCellContent;
	bool           Voxel;
	bool           NewTheater;
	bool           HasRadialIndicator;
	bool           IgnoresFirestorm;
	bool           UseLineTrail;
	ColorStruct    LineTrailColor;
	PROTECTED_PROPERTY(BYTE, align_23E[2]);
	int            LineTrailColorDecrement;

	IndexClass<MainVoxelIndexKey, VoxelCacheStruct*> VoxelMainCache;
	IndexClass<TurretWeaponVoxelIndexKey, VoxelCacheStruct*> VoxelTurretWeaponCache;
	IndexClass<ShadowVoxelIndexKey, VoxelCacheStruct*> VoxelShadowCache;
	IndexClass<TurretBarrelVoxelIndexKey, VoxelCacheStruct*> VoxelTurretBarrelCache;
};
