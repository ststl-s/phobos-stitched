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
};
