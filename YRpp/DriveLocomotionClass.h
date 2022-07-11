//Locomotor = {4A582741-9839-11d1-B709-00A024DDAFD1}

#pragma once

#include <LocomotionClass.h>

class NOVTABLE DriveLocomotionClass : public LocomotionClass, public IPiggyback
{
public:
	// TODO stub virtuals implementations

	//Destructor
	virtual ~DriveLocomotionClass() RX;

	//Constructor
	DriveLocomotionClass()
		: DriveLocomotionClass(noinit_t())
	{ JMP_THIS(0x4AF540); }

protected:
	explicit __forceinline DriveLocomotionClass(noinit_t)
		: LocomotionClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	DWORD Ramp1;
	DWORD Ramp2;
	RepeatableTimerStruct SlopeTimer;
	CoordStruct Destination;
	CoordStruct HeadToCoord;
	int SpeedAccum;
	double movementspeed_50;
	DWORD TrackNumber;
	int TrackIndex;
	bool IsOnShortTrack;
	BYTE field_61;
	bool IsRotating;
	bool IsDriving;
	bool can_crush_64;
	bool IsLocked;
	AbstractClass *Raider;
	int field_6C;
};

static_assert(sizeof(DriveLocomotionClass) == 0x70);
