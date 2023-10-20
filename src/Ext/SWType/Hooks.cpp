#include "Body.h"

#include <BitFont.h>
#include <EventClass.h>
#include <GameStrings.h>
#include <WWMouseClass.h>

#include <Ext/House/Body.h>
#include <Ext/Side/Body.h>

#include <Misc/PhobosGlobal.h>
#include <Misc/PhobosToolTip.h>

#include <Utilities/EnumFunctions.h>
#include <Utilities/GeneralUtils.h>
#include <Utilities/Macro.h>

//Ares hooked at 0x6CC390 and jumped to 0x6CDE40
// If a super is not handled by Ares however, we do it at the original entry point
DEFINE_HOOK_AGAIN(0x6CC390, SuperClass_Place_FireExt, 0x6)
DEFINE_HOOK(0x6CDE40, SuperClass_Place_FireExt, 0x4)
{
	GET(SuperClass* const, pSuper, ECX);
	GET_STACK(CellStruct const* const, pCell, 0x4);
	GET_STACK(bool const, isPlayer, 0x8);

	if (R->Origin() == 0x6CC390)
	{
		Debug::Log("[Phobos Launch] %s\n", pSuper->Type->get_ID());

		auto const handled = SWTypeExt::Activate(pSuper, *pCell, isPlayer);

		if (handled)
			return 0x6CDE40;
	}

	SWTypeExt::FireSuperWeaponExt(pSuper, *pCell);

	return 0;
}

//DEFINE_HOOK(0x50B1CA, SuperClass_After_Update, 0x6)
//{
//	// PhobosGlobal::Global()->CheckSuperQueued();
//	// PhobosGlobal::Global()->CheckFallUnitQueued();
//
//	return 0;
//}

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

	/*
	if (!(pTypeExt->TimerPercentage.Get(pRulesExt->TimerPercentage) ||
		((pTypeExt->SW_Cumulative_ShowTrueTimer || pTypeExt->SW_Cumulative_ShowCountInTimer) && pTypeExt->SW_Cumulative) ||
		pTypeExt->ShowTimerCustom ||
		pTypeExt->TimerXOffset.Get(pRulesExt->TimerXOffset != 0) ||
		pRulesExt->TimerFlashFrames != 60))
		return 0;
	*/

	int left = pSuper->RechargeTimer.GetTimeLeft();
	int total = pSuper->GetRechargeTime();
	int passed = total - left;

	wchar_t textTime[0x30] = L"\0";
	wchar_t textName[0x100] = L"\0";

	const wchar_t* uiname = pSuper->Type->UIName;
	if (pTypeExt->ShowTimerCustom_UIName.isset())
		uiname = pTypeExt->ShowTimerCustom_UIName.Get().Text;

	auto pHouseExt = HouseExt::ExtMap.Find(pSuper->Owner);

	int count = 0;

	int superindex = 0;
	for (int i = 0; i < pSuper->Owner->Supers.Count; i++)
	{
		if (pSuper == pSuper->Owner->Supers[i])
		{
			superindex = i;
			break;
		}
	}
	if (pTypeExt->SW_Cumulative && pTypeExt->SW_Cumulative_ShowCountInTimer)
	{
		if (left <= 0)
		{
			count = 1;
		}

		if (pHouseExt->SuperWeaponCumulativeCount[superindex] > 0)
		{
			count += pHouseExt->SuperWeaponCumulativeCount[superindex];
		}
	}

	if (pTypeExt->SW_Cumulative && pTypeExt->SW_Cumulative_ShowTrueTimer)
	{
		if (left <= 0 && pHouseExt->SuperWeaponCumulativeCount[superindex] < pHouseExt->SuperWeaponCumulativeMaxCount[superindex])
		{
			left = pHouseExt->SuperWeaponCumulativeCharge[superindex];
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

	if ((TimeWidth / 5) > 2)
	{
		for (int i = 0; i < (TimeWidth / 5); i++)
		{
			wcscat_s(textName, L" ");
		}
	}
	else
	{
		wcscat_s(textName, L"  ");
	}
	/*
	int spacefixWidth = 0, spacefixHeight = 0;
	BitFont::Instance->GetTextDimension(spacefix, &spacefixWidth, &spacefixHeight, 200);
	spacefixWidth -= 2; */

	wchar_t textXOffset[0x100] = L"\0";
	for (int i = 0; i < abs(pTypeExt->TimerXOffset.Get(pRulesExt->TimerXOffset)); i++)
	{
		wcscat_s(textXOffset, L" ");
	}
	int textXOffsetWidth = 0, textXOffsetHeight = 0;
	BitFont::Instance->GetTextDimension(textXOffset, &textXOffsetWidth, &textXOffsetHeight, 200);
	if (pTypeExt->TimerXOffset.Get(pRulesExt->TimerXOffset) < 0)
	{
		textXOffsetWidth = -textXOffsetWidth;
	}

	BitFont::Instance->GetTextDimension(textName, &NameWidth, &NameHeight, 200);

	Point2D posTime = {
		DSurface::ViewBounds->Width - 3,
		DSurface::ViewBounds->Height - (index + 1) * (TimeHeight + 2), // see 0x6D4D0B
	};
	Point2D posName = {
		//posTime.X - TimeWidth + 11 - spacefixWidth + pTypeExt->TimerXOffset.Get(pRulesExt->TimerXOffset),
		posTime.X + textXOffsetWidth,
		posTime.Y
	};

	TextPrintType textFlags = TextPrintType::Right | pRulesExt->TextType_SW.Get(TextPrintType::Background);
	TextPrintType timeflags = TextPrintType::Right | TextPrintType::NoShadow;

	ColorScheme* pHouseColorScheme = ColorScheme::Array->Items[pSuper->Owner->ColorSchemeIndex];

	DSurface::Composite->DrawText(textName, bounds, posName, pHouseColorScheme, 0, textFlags);

	Point2D posOffset = posTime;
	posOffset.X += 2;
	if (textXOffsetWidth < 0)
	{
		DSurface::Composite->DrawText(textXOffset, bounds, posOffset, pHouseColorScheme, 0, textFlags);
	}

	int frames = pRulesExt->TimerFlashFrames;
	if (!pSuper->RechargeTimer.HasTimeLeft() && (Unsorted::CurrentFrame % (2 * frames) > (frames - 1)))
		DSurface::Composite->DrawText(textTime, bounds, posTime, COLOR_WHITE, 0, timeflags);
	else
		DSurface::Composite->DrawText(textTime, bounds, posTime, pHouseColorScheme, 0, timeflags);

	if (!pSuper->RechargeTimer.HasTimeLeft()) // 100% already
	{
		GScreenAnimTypeClass* pReadyShapeType = pRulesExt->ReadyShapeType_SW.Get();
		if (pReadyShapeType)
		{
			SHPStruct* shape = pReadyShapeType->SHP_ShowAnim;
			ConvertClass* palette = pReadyShapeType->PAL_ShowAnim;
			if (shape && palette)
			{
				int frameCurrent = (Unsorted::CurrentFrame / pReadyShapeType->ShowAnim_FrameKeep) % shape->Frames;

				Point2D posAnim;
				posAnim = {
					//posTime.X - NameWidth - spacefixWidth - TimeWidth - ShowAnimSHP->Width,
					posTime.X - NameWidth - TimeWidth - shape->Width,
					posTime.Y + (NameHeight >> 1) - (shape->Height >> 1)
				};
				posAnim += pReadyShapeType->ShowAnim_Offset.Get();

				RectangleStruct rect = { 0, 0, 0, 0 };
				DSurface::Composite->GetRect(&rect);

				DSurface::Composite->DrawSHP
				(
					palette,
					shape,
					frameCurrent,
					posAnim,
					rect,
					BlitterFlags::None | EnumFunctions::GetTranslucentLevel(pReadyShapeType->ShowAnim_TranslucentLevel.Get())
				);
			}
		}
	}

	return SkipGameCode;
}

DEFINE_HOOK(0x6D4D3C, SuperClass_Timer_DrawText_UIName, 0x5)
{
	LEA_STACK(TextPrintType*, Flag, STACK_OFFSET(0x658, -0x644));
	GET_STACK(Point2D*, Location, STACK_OFFSET(0x658, -0x650));
	GET_STACK(int, SecondsLeft, STACK_OFFSET(0x658, 0x8));
	GET(wchar_t*, Text, EDX);

	auto pRulesExt = RulesExt::Global();

	if (!pRulesExt->TextType_SW.isset())
		return 0;

	TextPrintType pTextType = pRulesExt->TextType_SW.Get();
	switch (pTextType)
	{
	case TextPrintType::Background:
		break;

	case TextPrintType::NoShadow:
		*Flag = TextPrintType::Right;
		break;

	case TextPrintType::GradAll:
	{
		*Flag = TextPrintType::Right | pTextType;

		int decidedCount = (SecondsLeft >= 3600) ?
			pRulesExt->TimerXOffset_HMS.Get() : pRulesExt->TimerXOffset_MS.Get();

		Location->X = DSurface::ViewBounds->Width - 3;

		wchar_t LongText[0x20] = L"";
		swprintf_s(LongText, Text);
		for (int count = 0; count < decidedCount; count++)
			wcscat_s(LongText, L" ");

		R->EDX(LongText);
	}break;

	default:
		break;
	}

	return 0;
}

DEFINE_HOOK(0x6D4D94, SuperClass_Timer_DrawText_Time, 0x5)
{
	LEA_STACK(TextPrintType*, Flag, STACK_OFFSET(0x65C, -0x648));

	if (RulesExt::Global()->TextType_SW.isset())
	{
		if (RulesExt::Global()->TextType_SW.Get() != TextPrintType::Background)
			*Flag = TextPrintType::Right;
	}

	return 0;
}

DEFINE_HOOK(0x6DC3C3, TacticalClass_Render_SW_SquaredRange, 0x6)
{
	enum { SkipEllipse = 0x6DC3F3 };

	GET(SuperWeaponTypeClass*, pSWType, EDI);
	GET_STACK(CoordStruct, Coord, 0x3C);
	GET(float, range, EAX);

	const auto pTypeExt = SWTypeExt::ExtMap.Find(pSWType);
	if (pTypeExt->SW_Squared.Get())
	{
		Point2D pos = TacticalClass::Instance->CoordsToClient(Coord);
		pos.Y -= 15;
		pos += pTypeExt->SW_Squared_Offset.Get();

		double decidedRange = pTypeExt->SW_Squared_Range.Get(static_cast<double>(range));

		GeneralUtils::DrawSquare(pos, decidedRange, Drawing::RGB_To_Int(HouseClass::CurrentPlayer->Color));

		return SkipEllipse;
	}

	return 0;
}

void __declspec(naked) _SkipEllipse_Multi()
{
	ADD_ESP(0xC);
	JMP(0x6DC37E);
}
DEFINE_HOOK(0x6DC353, TacticalClass_Render_SW_SquaredRange_Multi, 0x6)
{
	GET(SuperWeaponTypeClass*, pSWType, EDI);
	GET_STACK(CoordStruct, Coord, 0x48);
	GET_STACK(float, range, 0x0);

	const auto pTypeExt = SWTypeExt::ExtMap.Find(pSWType);
	if (pTypeExt->SW_Squared.Get())
	{
		Point2D pos = TacticalClass::Instance->CoordsToClient(Coord);
		pos.Y -= 15;
		pos += pTypeExt->SW_Squared_Offset.Get();

		double decidedRange = static_cast<double>(range);
		if (pTypeExt->SW_Squared_Range.isset())
		{
			decidedRange = pSWType->Range > 0 ?
				range / pSWType->Range * pTypeExt->SW_Squared_Range.Get() :
				pTypeExt->SW_Squared_Range.Get();
		}

		GeneralUtils::DrawSquare(pos, decidedRange, Drawing::RGB_To_Int(HouseClass::CurrentPlayer->Color));

		return (int)_SkipEllipse_Multi;
	}

	return 0;
}

DEFINE_HOOK(0x69300B, MouseClass_UpdateCursor, 0x6)
{
	const auto pRulesExt = RulesExt::Global();
	const auto pCurrent = HouseClass::CurrentPlayer();
	if (!pRulesExt->EnableSWBar || pCurrent->Defeated)
		return 0;

	int superCount = 0;

	for (const auto pSuper : pCurrent->Supers)
	{
		if (pRulesExt->MaxSW_Global.Get() >= 0 && superCount >= pRulesExt->MaxSW_Global.Get())
			break;

		if (pSuper->Granted && SWTypeExt::ExtMap.Find(pSuper->Type)->InSWBar)
			superCount++;
	}

	if (superCount == 0)
		return 0;

	const Point2D crdCursor = { WWMouseClass::Instance->GetX(), WWMouseClass::Instance->GetY() };
	const int cameoWidth = 60;
	const int cameoHeight = 48;
	Point2D location = { 0, (DSurface::ViewBounds->Height - std::min(superCount, pRulesExt->MaxSWPerRow.Get()) * cameoHeight) / 2 };
	int location_Y = location.Y;
	int row = 0, line = 0;

	for (int idx = 0; idx < superCount && pRulesExt->MaxSWPerRow - line > 0; idx++)
	{
		if (crdCursor.X > location.X && crdCursor.X < location.X + cameoWidth && crdCursor.Y > location.Y && crdCursor.Y < location.Y + cameoHeight)
		{
			R->Stack(STACK_OFFSET(0x30, -0x24), NULL);
			R->EAX(Action::None);
			return 0x69301A;
		}

		row++;

		if (row >= pRulesExt->MaxSWPerRow - line)
		{
			row = 0;
			line++;
			location_Y += cameoHeight / 2;
			location = { location.X + cameoWidth, location_Y };
		}
		else
		{
			location.Y += cameoHeight;
		}
	}

	return 0;
}

DEFINE_HOOK(0x6931A5, MouseClass_UpdateCursor_LeftPress, 0x6)
{
	const auto pRulesExt = RulesExt::Global();
	const auto pCurrent = HouseClass::CurrentPlayer();
	if (!pRulesExt->EnableSWBar || pCurrent->Defeated)
		return 0;

	int superCount = 0;

	for (const auto pSuper : pCurrent->Supers)
	{
		if (pRulesExt->MaxSW_Global.Get() >= 0 && superCount >= pRulesExt->MaxSW_Global.Get())
			break;

		if (pSuper->Granted && SWTypeExt::ExtMap.Find(pSuper->Type)->InSWBar)
			superCount++;
	}

	if (superCount == 0)
		return 0;

	const Point2D crdCursor = { WWMouseClass::Instance->GetX(), WWMouseClass::Instance->GetY() };
	const int cameoWidth = 60;
	const int cameoHeight = 48;
	Point2D location = { 0, (DSurface::ViewBounds->Height - std::min(superCount, pRulesExt->MaxSWPerRow.Get()) * cameoHeight) / 2 };
	int location_Y = location.Y;
	int row = 0, line = 0;

	for (int idx = 0; idx < superCount && pRulesExt->MaxSWPerRow - line > 0; idx++)
	{
		if (crdCursor.X > location.X && crdCursor.X < location.X + cameoWidth && crdCursor.Y > location.Y && crdCursor.Y < location.Y + cameoHeight)
		{
			R->EAX(Action::None);
			return 0x6931B4;
		}

		row++;

		if (row >= pRulesExt->MaxSWPerRow - line)
		{
			row = 0;
			line++;
			location_Y += cameoHeight / 2;
			location = { location.X + cameoWidth, location_Y };
		}
		else
		{
			location.Y += cameoHeight;
		}
	}

	return 0;
}

DEFINE_HOOK(0x693268, MouseClass_UpdateCursor_LeftRelease, 0x5)
{
	const auto pRulesExt = RulesExt::Global();
	const auto pCurrent = HouseClass::CurrentPlayer();
	if (!pRulesExt->EnableSWBar || pCurrent->Defeated)
		return 0;

	std::vector<SuperClass*> grantedSupers;

	for (const auto pSuper : pCurrent->Supers)
	{
		if (pRulesExt->MaxSW_Global.Get() >= 0 && static_cast<int>(grantedSupers.size()) >= pRulesExt->MaxSW_Global.Get())
			break;

		if (pSuper->Granted && SWTypeExt::ExtMap.Find(pSuper->Type)->InSWBar)
			grantedSupers.emplace_back(pSuper);
	}

	if (grantedSupers.empty())
		return 0;

	std::sort(grantedSupers.begin(), grantedSupers.end(),
		[](SuperClass* a, SuperClass* b)
		{
			return BuildType::SortsBefore
			(
				AbstractType::Special,
				a->Type->ArrayIndex,
				AbstractType::Special,
				b->Type->ArrayIndex
			);
		});

	const Point2D crdCursor = { WWMouseClass::Instance->GetX(), WWMouseClass::Instance->GetY() };
	const int cameoWidth = 60;
	const int cameoHeight = 48;
	const int superCount = static_cast<int>(grantedSupers.size());
	Point2D location = { 0, (DSurface::ViewBounds->Height - std::min(superCount, pRulesExt->MaxSWPerRow.Get()) * cameoHeight) / 2 };
	int location_Y = location.Y;
	int row = 0, line = 0;

	for (const auto pSuper : grantedSupers)
	{
		if (crdCursor.X > location.X && crdCursor.X < location.X + cameoWidth && crdCursor.Y > location.Y && crdCursor.Y < location.Y + cameoHeight)
		{
			bool useAITargeting = false;

			if (const auto pSWExt = SWTypeExt::ExtMap.Find(pSuper->Type))
			{
				useAITargeting = pSWExt->SW_UseAITargeting;
				VocClass::PlayGlobal(RulesClass::Instance->GUIBuildSound, 0x2000, 1.0);
				const bool manual = !pSWExt->SW_ManualFire && pSWExt->SW_AutoFire;
				const bool unstopable = pSuper->Type->UseChargeDrain && pSuper->ChargeDrainState == ChargeDrainState::Draining
					&& pSWExt->SW_Unstoppable;

				if (!pSuper->CanFire() && !manual)
				{
					VoxClass::PlayIndex(pSWExt->EVA_Impatient);
				}
				else
				{
					if (!pCurrent->CanTransactMoney(pSWExt->Money_Amount))
					{
						VoxClass::PlayIndex(pSWExt->EVA_InsufficientFunds.Get(VoxClass::FindIndex(GameStrings::EVA_InsufficientFunds)));
						pSWExt->PrintMessage(pSWExt->Message_InsufficientFunds, pCurrent);
					}
					else if (!manual && !unstopable)
					{
						const auto swIndex = pSuper->Type->ArrayIndex;

						if (pSuper->Type->Action == Action::None || useAITargeting)
						{
							EventClass::AddEvent(EventClass(pCurrent->ArrayIndex, EventType::SPECIAL_PLACE, swIndex, CellStruct::Empty));
						}
						else
						{
							DisplayClass::Instance->CurrentBuilding = nullptr;
							DisplayClass::Instance->CurrentBuildingType = nullptr;
							DisplayClass::Instance->unknown_11AC = static_cast<DWORD>(-1);
							//DisplayClass::Instance->SetActiveFoundation(nullptr);
							MapClass::Instance->SetRepairMode(0);
							DisplayClass::Instance->SetSellMode(0);
							DisplayClass::Instance->PowerToggleMode = false;
							DisplayClass::Instance->PlanningMode = false;
							DisplayClass::Instance->PlaceBeaconMode = false;
							DisplayClass::Instance->CurrentSWTypeIndex = swIndex;
							MapClass::Instance->UnselectAll();
							VoxClass::PlayIndex(pSWExt->EVA_SelectTarget.Get(VoxClass::FindIndex(GameStrings::EVA_SelectTarget)));
						}
					}
				}
			}
			if (pSuper->CanFire())
			{
				MapClass::Instance->UnselectAll();

				if(!useAITargeting)
					Unsorted::CurrentSWType = pSuper->Type->ArrayIndex;
			}

			R->Stack(STACK_OFFSET(0x28, 0x8), NULL);
			R->EAX(Action::None);
			return 0x693276;
		}

		row++;

		if (row >= pRulesExt->MaxSWPerRow - line)
		{
			row = 0;
			line++;
			location_Y += cameoHeight / 2;
			location = { location.X + cameoWidth, location_Y };
		}
		else
		{
			location.Y += cameoHeight;
		}
	}

	return 0;
}

DEFINE_HOOK(0x4F4583, GScreenClass_DrawOnTop_TheDarkSideOfTheMoon, 0x6)
{
	const auto pRulesExt = RulesExt::Global();
	const auto pCurrent = HouseClass::CurrentPlayer();
	if (!pRulesExt->EnableSWBar || !pCurrent || pCurrent->Defeated)
		return 0;

	std::vector<SuperClass*> grantedSupers;

	for (const auto pSuper : pCurrent->Supers)
	{
		if (pRulesExt->MaxSW_Global.Get() >= 0 && static_cast<int>(grantedSupers.size()) >= pRulesExt->MaxSW_Global.Get())
			break;

		if (pSuper->Granted && SWTypeExt::ExtMap.Find(pSuper->Type)->InSWBar)
			grantedSupers.emplace_back(pSuper);
	}

	if (grantedSupers.empty())
		return 0;

	std::sort(grantedSupers.begin(), grantedSupers.end(),
		[](SuperClass* a, SuperClass* b)
		{
			return BuildType::SortsBefore
			(
				AbstractType::Special,
				a->Type->ArrayIndex,
				AbstractType::Special,
				b->Type->ArrayIndex
			);
		});

	DSurface* pSurface = DSurface::Composite;
	BitFont* pFont = BitFont::Instance;
	RectangleStruct bounds = DSurface::ViewBounds();
	const Point2D crdCursor = { WWMouseClass::Instance->GetX(), WWMouseClass::Instance->GetY() };
	const int cameoWidth = 60;
	const int cameoHeight = 48;
	const int superCount = static_cast<int>(grantedSupers.size());
	Point2D location = { 0, (bounds.Height - std::min(superCount, pRulesExt->MaxSWPerRow.Get()) * cameoHeight) / 2 };
	int location_Y = location.Y;
	RectangleStruct destRect = { 0, location.Y, cameoWidth, cameoHeight };
	Point2D tooptipLocation = { cameoWidth, 0 };
	const COLORREF tooltipColor = Drawing::RGB_To_Int(Drawing::TooltipColor());
	SWTypeExt::ExtData* tooltipTypeExt = nullptr;
	int row = 0, line = 0;

	if (pRulesExt->SWBarSHP_Top.isset())
	{
		const auto pSHP = pRulesExt->SWBarSHP_Top.Get();
		Point2D loc = { 0, location.Y - pSHP->Height };

		pSurface->DrawSHP
		(
			pRulesExt->SWBarPalette.GetOrDefaultConvert(FileSystem::PALETTE_PAL),
			pSHP,
			0,
			loc,
			bounds, BlitterFlags::bf_400
		);
	}

	for (int idx = 0; idx < superCount && pRulesExt->MaxSWPerRow - line > 0; idx++)
	{
		const auto pSuper = grantedSupers[idx];

		if (const auto pSWExt = SWTypeExt::ExtMap.Find(pSuper->Type))
		{
			// support for pcx cameos
			if (auto pPCX = pSWExt->SidebarPCX.GetSurface())
			{
				PCX::Instance->BlitToSurface(&destRect, pSurface, pPCX);
			}
			else if (auto pCameo = pSuper->Type->SidebarImage) // old shp cameos, fixed palette
			{
				auto pCameoRef = pCameo->AsReference();
				char pFilename[0x20];
				strcpy_s(pFilename, pRulesExt->MissingCameo.data());
				_strlwr_s(pFilename);

				if (!_stricmp(pCameoRef->Filename, "xxicon.shp") && strstr(pFilename, ".pcx"))
				{
					PCX::Instance->LoadFile(pFilename);
					if (auto CameoPCX = PCX::Instance->GetSurface(pFilename))
						PCX::Instance->BlitToSurface(&destRect, pSurface, CameoPCX);
				}
				else
				{
					auto pConvert = pSWExt->CameoPal.Convert ? pSWExt->CameoPal.GetConvert() : FileSystem::CAMEO_PAL;
					pSurface->DrawSHP
					(
						pConvert,
						pCameo,
						0,
						location,
						bounds,
						BlitterFlags::bf_400
					);
				}
			}
			else
				continue;

			if (pRulesExt->SWBarSHP_Right.isset())
			{
				const auto pSHP = pRulesExt->SWBarSHP_Right.Get();

				pSurface->DrawSHP
				(
					pRulesExt->SWBarPalette.GetOrDefaultConvert(FileSystem::PALETTE_PAL),
					pSHP,
					0,
					location,
					bounds,
					BlitterFlags::bf_400
				);
			}

			if (pSuper->IsCharged && !pCurrent->CanTransactMoney(pSWExt->Money_Amount))
			{
				pSurface->DrawSHP
				(
					FileSystem::SIDEBAR_PAL,
					FileSystem::DARKEN_SHP,
					0,
					location,
					bounds,
					BlitterFlags::Darken | BlitterFlags::bf_400
				);
			}

			if (!tooltipTypeExt && crdCursor.X > location.X && crdCursor.X < location.X + cameoWidth &&
				crdCursor.Y > location.Y && crdCursor.Y < location.Y + cameoHeight)
			{
				tooltipTypeExt = pSWExt;
				tooptipLocation = { location.X + cameoWidth, location.Y };

				RectangleStruct cameoRect = { location.X, location.Y, cameoWidth, cameoHeight };
				pSurface->DrawRect(&cameoRect, tooltipColor);
			}

			if (!pSuper->RechargeTimer.Completed())
			{
				Point2D loc = { location.X, location.Y };
				pSurface->DrawSHP
				(
					FileSystem::SIDEBAR_PAL,
					FileSystem::GCLOCK2_SHP,
					pSuper->GetCameoChargeState() + 1,
					loc,
					bounds,
					BlitterFlags::bf_400 | BlitterFlags::TransLucent50
				);
			}

			const auto buffer = pSuper->NameReadiness();;

			if (GeneralUtils::IsValidString(buffer))
			{
				Point2D textLoc = { location.X + cameoWidth / 2, location.Y };
				TextPrintType printType = TextPrintType::FullShadow | TextPrintType::Point8 | pRulesExt->TextType_Hold_Singular.Get(TextPrintType::Background) | TextPrintType::Center;

				pSurface->DrawText(buffer, bounds, textLoc, tooltipColor, 0, printType);
			}

			row++;

			if (row >= pRulesExt->MaxSWPerRow - line)
			{
				if (pRulesExt->SWBarSHP_Bottom.isset())
				{
					const auto pSHP = pRulesExt->SWBarSHP_Bottom.Get();

					pSurface->DrawSHP
					(
						pRulesExt->SWBarPalette.GetOrDefaultConvert(FileSystem::PALETTE_PAL),
						pSHP,
						0,
						location,
						bounds,
						BlitterFlags::bf_400
					);
				}

				row = 0;
				line++;
				location_Y += cameoHeight / 2;
				location = { location.X + cameoWidth, location_Y };
				destRect.X = location.X;
				destRect.Y = location.Y;
			}
			else
			{
				location.Y += cameoHeight;
				destRect.Y += cameoHeight;
			}
		}
	}

#pragma region Draw Tooltip
	if (tooltipTypeExt)
	{
		PhobosToolTip::Instance.HelpText(tooltipTypeExt->OwnerObject());
		const wchar_t* pDesc = PhobosToolTip::Instance.GetBuffer();

		if (GeneralUtils::IsValidString(pDesc))
		{
			const int maxWidth = Phobos::UI::MaxToolTipWidth > 0 ? Phobos::UI::MaxToolTipWidth : bounds.Width;
			int width = 0, height = 0;

			auto GetTextRect = [&pFont, &width, &height, maxWidth](const wchar_t* text)
			{
				int nWidth = 0, nHeight = 0;
				pFont->GetTextDimension(text, &nWidth, &nHeight, maxWidth);

				width = std::max(width, nWidth);
				height += nHeight;
			};

			GetTextRect(pDesc);

			width += 8;
			height += 5;
			tooptipLocation.Y = std::clamp(tooptipLocation.Y, 0, bounds.Height - height + cameoHeight);
			RectangleStruct fillRect = { tooptipLocation.X, tooptipLocation.Y, width, height };
			ColorStruct fillColor = { 0,0,0 };
			int fillOpacity = 100;

			if (auto const pSide = SideClass::Array->GetItemOrDefault(ScenarioClass::Instance->PlayerSideIndex))
			{
				if (auto pSideExt = SideExt::ExtMap.Find(pSide))
				{
					fillColor = pSideExt->ToolTip_Background_Color.Get(pRulesExt->ToolTip_Background_Color);
					fillOpacity = pSideExt->ToolTip_Background_Opacity.Get(pRulesExt->ToolTip_Background_Opacity);
				}
			}

			pSurface->FillRectTrans(&fillRect, fillColor, fillOpacity);
			pSurface->DrawRect(&fillRect, tooltipColor);

			tooptipLocation += { 4, 3 };
			const int textHeight = pFont->InternalPTR->FontHeight + 4;

			auto DrawTextBox = [&tooptipLocation, textHeight, tooltipColor](const wchar_t* pText)
			{
				wchar_t buffer[0x400];
				wcscpy_s(buffer, pText);
				wchar_t* context = nullptr, delims[4] = L"\n";
				DSurface* pSurface = DSurface::Composite;
				const TextPrintType printType = TextPrintType::FullShadow | TextPrintType::Point8;

				for (auto pCur = wcstok_s(buffer, delims, &context); pCur; pCur = wcstok_s(nullptr, delims, &context))
				{
					if (!wcslen(pCur))
						continue;

					pSurface->DrawText
					(
						pCur,
						DSurface::ViewBounds,
						tooptipLocation,
						tooltipColor,
						0,
						printType
					);

					tooptipLocation.Y += textHeight;
				}
			};

			DrawTextBox(pDesc);
		}
	}
#pragma endregion
	return 0;
}

DEFINE_HOOK(0x6DBE74, Tactical_SuperLinesCircles_ShowDesignatorRange, 0x7)
{
	if (!Phobos::Config::ShowDesignatorRange || !(RulesExt::Global()->ShowDesignatorRange) || Unsorted::CurrentSWType == -1)
		return 0;

	const auto pSuperType = SuperWeaponTypeClass::Array()->GetItem(Unsorted::CurrentSWType);
	const auto pExt = SWTypeExt::ExtMap.Find(pSuperType);

	if (!pExt->ShowDesignatorRange)
		return 0;

	for (const auto pCurrentTechno : *TechnoClass::Array)
	{
		const auto pCurrentTechnoType = pCurrentTechno->GetTechnoType();
		const auto pOwner = pCurrentTechno->Owner;

		if (!pCurrentTechno->IsAlive
			|| pCurrentTechno->InLimbo
			|| (pOwner != HouseClass::CurrentPlayer && pOwner->IsAlliedWith(HouseClass::CurrentPlayer))                  // Ally objects are never designators or inhibitors
			|| (pOwner == HouseClass::CurrentPlayer && !pExt->SW_Designators.Contains(pCurrentTechnoType))               // Only owned objects can be designators
			|| (!pOwner->IsAlliedWith(HouseClass::CurrentPlayer) && !pExt->SW_Inhibitors.Contains(pCurrentTechnoType)))  // Only enemy objects can be inhibitors
		{
			continue;
		}

		const auto pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pCurrentTechnoType);

		const float radius = pOwner == HouseClass::CurrentPlayer
			? (float)(pTechnoTypeExt->DesignatorRange.Get(pCurrentTechnoType->Sight))
			: (float)(pTechnoTypeExt->InhibitorRange.Get(pCurrentTechnoType->Sight));

		CoordStruct coords = pCurrentTechno->GetCenterCoords();
		coords.Z = MapClass::Instance->GetCellFloorHeight(coords);
		const auto color = pOwner->Color;
		Game::DrawRadialIndicator(false, true, coords, color, radius, false, true);
	}

	return 0;
}
