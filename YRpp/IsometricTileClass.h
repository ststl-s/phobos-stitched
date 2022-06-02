#pragma once

#include <ObjectClass.h>

//forward declarations
class IsometricTileTypeClass;

class NOVTABLE IsometricTileClass : public ObjectClass
{
public:
	static const AbstractType AbsID = AbstractType::Isotile;

	//Array
	static constexpr constant_ptr<DynamicVectorClass<IsometricTileClass*>, 0x87F750u> Array{};

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) JMP_THIS(0x543AB0);

	virtual HRESULT __stdcall Load(IStream* pStm) JMP_STD(0x543990);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) JMP_STD(0x5439F0);

	//AbstractClass
	virtual AbstractType WhatAmI() const { return AbstractType::Isotile; }
	virtual int Size() const { return 0xB0; }

	//ObjectClass
	virtual ObjectTypeClass* GetType() const { return this->Type; }
	virtual bool Limbo() JMP_THIS(0x543A40);
	virtual bool Unlimbo(const CoordStruct& Crd, Direction::Value dFaceDir) JMP_THIS(0x543A10);
	virtual void Draw(Point2D* pLocation, RectangleStruct* pBounds) const {}
	virtual bool UpdatePlacement(PlacementType value) JMP_THIS(0x543330);

	//Destructor
	virtual ~IsometricTileClass() JMP_THIS(0x543B10);

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
