#pragma once

#include <map>

#include <SuperWeaponTypeClass.h>

#include <Misc/TypeConvertHelper.h>

#include <Utilities/Constructs.h>
#include <Utilities/Container.h>
#include <Utilities/Enum.h>
#include <Utilities/Template.h>

class GScreenAnimTypeClass;
class VoxClass;

class SWTypeExt
{
public:
	using base_type = SuperWeaponTypeClass;

	class ExtData final : public Extension<SuperWeaponTypeClass>
	{
	public:

		PhobosFixedString<0x20> TypeID;

		//Ares 0.A
		Valueable<int> Money_Amount;
		ValueableIdx<VoxClass> EVA_Impatient;
		NullableIdx<VoxClass> EVA_InsufficientFunds;
		NullableIdx<VoxClass> EVA_SelectTarget;
		Valueable<bool> SW_UseAITargeting;
		Valueable<bool> SW_AutoFire;
		Valueable<bool> SW_ManualFire;
		Valueable<bool> SW_ShowCameo;
		Valueable<bool> SW_Unstoppable;
		ValueableVector<TechnoTypeClass*> SW_Inhibitors;
		Valueable<bool> SW_AnyInhibitor;
		ValueableVector<TechnoTypeClass*> SW_Designators;
		Valueable<bool> SW_AnyDesignator;
		Valueable<bool> SW_VirtualCharge;
		Valueable<double> SW_RangeMinimum;
		Valueable<double> SW_RangeMaximum;
		DWORD SW_RequiredHouses;
		DWORD SW_ForbiddenHouses;
		ValueableVector<BuildingTypeClass*> SW_AuxBuildings;
		ValueableVector<BuildingTypeClass*> SW_NegBuildings;

		Valueable<CSFText> Message_InsufficientFunds {};
		Valueable<int> Message_ColorScheme { -1 };
		Valueable<bool> Message_FirerColor { false };

		Valueable<CSFText> UIDescription;
		Valueable<int> Power;
		Valueable<int> CameoPriority;
		ValueableVector<BuildingTypeClass*> LimboDelivery_Types;
		ValueableVector<int> LimboDelivery_IDs;
		ValueableVector<float> LimboDelivery_RollChances;
		Valueable<AffectedHouse> LimboKill_Affected;
		ValueableVector<int> LimboKill_IDs;
		Valueable<double> RandomBuffer;
		ValueableVector<SuperWeaponTypeClass*> SW_Next;
		Valueable<bool> SW_Next_RealLaunch;
		Valueable<bool> SW_Next_IgnoreInhibitors;
		Valueable<bool> SW_Next_IgnoreDesignators;
		ValueableVector<float> SW_Next_RollChances;

		Valueable<bool> SW_Cumulative;
		Valueable<int> SW_Cumulative_MaxCount;
		Valueable<int> SW_Cumulative_InitialCount;
		ValueableVector<int> SW_Cumulative_AdditionCounts;
		ValueableVector<TechnoTypeClass*> SW_Cumulative_AdditionTypes;
		Valueable<bool> SW_Cumulative_ShowCountInTimer;
		Valueable<bool> SW_Cumulative_ShowTrueTimer;

		Valueable<bool> ShowTimerCustom;
		Valueable<ShowTimerType> ShowTimerCustom_Type;
		Valueable<bool> ShowTimerCustom_AlwaysShow;
		Nullable<CSFText> ShowTimerCustom_UIName;

		Nullable<WarheadTypeClass*> Detonate_Warhead;
		Nullable<WeaponTypeClass*> Detonate_Weapon;
		Nullable<int> Detonate_Damage;

		ValueableVector<ValueableVector<int>> LimboDelivery_RandomWeightsData;
		ValueableVector<ValueableVector<int>> SW_Next_RandomWeightsData;

		Nullable<GScreenAnimTypeClass*> GScreenAnimType;
		Nullable<GScreenAnimTypeClass*> CursorAnimType;

		Valueable<bool> CreateBuilding;
		Valueable<BuildingTypeClass*> CreateBuilding_Type;
		Valueable<int> CreateBuilding_Duration;
		Valueable<int> CreateBuilding_Reload;
		Valueable<bool> CreateBuilding_AutoCreate;

		ValueableIdxVector<SuperWeaponTypeClass> ScreenSW;
		Valueable<Point2D> ScreenSW_Offset;
		Valueable<int> ScreenSW_Duration;
		Valueable<int> ScreenSW_Reload;
		Valueable<bool> ScreenSW_AutoLaunch;

		Nullable<SuperWeaponTypeClass*> NextSuperWeapon;

		ValueableIdxVector<SuperWeaponTypeClass> ResetSW;
		Valueable<bool> ResetSW_UseCurrtenRechargeTime;
		Valueable<bool> ResetSW_UseCurrtenRechargeTime_ForceSet;

		Valueable<AffectedHouse> SW_AffectsHouse;
		Valueable<AffectedTarget> SW_AffectsTarget;

		Valueable<int> SW_Priority;

		Nullable<bool> TimerPercentage;
		Nullable<int> TimerXOffset;

		Valueable<bool> SW_Squared;
		Nullable<double> SW_Squared_Range;
		Valueable<Point2D> SW_Squared_Offset;

		ValueableIdxVector<SuperWeaponTypeClass> SW_ShareRechargeTimeTypes;
		ValueableIdxVector<SuperWeaponTypeClass> SW_ShareCumulativeCountTypes;

		Valueable<bool> InSWBar { true };

		CustomPalette CameoPal {};
		PhobosPCXFile SidebarPCX {};

		std::vector<TypeConvertGroup> Convert_Pairs;

		Valueable<bool> ShowDesignatorRange;

		// MultipleSWFirer
		std::vector<SuperWeaponTypeClass*> MultipleSWFirer_FireSW_Types;
		std::vector<int> MultipleSWFirer_FireSW_Deferments;
		Valueable<bool> MultipleSWFirer_RandomPick;
		std::vector<bool> MultipleSWFirer_RealLaunch;

		// WeaponDetonateOnTechno
		ValueableVector<WeaponTypeClass*> WeaponDetonate_Weapons;
		ValueableVector<TechnoTypeClass*> WeaponDetonate_TechnoTypes;
		ValueableVector<TechnoTypeClass*> WeaponDetonate_TechnoTypes_Ignore;
		Valueable<bool> WeaponDetonate_RandomPick_Weapon;
		Valueable<bool> WeaponDetonate_RandomPick_TechnoType;
		ValueableVector<int> WeaponDetonate_RandomPick_Weapon_Weights;
		std::map<int, int> WeaponDetonate_PerSum_WeaponWeights;
		ValueableVector<int> WeaponDetonate_RandomPick_TechnoType_Weights;
		std::map<int, int> WeaponDetonate_PerSum_TechnoTypeWeights;

		// UnitFall
		Valueable<bool> UnitFall_RandomPick;
		Valueable<bool> UnitFall_RandomInRange;
		ValueableVector<TechnoTypeClass*> UnitFall_Types;
		ValueableVector<int> UnitFall_Deferments;
		ValueableVector<int> UnitFall_Heights;
		ValueableVector<bool> UnitFall_UseParachutes;
		ValueableVector<OwnerHouseKind> UnitFall_Owners;
		ValueableVector<WeaponTypeClass*> UnitFall_Weapons;
		ValueableVector<AnimTypeClass*> UnitFall_Anims;
		ValueableVector<double> UnitFall_RandomPickWeights;
		ValueableVector<unsigned short> UnitFall_Facings;
		ValueableVector<bool> UnitFall_RandomFacings;
		ValueableVector<int> UnitFall_Healths;
		ValueableVector<Mission> UnitFall_Missions;
		ValueableVector<double> UnitFall_Veterancys;
		ValueableVector<bool> UnitFall_Destorys;
		ValueableVector<int> UnitFall_DestoryHeights;
		ValueableVector<bool> UnitFall_AlwaysFalls;

		//EMPulseExtra
		Valueable<bool> EMPulse_Linked;
		Valueable<bool> EMPulse_TargetSelf;
		ValueableVector<TechnoTypeClass*> EMPulse_Cannons;
		Valueable<AnimTypeClass*> EMPulse_PulseBall;
		Valueable<bool> EMPulse_IgnoreMission;

		virtual void InitializeConstants();

		ExtData(SuperWeaponTypeClass* OwnerObject) : Extension<SuperWeaponTypeClass>(OwnerObject)
			, TypeID { "" }
			, Money_Amount { 0 }
			, EVA_Impatient { -1 }
			, EVA_InsufficientFunds {}
			, EVA_SelectTarget {}
			, SW_UseAITargeting { false }
			, SW_AutoFire { false }
			, SW_ManualFire { true }
			, SW_ShowCameo { true }
			, SW_Unstoppable { false }
			, SW_Inhibitors {}
			, SW_AnyInhibitor { false }
			, SW_Designators {}
			, SW_AnyDesignator { false }
			, SW_RangeMinimum { -1.0 }
			, SW_RangeMaximum { -1.0 }
			, SW_RequiredHouses { 0xFFFFFFFFu }
			, SW_ForbiddenHouses { 0u }
			, SW_AuxBuildings {}
			, SW_NegBuildings {}
			, Message_InsufficientFunds {}
			, Message_ColorScheme { -1 }
			, Message_FirerColor { false }
			, UIDescription {}
			, Power { 0 }
			, CameoPriority { 0 }
			, LimboDelivery_Types {}
			, LimboDelivery_IDs {}
			, LimboDelivery_RollChances {}
			, LimboDelivery_RandomWeightsData {}
			, LimboKill_Affected { AffectedHouse::Owner }
			, LimboKill_IDs {}
			, RandomBuffer { 0.0 }
			, Detonate_Warhead {}
			, Detonate_Weapon {}
			, Detonate_Damage {}
			, SW_Next {}
			, SW_Next_RealLaunch { true }
			, SW_Next_IgnoreInhibitors { false }
			, SW_Next_IgnoreDesignators { true }
			, SW_Next_RollChances {}
			, SW_Next_RandomWeightsData {}
			, GScreenAnimType {}
			, CursorAnimType {}
			, CreateBuilding { false }
			, CreateBuilding_Type {}
			, CreateBuilding_Duration { 1500 }
			, CreateBuilding_Reload { 100 }
			, CreateBuilding_AutoCreate { false }
			, ScreenSW {}
			, ScreenSW_Offset { { 0, 0 } }
			, ScreenSW_Duration { 1500 }
			, ScreenSW_Reload { 100 }
			, ScreenSW_AutoLaunch { false }
			, NextSuperWeapon {}

			, SW_Cumulative { false }
			, SW_Cumulative_MaxCount { -1 }
			, SW_Cumulative_InitialCount { 0 }
			, SW_Cumulative_AdditionTypes {}
			, SW_Cumulative_AdditionCounts {}
			, SW_Cumulative_ShowCountInTimer { true }
			, SW_Cumulative_ShowTrueTimer { true }

			, ShowTimerCustom { false }
			, ShowTimerCustom_Type { ShowTimerType::Hour }
			, ShowTimerCustom_AlwaysShow { true }
			, ShowTimerCustom_UIName {}

			, ResetSW {}
			, ResetSW_UseCurrtenRechargeTime { false }
			, ResetSW_UseCurrtenRechargeTime_ForceSet { false }

			, SW_AffectsHouse { AffectedHouse::Owner }
			, SW_AffectsTarget { AffectedTarget::AllTechnos }
			, SW_VirtualCharge { false }
			, SW_Priority { 0 }

			, TimerPercentage { }
			, TimerXOffset { }

			, SW_Squared { false }
			, SW_Squared_Range { }
			, SW_Squared_Offset { { 0, 0 } }

			, SW_ShareRechargeTimeTypes {}
			, SW_ShareCumulativeCountTypes {}

			, InSWBar { true }

			, CameoPal {}
			, SidebarPCX {}

			, Convert_Pairs {}

			, ShowDesignatorRange { true }

			, MultipleSWFirer_FireSW_Types {}
			, MultipleSWFirer_FireSW_Deferments {}
			, MultipleSWFirer_RandomPick {}
			, MultipleSWFirer_RealLaunch {}

			, WeaponDetonate_Weapons {}
			, WeaponDetonate_TechnoTypes {}
			, WeaponDetonate_TechnoTypes_Ignore {}
			, WeaponDetonate_RandomPick_Weapon { false }
			, WeaponDetonate_RandomPick_TechnoType { false }
			, WeaponDetonate_RandomPick_Weapon_Weights {}
			, WeaponDetonate_PerSum_WeaponWeights {}
			, WeaponDetonate_RandomPick_TechnoType_Weights {}
			, WeaponDetonate_PerSum_TechnoTypeWeights {}

			, UnitFall_RandomPick { false }
			, UnitFall_RandomInRange { false }
			, UnitFall_Types {}
			, UnitFall_Deferments {}
			, UnitFall_Heights {}
			, UnitFall_UseParachutes {}
			, UnitFall_Owners {}
			, UnitFall_Weapons {}
			, UnitFall_Anims {}
			, UnitFall_RandomPickWeights {}
			, UnitFall_Facings {}
			, UnitFall_RandomFacings {}
			, UnitFall_Healths {}
			, UnitFall_Missions {}
			, UnitFall_Veterancys {}
			, UnitFall_Destorys {}
			, UnitFall_DestoryHeights {}
			, UnitFall_AlwaysFalls {}

			, EMPulse_Cannons {}
			, EMPulse_Linked { false }
			, EMPulse_TargetSelf { false }
			, EMPulse_IgnoreMission { false }
			, EMPulse_PulseBall {}
		{ }

		// Ares 0.A functions
		bool IsInhibitor(HouseClass* pOwner, TechnoClass* pTechno) const;
		bool HasInhibitor(HouseClass* pOwner, const CellStruct& coords) const;
		bool IsInhibitorEligible(HouseClass* pOwner, const CellStruct& coords, TechnoClass* pTechno) const;
		bool IsDesignator(HouseClass* pOwner, TechnoClass* pTechno) const;
		bool HasDesignator(HouseClass* pOwner, const CellStruct& coords) const;
		bool IsDesignatorEligible(HouseClass* pOwner, const CellStruct& coords, TechnoClass* pTechno) const;
		bool IsLaunchSiteEligible(const CellStruct& Coords, BuildingClass* pBuilding, bool ignoreRange) const;
		bool IsLaunchSite(BuildingClass* pBuilding) const;
		std::pair<double, double> GetLaunchSiteRange(BuildingClass* pBuilding = nullptr) const;
		bool IsAvailable(HouseClass* pHouse) const;
		void PrintMessage(const CSFText& message, HouseClass* pFirer);

		void ApplyLimboDelivery(HouseClass* pHouse);
		void ApplyLimboKill(HouseClass* pHouse);
		void ApplyDetonation(HouseClass* pHouse, const CellStruct& cell);
		void ApplySWNext(SuperClass* pSW, const CellStruct& cell);
		void ApplyTypeConversion(SuperClass* pSW);

		void FireSuperWeaponAnim(SuperClass* pSW, HouseClass* pHouse);

		void FireNextSuperWeapon(SuperClass* pSW, HouseClass* pHouse);

		virtual void LoadFromINIFile(CCINIClass* pINI) override;
		virtual ~ExtData() = default;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;

		virtual void SaveToStream(PhobosStreamWriter& Stm) override;
	private:
		std::vector<int> WeightedRollsHandler(ValueableVector<float>* chances, ValueableVector<ValueableVector<int>>* weights, size_t size);

		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<SWTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static void FireSuperWeaponExt(SuperClass* pSW, const CellStruct& cell);

	static bool Activate(SuperClass* pSuper, CellStruct cell, bool isPlayer);

	static ExtContainer ExtMap;
	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

};
