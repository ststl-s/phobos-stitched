#include "Body.h"

#include <Utilities/Macro.h>

//UnitClass::ReceiveCommand_RequestLoading Check Moving
DEFINE_JUMP(LJMP, 0x7377EC, 0x73781E);

DEFINE_HOOK(0x51A258, InfantryClass_UpdatePosition_Enter, 0x5)
{
	GET(TechnoClass*, pTarget, EDI);
	GET(AircraftClass*, pTargetAircraft, EBX);
	GET(InfantryClass*, pThis, ESI);

	if (pThis->GetCurrentMission() == Mission::Enter
		&& pTarget != nullptr
		&& (pThis->Destination == pTarget
			|| pThis->Target == pTarget
			|| pTargetAircraft && pTarget == pTargetAircraft))
	{
	}
}
