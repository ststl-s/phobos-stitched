#include <Helpers/Macro.h>
#include <LoadOptionsClass.h>
#include <Phobos.h>
#include <Utilities/Debug.h>

DEFINE_HOOK(0x67D04E, Game_Save_SavegameInformation, 0x7)
{
	REF_STACK(SavegameInformation, Info, STACK_OFFS(0x4A4, 0x3F4));
	Info.Version = Info.Version + SAVEGAME_ID;
	return 0;
}

DEFINE_HOOK(0x559F27, LoadOptionsClass_GetFileInfo, 0xA)
{
	REF_STACK(SavegameInformation, Info, STACK_OFFS(0x400, 0x3F4));
	Info.Version = Info.Version - SAVEGAME_ID;
	return 0;
}

// Ares saves its things at the end of the save
// Phobos will save the things at the beginning of the save
// Considering how DTA gets the scenario name, I decided to save it after Rules - secsome
DEFINE_HOOK(0x67D32C, SaveGame_Phobos, 0x5)
{
	GET(IStream*, pStm, ESI);
	//UNREFERENCED_PARAMETER(pStm);
	Phobos::SaveGameData(pStm);
	return 0;
}

DEFINE_HOOK(0x67D1B4, SaveGame_After, 0x6)
{
	GET_STACK(IStream*, pStm, 0x1C);
	Phobos::SaveGameDataAfter(pStm);
	return 0;
}

DEFINE_HOOK(0x67E826, LoadGame_Phobos, 0x6)
{
	PointerMapper::Map.clear();
	GET(IStream*, pStm, ESI);
	//UNREFERENCED_PARAMETER(pStm);
	Phobos::LoadGameData(pStm);
	return 0;
}

/*
if you have something use TriggerClass*, TechnoTypeClass*,
TechnoClass*, you'd better put it into Phobos.Ext.cpp->ProcessAfter
*/
DEFINE_HOOK(0x67E65E, LoadGame_AfterAll, 0x6)
{
	GET_STACK(IStream*, pStm, 0x10);
	Phobos::LoadGameDataAfter(pStm);
	return 0;
}