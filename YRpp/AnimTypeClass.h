/*
	AnimTypes are initialized by INI files.
*/

#pragma once

#include <ObjectTypeClass.h>

//forward declarations
class OverlayTypeClass;
class ParticleTypeClass;
class WarheadTypeClass;

class NOVTABLE AnimTypeClass : public ObjectTypeClass
{
public:
	static const AbstractType AbsID = AbstractType::AnimType;

	//Array
	ABSTRACTTYPE_ARRAY(AnimTypeClass, 0x8B4150u);

	//static
	static int __fastcall LoadAllAnimFile(TheaterType theater) JMP_STD(0x427940);
	//static int __fastcall FindIndex(const char* pID) JMP_STD(0x427CB0);
	//static AnimTypeClass* __fastcall Find(const char* pID) JMP_STD(0x428B80);
	//static AnimTypeClass* __fastcall FindOrAllocate(const char* pID) JMP_STD(0x428B80);
	//static AnimTypeClass* __fastcall FindOrAllocate(const char* pID, DynamicVectorClass<AnimTypeClass*>* Array) JMP_STD(0x428F70);

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override JMP_STD(0x428990);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x428800);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override JMP_STD(0x428970);

	//AbstractClass
	virtual void PointerExpired(AbstractClass* pAbstract, bool removed) override JMP_THIS(0x428C10);
	virtual AbstractType WhatAmI() const override { return AbstractType::AnimType; }
	virtual int	Size() const override { return 0x378; }
	virtual void ComputeCRC(CRCEngine& crc) const override JMP_THIS(0x4289D0);
	virtual int GetArrayIndex() const override { return this->ArrayIndex; }

	//AbstractTypeClass
	virtual void LoadTheaterSpecificArt(TheaterType th_type) override JMP_THIS(0x427A80);
	virtual bool LoadFromINI(CCINIClass* pINI) override JMP_THIS(0x427D00);

	//ObjectTypeClass
	virtual bool SpawnAtMapCoords(CellStruct* pMapCoords, HouseClass* pOwner) override { return false; } //yes, it return false directly, I agree with the below
	virtual ObjectClass* CreateObject(HouseClass* owner) override { return nullptr; }; // ! this just returns NULL instead of creating the anim, fucking slackers

	//AnimTypeClass
	virtual SHPStruct* LoadImage() JMP_THIS(0x428C30);
	virtual void Load2DArt() JMP_THIS(0x427B50);

	//Destructor
	virtual ~AnimTypeClass() JMP_THIS(0x428EA0);

	//Constructor
	AnimTypeClass(const char* pID) noexcept
		: AnimTypeClass(noinit_t())
	{ JMP_THIS(0x427530); }

	void FreeLoadedImage() JMP_THIS(0x428DE0);

protected:
	explicit __forceinline AnimTypeClass(noinit_t) noexcept
		: ObjectTypeClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	int ArrayIndex;
	int MiddleFrameIndex;
	int MiddleFrameWidth;
	int MiddleFrameHeight;
	BYTE unknown_2A4;
	PROTECTED_PROPERTY(BYTE,align_2A5[3])
	double Damage;
	int Rate;
	int Start;
	int LoopStart;
	int LoopEnd;
	int End;
	int LoopCount;
	AnimTypeClass* Next;
	int SpawnsParticle; // index of that ParticleTypeClass
	int NumParticles;
	int DetailLevel;
	int TranslucencyDetailLevel;
	RandomStruct RandomLoopDelay;
	RandomStruct RandomRate;
	int Translucency;
	AnimTypeClass* Spawns;
	int SpawnCount;
	int Report;		//VocClass index
	int StopSound;		//VocClass index
	AnimTypeClass* BounceAnim;
	AnimTypeClass* ExpireAnim;
	AnimTypeClass* TrailerAnim;
	int TrailerSeperation;	//MISTYPE BY WESTWOOD!
	double Elasticity;
	double MinZVel;
	double unknown_double_320;
	double MaxXYVel;
	WarheadTypeClass* Warhead;
	int DamageRadius;
	OverlayTypeClass* TiberiumSpawnType;
	int TiberiumSpreadRadius;
	int YSortAdjust;
	int YDrawOffset;
	int ZAdjust;
	int MakeInfantry;
	int RunningFrames;
	bool IsFlamingGuy;
	bool IsVeins;
	bool IsMeteor;
	bool TiberiumChainReaction;
	bool IsTiberium;
	bool HideIfNoOre;
	bool Bouncer;
	bool Tiled;
	bool ShouldUseCellDrawer;
	bool UseNormalLight;
	bool DemandLoad; // not loaded from ini anymore
	bool FreeLoad;  // not loaded from ini anymore
	bool IsAnimatedTiberium;
	bool AltPalette;
	bool Normalized;
	PROTECTED_PROPERTY(BYTE, align_363);
	Layer Layer;
	bool DoubleThick;
	bool Flat;
	bool Translucent;
	bool Scorch;
	bool Flamer;
	bool Crater;
	bool ForceBigCraters;
	bool Sticky;
	bool PingPong;
	bool Reverse;
	bool Shadow;
	bool PsiWarning;
	bool ShouldFogRemove;
	PROTECTED_PROPERTY(BYTE, align_375[3]);
};
