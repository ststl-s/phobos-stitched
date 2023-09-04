#pragma once

#include <functional>
#include <vector>

#include <Syringe.h>

#include "AresHelper.h"

//jmp address
std::vector<byte> GenerateHookJMP(uintptr_t address, size_t size)
{
	uintptr_t addAbs = address + AresHelper::AresBaseAddress;
	const byte* const addBytes = reinterpret_cast<byte*>(&addAbs);

	//jmp addAbs
	std::vector<byte> result{ 0xE9, addBytes[0], addBytes[1], addBytes[2], addBytes[3]  };

	while (result.size() < size)
		result.emplace_back(0x90); //nop
}

class AresHook
{
private:

	struct Hook
	{
		uintptr_t Address;
		std::string Name;
		size_t Size;
		uintptr_t FuncAddress;

		Hook() = default;
		Hook(uintptr_t address, const char* funcName, size_t size, uintptr_t funcAddress)
			: Address(address)
			, Name(funcName)
			, Size(size)
			, FuncAddress(funcAddress)
		{}
	};

	static std::vector<Hook> Hooks;

public:

	static void RegisterAresHook(uintptr_t address, const char* funcName, size_t size, uintptr_t funcAddress, AresHelper::Version aresVersion);
	static void WriteHooks();
	static void Init();
	static REGISTERS Entrepot();
};
