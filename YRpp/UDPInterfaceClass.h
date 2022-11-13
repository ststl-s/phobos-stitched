#pragma once
#include <WinsockInterfaceClass.h>

class UDPInterfaceClass : public WinsockInterfaceClass
{
public:
	// Static
	static constexpr reference<UDPInterfaceClass*, 0x887628u> const Instance{};

	bool OpenSocket(int port = 0)
		{ JMP_THIS(0x7B30B0) }

	UDPInterfaceClass()
		{ JMP_THIS(0x7B2DB0); }

	~UDPInterfaceClass()
		{ JMP_THIS(0x7B2E50); }

	// Properties
	int16_t Port;
	int SpareSockets[8];
	int16_t SpareSocketPorts[8];
	DWORD NextSpareSocket;
	DWORD Socket;
	DynamicVectorClass<void*> BroadcastAddresses;
	int16_t words3F350[256];
	DynamicVectorClass<void*> LocalAddresses;
	DWORD NextAddressPort;
	int Addresses[16];
	int16_t Ports[16];
};
static_assert(sizeof(UDPInterfaceClass) == 0x3F5CC);
