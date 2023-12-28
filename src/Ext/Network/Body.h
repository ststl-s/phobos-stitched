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
		AutoCreateBuilding = 0x88,
		UpdateGScreenCreate = 0x89,
		CheckSelectSW = 0x90,
		AttachEffect = 0x91,

		First = Convert,
		Last = AttachEffect
	};

	class Handlers
	{
	public:
		static void RaiseConvert(TechnoClass* pTechno);
		static void RespondToConvert(EventClass* pEvent);
		static void RaiseAutoRepair(HouseClass* pHouse);
		static void RespondToAutoRepair(EventClass* pEvent);
		static void RaiseSpreadAttack(TechnoClass* pTechno);
		static void RespondToSpreadAttack(EventClass* pEvent);
		static void RaiseToSelectSW(HouseClass* pHouse);
		static void RespondToSelectSW(EventClass* pEvent);
		static void RaiseCreateBuilding(CoordStruct coord);
		static void RespondToCreateBuilding(EventClass* pEvent);
		static void RaiseCreateBuildingAuto(TechnoClass* pTechno);
		static void RespondToCreateBuildingAuto(EventClass* pEvent);
		static void RaiseBackwarp(TechnoClass* pTechno);
		static void RespondToBackwarp(EventClass* pEvent);
		static void RaiseAutoCreateBuilding(CoordStruct coord);
		static void RespondToAutoCreateBuilding(EventClass* pEvent);
		static void RaiseUpdateGScreenCreate();
		static void RespondToUpdateGScreenCreate(EventClass* pEvent);
		static void RaiseToCheckSelectSW(HouseClass* pHouse);
		static void RespondToCheckSelectSW(EventClass* pEvent);
		static void RaiseAttachEffect(TechnoClass* pTechno);
		static void RespondToAttachEffect(EventClass* pEvent);
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

	struct CoordStructClick
	{
		TargetClass X;
		TargetClass Y;
		TargetClass Z;

		static inline constexpr size_t size() { return sizeof(CoordStructClick); }
	};
};
