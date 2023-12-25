#include <Phobos.h>

#include <Helpers/Macro.h>
#include <PreviewClass.h>
#include <Surface.h>

#include <Ext/Scenario/Body.h>
#include <Ext/House/Body.h>
#include <Ext/Side/Body.h>
#include <Ext/Rules/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/SWType/Body.h>
#include <Misc/FlyingStrings.h>
#include <New/Entity/BannerClass.h>
#include <Utilities/Debug.h>
#include <Utilities/Macro.h>

DEFINE_HOOK(0x777C41, UI_ApplyAppIcon, 0x9)
{
	if (Phobos::AppIconPath != nullptr)
	{
		Debug::Log("Applying AppIcon from \"%s\"\n", Phobos::AppIconPath);

		R->EAX(LoadImage(NULL, Phobos::AppIconPath, IMAGE_ICON, 0, 0, LR_LOADFROMFILE));
		return 0x777C4A;
	}

	return 0;
}

DEFINE_HOOK(0x640B8D, LoadingScreen_DisableEmptySpawnPositions, 0x6)
{
	GET(bool, esi, ESI);
	if (Phobos::UI::DisableEmptySpawnPositions || !esi)
	{
		return 0x640CE2;
	}
	return 0x640B93;
}

//DEFINE_HOOK(0x640E78, LoadingScreen_DisableColorPoints, 0x6)
//{
//	return 0x641071;
//}

// Allow size = 0 for map previews
DEFINE_HOOK(0x641B41, LoadingScreen_SkipPreview, 0x8)
{
	GET(RectangleStruct*, pRect, EAX);
	if (pRect->Width > 0 && pRect->Height > 0)
	{
		return 0;
	}
	return 0x641D4E;
}

DEFINE_HOOK(0x641EE0, PreviewClass_ReadPreview, 0x6)
{
	GET(PreviewClass*, pThis, ECX);
	GET_STACK(const char*, lpMapFile, 0x4);

	CCFileClass file(lpMapFile);
	if (file.Exists() && file.Open(FileAccessMode::Read))
	{
		CCINIClass ini;
		ini.ReadCCFile(&file, true);
		ini.CurrentSection = nullptr;
		ini.CurrentSectionName = nullptr;

		ScenarioClass::Instance->ReadStartPoints(ini);

		R->EAX(pThis->ReadPreviewPack(ini));
	}
	else
		R->EAX(false);

	return 0x64203D;
}

DEFINE_HOOK(0x4A25E0, CreditsClass_GraphicLogic_HarvesterCounter, 0x7)
{
	auto const pPlayer = HouseClass::CurrentPlayer();
	if (pPlayer->Defeated)
		return 0;

	if (Phobos::UI::ShowHarvesterCounter)
	{
		auto pSideExt = SideExt::ExtMap.Find(SideClass::Array->GetItem(pPlayer->SideIndex));
		wchar_t counter[0x20];
		auto nActive = HouseExt::ActiveHarvesterCount(pPlayer);
		auto nTotal = HouseExt::TotalHarvesterCount(pPlayer);
		auto nPercentage = nTotal == 0 ? 1.0 : (double)nActive / (double)nTotal;
		auto TextFlags = TextPrintType::UseGradPal | TextPrintType::Metal12
			| static_cast<TextPrintType>(Phobos::UI::HarvesterCounter_Align);

		ColorStruct clrToolTip = nPercentage > Phobos::UI::HarvesterCounter_ConditionYellow
			? Drawing::TooltipColor() : nPercentage > Phobos::UI::HarvesterCounter_ConditionRed
			? pSideExt->Sidebar_HarvesterCounter_Yellow : pSideExt->Sidebar_HarvesterCounter_Red;

		swprintf_s(counter, L"%ls%d/%d", Phobos::UI::HarvesterLabel, nActive, nTotal);

		Point2D pos = {
			DSurface::Sidebar->GetWidth() / 2 + 50 + pSideExt->Sidebar_HarvesterCounter_Offset.Get().X,
			2 + pSideExt->Sidebar_HarvesterCounter_Offset.Get().Y
		};

		RectangleStruct rect;
		DSurface::Sidebar->GetRect(&rect);

		DSurface::Sidebar->DrawText(counter, rect, pos, Drawing::RGB_To_Int(clrToolTip), 0, TextFlags);
	}

	if (Phobos::UI::ShowPowerDelta)
	{
		auto pSideExt = SideExt::ExtMap.Find(SideClass::Array->GetItem(pPlayer->SideIndex));
		wchar_t counter[0x20];

		ColorStruct clrToolTip;

		if (pPlayer->PowerBlackoutTimer.InProgress())
		{
			clrToolTip = pSideExt->Sidebar_PowerDelta_Grey;
			swprintf_s(counter, L"%ls", Phobos::UI::PowerBlackoutLabel);
		}
		else
		{
			int delta = pPlayer->PowerOutput - pPlayer->PowerDrain;

			double percent = pPlayer->PowerOutput != 0
				? (double)pPlayer->PowerDrain / (double)pPlayer->PowerOutput : pPlayer->PowerDrain != 0
				? Phobos::UI::PowerDelta_ConditionRed * 2.f : Phobos::UI::PowerDelta_ConditionYellow;

			clrToolTip = percent < Phobos::UI::PowerDelta_ConditionYellow
				? pSideExt->Sidebar_PowerDelta_Green : LESS_EQUAL(percent, Phobos::UI::PowerDelta_ConditionRed)
				? pSideExt->Sidebar_PowerDelta_Yellow : pSideExt->Sidebar_PowerDelta_Red;

			swprintf_s(counter, L"%ls%+d", Phobos::UI::PowerLabel, delta);
		}

		Point2D pos = {
			DSurface::Sidebar->GetWidth() / 2 - 70 + pSideExt->Sidebar_PowerDelta_Offset.Get().X,
			2 + pSideExt->Sidebar_PowerDelta_Offset.Get().Y
		};

		RectangleStruct rect = { 0, 0, 0, 0 };
		DSurface::Sidebar->GetRect(&rect);

		auto const TextFlags = static_cast<TextPrintType>(static_cast<int>(TextPrintType::UseGradPal | TextPrintType::Metal12)
				| static_cast<int>(pSideExt->Sidebar_PowerDelta_Align.Get()));

		DSurface::Sidebar->DrawText(counter, rect, pos, Drawing::RGB_To_Int(clrToolTip), 0, TextFlags);
	}

	return 0;
}

DEFINE_HOOK(0x4A25E0, CreditsClass_GraphicLogic_ScoreCounter_Top, 0x7)
{
	if (Phobos::UI::ShowScoreCounter)
	{
		auto pPlayer = HouseClass::CurrentPlayer();
		auto pSideExt = SideExt::ExtMap.Find(SideClass::Array->GetItem(HouseClass::CurrentPlayer->SideIndex));
		if (!pSideExt->Sidebar_ScoreCounter_DrawOnCommandBar.Get())
		{
			auto TextFlags = static_cast<TextPrintType>(static_cast<int>(TextPrintType::UseGradPal | TextPrintType::Metal12)
				| static_cast<int>(pSideExt->Sidebar_ScoreCounter_Align.Get()));
			wchar_t counter[0x20];

			swprintf_s(counter, L"%d%ls", pPlayer->SiloMoney, Phobos::UI::ScoreLabel);

			Point2D pos = {
				DSurface::Sidebar->GetWidth() / 2 - 65 + pSideExt->Sidebar_ScoreCounter_Offset.Get().X,
				2 + pSideExt->Sidebar_ScoreCounter_Offset.Get().Y
			};

			RectangleStruct rect = { 0, 0, 0, 0 };
			DSurface::Sidebar->GetRect(&rect);

			DSurface::Sidebar->DrawText(counter, rect, pos, Drawing::RGB_To_Int(pSideExt->Sidebar_ScoreCounter_Color), 0, TextFlags);
		}
	}

	return 0;
}

DEFINE_HOOK(0x4F45A8, GScreenClass_Render_ScoreCounter_Bottom, 0x5)
{
	if (Phobos::UI::ShowScoreCounter)
	{
		auto pPlayer = HouseClass::CurrentPlayer();
		auto pSideExt = SideExt::ExtMap.Find(SideClass::Array->GetItem(HouseClass::CurrentPlayer->SideIndex));
		if (pSideExt->Sidebar_ScoreCounter_DrawOnCommandBar.Get())
		{
			auto TextFlags = static_cast<TextPrintType>(static_cast<int>(TextPrintType::UseGradPal | TextPrintType::Metal12)
				| static_cast<int>(pSideExt->Sidebar_ScoreCounter_Align.Get()));
			int XPosition = DSurface::Composite->GetWidth() - 80;
			int YPosition = DSurface::Composite->GetHeight() - 25;
			wchar_t counter[0x20];
			RectangleStruct rect = { 0, 0, 0, 0 };
			RectangleStruct rect2 = { XPosition - 35, YPosition - 1, 70, 18 };

			swprintf_s(counter, L"%d%ls", pPlayer->SiloMoney, Phobos::UI::ScoreLabel);

			Point2D pos = {
				XPosition + pSideExt->Sidebar_ScoreCounter_Offset.Get().X,
				YPosition + pSideExt->Sidebar_ScoreCounter_Offset.Get().Y
			};

			DSurface::Composite->GetRect(&rect);

			DSurface::Composite->FillRect(&rect2, 0);
			DSurface::Composite->DrawText(counter, rect, pos, Drawing::RGB_To_Int(pSideExt->Sidebar_ScoreCounter_Color), 0, TextFlags);
			DSurface::Composite->DrawRect(&rect2, Drawing::RGB_To_Int(pSideExt->Sidebar_ScoreCounter_Color));
		}
	}

	return 0;
}

DEFINE_HOOK_AGAIN(0x6CE8AA, Replace_XXICON_With_New, 0x7)   //SWTypeClass::Load
DEFINE_HOOK_AGAIN(0x6CEE31, Replace_XXICON_With_New, 0x7)   //SWTypeClass::ReadINI
DEFINE_HOOK_AGAIN(0x716D13, Replace_XXICON_With_New, 0x7)   //TechnoTypeClass::Load
DEFINE_HOOK(0x715A4D, Replace_XXICON_With_New, 0x7)         //TechnoTypeClass::ReadINI
{
	char pFilename[0x20];
	strcpy_s(pFilename, RulesExt::Global()->MissingCameo.data());
	_strlwr_s(pFilename);

	if (_stricmp(pFilename, "xxicon.shp")
		&& strstr(pFilename, ".shp"))
	{
		if (auto pFile = FileSystem::LoadFile(RulesExt::Global()->MissingCameo, false))
		{
			R->EAX(pFile);
			return R->Origin() + 0xC;
		}
	}

	return 0;
}

DEFINE_HOOK(0x6A8463, StripClass_OperatorLessThan_CameoPriority, 0x5)
{
	GET_STACK(TechnoTypeClass*, pLeft, STACK_OFFSET(0x1C, -0x8));
	GET_STACK(TechnoTypeClass*, pRight, STACK_OFFSET(0x1C, -0x4));
	GET_STACK(int, idxLeft, STACK_OFFSET(0x1C, 0x8));
	GET_STACK(int, idxRight, STACK_OFFSET(0x1C, 0x10));
	GET_STACK(AbstractType, rttiLeft, STACK_OFFSET(0x1C, 0x4));
	GET_STACK(AbstractType, rttiRight, STACK_OFFSET(0x1C, 0xC));
	auto pLeftTechnoExt = TechnoTypeExt::ExtMap.Find(pLeft);
	auto pRightTechnoExt = TechnoTypeExt::ExtMap.Find(pRight);
	auto pLeftSWExt = (rttiLeft == AbstractType::Special || rttiLeft == AbstractType::Super || rttiLeft == AbstractType::SuperWeaponType)
		? SWTypeExt::ExtMap.Find(SuperWeaponTypeClass::Array->GetItem(idxLeft)) : nullptr;
	auto pRightSWExt = (rttiRight == AbstractType::Special || rttiRight == AbstractType::Super || rttiRight == AbstractType::SuperWeaponType)
		? SWTypeExt::ExtMap.Find(SuperWeaponTypeClass::Array->GetItem(idxRight)) : nullptr;

	if ((pLeftTechnoExt || pLeftSWExt) && (pRightTechnoExt || pRightSWExt))
	{
		auto leftPriority = pLeftTechnoExt ? pLeftTechnoExt->CameoPriority : pLeftSWExt->CameoPriority;
		auto rightPriority = pRightTechnoExt ? pRightTechnoExt->CameoPriority : pRightSWExt->CameoPriority;
		enum { rTrue = 0x6A8692, rFalse = 0x6A86A0 };

		if (leftPriority > rightPriority)
			return rTrue;
		else if (rightPriority > leftPriority)
			return rFalse;
	}

	// Restore overridden instructions
	GET(AbstractType, rtti1, ESI);
	return rtti1 == AbstractType::Special ? 0x6A8477 : 0x6A8468;
}

DEFINE_HOOK(0x6D4B25, TacticalClass_Render_Banner, 0x5)
{
	BannerClass::Update();

	return 0;
}

DEFINE_HOOK(0x6D4684, TacticalClass_Draw_FlyingStrings, 0x6)
{
	FlyingStrings::UpdateAll();
	return 0;
}

DEFINE_HOOK(0x456776, BuildingClass_DrawRadialIndicator_Visibility, 0x6)
{
	enum { ContinueDraw = 0x456789, DoNotDraw = 0x456962 };
	GET(BuildingClass* const, pThis, ESI);

	if (HouseClass::IsCurrentPlayerObserver() || pThis->Owner->IsControlledByCurrentPlayer())
		return ContinueDraw;

	AffectedHouse const canSee = RulesExt::Global()->RadialIndicatorVisibility.Get();
	if (pThis->Owner->IsAlliedWith(HouseClass::CurrentPlayer) ? canSee & AffectedHouse::Allies : canSee & AffectedHouse::Enemies)
		return ContinueDraw;

	return DoNotDraw;
}

// disable save button on the ingame menu
// Powered by Ares 0.A source code
// Author : Alexb
DEFINE_HOOK(0x4F17F6, sub_4F1720_DisableSaves, 0x6)
{
	GET(HWND, hDlg, EBP);

	if (HWND hItem = GetDlgItem(hDlg, 1311))
	{
		if (!ScenarioExt::Global()->CanSaveGame)
		{
			EnableWindow(hItem, FALSE);
		}
		else
		{
			EnableWindow(hItem, TRUE);
		}
	}

	return 0x4F1834;
}

static bool __fastcall _DSurface_FillRect_Wrapper(DSurface* pSurface, DWORD _, RectangleStruct* pFillRect, COLORREF nColor)
{
	pFillRect->X -= 2;
	ColorStruct color = Drawing::Int_To_RGB(nColor);

	auto pRulesExt = RulesExt::Global();
	return pSurface->FillRectTrans(pFillRect, color, pRulesExt->MessageBox_Translucency);
}
DEFINE_JUMP(CALL, 0x623AA8, GET_OFFSET(_DSurface_FillRect_Wrapper));
