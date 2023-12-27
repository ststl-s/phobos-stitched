#include "Commands.h"

#include "ObjectInfo.h"
#include "NextIdleHarvester.h"
#include "QuickSave.h"
#include "RepeatLastBuilding.h"
#include "RepeatLastCombat.h"
#include "DamageDisplay.h"
#include "FrameByFrame.h"
#include "FrameStep.h"
#include "AnimDisplay.h"
#include "CreateBuilding.h"
#include "CreateBuildingAuto.h"
#include "KillMsgDisplay.h"
#include "SelectSW.h"
#include "AutoRepair.h"
#include "SpreadAttack.h"
#include "ToggleDigitalDisplay.h"
#include "NextSWBarPage.h"
#include "LastSWBarPage.h"
#include "HideSWBar.h"
#include "Convert.h"
#include "Backwarp.h"

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
	MakeCommand<KillMsgDisplayCommandClass>();
	MakeCommand<SelectSWCommandClass>();
	MakeCommand<AutoRepairCommandClass>();
	MakeCommand<SpreadAttackCommandClass>();
	MakeCommand<ToggleDigitalDisplayCommandClass>();
	MakeCommand<NextSWBarPageCommandClass>();
	MakeCommand<LastSWBarPageCommandClass>();
	MakeCommand<HideSWBarCommandClass>();
	MakeCommand<ConvertCommandClass>();
	MakeCommand<BackwarpCommandClass>();

	MakeCommand<FrameByFrameCommandClass>();
	MakeCommand<FrameStepCommandClass<1>>(); // Single step in
	MakeCommand<FrameStepCommandClass<5>>(); // Speed 1
	MakeCommand<FrameStepCommandClass<10>>(); // Speed 2
	MakeCommand<FrameStepCommandClass<15>>(); // Speed 3
	MakeCommand<FrameStepCommandClass<30>>(); // Speed 4
	MakeCommand<FrameStepCommandClass<60>>(); // Speed 5

	return 0;
}
