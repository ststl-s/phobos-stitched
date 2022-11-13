#pragma once

#include <GeneralDefinitions.h>
#include <MPGameModeClass.h>
#include <GameModeOptionsClass.h>
#include <IPX.h>

#include <Helpers/CompileTime.h>

struct SessionOptionsClass
{
	int MPGameMode;
	int ScenIndex;
	int GameSpeed;
	int Credits;
	int UnitCount;
	bool ShortGame;
	bool SuperWeaponsAllowed;
	bool BuildOffAlly;
	bool MCVRepacks;
	bool CratesAppear;
	Vector3D<int> SlotData[8];
};

#pragma pack(push, 1)
struct NodeNameType
{
	static constexpr constant_ptr<DynamicVectorClass<NodeNameType*>, 0xA8DA74> const Array {};

	wchar_t Name[20];
	sockaddr_in Address;
	char Serial[19];
	int Country;
	int InitialCountry;
	int Color;
	int InitialColor;
	int StartPoint;
	int InitialStartPoint;
	int Team;
	int InitialTeam;
	DWORD SpectatorFlag; // 0xFFFFFFFF if Spectator
	int HouseIndex;
	int Time;
	DWORD unknown_int_77;
	int Clan;
	DWORD unknown_int_7F;
	BYTE unknown_byte_83;
	BYTE unknown_byte_84;
};
#pragma pack(pop)
static_assert(sizeof(NodeNameType) == 0x85);

#pragma pack(push, 4)
class SessionClass
{
public:
	static constexpr reference<SessionClass, 0xA8B238u> const Instance {};

	static bool IsCampaign()
	{
		return Instance->GameMode == GameMode::Campaign;
	}

	static bool IsSingleplayer()
	{
		return Instance->GameMode == GameMode::Campaign
			|| Instance->GameMode == GameMode::Skirmish;
	}

	// non-virtual
	void ReadScenarioDescriptions()
	{ JMP_THIS(0x699980) }

	bool CreateConnections()
	{ JMP_THIS(0x697B70) }

	GameMode GameMode;
	MPGameModeClass* MPGameMode;
	DWORD unknown_08;
	DWORD unknown_0C;
	DWORD unknown_10;
	DWORD unknown_14;
	GameModeOptionsClass Config;
	DWORD UniqueID;
	char Handle[20];
	int PlayerColor;
	DWORD unknown_160;
	DWORD unknown_164;
	DWORD unknown_168;
	DWORD unknown_16C;
	DWORD unknown_170;
	int idxSide;
	int idxSide2;
	int Color;
	int Color2;
	int Side;
	int Side2;
	SessionOptionsClass Skirmish;
	SessionOptionsClass LAN;
	SessionOptionsClass WOL;
	BOOL MultiplayerObserver;
	DWORD Unknown_304;
	bool WOLLimitResolution;
	int LastNickSlot;
	int MPlayerMax;
	int MPlayerCount;
	int MaxAhead;
	int FrameSendRate;
	int DesiredFrameRate;
	int ProcessTimer;
	int ProcessTicks;
	int ProcessFrames;
	int MaxMaxAhead;
	int PrecalcMaxAhead;
	int PrecalcDesiredFrameRate;
	struct
	{
		char Name[64];
		int MaxRoundTrip;
		int Resends;
		int Lost;
		int PercentLost;
		int MaxAvgRoundTrip;
		int FrameSyncStalls;
		int CommandCoundStalls;
		IPXAddressClass Address;
	} MPStats[8];
	bool EnableMultiplayerDebug;
	bool DrawMPDebugStats;
	char field_67E;
	char field_67F;
	int LoadGame;
	int SaveGame;
	char field_688;
	bool SawCompletion;
	bool OutOfSync;
	char field_68B;
	int GameVersion;
	DynamicVectorClass<class MultiMission*> MultiMission;
	char ScenarioFilename[0x202]; // 0x6A8
	PROTECTED_PROPERTY(BYTE, unknown_8AA[0x1F62]);
	DynamicVectorClass<NodeNameType*> unknown_vector_280C;
	DynamicVectorClass<NodeNameType*> unknown_vector_2824;
	DynamicVectorClass<NodeNameType*> StartSpots;
	PROTECTED_PROPERTY(DWORD, unknown_2854[0x221]);
	bool CurrentlyInGame; // at least used for deciding dialog backgrounds
};
#pragma pack(pop)

static_assert(sizeof(SessionClass) == 0x30DC);
