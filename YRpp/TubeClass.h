#pragma once

#include <AbstractClass.h>

class NOVTABLE TubeClass : AbstractClass
{
public:
	static constexpr constant_ptr<DynamicVectorClass<TubeClass*>, 0x8B4138u> const Array {};

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override JMP_STD(0x7286D0);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x7281A0);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override JMP_STD(0x7281E0);

	//Destructor
	virtual ~TubeClass() override RX;//JMP_THIS(0x728710);

	//AbstractClass
	virtual AbstractType WhatAmI() const override JMP_THIS(0x7286C0);
	virtual int Size() const override JMP_THIS(0x7286B0);
	virtual void ComputeCRC(CRCEngine& crc) const override JMP_THIS(0x728630);

	//Constructor
	TubeClass(CellStruct& cell, DWORD dwUnk) noexcept
		: AbstractClass(noinit_t())
	{ JMP_THIS(0x727FD0); }

protected:
	explicit __forceinline TubeClass(noinit_t) noexcept
		: AbstractClass(noinit_t())
	{ }

public:
	CellStruct unknown_24;
	CellStruct unknown_28;
	DWORD unknown_2C;
	BYTE unknown30[0x190];
	DWORD unknown_1C0;
};

static_assert(sizeof(TubeClass) == 0x1C4);
