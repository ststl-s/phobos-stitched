/*
	Warheads
*/

#pragma once

#include <AbstractTypeClass.h>

//forward declarations
class AnimTypeClass;
class ParticleTypeClass;
class VoxelAnimTypeClass;

struct WarheadFlags {
	bool ForceFire;
	bool Retaliate;
	bool PassiveAcquire;

	WarheadFlags(bool FF = true, bool Retal = true, bool Acquire = true) : ForceFire(FF), Retaliate(Retal), PassiveAcquire(Acquire) {};
};

class NOVTABLE WarheadTypeClass : public AbstractTypeClass
{
public:
	static const AbstractType AbsID = AbstractType::WarheadType;

	//Array
	ABSTRACTTYPE_ARRAY(WarheadTypeClass, 0x8874C0u);

	//static
	//static WarheadTypeClass* __fastcall Find(const char* pID) JMP_STD(0x75E4A0);
	//static WarheadTypeClass* __fastcall FindOrAllocate(const char* pID) JMP_STD(0x75E3B0);

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) JMP_STD(0x75E080);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) JMP_STD(0x75E0C0);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) JMP_STD(0x75E2C0);

	//Destructor
	virtual ~WarheadTypeClass() JMP_THIS(0x75E510);

	//AbstractClass
	virtual void PointerExpired(AbstractClass* pAbstract, bool removed) JMP_THIS(0x75E440);
	virtual AbstractType WhatAmI() const { return AbstractType::WarheadType; }
	virtual int Size() const { return 0x1D0; }
	virtual void CalculateChecksum(Checksummer& checksum) const JMP_THIS(0x75DEC0);

	//AbstractTypeClass
	virtual bool LoadFromINI(CCINIClass* pINI) JMP_THIS(0x75D3A0);

	//Constructor
	WarheadTypeClass(const char* pID)
		: WarheadTypeClass(noinit_t())
	{ JMP_THIS(0x75CEC0); }

protected:
	explicit __forceinline WarheadTypeClass(noinit_t)
		: AbstractTypeClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	double  Deform;

	double Verses [0xB];

	double  ProneDamage;
	int     DeformTreshold;

	TypeList<AnimTypeClass*> AnimList;

	InfDeath InfDeath;
	float   CellSpread;
	float   CellInset;
	float   PercentAtMax;
	bool    CausesDelayKill;
	PROTECTED_PROPERTY(BYTE, align_131[3]);
	int     DelayKillFrames;
	float   DelayKillAtMax;
	float   CombatLightSize;
	ParticleTypeClass* Particle;
	bool    Wall;
	bool    WallAbsoluteDestroyer;
	bool    PenetratesBunker;
	bool    Wood;
	bool    Tiberium;
	bool    unknown_bool_149;
	bool    Sparky;
	bool    Sonic;
	bool    Fire;
	bool    Conventional;
	bool    Rocker;
	bool    DirectRocker;
	bool    Bright;
	bool    CLDisableRed;
	bool    CLDisableGreen;
	bool    CLDisableBlue;
	bool    EMEffect;
	bool    MindControl;
	bool    Poison;
	bool    IvanBomb;
	bool    ElectricAssault;
	bool    Parasite;
	bool    Temporal;
	bool    IsLocomotor;
	_GUID   Locomotor;
	bool    Airstrike;
	bool    Psychedelic;
	bool    BombDisarm;
	PROTECTED_PROPERTY(BYTE, align_16F);
	int     Paralyzes;
	bool    Culling;
	bool    MakesDisguise;
	bool    NukeMaker;
	bool    Radiation;
	bool    PsychicDamage;
	bool    AffectsAllies;
	bool    Bullets;
	bool    Veinhole;
	int     ShakeXlo;
	int     ShakeXhi;
	int     ShakeYlo;
	int     ShakeYhi;

	TypeList<VoxelAnimTypeClass*> DebrisTypes;
	TypeList<int> DebrisMaximums;

	int     MaxDebris;
	int     MinDebris;
	PROTECTED_PROPERTY(DWORD, unused_1CC); //???
};
