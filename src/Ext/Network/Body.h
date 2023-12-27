#pragma once

#include <EventClass.h>

class ExtraPhobosNetEvent
{
public:
	enum class Events : unsigned char
	{
		Convert = 0x81,

		First = Convert,
		Last = Convert
	};

	class Handlers
	{
	public:
		static void RaiseConvert(TechnoClass* pTechno);
		static void RespondToConvert(EventClass* pEvent);
	};

	struct SpecialClick1
	{
		TargetClass SourceCell;

		static inline constexpr size_t size() { return sizeof(SpecialClick1); }
	};

	struct SpecialClick2
	{
		TargetClass SourceCell;
		TargetClass TargetCell;

		static inline constexpr size_t size() { return sizeof(SpecialClick2); }
	};
};
