#include "Body.h"

#include <SuperClass.h>
#include <BuildingClass.h>
#include <HouseClass.h>
#include <TechnoClass.h>

#include <Misc/PhobosGlobal.h>

#include <Ext/House/Body.h>

#include <BitFont.h>
#include <Utilities/EnumFunctions.h>
#include <Utilities/Macro.h>

DEFINE_HOOK(0x6CC390, SuperClass_Launch, 0x6)
{
	GET(SuperClass* const, pSuper, ECX);
	GET_STACK(CellStruct const* const, pCell, 0x4);
	GET_STACK(bool const, isPlayer, 0x8);

	Debug::Log("[Phobos Launch] %s\n", pSuper->Type->get_ID());

	auto const handled = SWTypeExt::Activate(pSuper, *pCell, isPlayer);

	return handled ? 0x6CDE40 : 0;
}

//Ares hooked at 0x6CC390 and jumped to 0x6CDE40
// If a super is not handled by Ares however, we do it at the original entry point
DEFINE_HOOK_AGAIN(0x6CC390, SuperClass_Place_FireExt, 0x6)
DEFINE_HOOK(0x6CDE40, SuperClass_Place_FireExt, 0x4)
{
	GET(SuperClass* const, pSuper, ECX);
	GET_STACK(CellStruct const* const, pCell, 0x4);
	GET_STACK(bool const, isPlayer, 0x8);

	SWTypeExt::FireSuperWeaponExt(pSuper, *pCell);

	return 0;
}

DEFINE_HOOK(0x50B1CA, SuperClass_After_Update, 0x6)
{
	PhobosGlobal::Global()->CheckSuperQueued();

	return 0;
}

DEFINE_HOOK(0x6CB5D2, SuperClass_Grant_AddToShowTimer, 0x9)
{
	GET(SuperClass*, pThis, ESI);

	enum { SkipGameCode = 0x6CB63E };

	if (pThis->Type->ShowTimer && !pThis->Owner->Type->MultiplayPassive)
	{
		SuperClass::ShowTimers->AddItem(pThis);

		const auto pTypeExt = SWTypeExt::ExtMap.Find(pThis->Type);
		int priority = pTypeExt->SW_Priority;
		int size = SuperClass::ShowTimers->Count;

		for (int i = 0; i < size; i++)
		{
			int otherPriority = SWTypeExt::ExtMap.Find(SuperClass::ShowTimers->GetItem(i)->Type)->SW_Priority;

			if (priority > otherPriority)
			{
				std::swap(SuperClass::ShowTimers->Items[i], SuperClass::ShowTimers->Items[size - 1]);

				for (int j = i + 1; j < size - 1; j++)
				{
					std::swap(SuperClass::ShowTimers->Items[j], SuperClass::ShowTimers->Items[size - 1]);
				}

				break;
			}
		}
	}

	return SkipGameCode;
}

DEFINE_HOOK(0x6D4A35, SuperClass_ShowTimer_DrawText, 0x6)
{
	GET(SuperClass*, pSuper, ECX);
	GET(int, index, EBX); // in ShowTimer-SW items array

	enum { SkipGameCode = 0x6D4A70 };

	const auto pTypeExt = SWTypeExt::ExtMap.Find(pSuper->Type);
    auto pRulesExt = RulesExt::Global();

	if (!(pTypeExt->TimerPercentage.Get(pRulesExt->TimerPercentage) ||
		 ((pTypeExt->SW_Cumulative_ShowTrueTimer || pTypeExt->SW_Cumulative_ShowCountInTimer) && pTypeExt->SW_Cumulative) ||
		 (pTypeExt->ShowTimerCustom)))
		return 0;

	int left = pSuper->RechargeTimer.GetTimeLeft();
	int total = pSuper->GetRechargeTime();
	int passed = total - left;

	wchar_t textTime[0x30] = L"\0";
	wchar_t textName[0x100] = L"\0";
	const wchar_t* uiname = pSuper->Type->UIName;

	auto pHouseExt = HouseExt::ExtMap.Find(pSuper->Owner);

	int count = 0;

	if (pTypeExt->SW_Cumulative && pTypeExt->SW_Cumulative_ShowCountInTimer)
	{
		if (left <= 0)
		{
			count = 1;
		}

		if (pHouseExt->SuperWeaponCumulativeCount[index] > 0)
		{
			count += pHouseExt->SuperWeaponCumulativeCount[index];
		}
	}

	if (pTypeExt->SW_Cumulative && pTypeExt->SW_Cumulative_ShowTrueTimer)
	{
		if (left <= 0 && pHouseExt->SuperWeaponCumulativeCount[index] < pHouseExt->SuperWeaponCumulativeMaxCount[index])
		{
			left = pHouseExt->SuperWeaponCumulativeCharge[index];
		}
	}

	if (pTypeExt->TimerPercentage.Get(pRulesExt->TimerPercentage))
	{
		double ratio = (total <= 0) ? 1 : (double)passed / (double)total * 100;

		switch (pRulesExt->TimerPrecision)
		{
		case 0:
			swprintf_s(textTime, L"%d", (int)ratio);
			break;

		case 1:
			swprintf_s(textTime, L"%.1f", ratio);
			break;

		case 2:
			swprintf_s(textTime, L"%.2f", ratio);
			break;

		case 3:
			swprintf_s(textTime, L"%.3f", ratio);
			break;

		default:
			swprintf_s(textTime, L"%d", (int)ratio);
			break;
		}

		wcscat_s(textTime, L"%%");
	}
	else
	{
		int truetime = left / 15;
		if (pTypeExt->ShowTimerCustom)
		{
			if (pTypeExt->ShowTimerCustom_Type == ShowTimerType::Hour)
			{
				int hour = truetime / 3600;
				int minute = truetime % 3600 / 60;
				int second = truetime % 3600 % 60;

				wchar_t texthour[0x30] = L"\0";
				wchar_t textminute[0x30] = L"\0";
				wchar_t textsecond[0x30] = L"\0";

				if (hour > 0 || pTypeExt->ShowTimerCustom_AlwaysShow)
				{
					swprintf_s(texthour, L"%d:", hour);

					if (minute >= 10)
					{
						swprintf_s(textminute, L"%d:", minute);
					}
					else
					{
						swprintf_s(textminute, L"0%d:", minute);
					}

					if (second >= 10)
					{
						swprintf_s(textsecond, L"%d", second);
					}
					else
					{
						swprintf_s(textsecond, L"0%d", second);
					}

					wcscat_s(textminute, textsecond);
					wcscat_s(texthour, textminute);
					wcscat_s(textTime, texthour);
				}
				else
				{
					if (minute > 0)
					{
						swprintf_s(textminute, L"%d:", minute);

						if (second >= 10)
						{
							swprintf_s(textsecond, L"%d", second);
						}
						else
						{
							swprintf_s(textsecond, L"0%d", second);
						}

						wcscat_s(textminute, textsecond);
						wcscat_s(textTime, textminute);
					}
					else
					{
						swprintf_s(textsecond, L"%d", second);
						wcscat_s(textTime, textsecond);
					}
				}
			}
			else if (pTypeExt->ShowTimerCustom_Type == ShowTimerType::Minute)
			{
				int minute = truetime / 60;
				int second = truetime % 60;

				wchar_t textminute[0x30] = L"";
				wchar_t textsecond[0x30] = L"";

				if (minute > 0 || pTypeExt->ShowTimerCustom_AlwaysShow)
				{
					swprintf_s(textminute, L"%d:", minute);

					if (second >= 10)
					{
						swprintf_s(textsecond, L"%d", second);
					}
					else
					{
						swprintf_s(textsecond, L"0%d", second);
					}

					wcscat_s(textminute, textsecond);
					wcscat_s(textTime, textminute);
				}
				else
				{
					swprintf_s(textsecond, L"%d", second);
					wcscat_s(textTime, textsecond);
				}
			}
			else
			{
				wchar_t textsecond[0x30] = L"";
				swprintf_s(textsecond, L"%d", truetime);
				wcscat_s(textTime, textsecond);
			}
		}
		else
		{
			int hour = truetime / 3600;
			int minute = truetime % 3600 / 60;
			int second = truetime % 3600 % 60;

			wchar_t texthour[0x30] = L"";
			wchar_t textminute[0x30] = L"";
			wchar_t textsecond[0x30] = L"";

			if (hour > 0)
			{
				swprintf_s(texthour, L"%d:", hour);

				if (minute >= 10)
				{
					swprintf_s(textminute, L"%d:", minute);
				}
				else
				{
					swprintf_s(textminute, L"0%d:", minute);
				}

				if (second >= 10)
				{
					swprintf_s(textsecond, L"%d", second);
				}
				else
				{
					swprintf_s(textsecond, L"0%d", second);
				}

				wcscat_s(textminute, textsecond);
				wcscat_s(texthour, textminute);
				wcscat_s(textTime, texthour);
			}
			else
			{
				if (minute >= 10)
				{
					swprintf_s(textminute, L"%d:", minute);
				}
				else
				{
					swprintf_s(textminute, L"0%d:", minute);
				}

				if (second >= 10)
				{
					swprintf_s(textsecond, L"%d", second);
				}
				else
				{
					swprintf_s(textsecond, L"0%d", second);
				}

				wcscat_s(textminute, textsecond);
				wcscat_s(textTime, textminute);
			}
		}
	}

	if (count > 0)
	{
		swprintf_s(textName, L"%ls x%d  ", uiname, count);
	}
	else
	{
		swprintf_s(textName, L"%ls  ", uiname);
	}

	RectangleStruct bounds = { 0, 0, 0, 0 };
	DSurface::Composite->GetRect(&bounds);

	int TimeWidth = 0, TimeHeight = 0;
	int NameWidth = 0, NameHeight = 0;
	BitFont::Instance->GetTextDimension(textTime, &TimeWidth, &TimeHeight, 200);
	BitFont::Instance->GetTextDimension(textName, &NameWidth, &NameHeight, 200);

	if ((TimeWidth / 11) > 2)
	{
		for (int i = 0; i < (TimeWidth / 11); i++)
		{
			wcscat_s(textName, L" ");
		}
	}
	else
	{
		wcscat_s(textName, L"  ");
	}

	Point2D posTime = {
		DSurface::ViewBounds->Width - 3,
		DSurface::ViewBounds->Height - (index + 1) * (TimeHeight + 2), // see 0x6D4D0B
	};
	Point2D posName = {
		posTime.X - TimeWidth + 11 + pTypeExt->TimerXOffset.Get(pRulesExt->TimerXOffset),
		posTime.Y
	};

	TextPrintType flags = TextPrintType::Right | pRulesExt->TextType_SW.Get(TextPrintType::Background);

	ColorScheme* HouseColorScheme = ColorScheme::Array->Items[pSuper->Owner->ColorSchemeIndex];

	DSurface::Composite->DrawTextA(textName, &bounds, &posName, HouseColorScheme, 0, flags);

	int frames = pRulesExt->TimerFlashFrames;
	if (!pSuper->RechargeTimer.HasTimeLeft() && (Unsorted::CurrentFrame % (2 * frames) > (frames - 1)))
		DSurface::Composite->DrawTextA(textTime, &bounds, &posTime, COLOR_WHITE, 0, flags);
	else
		DSurface::Composite->DrawTextA(textTime, &bounds, &posTime, HouseColorScheme, 0, flags);

	if (!pSuper->RechargeTimer.HasTimeLeft()) // 100% already
	{
		GScreenAnimTypeClass* pReadyShapeType = pRulesExt->ReadyShapeType_SW.Get();
		if (pReadyShapeType)
		{
			SHPStruct* ShowAnimSHP = pReadyShapeType->SHP_ShowAnim;
			ConvertClass* ShowAnimPAL = pReadyShapeType->PAL_ShowAnim;
			if (ShowAnimSHP && ShowAnimPAL)
			{
				int frameCurrent = (Unsorted::CurrentFrame / pReadyShapeType->ShowAnim_FrameKeep) % ShowAnimSHP->Frames;

				Point2D posAnim;
				posAnim = {
					posTime.X - NameWidth - TimeWidth - ShowAnimSHP->Width,
					posTime.Y + (NameHeight >> 1) - (ShowAnimSHP->Height >> 1)
				};
				posAnim += pReadyShapeType->ShowAnim_Offset.Get();

				RectangleStruct vRect = { 0, 0, 0, 0 };
				DSurface::Composite->GetRect(&vRect);

				auto const nFlag = BlitterFlags::None | EnumFunctions::GetTranslucentLevel(pReadyShapeType->ShowAnim_TranslucentLevel.Get());

				DSurface::Composite->DrawSHP(ShowAnimPAL, ShowAnimSHP, frameCurrent, &posAnim, &vRect, nFlag,
					0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
			}
		}
	}

	return SkipGameCode;
}

DEFINE_HOOK(0x6D4D3C, SuperClass_Timer_DrawText_UIName, 0x5)
{
	LEA_STACK(TextPrintType*, Flag, STACK_OFFSET(0x658, -0x644));

	*Flag = TextPrintType::Right | RulesExt::Global()->TextType_SW.Get(TextPrintType::Background);

	return 0;
}

DEFINE_HOOK(0x6D4D94, SuperClass_Timer_DrawText_Time, 0x5)
{
	LEA_STACK(TextPrintType*, Flag, STACK_OFFSET(0x65C, -0x648));

	*Flag = TextPrintType::Right | RulesExt::Global()->TextType_SW.Get(TextPrintType::Background);

	return 0;
}
