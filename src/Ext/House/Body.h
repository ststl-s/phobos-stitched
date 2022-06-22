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
		bool ForceOnlyTargetHouseEnemy;
		int ForceOnlyTargetHouseEnemyMode;

		BuildingClass* Factory_BuildingType;
		BuildingClass* Factory_InfantryType;
		BuildingClass* Factory_VehicleType;
		BuildingClass* Factory_NavyType;
		BuildingClass* Factory_AircraftType;

		ValueableVector<int> vAlreadyGranted;
		bool ScoreVectorInited;

		ExtData(HouseClass* OwnerObject) : Extension<HouseClass>(OwnerObject)
			, OwnedLimboBuildingTypes {}
			, ForceOnlyTargetHouseEnemy { false }
			, ForceOnlyTargetHouseEnemyMode { -1 }
			, Factory_BuildingType(nullptr)
			, Factory_InfantryType(nullptr)
			, Factory_VehicleType(nullptr)
			, Factory_NavyType(nullptr)
			, Factory_AircraftType(nullptr)
			, vAlreadyGranted()
			, ScoreVectorInited(false)
		{ }

		virtual ~ExtData() = default;

		//virtual void Initialize() override;
		virtual void InvalidatePointer(void* ptr, bool bRemoved) override {}

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<HouseExt> {
	public:
		ExtContainer();
		~ExtContainer();
	};

	enum class BuildLimitStatus
	{
		ReachedPermanently = -1, // remove cameo
		ReachedTemporarily = 0, // black out cameo
		NotReached = 1, // don't do anything
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

	static void HouseExt::ForceOnlyTargetHouseEnemy(HouseClass* pThis, int mode);
	static void HouseExt::GrantScoreSuperPower(HouseClass* pThis, int SWIDX);
	static int GetHouseIndex(int param, TeamClass* pTeam, TActionClass* pTAction);
	static int HouseExt::CountOwnedIncludeDeploy(HouseClass* pThis, TechnoTypeClass* pItem);
	static BuildLimitStatus BuildLimitGroupCheck(HouseClass* pThis, TechnoTypeClass* pItem, bool buildLimitOnly, bool includeQueued, BuildLimitStatus Origin);
	static BuildLimitStatus BuildLimitGroupValidate(HouseClass* pThis, TechnoTypeClass* pItem, bool includeQueued, BuildLimitStatus Origin);
	static FactoryState HasFactory_Ares(HouseClass* pThis, TechnoTypeClass* pItem, bool requirePower);
};