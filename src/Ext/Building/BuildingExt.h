#pragma once

#include <BuildingClass.h>
#include <HouseClass.h>
#include <TiberiumClass.h>
#include <FactoryClass.h>

#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/Building/BuildingExt.h>

#include <Misc/FlyingStrings.h>

#include <Utilities/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

class BuildingExt
{
public:
	using base_type = BuildingClass;

	static constexpr DWORD Canary = 0x87654321;
	static constexpr size_t ExtPointerOffset = 0x6FC;

	class ExtData final : public Extension<BuildingClass>
	{
	public:
		BuildingTypeExt::ExtData* TypeExtData = nullptr;
		TechnoExt::ExtData* TechnoExtData = nullptr;
		bool DeployedTechno = false;
		int LimboID = 0;
		int GrindingWeapon_LastFiredFrame = 0;
		BuildingClass* CurrentAirFactory = nullptr;
		int AccumulatedIncome = 0;
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
		AnimClass* SpyEffectAnim = nullptr;
		int SpyEffectAnimDuration = 0;
		AffectedHouse SpyEffectAnimDisplayHouses = AffectedHouse::All;
		bool SellWeaponDetonated = false;
		int OverPowerLevel = 0;
		int BuildingROFFix = -1;
		std::vector<DynamicVectorClass<int>> IFVTurrets = {};
		int IFVMode = 0;
		WeaponTypeClass* CurrtenWeapon = nullptr;

		ExtData(BuildingClass* OwnerObject) : Extension<BuildingClass>(OwnerObject)
			, TypeExtData { nullptr }
			, DeployedTechno { false }
			, LimboID { -1 }
			, GrindingWeapon_LastFiredFrame { 0 }
			, CurrentAirFactory { nullptr }
			, AccumulatedIncome { 0 }
		{ }

		void DisplayRefund();
		void ApplyPoweredKillSpawns();
		bool HasSuperWeapon(int index, bool withUpgrades) const;
		void BuildingPowered();
		void SabotageBuilding();
		void SellBuilding();
		void CaptureBuilding();
		void ForbidSell();
		void RevealSight();
		void SpyEffectAnimCheck();
		void AutoRepairCheck();
		void OccupantsWeaponChange();
		void SelectIFVWeapon();
		void OccupantsWeapon();
		void BuildingWeaponChange();

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
	static void __fastcall BuildingPassengerFix(BuildingClass* pThis);
	static void __fastcall BuildingSpawnFix(BuildingClass* pThis);

	static void FireDeathWeapon_Upgrades(BuildingClass* pThis);
};
