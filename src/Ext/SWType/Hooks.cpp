#include "Body.h"

#include <SuperClass.h>
#include <BuildingClass.h>
#include <HouseClass.h>
#include <TechnoClass.h>

#include <Misc/PhobosGlobal.h>

#include <BitFont.h>
#include <Utilities/EnumFunctions.h>

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

	if (!pTypeExt->TimerPercentage.Get(pRulesExt->TimerPercentage))
		return 0;

	int left = pSuper->RechargeTimer.GetTimeLeft();
	int total = pSuper->GetRechargeTime();
	int passed = total - left;

	if (left >= 0 && total >= 0)
	{
		wchar_t textTime[0x30] = L"\0";
		wchar_t textName[0x100] = L"\0";
		const wchar_t* uiname = pSuper->Type->UIName;
		double ratio = (total == 0) ? 1 : (double)passed / (double)total * 100;

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
		swprintf_s(textName, L"%ls  ", uiname);

		RectangleStruct bounds = { 0, 0, 0, 0 };
		DSurface::Composite->GetRect(&bounds);

		int TimeWidth = 0, TimeHeight = 0;
		int NameWidth = 0, NameHeight = 0;
		BitFont::Instance->GetTextDimension(textTime, &TimeWidth, &TimeHeight, 200);
		BitFont::Instance->GetTextDimension(textName, &NameWidth, &NameHeight, 200);

		Point2D posTime = {
			DSurface::ViewBounds->Width - 3,
			DSurface::ViewBounds->Height - (index + 1) * (TimeHeight + 2), // see 0x6D4D0B
		};
		Point2D posName = {
			posTime.X - TimeWidth + 11 + pTypeExt->TimerXOffset.Get(pRulesExt->TimerXOffset),
			posTime.Y
		};

		TextPrintType flags = TextPrintType::Right | pRulesExt->TextType_SW.Get(TextPrintType::Background);

		ColorStruct decidedColor = pSuper->Owner->Color;
		int frames = pRulesExt->TimerFlashFrames;
		if ( !pSuper->RechargeTimer.HasTimeLeft() && ( Unsorted::CurrentFrame % (2 * frames) > (frames - 1) ) )
			decidedColor = { 255, 255, 255 };

		DSurface::Composite->DrawTextA(textTime, &bounds, &posTime, Drawing::RGB_To_Int(decidedColor), 0, flags);
		DSurface::Composite->DrawTextA(textName, &bounds, &posName, Drawing::RGB_To_Int(pSuper->Owner->Color), 0, flags);

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

	return 0;
}
