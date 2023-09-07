#include "AresHook.h"

#include <numeric>

template <uintptr_t address>
void __stdcall AresHook::Transfer()
{
	HANDLE hThread = GetCurrentThread();
	CONTEXT context = CONTEXT();
	context.ContextFlags = CONTEXT_FULL;

	if (GetThreadContext(hThread, &context))
	{
		REGISTERS reg(context);
		DWORD retnval = 0;

		reg.Origin(address);

		for (const auto& hook : Hooks[address])
		{
			DWORD result = hook.function(&reg);

			if (result != 0)
				retnval = result;
		}

		if (retnval != 0)
			context.Eip = retnval;
		else
			context.Eip = &SourceCode[address][0];

		reg.SetContext(context);

		SetThreadContext(hThread, &context);
	}
	else
	{
		Debug::Log("[" __FUNCTION__ "] Get thread context failed {0x%X}\n", address);
	}
}

template <uintptr_t address>
void AresHook::RegisterAresHook(size_t size, std::function<DWORD(REGISTERS*)> function, AresHelper::Version version)
{

	Transfers[address] = reinterpret_cast<uintptr_t>(&Transfer<address>);

	Hooks[address].emplace_back("", static_cast<SIZE_T>(size), function);
}

void AresHook::WriteAresHooks()
{
	if (!AresHelper::CanUseAres || AresHelper::PhobosBaseAddress == 0x0)
		return;

	for (const auto& item : Hooks)
	{
		const uintptr_t address = item.first;

		if (address == 0U)
			continue;

		HANDLE hProcess = GetCurrentProcess();

		SIZE_T maxSize = 5;

		for (const auto& hook : item.second)
		{
			maxSize = std::max(maxSize, hook.size);
		}

		auto& sourceCopy = SourceCode[address];
		sourceCopy.resize(maxSize + 1);
		SIZE_T tmp = 0;

		ReadProcessMemory(hProcess, reinterpret_cast<LPCVOID>(AresHelper::AresBaseAddress + address), &sourceCopy[0], maxSize, &tmp);
		memcpy(&sourceCopy + maxSize, &RETN, sizeof(RETN));

		std::vector<byte> code { CALL, INIT, INIT, INIT, INIT };
		code.resize(maxSize, NOP);
		uintptr_t transferAddress = Transfers[address];
		uintptr_t targetAddress = reinterpret_cast<uintptr_t>(AresHelper::AresDllHmodule) + address;
		int offset = RelativeOffset((targetAddress + 5), transferAddress);
		memcpy(&code[0] + 0x1, &offset, sizeof(offset));

		SIZE_T writtenBytes = 0;

		WriteProcessMemory
		(
			hProcess,
			reinterpret_cast<LPVOID>(AresHelper::AresBaseAddress + address),
			&code[0],
			maxSize,
			&writtenBytes
		);
	}
}

void AresHook::Initialize()
{

}

//void AresHook::Initialize()
//{
//	RegisterAresHook<0x000000>(0x5, Ares_Hook_Test, AresHelper::Version::Ares30p);
//}
//
//ARES_HOOK(0x000000, Ares_Hook_Test, 0x5)
//{
//	return 0;
//}
