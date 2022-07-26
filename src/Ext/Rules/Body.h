#pragma once

#include <CCINIClass.h>
#include <RulesClass.h>
#include <AITriggerTypeClass.h>

#include <Utilities/Enum.h>
#include <Utilities/Container.h>
#include <Utilities/Constructs.h>
#include <Utilities/Template.h>
#include <Utilities/Anchor.h>
#include <Utilities/Debug.h>

#include <New/Type/GScreenAnimTypeClass.h>

class AnimTypeClass;
class MouseCursor;
class TechnoTypeClass;
class VocClass;
class WarheadTypeClass;
class DigitalDisplayTypeClass;

class RulesExt
{
public:
	using base_type = RulesClass;

	class ExtData final : public Extension<RulesClass>
	{
	public:
		DynamicVectorClass<DynamicVectorClass<TechnoTypeClass*>> AITargetTypesLists;
		DynamicVectorClass<DynamicVectorClass<ScriptTypeClass*>> AIScriptsLists;
		DynamicVectorClass<DynamicVectorClass<HouseTypeClass*>> AIHousesLists;
		DynamicVectorClass<DynamicVectorClass<AITriggerTypeClass*>> AITriggersLists;
		DynamicVectorClass<DynamicVectorClass<std::string>> AIConditionsLists;

		Valueable<int> Storage_TiberiumIndex;
		Nullable<int> InfantryGainSelfHealCap;
		Nullable<int> UnitsGainSelfHealCap;
		Valueable<bool> EnemyInsignia;
		Valueable<bool> ShowAllyDisguiseBlinking;
		Valueable<int> RadApplicationDelay_Building;
		Valueable<bool> RadWarhead_Detonate;
		Valueable<bool> RadHasOwner;
		Valueable<bool> RadHasInvoker;
		Valueable<double> JumpjetCrash;
		Valueable<bool> JumpjetNoWobbles;
		Valueable<bool> JumpjetAllowLayerDeviation;
		Valueable<bool> JumpjetTurnToTarget;
		PhobosFixedString<32u> MissingCameo;
		Valueable<Vector3D<int>> Pips_Shield;
		Nullable<SHPStruct*> Pips_Shield_Background;
		Valueable<Vector3D<int>> Pips_Shield_Building;
		Nullable<int> Pips_Shield_Building_Empty;
		Valueable<int> PlacementPreview_Grid_Translucent;
		Valueable<int> PlacementPreview_Building_Translucent;

		Valueable<bool> UseSelectBox;
		Valueable<SHPStruct*> SelectBox_Shape_Infantry;
		CustomPalette SelectBox_Palette_Infantry;
		Nullable<Vector3D<int>> SelectBox_Frame_Infantry;
		Nullable<Vector2D<int>> SelectBox_DrawOffset_Infantry;
		Valueable<SHPStruct*> SelectBox_Shape_Unit;
		CustomPalette SelectBox_Palette_Unit;
		Nullable<Vector3D<int>> SelectBox_Frame_Unit;
		Nullable<Vector2D<int>> SelectBox_DrawOffset_Unit;
		Nullable<int> SelectBox_TranslucentLevel;
		Valueable<AffectedHouse> SelectBox_CanSee;
		Valueable<bool> SelectBox_CanObserverSee;

		Valueable<Point2D> Pips_SelfHeal_Infantry;
		Valueable<Point2D> Pips_SelfHeal_Units;
		Valueable<Point2D> Pips_SelfHeal_Buildings;
		Valueable<Point2D> Pips_SelfHeal_Infantry_Offset;
		Valueable<Point2D> Pips_SelfHeal_Units_Offset;
		Valueable<Point2D> Pips_SelfHeal_Buildings_Offset;
		Valueable<bool> IronCurtain_KeptOnDeploy;

		ValueableVector<DigitalDisplayTypeClass*> Buildings_DefaultDigitalDisplayTypes;
		ValueableVector<DigitalDisplayTypeClass*> Infantry_DefaultDigitalDisplayTypes;
		ValueableVector<DigitalDisplayTypeClass*> Vehicles_DefaultDigitalDisplayTypes;
		ValueableVector<DigitalDisplayTypeClass*> Aircraft_DefaultDigitalDisplayTypes;

		class HugeBarData
		{
		public:

			Valueable<double> HugeBar_RectWidthPercentage;
			Valueable<Point2D> HugeBar_RectWH;
			Damageable<ColorStruct> HugeBar_Pips_Color1;
			Damageable<ColorStruct> HugeBar_Pips_Color2;
			
			Valueable<SHPStruct*> HugeBar_Shape;
			Valueable<SHPStruct*> HugeBar_Pips_Shape;
			CustomPalette HugeBar_Palette;
			CustomPalette HugeBar_Pips_Palette;
			Damageable<int> HugeBar_Frame;
			Damageable<int> HugeBar_Pips_Frame;
			Valueable<int> HugeBar_Pips_Interval;
			
			Valueable<Point2D> HugeBar_Offset;
			Nullable<Point2D> HugeBar_Pips_Offset;
			Valueable<int> HugeBar_Pips_Num;

			Damageable<ColorStruct> Value_Text_Color;

			Valueable<SHPStruct*> Value_Shape;
			CustomPalette Value_Palette;
			Valueable<int> Value_Num_BaseFrame;
			Valueable<int> Value_Sign_BaseFrame;
			Valueable<int> Value_Shape_Interval;

			Valueable<bool> DisplayValue;
			Valueable<bool> Value_Percentage;
			Valueable<Point2D> Value_Offset;
			Anchor Anchor;
			DisplayInfoType InfoType;

			HugeBarData() = default;
			HugeBarData(DisplayInfoType infoType);

			void LoadFromINI(CCINIClass* pINI);

			void Load(PhobosStreamReader& stm);
			void Save(PhobosStreamWriter& stm);

		private:

			template <typename T>
			void Serialize(T& stm);
		};

		std::vector<std::unique_ptr<HugeBarData>> HugeBar_Config;

		Valueable<bool> CustomHealthBar;
		Nullable<Vector3D<int>> Pips;
		Nullable<Vector3D<int>> Pips_Buildings;

		Nullable<GScreenAnimTypeClass*> GScreenAnimType;
		int ShowAnim_FrameKeep_Check;
		int ShowAnim_CurrentFrameIndex;

		Valueable<IronCurtainAffects> IronCurtainToOrganic;
		Valueable<bool> Warheads_DecloakDamagedTargets;
		Valueable<bool> Warheads_CanBeDodge;

		ExtData(RulesClass* OwnerObject) : Extension<RulesClass>(OwnerObject)
			, Storage_TiberiumIndex { -1 }
			, InfantryGainSelfHealCap {}
			, UnitsGainSelfHealCap {}
			, EnemyInsignia { true }
			, ShowAllyDisguiseBlinking { false }

			, RadApplicationDelay_Building { 0 }
			, RadWarhead_Detonate { false }
			, RadHasOwner { false }
			, RadHasInvoker { false }

			, JumpjetCrash { 5.0 }
			, JumpjetNoWobbles { false }
			, JumpjetAllowLayerDeviation { true }
			, JumpjetTurnToTarget { false }

			, UseSelectBox { false }
			, SelectBox_Shape_Infantry { FileSystem::LoadSHPFile("select.shp") }
			, SelectBox_Palette_Infantry {}
			, SelectBox_Frame_Infantry { { 0, 0, 0 } }
			, SelectBox_DrawOffset_Infantry { { 0, 0 } }
			, SelectBox_Shape_Unit { FileSystem::LoadSHPFile("select.shp") }
			, SelectBox_Palette_Unit {}
			, SelectBox_Frame_Unit { { 3, 3, 3 } }
			, SelectBox_DrawOffset_Unit { { 0, 0 } }
			, SelectBox_TranslucentLevel { 0 }
			, SelectBox_CanSee { AffectedHouse::Owner }
			, SelectBox_CanObserverSee { true }

			, MissingCameo { "xxicon.shp" }

			, Pips_Shield { { 16,16,16 } }
			, Pips_Shield_Background { }
			, Pips_Shield_Building { { 5,5,5 } }
			, Pips_Shield_Building_Empty { }

			, PlacementPreview_Grid_Translucent { 0 }
			, PlacementPreview_Building_Translucent { 75 }

			, Pips_SelfHeal_Infantry { { 13, 20 } }
			, Pips_SelfHeal_Units { { 13, 20 } }
			, Pips_SelfHeal_Buildings { { 13, 20 } }
			, Pips_SelfHeal_Infantry_Offset { { 25, -35 } }
			, Pips_SelfHeal_Units_Offset { { 33, -32 } }
			, Pips_SelfHeal_Buildings_Offset { { 15, 10 } }

			, Buildings_DefaultDigitalDisplayTypes {}
			, Infantry_DefaultDigitalDisplayTypes {}
			, Vehicles_DefaultDigitalDisplayTypes {}
			, Aircraft_DefaultDigitalDisplayTypes {}

			, HugeBar_Config()

			, CustomHealthBar { false }
			, Pips { { 16, 17, 18 } }
			, Pips_Buildings { { 1, 2, 4 } }
			, GScreenAnimType {}
			, ShowAnim_FrameKeep_Check { 0 }
			, ShowAnim_CurrentFrameIndex { 0 }
			, IronCurtainToOrganic { IronCurtainAffects::Kill }
			, Warheads_DecloakDamagedTargets { true }
			, Warheads_CanBeDodge { true }
			, IronCurtain_KeptOnDeploy { true }
		{ }

		virtual ~ExtData() = default;

		virtual void LoadFromINIFile(CCINIClass* pINI) override;
		virtual void LoadBeforeTypeData(RulesClass* pThis, CCINIClass* pINI);
		virtual void LoadAfterTypeData(RulesClass* pThis, CCINIClass* pINI);
		virtual void InitializeConstants() override;
		void InitializeAfterTypeData(RulesClass* pThis);

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

private:
	static std::unique_ptr<ExtData> Data;

public:
	static IStream* g_pStm;

	static void Allocate(RulesClass* pThis);
	static void Remove(RulesClass* pThis);

	static void LoadFromINIFile(RulesClass* pThis, CCINIClass* pINI);
	static void LoadBeforeTypeData(RulesClass* pThis, CCINIClass* pINI);
	static void LoadAfterTypeData(RulesClass* pThis, CCINIClass* pINI);

	static ExtData* Global()
	{
		return Data.get();
	}

	static void Clear()
	{
		Allocate(RulesClass::Instance);
	}

	static void PointerGotInvalid(void* ptr, bool removed)
	{
		Global()->InvalidatePointer(ptr, removed);
	}

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static bool DetailsCurrentlyEnabled();
	static bool DetailsCurrentlyEnabled(int minDetailLevel);

	static void RunAnim();
};
