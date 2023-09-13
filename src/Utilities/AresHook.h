#pragma once

#include "AresHelper.h"

#include <functional>
#include <map>
#include <vector>

#include <Syringe.h>

#define ARES_HOOK(address, name, size) \
EXPORT_FUNC(name)

class AresHook
{
private:

	static constexpr byte INIT = 0x00;
	static constexpr byte INIT3 = 0xCC;
	static constexpr byte NOP = 0x90;
	static constexpr byte JMP = 0xE8;
	static constexpr byte CALL = 0xE9;

	struct Hook
	{
		std::string funcname;
		SIZE_T size;
		std::function<DWORD(REGISTERS*)> function;

		Hook() = default;
		Hook(const char* funcname, SIZE_T size, std::function<DWORD(REGISTERS*)> function)
			: funcname(funcname)
			, size(size)
			, function(function)
		{ }
	};

	static DWORD RelativeOffset(uintptr_t from, uintptr_t to)
	{
		return to - from;
	}

	static std::map<uintptr_t, std::vector<Hook>> Hooks;
	static std::map<uintptr_t, uintptr_t> Transfers;
	static std::map<uintptr_t, std::vector<byte>> SourceCode;

	template <uintptr_t address>
	static void __stdcall Transfer();

	template <uintptr_t address>
	static void RegisterAresHook(size_t size, std::function<DWORD __cdecl(REGISTERS*)> function, AresHelper::Version version);

public:

	static void WriteAresHooks();

	static void Initialize();
};
