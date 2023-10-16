/*
	I have not the slightest idea what this is good for...
*/

#pragma once

#include <AbstractClass.h>

class NOVTABLE NeuronClass : public AbstractClass
{
public:
	static const AbstractType AbsID = AbstractType::Neuron;

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override JMP_STD(0x43A500);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x43A540);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override JMP_STD(0x43A5B0);

	//Destructor
	virtual ~NeuronClass() RX; //JMP_THIS(0x43A9C0);

	//AbstractClass
	virtual AbstractType WhatAmI() const override JMP_THIS(0x43A9A0);
	virtual int Size() const override JMP_THIS(0x43A9B0);
	virtual void ComputeCRC(CRCEngine& crc) const override JMP_THIS(0x43A5D0);

	//Constructor
	NeuronClass() noexcept
		: NeuronClass(noinit_t())
	{ JMP_THIS(0x43A350); }

protected:
	explicit __forceinline NeuronClass(noinit_t) noexcept
		: AbstractClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	void* unknown_ptr_24;
	void* unknown_ptr_28;
	void* unknown_ptr_2C;
	int unknown_time_30;
	int unknown_34;
};

//Even more questions marks on the use of this... >.<
class BrainClass
{
public:
	virtual ~BrainClass() RX; //JMP_THIS(0x43AA90)

	//no, that's not constructor
	//BrainClass()
		//{ THISCALL(0x43A600); }

	//Properties
	DynamicVectorClass<NeuronClass*> Neurons;	//???
};
