#include "Commands.h"

#include "ObjectInfo.h"
#include "NextIdleHarvester.h"
#include "QuickSave.h"
#include "RepeatLastBuilding.h"
#include "RepeatLastCombat.h"
#include "DamageDisplay.h"
#include "AnimDisplay.h"
#include "CreateBuilding.h"
#include "CreateBuildingAuto.h"

int RepeatLastBuildingCommandClass::LastBuildingID = -1;
int RepeatLastCombatCommandClass::LastBuildingID = -1;

DEFINE_HOOK(0x533066, CommandClassCallback_Register, 0x6)
{
	// Load it after Ares'
	
	MakeCommand<ObjectInfoCommandClass>();
	MakeCommand<NextIdleHarvesterCommandClass>();
	MakeCommand<QuickSaveCommandClass>();
	MakeCommand<RepeatLastBuildingCommandClass>();
	MakeCommand<RepeatLastCombatCommandClass>();
	MakeCommand<DamageDisplayCommandClass>();
	MakeCommand<AnimDisplayCommandClass>();
	MakeCommand<CreateBuildingCommandClass>();
	MakeCommand<CreateBuildingAutoCommandClass>();

	return 0;
}