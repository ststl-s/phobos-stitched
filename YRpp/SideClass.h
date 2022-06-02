/*
	Sides
*/

#pragma once

#include <AbstractTypeClass.h>

class NOVTABLE SideClass : public AbstractTypeClass
{
public:
	static const AbstractType AbsID = AbstractType::Side;

	//Array
	ABSTRACTTYPE_ARRAY(SideClass, 0x8B4120u);

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) JMP_STD(0x6A4740);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) JMP_STD(0x6A4780);
	virtual HRESULT __stdcall Save(IStream* pStm,BOOL fClearDirty) JMP_STD(0x6A48A0);

	//Destructor
	virtual ~SideClass() JMP_THIS(0x6A4930);

	//AbstractClass
	virtual AbstractType WhatAmI() const { return AbstractType::Side; }
	virtual int Size() const { return 0xB4; }
	virtual void CalculateChecksum(Checksummer& checksum) const JMP_THIS(0x6A4710);

	//Constructor
	SideClass(const char* pID) noexcept
		: SideClass(noinit_t())
	{ JMP_THIS(0x6A4550); }

protected:
	explicit __forceinline SideClass(noinit_t) noexcept
		: AbstractTypeClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	TypeList<int> HouseTypes;	//indices!

};
