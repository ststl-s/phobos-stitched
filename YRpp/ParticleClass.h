/*
	Particles
*/

#pragma once

#include <ObjectClass.h>
#include <ParticleTypeClass.h>

//forward declarations
class ParticleSystemClass;

class NOVTABLE ParticleClass : public ObjectClass
{
public:
	static const AbstractType AbsID = AbstractType::Particle;

	//Static
	static constexpr constant_ptr<DynamicVectorClass<ParticleClass*>, 0xA83DC8u> const Array{};

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override JMP_STD(0x62D930);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x62D7A0);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override JMP_STD(0x62D810);

	//Destructor
	virtual ~ParticleClass() override RX;// JMP_THIS(0x62D9A0);

	//AbstractClass
	virtual AbstractType WhatAmI() const override { return AbstractType::Particle; }
	virtual int Size() const override { return 0x138; }

	//ObjectClass
	virtual Layer InWhichLayer() const override JMP_THIS(0x62D770);
	virtual ObjectTypeClass* GetType() const override JMP_THIS(0x62D990);
	virtual CellStruct const* GetFoundationData(bool includeBib = false) const override JMP_THIS(0x62D710);
	virtual void DrawIt(Point2D* pLocation, RectangleStruct* pBounds) const override JMP_THIS(0x62CEC0);
	virtual bool UpdatePlacement(PlacementType value) override JMP_THIS(0x62D6F0);

	//ParticleClass
	virtual int vt_entry_1E8() JMP_THIS(0x62D830);

	//Constructor
	ParticleClass
	(
		ParticleTypeClass* pParticleType,
		CoordStruct* pCrd1,
		CoordStruct* pCrd2,
		ParticleSystemClass* pParticleSystem
	) noexcept
		: ParticleClass(noinit_t())
	{ JMP_THIS(0x62B5E0); }

protected:
	explicit __forceinline ParticleClass(noinit_t) noexcept
		: ObjectClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	ParticleTypeClass* Type;
	BYTE   unknown_B0;
	BYTE   unknown_B1;
	BYTE   unknown_B2;
	PROTECTED_PROPERTY(BYTE, align_B3);
	DWORD  unknown_B4;
	DWORD  unknown_B8;
	DWORD  unknown_BC;
	DWORD  unknown_C0;
	DWORD  unknown_C4;
	DWORD  unknown_C8;
	DWORD  unknown_CC;
	double unknown_double_D0;
	DWORD  unknown_D8;
	DWORD  unknown_DC;
	DWORD  unknown_E0;
	float  Velocity;
	CoordStruct unknown_coords_E8; //Crd2 in CTOR
	CoordStruct unknown_coords_F4; //Crd1 in CTOR
	CoordStruct unknown_coords_100; //{ 0, 0, 0} in CTOR
	Vector3D<float> unknown_vector3d_10C;
	Vector3D<float> unknown_vector3d_118;
	ParticleSystemClass*   ParticleSystem;
	WORD   RemainingEC;
	WORD   RemainingDC;
	BYTE   StateAIAdvance;
	BYTE   unknown_12D;
	BYTE   StartStateAI;
	BYTE   Translucency;
	BYTE   unknown_130;
	BYTE   unknown_131;
	PROTECTED_PROPERTY(BYTE, align_132[2]);
	PROTECTED_PROPERTY(DWORD, align_134);
};
