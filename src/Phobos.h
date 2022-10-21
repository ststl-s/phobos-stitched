#pragma once
#include <Phobos.version.h>
#include <Windows.h>
#include <Utilities/Template.h>

class CCINIClass;
class AbstractClass;
enum class TextAlign;

constexpr auto NONE_STR = "<none>";
constexpr auto NONE_STR2 = "none";
constexpr auto TOOLTIPS_SECTION = "ToolTips";
constexpr auto SIDEBAR_SECTION = "Sidebar";

class Phobos
{
public:
	static void CmdLineParse(char**, int);

	static CCINIClass* OpenConfig(const char*);
	static void CloseConfig(CCINIClass*&);

	static void ExeRun();
	static void ExeTerminate();

	//variables
	static HANDLE hInstance;

	static const size_t readLength = 2048;
	static char readBuffer[readLength];
	static wchar_t wideBuffer[readLength];
	static const char readDelims[4];

	static const char* AppIconPath;
	static const wchar_t* VersionDescription;

	static bool Debug_DisplayDamageNumbers;

	static bool Debug_DisplayAnimation;

	static bool CreateBuildingAllowed;
	static bool CreateBuildingFire;
	static bool ScreenSWAllowed;
	static bool ScreenSWFire;

#ifdef DEBUG
	static bool DetachFromDebugger();
#endif
	static void Clear();
	static void PointerGotInvalid(AbstractClass* const pInvalid, bool const removed);
	static HRESULT SaveGameData(IStream* pStm);
	static void LoadGameData(IStream* pStm);

	class UI
	{
	public:
		static bool DisableEmptySpawnPositions;
		static bool ExtendedToolTips;
		static int MaxToolTipWidth;
		static bool ShowHarvesterCounter;
		static double HarvesterCounter_ConditionYellow;
		static double HarvesterCounter_ConditionRed;
		static bool ShowProducingProgress;
		static bool ShowPowerDelta;
		static double PowerDelta_ConditionYellow;
		static double PowerDelta_ConditionRed;
		static bool ShowScoreCounter;

		static const wchar_t* CostLabel;
		static const wchar_t* PowerLabel;
		static const wchar_t* TimeLabel;
		static const wchar_t* HarvesterLabel;
		static const wchar_t* ScoreLabel;

		static Valueable<TextAlign> HarvesterCounter_Align;
	};

	class Config
	{
	public:
		static bool ToolTipDescriptions;
		static bool ToolTipBlur;
		static bool PrioritySelectionFiltering;
		static bool DevelopmentCommands;
		static bool ArtImageSwap;
		static bool EnableSelectBox;
		static bool DigitalDisplay_Enable;
		static bool AllowBypassBuildLimit[3];
		static bool ShowPlacementPreview;
	};
};
