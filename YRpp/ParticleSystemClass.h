/*
	ParticleSystems
*/

#pragma once

#include <ObjectClass.h>
#include <ParticleSystemTypeClass.h>
#include <ParticleClass.h>

class NOVTABLE ParticleSystemClass : public ObjectClass
{
public:
	static const AbstractType AbsID = AbstractType::ParticleSystem;

	//Static
	static constexpr constant_ptr<DynamicVectorClass<ParticleSystemClass*>, 0xA80208u> const Array{};

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override JMP_STD(0x6301A0);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x62FF20);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override JMP_STD(0x630090);

	//Destructor
	virtual ~ParticleSystemClass() override JMP_THIS(0x630230);

	//AbstractClass
	virtual void PointerExpired(AbstractClass* pAbstract, bool removed) override JMP_THIS(0x62FE90);
	virtual AbstractType WhatAmI() const override { return AbstractType::ParticleSystem; }
	virtual int Size() const override { return 0x100; }
	virtual void ComputeCRC(CRCEngine& crc) const override JMP_THIS(0x630100);
	virtual bool IsDead() const override JMP_THIS(0x62FE60);
	virtual void Update() override JMP_THIS(0x62FD60);

	//ObjectClass
	virtual Layer InWhichLayer() const override JMP_THIS(0x62FE80);
	virtual ObjectTypeClass* GetType() const override JMP_THIS(0x630220);
	virtual void UnInit() override JMP_THIS(0x6301E0);
	virtual void DrawIt(Point2D* pLocation, RectangleStruct* pBounds) const override JMP_THIS(0x62E280);

	//Constructor
	ParticleSystemClass
	(
		ParticleSystemTypeClass* pParticleSystemType,
		const CoordStruct& coords,
		AbstractClass* pTarget,
		ObjectClass* pOwner,
		const CoordStruct& targetCoords,
		HouseClass* pOwnerHouse
	) noexcept : ParticleSystemClass(noinit_t())
	{ JMP_THIS(0x62DC50); }

protected:
	explicit __forceinline ParticleSystemClass(noinit_t) noexcept
		: ObjectClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	ParticleSystemTypeClass* Type;
	CoordStruct  SpawnDistanceToOwner;
	DynamicVectorClass<ParticleClass*> Particles;
	CoordStruct TargetCoords;
	ObjectClass* Owner;
	AbstractClass* Target; // CellClass or TechnoClass
	int          SpawnFrames; //from ParSysTypeClass
	int          Lifetime; //from ParSysTypeClass
	int          SparkSpawnFrames; //from ParSysTypeClass
	int          SpotlightRadius; //defaults to 29
	bool         TimeToDie;
	bool         unknown_bool_F9;
	PROTECTED_PROPERTY(BYTE, align_F10[2]);
	HouseClass*  OwnerHouse;
};
