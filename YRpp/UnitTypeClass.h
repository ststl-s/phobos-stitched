#pragma once

#include <TechnoTypeClass.h>

class TechnoClass;

class NOVTABLE UnitTypeClass : public TechnoTypeClass
{
public:
	static const AbstractType AbsID = AbstractType::UnitType;

	//Array
	ABSTRACTTYPE_ARRAY(UnitTypeClass, 0xA83CE0u);

	//static
	static void* sub_7473E0() JMP_STD(0x7473E0);
	//static int __fastcall FindIndex(const char* pID) JMP_STD(0x747370);
	//static UnitTypeClass* __fastcall FindOrAllocate(const char* pID) JMP_STD(0x7480D0);

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) JMP_STD(0x747F30);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) JMP_STD(0x748010);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) JMP_STD(0x7480B0);

	//Destructor
	virtual ~UnitTypeClass() JMP_THIS(0x748190);

	//AbstractClass
	virtual AbstractType WhatAmI() const { return AbstractType::UnitType; }
	virtual int Size() const { return 0xE78; }
	virtual void CalculateChecksum(Checksummer& checksum) const JMP_THIS(0x747F70);
	virtual int GetArrayIndex() const { return this->ArrayIndex; }

	//AbstractTypeClass
	virtual bool LoadFromINI(CCINIClass* pINI) JMP_THIS(0x747620);

	//ObjectTypeClass
	virtual CoordStruct* vt_entry_6C(CoordStruct* pDest, CoordStruct* pSrc) const JMP_THIS(0x747EB0);
	virtual void vt_entry_78(DWORD dwUnk) const JMP_THIS(0x747590);
	virtual void Dimension2(CoordStruct* pDest) JMP_THIS(0x7475D0);
	virtual bool SpawnAtMapCoords(CellStruct* pMapCoords, HouseClass* pOwner) JMP_THIS(0x7474B0);
	virtual ObjectClass* CreateObject(HouseClass* pOwner) JMP_THIS(0x747568);

	//TechnoTypeClass
	virtual bool vt_entry_A0() { return this->TechnoTypeClass::vt_entry_A0(); }
	virtual bool CanAttackMove() const { return this->TechnoTypeClass::CanAttackMove(); }//JMP_THIS(0x7473D0)
	virtual int GetRepairStep() const JMP_THIS(0x747F20);

	//Constructor
	UnitTypeClass(const char* pID) noexcept
		: UnitTypeClass(noinit_t())
	{ JMP_THIS(0x7470D0); }

protected:
	explicit __forceinline UnitTypeClass(noinit_t) noexcept
		: TechnoTypeClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	int ArrayIndex;
	LandType MovementRestrictedTo;
	CoordStruct HalfDamageSmokeLocation;
	bool Passive;
	bool CrateGoodie;
	bool Harvester;
	bool Weeder;
	bool unknown_E10;
	bool HasTurret; //not read from the INIs
	bool DeployToFire;
	bool IsSimpleDeployer;
	bool IsTilter;
	bool UseTurretShadow;
	bool TooBigToFitUnderBridge;
	bool CanBeach;
	bool SmallVisceroid;
	bool LargeVisceroid;
	bool CarriesCrate;
	bool NonVehicle;
	int StandingFrames;
	int DeathFrames;
	int DeathFrameRate;
	int StartStandFrame;
	int StartWalkFrame;
	int StartFiringFrame;
	int StartDeathFrame;
	int MaxDeathCounter;
	int Facings;
	int FiringSyncFrame0;
	int FiringSyncFrame1;
	int BurstDelay0;
	int BurstDelay1;
	int BurstDelay2;
	int BurstDelay3;
	SHPStruct* AltImage;
	char WalkFrames;
	char FiringFrames;
	char AltImageFile [0x19];
	PROTECTED_PROPERTY(BYTE, align_E77);
};
