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
				T* buffer;

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

	INI_EX exINI(pINI);

	this->Storage_TiberiumIndex.Read(exINI, GENERAL_SECTION, "Storage.TiberiumIndex");
	this->JumpjetAllowLayerDeviation.Read(exINI, "JumpjetControls", "AllowLayerDeviation");
	this->RadApplicationDelay_Building.Read(exINI, "Radiation", "RadApplicationDelay.Building");
	this->MissingCameo.Read(pINI, "AudioVisual", "MissingCameo");
	this->Pips_Shield.Read(exINI, "AudioVisual", "Pips.Shield");
	this->Pips_Shield_Background_Filename.Read(pINI, "AudioVisual", "Pips.Shield.Background");
	this->Pips_Shield_Building.Read(exINI, "AudioVisual", "Pips.Shield.Building");
	this->Pips_Shield_Building_Empty.Read(exINI, "AudioVisual", "Pips.Shield.Building.Empty");

	if (this->Pips_Shield_Background_Filename)
	{
		char filename[0x20];
		strcpy(filename, this->Pips_Shield_Background_Filename);
		_strlwr_s(filename);
		this->Pips_Shield_Background_SHP = FileSystem::LoadSHPFile(filename);
	}

	this->PlacementGrid_TranslucentLevel.Read(exINI, "AudioVisual", "BuildingPlacementGrid.TranslucentLevel");
	this->BuildingPlacementPreview_TranslucantLevel.Read(exINI, "AudioVisual", "BuildingPlacementPreview.DefaultTranslucentLevel");
	this->Pips_SelfHeal_Infantry.Read(exINI, "AudioVisual", "Pips.SelfHeal.Infantry");
	this->Pips_SelfHeal_Units.Read(exINI, "AudioVisual", "Pips.SelfHeal.Units");
	this->Pips_SelfHeal_Buildings.Read(exINI, "AudioVisual", "Pips.SelfHeal.Buildings");

	this->DigitalDisplay_Enable.Read(exINI, sectionAudioVisual, "DigitalDisplay.Enable");
	this->Buildings_DefaultDigitalDisplayTypeHP.Read(exINI, sectionAudioVisual, "Buildings.DefaultDigitalDisplayTypeHP");
	this->Buildings_DefaultDigitalDisplayTypeSP.Read(exINI, sectionAudioVisual, "Buildings.DefaultDigitalDisplayTypeSP");
	this->Infantrys_DefaultDigitalDisplayTypeHP.Read(exINI, sectionAudioVisual, "Infantrys.DefaultDigitalDisplayTypeHP");
	this->Infantrys_DefaultDigitalDisplayTypeSP.Read(exINI, sectionAudioVisual, "Infantrys.DefaultDigitalDisplayTypeSP");
	this->Units_DefaultDigitalDisplayTypeHP.Read(exINI, sectionAudioVisual, "Units.DefaultDigitalDisplayTypeHP");
	this->Units_DefaultDigitalDisplayTypeSP.Read(exINI, sectionAudioVisual, "Units.DefaultDigitalDisplayTypeSP");

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
	this->HugeHP_UseSHPShowBar.Read(exINI, sectionHugeBar, "HugeHP.UseSHPShowBar");
	this->HugeHP_ShowValueSHP.Read(pINI, sectionHugeBar, "HugeHP.ShowValueSHP");
	this->HugeHP_ShowBarSHP.Read(pINI, sectionHugeBar, "HugeHP.ShowBarSHP");
	this->HugeHP_ShowPipsSHP.Read(pINI, sectionHugeBar, "HugeHP.ShowPipsSHP");
	this->HugeHP_ShowValuePAL.Read(pINI, sectionHugeBar, "HugeHP.ShowValuePAL");
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
		.Process(this->AIHousesLists)
		.Process(this->Storage_TiberiumIndex)
		.Process(this->RadApplicationDelay_Building)
		.Process(this->JumpjetCrash)
		.Process(this->JumpjetNoWobbles)
		.Process(this->JumpjetAllowLayerDeviation)
		.Process(this->MissingCameo)
		.Process(this->Pips_Shield)
		.Process(this->Pips_Shield_Background_Filename)
		.Process(this->Pips_Shield_Background_SHP)
		.Process(this->Pips_Shield_Building)
		.Process(this->Pips_Shield_Building_Empty)
		.Process(this->PlacementGrid_TranslucentLevel)
		.Process(this->BuildingPlacementPreview_TranslucantLevel)
		.Process(this->Pips_SelfHeal_Infantry)
		.Process(this->Pips_SelfHeal_Units)
		.Process(this->Pips_SelfHeal_Buildings)
		.Process(this->DigitalDisplay_Enable)
		.Process(this->Buildings_DefaultDigitalDisplayTypeHP)
		.Process(this->Buildings_DefaultDigitalDisplayTypeSP)
		.Process(this->Infantrys_DefaultDigitalDisplayTypeHP)
		.Process(this->Infantrys_DefaultDigitalDisplayTypeSP)
		.Process(this->Units_DefaultDigitalDisplayTypeHP)
		.Process(this->Units_DefaultDigitalDisplayTypeSP)
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
		.Process(this->HugeHP_UseSHPShowBar)
		.Process(this->HugeHP_ShowValueSHP)
		.Process(this->HugeHP_ShowBarSHP)
		.Process(this->HugeHP_ShowPipsSHP)
		.Process(this->HugeHP_ShowValuePAL)
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
		.Process(this->CustomHealthBar)
		.Process(this->Pips)
		.Process(this->Pips_Buildings)
		;
}

void RulesExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<RulesClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
	Debug::Log("[Test Logic] Load From Stream Process\n");
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
