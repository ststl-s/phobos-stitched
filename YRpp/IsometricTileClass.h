#pragma once

#include <IsometricTileTypeClass.h>
#include <ObjectClass.h>

class NOVTABLE IsometricTileClass : public ObjectClass
{
public:
	static const AbstractType AbsID = AbstractType::Isotile;

	//Array
	static constexpr constant_ptr<DynamicVectorClass<IsometricTileClass*>, 0x87F750u> Array{};

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override JMP_THIS(0x543AB0);

	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x543990);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) JMP_STD(0x5439F0);

	//Destructor
	virtual ~IsometricTileClass() override RX;// JMP_THIS(0x543B10);

	//AbstractClass
	virtual AbstractType WhatAmI() const override { return AbstractType::Isotile; }
	virtual int Size() const override { return 0xB0; }

	//ObjectClass
	virtual IsometricTileTypeClass* GetType() const override { return this->Type; }
	virtual bool Limbo() override JMP_THIS(0x543A40);
	virtual bool Unlimbo(const CoordStruct& Crd, DirType dFaceDir) override JMP_THIS(0x543A10);
	virtual void DrawIt(Point2D* pLocation, RectangleStruct* pBounds) const override {}
	virtual bool UpdatePlacement(PlacementType value) override JMP_THIS(0x543330);

	//Constructor
	IsometricTileClass(int idxType, CellStruct const& location) noexcept
		: IsometricTileClass(noinit_t())
	{ JMP_THIS(0x543780); }

protected:
	explicit __forceinline IsometricTileClass(noinit_t) noexcept
		: ObjectClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:
	IsometricTileTypeClass* Type;
};
