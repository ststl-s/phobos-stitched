#include "Body.h"

#include <HouseClass.h>
#include <FactoryClass.h>
#include <FileSystem.h>
#include <Ext/Side/Body.h>
#include <Ext/Rules/Body.h>
#include <Utilities/Macro.h>
#include <Utilities/EnumFunctions.h>
#include <Ext/House/Body.h>
#include <Ext/TechnoType/Body.h>
#include <TacticalClass.h>
#include <GameStrings.h>

DEFINE_HOOK(0x6A593E, SidebarClass_InitForHouse_AdditionalFiles, 0x5)
{
	char filename[0x20];

	for (int i = 0; i < 4; i++)
	{
		sprintf_s(filename, "tab%02dpp.shp", i);
		SidebarExt::TabProducingProgress[i] = GameCreate<SHPReference>(filename);
	}

	return 0;
}

DEFINE_HOOK(0x6A5EA1, SidebarClass_UnloadShapes_AdditionalFiles, 0x5)
{
	for (int i = 0; i < 4; i++)
	{
		if (SidebarExt::TabProducingProgress[i])
		{
			GameDelete(SidebarExt::TabProducingProgress[i]);
			SidebarExt::TabProducingProgress[i] = nullptr;
		}
	}

	return 0;
}

DEFINE_HOOK(0x6A6EB1, SidebarClass_DrawIt_ProducingProgress, 0x6)
{
	if (Phobos::UI::ShowProducingProgress)
	{
		auto pPlayer = HouseClass::CurrentPlayer();
		auto pSideExt = SideExt::ExtMap.Find(SideClass::Array->GetItem(HouseClass::CurrentPlayer->SideIndex));
		int XOffset = pSideExt->Sidebar_GDIPositions ? 29 : 32;
		int XBase = (pSideExt->Sidebar_GDIPositions ? 26 : 20) + pSideExt->Sidebar_ProducingProgress_Offset.Get().X;
		int YBase = 197 + pSideExt->Sidebar_ProducingProgress_Offset.Get().Y;

		for (int i = 0; i < 4; i++)
		{
			if (auto pSHP = SidebarExt::TabProducingProgress[i])
			{
				auto rtti = i == 0 || i == 1 ? AbstractType::BuildingType : AbstractType::InfantryType;
				FactoryClass* pFactory = nullptr;

				if (i != 3)
				{
					pFactory = pPlayer->GetPrimaryFactory(rtti, false, i == 1 ? BuildCat::Combat : BuildCat::DontCare);
				}
				else
				{
					pFactory = pPlayer->GetPrimaryFactory(AbstractType::UnitType, false, BuildCat::DontCare);
					if (!pFactory || !pFactory->Object)
						pFactory = pPlayer->GetPrimaryFactory(AbstractType::UnitType, true, BuildCat::DontCare);
					if (!pFactory || !pFactory->Object)
						pFactory = pPlayer->GetPrimaryFactory(AbstractType::AircraftType, false, BuildCat::DontCare);
				}

				int idxFrame = pFactory
					? (int)(((double)pFactory->GetProgress() / 54) * (pSHP->Frames - 1))
					: -1;

				Point2D vPos = { XBase + i * XOffset, YBase };
				RectangleStruct sidebarRect = DSurface::Sidebar()->GetRect();

				if (idxFrame != -1)
				{
					DSurface::Sidebar()->DrawSHP(FileSystem::SIDEBAR_PAL, pSHP, idxFrame, &vPos,
						&sidebarRect, BlitterFlags::bf_400, 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
				}
			}
		}
	}

	return 0;
}

// skip SidebarClass_DrawText_Background
DEFINE_JUMP(LJMP, 0x6A9DD1, 0x6A9E11); // Ready
DEFINE_JUMP(LJMP, 0x6A9F1E, 0x6A9F5E); // Holding multiple technos
DEFINE_JUMP(LJMP, 0x6A9FB3, 0x6A9FF3); // Holding singular techno or superweapon

DEFINE_HOOK(0x6A9E2D, SidebarClass_DrawText_Ready, 0x7)
{
	LEA_STACK(TextPrintType*, Flag, STACK_OFFSET(0x4A4, -0x490));
	LEA_STACK(COLORREF*, ForeColor, STACK_OFFSET(0x4A4, -0x498));

	*Flag |= static_cast<TextPrintType>(RulesExt::Global()->TextType_Ready.Get(TextPrintType::Background));

	auto pSideExt = SideExt::ExtMap.Find(SideClass::Array->GetItem(HouseClass::CurrentPlayer->SideIndex));
	auto pRulesExt = RulesExt::Global();

	if (pSideExt->TextColor_Ready.isset())
		*ForeColor = Drawing::RGB_To_Int(pSideExt->TextColor_Ready.Get());
	else if (pRulesExt->TextColor_Ready.isset())
		*ForeColor = Drawing::RGB_To_Int(pRulesExt->TextColor_Ready.Get());

	return 0;
}

DEFINE_HOOK(0x6A9F74, SidebarClass_DrawText_Hold_Multiple, 0x7)
{
	LEA_STACK(TextPrintType*, Flag, STACK_OFFSET(0x4A4, -0x490));
	LEA_STACK(COLORREF*, ForeColor, STACK_OFFSET(0x4A4, -0x498));

	*Flag |= static_cast<TextPrintType>(RulesExt::Global()->TextType_Hold_Multiple.Get(TextPrintType::Background));

	auto pSideExt = SideExt::ExtMap.Find(SideClass::Array->GetItem(HouseClass::CurrentPlayer->SideIndex));
	auto pRulesExt = RulesExt::Global();

	if (pSideExt->TextColor_Hold_Multiple.isset())
		*ForeColor = Drawing::RGB_To_Int(pSideExt->TextColor_Hold_Multiple.Get());
	else if (pRulesExt->TextColor_Hold_Multiple.isset())
		*ForeColor = Drawing::RGB_To_Int(pRulesExt->TextColor_Hold_Multiple.Get());

	return 0;
}

DEFINE_HOOK(0x6AA00B, SidebarClass_DrawText_Hold_Singular, 0x7)
{
	LEA_STACK(TextPrintType*, Flag, STACK_OFFSET(0x4A4, -0x490));
	LEA_STACK(COLORREF*, ForeColor, STACK_OFFSET(0x4A4, -0x498));

	*Flag |= static_cast<TextPrintType>(RulesExt::Global()->TextType_Hold_Singular.Get(TextPrintType::Background));

	auto pSideExt = SideExt::ExtMap.Find(SideClass::Array->GetItem(HouseClass::CurrentPlayer->SideIndex));
	auto pRulesExt = RulesExt::Global();

	if (pSideExt->TextColor_Hold_Singular.isset())
		*ForeColor = Drawing::RGB_To_Int(pSideExt->TextColor_Hold_Singular.Get());
	else if (pRulesExt->TextColor_Hold_Singular.isset())
		*ForeColor = Drawing::RGB_To_Int(pRulesExt->TextColor_Hold_Singular.Get());

	return 0;
}

DEFINE_HOOK(0x6A9E3E, SidebarClass_DrawSHP_Ready, 0x6)
{
	GET_STACK(bool, isReady, STACK_OFFSET(0x48C, -0x478));
	GET(Point2D, Location, ESI);

	if (isReady)
	{
		GScreenAnimTypeClass* pReadyShapeType = RulesExt::Global()->ReadyShapeType.Get();
		if (pReadyShapeType)
		{
			SHPStruct* ShowAnimSHP = pReadyShapeType->SHP_ShowAnim;
			ConvertClass* ShowAnimPAL = pReadyShapeType->PAL_ShowAnim;
			if (ShowAnimSHP && ShowAnimPAL)
			{
				int frameCurrent = (Unsorted::CurrentFrame / pReadyShapeType->ShowAnim_FrameKeep) % ShowAnimSHP->Frames;

				Point2D posAnim;
				posAnim = {
					Location.X + 30 - (ShowAnimSHP->Width >> 1),
					Location.Y
				};
				posAnim += pReadyShapeType->ShowAnim_Offset.Get();

				RectangleStruct vRect = { 0, 0, 0, 0 };
				DSurface::Sidebar->GetRect(&vRect);

				auto const nFlag = BlitterFlags::None | EnumFunctions::GetTranslucentLevel(pReadyShapeType->ShowAnim_TranslucentLevel.Get());

				DSurface::Sidebar->DrawSHP(ShowAnimPAL, ShowAnimSHP, frameCurrent, &posAnim, &vRect, nFlag,
					0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);

				MouseClass::Instance->RepaintSidebar(1);
			}
		}
	}

	return 0;
}

// Fantasy Adventure - Click black-out cameo to focus on the hero
DEFINE_HOOK(0x6AB64F, SidebarClass_ClickedAction, 0x6)
{
	GET(TechnoTypeClass*, pItem, EAX);

	HouseClass* pHouse = HouseClass::CurrentPlayer;

	if (pItem)
	{
		if (auto pTypeExt = TechnoTypeExt::ExtMap.Find(pItem))
		{
			if (pTypeExt->ClickCameoToFocus.Get(RulesExt::Global()->ClickCameoToFocus.Get()))
			{
				CanBuildResult canBuild = pHouse->CanBuild(pItem, true, false);
				if (canBuild == CanBuildResult::TemporarilyUnbuildable)
				{
					for (auto pTechno : *TechnoClass::Array)
					{
						if (pTechno->Owner == pHouse && pTechno->GetTechnoType() == pItem && pTechno->IsOnMap)
						{
							CoordStruct coords = pTechno->GetCoords();
							TacticalClass::Instance->SetTacticalPosition(&coords);
							pTechno->Flash(60);
							if (pItem->VoiceSelect.Items)
								pTechno->QueueVoice(pItem->VoiceSelect[0]);
							MapClass::Instance->MarkNeedsRedraw(1);
							break;
						}
					}
				}
			}
		}
	}

	return 0;
}
