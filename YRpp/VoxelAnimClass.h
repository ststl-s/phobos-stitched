/*
	Voxel Animations
*/

#pragma once

#include <ObjectClass.h>
#include <VoxelAnimTypeClass.h>
#include <BounceClass.h>

//forward declarations
class HouseClass;
class ParticleSystemClass;

class NOVTABLE VoxelAnimClass : public ObjectClass
{
public:
	static const AbstractType AbsID = AbstractType::VoxelAnim;

	//Static
	static constexpr constant_ptr<DynamicVectorClass<VoxelAnimClass*>, 0x887388u> const Array{};

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override JMP_STD(0x74AAD0);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x74A970);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override JMP_STD(0x74AA10);

	//Destructor
	virtual ~VoxelAnimClass() override RX;//JMP_THIS(0x74AB50);

	//AbstractClass
	virtual AbstractType WhatAmI() const override JMP_THIS(0x74AB20);
	virtual int	Size() const override JMP_THIS(0x74AB10);
	virtual void ComputeCRC(CRCEngine& crc) const override JMP_THIS(0x74AA30);
	virtual void Update() override JMP_THIS(0x749F30);

	//ObjectClass
	virtual Layer InWhichLayer() const override JMP_THIS(0x74A960);
	virtual VoxelAnimTypeClass* GetType() const override JMP_THIS(0x74AB30);
	virtual bool DrawIfVisible(RectangleStruct* pBounds, bool evenIfCloaked, DWORD dwUnk3) const override JMP_THIS(0x749B20);
	virtual CellStruct const* GetFoundationData(bool includeBib = false) const override JMP_THIS(0x74AB40);
	virtual void DrawIt(Point2D* pLocation, RectangleStruct* pBounds) const override JMP_THIS(0x749B70);

	//Constructor
	VoxelAnimClass(VoxelAnimTypeClass* pType, CoordStruct* pCoords, HouseClass* pOwner)
		: VoxelAnimClass(noinit_t())
	{ JMP_THIS(0x7493B0); }

protected:
	explicit __forceinline VoxelAnimClass(noinit_t)
		: ObjectClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	PROTECTED_PROPERTY(DWORD, unused_AC);
	DECLARE_PROPERTY(BounceClass, Bounce);
	int unknown_int_100;
	VoxelAnimTypeClass* Type;
	ParticleSystemClass* AttachedSystem;
	HouseClass* OwnerHouse;
	bool TimeToDie; // remove on next update
	PROTECTED_PROPERTY(BYTE, unused_111[3]);
	DECLARE_PROPERTY(AudioController, Audio3);
	DECLARE_PROPERTY(AudioController, Audio4);
	bool Invisible; // don't draw, but Update state anyway
	PROTECTED_PROPERTY(BYTE, unused_13D[3]);
	int Duration; // counting down to zero
	PROTECTED_PROPERTY(DWORD, unused_144);
};
