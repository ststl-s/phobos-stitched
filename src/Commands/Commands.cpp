#include "Commands.h"

#include "Dummy.h"
#include "ObjectInfo.h"
#include "NextIdleHarvester.h"
#include "QuickSave.h"
#include "RepeatLastBuilding.h"
#include "RepeatLastCombat.h"
#include "DamageDisplay.h"
#include "AnimDisplay.h"
#include "CreateBuilding.h"

int RepeatLastBuildingCommandClass::LastBuildingID = -1;
int RepeatLastCombatCommandClass::LastBuildingID = -1;

DEFINE_HOOK(0x533066, CommandClassCallback_Register, 0x6)
{
	// Load it after Ares'
	// Debug::Log("[Phobos] CommandClassCallback_Register Called!\n");

	//MakeCommand<DummyCommandClass>();
	MakeCommand<ObjectInfoCommandClass>();
	MakeCommand<NextIdleHarvesterCommandClass>();
	MakeCommand<QuickSaveCommandClass>();
	MakeCommand<RepeatLastBuildingCommandClass>();
	MakeCommand<RepeatLastCombatCommandClass>();
	MakeCommand<DamageDisplayCommandClass>();
	MakeCommand<AnimDisplayCommandClass>();
	MakeCommand<CreateBuildingCommandClass>();

	return 0;
}