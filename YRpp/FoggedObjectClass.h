#pragma once

#include <AbstractClass.h>

class NOVTABLE FoggedObjectClass : AbstractClass
{
public:
	static constexpr constant_ptr<DynamicVectorClass<FoggedObjectClass*>, 0x8B3D10u> const Array {};

	struct DrawRecord
	{

	};

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override JMP_STD(0x4D27D0);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x4D2510);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override JMP_STD(0x4D24A0);

	//Destructor
	virtual ~FoggedObjectClass() override RX;//JMP_THIS(0x4D2910);

	//AbstractClass
	virtual AbstractType WhatAmI() const override JMP_THIS(0x4D27B0);
	virtual int Size() const override JMP_THIS(0x4D27C0);
	virtual void ComputeCRC(CRCEngine& crc) const override JMP_THIS(0x4D2810);

	//FoggedObjectClass
	virtual CellStruct* vt_entry_60(CellStruct& cell) const JMP_THIS(0x4D28D0);

	//Constructor
	FoggedObjectClass() noexcept
		: AbstractClass(noinit_t())
	{ JMP_THIS(0x4D08B0); }

protected:
	explicit __forceinline FoggedObjectClass(noinit_t) noexcept
		: AbstractClass(noinit_t())
	{ }

public:
	int unknown_24;
	DWORD unknown_28;
	DWORD unknown_2C;
	DWORD unknown_30;
	CoordStruct unknown_34;
	RectangleStruct unknown_40;
	DWORD unknown_50;
	int unknown_54;
	DWORD unknown_58;
	DynamicVectorClass<DrawRecord> unknown_5C;
	bool unknown_74;
};

static_assert(sizeof(FoggedObjectClass) == 0x78);
