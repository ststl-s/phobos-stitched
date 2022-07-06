#include <Phobos.h>

#include <Helpers/Macro.h>

#include <CCINIClass.h>
#include <Unsorted.h>
#include <Drawing.h>

#include "Utilities\Parser.h"
#include <Utilities/GeneralUtils.h>
#include <Utilities/Debug.h>
#include <Utilities/Patch.h>
#include <Utilities/Enum.h>
#include <Utilities/TemplateDef.h>

//#include "Misc/Patches.Blitters.h"

#include <Ext/Techno/Body.h>

#include <Misc/GScreenDisplay.h>
#include <Misc/GScreenCreate.h>

#ifndef IS_RELEASE_VER
bool HideWarning = false;
#endif

HANDLE Phobos::hInstance = 0;

char Phobos::readBuffer[Phobos::readLength];
wchar_t Phobos::wideBuffer[Phobos::readLength];
const char Phobos::readDelims[4] = ",";

const char* Phobos::AppIconPath = nullptr;

bool Phobos::Debug_DisplayDamageNumbers = false;

bool Phobos::Debug_DisplayAnimation = true;

bool Phobos::CreateBuildingAllowed = false;
bool Phobos::CreateBuildingFire = false;

#ifdef STR_GIT_COMMIT
const wchar_t* Phobos::VersionDescription = L"Ehance bos nightly build (" STR_GIT_COMMIT L" @ " STR_GIT_BRANCH L"). DO NOT SHIP IN MODS!";
#elif !defined(IS_RELEASE_VER)
const wchar_t* Phobos::VersionDescription = L"Ehance bos development build #" _STR(BUILD_DATE) L". Base on Phobos build #" _STR(BUILD_NUMBER) L". Please test the build before shipping.";
#else
//const wchar_t* Phobos::VersionDescription = L"Ehance bos release build v" FILE_VERSION_STR L".";
#endif

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
const wchar_t* Phobos::UI::TimeLabel = L"";
const wchar_t* Phobos::UI::HarvesterLabel = L"";
bool Phobos::UI::ShowPowerDelta = false;
double Phobos::UI::PowerDelta_ConditionYellow = 0.75;
double Phobos::UI::PowerDelta_ConditionRed = 1.0;
const wchar_t* Phobos::UI::ScoreLabel = L"";

Valueable<TextAlign> Phobos::UI::HarvesterCounter_Align { TextAlign::Center };

bool Phobos::Config::ToolTipDescriptions = true;
bool Phobos::Config::PrioritySelectionFiltering = true;
bool Phobos::Config::DevelopmentCommands = true;
bool Phobos::Config::ArtImageSwap = false;
bool Phobos::Config::AllowParallelAIQueues = true;
bool Phobos::Config::ExtendParallelAIQueues[5] = { true, true, true, true, true };
bool Phobos::Config::EnableBuildingPlacementPreview = false;
bool Phobos::Config::EnableSelectBox = false;
bool Phobos::Config::DigitalDisplay_Enable = false;

bool Phobos::Config::AllowBypassBuildLimit[3] = { false,false,false };

void Phobos::CmdLineParse(char** ppArgs, int nNumArgs)
{
	// > 1 because the exe path itself counts as an argument, too!
	for (int i = 1; i < nNumArgs; i++)
	{
		const char* pArg = ppArgs[i];

		if (_stricmp(pArg, "-Icon") == 0)
		{
			Phobos::AppIconPath = ppArgs[++i];
		}

#ifndef IS_RELEASE_VER 
		if (_stricmp(pArg, "-b=" _STR(BUILD_NUMBER)) == 0)
		{
			HideWarning = true;
		}
#endif
	}

	Debug::Log("Initialized Phobos " PRODUCT_VERSION "\n");
}

CCINIClass* Phobos::OpenConfig(const char* file)
{
	CCINIClass* pINI = GameCreate<CCINIClass>();

	if (pINI)
	{
		CCFileClass* cfg = GameCreate<CCFileClass>(file);

		if (cfg)
		{
			if (cfg->Exists())
			{
				pINI->ReadCCFile(cfg);
			}
			GameDelete(cfg);
		}
	}

	return pINI;
}

void Phobos::CloseConfig(CCINIClass*& pINI)
{
	if (pINI)
	{
		GameDelete(pINI);
		pINI = nullptr;
	}
}

// =============================
// hooks

bool __stdcall DllMain(HANDLE hInstance, DWORD dwReason, LPVOID v)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		Phobos::hInstance = hInstance;
	}
	return true;
}

DEFINE_HOOK(0x7CD810, ExeRun, 0x9)
{
	Patch::ApplyStatic();

#ifdef DEBUG

	if (Phobos::DetachFromDebugger())
	{
		MessageBoxW(NULL,
		L"You can now attach a debugger.\n\n"

		L"Press OK to continue YR execution.",
		L"Debugger Notice", MB_OK);
	}
	else
	{
		MessageBoxW(NULL,
		L"You can now attach a debugger.\n\n"

		L"To attach a debugger find the YR process in Process Hacker "
		L"/ Visual Studio processes window and detach debuggers from it, "
		L"then you can attach your own debugger. After this you should "
		L"terminate Syringe.exe because it won't automatically exit when YR is closed.\n\n"

		L"Press OK to continue YR execution.",
		L"Debugger Notice", MB_OK);
	}


#endif

	return 0;
}

DEFINE_HOOK(0x52F639, _YR_CmdLineParse, 0x5)
{
	GET(char**, ppArgs, ESI);
	GET(int, nNumArgs, EDI);

	Phobos::CmdLineParse(ppArgs, nNumArgs);
	return 0;
}

DEFINE_HOOK(0x5FACDF, OptionsClass_LoadSettings_LoadPhobosSettings, 0x5)
{
	Phobos::Config::ToolTipDescriptions = CCINIClass::INI_RA2MD->ReadBool("Phobos", "ToolTipDescriptions", true);
	Phobos::Config::PrioritySelectionFiltering = CCINIClass::INI_RA2MD->ReadBool("Phobos", "PrioritySelectionFiltering", true);
	Phobos::Config::EnableBuildingPlacementPreview = CCINIClass::INI_RA2MD->ReadBool("Phobos", "ShowBuildingPlacementPreview", false);
	Phobos::Config::EnableSelectBox = CCINIClass::INI_RA2MD->ReadBool("Phobos", "EnableSelectBox", false);
	Phobos::Config::DigitalDisplay_Enable = CCINIClass::INI_RA2MD->ReadBool("Phobos", "DigitalDisplay.Enable", false);

	CCINIClass* pINI_UIMD = Phobos::OpenConfig(reinterpret_cast<const char*>(0x827DC8)/*UIMD.INI*/);
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

		Phobos::UI::HarvesterCounter_Align.Read(exINI, SIDEBAR_SECTION, "HarvesterCounter.Align");

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

	CCINIClass* pINI_RULESMD = Phobos::OpenConfig(reinterpret_cast<const char*>(0x826260)/*"RULESMD.INI"*/);
	Phobos::Config::ArtImageSwap = pINI_RULESMD->ReadBool("General", "ArtImageSwap", false);
	Phobos::CloseConfig(pINI_RULESMD);

	return 0;
}

DEFINE_HOOK(0x66E9DF, RulesClass_Process_Phobos, 0x8)
{
	GET(CCINIClass*, rulesINI, EDI);

	Phobos::Config::DevelopmentCommands = rulesINI->ReadBool("GlobalControls", "DebugKeysEnabled", Phobos::Config::DevelopmentCommands);
	Phobos::Config::AllowParallelAIQueues = rulesINI->ReadBool("GlobalControls", "AllowParallelAIQueues", Phobos::Config::AllowParallelAIQueues);

	if (rulesINI->ReadString("GlobalControls", "AllowBypassBuildLimit", "", Phobos::readBuffer))
	{
		bool temp[3] = {};
		int read = Parser<bool, 3>::Parse(Phobos::readBuffer, temp);

		for (int i = 0; i < read; i++)
		{
			Phobos::Config::AllowBypassBuildLimit[i] = temp[2 - i];
		}
	}

	if (rulesINI->ReadString("GlobalControls", "ExtendParallelAIQueues", "", Phobos::readBuffer))
	{
		bool temp[5] = {};
		int read = Parser<bool, 5>::Parse(Phobos::readBuffer, temp);

		for (int i = 0; i < read; ++i)
			Phobos::Config::ExtendParallelAIQueues[i] = temp[i];
	}

	return 0;
}

//#ifndef IS_RELEASE_VER
DEFINE_HOOK(0x4F4583, GScreenClass_DrawText, 0x6)
{
	// TechnoExt::RunHugeHP(); // Techno/Hooks.cpp
	GScreenDisplay::UpdateAll();
	GScreenCreate::UpdateAll();
	RulesExt::RunAnim();
#ifndef IS_RELEASE_VER
#ifndef STR_GIT_COMMIT
	if (!HideWarning)
#endif // !STR_GIT_COMMIT
	{
		auto wanted = Drawing::GetTextDimensions(Phobos::VersionDescription, { 0,0 }, 0, 2, 0);

		RectangleStruct rect = {
			DSurface::Composite->GetWidth() - wanted.Width - 10,
			0,
			wanted.Width + 10,
			wanted.Height + 10
		};

		Point2D location { rect.X + 5,5 };

		DSurface::Composite->FillRect(&rect, COLOR_BLACK);
		DSurface::Composite->DrawText(Phobos::VersionDescription, &location, COLOR_RED);
	}
#endif
	return 0;
}
//#endif
