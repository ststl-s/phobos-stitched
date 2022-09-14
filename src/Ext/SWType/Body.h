#pragma once
#include <SuperWeaponTypeClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>
#include <New/Type/GScreenAnimTypeClass.h>

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
		ValueableVector<TechnoTypeClass*> SW_Inhibitors;
		Valueable<bool> SW_AnyInhibitor;
		ValueableVector<TechnoTypeClass*> SW_Designators;
		Valueable<bool> SW_AnyDesignator;
		Valueable<bool> SW_VirtualCharge;

		Valueable<CSFText> UIDescription;
		Valueable<int> CameoPriority;
		ValueableVector<TechnoTypeClass*> LimboDelivery_Types;
		ValueableVector<int> LimboDelivery_IDs;
		ValueableVector<float> LimboDelivery_RollChances;
		Valueable<AffectedHouse> LimboKill_Affected;
		ValueableVector<int> LimboKill_IDs;
		Valueable<double> RandomBuffer;


		ValueableVector<ValueableVector<int>> LimboDelivery_RandomWeightsData;

		Nullable<GScreenAnimTypeClass*> GScreenAnimType;

		Valueable<bool> CreateBuilding;
		Valueable<BuildingTypeClass*> CreateBuilding_Type;
		Valueable<int> CreateBuilding_Duration;
		Valueable<int> CreateBuilding_Reload;
		Valueable<bool> CreateBuilding_AutoCreate;

		Nullable<SuperWeaponTypeClass*> NextSuperWeapon;

		Valueable<AffectedHouse> SW_AffectsHouse;
		Valueable<AffectedTarget> SW_AffectsTarget;

		// MultipleSWFirer
		std::vector<SuperWeaponTypeClass*> MultipleSWFirer_FireSW_Types;
		std::vector<int> MultipleSWFirer_FireSW_Deferments;
		Valueable<bool> MultipleSWFirer_RandomPick;
		std::vector<BOOL> MultipleSWFirer_RealLaunch;

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

		virtual void InitializeConstants();

		ExtData(SuperWeaponTypeClass* OwnerObject) : Extension<SuperWeaponTypeClass>(OwnerObject)
			, TypeID { "" }
			, Money_Amount { 0 }
			, SW_Inhibitors {}
			, SW_AnyInhibitor { false }
			, SW_Designators { }
			, SW_AnyDesignator { false }
			, UIDescription {}
			, CameoPriority { 0 }
			, LimboDelivery_Types {}
			, LimboDelivery_IDs {}
			, LimboDelivery_RollChances {}
			, LimboDelivery_RandomWeightsData {}
			, LimboKill_Affected { AffectedHouse::Owner }
			, LimboKill_IDs {}
			, RandomBuffer { 0.0 }
			, GScreenAnimType {}
			, CreateBuilding { false }
			, CreateBuilding_Type {}
			, CreateBuilding_Duration { 1500 }
			, CreateBuilding_Reload { 100 }
			, CreateBuilding_AutoCreate { false }
			, NextSuperWeapon {}

			, SW_AffectsHouse { AffectedHouse::Owner }
			, SW_AffectsTarget { AffectedTarget::AllTechnos }
			, SW_VirtualCharge { false }

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
		{ }

		// Ares 0.A functions
		bool IsInhibitor(HouseClass* pOwner, TechnoClass* pTechno) const;
		bool HasInhibitor(HouseClass* pOwner, const CellStruct& coords) const;
		bool IsInhibitorEligible(HouseClass* pOwner, const CellStruct& coords, TechnoClass* pTechno) const;

		bool IsDesignator(HouseClass* pOwner, TechnoClass* pTechno) const;
		bool HasDesignator(HouseClass* pOwner, const CellStruct& coords) const;
		bool IsDesignatorEligible(HouseClass* pOwner, const CellStruct& coords, TechnoClass* pTechno) const;

		void ApplyLimboDelivery(HouseClass* pHouse);
		void ApplyLimboKill(HouseClass* pHouse);

		void FireSuperWeaponAnim(SuperClass* pSW, HouseClass* pHouse);

		void FireNextSuperWeapon(SuperClass* pSW, HouseClass* pHouse);

		virtual void LoadFromINIFile(CCINIClass* pINI) override;
		virtual ~ExtData() = default;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;

		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

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
