/*
	Base class for all game objects with missions (yeah... not many).
*/

#pragma once

#include <ObjectClass.h>

class CCINIClass;

class MissionControlClass
{
	public:
		static MissionControlClass* Array()
			{ return reinterpret_cast<MissionControlClass*>(0xA8E3A8); }

		static const char** Names()
			{ return reinterpret_cast<const char**>(0x816CAC); }

		static MissionControlClass* __fastcall Find(const char* pName)
			{ JMP_STD(0x5B3910); }

		static Mission __fastcall FindIndex(const char* pName)
			{ JMP_STD(0x5B3910); }

		static const char* __fastcall FindName(const Mission& index)
			{ JMP_STD(0x5B3740); }

		MissionControlClass()
			{ JMP_THIS(0x5B3700); }

		const char* GetName()
			{ JMP_THIS(0x5B3740); }

		void LoadFromINI(CCINIClass* pINI)
			{ JMP_THIS(0x5B3760); }

		//Properties
		int ArrayIndex;
		bool NoThreat;
		bool Zombie;
		bool Recruitable;
		bool Paralyzed;
		bool Retaliate;
		bool Scatter;
		double Rate; //default 0.016
		double AARate; //default 0.016
};

class NOVTABLE MissionClass : public ObjectClass
{
public:
	//Destructor
	virtual ~MissionClass() JMP_THIS(0x5B3A60);

	//AbstractClass
	virtual void CalculateChecksum(Checksummer& checksum) const JMP_THIS(0x5B3970);
	virtual void Update() JMP_THIS(0x5B3060);

	//ObjectClass
	virtual Mission GetCurrentMission() const
	{
		/*if (this->CurrentMission == Mission::None)
			return this->QueuedMission;
		return this->CurrentMission;*/
		JMP_THIS(0x5B3040);
	}

	//MissionClass
	virtual bool QueueMission(Mission mission, bool start_mission) JMP_THIS(0x5B35E0);
	virtual bool NextMission() JMP_THIS(0x5B3570);
	virtual void ForceMission(Mission mission) JMP_THIS(0x5B2FD0);

	virtual void vt_entry_1F4(Mission mission) JMP_THIS(0x5B3650);
	virtual bool Mission_Revert() JMP_THIS(0x5B36B0);
	virtual bool MissionIsOverriden() const { return this->unknown_mission_B0 != Mission::None; }
	virtual bool ReadyToNextMission() const { return true; }

	virtual int Mission_Sleep() { return 450; }
	virtual int Mission_Harmless() { return 450; }
	virtual int Mission_Ambush() { return 450; }
	virtual int Mission_Attack() { return 450; }
	virtual int Mission_Capture() { return 450; }
	virtual int Mission_Eaten() { return 450; }
	virtual int Mission_Guard() { return 450; }
	virtual int Mission_AreaGuard() { return 450; }
	virtual int Mission_Harvest() { return 450; }
	virtual int Mission_Hunt() { return 450; }
	virtual int Mission_Move() { return 450; }
	virtual int Mission_Retreat() { return 450; }
	virtual int Mission_Return() { return 450; }
	virtual int Mission_Stop() { return 450; }
	virtual int Mission_Unload() { return 450; }
	virtual int Mission_Enter() { return 450; }
	virtual int Mission_Construction() { return 450; }
	virtual int Mission_Selling() { return 450; }
	virtual int Mission_Repair() { return 450; }
	virtual int Mission_Missile() { return 450; }
	virtual int Mission_Open() { return 450; }
	virtual int Mission_Rescue() { return 450; }
	virtual int Mission_Patrol() { return 450; }
	virtual int Mission_ParaDropApproach() { return 450; }
	virtual int Mission_ParaDropOverfly() { return 450; }
	virtual int Mission_Wait() { return 450; }
	virtual int Mission_SpyPlaneApproach() { return 450; }
	virtual int Mission_SpyPlaneOverfly() { return 450; }

	//Constructor
	MissionClass() noexcept
		: MissionClass(noinit_t())
	{ THISCALL(0x5B2DA0); }

protected:
	explicit __forceinline MissionClass(noinit_t) noexcept
		: ObjectClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	Mission  CurrentMission;
	Mission  unknown_mission_B0;
	Mission  QueuedMission;
	bool     unknown_bool_B8;
	PROTECTED_PROPERTY(BYTE, align_B9[3]);
	int      MissionStatus;
	int      CurrentMissionStartTime;	//in frames
	DWORD    unknown_C4;
	DECLARE_PROPERTY(TimerStruct, UpdateTimer);
};
