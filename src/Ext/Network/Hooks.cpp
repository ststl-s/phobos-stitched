#include "Body.h"
#include <Helpers/Macro.h>

DEFINE_HOOK(0x4C6CB0, Networking_RespondToEvent, 0x6)
{
	GET(EventClass*, pEvent, ECX);
	auto eventType = static_cast<ExtraPhobosNetEvent::Events>(pEvent->Type);

	if (eventType >= ExtraPhobosNetEvent::Events::First)
	{
		// Received Flare event, do something about it
		switch (eventType)
		{
		case ExtraPhobosNetEvent::Events::Convert:
			ExtraPhobosNetEvent::Handlers::RespondToConvert(pEvent);
			break;
		default:
			break;
		}
	}

	return 0;
}

DEFINE_HOOK(0x4C65EF, sub_4C65E0_ShutFuckUpLog, 0x7)
{
	GET(NetworkEvents, events, EAX);

	auto eventType = static_cast<ExtraPhobosNetEvent::Events>(events);

	if (eventType >= ExtraPhobosNetEvent::Events::First && eventType <= ExtraPhobosNetEvent::Events::Last)
	{
		switch (eventType)
		{
		case ExtraPhobosNetEvent::Events::Convert:
			R->ECX("Convert");
			break;
		}

		return 0x4C65F6;
	}

	return 0;
}

int EventLength(uint8_t nInput)
{
	if (nInput <= 0x2Eu)
		return static_cast<int>(EventClass::EventLengthArr[nInput]);

	switch (nInput)
	{
	case 0x60u:
		return 10;
	case 0x61u:
	case 0x62u:
	case 0x81u:
		return 5;
	default:
		break;
	}

	return 0;
}

DEFINE_HOOK(0x64BE7D, sub_64BDD0_PayloadSize1, 0x6)
{
	GET(uint8_t, nSize, EDI);

	const auto length = EventLength(nSize);

	R->ECX(length);
	R->EBP(length);
	R->Stack(0x20, length);

	return nSize == 4 ? 0x64BF1A : 0x64BE97;
}

DEFINE_HOOK(0x64C30E, sub_64BDD0_PayloadSize2, 0x6)
{
	GET(uint8_t, nSize, ESI);

	const auto length = EventLength(nSize);

	R->ECX(length);
	R->EBP(length + (nSize == 4));

	return 0x64C321;
}

DEFINE_HOOK(0x64B6FE, sub_64B660_PayloadSize, 0x6)
{
	GET(uint8_t, nSize, EDI);

	const auto length = EventLength(nSize);

	R->EDX(length);
	R->EBP(length);

	return (nSize == 0x1Fu) ? 0x64B710 : 0x64B71D;
}
