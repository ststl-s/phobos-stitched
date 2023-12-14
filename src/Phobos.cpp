#include <Phobos.h>

#include <Helpers/Macro.h>

#include <CCINIClass.h>
#include <Unsorted.h>
#include <Drawing.h>
#include <GameStrings.h>

#include <Ext/Scenario/Body.h>

#include <Utilities/Debug.h>
#include <Utilities/Enum.h>
#include <Utilities/GeneralUtils.h>
#include <Utilities/Macro.h>
#include <Utilities/Parser.h>
#include <Utilities/Patch.h>
#include <Utilities/TemplateDef.h>

//#include "Misc/Patches.Blitters.h"

#include <Ext/Techno/Body.h>

#include "Misc/BlittersFix.h"

#ifndef IS_RELEASE_VER
bool HideWarning = false;
#endif

HANDLE Phobos::hInstance = 0;

char Phobos::readBuffer[Phobos::readLength];
wchar_t Phobos::wideBuffer[Phobos::readLength];
const char Phobos::readDelims[4] = ",";

const char* Phobos::AppIconPath = nullptr;

bool Phobos::DisplayDamageNumbers = false;

bool Phobos::Debug_DisplayDamageNumbers = false;

bool Phobos::Debug_DisplayKillMsg = false;

bool Phobos::Debug_DisplayAnimation = true;

bool Phobos::CreateBuildingAllowed = false;
bool Phobos::CreateBuildingFire = false;
bool Phobos::ScreenSWAllowed = false;
bool Phobos::ScreenSWFire = false;

bool Phobos::ToSelectSW = false;

bool Phobos::AutoRepair = false;
bool Phobos::MixFirst = false;
int Phobos::ExpandCount = 99;

bool Phobos::IsLoadingSaveGame = false;

#ifdef STR_GIT_COMMIT
const wchar_t* Phobos::VersionDescription = L"ExtraPhobos nightly build (" STR_GIT_COMMIT L" @ " STR_GIT_BRANCH L"). DO NOT SHIP IN MODS!";
#elif !defined(IS_RELEASE_VER)
const wchar_t* Phobos::VersionDescription = L"ExtraPhobos test #" _STR(TEST_VERSION) L". Based on Phobos build #" _STR(BUILD_NUMBER) L". Please test the build before shipping.";
#else
//const wchar_t* Phobos::VersionDescription = L"ExtraPhobos release build v" FILE_VERSION_STR L".";
#endif

void Phobos::CmdLineParse(char** ppArgs, int nNumArgs)
{
	bool foundInheritance = false;
	bool foundInclude = false;

	// > 1 because the exe path itself counts as an argument, too!
	for (int i = 1; i < nNumArgs; i++)
	{
		const char* pArg = ppArgs[i];

		if (_stricmp(pArg, "-Icon") == 0)
		{
			Phobos::AppIconPath = ppArgs[++i];
		}

		if (_stricmp(pArg, "-MixFirst") == 0)
		{
			Phobos::MixFirst = true;
		}

		char check[0x100];
		strcpy_s(check, pArg);
		char* ext = NULL;
		UNREFERENCED_PARAMETER(strtok_s(check, "=", &ext));

		if (_stricmp(check, "-Count") == 0)
		{
			if (ext)
				Phobos::ExpandCount = atoi(ext);
		}

#ifndef IS_RELEASE_VER
		if (_stricmp(pArg, "-b=" _STR(BUILD_NUMBER)) == 0)
		{
			HideWarning = true;
		}
#endif

		if (_stricmp(pArg, "-Inheritance") == 0)
		{
			foundInheritance = true;
		}
		if (_stricmp(pArg, "-Include") == 0)
		{
			foundInclude = true;
		}
	}

	if (foundInclude)
	{
		Patch::Apply_RAW(0x474200, // Apply CCINIClass_ReadCCFile1_DisableAres
			{ 0x8B, 0xF1, 0x8D, 0x54, 0x24, 0x0C }
		);

		Patch::Apply_RAW(0x474314, // Apply CCINIClass_ReadCCFile2_DisableAres
			{ 0x81, 0xC4, 0xA8, 0x00, 0x00, 0x00 }
		);
	}
	else
	{
		Patch::Apply_RAW(0x474230, // Revert CCINIClass_Load_Inheritance
			{ 0x8B, 0xE8, 0x88, 0x5E, 0x40 }
		);
	}

	if (foundInheritance)
	{
		Patch::Apply_RAW(0x528A10, // Apply INIClass_GetString_DisableAres
			{ 0x83, 0xEC, 0x0C, 0x33, 0xC0 }
		);

		Patch::Apply_RAW(0x526CC0, // Apply INIClass_GetKeyName_DisableAres
			{ 0x8B, 0x54, 0x24, 0x04, 0x83, 0xEC, 0x0C }
		);
	}
	else
	{
		Patch::Apply_RAW(0x528BAC, // Revert INIClass_GetString_Inheritance_NoEntry
			{ 0x8B, 0x7C, 0x24, 0x2C, 0x33, 0xC0, 0x8B, 0x4C, 0x24, 0x28 }
		);
	}

	Debug::Log("Initialized Phobos Build #" _STR(BUILD_NUMBER) "\n");
	Debug::Log("ExtraPhobos Test #" _STR(TEST_VERSION) "Build time " BUILD_DATE "\n");
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

void Phobos::ExeRun()
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

	if (!Console::Create())
	{
		MessageBoxW(NULL,
		L"Failed to allocate the debug console!",
		L"Debug Console Notice", MB_OK);
	}

#endif
}

void Phobos::ExeTerminate()
{
	Console::Release();
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
	Phobos::ExeRun();
	//AresHelper::Init();

	return 0;
}

void NAKED _ExeTerminate()
{
	// Call WinMain
	SET_REG32(EAX, 0x6BB9A0);
	CALL(EAX);
	PUSH_REG(EAX);

	Phobos::ExeTerminate();

	// Jump back
	POP_REG(EAX);
	SET_REG32(EBX, 0x7CD8EF);
	__asm {jmp ebx};
}
DEFINE_JUMP(LJMP, 0x7CD8EA, GET_OFFSET(_ExeTerminate));

DEFINE_HOOK(0x52F639, _YR_CmdLineParse, 0x5)
{
	GET(char**, ppArgs, ESI);
	GET(int, nNumArgs, EDI);

	Phobos::CmdLineParse(ppArgs, nNumArgs);
	Debug::LogDeferredFinalize();

	return 0;
}

DEFINE_HOOK(0x67E44D, LoadGame_SetFlag, 0x5)
{
	Phobos::IsLoadingSaveGame = true;
	return 0;
}

DEFINE_HOOK(0x67E68A, LoadGame_UnsetFlag, 0x5)
{
	Phobos::IsLoadingSaveGame = false;
	return 0;
}

#ifndef IS_RELEASE_VER
#ifndef STR_GIT_COMMIT
DEFINE_HOOK(0x4F4583, GScreenClass_DrawText, 0x6)
{
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
		DSurface::Composite->DrawText(Phobos::VersionDescription, location, COLOR_RED);
	}
	return 0;
}
#endif

#include <DbgHelp.h>
#include <chrono>

LONG WINAPI ExceptionHandler(EXCEPTION_POINTERS* exceptionPointers)
{
	Debug::LogFunctionStack(__FUNCTION__);
	auto now = std::chrono::system_clock::now();
	std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
	std::tm* localTime = std::localtime(&currentTime);
	std::stringstream filenameBuffer;

	filenameBuffer << "dumpfile-";
	filenameBuffer << std::setfill('0') << std::setw(4) << localTime->tm_year + 1900;
	filenameBuffer << std::setfill('0') << std::setw(2) << localTime->tm_mon + 1;
	filenameBuffer << std::setfill('0') << std::setw(2) << localTime->tm_mday;
	filenameBuffer << std::setfill('0') << std::setw(2) << localTime->tm_hour;
	filenameBuffer << std::setfill('0') << std::setw(2) << localTime->tm_min;

	HANDLE hFile = CreateFile(("dump files/" + filenameBuffer.str()).c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		MINIDUMP_EXCEPTION_INFORMATION exceptionInfo = MINIDUMP_EXCEPTION_INFORMATION();
		exceptionInfo.ThreadId = GetCurrentThreadId();
		exceptionInfo.ExceptionPointers = exceptionPointers;
		exceptionInfo.ClientPointers = TRUE;

		MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &exceptionInfo, nullptr, nullptr);

		CloseHandle(hFile);
	}

	return EXCEPTION_EXECUTE_HANDLER;
}

DEFINE_HOOK(0x6BBFE1, _WinMain_Create_DMP, 0x5)
{
	SetUnhandledExceptionFilter(ExceptionHandler);

	return 0;
}
