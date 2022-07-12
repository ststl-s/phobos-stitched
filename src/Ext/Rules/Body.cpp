#include "Body.h"
#include <Ext/Side/Body.h>
#include <Utilities/TemplateDef.h>
#include <FPSCounter.h>
#include <GameOptionsClass.h>
#include <HouseTypeClass.h>

#include <New/Type/RadTypeClass.h>
#include <New/Type/ShieldTypeClass.h>
#include <New/Type/LaserTrailTypeClass.h>
#include <New/Type/AttachmentTypeClass.h>
#include <New/Type/BannerTypeClass.h>
#include <New/Type/DigitalDisplayTypeClass.h>
#include <New/Entity/ExternVariableClass.h>
#include <New/Type/FireScriptTypeClass.h>
#include <New/Type/IonCannonTypeClass.h>
#include <New/Type/GScreenAnimTypeClass.h>
#include <New/Type/AttachEffectTypeClass.h>

#include <Utilities/EnumFunctions.h>

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
	DigitalDisplayTypeClass::LoadFromINIList(pINI);
	RadTypeClass::LoadFromINIList(pINI);
	ShieldTypeClass::LoadFromINIList(pINI);
	LaserTrailTypeClass::LoadFromINIList(&CCINIClass::INI_Art.get());
	AttachmentTypeClass::LoadFromINIList(pINI);
	BannerTypeClass::LoadFromINIList(pINI);
	IonCannonTypeClass::LoadFromINIList(pINI);
	GScreenAnimTypeClass::LoadFromINIList(pINI);
	AttachEffectTypeClass::LoadFromINIList(pINI);

	ExternVariableClass::LoadVariablesFromDir("*.ini");
	FireScriptTypeClass::LoadFromDir("*.ini");

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
	const char* sectionAudioVisual = "AudioVisual";
	const char* sectionHugeBar = "HugeBar";
	const char* sectionAIConditionsList = "AIConditionsList";

	INI_EX exINI(pINI);

	this->Storage_TiberiumIndex.Read(exINI, GENERAL_SECTION, "Storage.TiberiumIndex");
	this->InfantryGainSelfHealCap.Read(exINI, GENERAL_SECTION, "InfantryGainSelfHealCap");
	this->UnitsGainSelfHealCap.Read(exINI, GENERAL_SECTION, "UnitsGainSelfHealCap");
	this->EnemyInsignia.Read(exINI, GENERAL_SECTION, "EnemyInsignia");
	this->ShowAllyDisguiseBlinking.Read(exINI, GENERAL_SECTION, "ShowAllyDisguiseBlinking");
	this->JumpjetAllowLayerDeviation.Read(exINI, "JumpjetControls", "AllowLayerDeviation");
	this->JumpjetTurnToTarget.Read(exINI, "JumpjetControls", "TurnToTarget");
	this->RadApplicationDelay_Building.Read(exINI, "Radiation", "RadApplicationDelay.Building");
	this->RadWarhead_Detonate.Read(exINI, "Radiation", "RadSiteWarhead.Detonate");
	this->RadHasOwner.Read(exINI, "Radiation", "RadHasOwner");
	this->RadHasInvoker.Read(exINI, "Radiation", "RadHasInvoker");
	this->MissingCameo.Read(pINI, sectionAudioVisual, "MissingCameo");
	this->Pips_Shield.Read(exINI, sectionAudioVisual, "Pips.Shield");
	this->Pips_Shield_Background.Read(exINI, sectionAudioVisual, "Pips.Shield.Background");
	this->Pips_Shield_Building.Read(exINI, sectionAudioVisual, "Pips.Shield.Building");
	this->Pips_Shield_Building_Empty.Read(exINI, sectionAudioVisual, "Pips.Shield.Building.Empty");

	this->UseSelectBox.Read(exINI, sectionAudioVisual, "UseSelectBox");
	this->SelectBox_Shape_Infantry.Read(pINI, sectionAudioVisual, "SelectBox.Shape.Infantry");
	this->SelectBox_Palette_Infantry.Read(pINI, sectionAudioVisual, "SelectBox.Palette.Infantry");
	this->SelectBox_Frame_Infantry.Read(exINI, sectionAudioVisual, "SelectBox.Frame.Infantry");
	this->SelectBox_DrawOffset_Infantry.Read(exINI, sectionAudioVisual, "SelectBox.DrawOffset.Infantry");
	this->SelectBox_Shape_Unit.Read(pINI, sectionAudioVisual, "SelectBox.Shape.Unit");
	this->SelectBox_Palette_Unit.Read(pINI, sectionAudioVisual, "SelectBox.Palette.Unit");
	this->SelectBox_Frame_Unit.Read(exINI, sectionAudioVisual, "SelectBox.Frame.Unit");
	this->SelectBox_DrawOffset_Unit.Read(exINI, sectionAudioVisual, "SelectBox.DrawOffset.Unit");
	this->SelectBox_TranslucentLevel.Read(exINI, sectionAudioVisual, "SelectBox.TranslucentLevel");
	this->SelectBox_CanSee.Read(exINI, sectionAudioVisual, "SelectBox.CanSee");
	this->SelectBox_CanObserverSee.Read(exINI, sectionAudioVisual, "SelectBox.CanObserverSee");

	this->PlacementGrid_TranslucentLevel.Read(exINI, sectionAudioVisual, "BuildingPlacementGrid.TranslucentLevel");
	this->BuildingPlacementPreview_TranslucentLevel.Read(exINI, sectionAudioVisual, "BuildingPlacementPreview.DefaultTranslucentLevel");
	this->Pips_SelfHeal_Infantry.Read(exINI, sectionAudioVisual, "Pips.SelfHeal.Infantry");
	this->Pips_SelfHeal_Units.Read(exINI, sectionAudioVisual, "Pips.SelfHeal.Units");
	this->Pips_SelfHeal_Buildings.Read(exINI, "AudioVisual", "Pips.SelfHeal.Buildings");
	this->Pips_SelfHeal_Infantry_Offset.Read(exINI, sectionAudioVisual, "Pips.SelfHeal.Infantry.Offset");
	this->Pips_SelfHeal_Units_Offset.Read(exINI, sectionAudioVisual, "Pips.SelfHeal.Units.Offset");
	this->Pips_SelfHeal_Buildings_Offset.Read(exINI, sectionAudioVisual, "Pips.SelfHeal.Buildings.Offset");

	this->IronCurtain_SyncOnDeploy.Read(exINI, "CombatDamage", "IronCurtain.SyncOnDeploy");

	this->Buildings_DefaultDigitalDisplayTypes.Read(exINI, sectionAudioVisual, "Buildings.DefaultDigitalDisplayTypes");
	this->Infantrys_DefaultDigitalDisplayTypes.Read(exINI, sectionAudioVisual, "Infantrys.DefaultDigitalDisplayTypes");
	this->Units_DefaultDigitalDisplayTypes.Read(exINI, sectionAudioVisual, "Units.DefaultDigitalDisplayTypes");
	this->Aircrafts_DefaultDigitalDisplayTypes.Read(exINI, sectionAudioVisual, "Aircrafts.DefaultDigitalDisplayTypes");

	this->HugeHP_PipWidth.Read(exINI, sectionHugeBar, "HugeHP.PipWidth");
	this->HugeHP_PipsCount.Read(exINI, sectionHugeBar, "HugeHP.PipsCount");
	this->HugeHP_PipsOffset.Read(exINI, sectionHugeBar, "HugeHP.PipsOffset");
	this->HugeHP_PipToPipOffset.Read(exINI, sectionHugeBar, "HugeHP.PipToPipOffset");
	this->HugeSP_PipWidth.Read(exINI, sectionHugeBar, "HugeSP.PipWidth");
	this->HugeSP_PipsCount.Read(exINI, sectionHugeBar, "HugeSP.PipsCount");
	this->HugeSP_PipsOffset.Read(exINI, sectionHugeBar, "HugeSP.PipsOffset");
	this->HugeSP_PipToPipOffset.Read(exINI, sectionHugeBar, "HugeSP.PipToPipOffset");
	this->HugeHP_BarFrames.Read(exINI, sectionHugeBar, "HugeHP.BarFrames");
	this->HugeHP_PipsFrames.Read(exINI, sectionHugeBar, "HugeHP.PipsFrames");
	this->HugeSP_BarFrames.Read(exINI, sectionHugeBar, "HugeSP.BarFrames");
	this->HugeSP_PipsFrames.Read(exINI, sectionHugeBar, "HugeSP.PipsFrames");
	this->HugeSP_BarFrameEmpty.Read(exINI, sectionHugeBar, "HugeSP.BarFrameEmpty");
	this->HugeSP_ShowValueAlways.Read(exINI, sectionHugeBar, "HugeSP.ShowValueAlways");
	this->HugeHP_DrawOrderReverse.Read(exINI, sectionHugeBar, "HugeHP.DrawOrderReverse");
	this->HugeHP_HighColor1.Read(exINI, sectionHugeBar, "HugeHP.HighColor1");
	this->HugeHP_HighColor2.Read(exINI, sectionHugeBar, "HugeHP.HighColor2");
	this->HugeHP_MidColor1.Read(exINI, sectionHugeBar, "HugeHP.MidColor1");
	this->HugeHP_MidColor2.Read(exINI, sectionHugeBar, "HugeHP.MidColor2");
	this->HugeHP_LowColor1.Read(exINI, sectionHugeBar, "HugeHP.LowColor1");
	this->HugeHP_LowColor2.Read(exINI, sectionHugeBar, "HugeHP.LowColor2");
	this->HugeHP_BorderWH.Read(exINI, sectionHugeBar, "HugeHP.BorderWH");
	this->HugeHP_RectWH.Read(exINI, sectionHugeBar, "HugeHP.RectWH");
	this->HugeHP_RectCount.Read(exINI, sectionHugeBar, "HugeHP.RectCount");
	this->HugeHP_ShowOffset.Read(exINI, sectionHugeBar, "HugeHP.ShowOffset");
	this->HugeHP_HighValueColor.Read(exINI, sectionHugeBar, "HugeHP.HighValueColor");
	this->HugeHP_MidValueColor.Read(exINI, sectionHugeBar, "HugeHP.MidValueColor");
	this->HugeHP_LowValueColor.Read(exINI, sectionHugeBar, "HugeHP.LowValueColor");
	this->HugeHP_UseSHPShowValue.Read(exINI, sectionHugeBar, "HugeHP.UseSHPShowValue");
	this->HugeHP_CustomSHPShowBar.Read(exINI, sectionHugeBar, "HugeHP.CustomSHPShowBar");
	this->HugeSP_CustomSHPShowBar.Read(exINI, sectionHugeBar, "HugeSP.CustomSHPShowBar");
	this->HugeHP_UseSHPShowBar.Read(exINI, sectionHugeBar, "HugeHP.UseSHPShowBar");
	this->HugeHP_ShowValueSHP.Read(pINI, sectionHugeBar, "HugeHP.ShowValueSHP");
	this->HugeHP_ShowCustomSHP.Read(pINI, sectionHugeBar, "HugeHP.ShowCustomSHP");
	this->HugeSP_ShowCustomSHP.Read(pINI, sectionHugeBar, "HugeSP.ShowCustomSHP");
	this->HugeHP_ShowBarSHP.Read(pINI, sectionHugeBar, "HugeHP.ShowBarSHP");
	this->HugeHP_ShowPipsSHP.Read(pINI, sectionHugeBar, "HugeHP.ShowPipsSHP");
	this->HugeHP_ShowValuePAL.Read(pINI, sectionHugeBar, "HugeHP.ShowValuePAL");
	this->HugeHP_ShowCustomPAL.Read(pINI, sectionHugeBar, "HugeHP.ShowCustomPAL");
	this->HugeSP_ShowCustomPAL.Read(pINI, sectionHugeBar, "HugeSP.ShowCustomPAL");
	this->HugeHP_ShowBarPAL.Read(pINI, sectionHugeBar, "HugeHP.ShowBarPAL");
	this->HugeHP_ShowPipsPAL.Read(pINI, sectionHugeBar, "HugeHP.ShowPipsPAL");
	this->HugeHP_SHPNumberWidth.Read(exINI, sectionHugeBar, "HugeHP.SHPNumberWidth");
	this->HugeHP_SHPNumberInterval.Read(exINI, sectionHugeBar, "HugeHP.SHPNumberInterval");
	this->HugeHP_ShowValueOffset.Read(exINI, sectionHugeBar, "HugeHP.ShowValueOffset");
	this->HugeSP_HighColor1.Read(exINI, sectionHugeBar, "HugeSP.HighColor1");
	this->HugeSP_HighColor2.Read(exINI, sectionHugeBar, "HugeSP.HighColor2");
	this->HugeSP_MidColor1.Read(exINI, sectionHugeBar, "HugeSP.MidColor1");
	this->HugeSP_MidColor2.Read(exINI, sectionHugeBar, "HugeSP.MidColor2");
	this->HugeSP_LowColor1.Read(exINI, sectionHugeBar, "HugeSP.LowColor1");
	this->HugeSP_LowColor2.Read(exINI, sectionHugeBar, "HugeSP.LowColor2");
	this->HugeSP_BorderWH.Read(exINI, sectionHugeBar, "HugeSP.BorderWH");
	this->HugeSP_RectWH.Read(exINI, sectionHugeBar, "HugeSP.RectWH");
	this->HugeSP_RectCount.Read(exINI, sectionHugeBar, "HugeSP.RectCount");
	this->HugeSP_ShowOffset.Read(exINI, sectionHugeBar, "HugeSP.ShowOffset");
	this->HugeSP_HighValueColor.Read(exINI, sectionHugeBar, "HugeSP.HighValueColor");
	this->HugeSP_MidValueColor.Read(exINI, sectionHugeBar, "HugeSP.MidValueColor");
	this->HugeSP_LowValueColor.Read(exINI, sectionHugeBar, "HugeSP.LowValueColor");
	this->HugeSP_UseSHPShowValue.Read(exINI, sectionHugeBar, "HugeSP.UseSHPShowValue");
	this->HugeSP_ShowValueSHP.Read(pINI, sectionHugeBar, "HugeSP.ShowValueSHP");
	this->HugeSP_ShowValuePAL.Read(pINI, sectionHugeBar, "HugeSP.ShowValuePAL");
	this->HugeSP_SHPNumberWidth.Read(exINI, sectionHugeBar, "HugeSP.SHPNumberWidth");
	this->HugeSP_SHPNumberInterval.Read(exINI, sectionHugeBar, "HugeSP.SHPNumberInterval");
	this->HugeSP_ShowValueOffset.Read(exINI, sectionHugeBar, "HugeSP.ShowValueOffset");

	this->CustomHealthBar.Read(exINI, sectionAudioVisual, "CustomHealthBar");
	this->Pips.Read(exINI, "AudioVisual", "HealthBar.Pips");
	this->Pips_Buildings.Read(exINI, "AudioVisual", "HealthBar.Pips.Buildings");

	this->GScreenAnimType.Read(exINI, "AudioVisual", "GScreenAnimType", true);
	this->IronCurtainToOrganic.Read(exINI, "CombatDamage", "IronCurtainToOrganic");
	this->Warheads_DecloakDamagedTargets.Read(exINI, GENERAL_SECTION, "Warheads.DecloakDamagedTargets");
	this->Warheads_CanBeDodge.Read(exINI, GENERAL_SECTION, "Warheads.CanBeDodge");

	if (HugeHP_UseSHPShowValue.Get())
	{
		SHP_HugeHP = FileSystem::LoadSHPFile(HugeHP_ShowValueSHP);
		if (strcmp(HugeHP_ShowValuePAL.data(), "") == 0) PAL_HugeHP = FileSystem::PALETTE_PAL;
		else PAL_HugeHP = FileSystem::LoadPALFile(HugeHP_ShowValuePAL.data(), DSurface::Composite);
		if (SHP_HugeHP == nullptr)
			Debug::Log("[HugeHP::Error] SHP file \"%s\" not found\n", HugeHP_ShowValueSHP.data());
		if (PAL_HugeHP == nullptr)
			Debug::Log("[HugeHP::Error] PAL file \"%s\" not found\n", HugeHP_ShowValuePAL.data());
	}
	if (HugeHP_CustomSHPShowBar.Get()) // 激活自定义SHP巨型血条，读取包含框和格子的SHP文件
	{
		SHP_HugeHPCustom = FileSystem::LoadSHPFile(HugeHP_ShowCustomSHP);

		if (strcmp(HugeHP_ShowCustomPAL.data(), "") == 0)
			PAL_HugeHPCustom = FileSystem::PALETTE_PAL;
		else
			PAL_HugeHPCustom = FileSystem::LoadPALFile(HugeHP_ShowCustomPAL.data(), DSurface::Composite);

		if (SHP_HugeHPCustom == nullptr)
			Debug::Log("[HugeHP::Error] SHP file \"%s\" not found\n", HugeHP_ShowCustomSHP.data());
		if (PAL_HugeHPCustom == nullptr)
			Debug::Log("[HugeHP::Error] PAL file \"%s\" not found\n", HugeHP_ShowCustomPAL.data());
	}
	if (HugeSP_CustomSHPShowBar.Get()) // 激活自定义SHP巨型护盾条，读取包含框和格子的SHP文件
	{
		SHP_HugeSPCustom = FileSystem::LoadSHPFile(HugeSP_ShowCustomSHP);

		if (strcmp(HugeSP_ShowCustomPAL.data(), "") == 0)
			PAL_HugeSPCustom = FileSystem::PALETTE_PAL;
		else
			PAL_HugeSPCustom = FileSystem::LoadPALFile(HugeSP_ShowCustomPAL.data(), DSurface::Composite);

		if (SHP_HugeSPCustom == nullptr)
			Debug::Log("[HugeHP::Error] SHP file \"%s\" not found\n", HugeSP_ShowCustomSHP.data());
		if (PAL_HugeSPCustom == nullptr)
			Debug::Log("[HugeHP::Error] PAL file \"%s\" not found\n", HugeSP_ShowCustomPAL.data());
	}
	if (HugeHP_UseSHPShowBar.Get()) // 激活SHP巨型血条，读取框和格子
	{
		SHP_HugeHPBar = FileSystem::LoadSHPFile(HugeHP_ShowBarSHP);
		if (strcmp(HugeHP_ShowBarPAL.data(), "") == 0) PAL_HugeHPBar = FileSystem::PALETTE_PAL;
		else PAL_HugeHPBar = FileSystem::LoadPALFile(HugeHP_ShowBarPAL.data(), DSurface::Composite);
		if (SHP_HugeHPBar == nullptr)
			Debug::Log("[HugeHP::Error] SHP file \"%s\" not found\n", HugeHP_ShowBarSHP.data());
		if (PAL_HugeHPBar == nullptr)
			Debug::Log("[HugeHP::Error] PAL file \"%s\" not found\n", HugeHP_ShowBarPAL.data());

		SHP_HugeHPPips = FileSystem::LoadSHPFile(HugeHP_ShowPipsSHP);
		if (strcmp(HugeHP_ShowPipsPAL.data(), "") == 0) PAL_HugeHPPips = FileSystem::PALETTE_PAL;
		else PAL_HugeHPPips = FileSystem::LoadPALFile(HugeHP_ShowPipsPAL.data(), DSurface::Composite);
		if (SHP_HugeHPPips == nullptr)
			Debug::Log("[HugeHP::Error] SHP file \"%s\" not found\n", HugeHP_ShowPipsSHP.data());
		if (PAL_HugeHPPips == nullptr)
			Debug::Log("[HugeHP::Error] PAL file \"%s\" not found\n", HugeHP_ShowPipsPAL.data());
	}
	if (HugeSP_UseSHPShowValue.Get())
	{
		SHP_HugeSP = FileSystem::LoadSHPFile(HugeSP_ShowValueSHP);
		if (strcmp(HugeSP_ShowValuePAL.data(), "") == 0) PAL_HugeSP = FileSystem::PALETTE_PAL;
		else PAL_HugeSP = FileSystem::LoadPALFile(HugeSP_ShowValuePAL.data(), DSurface::Composite);
		if (SHP_HugeSP == nullptr)
			Debug::Log("[HugeSP::Error] SHP file \"%s\" not found\n", HugeSP_ShowValueSHP.data());
		if (PAL_HugeSP == nullptr)
			Debug::Log("[HugeSP::Error] PAL file \"%s\" not found\n", HugeSP_ShowValuePAL.data());
	}

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

			// 当前帧序号
			int frameCurrent = RulesExt::Global()->ShowAnim_CurrentFrameIndex;

			// 左上角坐标，默认将SHP文件放置到屏幕中央
			Point2D posAnim = {
				DSurface::Composite->GetWidth() / 2 - ShowAnimSHP->Width / 2,
				DSurface::Composite->GetHeight() / 2 - ShowAnimSHP->Height / 2
			};
			posAnim += pGlobalAnimType->ShowAnim_Offset.Get();

			// 透明度
			auto const nFlag = BlitterFlags::None | EnumFunctions::GetTranslucentLevel(pGlobalAnimType->ShowAnim_TranslucentLevel.Get());

			// 绘制
			DSurface::Composite->DrawSHP(ShowAnimPAL, ShowAnimSHP, frameCurrent, &posAnim, &DSurface::ViewBounds, nFlag,
			0, 0, ZGradient::Ground, 1000, 0, nullptr, 0, 0, 0);

			RulesExt::Global()->ShowAnim_FrameKeep_Check++; // 内部计数器
			if (RulesExt::Global()->ShowAnim_FrameKeep_Check >= pGlobalAnimType->ShowAnim_FrameKeep) // 达到设定的FrameKeep，则下次换帧播放
			{
				RulesExt::Global()->ShowAnim_CurrentFrameIndex++; // 帧序号
				if (RulesExt::Global()->ShowAnim_CurrentFrameIndex >= ShowAnimSHP->Frames) // 帧序号溢出则回到0号帧
					RulesExt::Global()->ShowAnim_CurrentFrameIndex = 0;

				RulesExt::Global()->ShowAnim_FrameKeep_Check = 0; // 每次换帧时，内部计数器归零
			}
		}
	}
}

// =============================
// load / save

template <typename T>
void RulesExt::ExtData::Serialize(T& Stm)
{
	for (auto& it : FireScriptTypeClass::ScriptArray) delete it;
	FireScriptTypeClass::ScriptArray.clear();
	Stm
		.Process(this->AITriggersLists)
		.Process(this->AITargetTypesLists)
		.Process(this->AIScriptsLists)
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
		.Process(this->RadApplicationDelay_Building)
		.Process(this->RadWarhead_Detonate)
		.Process(this->RadHasOwner)
		.Process(this->RadHasInvoker)
		.Process(this->JumpjetCrash)
		.Process(this->JumpjetNoWobbles)
		.Process(this->JumpjetAllowLayerDeviation)
		.Process(this->JumpjetTurnToTarget)
		.Process(this->MissingCameo)
		.Process(this->Pips_Shield)
		.Process(this->Pips_Shield_Background)
		.Process(this->Pips_Shield_Building)
		.Process(this->Pips_Shield_Building_Empty)
		.Process(this->PlacementGrid_TranslucentLevel)
		.Process(this->BuildingPlacementPreview_TranslucentLevel)
		.Process(this->Pips_SelfHeal_Infantry)
		.Process(this->Pips_SelfHeal_Units)
		.Process(this->Pips_SelfHeal_Buildings)
		.Process(this->Pips_SelfHeal_Infantry_Offset)
		.Process(this->Pips_SelfHeal_Units_Offset)
		.Process(this->Pips_SelfHeal_Buildings_Offset)
		.Process(this->Buildings_DefaultDigitalDisplayTypes)
		.Process(this->Infantrys_DefaultDigitalDisplayTypes)
		.Process(this->Units_DefaultDigitalDisplayTypes)
		.Process(this->Aircrafts_DefaultDigitalDisplayTypes)
		.Process(this->HugeHP_HighColor1)
		.Process(this->HugeHP_HighColor2)
		.Process(this->HugeHP_MidColor1)
		.Process(this->HugeHP_MidColor2)
		.Process(this->HugeHP_LowColor1)
		.Process(this->HugeHP_LowColor2)
		.Process(this->HugeHP_BorderWH)
		.Process(this->HugeHP_RectWH)
		.Process(this->HugeHP_RectCount)
		.Process(this->HugeHP_ShowOffset)
		.Process(this->HugeHP_HighValueColor)
		.Process(this->HugeHP_MidValueColor)
		.Process(this->HugeHP_LowValueColor)
		.Process(this->HugeHP_UseSHPShowValue)
		.Process(this->HugeHP_CustomSHPShowBar)
		.Process(this->HugeSP_CustomSHPShowBar)
		.Process(this->HugeHP_UseSHPShowBar)
		.Process(this->HugeHP_ShowValueSHP)
		.Process(this->HugeHP_ShowValuePAL)
		.Process(this->HugeHP_ShowCustomSHP)
		.Process(this->HugeSP_ShowCustomSHP)
		.Process(this->HugeHP_ShowBarSHP)
		.Process(this->HugeHP_ShowPipsSHP)
		.Process(this->HugeHP_ShowCustomPAL)
		.Process(this->HugeSP_ShowCustomPAL)
		.Process(this->HugeHP_ShowBarPAL)
		.Process(this->HugeHP_ShowPipsPAL)
		.Process(this->HugeHP_SHPNumberWidth)
		.Process(this->HugeHP_SHPNumberInterval)
		.Process(this->HugeHP_ShowValueOffset)
		.Process(this->HugeSP_HighColor1)
		.Process(this->HugeSP_HighColor2)
		.Process(this->HugeSP_MidColor1)
		.Process(this->HugeSP_MidColor2)
		.Process(this->HugeSP_LowColor1)
		.Process(this->HugeSP_LowColor2)
		.Process(this->HugeSP_BorderWH)
		.Process(this->HugeSP_RectWH)
		.Process(this->HugeSP_RectCount)
		.Process(this->HugeSP_ShowOffset)
		.Process(this->HugeSP_HighValueColor)
		.Process(this->HugeSP_MidValueColor)
		.Process(this->HugeSP_LowValueColor)
		.Process(this->HugeSP_UseSHPShowValue)
		.Process(this->HugeSP_ShowValueSHP)
		.Process(this->HugeSP_ShowValuePAL)
		.Process(this->HugeSP_SHPNumberWidth)
		.Process(this->HugeSP_SHPNumberInterval)
		.Process(this->HugeSP_ShowValueOffset)
		.Process(this->HugeHP_PipWidth)
		.Process(this->HugeHP_PipsCount)
		.Process(this->HugeHP_PipsOffset)
		.Process(this->HugeHP_PipToPipOffset)
		.Process(this->HugeSP_PipWidth)
		.Process(this->HugeSP_PipsCount)
		.Process(this->HugeSP_PipsOffset)
		.Process(this->HugeSP_PipToPipOffset)
		.Process(this->HugeHP_BarFrames)
		.Process(this->HugeHP_PipsFrames)
		.Process(this->HugeSP_BarFrames)
		.Process(this->HugeSP_PipsFrames)
		.Process(this->HugeSP_BarFrameEmpty)
		.Process(this->HugeSP_ShowValueAlways)
		.Process(this->HugeHP_DrawOrderReverse)
		.Process(this->CustomHealthBar)
		.Process(this->Pips)
		.Process(this->Pips_Buildings)
		.Process(this->GScreenAnimType)
		.Process(this->ShowAnim_FrameKeep_Check)
		.Process(this->ShowAnim_CurrentFrameIndex)
		.Process(this->IronCurtainToOrganic)
		.Process(this->Warheads_DecloakDamagedTargets)
		.Process(this->Warheads_CanBeDodge)
		.Process(this->IronCurtain_SyncOnDeploy)
		;
}

void RulesExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<RulesClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
	if (HugeHP_UseSHPShowValue.Get())
	{
		SHP_HugeHP = FileSystem::LoadSHPFile(HugeHP_ShowValueSHP);
		if (strcmp(HugeHP_ShowValuePAL.data(), "") == 0) PAL_HugeHP = FileSystem::PALETTE_PAL;
		else PAL_HugeHP = FileSystem::LoadPALFile(HugeHP_ShowValuePAL.data(), DSurface::Composite);
		if (SHP_HugeHP == nullptr)
			Debug::Log("[HugeHP::Error] SHP file \"%s\" not found\n", HugeHP_ShowValueSHP.data());
		if (PAL_HugeHP == nullptr)
			Debug::Log("[HugeHP::Error] PAL file \"%s\" not found\n", HugeHP_ShowValuePAL.data());
	}
	if (HugeSP_UseSHPShowValue.Get())
	{
		SHP_HugeSP = FileSystem::LoadSHPFile(HugeSP_ShowValueSHP);
		if (strcmp(HugeSP_ShowValuePAL.data(), "") == 0) PAL_HugeSP = FileSystem::PALETTE_PAL;
		else PAL_HugeSP = FileSystem::LoadPALFile(HugeSP_ShowValuePAL.data(), DSurface::Composite);
		if (SHP_HugeSP == nullptr)
			Debug::Log("[HugeSP::Error] SHP file \"%s\" not found\n", HugeSP_ShowValueSHP.data());
		if (PAL_HugeSP == nullptr)
			Debug::Log("[HugeSP::Error] PAL file \"%s\" not found\n", HugeSP_ShowValuePAL.data());
	}
	if (HugeHP_CustomSHPShowBar.Get()) // 激活自定义SHP巨型血条，读取包含框和格子的SHP文件
	{
		SHP_HugeHPCustom = FileSystem::LoadSHPFile(HugeHP_ShowCustomSHP);

		if (strcmp(HugeHP_ShowCustomPAL.data(), "") == 0)
			PAL_HugeHPCustom = FileSystem::PALETTE_PAL;
		else
			PAL_HugeHPCustom = FileSystem::LoadPALFile(HugeHP_ShowCustomPAL.data(), DSurface::Composite);

		if (SHP_HugeHPCustom == nullptr)
			Debug::Log("[HugeHP::Error] SHP file \"%s\" not found\n", HugeHP_ShowCustomSHP.data());
		if (PAL_HugeHPCustom == nullptr)
			Debug::Log("[HugeHP::Error] PAL file \"%s\" not found\n", HugeHP_ShowCustomPAL.data());
	}
	if (HugeSP_CustomSHPShowBar.Get()) // 激活自定义SHP巨型护盾条，读取包含框和格子的SHP文件
	{
		SHP_HugeSPCustom = FileSystem::LoadSHPFile(HugeSP_ShowCustomSHP);

		if (strcmp(HugeSP_ShowCustomPAL.data(), "") == 0)
			PAL_HugeSPCustom = FileSystem::PALETTE_PAL;
		else
			PAL_HugeSPCustom = FileSystem::LoadPALFile(HugeSP_ShowCustomPAL.data(), DSurface::Composite);

		if (SHP_HugeSPCustom == nullptr)
			Debug::Log("[HugeHP::Error] SHP file \"%s\" not found\n", HugeSP_ShowCustomSHP.data());
		if (PAL_HugeSPCustom == nullptr)
			Debug::Log("[HugeHP::Error] PAL file \"%s\" not found\n", HugeSP_ShowCustomPAL.data());
	}
	if (HugeHP_UseSHPShowBar.Get()) // 激活SHP巨型血条，读取框和格子，各3帧
	{
		SHP_HugeHPBar = FileSystem::LoadSHPFile(HugeHP_ShowBarSHP);
		if (strcmp(HugeHP_ShowBarPAL.data(), "") == 0) PAL_HugeHPBar = FileSystem::PALETTE_PAL;
		else PAL_HugeHPBar = FileSystem::LoadPALFile(HugeHP_ShowBarPAL.data(), DSurface::Composite);
		if (SHP_HugeHPBar == nullptr)
			Debug::Log("[HugeHP::Error] SHP file \"%s\" not found\n", HugeHP_ShowBarSHP.data());
		if (PAL_HugeHPBar == nullptr)
			Debug::Log("[HugeHP::Error] PAL file \"%s\" not found\n", HugeHP_ShowBarPAL.data());

		SHP_HugeHPPips = FileSystem::LoadSHPFile(HugeHP_ShowPipsSHP);
		if (strcmp(HugeHP_ShowPipsPAL.data(), "") == 0) PAL_HugeHPPips = FileSystem::PALETTE_PAL;
		else PAL_HugeHPPips = FileSystem::LoadPALFile(HugeHP_ShowPipsPAL.data(), DSurface::Composite);
		if (SHP_HugeHPPips == nullptr)
			Debug::Log("[HugeHP::Error] SHP file \"%s\" not found\n", HugeHP_ShowPipsSHP.data());
		if (PAL_HugeHPPips == nullptr)
			Debug::Log("[HugeHP::Error] PAL file \"%s\" not found\n", HugeHP_ShowPipsPAL.data());
	}
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
