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

		// MultipleSWFirer
		std::vector<SuperWeaponTypeClass*> FireSW_Types;
		std::vector<int> FireSW_Deferments;

		virtual void InitializeConstants();

		ExtData(SuperWeaponTypeClass* OwnerObject) : Extension<SuperWeaponTypeClass>(OwnerObject)
			, TypeID { "" }
			, Money_Amount { 0 }
			, SW_Inhibitors {}
			, SW_AnyInhibitor { false }
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
			, FireSW_Types {}
			, FireSW_Deferments {}
		{ }


		void FireSuperWeapon(SuperClass* pSW, HouseClass* pHouse, CoordStruct coords);

		void FireSuperWeaponAnim(SuperClass* pSW, HouseClass* pHouse);

		virtual void LoadFromINIFile(CCINIClass* pINI) override;
		virtual ~ExtData() = default;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;

		virtual void SaveToStream(PhobosStreamWriter& Stm) override;
	private:
		void ApplyLimboDelivery(HouseClass* pHouse);
		void ApplyLimboKill(HouseClass* pHouse);

		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<SWTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static bool Activate(SuperClass* pSuper, CellStruct cell, bool isPlayer);

	static ExtContainer ExtMap;
	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static bool IsInhibitor(SWTypeExt::ExtData* pSWType, HouseClass* pOwner, TechnoClass* pTechno);
	static bool HasInhibitor(SWTypeExt::ExtData* pSWType, HouseClass* pOwner, const CellStruct& Coords);
	static bool IsInhibitorEligible(SWTypeExt::ExtData* pSWType, HouseClass* pOwner, const CellStruct& Coords, TechnoClass* pTechno);
};
