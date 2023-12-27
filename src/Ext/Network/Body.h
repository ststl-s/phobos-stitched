#pragma once

#include <EventClass.h>

class ExtraPhobosNetEvent
{
public:
	enum class Events : unsigned char
	{
		Convert = 0x81,
		AutoRepair = 0x82,
		SpreadAttack = 0x83,
		ToSelectSW = 0x84,
		CreateBuilding = 0x85,
		CreateBuildingAuto = 0x86,
		Backwarp = 0x87,

		First = Convert,
		Last = Backwarp
	};

	class Handlers
	{
	public:
		static void RaiseConvert(TechnoClass* pTechno);
		static void RespondToConvert(EventClass* pEvent);
		static void RaiseAutoRepair(TechnoClass* pTechno);
		static void RespondToAutoRepair(EventClass* pEvent);
		static void RaiseSpreadAttack(TechnoClass* pTechno);
		static void RespondToSpreadAttack(EventClass* pEvent);
		static void RaiseToSelectSW(TechnoClass* pTechno);
		static void RespondToSelectSW(EventClass* pEvent);
		static void RaiseCreateBuilding(TechnoClass* pTechno);
		static void RespondToCreateBuilding(EventClass* pEvent);
		static void RaiseCreateBuildingAuto(TechnoClass* pTechno);
		static void RespondToCreateBuildingAuto(EventClass* pEvent);
		static void RaiseBackwarp(TechnoClass* pTechno);
		static void RespondToBackwarp(EventClass* pEvent);
	};

	struct SpecialClick1
	{
		TargetClass SourceCell;

		static inline constexpr size_t size() { return sizeof(SpecialClick1); }
	};

	struct SpecialClick2
	{
		TargetClass SourceCell;
		TargetClass TargetCell;

		static inline constexpr size_t size() { return sizeof(SpecialClick2); }
	};
};
