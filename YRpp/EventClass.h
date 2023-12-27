#pragma once

#include <Helpers/CompileTime.h>

#include <TargetClass.h>
#include <Unsorted.h>

#pragma pack(push, 1)
union EventData
{
	EventData() { }

	struct nothing
	{
		char Data[0x68];
	} nothing;
	static_assert(sizeof(nothing) == 0x68);

	struct unkData
	{
		DWORD Checksum;
		WORD CommandCount;
		BYTE Delay;
		BYTE ExtraData[0x61];
	} unkData;
	static_assert(sizeof(unkData) == 0x68);

	struct Animation
	{
		int ID; // Anim ID
		int AnimOwner; // House ID
		CellStruct Location;
		BYTE ExtraData[0x5C];
	} Animation;
	static_assert(sizeof(Animation) == 0x68);

	struct FrameInfo
	{
		BYTE CRC;
		WORD CommandCount;
		BYTE Delay;
		BYTE ExtraData[0x64];
	} FrameInfo;
	static_assert(sizeof(FrameInfo) == 0x68);

	struct Target
	{
		TargetClass Whom;
		BYTE ExtraData[0x63];
	} Target;
	static_assert(sizeof(Target) == 0x68);

	struct MegaMission
	{
		TargetClass Whom;
		BYTE Mission; // Mission but should be byte
		char _gap_;
		TargetClass Target;
		TargetClass Destination;
		TargetClass Follow;
		bool IsPlanningEvent;
		BYTE ExtraData[0x51];
	} MegaMission;
	static_assert(sizeof(MegaMission) == 0x68);

	struct MegaMission_F
	{
		TargetClass Whom;
		BYTE Mission;
		TargetClass Target;
		TargetClass Destination;
		int Speed;
		int MaxSpeed;
		BYTE ExtraData[0x50];
	} MegaMission_F; // Seems unused in YR?
	static_assert(sizeof(MegaMission_F) == 0x68);

	struct Production
	{
		int RTTI_ID;
		int Heap_ID;
		int IsNaval;
		BYTE ExtraData[0x5C];
	} Production;
	static_assert(sizeof(Production) == 0x68);

	struct Unknown_LongLong
	{
		int Unknown_0; //4
		long long Data; //8
		int Unknown_C; //4
		BYTE ExtraData[0x58];
	} Unknown_LongLong;
	//static inline constexpr size_t TotalSizeOfAdditinalData_1 = sizeof(EventData::Unknown_LongLong);
	static_assert(sizeof(Unknown_LongLong) == 0x68);

	struct Unknown_Tuple
	{
		int Unknown_0;
		int Unknown_4;
		int Data;
		int Unknown_C;
		BYTE ExtraData[0x58];
	} Unknown_Tuple;
	static_assert(sizeof(Unknown_Tuple) == 0x68);

	struct Place
	{
		AbstractType RTTIType;
		int HeapID;
		int IsNaval;
		CellStruct Location;
		BYTE ExtraData[0x58];
	} Place;
	static_assert(sizeof(Place) == 0x68);

	struct SpecialPlace
	{
		int ID;
		CellStruct Location;
		BYTE ExtraData[0x60];
	} SpecialPlace;
	static_assert(sizeof(SpecialPlace) == 0x68);

	struct Specific
	{
		AbstractType RTTIType;
		int ID;
		BYTE ExtraData[0x60];
	} Specific;
	static_assert(sizeof(Specific) == 0x68);
};

class EventClass;

template<size_t Length>
struct EventList
{
public:
	int Count;
	int Head;
	int Tail;
	EventClass List[Length];
	int Timings[Length];
};

class EventClass
{
public:
	static constexpr reference<const char*, 0x82091C, 47> const EventNames{};
	static constexpr reference<unsigned char, 0x8208ECu, 36u> const EventLengthArr{};

	static constexpr reference<EventList<0x80>, 0xA802C8> OutList{};
	static constexpr reference<EventList<0x4000>, 0x8B41F8> DoList{};
	// If the event is a MegaMission, then add it to this list
	static constexpr reference<EventList<0x100>, 0xA83ED0> MegaMissionList{};

	// this points to CRCs from 0x100 last frames
	static constexpr reference<DWORD, 0xB04474, 256> const LatestFramesCRC{};
	static constexpr reference<DWORD, 0xAC51FC> const CurrentFrameCRC{};

	static bool AddEvent(EventClass& event)
	{
		if (OutList->Count >= 128)
			return false;

		OutList->List[OutList->Tail] = event;

		OutList->Timings[OutList->Tail] = static_cast<int>(Imports::TimeGetTime()());

		++OutList->Count;
		OutList->Tail = (OutList->Tail + 1) & 127;
		return true;
	}

	explicit EventClass()
	{
		memset(this, 0, sizeof(*this));
	}

	// Special
	explicit EventClass(int houseIndex, int id)
	{
		JMP_THIS(0x4C65A0);
	}

	// Target
	explicit EventClass(int houseIndex, NetworkEvents eventType, int id, int rtti)
	{
		JMP_THIS(0x4C65E0);
	}

	// Sellcell
	explicit EventClass(int houseIndex, NetworkEvents eventType, const CellStruct& cell)
	{
		JMP_THIS(0x4C6650);
	}

	// Archive & Planning_Connect
	explicit EventClass(int houseIndex, NetworkEvents eventType, TargetClass src, TargetClass dest)
	{
		JMP_THIS(0x4C6780);
	}

	// Anim
	explicit EventClass(int houseIndex, int anim_id, HouseClass* pHouse, const CellStruct& cell)
	{
		JMP_THIS(0x4C6800);
	}

	// MegaMission
	explicit EventClass(int houseIndex, TargetClass src, Mission mission, TargetClass target, TargetClass dest, TargetClass follow)
	{
		JMP_THIS(0x4C6860);
	}

	// MegaMission_F
	explicit EventClass(int houseIndex, TargetClass src, Mission mission, TargetClass target, TargetClass dest, SpeedType speed, int/*MPHType*/ maxSpeed)
	{
		JMP_THIS(0x4C68E0);
	}

	// Production
	explicit EventClass(int houseIndex, NetworkEvents eventType, int rtti_id, int heap_id, BOOL is_naval)
	{
		JMP_THIS(0x4C6970);
	}

	// Unknown_LongLong
	// explicit EventClass(int houseIndex, NetworkEvents eventType, int unknown_0, const int& unknown_c)
	// {
	//	 JMP_THIS(0x4C69E0);
	// }

	// Unknown_Tuple
	explicit EventClass(int houseIndex, NetworkEvents eventType, int unknown_0, int unknown_4, int& unknown_c)
	{
		JMP_THIS(0x4C6A60);
	}

	// Place
	explicit EventClass(int houseIndex, NetworkEvents eventType, AbstractType rttitype, int heapid, int is_naval, const CellStruct& cell)
	{
		JMP_THIS(0x4C6AE0);
	}

	// SpecialPlace
	explicit EventClass(int houseIndex, NetworkEvents eventType, int id, const CellStruct& cell)
	{
		JMP_THIS(0x4C6B60);
	}

	// Specific?, maybe int[2] otherwise
	explicit EventClass(int houseIndex, NetworkEvents eventType, AbstractType rttitype, int id)
	{
		JMP_THIS(0x4C6BE0);
	}

	// Address Change
	explicit EventClass(int houseIndex, void* /*IPAddressClass*/ ip, char unknown_0)
	{
		JMP_THIS(0x4C6C50);
	}

	explicit EventClass(const EventClass& another)
	{
		memcpy(this, &another, sizeof(*this));
	}

	EventClass& operator=(const EventClass& another)
	{
		if (this != &another)
			memcpy(this, &another, sizeof(*this));
		return *this;
	}

	NetworkEvents Type;
	bool IsExecuted;
	char HouseIndex; // '-1' stands for not a valid house
	unsigned int Frame; // 'Frame' is the frame that the command should execute on.

	EventData Data;

	bool operator==(const EventClass& q) const
	{
		return memcmp(this, &q, sizeof(q)) == 0;
	};
};
#pragma pack(pop)

static_assert(sizeof(EventClass) == 111);
