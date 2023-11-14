#include "Body.h"

#include <Utilities/Macro.h>

//烈葱太强啦！

DEFINE_HOOK(0x68811C, ScenarioClass_AssignHouses_SetObserver, 0x6)
{
	GET(HouseClass*, pObserver, EBP);
	HouseExt::ExtMap.Find(pObserver)->IsObserver = true;
	return 0;
}

#pragma region Hooks

#define Hook_IsCurrentPlayerObserver(addr, func, mode, size, ret1, ret2) \
DEFINE_HOOK(addr, ##func##_IsCurrentPlayerOberver##mode##, size) \
{ \
	return HouseExt::IsCurrentPlayerObserver() ? ret1 : ret2; \
}

#define Hook_IsCurrentPlayerObserver2(addr, func, reg) \
DEFINE_HOOK(addr, ##func##_IsCurrentPlayerOberver, 0x6) \
{ \
	R->reg(HouseExt::IsCurrentPlayerObserver() ? HouseClass::CurrentPlayer.get() : nullptr); \
	return addr + 0x6; \
}

#define Hook_IsObserver(addr, func, mode, size, reg, ret1, ret2) \
DEFINE_HOOK(addr, ##func##_IsObserver##mode##, size) \
{ \
	GET(HouseClass* const, pThis, reg); \
	return HouseExt::IsObserver(pThis) ? ret1 : ret2; \
}

#define Hook_IsObserver2(addr, func, mode, size, reg) \
DEFINE_HOOK(addr, ##func##_IsObserver##mode##, size) \
{ \
	GET(HouseClass* const, pThis, reg); R->EAX(HouseExt::IsObserver(pThis) ? pThis : nullptr); \
	return 0; \
}

#pragma endregion

Hook_IsCurrentPlayerObserver(0x4A23A8, sub_4A2370, , 0x8, 0x4A23B0, 0x4A24F4)

DEFINE_HOOK(0x4A2614, sub_4A2600_IsObserver, 0x5)
{
	R->EAX(HouseClass::CurrentPlayer.get());
	return HouseExt::IsCurrentPlayerObserver() ? 0x4A261D : 0x4A267D;
}

Hook_IsCurrentPlayerObserver(0x4FC262, HouseClass_AcceptDefeat, , 0x6, 0x4FC2EF, 0x4FC271)
Hook_IsObserver2(0x4FC348, HouseClass_AcceptDefeat, 2, 0x5, ESI)

#pragma region CheckDefeat
DEFINE_HOOK(0x4FC3F6, HouseClass_AcceptDefeat_CheckDefeat, 0x6)
{
	const auto gameMode = SessionClass::Instance->GameMode;
	if (gameMode == GameMode::Skirmish || gameMode == GameMode::Campaign)
		return 0;

	const auto items = HouseClass::Array.get();
	int alive = 0, aliveHuman = 0, observer = 0, totalHuman = 0;
	DynamicVectorClass<int> teams;

	for (auto pHouse : *items)
	{
		if (pHouse && !pHouse->IsNeutral())
		{
			if (!HouseExt::IsObserver(pHouse))
			{
				bool isHuman = pHouse->IsControlledByHuman();
				if (isHuman)
					totalHuman++;

				if (!pHouse->Defeated)
				{
					if (isHuman)
						aliveHuman++;

					alive++;

					if (pHouse->TournamentTeamID)
						teams.AddUnique(pHouse->TournamentTeamID);
					else
						teams.AddItem(pHouse->TournamentTeamID);
				}
			}
			else
				observer++;
		}
	}

	R->EDI(alive);
	R->EBX(aliveHuman);

	if (totalHuman)
	{
		if (teams.Count <= 1 || aliveHuman == 0)
			observer = 0;
	}
	else
	{
		if (teams.Count <= 1)
			observer = 0;
	}

	R->Stack(STACK_OFFSET(0xC0, -0xAC), alive + observer);
	R->Stack(STACK_OFFSET(0xC0, -0xA8), aliveHuman + observer);

	return 0x4FC457;
}
#pragma endregion

Hook_IsCurrentPlayerObserver(0x4FCD88, HouseClass_Lose, , 0x5, 0x4FCDA6, 0x4FCD97)
Hook_IsObserver(0x5C98E5, sub_5C98A0, , 0x6, EDI, 0x5C9A7E, 0x5C98F1)
Hook_IsCurrentPlayerObserver(0x657EE3, sub_657DA0, , 0x6, 0x657F70, 0x657EF2)
Hook_IsObserver2(0x658390, sub_658330, , 0x5, EBX)
Hook_IsObserver(0x658478, sub_658330, 2, 0x6, EBX, 0x658480, 0x65848A)

DEFINE_HOOK(0x6A557A, SidebarClass_vt_entry_20, 0x5)
{
	return SessionClass::Instance->GameMode != GameMode::Campaign ? 0x6A558D : 0x6A5830;
}

Hook_IsObserver(0x6A55B7, SidebarClass_vt_entry_20, , 0x6, EAX, 0x6A55CF, 0x6A55C8)

DEFINE_JUMP(LJMP, 0x6A57F6, 0x6A57FF)

Hook_IsObserver(0x6A5694, SidebarClass_vt_entry_20, 2, 0x6, ESI, 0x6A569C, 0x6A56AD)
Hook_IsObserver(0x6A57EE, SidebarClass_vt_entry_20, 3, 0x6, EAX, 0x6A580E, 0x6A57F6)
Hook_IsCurrentPlayerObserver(0x6A6610, sub_6A6610, , 0x5, 0x6A66EA, 0x6A6623)
Hook_IsCurrentPlayerObserver2(0x6A6A82, sub_6A6A00, EDI)
Hook_IsCurrentPlayerObserver2(0x6A6B5F, Text_6A6B7B, EBP)
Hook_IsCurrentPlayerObserver(0x6A7BAD, SidebarClass_AI, , 0xA, 0x6A7BAF, 0x6A7BB7)
Hook_IsCurrentPlayerObserver(0x6A7BE7, SidebarClass_AI, 2, 0x6, 0x6A7BEB, 0x6A7C07)
Hook_IsCurrentPlayerObserver(0x6A7CD9, SidebarClass_AI, 3, 0x6, 0x6A7CE3, 0x6A7CE8)
Hook_IsCurrentPlayerObserver2(0x6A88AE, Text_6A88DB, ESI)
Hook_IsCurrentPlayerObserver2(0x6A896A, Text_6A8994, ESI)
Hook_IsCurrentPlayerObserver2(0x6A8A2B, Text_6A8A45, EBP)

Hook_IsCurrentPlayerObserver(0x6A8BB7, sub_6A8B30, , 0x8, 0x6A8BB9, 0x6A8BCB)
Hook_IsCurrentPlayerObserver(0x6A9038, sub_6A8B30, 2, 0x6, 0x6A904B, 0x6A9258)
Hook_IsObserver(0x6A9142, sub_6A8B30, 3, 0x6, ESI, 0x6A914A, 0x6A915B)
Hook_IsObserver(0x6A91EE, sub_6A8B30, 4, 0x5, ESI, 0x6A91F7, 0x6A9208)
Hook_IsCurrentPlayerObserver2(0x6A95A9, StripClass_Sub6A9540, EBP)
Hook_IsCurrentPlayerObserver(0x6A964E, StripClass_Sub6A9540, 2, 0x6, 0x6AA05B, 0x6A9654)
Hook_IsCurrentPlayerObserver(0x6AA04F, StripClass_Sub6A9540, 3, 0x8, 0x6AA057, 0x6AA59B)
Hook_IsCurrentPlayerObserver(0x6C6F81, sub_6C6F50, , 0x8, 0x6C6F8B, 0x6C6F9D)

DEFINE_HOOK(0x6C73F8, sub_6C6F50_IsObserver2, 0x6)
{
	GET(HouseClass*, pHouse, EAX);
	return pHouse->IsHumanPlayer && !HouseExt::IsObserver(pHouse) ? 0x6C7406 : 0x6C7414;
}


// Skip IsCampaign check (confirmed being useless from Mental Omega mappers)
DEFINE_JUMP(LJMP, 0x703A09, 0x703A16)
DEFINE_JUMP(LJMP, 0x45455B, 0x454564)

DEFINE_HOOK(0x4870D0, CellClass_IsSensorsOfHouse_Observer, 0x6)
{
	if (HouseExt::IsCurrentPlayerObserver())
	{
		R->AL(true);
		return 0x4870DE;
	}

	return 0;
}

Hook_IsCurrentPlayerObserver(0x703819, TechnoClass_Cloak_Deselect, , 0x6, 0x70383C, 0)

DEFINE_HOOK(0x65FA70, RadarEventClass_Create_ObserverSkipSensed, 0x6)
{
	GET(RadarEventType, nType, ECX);
	return nType == RadarEventType::EnemySensed && HouseExt::IsCurrentPlayerObserver() ? 0x65FB52 : 0;
}
