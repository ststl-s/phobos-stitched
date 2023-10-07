#pragma once

#include <AbstractClass.h>

//forward declarations
class AircraftTypeClass;
class ObjectClass;
class TechnoClass;
class FootClass;

//The AirstrikeClass handles the airstrikes Boris calls in.
class NOVTABLE AirstrikeClass : public AbstractClass
{
public:
	static const AbstractType AbsID = AbstractType::Airstrike;

	static constexpr constant_ptr<DynamicVectorClass<AirstrikeClass*>, 0x889FB8u> const Array {};

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override JMP_STD(0x41D7A0);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x41D6F0);
	virtual HRESULT __stdcall Save(IStream* pStm,BOOL fClearDirty) override JMP_STD(0x41D780);

	//Destructor
	virtual ~AirstrikeClass() override JMP_THIS(0x41DD50);

	//AbstractClass
	virtual AbstractType WhatAmI() const override JMP_THIS(0x41DD40);
	virtual int	Size() const override JMP_THIS(0x41DD30);
	virtual void ComputeCRC(CRCEngine& crc) const override JMP_THIS(0x41D6E0);
	virtual void Update() override JMP_THIS(0x41DC50);

	//non-virtual
	void StartMission(ObjectClass* pTarget)
	{ JMP_THIS(0x41D830); }

	//Constructor
	AirstrikeClass(TechnoClass* pOwner) noexcept
		: AirstrikeClass(noinit_t())
	{ JMP_THIS(0x41D380); }

protected:
	explicit __forceinline AirstrikeClass(noinit_t) noexcept
		: AbstractClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	int AirstrikeTeam;			//As in the INI files.
	int EliteAirstrikeTeam;	//As in the INI files.
	int AirstrikeTeamTypeIndex;	//As in the INI files.
	int EliteAirstrikeTeamTypeIndex;	//As in the INI files.
	DWORD unknown_34;
	DWORD unknown_38;	//unused?
	bool IsOnMission;	//Is the Aircraft on its way?
	bool unknown_bool_3D;
	DWORD TeamDissolveFrame;	//when was the last time this team was invoked and subsequently dissolved
	int AirstrikeRechargeTime;	//As in the INI files.
	int EliteAirstrikeRechargeTime;	//As in the INI files.
	TechnoClass* Owner;		//The unit that called the Airstrike (usually Boris).
	ObjectClass* Target;	//The Airstrike's target.
	AircraftTypeClass* AirstrikeTeamType;	//As in the INI files.
	AircraftTypeClass* EliteAirstrikeTeamType;	//As in the INI files.
	FootClass* FirstObject;
};
