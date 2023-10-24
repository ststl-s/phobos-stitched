#include <Phobos.h>
#include <Utilities/Macro.h>
#include <SessionClass.h>
#include <GameOptionsClass.h>
#include <Unsorted.h>

namespace GameSpeedTemp
{
	static int counter = 0;
}

DEFINE_HOOK(0x69BAE7, SessionClass_Resume_CampaignGameSpeed, 0xA)
{
	GameOptionsClass::Instance->GameSpeed = Phobos::Config::CampaignDefaultGameSpeed;
	return 0x69BAF1;
}

DEFINE_HOOK(0x55E160, SyncDelay_Start, 0x6)
{
	constexpr reference<CDTimerClass, 0x887348> FrameTimer;
	//constexpr reference<CDTimerClass, 0x887328> NFTTimer;
	if (!Phobos::Misc::CustomGS)
		return 0;
	if ((Phobos::Misc::CustomGS_ChangeInterval[FrameTimer->TimeLeft] > 0)
		&& (GameSpeedTemp::counter % Phobos::Misc::CustomGS_ChangeInterval[FrameTimer->TimeLeft] == 0))
	{
		FrameTimer->TimeLeft = Phobos::Misc::CustomGS_ChangeDelay[FrameTimer->TimeLeft];
		GameSpeedTemp::counter = 1;
	}
	else
	{
		FrameTimer->TimeLeft = Phobos::Misc::CustomGS_DefaultDelay[FrameTimer->TimeLeft];
		GameSpeedTemp::counter++;
	}

	return 0;
}

DEFINE_HOOK(0x55E33B, SyncDelay_End, 0x6)
{
	constexpr reference<CDTimerClass, 0x887348> FrameTimer;
	FrameTimer->TimeLeft = GameOptionsClass::Instance->GameSpeed;
	return 0;
}
