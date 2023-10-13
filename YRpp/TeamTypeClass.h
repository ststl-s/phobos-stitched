/*
	[TeamTypes]
*/

#pragma once

#include <AbstractTypeClass.h>
#include <ScriptTypeClass.h>
#include <TaskForceClass.h>

//forward declarations
class FootClass;
class TagClass;
class TeamClass;
class TechnoTypeClass;

class NOVTABLE TeamTypeClass : public AbstractTypeClass
{
public:
	static const AbstractType AbsID = AbstractType::TeamType;

	//Array
	ABSTRACTTYPE_ARRAY(TeamTypeClass, 0xA8ECA0u);

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override JMP_STD(0x6F1C40);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x6F1BB0);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override JMP_STD(0x6F1B90);

	//Destructor
	virtual ~TeamTypeClass() override RX;// JMP_THIS(0x6F20D0);

	//AbstractClass
	virtual void PointerExpired(AbstractClass* pAbstract, bool removed) override JMP_THIS(0x6F1030);
	virtual AbstractType WhatAmI() const override JMP_THIS(0x6F20A0);
	virtual int Size() const override JMP_THIS(0x6F20B0);
	virtual void ComputeCRC(CRCEngine& crc) const override JMP_THIS(0x6F1C80);
	virtual int GetArrayIndex() const override JMP_THIS(0x6F20C0);

	//AbstractTypeClass
	virtual bool LoadFromINI(CCINIClass* pINI) override JMP_THIS(0x6F1090);
	virtual bool SaveToINI(CCINIClass* pINI) override JMP_THIS(0x6F1550);

	//static
	static bool __fastcall LoadFromINIList(CCINIClass *pINI, BOOL global)
	{ JMP_STD(0x6F19B0); }

	static int __fastcall SaveToINILise(CCINIClass* pINI, BOOL global)
	{ JMP_STD(0x6F1AB0); }

	static void ProcessAllTaskforces()
	{ JMP_STD(0x6F2040); }

	//non-virtual
	TeamClass * CreateTeam(HouseClass *pHouse)
	{ JMP_THIS(0x6F09C0); }

	void DestroyAllInstances()
	{ JMP_THIS(0x6F0A70); }

	int GetGroup() const
	{ JMP_THIS(0x6F1870); }

	CellStruct* GetWaypoint(CellStruct *buffer) const
	{ JMP_THIS(0x6F18A0); }

	CellStruct* GetTransportWaypoint(CellStruct *buffer) const
	{ JMP_THIS(0x6F18E0); }

	bool CanRecruitUnit(FootClass* pUnit, HouseClass* pOwner) const
	{ JMP_THIS(0x6F1E20); }

	void FlashAllInstances(int Duration)
	{ JMP_THIS(0x6F1F30); }

	TeamClass * FindFirstInstance() const
	{ JMP_THIS(0x6F1F70); }

	void ProcessTaskForce()
	{ JMP_THIS(0x6F1FA0); }

	HouseClass* GetHouse() const
	{ JMP_THIS(0x6F2070); }

	//Constructor
	TeamTypeClass(const char* pID) noexcept
		: TeamTypeClass(noinit_t())
	{ JMP_THIS(0x6F06E0); }

protected:
	explicit __forceinline TeamTypeClass(noinit_t) noexcept
		: AbstractTypeClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	int      ArrayIndex;
	int      Group;
	int      VeteranLevel;
	bool     Loadable;
	bool     Full;
	bool     Annoyance;
	bool     GuardSlower;
	bool     Recruiter;
	bool     Autocreate;
	bool     Prebuild;
	bool     Reinforce;
	bool     Whiner;
	bool     Aggressive;
	bool     LooseRecruit;
	bool     Suicide;
	bool     Droppod;
	bool     UseTransportOrigin;
	bool     DropshipLoadout;
	bool     OnTransOnly;
	int      Priority;
	int      Max;
	int      field_BC;
	int      MindControlDecision;
	HouseClass *     Owner;
	int      idxHouse; // idx for MP
	int      TechLevel;
	TagClass* Tag;
	int      Waypoint;
	int      TransportWaypoint;
	int      cntInstances;
	ScriptTypeClass*  ScriptType;
	TaskForceClass*   TaskForce;
	int      IsGlobal;
	int      field_EC;
	bool     field_F0;
	bool     field_F1;
	bool     AvoidThreats;
	bool     IonImmune;
	bool     TransportsReturnOnUnload;
	bool     AreTeamMembersRecruitable;
	bool     IsBaseDefense;
	bool     OnlyTargetHouseEnemy;

};
