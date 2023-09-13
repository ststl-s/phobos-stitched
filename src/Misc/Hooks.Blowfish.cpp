#include <Phobos.h>
#include <CCFileClass.h>
#include <GameStrings.h>

#include <Utilities/Macro.h>
#include <Utilities/Debug.h>

HRESULT __stdcall Blowfish_Loader(
	REFCLSID  rclsid,
	LPUNKNOWN pUnkOuter,
	DWORD     dwClsContext,
	REFIID    riid,
	LPVOID* ppv
)
{
	typedef HRESULT(__stdcall* pDllGetClassObject)(const IID&, const IID&, IClassFactory**);

	auto result = REGDB_E_KEYMISSING;

	// First, let's try to run the vanilla function
	result = CoCreateInstance(rclsid, pUnkOuter, dwClsContext, riid, ppv);
	if (SUCCEEDED(result))
		return result;

	HMODULE hDll = LoadLibrary(GameStrings::BLOWFISH_DLL);
	if (hDll)
	{
		auto GetClassObject = (pDllGetClassObject)GetProcAddress(hDll, "DllGetClassObject");
		if (GetClassObject)
		{

			IClassFactory* pIFactory;
			result = GetClassObject(rclsid, IID_IClassFactory, &pIFactory);

			if (SUCCEEDED(result))
			{
				result = pIFactory->CreateInstance(pUnkOuter, riid, ppv);
				pIFactory->Release();
			}
		}
	}

	if (!SUCCEEDED(result))
	{
		FreeLibrary(hDll);

		const char* Message = "File Blowfish.dll was not found\n";
		MessageBox(0, Message, "Fatal error ", MB_ICONERROR);
		Debug::FatalErrorAndExit(Message);
	}

	return result;
}

DEFINE_JUMP(CALL6, 0x6BEDDD, GET_OFFSET(Blowfish_Loader))
DEFINE_JUMP(CALL6, 0x437F6E, GET_OFFSET(Blowfish_Loader))

DEFINE_HOOK(0x473D28, CCFileClass_Open_MixFirst, 0x5)
{
	if (Phobos::MixFirst)
	{
		enum { SkipCheck = 0x473D3A };

		GET(CCFileClass*, pThis, ESI);

		R->EBX(0);

		char fileName[0x100];
		strcpy_s(fileName, pThis->GetFileName());
		char* ext = NULL;
		strtok_s(fileName, ".", &ext);


		//各种扩展文件的筛选
		{
			if (_stricmp(ext, "map") == 0)
				return SkipCheck;

			if (_stricmp(ext, "csf") == 0)
				return SkipCheck;

			if (_stricmp(ext, "fnt") == 0)
				return SkipCheck;

			if (_stricmp(ext, "shp") == 0)
				return SkipCheck;

			if (_stricmp(ext, "pal") == 0)
				return SkipCheck;

			if (_stricmp(ext, "hva") == 0)
				return SkipCheck;

			if (_stricmp(ext, "vxl") == 0)
				return SkipCheck;

			if (_stricmp(ext, "pcx") == 0)
				return SkipCheck;

			if (_stricmp(ext, "wav") == 0)
				return SkipCheck;

			if (_stricmp(ext, "ini") == 0)
				return SkipCheck;
		}
	}

	return 0;
}

DEFINE_HOOK(0x5301AC, Expandmd_Count, 0x9)
{
	R->EDI(Phobos::ExpandCount);

	return 0x53028A;
}
