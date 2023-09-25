/*
	TaskForces as in the AI inis
*/

#pragma once

#include <AbstractTypeClass.h>

//forward declarations
class TechnoTypeClass;

struct TaskForceEntryStruct
{
	int Amount;
	TechnoTypeClass* Type;

	const char* GetID()
	{ JMP_THIS(0x4C4F90); }
};

class NOVTABLE TaskForceClass : public AbstractTypeClass
{
public:
	static const AbstractType AbsID = AbstractType::TaskForce;

	//Array
	ABSTRACTTYPE_ARRAY(TaskForceClass, 0xA8E8D0u);

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override JMP_STD(0x6E8710);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x6E86A0);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override JMP_STD(0x6E8680);

	//Destructor
	virtual ~TaskForceClass() override JMP_THIS(0x6E87F0);

	//AbstractClass
	virtual AbstractType WhatAmI() const override JMP_THIS(0x6E87D0);
	virtual int Size() const override JMP_THIS(0x6E86E0);
	virtual void ComputeCRC(CRCEngine& crc) const override JMP_THIS(0x6E8750);

	//AbstractTypeClass
	virtual bool LoadFromINI(CCINIClass* pINI) override JMP_THIS(0x6E8420);
	virtual bool SaveToINI(CCINIClass* pINI) override JMP_THIS(0x6E8510);

	//Constructor
	TaskForceClass(const char* pID) noexcept
		: TaskForceClass(noinit_t())
	{ JMP_THIS(0x6E7E80); }

protected:
	explicit __forceinline TaskForceClass(noinit_t) noexcept
		: AbstractTypeClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	int     Group;
	int     CountEntries;
	bool    IsGlobal;
	TaskForceEntryStruct Entries [0x6];
};
