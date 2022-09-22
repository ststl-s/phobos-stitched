/*
	AnimTypes are initialized by INI files.
*/

#pragma once

#include <TechnoTypeClass.h>

struct SubSequenceStruct
{
	int StartFrame;
	int CountFrames;
	int FacingMultiplier;
	SequenceFacing Facing;
	int SoundCount;
	int Sound1StartFrame;
	int Sound1Index; // VocClass
	int Sound2StartFrame;
	int Sound2Index; // VocClass
};

struct SequenceStruct
{
	SubSequenceStruct& GetSequence(Sequence sequence) {
		return this->Sequences[static_cast<int>(sequence)];
	}

	const SubSequenceStruct& GetSequence(Sequence sequence) const {
		return this->Sequences[static_cast<int>(sequence)];
	}

	SubSequenceStruct Sequences[42];
};

class NOVTABLE InfantryTypeClass : public TechnoTypeClass
{
public:
	static const AbstractType AbsID = AbstractType::InfantryType;

	//Array
	ABSTRACTTYPE_ARRAY(InfantryTypeClass, 0xA8E348u);

	//static
	//static int __fastcall FindIndex(InfantryTypeClass* pThis) JMP_THIS(0x523C90);
	//static InfantryTypeClass* __fastcall FindOrAllocate(const char* pID) JMP_THIS(0x524CB0);

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override JMP_STD(0x524C70);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x524960);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override JMP_STD(0x524B60);

	//Destructor
	virtual ~InfantryTypeClass() override JMP_THIS(0x524D70);

	//AbstractClass
	virtual AbstractType WhatAmI() const override { return AbstractType::InfantryType; }
	virtual int	Size() const { return 0xED0; }
	virtual void ComputeCRC(CRCEngine& crc) const override JMP_THIS(0x524840);
	virtual int GetArrayIndex() const override { return this->ArrayIndex; }

	//AbstractTypeClass
	virtual bool LoadFromINI(CCINIClass* pINI) override JMP_THIS(0x5240A0);

	//ObjectTypeClass
	virtual CoordStruct* vt_entry_6C(CoordStruct* pDest, CoordStruct* pSrc) const override JMP_THIS(0x5247D0);
	virtual void Dimension2(CoordStruct* pDest) override JMP_THIS(0x524760);
	virtual bool SpawnAtMapCoords(CellStruct* pMapCoords, HouseClass* pOwner) override JMP_THIS(0x523B40);
	virtual ObjectClass* CreateObject(HouseClass* pOwner) override JMP_THIS(0x523B10);
	virtual CellStruct* GetFoundationData(bool IncludeBib) const override JMP_THIS(0x523C20);

	//TechnoTypeClass
	virtual bool vt_entry_A0() override { return this->TechnoTypeClass::vt_entry_A0(); } //what the hell, return true directly
	virtual bool CanAttackMove() const override { return this->TechnoTypeClass::CanAttackMove(); }//JMP_THIS(0x5247C0)
	virtual int GetRepairStepCost() const override { return 0; }
	virtual int GetRepairStep() const override JMP_THIS(0x524790);

	//non-virtual
	int ReadSequence()
	{ JMP_THIS(0x523D00); }

	//Constructor
	InfantryTypeClass(const char* pID) noexcept
		: InfantryTypeClass(noinit_t())
	{ JMP_THIS(0x5236A0); }

	InfantryTypeClass(IStream* pStm) noexcept
		: InfantryTypeClass(noinit_t())
	{ JMP_THIS(0x523980); }

protected:
	explicit __forceinline InfantryTypeClass(noinit_t) noexcept
		: TechnoTypeClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	int ArrayIndex;
	PipIndex Pip;
	PipIndex OccupyPip;
	WeaponStruct OccupyWeapon;
	WeaponStruct EliteOccupyWeapon;
	SequenceStruct* Sequence;
	int FireUp;
	int FireProne;
	int SecondaryFire;
	int SecondaryProne;
	TypeList<AnimTypeClass*> DeadBodies;
	TypeList<AnimTypeClass*> DeathAnims;
	TypeList<int> VoiceComment;
	int EnterWaterSound;
	int LeaveWaterSound;
	bool Cyborg;
	bool NotHuman;
	bool Ivan; //used for the bomb attack cursor...
	PROTECTED_PROPERTY(BYTE, align_EAF);
	int DirectionDistance;
	bool Occupier;
	bool Assaulter;
	PROTECTED_PROPERTY(BYTE, align_EB6[2]);
	int HarvestRate;
	bool Fearless;
	bool Crawls;
	bool Infiltrate;
	bool Fraidycat;
	bool TiberiumProof;
	bool Civilian;
	bool C4;
	bool Engineer;
	bool Agent;
	bool Thief;
	bool VehicleThief;
	bool Doggie;
	bool Deployer;
	bool DeployedCrushable;
	bool UseOwnName;
	bool JumpJetTurn;
	PROTECTED_PROPERTY(BYTE, align_ECC[4]);
};
