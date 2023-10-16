/*
	SuperWeaponTypes!! =D
*/

#pragma once

#include <FileSystem.h>
#include <AbstractTypeClass.h>

//forward declarations
class BuildingTypeClass;
class ObjectClass;
class WeaponTypeClass;

class NOVTABLE SuperWeaponTypeClass : public AbstractTypeClass
{
public:
	static const AbstractType AbsID = AbstractType::SuperWeaponType;

	//Array
	ABSTRACTTYPE_ARRAY(SuperWeaponTypeClass, 0xA8E330u);

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override JMP_STD(0x6CE7C0);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x6CE800);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override JMP_STD(0x6CE8D0);

	//Destructor
	virtual ~SuperWeaponTypeClass() RX; //JMP_THIS(0x6CEFE0);

	//AbstractClass
	virtual AbstractType WhatAmI() const override JMP_THIS(0x6CE8F0);
	virtual int Size() const override JMP_THIS(0x6CE900);
	virtual void ComputeCRC(CRCEngine& crc) const override JMP_THIS(0x6CE910);
	virtual int GetArrayIndex() const override JMP_THIS(0x6CEA10);

	//AbstractTypeClass
	virtual bool LoadFromINI(CCINIClass* pINI) override JMP_THIS(0x6CEA20);

	//SuperWeaponTypeClass
	virtual Action MouseOverObject(CellStruct const& cell, ObjectClass* pObjBelowMouse) const JMP_THIS(0x6CEF80);

	// non-virtual
	static SuperWeaponTypeClass * __fastcall FindFirstOfAction(Action Action)
	{ JMP_STD(0x6CEEB0); }

	//Constructor
	SuperWeaponTypeClass(const char* pID) noexcept
		: SuperWeaponTypeClass(noinit_t())
	{ JMP_THIS(0x6CE5B0); }

protected:
	explicit __forceinline SuperWeaponTypeClass(noinit_t) noexcept
		: AbstractTypeClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	int     ArrayIndex;
	WeaponTypeClass* WeaponType;

	//I believe these four are the leftover TS sounds
	int     RechargeVoice; // not read, unused
	int     ChargingVoice; // not read, unused
	int     ImpatientVoice; // not read, unused
	int     SuspendVoice; // not read, unused
	//---

	int     RechargeTime; //in frames
	SuperWeaponType Type;
	SHPStruct* SidebarImage;
	Action Action;
	int     SpecialSound;
	int     StartSound;
	BuildingTypeClass* AuxBuilding;
	char SidebarImageFile [0x18];
	PROTECTED_PROPERTY(BYTE, zero_E4);
	bool    UseChargeDrain;
	bool    IsPowered;
	bool    DisableableFromShell;
	int     FlashSidebarTabFrames;
	bool    AIDefendAgainst;
	bool    PreClick;
	bool    PostClick;
	int		PreDependent;
	bool    ShowTimer;
	bool    ManualControl;
	float   Range;
	int     LineMultiplier;

};
