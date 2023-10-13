/*
	RadSites
*/

#pragma once

#include <FileSystem.h>
#include <AbstractClass.h>

class ObjectClass;

class NOVTABLE AlphaShapeClass : public AbstractClass
{
public:
	static const AbstractType AbsID = AbstractType::AlphaShape;

	//Static
	static constexpr constant_ptr<DynamicVectorClass<AlphaShapeClass*>, 0x88A0F0u> const Array{};

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override JMP_STD(0x420D40);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x420DE0);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override JMP_STD(0x420E40);

	//Destructor
	virtual ~AlphaShapeClass() override RX;// JMP_THIS(0x421730);

	//AbstractClass
	virtual void PointerExpired(AbstractClass* pAbstract, bool removed) override JMP_THIS(0x420E70);
	virtual AbstractType WhatAmI() const override JMP_THIS(0x420D80);
	virtual int	Size() const override JMP_THIS(0x420D90);
	virtual void ComputeCRC(CRCEngine& crc) const override JMP_THIS(0x420DA0);

	//Constructor
	AlphaShapeClass(ObjectClass* pObject, int x, int y) noexcept
		: AlphaShapeClass(noinit_t())
	{ JMP_THIS(0x420960); }

protected:
	explicit __forceinline AlphaShapeClass(noinit_t) noexcept
		: AbstractClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	ObjectClass* AttachedTo;	//To which object is this AlphaShape attached?
	RectangleStruct Rect;
	SHPStruct* AlphaImage;
	bool IsObjectGone;	//Set if AttachedTo is NULL.
};
