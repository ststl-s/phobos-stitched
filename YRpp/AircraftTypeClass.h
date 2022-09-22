#pragma once

#include <TechnoTypeClass.h>

class NOVTABLE AircraftTypeClass : public TechnoTypeClass
{
public:
	static const AbstractType AbsID = AbstractType::AircraftType;

	//Array
	ABSTRACTTYPE_ARRAY(AircraftTypeClass, 0xA8B218u);

	//static
	static void* LoadSomething() JMP_STD(0x41CAF0);
	//static int __fastcall FindIndex(const char* pID) JMP_STD(0x41CAA0);
	//static AircraftTypeClass* __fastcall FindOrAllocate(const char* pID) JMP_STD(0x41CEF0);

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) JMP_STD(0x41CEB0);
	virtual HRESULT __stdcall Load(IStream* pStm) JMP_STD(0x41CE20);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) JMP_STD(0x41CE90);

	//Destructor
	virtual ~AircraftTypeClass() JMP_THIS(0x41CFE0);

	//AbstractClass
	virtual AbstractType WhatAmI() const { return AbstractType::AircraftType; }
	virtual int	Size() const { return 0xE10; }
	virtual void ComputeCRC(CRCEngine& crc) const JMP_THIS(0x41CDB0);
	virtual int GetArrayIndex() const { return this->ArrayIndex; }

	//AbstractTypeClass
	virtual bool LoadFromINI(CCINIClass* pINI) JMP_THIS(0x41CC20);
	
	//ObjectTypeClass
	virtual void Dimension2(CoordStruct* pDest) JMP_THIS(0x41CBF0);
	virtual bool SpawnAtMapCoords(CellStruct* pMapCoords, HouseClass* pOwner) { return false; }
	virtual ObjectClass* CreateObject(HouseClass* pOwner) JMP_THIS(0x41CB20);
	virtual CellStruct* GetFoundationData(bool IncludeBib) const JMP_THIS(0x41CB70);

	//TechnoTypeClass
	virtual bool vt_entry_A0() { return false; }
	virtual bool CanAttackMove() const { return false; }//JMP_THIS(0x41CB60)

	//Constructor
	AircraftTypeClass(const char* pID) noexcept
		: AircraftTypeClass(noinit_t())
	{ JMP_THIS(0x41C8B0); }

protected:
	explicit __forceinline AircraftTypeClass(noinit_t) noexcept
		: TechnoTypeClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	int ArrayIndex;
	bool Carryall;
	PROTECTED_PROPERTY(BYTE, align_DFD[3]);
	AnimTypeClass* Trailer;
	int SpawnDelay;
	bool Rotors;
	bool CustomRotor;
	bool Landable;
	bool FlyBy;
	bool FlyBack;
	bool AirportBound;
	bool Fighter;
	PROTECTED_PROPERTY(BYTE, align_E0F);
};
