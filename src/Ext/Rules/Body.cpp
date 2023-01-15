#include "Body.h"
#include <Ext/Side/Body.h>
#include <Utilities/TemplateDef.h>
#include <FPSCounter.h>
#include <GameOptionsClass.h>
#include <HouseTypeClass.h>
#include <GameStrings.h>

#include <New/Type/RadTypeClass.h>
#include <New/Type/ShieldTypeClass.h>
#include <New/Type/LaserTrailTypeClass.h>
#include <New/Type/AttachmentTypeClass.h>
#include <New/Type/BannerTypeClass.h>
#include <New/Type/DigitalDisplayTypeClass.h>
#include <New/Entity/ExternVariableClass.h>
#include <New/Type/IonCannonTypeClass.h>
#include <New/Type/GScreenAnimTypeClass.h>
#include <New/Type/AttachEffectTypeClass.h>
#include <New/Type/TemperatureTypeClass.h>
#include <New/Armor/Armor.h>

#include <Utilities/EnumFunctions.h>
#include <WWMouseClass.h>

template<> const DWORD Extension<RulesClass>::Canary = 0x12341234;
std::unique_ptr<RulesExt::ExtData> RulesExt::Data = nullptr;

void RulesExt::Allocate(RulesClass* pThis)
{
	Data = std::make_unique<RulesExt::ExtData>(pThis);
}

void RulesExt::Remove(RulesClass* pThis)
{
	Data = nullptr;
}

void RulesExt::LoadFromINIFile(RulesClass* pThis, CCINIClass* pINI)
{
	Data->LoadFromINI(pINI);
}

void RulesExt::LoadBeforeTypeData(RulesClass* pThis, CCINIClass* pINI)
{
	CustomArmor::LoadFromINIList(pINI);
	AttachEffectTypeClass::LoadFromINIList(pINI);
	DigitalDisplayTypeClass::LoadFromINIList(pINI);
	RadTypeClass::LoadFromINIList(pINI);
	ShieldTypeClass::LoadFromINIList(pINI);
	LaserTrailTypeClass::LoadFromINIList(&CCINIClass::INI_Art.get());
	AttachmentTypeClass::LoadFromINIList(pINI);
	BannerTypeClass::LoadFromINIList(pINI);
	IonCannonTypeClass::LoadFromINIList(pINI);
	GScreenAnimTypeClass::LoadFromINIList(pINI);
	TemperatureTypeClass::LoadFromINIList(pINI);

	ExternVariableClass::LoadVariablesFromDir("*.ini");

	Data->LoadBeforeTypeData(pThis, pINI);
}

void RulesExt::LoadAfterTypeData(RulesClass* pThis, CCINIClass* pINI)
{
	if (pINI == CCINIClass::INI_Rules)
		Data->InitializeAfterTypeData(pThis);

	Data->LoadAfterTypeData(pThis, pINI);
}

void RulesExt::ExtData::InitializeConstants()
{

}

// earliest loader - can't really do much because nothing else is initialized yet, so lookups won't work
void RulesExt::ExtData::LoadFromINIFile(CCINIClass* pINI)
{

}

namespace ObjectTypeParser
{
	template<typename T>
	void Exec(CCINIClass* pINI, DynamicVectorClass<DynamicVectorClass<T*>>& nVecDest, const char* pKey, bool bDebug = true)
	{
		for (int i = 0; i < pINI->GetKeyCount(pKey); ++i)
		{
			DynamicVectorClass<T*> _Buffer;
			char* context = nullptr;
			pINI->ReadString(pKey, pINI->GetKeyName(pKey, i), "", Phobos::readBuffer);

			for (char* cur = strtok_s(Phobos::readBuffer, Phobos::readDelims, &context);
				cur; cur = strtok_s(nullptr, Phobos::readDelims, &context))
			{
				T* buffer = nullptr;

				if (Parser<T*>::TryParse(cur, &buffer))
					_Buffer.AddItem(buffer);
				else
				{
					if (bDebug)
						Debug::Log("ObjectTypeParser DEBUG: [%s][%d]: Error parsing [%s]\n", pKey, nVecDest.Count, cur);
				}
			}

			nVecDest.AddItem(_Buffer);
			_Buffer.Clear();
		}
	}
};

void RulesExt::ExtData::LoadBeforeTypeData(RulesClass* pThis, CCINIClass* pINI)
{
	RulesExt::ExtData* pData = RulesExt::Global();

	if (!pData)
		return;

	const char* sectionAITargetTypes = "AITargetTypes";
	const char* sectionAIScriptsList = "AIScriptsList";
	const char* sectionAIHousesList = "AIHousesList";
	const char* sectionAITriggersList = "AITriggersList";
	const char* sectionAIConditionsList = "AIConditionsList";

	INI_EX exINI(pINI);

	this->Storage_TiberiumIndex.Read(exINI, GameStrings::General, "Storage.TiberiumIndex");
	this->InfantryGainSelfHealCap.Read(exINI, GameStrings::General, "InfantryGainSelfHealCap");
	this->UnitsGainSelfHealCap.Read(exINI, GameStrings::General, "UnitsGainSelfHealCap");
	this->EnemyInsignia.Read(exINI, GameStrings::General, "EnemyInsignia");
	this->ShowAllyDisguiseBlinking.Read(exINI, GameStrings::General, "ShowAllyDisguiseBlinking");
	this->JumpjetAllowLayerDeviation.Read(exINI, "JumpjetControls", "AllowLayerDeviation");
	this->JumpjetTurnToTarget.Read(exINI, "JumpjetControls", "TurnToTarget");
	this->UseGlobalRadApplicationDelay.Read(exINI, GameStrings::Radiation, "UseGlobalRadApplicationDelay");
	this->RadApplicationDelay_Building.Read(exINI, GameStrings::Radiation, "RadApplicationDelay.Building");
	this->RadWarhead_Detonate.Read(exINI, GameStrings::Radiation, "RadSiteWarhead.Detonate");
	this->RadHasOwner.Read(exINI, GameStrings::Radiation, "RadHasOwner");
	this->RadHasInvoker.Read(exINI, GameStrings::Radiation, "RadHasInvoker");
	this->MissingCameo.Read(pINI, GameStrings::AudioVisual, "MissingCameo");
	this->JumpjetTurnToTarget.Read(exINI, "JumpjetControls", "TurnToTarget");
	this->PlacementPreview.Read(exINI, GameStrings::AudioVisual, "PlacementPreview");
	this->PlacementPreview_Translucency.Read(exINI, GameStrings::AudioVisual, "PlacementPreview.Translucency");
	this->PlacementGrid_Translucency.Read(exINI, GameStrings::AudioVisual, "PlacementGrid.Translucency");
	this->Pips_Shield.Read(exINI, GameStrings::AudioVisual, "Pips.Shield");
	this->Pips_Shield_Background.Read(exINI, GameStrings::AudioVisual, "Pips.Shield.Background");
	this->Pips_Shield_Building.Read(exINI, GameStrings::AudioVisual, "Pips.Shield.Building");
	this->Pips_Shield_Building_Empty.Read(exINI, GameStrings::AudioVisual, "Pips.Shield.Building.Empty");

	this->UseSelectBox.Read(exINI, GameStrings::AudioVisual, "UseSelectBox");
	this->SelectBox_Shape_Infantry.Read(exINI, GameStrings::AudioVisual, "SelectBox.Shape.Infantry");
	this->SelectBox_Palette_Infantry.LoadFromINI(pINI, GameStrings::AudioVisual, "SelectBox.Palette.Infantry");
	this->SelectBox_Frame_Infantry.Read(exINI, GameStrings::AudioVisual, "SelectBox.Frame.Infantry");
	this->SelectBox_DrawOffset_Infantry.Read(exINI, GameStrings::AudioVisual, "SelectBox.DrawOffset.Infantry");
	this->SelectBox_Shape_Unit.Read(exINI, GameStrings::AudioVisual, "SelectBox.Shape.Unit");
	this->SelectBox_Palette_Unit.LoadFromINI(pINI, GameStrings::AudioVisual, "SelectBox.Palette.Unit");
	this->SelectBox_Frame_Unit.Read(exINI, GameStrings::AudioVisual, "SelectBox.Frame.Unit");
	this->SelectBox_DrawOffset_Unit.Read(exINI, GameStrings::AudioVisual, "SelectBox.DrawOffset.Unit");
	this->SelectBox_TranslucentLevel.Read(exINI, GameStrings::AudioVisual, "SelectBox.TranslucentLevel");
	this->SelectBox_CanSee.Read(exINI, GameStrings::AudioVisual, "SelectBox.CanSee");
	this->SelectBox_CanObserverSee.Read(exINI, GameStrings::AudioVisual, "SelectBox.CanObserverSee");

	this->PlacementPreview.Read(exINI, GameStrings::AudioVisual, "PlacementPreview");
	this->PlacementPreview_Translucency.Read(exINI, GameStrings::AudioVisual, "PlacementPreview.Translucency");
	this->PlacementGrid_Translucency.Read(exINI, GameStrings::AudioVisual, "PlacementGrid.Translucency");

	this->Pips_SelfHeal_Infantry.Read(exINI, GameStrings::AudioVisual, "Pips.SelfHeal.Infantry");
	this->Pips_SelfHeal_Units.Read(exINI, GameStrings::AudioVisual, "Pips.SelfHeal.Units");
	this->Pips_SelfHeal_Buildings.Read(exINI, GameStrings::AudioVisual, "Pips.SelfHeal.Buildings");
	this->Pips_SelfHeal_Infantry_Offset.Read(exINI, GameStrings::AudioVisual, "Pips.SelfHeal.Infantry.Offset");
	this->Pips_SelfHeal_Units_Offset.Read(exINI, GameStrings::AudioVisual, "Pips.SelfHeal.Units.Offset");
	this->Pips_SelfHeal_Buildings_Offset.Read(exINI, GameStrings::AudioVisual, "Pips.SelfHeal.Buildings.Offset");

	this->IronCurtain_EffectOnOrganics.Read(exINI, GameStrings::CombatDamage, "IronCurtain.EffectOnOrganics");
	this->IronCurtain_KillOrganicsWarhead.Read(exINI, GameStrings::CombatDamage, "IronCurtain.KillOrganicsWarhead");
	this->IronCurtain_KeptOnDeploy.Read(exINI, GameStrings::CombatDamage, "IronCurtain.KeptOnDeploy");

	this->Buildings_DefaultDigitalDisplayTypes.Read(exINI, GameStrings::AudioVisual, "Buildings.DefaultDigitalDisplayTypes");
	this->Infantry_DefaultDigitalDisplayTypes.Read(exINI, GameStrings::AudioVisual, "Infantry.DefaultDigitalDisplayTypes");
	this->Vehicles_DefaultDigitalDisplayTypes.Read(exINI, GameStrings::AudioVisual, "Vehicles.DefaultDigitalDisplayTypes");
	this->Aircraft_DefaultDigitalDisplayTypes.Read(exINI, GameStrings::AudioVisual, "Aircraft.DefaultDigitalDisplayTypes");

	if (HugeBar_Config.empty())
	{
		this->HugeBar_Config.emplace_back(new HugeBarData(DisplayInfoType::Health));
		this->HugeBar_Config.emplace_back(new HugeBarData(DisplayInfoType::Shield));
	}

	this->HugeBar_Config[0]->LoadFromINI(pINI);
	this->HugeBar_Config[1]->LoadFromINI(pINI);

	this->CustomHealthBar.Read(exINI, GameStrings::AudioVisual, "CustomHealthBar");
	this->Pips.Read(exINI, GameStrings::AudioVisual, "HealthBar.Pips");
	this->Pips_Buildings.Read(exINI, GameStrings::AudioVisual, "HealthBar.Pips.Buildings");

	this->GScreenAnimType.Read(exINI, GameStrings::AudioVisual, "GScreenAnimType", true);
	this->Warheads_DecloakDamagedTargets.Read(exINI, GameStrings::CombatDamage, "Warheads.DecloakDamagedTargets");
	this->Warheads_CanBeDodge.Read(exINI, GameStrings::CombatDamage, "Warheads.CanBeDodge");

	this->DirectionalArmor.Read(exINI, GameStrings::CombatDamage, "DirectionalArmor");
	this->DirectionalArmor_FrontMultiplier.Read(exINI, GameStrings::CombatDamage, "DirectionalArmor.FrontMultiplier");
	this->DirectionalArmor_SideMultiplier.Read(exINI, GameStrings::CombatDamage, "DirectionalArmor.SideMultiplier");
	this->DirectionalArmor_BackMultiplier.Read(exINI, GameStrings::CombatDamage, "DirectionalArmor.BackMultiplier");
	this->DirectionalArmor_FrontField.Read(exINI, GameStrings::CombatDamage, "DirectionalArmor.FrontField");
	this->DirectionalArmor_BackField.Read(exINI, GameStrings::CombatDamage, "DirectionalArmor.BackField");
	this->DirectionalWarhead.Read(exINI, GameStrings::CombatDamage, "DirectionalWarhead");
	this->Directional_Multiplier.Read(exINI, GameStrings::CombatDamage, "Directional.Multiplier");

	this->Parachute_OpenHeight.Read(exINI, GameStrings::General, "Parachute.OpenHeight");

	this->DirectionalArmor_FrontField = Math::min(this->DirectionalArmor_FrontField, 1.0f);
	this->DirectionalArmor_FrontField = Math::max(this->DirectionalArmor_FrontField, 0.0f);
	this->DirectionalArmor_BackField = Math::min(this->DirectionalArmor_BackField, 1.0f);
	this->DirectionalArmor_BackField = Math::max(this->DirectionalArmor_BackField, 0.0f);

	this->ToolTip_Background_Color.Read(exINI, GameStrings::AudioVisual, "ToolTip.Background.Color");
	this->ToolTip_Background_Opacity.Read(exINI, GameStrings::AudioVisual, "ToolTip.Background.Opacity");
	this->ToolTip_Background_BlurSize.Read(exINI, GameStrings::AudioVisual, "ToolTip.Background.BlurSize");
	this->RadialIndicatorVisibility.Read(exINI, GameStrings::AudioVisual, "RadialIndicatorVisibility");

	this->AllowParallelAIQueues.Read(exINI, "GlobalControls", "AllowParallelAIQueues");
	this->ForbidParallelAIQueues_Aircraft.Read(exINI, "GlobalControls", "ForbidParallelAIQueues.Infantry");
	this->ForbidParallelAIQueues_Building.Read(exINI, "GlobalControls", "ForbidParallelAIQueues.Building");
	this->ForbidParallelAIQueues_Infantry.Read(exINI, "GlobalControls", "ForbidParallelAIQueues.Infantry");
	this->ForbidParallelAIQueues_Navy.Read(exINI, "GlobalControls", "ForbidParallelAIQueues.Navy");
	this->ForbidParallelAIQueues_Vehicle.Read(exINI, "GlobalControls", "ForbidParallelAIQueues.Vehicle");

	this->IronCurtain_KeptOnDeploy.Read(exINI, GameStrings::CombatDamage, "IronCurtain.KeptOnDeploy");
	this->ROF_RandomDelay.Read(exINI, GameStrings::CombatDamage, "ROF.RandomDelay");

	this->VeteranAnim.Read(exINI, GameStrings::General, "VeteranAnim");
	this->EliteAnim.Read(exINI, GameStrings::General, "EliteAnim");

	this->AllowPlanningMode_Aircraft.Read(exINI, "GlobalControls", "AllowPlanningMode.Aircraft");
	this->AllowPlanningMode_Building.Read(exINI, "GlobalControls", "AllowPlanningMode.Building");

	this->TextType_Ready.Read(exINI, GameStrings::AudioVisual, "TextType.Ready");
	this->TextType_Hold_Multiple.Read(exINI, GameStrings::AudioVisual, "TextType.Hold.Multiple");
	this->TextType_Hold_Singular.Read(exINI, GameStrings::AudioVisual, "TextType.Hold.Singular");
	this->TextColor_Ready.Read(exINI, GameStrings::AudioVisual, "TextColor.Ready");
	this->TextColor_Hold_Multiple.Read(exINI, GameStrings::AudioVisual, "TextColor.Hold.Multiple");
	this->TextColor_Hold_Singular.Read(exINI, GameStrings::AudioVisual, "TextColor.Hold.Singular");

	this->ReadyShapeType.Read(exINI, GameStrings::AudioVisual, "ReadyShapeType");

	this->TimerPercentage.Read(exINI, GameStrings::AudioVisual, "Timer.Percentage");
	this->TimerXOffset.Read(exINI, GameStrings::AudioVisual, "Timer.XOffset");
	this->TimerFlashFrames.Read(exINI, GameStrings::AudioVisual, "Timer.FlashFrames");
	this->TimerPrecision.Read(exINI, GameStrings::AudioVisual, "Timer.Precision");
	this->ReadyShapeType_SW.Read(exINI, GameStrings::AudioVisual, "ReadyShapeType.SW");
	this->TextType_SW.Read(exINI, GameStrings::AudioVisual, "TextType.SW");

	this->OnFire.Read(exINI, GameStrings::AudioVisual, "OnFire");

	// Section AITargetTypes
	/*
	int itemsCount = pINI->GetKeyCount(sectionAITargetTypes);
	for (int i = 0; i < itemsCount; ++i)
	{
		DynamicVectorClass<TechnoTypeClass*> objectsList;
		char* context = nullptr;
		pINI->ReadString(sectionAITargetTypes, pINI->GetKeyName(sectionAITargetTypes, i), "", Phobos::readBuffer);

		for (char* cur = strtok_s(Phobos::readBuffer, Phobos::readDelims, &context); cur; cur = strtok_s(nullptr, Phobos::readDelims, &context))
		{
			TechnoTypeClass* buffer;
			if (Parser<TechnoTypeClass*>::TryParse(cur, &buffer))
				objectsList.AddItem(buffer);
			else
				Debug::Log("DEBUG: [AITargetTypes][%d]: Error parsing [%s]\n", AITargetTypesLists.Count, cur);
		}

		AITargetTypesLists.AddItem(objectsList);
		objectsList.Clear();
	}
	*/
	ObjectTypeParser::Exec(pINI, AITargetTypesLists, sectionAITargetTypes, true);

	// Section AIScriptsList
	/*
	int scriptitemsCount = pINI->GetKeyCount(sectionAIScriptsList);
	for (int i = 0; i < scriptitemsCount; ++i)
	{
		DynamicVectorClass<ScriptTypeClass*> objectsList;

		char* context = nullptr;
		pINI->ReadString(sectionAIScriptsList, pINI->GetKeyName(sectionAIScriptsList, i), "", Phobos::readBuffer);

		for (char* cur = strtok_s(Phobos::readBuffer, Phobos::readDelims, &context); cur; cur = strtok_s(nullptr, Phobos::readDelims, &context))
		{
			ScriptTypeClass* pNewScript = GameCreate<ScriptTypeClass>(cur);
			objectsList.AddItem(pNewScript);
		}

		AIScriptsLists.AddItem(objectsList);
		objectsList.Clear();
	}
	*/
	ObjectTypeParser::Exec(pINI, AIScriptsLists, sectionAIScriptsList, false);

	// Section AIHousesList
	int houseItemsCount = pINI->GetKeyCount(sectionAIHousesList);
	for (int i = 0; i < houseItemsCount; ++i)
	{
		DynamicVectorClass<HouseTypeClass*> objectsList;

		char* context = nullptr;
		pINI->ReadString(sectionAIHousesList, pINI->GetKeyName(sectionAIHousesList, i), "", Phobos::readBuffer);

		for (char* cur = strtok_s(Phobos::readBuffer, Phobos::readDelims, &context); cur; cur = strtok_s(nullptr, Phobos::readDelims, &context))
		{
			HouseTypeClass* pNewHouse = GameCreate<HouseTypeClass>(cur);
			objectsList.AddItem(pNewHouse);
		}

		AIHousesLists.AddItem(objectsList);
		objectsList.Clear();
	}

	// Section AITriggersList
	int triggerItemsCount = pINI->GetKeyCount(sectionAITriggersList);
	for (int i = 0; i < triggerItemsCount; ++i)
	{
		DynamicVectorClass<AITriggerTypeClass*> objectsList;

		char* context = nullptr;
		pINI->ReadString(sectionAITriggersList, pINI->GetKeyName(sectionAITriggersList, i), "", Phobos::readBuffer);

		for (char* cur = strtok_s(Phobos::readBuffer, Phobos::readDelims, &context); cur; cur = strtok_s(nullptr, Phobos::readDelims, &context))
		{
			AITriggerTypeClass* pNewTrigger = GameCreate<AITriggerTypeClass>(cur);
			objectsList.AddItem(pNewTrigger);
		}

		AITriggersLists.AddItem(objectsList);
		objectsList.Clear();
	}

	// Section AIConditionsList
	int AIConditionsitemsCount = pINI->GetKeyCount(sectionAIConditionsList);
	for (int i = 0; i < AIConditionsitemsCount; ++i)
	{
		DynamicVectorClass<std::string> objectsList;

		char* context = nullptr;
		pINI->ReadString(sectionAIConditionsList, pINI->GetKeyName(sectionAIConditionsList, i), "", Phobos::readBuffer);

		for (char* cur = strtok_s(Phobos::readBuffer, "/", &context); cur; cur = strtok_s(nullptr, "/", &context))
		{
			objectsList.AddItem(cur);
		}

		AIConditionsLists.AddItem(objectsList);
		objectsList.Clear();
	}
}

// this runs between the before and after type data loading methods for rules ini
void RulesExt::ExtData::InitializeAfterTypeData(RulesClass* const pThis)
{

}

// this should load everything that TypeData is not dependant on
// i.e. InfantryElectrocuted= can go here since nothing refers to it
// but [GenericPrerequisites] have to go earlier because they're used in parsing TypeData
void RulesExt::ExtData::LoadAfterTypeData(RulesClass* pThis, CCINIClass* pINI)
{
	RulesExt::ExtData* pData = RulesExt::Global();

	if (!pData)
		return;

	INI_EX exINI(pINI);
}

bool RulesExt::DetailsCurrentlyEnabled()
{
	// not only checks for the min frame rate from the rules, but also whether
	// the low frame rate is actually desired. in that case, don't reduce.
	auto const current = FPSCounter::CurrentFrameRate();
	auto const wanted = static_cast<unsigned int>(
		60 / Math::clamp(GameOptionsClass::Instance->GameSpeed, 1, 6));

	return current >= wanted || current >= Detail::GetMinFrameRate();
}

bool RulesExt::DetailsCurrentlyEnabled(int const minDetailLevel)
{
	return GameOptionsClass::Instance->DetailLevel >= minDetailLevel
		&& DetailsCurrentlyEnabled();
}

void RulesExt::RunAnim()
{
	if (Phobos::Debug_DisplayAnimation)
	{
		GScreenAnimTypeClass* pGlobalAnimType = RulesExt::Global()->GScreenAnimType.Get();

		if (pGlobalAnimType)
		{
			SHPStruct* ShowAnimSHP = pGlobalAnimType->SHP_ShowAnim;
			ConvertClass* ShowAnimPAL = pGlobalAnimType->PAL_ShowAnim;
			if (ShowAnimSHP == nullptr || ShowAnimPAL == nullptr)
				return;

			int frameCurrent = RulesExt::Global()->ShowAnim_CurrentFrameIndex;

			Point2D posAnim;

			if (pGlobalAnimType->ShowAnim_IsOnCursor)
				posAnim = {
					WWMouseClass::Instance->GetX() - ( ShowAnimSHP->Width >> 1 ),
					WWMouseClass::Instance->GetY() - ( ShowAnimSHP->Height >> 1)
				};
			else
				posAnim = {
					( DSurface::Composite->GetWidth() - ShowAnimSHP->Width ) >> 1,
					( DSurface::Composite->GetHeight() - ShowAnimSHP->Height ) >> 1
				};

			posAnim += pGlobalAnimType->ShowAnim_Offset.Get();

			auto const nFlag = BlitterFlags::None | EnumFunctions::GetTranslucentLevel(pGlobalAnimType->ShowAnim_TranslucentLevel.Get());

			DSurface::Composite->DrawSHP(ShowAnimPAL, ShowAnimSHP, frameCurrent, &posAnim, &DSurface::ViewBounds, nFlag,
			0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);

			RulesExt::Global()->ShowAnim_FrameKeep_Check++;
			if (RulesExt::Global()->ShowAnim_FrameKeep_Check >= pGlobalAnimType->ShowAnim_FrameKeep)
			{
				RulesExt::Global()->ShowAnim_CurrentFrameIndex++;
				if (RulesExt::Global()->ShowAnim_CurrentFrameIndex >= ShowAnimSHP->Frames)
					RulesExt::Global()->ShowAnim_CurrentFrameIndex = 0;

				RulesExt::Global()->ShowAnim_FrameKeep_Check = 0;
			}
		}
	}
}

RulesExt::ExtData::HugeBarData::HugeBarData(DisplayInfoType infoType)
	: HugeBar_RectWidthPercentage(0.82)
	, HugeBar_RectWH({ -1, 30 })
	, HugeBar_Frame(-1)
	, HugeBar_Pips_Frame(-1)
	, HugeBar_Pips_Num(100)
	, Value_Shape_Interval(8)
	, Value_Num_BaseFrame(0)
	, Value_Sign_BaseFrame(30)
	, DisplayValue(true)
	, Anchor(HorizontalPosition::Center, VerticalPosition::Top)
	, InfoType(infoType)
{
	switch (infoType)
	{
	case DisplayInfoType::Health:
		HugeBar_Pips_Color1 = Damageable<ColorStruct>({ 0, 255, 0 }, { 255, 255, 0 }, { 255, 0, 0 });
		HugeBar_Pips_Color2 = Damageable<ColorStruct>({ 0, 216, 0 }, { 255, 180, 0 }, { 216, 0, 0 });
		Value_Text_Color = Damageable<ColorStruct>({ 0, 255, 0 }, { 255, 180, 0 }, { 255, 0, 0 });
		break;
	case DisplayInfoType::Shield:
		HugeBar_Pips_Color1 = Damageable<ColorStruct>({ 0, 0, 255 });
		HugeBar_Pips_Color2 = Damageable<ColorStruct>({ 0, 0, 216 });
		Value_Text_Color = Damageable<ColorStruct>({ 0, 0, 216 });
		break;
	default:
		break;
	}
}

void RulesExt::ExtData::HugeBarData::LoadFromINI(CCINIClass* pINI)
{
	char typeName[0x20];

	switch (InfoType)
	{
	case DisplayInfoType::Health:
		strcpy_s(typeName, "Health");
		break;
	case DisplayInfoType::Shield:
		strcpy_s(typeName, "Shield");
		break;
	default:
		return;
	}

	char section[0x20];
	sprintf_s(section, "HugeBar_%s", typeName);
	INI_EX exINI(pINI);

	this->HugeBar_RectWidthPercentage.Read(exINI, section, "HugeBar.RectWidthPercentage");
	this->HugeBar_RectWH.Read(exINI, section, "HugeBar.RectWH");
	this->HugeBar_Pips_Color1.Read(exINI, section, "HugeBar.Pips.Color1.%s");
	this->HugeBar_Pips_Color2.Read(exINI, section, "HugeBar.Pips.Color2.%s");

	this->HugeBar_Shape.Read(exINI, section, "HugeBar.Shape");
	this->HugeBar_Palette.LoadFromINI(pINI, section, "HugeBar.Palette");
	this->HugeBar_Frame.Read(exINI, section, "HugeBar.Frame.%s");
	this->HugeBar_Pips_Shape.Read(exINI, section, "HugeBar.Pips.Shape");
	this->HugeBar_Pips_Palette.LoadFromINI(pINI, section, "HugeBar.Pips.Palette");
	this->HugeBar_Pips_Frame.Read(exINI, section, "HugeBar.Pips.Frame.%s");
	this->HugeBar_Pips_Interval.Read(exINI, section, "HugeBar.Pips.Interval");

	this->HugeBar_Offset.Read(exINI, section, "HugeBar.Offset");
	this->HugeBar_Pips_Offset.Read(exINI, section, "HugeBar.Pips.Offset");
	this->HugeBar_Pips_Num.Read(exINI, section, "HugeBar.Pips.Num");

	this->Value_Text_Color.Read(exINI, section, "Value.Text.Color.%s");

	this->Value_Shape.Read(exINI, section, "Value.Shape");
	this->Value_Palette.LoadFromINI(pINI, section, "Value.Palette");
	this->Value_Num_BaseFrame.Read(exINI, section, "Value.Num.BaseFrame");
	this->Value_Sign_BaseFrame.Read(exINI, section, "Value.Sign.BaseFrame");
	this->Value_Shape_Interval.Read(exINI, section, "Value.Shape.Interval");

	this->DisplayValue.Read(exINI, section, "DisplayValue");
	this->Value_Offset.Read(exINI, section, "Value.Offset");
	this->Value_Percentage.Read(exINI, section, "Value.Percentage");
	this->Anchor.Read(exINI, section, "Anchor.%s");
}

// =============================
// load / save

template <typename T>
void RulesExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->AITriggersLists)
		.Process(this->AITargetTypesLists)
		.Process(this->AIScriptsLists)
		.Process(this->HarvesterTypes)
		.Process(this->AIConditionsLists)
		.Process(this->AIHousesLists)

		.Process(this->Storage_TiberiumIndex)

		.Process(this->InfantryGainSelfHealCap)
		.Process(this->UnitsGainSelfHealCap)
		.Process(this->EnemyInsignia)

		.Process(this->ShowAllyDisguiseBlinking)

		.Process(this->UseSelectBox)
		.Process(this->SelectBox_Shape_Infantry)
		.Process(this->SelectBox_Palette_Infantry)
		.Process(this->SelectBox_Frame_Infantry)
		.Process(this->SelectBox_DrawOffset_Infantry)
		.Process(this->SelectBox_Shape_Unit)
		.Process(this->SelectBox_Palette_Unit)
		.Process(this->SelectBox_Frame_Unit)
		.Process(this->SelectBox_DrawOffset_Unit)
		.Process(this->SelectBox_TranslucentLevel)
		.Process(this->SelectBox_CanSee)
		.Process(this->SelectBox_CanObserverSee)

		.Process(this->UseGlobalRadApplicationDelay)
		.Process(this->RadApplicationDelay_Building)

		.Process(this->RadWarhead_Detonate)
		.Process(this->RadHasOwner)
		.Process(this->RadHasInvoker)

		.Process(this->JumpjetCrash)
		.Process(this->JumpjetNoWobbles)
		.Process(this->JumpjetAllowLayerDeviation)
		.Process(this->JumpjetTurnToTarget)

		.Process(this->MissingCameo)
		.Process(this->PlacementGrid_Translucency)
		.Process(this->PlacementPreview)
		.Process(this->PlacementPreview_Translucency)
		.Process(this->Pips_Shield)
		.Process(this->Pips_Shield_Background)
		.Process(this->Pips_Shield_Building)
		.Process(this->Pips_Shield_Building_Empty)
		.Process(this->Pips_SelfHeal_Infantry)
		.Process(this->Pips_SelfHeal_Units)
		.Process(this->Pips_SelfHeal_Buildings)
		.Process(this->Pips_SelfHeal_Infantry_Offset)
		.Process(this->Pips_SelfHeal_Units_Offset)
		.Process(this->Pips_SelfHeal_Buildings_Offset)

		.Process(this->Buildings_DefaultDigitalDisplayTypes)
		.Process(this->Infantry_DefaultDigitalDisplayTypes)
		.Process(this->Vehicles_DefaultDigitalDisplayTypes)
		.Process(this->Aircraft_DefaultDigitalDisplayTypes)

		.Process(this->HugeBar_Config)

		.Process(this->CustomHealthBar)
		.Process(this->Pips)
		.Process(this->Pips_Buildings)

		.Process(this->GScreenAnimType)

		.Process(this->ShowAnim_FrameKeep_Check)
		.Process(this->ShowAnim_CurrentFrameIndex)
		.Process(this->Warheads_DecloakDamagedTargets)
		.Process(this->Warheads_CanBeDodge)

		.Process(this->IronCurtain_EffectOnOrganics)
		.Process(this->IronCurtain_KillOrganicsWarhead)
		.Process(this->IronCurtain_KeptOnDeploy)

		.Process(this->DirectionalArmor)
		.Process(this->DirectionalArmor_FrontMultiplier)
		.Process(this->DirectionalArmor_SideMultiplier)
		.Process(this->DirectionalArmor_BackMultiplier)
		.Process(this->DirectionalArmor_FrontField)
		.Process(this->DirectionalArmor_BackField)
		.Process(this->DirectionalWarhead)
		.Process(this->Directional_Multiplier)

		.Process(this->Parachute_OpenHeight)

		.Process(this->AllowParallelAIQueues)
		.Process(this->ForbidParallelAIQueues_Aircraft)
		.Process(this->ForbidParallelAIQueues_Building)
		.Process(this->ForbidParallelAIQueues_Infantry)
		.Process(this->ForbidParallelAIQueues_Navy)
		.Process(this->ForbidParallelAIQueues_Vehicle)

		.Process(this->IronCurtain_KeptOnDeploy)
		.Process(this->ROF_RandomDelay)

		.Process(this->ToolTip_Background_Color)
		.Process(this->ToolTip_Background_Opacity)
		.Process(this->ToolTip_Background_BlurSize)
		.Process(this->RadialIndicatorVisibility)

		.Process(this->VeteranAnim)
		.Process(this->EliteAnim)

		.Process(this->AllowPlanningMode_Aircraft)
		.Process(this->AllowPlanningMode_Building)

		.Process(this->TextType_Ready)
		.Process(this->TextType_Hold_Multiple)
		.Process(this->TextType_Hold_Singular)
		.Process(this->TextColor_Ready)
		.Process(this->TextColor_Hold_Multiple)
		.Process(this->TextColor_Hold_Singular)

		.Process(this->ReadyShapeType)

		.Process(this->TimerPercentage)
		.Process(this->TimerXOffset)
		.Process(this->TimerFlashFrames)
		.Process(this->TimerPrecision)
		.Process(this->ReadyShapeType_SW)
		.Process(this->TextType_SW)

		.Process(this->OnFire)
		;
}

template <typename T>
bool RulesExt::ExtData::HugeBarData::Serialize(T& stm)
{
	return stm
		.Process(this->HugeBar_RectWidthPercentage)
		.Process(this->HugeBar_RectWH)
		.Process(this->HugeBar_Pips_Color1)
		.Process(this->HugeBar_Pips_Color2)

		.Process(this->HugeBar_Shape)
		.Process(this->HugeBar_Palette)
		.Process(this->HugeBar_Frame)
		.Process(this->HugeBar_Pips_Shape)
		.Process(this->HugeBar_Pips_Palette)
		.Process(this->HugeBar_Pips_Frame)
		.Process(this->HugeBar_Pips_Interval)

		.Process(this->HugeBar_Offset)
		.Process(this->HugeBar_Pips_Offset)
		.Process(this->HugeBar_Pips_Num)

		.Process(this->Value_Text_Color)

		.Process(this->Value_Shape)
		.Process(this->Value_Palette)
		.Process(this->Value_Num_BaseFrame)
		.Process(this->Value_Sign_BaseFrame)
		.Process(this->Value_Shape_Interval)

		.Process(this->DisplayValue)
		.Process(this->Value_Offset)
		.Process(this->Value_Percentage)
		.Process(this->Anchor)
		.Process(this->InfoType)

		.Success()
		;
}

void RulesExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<RulesClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
	ExternVariableClass::LoadVariablesFromDir();
}

void RulesExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<RulesClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool RulesExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm.Success();
}

bool RulesExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm.Success();
}

bool RulesExt::ExtData::HugeBarData::Load(PhobosStreamReader& stm, bool registerForChange)
{
	return this->Serialize(stm);
}

bool RulesExt::ExtData::HugeBarData::Save(PhobosStreamWriter& stm) const
{
	return const_cast<HugeBarData*>(this)->Serialize(stm);
}

// =============================
// container hooks

DEFINE_HOOK(0x667A1D, RulesClass_CTOR, 0x5)
{
	GET(RulesClass*, pItem, ESI);

	RulesExt::Allocate(pItem);

	return 0;
}

DEFINE_HOOK(0x667A30, RulesClass_DTOR, 0x5)
{
	GET(RulesClass*, pItem, ECX);

	RulesExt::Remove(pItem);

	return 0;
}

IStream* RulesExt::g_pStm = nullptr;

DEFINE_HOOK_AGAIN(0x674730, RulesClass_SaveLoad_Prefix, 0x6)
DEFINE_HOOK(0x675210, RulesClass_SaveLoad_Prefix, 0x5)
{
	//GET(RulesClass*, pItem, ECX);
	GET_STACK(IStream*, pStm, 0x4);

	RulesExt::g_pStm = pStm;

	return 0;
}

DEFINE_HOOK(0x678841, RulesClass_Load_Suffix, 0x7)
{
	auto buffer = RulesExt::Global();

	PhobosByteStream Stm(0);
	if (Stm.ReadBlockFromStream(RulesExt::g_pStm))
	{
		PhobosStreamReader Reader(Stm);

		if (Reader.Expect(RulesExt::ExtData::Canary) && Reader.RegisterChange(buffer))
			buffer->LoadFromStream(Reader);
	}

	return 0;
}

DEFINE_HOOK(0x675205, RulesClass_Save_Suffix, 0x8)
{
	auto buffer = RulesExt::Global();
	PhobosByteStream saver(sizeof(*buffer));
	PhobosStreamWriter writer(saver);

	writer.Expect(RulesExt::ExtData::Canary);
	writer.RegisterChange(buffer);

	buffer->SaveToStream(writer);
	saver.WriteBlockToStream(RulesExt::g_pStm);

	return 0;
}

// DEFINE_HOOK(0x52D149, InitRules_PostInit, 0x5)
// {
// 	LaserTrailTypeClass::LoadFromINIList(&CCINIClass::INI_Art.get());
// 	return 0;
// }

DEFINE_HOOK(0x668BF0, RulesClass_Addition, 0x5)
{
	GET(RulesClass*, pItem, ECX);
	GET_STACK(CCINIClass*, pINI, 0x4);

	//	RulesClass::Initialized = false;
	RulesExt::LoadFromINIFile(pItem, pINI);

	return 0;
}

DEFINE_HOOK(0x679A15, RulesData_LoadBeforeTypeData, 0x6)
{
	GET(RulesClass*, pItem, ECX);
	GET_STACK(CCINIClass*, pINI, 0x4);

	//	RulesClass::Initialized = true;
	RulesExt::LoadBeforeTypeData(pItem, pINI);

	return 0;
}

DEFINE_HOOK(0x679CAF, RulesData_LoadAfterTypeData, 0x5)
{
	RulesClass* pItem = RulesClass::Instance();
	GET(CCINIClass*, pINI, ESI);

	RulesExt::LoadAfterTypeData(pItem, pINI);

	return 0;
}
