/*
	Actual AI Team Scripts
*/

#pragma once

#include <AbstractClass.h>

//forward declarations
#include <ScriptTypeClass.h>

class NOVTABLE ScriptClass : public AbstractClass
{
public:
	static const AbstractType AbsID = AbstractType::Script;

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override JMP_STD(0x6915F0);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x691630);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override JMP_STD(0x691690);

	//Destructor
	virtual ~ScriptClass() override JMP_THIS(0x691EE0);

	//AbstractClass
	virtual AbstractType WhatAmI() const override { return AbstractType::Script; }
	virtual int Size() const override { return 0x30; }
	virtual void ComputeCRC(CRCEngine& crc) const override JMP_THIS(0x6914E0);

	ScriptActionNode* GetCurrentAction(ScriptActionNode *buffer) const
	{ JMP_THIS(0x691500); }

	ScriptActionNode* GetNextAction(ScriptActionNode *buffer) const
	{ JMP_THIS(0x691540); }

	bool ClearMission()
	{ JMP_THIS(0x691590); }

	bool SetMission(int nLine)
	{ JMP_THIS(0x6915A0); }

	bool NextMission()
	{ ++this->CurrentMission; return this->HasNextMission(); }

	bool HasNextMission() const
	{ JMP_THIS(0x6915B0); }

	bool IsMissionValid() const
	{ JMP_THIS(0x6915D0); }

	//Constructor
	ScriptClass(ScriptTypeClass* pType) noexcept
		: AbstractClass(noinit_t())
	{ JMP_THIS(0x6913C0); }

protected:
	explicit __forceinline ScriptClass(noinit_t) noexcept
		: AbstractClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	ScriptTypeClass * Type;
	int field_28;
	int CurrentMission;
};
