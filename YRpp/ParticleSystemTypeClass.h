/*
	ParticleSystemTypes are initialized by INI files.
*/

#pragma once

#include <ObjectTypeClass.h>

//forward declarations

class NOVTABLE ParticleSystemTypeClass : public ObjectTypeClass
{
public:
	static const AbstractType AbsID = AbstractType::ParticleSystemType;
	
	//static
	static BehavesLike __fastcall GetBehave(const char* behaveID) JMP_STD(0x644850);
	static int __fastcall FindIndexOrAllocate(const char* pID) JMP_STD(0x644630);
	//static ParticleSystemTypeClass* __fastcall FindOrAllocate(const char* pID) JMP_STD(0x644890);

	//Array
	ABSTRACTTYPE_ARRAY(ParticleSystemTypeClass, 0xA83D68u);

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) JMP_STD(0x6447A0);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) JMP_STD(0x6447E0);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) JMP_STD(0x644830);

	//Destructor
	virtual ~ParticleSystemTypeClass() JMP_THIS(0x644960);

	//AbstractClass
	virtual AbstractType WhatAmI() const { return AbstractType::ParticleSystemType; }
	virtual int Size() const { return 0x310; }
	virtual void CalculateChecksum(Checksummer& checksum) const JMP_THIS(0x644700);

	//AbstractTypeClass
	virtual bool LoadFromINI(CCINIClass* pINI) JMP_THIS(0x6442D0);

	//ObjectTypeClass
	virtual bool SpawnAtMapCoords(CellStruct* pMapCoords, HouseClass* pOwner) { return false; }
	virtual ObjectClass* CreateObject(HouseClass* owner) { return nullptr; }

	//Constructor
	ParticleSystemTypeClass(const char* pID) noexcept
		: ParticleSystemTypeClass(noinit_t())
	{ JMP_THIS(0x6440A0); }

protected:
	explicit __forceinline ParticleSystemTypeClass(noinit_t) noexcept
		: ObjectTypeClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	int      HoldsWhat; //ParticleType Array index
	bool     Spawns;
	PROTECTED_PROPERTY(BYTE, align_299[3]);
	int      SpawnFrames;
	float    Slowdown;
	int      ParticleCap;
	int      SpawnRadius;
	float    SpawnCutoff;
	float    SpawnTranslucencyCutoff;
	BehavesLike BehavesLike;
	int      Lifetime;
	Vector3D<float> SpawnDirection;
	double   ParticlesPerCoord;
	double   SpiralDeltaPerCoord;
	double   SpiralRadius;
	double   PositionPerturbationCoefficient;
	double   MovementPerturbationCoefficient;
	double   VelocityPerturbationCoefficient;
	double   SpawnSparkPercentage;
	int      SparkSpawnFrames;
	int      LightSize;
	ColorStruct LaserColor;
	bool     Laser;
	bool     OneFrameLight;
	PROTECTED_PROPERTY(BYTE, align_30D[3]);
};
