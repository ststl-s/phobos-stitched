#include "AresHook.h"

std::vector<AresHook::Hook> AresHook::Hooks;

void AresHook::RegisterAresHook(uintptr_t address, const char* funcName, size_t size, uintptr_t funcAddress, AresHelper::Version aresVersion)
{
	if (AresHelper::CanUseAres && AresHelper::AresVersion == aresVersion)
		Hooks.emplace_back(address, funcName, size, funcAddress);
}

void AresHook::WriteHooks()
{
	if (!AresHelper::CanUseAres)
		return;

	for (const Hook& hook : Hooks)
	{
		const std::vector<byte> data(GenerateHookJMP(hook.Address, hook.Size));
		SIZE_T bytesWritten = 0;
		SIZE_T size = static_cast<SIZE_T>(data.size());

		WriteProcessMemory
		(
			GetCurrentProcess(),
			reinterpret_cast<void*>(AresHelper::AresBaseAddress + hook.Address),
			&data[0],
			size,
			&bytesWritten
		);

		if (bytesWritten == size)
			Debug::Log("[" __FUNCTION__ "] Hook written{ 0x%X, %s, %u }", hook.Address, hook.Name.c_str(), size);
		else
			Debug::Log("[" __FUNCTION__ "] Hooks written faild { 0x%X, %s, %u }", hook.Address, hook.Name.c_str(), size);
	}
}

void AresHook::Init()
{
	//Register Ares Hooks
}

REGISTERS AresHook::Entrepot()
{
	__asm
	{
		mov r
	}
}
