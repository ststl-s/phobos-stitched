#pragma once

#include <GeneralStructures.h>
#include <ObjectClass.h>

class NOVTABLE BuildingLightClass : public ObjectClass
{
public:
	static const AbstractType AbsID = AbstractType::BuildingLight;

	//Static
	static constexpr constant_ptr<DynamicVectorClass<BuildingLightClass*>, 0x8B4190u> const Array{};

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override JMP_STD(0x436910);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x436950);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override JMP_STD(0x4369C0);

	//Destructor
	virtual ~BuildingLightClass() override RX;// JMP_THIS(0x4370C0);

	//AbstractClass
	virtual void PointerExpired(AbstractClass* pAbstract, bool removed) override JMP_THIS(0x436A00);
	virtual AbstractType WhatAmI() const override JMP_THIS(0x4370B0);
	virtual int	Size() const override JMP_THIS(0x436900);
	virtual void ComputeCRC(CRCEngine& crc) const override JMP_THIS(0x436F40);
	virtual void Update() override JMP_THIS(0x4361D0);

	//ObjectClass
	virtual Layer InWhichLayer() const override JMP_THIS(0x4369F0);
	virtual ObjectTypeClass* GetType() const override JMP_THIS(0x4369E0);
	virtual bool Limbo() override JMP_THIS(0x437030);
	virtual bool Unlimbo(const CoordStruct& coords, DirType faceDir) override JMP_THIS(0x437050);
	virtual void DrawIt(Point2D* pLocation, RectangleStruct* pBounds) const override JMP_THIS(0x435BE0);

	// non-virtual
	void SetBehaviour(SpotlightBehaviour mode)
	{ JMP_THIS(0x436BE0); }

	//Constructor
	BuildingLightClass(ObjectClass* pOwner) noexcept
		: BuildingLightClass(noinit_t())
	{ JMP_THIS(0x435820); }

protected:
	explicit __forceinline BuildingLightClass(noinit_t) noexcept
		: ObjectClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	double Speed;
	CoordStruct field_B8;
	CoordStruct field_C4;
	double Acceleration;
	bool Direction;
	SpotlightBehaviour BehaviourMode;
	ObjectClass * FollowingObject;
	TechnoClass * OwnerObject;
};
