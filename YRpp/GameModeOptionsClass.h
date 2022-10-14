#pragma once

#include <wchar.h>

#include <Helpers/CompileTime.h>

class StartingSlotClass;

struct AISlotsStruct
{
	int AIDifficulties[8];
	StartingSlotClass* StartingSpots[8];
	int Colours[8];
	int Starts[8];
	int Teams[8];
};

class GameModeOptionsClass
{
public:
	// this is the same as SessionClass::Instance->Config
	static constexpr reference<GameModeOptionsClass, 0xA8B250u> const Instance{};

	int MPModeIndex;
	int ScenarioIndex;
	bool Bases;
	PROTECTED_PROPERTY(BYTE, align_9[3]);
	int Money;
	bool BridgeDestruction;
	bool Crates;
	bool ShortGame;
	bool SWAllowed;
	bool BuildOffAlly;
	PROTECTED_PROPERTY(BYTE, align_15[3]);
	int GameSpeed;
	bool MultiEngineer;
	PROTECTED_PROPERTY(BYTE, align_1D[3]);
	int UnitCount;
	int AIPlayers;
	int AIDifficulty;
	AISlotsStruct AISlots;
	bool AlliesAllowed;
	bool HarvesterTruce;
	bool CTF;
	bool FogOfWar;
	bool MCVRedeploy;
	PROTECTED_PROPERTY(BYTE, align_D1);
	wchar_t MapDescription[45];
};
