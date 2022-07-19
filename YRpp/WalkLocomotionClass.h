#pragma once

#include <LocomotionClass.h>

// Locomotor = {4A582744-9839-11d1-B709-00A024DDAFD1}
class WalkLocomotionClass : public LocomotionClass, public IPiggyback
{
public:

	//Constructor
	WalkLocomotionClass()
		: WalkLocomotionClass(noinit_t())
	{ JMP_THIS(0x75AA90); }

protected:
	explicit __forceinline WalkLocomotionClass(noinit_t)
		: LocomotionClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:
	CoordStruct Coord_1C;
	CoordStruct Coord_28;
	BYTE field_34;
	BYTE field_35;
	BYTE field_36;
	DWORD field_38;
};

static_assert(sizeof(WalkLocomotionClass) == 0x3C);
