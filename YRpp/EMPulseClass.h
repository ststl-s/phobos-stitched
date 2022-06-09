/*
	EMP - no, you're NOT seeing things :P
*/

#pragma once

#include <AbstractClass.h>

class NOVTABLE EMPulseClass : public AbstractClass
{
public:
	static const AbstractType AbsID = AbstractType::EMPulse;

	//Static
	static constexpr constant_ptr<DynamicVectorClass<EMPulseClass*>, 0x8A3870u> const Array{};

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) JMP_STD(0x4C59F0);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) JMP_STD(0x4C5A30);
	virtual HRESULT __stdcall Save(IStream* pStm,BOOL fClearDirty) JMP_STD(0x4C5A80);

	//Destructor
	virtual ~EMPulseClass() JMP_THIS(0x4C5AC0);

	//AbstractClass
	virtual AbstractType WhatAmI() const { return AbstractType::EMPulse; }
	virtual int Size() const { return 0x34; }
	virtual void CalculateChecksum(Checksummer& checksum) const JMP_THIS(0x4C59A0);

	//Constructor
	EMPulseClass(CellStruct dwCrd, int nSpread, int nDuration,
		TechnoClass* pGenerator) noexcept : EMPulseClass(noinit_t())
	{ JMP_THIS(0x4C52B0); }

protected:
	explicit __forceinline EMPulseClass(noinit_t) noexcept
		: AbstractClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	CellStruct BaseCoords;
	int Spread;
	int CreationTime;	//frame in which this EMP got created
	int Duration;
};
