#include "Phobos.h"

#include <GameStrings.h>
#include <CCINIClass.h>

#include <Ext/Scenario/Body.h>

#include <Misc/BlittersFix.h>

#include <Utilities/Enum.h>
#include <Utilities/GeneralUtils.h>
#include <Utilities/Macro.h>
#include <Utilities/Parser.h>
#include <Utilities/Patch.h>
#include <Utilities/TemplateDef.h>

bool Phobos::UI::DisableEmptySpawnPositions = false;
bool Phobos::UI::ExtendedToolTips = false;
int Phobos::UI::MaxToolTipWidth = 0;
bool Phobos::UI::ShowHarvesterCounter = false;
double Phobos::UI::HarvesterCounter_ConditionYellow = 0.99;
double Phobos::UI::HarvesterCounter_ConditionRed = 0.5;
bool Phobos::UI::ShowProducingProgress = false;
bool Phobos::UI::ShowScoreCounter = false;
const wchar_t* Phobos::UI::CostLabel = L"";
const wchar_t* Phobos::UI::PowerLabel = L"";
const wchar_t* Phobos::UI::PowerBlackoutLabel = L"";
const wchar_t* Phobos::UI::TimeLabel = L"";
const wchar_t* Phobos::UI::HarvesterLabel = L"";
bool Phobos::UI::ShowPowerDelta = false;
double Phobos::UI::PowerDelta_ConditionYellow = 0.75;
double Phobos::UI::PowerDelta_ConditionRed = 1.0;
const wchar_t* Phobos::UI::ScoreLabel = L"";
const wchar_t* Phobos::UI::KillLabel = L"";

TextAlign Phobos::UI::HarvesterCounter_Align = TextAlign::Center;

bool Phobos::Config::ToolTipDescriptions = true;
bool Phobos::Config::ToolTipBlur = false;
bool Phobos::Config::PrioritySelectionFiltering = true;
bool Phobos::Config::DevelopmentCommands = true;
bool Phobos::Config::ArtImageSwap = false;
bool Phobos::Config::ShowPlacementPreview = false;
bool Phobos::Config::EnableSelectBox = false;
bool Phobos::Config::DigitalDisplay_Enable = false;
bool Phobos::Config::AllowBypassBuildLimit[3] = { false,false,false };
bool Phobos::Config::SkirmishUnlimitedColors = false;
bool Phobos::Config::ShowDesignatorRange = false;

DEFINE_HOOK(0x5FACDF, OptionsClass_LoadSettings_LoadPhobosSettings, 0x5)
{
	Phobos::Config::ToolTipDescriptions = CCINIClass::INI_RA2MD->ReadBool("Phobos", "ToolTipDescriptions", true);
	Phobos::Config::ToolTipBlur = CCINIClass::INI_RA2MD->ReadBool("Phobos", "ToolTipBlur", false);
	Phobos::Config::PrioritySelectionFiltering = CCINIClass::INI_RA2MD->ReadBool("Phobos", "PrioritySelectionFiltering", true);
	Phobos::Config::ShowPlacementPreview = CCINIClass::INI_RA2MD->ReadBool("Phobos", "ShowPlacementPreview", true);
	Phobos::Config::EnableSelectBox = CCINIClass::INI_RA2MD->ReadBool("Phobos", "EnableSelectBox", false);
	Phobos::Config::DigitalDisplay_Enable = CCINIClass::INI_RA2MD->ReadBool("Phobos", "DigitalDisplay.Enable", false);
	Phobos::Config::ShowDesignatorRange = CCINIClass::INI_RA2MD->ReadBool("Phobos", "ShowDesignatorRange", false);

	CCINIClass* pINI_UIMD = Phobos::OpenConfig(GameStrings::UIMD_INI);
	INI_EX exINI(pINI_UIMD);

	// LoadingScreen
	{
		Phobos::UI::DisableEmptySpawnPositions =
			pINI_UIMD->ReadBool("LoadingScreen", "DisableEmptySpawnPositions", false);
	}

	// ToolTips
	{
		Phobos::UI::ExtendedToolTips =
			pINI_UIMD->ReadBool(TOOLTIPS_SECTION, "ExtendedToolTips", false);

		Phobos::UI::MaxToolTipWidth =
			pINI_UIMD->ReadInteger(TOOLTIPS_SECTION, "MaxWidth", 0);

		pINI_UIMD->ReadString(TOOLTIPS_SECTION, "CostLabel", NONE_STR, Phobos::readBuffer);
		Phobos::UI::CostLabel = GeneralUtils::LoadStringOrDefault(Phobos::readBuffer, L"$");

		pINI_UIMD->ReadString(TOOLTIPS_SECTION, "PowerLabel", NONE_STR, Phobos::readBuffer);
		Phobos::UI::PowerLabel = GeneralUtils::LoadStringOrDefault(Phobos::readBuffer, L"\u26a1"); // ⚡

		pINI_UIMD->ReadString(TOOLTIPS_SECTION, "PowerBlackoutLabel", NONE_STR, Phobos::readBuffer);
		Phobos::UI::PowerBlackoutLabel = GeneralUtils::LoadStringOrDefault(Phobos::readBuffer, L"\u26a1\u274c"); // ⚡❌

		pINI_UIMD->ReadString(TOOLTIPS_SECTION, "TimeLabel", NONE_STR, Phobos::readBuffer);
		Phobos::UI::TimeLabel = GeneralUtils::LoadStringOrDefault(Phobos::readBuffer, L"\u231a"); // ⌚
	}

	// Sidebar
	{
		Phobos::UI::ShowHarvesterCounter =
			pINI_UIMD->ReadBool(SIDEBAR_SECTION, "HarvesterCounter.Show", false);

		pINI_UIMD->ReadString(SIDEBAR_SECTION, "HarvesterCounter.Label", NONE_STR, Phobos::readBuffer);
		Phobos::UI::HarvesterLabel = GeneralUtils::LoadStringOrDefault(Phobos::readBuffer, L"\u26cf"); // ⛏

		Phobos::UI::HarvesterCounter_ConditionYellow =
			pINI_UIMD->ReadDouble(SIDEBAR_SECTION, "HarvesterCounter.ConditionYellow", Phobos::UI::HarvesterCounter_ConditionYellow);

		Phobos::UI::HarvesterCounter_ConditionRed =
			pINI_UIMD->ReadDouble(SIDEBAR_SECTION, "HarvesterCounter.ConditionRed", Phobos::UI::HarvesterCounter_ConditionRed);

		detail::read(Phobos::UI::HarvesterCounter_Align, exINI, SIDEBAR_SECTION, "HarvesterCounter.Align");

		Phobos::UI::ShowProducingProgress =
			pINI_UIMD->ReadBool(SIDEBAR_SECTION, "ProducingProgress.Show", false);

		Phobos::UI::ShowPowerDelta =
			pINI_UIMD->ReadBool(SIDEBAR_SECTION, "PowerDelta.Show", false);

		Phobos::UI::PowerDelta_ConditionYellow =
			pINI_UIMD->ReadDouble(SIDEBAR_SECTION, "PowerDelta.ConditionYellow", Phobos::UI::PowerDelta_ConditionYellow);

		Phobos::UI::PowerDelta_ConditionRed =
			pINI_UIMD->ReadDouble(SIDEBAR_SECTION, "PowerDelta.ConditionRed", Phobos::UI::PowerDelta_ConditionRed);

		Phobos::UI::ShowScoreCounter =
			pINI_UIMD->ReadBool(SIDEBAR_SECTION, "ScoreCounter.Show", false);

		pINI_UIMD->ReadString(SIDEBAR_SECTION, "ScoreCounter.Label", NONE_STR, Phobos::readBuffer);
		Phobos::UI::ScoreLabel = GeneralUtils::LoadStringOrDefault(Phobos::readBuffer, L"\u2605"); // ★
	}

	Phobos::CloseConfig(pINI_UIMD);

	CCINIClass* pINI_RULESMD = Phobos::OpenConfig(GameStrings::RULESMD_INI);

	Phobos::Config::ArtImageSwap = pINI_RULESMD->ReadBool(GameStrings::General, "ArtImageSwap", false);

	if (pINI_RULESMD->ReadBool(GameStrings::General, "FixTransparencyBlitters", true))
		BlittersFix::Apply();

	Phobos::Config::SkirmishUnlimitedColors = pINI_RULESMD->ReadBool(GameStrings::General, "SkirmishUnlimitedColors", false);
	if (Phobos::Config::SkirmishUnlimitedColors)
	{
		// Game_GetLinkedColor converts vanilla dropdown color index into color scheme index ([Colors] from rules)
		// What we want to do is to restore vanilla from Ares hook, and immediately return arg
		// So if spawner feeds us a number, it will be used to look up color scheme directly
		// Patch translates to:
		// mov eax, [esp+a1]
		// shl eax, 1
		// inc eax
		// retn 4
		byte temp[] = { 0x8B, 0x44, 0x24, 0x04, 0xD1, 0xE0, 0x40, 0xC2, 0x04, 0x00 };
		Patch patch { 0x69A310, 10, temp };
		patch.Apply();
	}

	pINI_RULESMD->ReadString(GameStrings::AudioVisual, "KillLabel", NONE_STR, Phobos::readBuffer);
	Phobos::UI::KillLabel = GeneralUtils::LoadStringOrDefault(Phobos::readBuffer, L"\u2620"); // ☠️

	Phobos::CloseConfig(pINI_RULESMD);

	CCINIClass* pMAP = Phobos::OpenConfig(ScenarioClass::Instance->FileName);
	ScenarioExt::Global()->CanSaveGame = pMAP->ReadBool("Basic", "CanSaveGame", true);
	Phobos::CloseConfig(pMAP);

	return 0;
}

DEFINE_HOOK(0x66E9DF, RulesClass_Process_Phobos, 0x8)
{
	GET(CCINIClass*, rulesINI, EDI);

#ifndef DEBUG
	Phobos::Config::DevelopmentCommands = rulesINI->ReadBool("GlobalControls", "DebugKeysEnabled", Phobos::Config::DevelopmentCommands);
#endif

	if (rulesINI->ReadString("GlobalControls", "AllowBypassBuildLimit", "", Phobos::readBuffer))
	{
		bool temp[3] = {};
		int read = Parser<bool, 3>::Parse(Phobos::readBuffer, temp);

		for (int i = 0; i < read; i++)
		{
			Phobos::Config::AllowBypassBuildLimit[i] = temp[2 - i];
		}
	}

	return 0;
}
