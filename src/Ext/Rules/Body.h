#pragma once

#include <CCINIClass.h>
#include <RulesClass.h>
#include <AITriggerTypeClass.h>

#include <Utilities/Container.h>
#include <Utilities/Constructs.h>
#include <Utilities/Template.h>

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

		Valueable<int> Storage_TiberiumIndex;
		Nullable<int> InfantryGainSelfHealCap;
		Nullable<int> UnitsGainSelfHealCap;
		Valueable<bool> EnemyInsignia;
		Valueable<bool> ShowAllyDisguiseBlinking;
		Valueable<int> RadApplicationDelay_Building;
		Valueable<double> JumpjetCrash;
		Valueable<bool> JumpjetNoWobbles;
		Valueable<bool> JumpjetAllowLayerDeviation;
		Valueable<bool> JumpjetTurnToTarget;
		PhobosFixedString<32u> MissingCameo;
		Valueable<Vector3D<int>> Pips_Shield;
		Nullable<SHPStruct*> Pips_Shield_Background;
		Valueable<Vector3D<int>> Pips_Shield_Building;
		Nullable<int> Pips_Shield_Building_Empty;
		Valueable<int> PlacementGrid_TranslucentLevel;
		Valueable<int> BuildingPlacementPreview_TranslucentLevel;

		SHPStruct* SHP_SelectBrdSHP_INF;
		ConvertClass* SHP_SelectBrdPAL_INF;
		SHPStruct* SHP_SelectBrdSHP_UNIT;
		ConvertClass* SHP_SelectBrdPAL_UNIT;

		Valueable<bool> UseSelectBrd;
		PhobosFixedString<32U> SelectBrd_SHP_Infantry;
		PhobosFixedString<32U> SelectBrd_PAL_Infantry;
		Nullable<Vector3D<int>> SelectBrd_Frame_Infantry;
		Nullable<Vector2D<int>> SelectBrd_DrawOffset_Infantry;
		PhobosFixedString<32U> SelectBrd_SHP_Unit;
		PhobosFixedString<32U> SelectBrd_PAL_Unit;
		Nullable<Vector3D<int>> SelectBrd_Frame_Unit;
		Nullable<Vector2D<int>> SelectBrd_DrawOffset_Unit;
		Nullable<int> SelectBrd_DefaultTranslucentLevel;
		Valueable<bool> SelectBrd_DefaultShowEnemy;

		Valueable<Point2D> Pips_SelfHeal_Infantry;
		Valueable<Point2D> Pips_SelfHeal_Units;
		Valueable<Point2D> Pips_SelfHeal_Buildings;
		Valueable<Point2D> Pips_SelfHeal_Infantry_Offset;
		Valueable<Point2D> Pips_SelfHeal_Units_Offset;
		Valueable<Point2D> Pips_SelfHeal_Buildings_Offset;

		ValueableVector<DigitalDisplayTypeClass*> Buildings_DefaultDigitalDisplayTypes;
		ValueableVector<DigitalDisplayTypeClass*> Infantrys_DefaultDigitalDisplayTypes;
		ValueableVector<DigitalDisplayTypeClass*> Units_DefaultDigitalDisplayTypes;
		ValueableVector<DigitalDisplayTypeClass*> Aircrafts_DefaultDigitalDisplayTypes;

		Valueable<Vector3D<int>> HugeHP_HighColor1;
		Valueable<Vector3D<int>> HugeHP_HighColor2;
		Valueable<Vector3D<int>> HugeHP_MidColor1;
		Valueable<Vector3D<int>> HugeHP_MidColor2;
		Valueable<Vector3D<int>> HugeHP_LowColor1;
		Valueable<Vector3D<int>> HugeHP_LowColor2;
		Nullable<Vector2D<int>> HugeHP_BorderWH;
		Nullable<Vector2D<int>> HugeHP_RectWH;
		Nullable<int> HugeHP_RectCount;
		Valueable<Vector2D<int>> HugeHP_ShowOffset;
		Valueable<Vector3D<int>> HugeHP_HighValueColor;
		Valueable<Vector3D<int>> HugeHP_MidValueColor;
		Valueable<Vector3D<int>> HugeHP_LowValueColor;
		Valueable<bool> HugeHP_UseSHPShowValue;
		Valueable<bool> HugeHP_CustomSHPShowBar;
		Valueable<bool> HugeSP_CustomSHPShowBar;
		Valueable<bool> HugeHP_UseSHPShowBar;
		PhobosFixedString<0x20> HugeHP_ShowValueSHP;
		PhobosFixedString<0x20> HugeHP_ShowCustomSHP;
		PhobosFixedString<0x20> HugeSP_ShowCustomSHP;
		PhobosFixedString<0x20> HugeHP_ShowBarSHP;
		PhobosFixedString<0x20> HugeHP_ShowPipsSHP;
		PhobosFixedString<0x20> HugeHP_ShowValuePAL;
		PhobosFixedString<0x20> HugeHP_ShowCustomPAL;
		PhobosFixedString<0x20> HugeSP_ShowCustomPAL;
		PhobosFixedString<0x20> HugeHP_ShowBarPAL;
		PhobosFixedString<0x20> HugeHP_ShowPipsPAL;
		Valueable<int> HugeHP_SHPNumberWidth;
		Valueable<int> HugeHP_SHPNumberInterval;
		Valueable<Vector2D<int>> HugeHP_ShowValueOffset;

		Valueable<Vector3D<int>> HugeSP_HighColor1;
		Valueable<Vector3D<int>> HugeSP_HighColor2;
		Valueable<Vector3D<int>> HugeSP_MidColor1;
		Valueable<Vector3D<int>> HugeSP_MidColor2;
		Valueable<Vector3D<int>> HugeSP_LowColor1;
		Valueable<Vector3D<int>> HugeSP_LowColor2;
		Nullable<Vector2D<int>> HugeSP_BorderWH;
		Nullable<Vector2D<int>> HugeSP_RectWH;
		Nullable<int> HugeSP_RectCount;
		Valueable<Vector2D<int>> HugeSP_ShowOffset;
		Valueable<Vector3D<int>> HugeSP_HighValueColor;
		Valueable<Vector3D<int>> HugeSP_MidValueColor;
		Valueable<Vector3D<int>> HugeSP_LowValueColor;
		Valueable<bool> HugeSP_UseSHPShowValue;
		PhobosFixedString<0x20> HugeSP_ShowValueSHP;
		PhobosFixedString<0x20> HugeSP_ShowValuePAL;
		Valueable<int> HugeSP_SHPNumberWidth;
		Valueable<int> HugeSP_SHPNumberInterval;
		Valueable<Vector2D<int>> HugeSP_ShowValueOffset;
		Nullable<int> HugeHP_PipWidth;
		Nullable<int> HugeHP_PipsCount;
		Valueable<Vector2D<int>> HugeHP_PipsOffset;
		Valueable<Vector2D<int>> HugeHP_PipToPipOffset;
		Nullable<int> HugeSP_PipWidth;
		Nullable<int> HugeSP_PipsCount;
		Valueable<Vector2D<int>> HugeSP_PipsOffset;
		Valueable<Vector2D<int>> HugeSP_PipToPipOffset;
		Valueable<Vector3D<int>> HugeHP_BarFrames;
		Valueable<Vector3D<int>> HugeHP_PipsFrames;
		Valueable<Vector3D<int>> HugeSP_BarFrames;
		Valueable<Vector3D<int>> HugeSP_PipsFrames;
		Valueable<int> HugeSP_BarFrameEmpty;
		Valueable<bool> HugeSP_ShowValueAlways;
		Valueable<bool> HugeHP_DrawOrderReverse;

		SHPStruct* SHP_HugeHP;
		ConvertClass* PAL_HugeHP;
		SHPStruct* SHP_HugeHPCustom;
		SHPStruct* SHP_HugeSPCustom;
		ConvertClass* PAL_HugeHPCustom;
		ConvertClass* PAL_HugeSPCustom;
		SHPStruct* SHP_HugeHPBar;
		ConvertClass* PAL_HugeHPBar;
		SHPStruct* SHP_HugeHPPips;
		ConvertClass* PAL_HugeHPPips;
		SHPStruct* SHP_HugeSP;
		ConvertClass* PAL_HugeSP;

		Valueable<bool> CustomHealthBar;
		Nullable<Vector3D<int>> Pips;
		Nullable<Vector3D<int>> Pips_Buildings;

		Nullable<GScreenAnimTypeClass*> GScreenAnimType;
        int ShowAnim_FrameKeep_Check;
        int ShowAnim_CurrentFrameIndex;

		ExtData(RulesClass* OwnerObject) : Extension<RulesClass>(OwnerObject)
			, Storage_TiberiumIndex { -1 }
			, InfantryGainSelfHealCap {}
			, UnitsGainSelfHealCap {}
			, EnemyInsignia { true }
			, ShowAllyDisguiseBlinking { false }
			, RadApplicationDelay_Building { 0 }
			, JumpjetCrash { 5.0 }
			, JumpjetNoWobbles { false }
			, JumpjetAllowLayerDeviation { true }
			, JumpjetTurnToTarget { false }
			, SHP_SelectBrdSHP_INF { nullptr }
			, SHP_SelectBrdPAL_INF { nullptr }
			, SHP_SelectBrdSHP_UNIT { nullptr }
			, SHP_SelectBrdPAL_UNIT { nullptr }
			, UseSelectBrd { false }
			, SelectBrd_SHP_Infantry { "select.shp" }
			, SelectBrd_PAL_Infantry { "palette.pal" }
			, SelectBrd_Frame_Infantry { {0,0,0} }
			, SelectBrd_DrawOffset_Infantry { {0,0} }
			, SelectBrd_SHP_Unit { "select.shp" }
			, SelectBrd_PAL_Unit { "palette.pal" }
			, SelectBrd_Frame_Unit { {3,3,3} }
			, SelectBrd_DrawOffset_Unit { {0,0} }
			, SelectBrd_DefaultTranslucentLevel { 0 }
			, SelectBrd_DefaultShowEnemy { true }
			, MissingCameo { "xxicon.shp" }
			, Pips_Shield { { 16,16,16 } }
			, Pips_Shield_Background { }
			, Pips_Shield_Building { { 5,5,5 } }
			, Pips_Shield_Building_Empty { }
			, PlacementGrid_TranslucentLevel { 0 }
			, BuildingPlacementPreview_TranslucentLevel { 3 }
			, Pips_SelfHeal_Infantry { { 13, 20 } }
			, Pips_SelfHeal_Units { { 13, 20 } }
			, Pips_SelfHeal_Buildings { { 13, 20 } }
			, Pips_SelfHeal_Infantry_Offset { { 25, -35 } }
			, Pips_SelfHeal_Units_Offset { { 33, -32 } }
			, Pips_SelfHeal_Buildings_Offset { { 15, 10 } }
			, Buildings_DefaultDigitalDisplayTypes {}
			, Infantrys_DefaultDigitalDisplayTypes {}
			, Units_DefaultDigitalDisplayTypes {}
			, Aircrafts_DefaultDigitalDisplayTypes {}
			, HugeHP_HighColor1 { { 0, 255, 0 } }
			, HugeHP_HighColor2 { { 0, 216, 0 } }
			, HugeHP_MidColor1 { { 255, 255, 0 } }
			, HugeHP_MidColor2 { { 252, 180, 0 } }
			, HugeHP_LowColor1 { { 255, 0, 0 } }
			, HugeHP_LowColor2 { { 216, 0, 0 } }
			, HugeHP_BorderWH {}
			, HugeHP_RectWH {}
			, HugeHP_RectCount {}
			, HugeHP_ShowOffset { { 0, 0 } }
			, HugeHP_HighValueColor { { 0, 255, 0 } }
			, HugeHP_MidValueColor { { 255, 255, 0 } }
			, HugeHP_LowValueColor { { 255, 0, 0 } }
			, HugeHP_UseSHPShowValue { false }
			, HugeHP_ShowValueSHP { "number.shp" }
			, HugeHP_ShowValuePAL { "" }
			, HugeHP_SHPNumberInterval { 4 }
			, HugeHP_SHPNumberWidth { 4 }
			, HugeHP_ShowValueOffset { { 0, 0 } }
			, HugeSP_HighColor1 { { 0, 0, 255 } }
			, HugeSP_HighColor2 { { 0, 0, 216 } }
			, HugeSP_MidColor1 { { 0, 0, 255 } }
			, HugeSP_MidColor2 { { 0, 0, 216 } }
			, HugeSP_LowColor1 { { 0, 0, 255 } }
			, HugeSP_LowColor2 { { 0, 0, 216 } }
			, HugeSP_BorderWH {}
			, HugeSP_RectWH {}
			, HugeSP_RectCount {}
			, HugeSP_ShowOffset { { 0, 0 } }
			, HugeSP_HighValueColor { { 0, 0, 255 } }
			, HugeSP_MidValueColor { { 0, 0, 255 } }
			, HugeSP_LowValueColor { { 0, 0, 255 } }
			, HugeSP_UseSHPShowValue { false }
			, HugeSP_ShowValueSHP { "number.shp" }
			, HugeHP_ShowCustomSHP{ "pipsbrd.shp" }
			, HugeSP_ShowCustomSHP{ "pipsbrd.shp" }
			, HugeHP_UseSHPShowBar{ false }
			, HugeHP_CustomSHPShowBar{ false }
			, HugeSP_CustomSHPShowBar{ false }
			, HugeHP_ShowBarSHP { "pipsbrd.shp" }
			, HugeHP_ShowPipsSHP { "pips.shp" }
			, HugeSP_ShowValuePAL { "" }
			, HugeHP_ShowCustomPAL{ "" }
			, HugeSP_ShowCustomPAL{ "" }
			, HugeHP_ShowBarPAL { "" }
			, HugeHP_ShowPipsPAL { "" }
			, HugeSP_SHPNumberInterval { 4 }
			, HugeSP_SHPNumberWidth { 4 }
			, HugeSP_ShowValueOffset { { 0, 0 } }
			, SHP_HugeHPCustom{ nullptr }
			, SHP_HugeSPCustom{ nullptr }
			, SHP_HugeHPBar { nullptr }
			, SHP_HugeHPPips { nullptr }
			, SHP_HugeHP { nullptr }
			, PAL_HugeSP { nullptr }
			, CustomHealthBar { false }
			, Pips { { 16, 17, 18 } }
			, Pips_Buildings { { 1, 2, 4 } }
			, HugeHP_PipWidth { }
			, HugeHP_PipsCount { }
			, HugeHP_PipsOffset { { 0, 0 } }
			, HugeHP_PipToPipOffset { { 0, 0 } }
			, HugeSP_PipWidth { }
			, HugeSP_PipsCount { }
			, HugeSP_PipsOffset { { 0, 0 } }
			, HugeSP_PipToPipOffset { { 0, 0 } }
			, HugeHP_BarFrames { { -1, -1, -1 } }
			, HugeHP_PipsFrames { { -1, -1, -1 } }
			, HugeSP_BarFrames { { -1, -1, -1 } }
			, HugeSP_PipsFrames { { -1, -1, -1 } }
			, HugeSP_BarFrameEmpty { -1 }
			, HugeSP_ShowValueAlways { false }
			, HugeHP_DrawOrderReverse { false }
			, GScreenAnimType {}
			, ShowAnim_FrameKeep_Check { 0 }
			, ShowAnim_CurrentFrameIndex { 0 }
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
