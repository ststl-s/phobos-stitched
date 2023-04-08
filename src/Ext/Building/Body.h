#pragma once
#include <BuildingClass.h>
#include <HouseClass.h>
#include <TiberiumClass.h>
#include <FactoryClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/Building/Body.h>

class BuildingExt
{
public:
	using base_type = BuildingClass;

	class ExtData final : public Extension<BuildingClass>
	{
	public:
		BuildingTypeExt::ExtData* TypeExtData = nullptr;
		bool DeployedTechno = false;
		int LimboID = 0;
		int GrindingWeapon_LastFiredFrame = 0;
		BuildingClass* CurrentAirFactory = nullptr;
		int AccumulatedGrindingRefund = 0;
		int OfflineTimer = -1;
		CDTimerClass SabotageTimer;
		CDTimerClass SellTimer;
		CDTimerClass CaptureTimer;
		int CaptureCount = 0;
		HouseClass* CaptureOwner = nullptr;
		HouseClass* OriginalOwner = nullptr;
		bool SellingForbidden = false;
		ValueableVector<HouseClass*> RevealSightHouses;
		ValueableVector<int> RevealSightRanges;
		ValueableVector<CDTimerClass> RevealSightTimers;
		ValueableVector<bool> RevealSightPermanents;

		ExtData(BuildingClass* OwnerObject) : Extension<BuildingClass>(OwnerObject)
			, TypeExtData { nullptr }
			, DeployedTechno { false }
			, LimboID { -1 }
			, GrindingWeapon_LastFiredFrame { 0 }
			, CurrentAirFactory { nullptr }
			, AccumulatedGrindingRefund { 0 }
		{ }

		void DisplayGrinderRefund();
		void ApplyPoweredKillSpawns();
		bool HasSuperWeapon(int index, bool withUpgrades) const;
		void BuildingPowered();
		void SabotageBuilding();
		void SellBuilding();
		void CaptureBuilding();
		void ForbidSell();
		void RevealSight();

		virtual ~ExtData() = default;

		// virtual void LoadFromINIFile(CCINIClass* pINI) override;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override
		{
			AnnounceInvalidPointer(CurrentAirFactory, ptr);
		}

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<BuildingExt>
	{
	public:
		ExtContainer();
		~ExtContainer();

		virtual bool InvalidateExtDataIgnorable(void* const ptr) const override
		{
			auto const abs = static_cast<AbstractClass*>(ptr)->WhatAmI();
			switch (abs)
			{
			case AbstractType::Building:
				return false;
			default:
				return true;
			}
		}
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static void UpdatePrimaryFactoryAI(BuildingClass* pThis);
	static int CountOccupiedDocks(BuildingClass* pBuilding);
	static bool HasFreeDocks(BuildingClass* pBuilding);
	static bool CanGrindTechno(BuildingClass* pBuilding, TechnoClass* pTechno);
	static bool DoGrindingExtras(BuildingClass* pBuilding, TechnoClass* pTechno, int refund);
	static bool __fastcall HasSWType(BuildingClass* pBuilding, int swIdx);
	static bool HandleInfiltrate(BuildingClass* pBuilding, HouseClass* pInfiltratorHouse);
	static bool HandleInfiltrateUpgrades(BuildingClass* pBuilding, HouseClass* pInfiltratorHouse, BuildingTypeExt::ExtData* pTypeExt);
	static void StoreTiberium(BuildingClass* pThis, float amount, int idxTiberiumType, int idxStorageTiberiumType);
};
