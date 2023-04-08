#pragma once
#include <HouseClass.h>
#include <TActionClass.h >

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

#include <Ext/BuildingType/Body.h>

#include <map>

class HouseExt
{
public:
	using base_type = HouseClass;
	class ExtData final : public Extension<HouseClass>
	{
	public:
		std::map<BuildingTypeExt::ExtData*, int> BuildingCounter;
		CounterClass OwnedLimboBuildingTypes;
		bool ForceOnlyTargetHouseEnemy = false;
		int ForceOnlyTargetHouseEnemyMode = 0;

		BuildingClass* Factory_BuildingType = nullptr;
		BuildingClass* Factory_InfantryType = nullptr;
		BuildingClass* Factory_VehicleType = nullptr;
		BuildingClass* Factory_NavyType = nullptr;
		BuildingClass* Factory_AircraftType = nullptr;

		//Read from INI
		bool RepairBaseNodes[3] = { false, false, false };

		std::map<int, std::vector<AircraftClass*>> OwnedAircraft;
		std::map<int, std::vector<BuildingClass*>> OwnedBuilding;
		std::map<int, std::vector<InfantryClass*>> OwnedInfantry;
		std::map<int, std::vector<UnitClass*>> OwnedUnit;

		std::vector<int> AlreadyGranted;
		bool ScoreVectorInited = false;

		std::vector<ValueableVector<TechnoTypeClass*>> DeactivateInfantry_Types;
		std::vector<ValueableVector<TechnoTypeClass*>> DeactivateInfantry_Ignore;
		std::vector<int> DeactivateInfantry_Duration;
		std::vector<ValueableVector<TechnoTypeClass*>> DeactivateVehicle_Types;
		std::vector<ValueableVector<TechnoTypeClass*>> DeactivateVehicle_Ignore;
		std::vector<int> DeactivateVehicle_Duration;
		std::vector<ValueableVector<TechnoTypeClass*>> DeactivateNaval_Types;
		std::vector<ValueableVector<TechnoTypeClass*>> DeactivateNaval_Ignore;
		std::vector<int> DeactivateNaval_Duration;
		std::vector<ValueableVector<TechnoTypeClass*>> DeactivateAircraft_Types;
		std::vector<ValueableVector<TechnoTypeClass*>> DeactivateAircraft_Ignore;
		std::vector<int> DeactivateAircraft_Duration;
		std::vector<ValueableVector<TechnoTypeClass*>> DeactivateBuilding_Types;
		std::vector<ValueableVector<TechnoTypeClass*>> DeactivateBuilding_Ignore;
		std::vector<int> DeactivateBuilding_Duration;
		std::vector<ValueableVector<TechnoTypeClass*>> DeactivateDefense_Types;
		std::vector<ValueableVector<TechnoTypeClass*>> DeactivateDefense_Ignore;
		std::vector<int> DeactivateDefense_Duration;

		bool Checked = false;

		bool VeterancyInit = false;
		std::vector<TechnoTypeClass*> InfantryVeterancyTypes;
		std::vector<double> InfantryVeterancy;
		std::vector<TechnoTypeClass*> VehicleVeterancyTypes;
		std::vector<double> VehicleVeterancy;
		std::vector<TechnoTypeClass*> NavalVeterancyTypes;
		std::vector<double> NavalVeterancy;
		std::vector<TechnoTypeClass*> AircraftVeterancyTypes;
		std::vector<double> AircraftVeterancy;
		std::vector<TechnoTypeClass*> BuildingVeterancyTypes;
		std::vector<double> BuildingVeterancy;

		ValueableIdxVector<SuperWeaponTypeClass> SpySuperWeaponTypes;
		std::vector<int> SpySuperWeaponDelay;

		std::vector<int> SuperWeaponCumulativeCharge;
		std::vector<int> SuperWeaponCumulativeCount;
		std::vector<int> SuperWeaponCumulativeMaxCount;
		std::vector<bool> SuperWeaponCumulativeInherit;
		std::vector<bool> SuperWeaponCumulativeSupplement;

		int PowerUnitOutPut = 0;
		int PowerUnitDrain = 0;
		int BuildingCount = 0;
		int BuildingCheckCount = 0;

		std::vector<TechnoClass*> WarpTechnos;
		std::vector<TechnoClass*> WarpOutTechnos;

		std::vector<TechnoClass*> TemporalStands;

		std::vector<TechnoClass*> UnitFallTechnos;
		std::vector<std::vector<SuperClass*>> UnitFallConnects;
		std::vector<int> UnitFallDeferments;
		std::vector<int> UnitFallHeights;
		std::vector<bool> UnitFallUseParachutes;
		std::vector<OwnerHouseKind> UnitFallOwners;
		std::vector<AnimTypeClass*> UnitFallAnims;
		std::vector<unsigned short> UnitFallFacings;
		std::vector<bool> UnitFallRandomFacings;
		std::vector<Mission> UnitFallMissions;
		std::vector<bool> UnitFallAlwaysFalls;
		std::vector<CellStruct> UnitFallCells;
		std::vector<SuperClass*> UnitFallReallySWs;
		std::vector<HouseClass*> UnitFallTechnoOwners;

		CDTimerClass GapRadarTimer;

		std::vector<HouseClass*> RevealRadarSightOwners;
		std::vector<BuildingClass*> RevealRadarSightBuildings;
		std::vector<HouseClass*> RevealRadarSightBuildingOwners;
		std::vector<bool> RevealRadarSightPermanents;
		std::vector<bool> KeepRevealRadarSights;
		std::vector<bool> RevealRadarSights_Infantry;
		std::vector<bool> RevealRadarSights_Unit;
		std::vector<bool> RevealRadarSights_Aircraft;
		std::vector<bool> RevealRadarSights_Building;
		std::vector<CDTimerClass> RevealRadarSightTimers;

		ExtData(HouseClass* OwnerObject) : Extension<HouseClass>(OwnerObject)
			, OwnedLimboBuildingTypes {}
			, ForceOnlyTargetHouseEnemy { false }
			, ForceOnlyTargetHouseEnemyMode { -1 }
			, Factory_BuildingType { nullptr }
			, Factory_InfantryType { nullptr }
			, Factory_VehicleType { nullptr }
			, Factory_NavyType { nullptr }
			, Factory_AircraftType { nullptr }
			, RepairBaseNodes { false,false,false }
			, AlreadyGranted()
			, ScoreVectorInited(false)
			, OwnedAircraft()
			, OwnedBuilding()
			, OwnedInfantry()
			, OwnedUnit()
			, DeactivateInfantry_Types()
			, DeactivateInfantry_Ignore()
			, DeactivateInfantry_Duration()
			, DeactivateVehicle_Types()
			, DeactivateVehicle_Ignore()
			, DeactivateVehicle_Duration()
			, DeactivateNaval_Types()
			, DeactivateNaval_Ignore()
			, DeactivateNaval_Duration()
			, DeactivateAircraft_Types()
			, DeactivateAircraft_Ignore()
			, DeactivateAircraft_Duration()
			, DeactivateBuilding_Types()
			, DeactivateBuilding_Ignore()
			, DeactivateBuilding_Duration()
			, DeactivateDefense_Types()
			, DeactivateDefense_Ignore()
			, DeactivateDefense_Duration()
			, VeterancyInit(false)
			, InfantryVeterancyTypes()
			, InfantryVeterancy()
			, VehicleVeterancyTypes()
			, VehicleVeterancy()
			, NavalVeterancyTypes()
			, NavalVeterancy()
			, AircraftVeterancyTypes()
			, AircraftVeterancy()
			, BuildingVeterancyTypes()
			, BuildingVeterancy()
			, Checked()
			, SpySuperWeaponTypes()
			, SpySuperWeaponDelay()
			, SuperWeaponCumulativeCharge()
			, SuperWeaponCumulativeCount()
			, SuperWeaponCumulativeMaxCount()
			, SuperWeaponCumulativeInherit()
			, SuperWeaponCumulativeSupplement()
			, PowerUnitOutPut()
			, PowerUnitDrain()
			, BuildingCount()
			, BuildingCheckCount()
			, WarpTechnos()
			, WarpOutTechnos()
			, TemporalStands()
			, UnitFallTechnos()
			, UnitFallConnects()
			, UnitFallDeferments()
			, UnitFallHeights()
			, UnitFallUseParachutes()
			, UnitFallOwners()
			, UnitFallAnims()
			, UnitFallFacings()
			, UnitFallRandomFacings()
			, UnitFallMissions()
			, UnitFallAlwaysFalls()
			, UnitFallCells()
			, UnitFallReallySWs()
			, UnitFallTechnoOwners()
			, GapRadarTimer()
			, RevealRadarSightOwners()
			, RevealRadarSightBuildings()
			, RevealRadarSightBuildingOwners()
			, RevealRadarSightPermanents()
			, KeepRevealRadarSights()
			, RevealRadarSights_Infantry()
			, RevealRadarSights_Unit()
			, RevealRadarSights_Aircraft()
			, RevealRadarSights_Building()
			, RevealRadarSightTimers()
		{ }

		virtual ~ExtData() = default;

		virtual void LoadFromINIFile(CCINIClass* pINI) override;
		//virtual void Initialize() override;
		virtual void InvalidatePointer(void* ptr, bool bRemoved) override
		{
			AnnounceInvalidPointer(Factory_AircraftType, ptr);
			AnnounceInvalidPointer(Factory_NavyType, ptr);
			AnnounceInvalidPointer(Factory_InfantryType, ptr);
			AnnounceInvalidPointer(Factory_VehicleType, ptr);
			AnnounceInvalidPointer(Factory_BuildingType, ptr);
		}

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<HouseExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	enum class FactoryState
	{
		NoFactory = 0, // there is no factory building for this
		Unpowered = 1, // there is a factory building, but it is offline
		Available = 2 // at least one factory building is as online as required
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static int CountOwnedLimbo(HouseClass* pThis, BuildingTypeClass const* const pItem);

	static int ActiveHarvesterCount(HouseClass* pThis);
	static int TotalHarvesterCount(HouseClass* pThis);
	static HouseClass* GetHouseKind(OwnerHouseKind kind, bool allowRandom, HouseClass* pDefault, HouseClass* pInvoker = nullptr, HouseClass* pVictim = nullptr);

	static void ForceOnlyTargetHouseEnemy(HouseClass* pThis, int mode);
	static void GrantScoreSuperPower(HouseClass* pThis, int SWIDX);
	static int GetHouseIndex(int param, TeamClass* pTeam, TActionClass* pTAction);
	static int CountOwnedIncludeDeploy(const HouseClass* pThis, const TechnoTypeClass* pItem);
	static CanBuildResult BuildLimitGroupCheck(const HouseClass* pThis, const TechnoTypeClass* pItem, bool buildLimitOnly, bool includeQueued);
	static FactoryState HasFactory_Ares(const HouseClass* pThis, const TechnoTypeClass* pItem, bool requirePower);
	static void RegisterGain(HouseClass* pThis, TechnoClass* pTechno);
	static void RegisterLoss(HouseClass* pThis, TechnoClass* pTechno);
	static const std::vector<TechnoClass*>& GetOwnedTechno(HouseClass* pThis, TechnoTypeClass* pType);
	static void TechnoDeactivate(HouseClass* pThis);
	static void TechnoVeterancyInit(HouseClass* pThis);
	static void TechnoUpgrade(HouseClass* pThis, double veterancy, ValueableVector<TechnoTypeClass*> types, ValueableVector<TechnoTypeClass*> ignore, AbstractType whatamI, bool naval = false, bool cumulative = false);
	static void SpySuperWeaponCount(HouseClass* pThis);
	static void CheckSuperWeaponCumulativeMax(HouseClass* pThis);
	static void SuperWeaponCumulative(HouseClass* pThis);
	static void SuperWeaponCumulativeReset(HouseClass* pThis, SuperClass* pSW);
	static void CheckTechnoType(HouseClass* pThis);
	static void SetWarpTechnos(HouseClass* pThis);
	static void SetWarpOutTechnos(HouseClass* pThis);
	static void TemporalStandsCheck(HouseClass* pThis);
	static void UnitFallCheck(HouseClass* pThis, SuperClass* pSW, const CellStruct& cell);
	static void UnitFallActivate(HouseClass* pThis);
	static void GapRadar(HouseClass* pThis);
	static void RevealRadarSight(HouseClass* pThis);
};
