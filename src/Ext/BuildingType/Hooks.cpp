#include "Body.h"

#include <TacticalClass.h>
#include <Ext/Rules/Body.h>

#include <Utilities/Macro.h>
#include <Utilities/EnumFunctions.h>

DEFINE_HOOK(0x460285, BuildingTypeClass_LoadFromINI_Muzzle, 0x6)
{
	enum { Skip = 0x460388, Read = 0x460299 };

	GET(BuildingTypeClass*, pThis, EBP);

	// Restore overriden instructions
	R->Stack(STACK_OFFSET(0x368, -0x358), 0);
	R->EDX(0);

	// Disable Vanilla Muzzle flash when MaxNumberOccupants is 0 or more than 10
	return !pThis->MaxNumberOccupants || pThis->MaxNumberOccupants > 10
		? Skip : Read;
}

DEFINE_HOOK(0x44043D, BuildingClass_AI_Temporaled_Chronosparkle_MuzzleFix, 0x8)
{
	GET(BuildingClass*, pThis, ESI);

	auto pType = pThis->Type;
	if (pType->MaxNumberOccupants > 10)
	{
		GET(int, nFiringIndex, EBX);
		auto pTypeExt = BuildingTypeExt::ExtMap.Find(pType);
		R->EAX(&pTypeExt->OccupierMuzzleFlashes[nFiringIndex]);
	}

	return 0;
}

DEFINE_HOOK(0x45387A, BuildingClass_FireOffset_Replace_MuzzleFix, 0xA)
{
	GET(BuildingClass*, pThis, ESI);

	auto pType = pThis->Type;
	if (pType->MaxNumberOccupants > 10)
	{
		auto pTypeExt = BuildingTypeExt::ExtMap.Find(pType);
		R->EDX(&pTypeExt->OccupierMuzzleFlashes[pThis->FiringOccupantIndex]);
	}

	return 0;
}

DEFINE_HOOK(0x458623, BuildingClass_KillOccupiers_Replace_MuzzleFix, 0x7)
{
	GET(BuildingClass*, pThis, ESI);

	auto pType = pThis->Type;
	if (pType->MaxNumberOccupants > 10)
	{
		GET(int, nFiringIndex, EDI);
		auto pTypeExt = BuildingTypeExt::ExtMap.Find(pType);
		R->ECX(&pTypeExt->OccupierMuzzleFlashes[nFiringIndex]);
	}

	return 0;
}

DEFINE_HOOK(0x6D528A, TacticalClass_DrawPlacement_PlacementPreview, 0x6)
{
	auto pRules = RulesExt::Global();

	if (!pRules->PlacementPreview || !Phobos::Config::ShowPlacementPreview)
		return 0;

	auto pBuilding = specific_cast<BuildingClass*>(DisplayClass::Instance->CurrentBuilding);
	auto pType = pBuilding ? pBuilding->Type : nullptr;
	auto pTypeExt = pType ? BuildingTypeExt::ExtMap.Find(pType) : nullptr;
	bool isShow = pTypeExt && pTypeExt->PlacementPreview;

	if (isShow)
	{
		CellClass* pCell = nullptr;
		{
			CellStruct nDisplayCell = Make_Global<CellStruct>(0x88095C);
			CellStruct nDisplayCell_Offset = Make_Global<CellStruct>(0x880960);

			pCell = MapClass::Instance->TryGetCellAt(nDisplayCell + nDisplayCell_Offset);
			if (!pCell)
				return 0;
		}

		int imageFrame = 0;
		SHPStruct* pImage = pTypeExt->PlacementPreview_Shape.GetSHP();
		{
			if (!pImage)
			{
				pImage = pType->LoadBuildup();

				if (pImage != nullptr)
					imageFrame = ((pImage->Frames / 2) - 1);
				else
					pImage = pType->GetImage();

				if (!pImage)
					return 0;
			}

			imageFrame = Math::clamp(pTypeExt->PlacementPreview_ShapeFrame.Get(imageFrame), 0, (int)pImage->Frames);
		}

		Point2D position = { 0, 0 };
		{
			CoordStruct offset = pTypeExt->PlacementPreview_Offset;
			int nHeight = offset.Z + pCell->GetFloorHeight({ 0, 0 });
			TacticalClass::Instance->CoordsToClient
			(
				CellClass::Cell2Coord(pCell->MapCoords, nHeight),
				&position
			);
			position.X += offset.X;
			position.Y += offset.Y;
		}

		BlitterFlags blitFlags = pTypeExt->PlacementPreview_Translucency.Get(pRules->PlacementPreview_Translucency) |
			BlitterFlags::Centered | BlitterFlags::Nonzero | BlitterFlags::MultiPass;

		ConvertClass* pPalette = nullptr;
		{
			if (pTypeExt->PlacementPreview_Remap.Get())
				pPalette = pBuilding->GetDrawer();
			else
				pPalette = pTypeExt->PlacementPreview_Palette.GetOrDefaultConvert(FileSystem::UNITx_PAL());
		}

		DSurface* pSurface = DSurface::Temp;
		RectangleStruct rect = pSurface->GetRect();
		rect.Height -= 32; // account for bottom bar

		pSurface->DrawSHP(pPalette, pImage, imageFrame, position, rect, blitFlags);
	}

	return 0;
}

DEFINE_HOOK(0x47EFAE, CellClass_Draw_It_MakePlacementGridTranparent, 0x6)
{
	LEA_STACK(BlitterFlags*, blitFlags, STACK_OFFSET(0x68, -0x58));

	*blitFlags |= RulesExt::Global()->PlacementGrid_Translucency;
	return 0;
}

DEFINE_HOOK(0x6FE3F1, TechnoClass_Fireat_OccupyDamageBonus, 0xB)
{
	enum
	{
		ApplyDamageBonus = 0x6FE405,
		Nothing = 0x0
	};

	GET(TechnoClass* const, pThis, ESI);

	if (auto const Building = specific_cast<BuildingClass*>(pThis))
	{
		if (auto const TypeExt = BuildingTypeExt::ExtMap.Find(Building->Type))
		{
			GET_STACK(int, nDamage, 0x2C);
			R->EAX(Game::F2I(nDamage * TypeExt->BuildingOccupyDamageMult.Get(RulesClass::Instance->OccupyDamageMultiplier)));
			return ApplyDamageBonus;
		}
	}

	return Nothing;
}

DEFINE_HOOK(0x6FE421, TechnoClass_Fireat_BunkerDamageBonus, 0xB)
{
	enum
	{
		ApplyDamageBonus = 0x6FE435,
		Nothing = 0x0
	};

	GET(TechnoClass* const, pThis, ESI);

	if (auto const Building = specific_cast<BuildingClass*>(pThis->BunkerLinkedItem))
	{
		if (auto const TypeExt = BuildingTypeExt::ExtMap.Find(Building->Type))
		{
			GET_STACK(int, nDamage, 0x2C);
			R->EAX(Game::F2I(nDamage * TypeExt->BuildingBunkerDamageMult.Get(RulesClass::Instance->OccupyDamageMultiplier)));
			return ApplyDamageBonus;
		}
	}

	return Nothing;
}

DEFINE_HOOK(0x6FD183, TechnoClass_RearmDelay_BuildingOccupyROFMult, 0xC)
{
	enum
	{
		ApplyRofMod = 0x6FD1AB,
		SkipRofMod = 0x6FD1B1,
		Nothing = 0x0
	};

	GET(TechnoClass*, pThis, ESI);

	if (auto const Building = specific_cast<BuildingClass*>(pThis))
	{
		if (auto const TypeExt = BuildingTypeExt::ExtMap.Find(Building->Type))
		{
			auto const nMult = TypeExt->BuildingOccupyROFMult.Get(RulesClass::Instance->OccupyROFMultiplier);
			if (nMult > 0.0f)
			{
				GET_STACK(int, nROF, STACK_OFFSET(0x10, 0x4));
				R->EAX(Game::F2I(((double)nROF) / nMult));
				return ApplyRofMod;
			}
			return SkipRofMod;
		}
	}

	return Nothing;
}

DEFINE_HOOK(0x6FD1C7, TechnoClass_RearmDelay_BuildingBunkerROFMult, 0xC)
{
	enum
	{
		ApplyRofMod = 0x6FD1EF,
		SkipRofMod = 0x6FD1F1,
		Nothing = 0x0
	};

	GET(TechnoClass*, pThis, ESI);

	if (auto const Building = specific_cast<BuildingClass*>(pThis->BunkerLinkedItem))
	{
		if (auto const TypeExt = BuildingTypeExt::ExtMap.Find(Building->Type))
		{
			auto const nMult = TypeExt->BuildingBunkerROFMult.Get(RulesClass::Instance->BunkerROFMultiplier);
			if (nMult > 0.0f)
			{
				GET_STACK(int, nROF, STACK_OFFSET(0x10, 0x4));
				R->EAX(Game::F2I(((double)nROF) / nMult));
				return ApplyRofMod;
			}
			return SkipRofMod;
		}
	}

	return Nothing;
}

DEFINE_HOOK_AGAIN(0x45933D, BuildingClass_BunkerSound, 0x5)
DEFINE_HOOK_AGAIN(0x4595D9, BuildingClass_BunkerSound, 0x5)
DEFINE_HOOK(0x459494, BuildingClass_BunkerSound, 0x5)
{
	enum
	{
		BunkerWallUpSound = 0x45933D,
		BunkerWallUpSound_Handled_ret = 0x459374,

		BunkerWallDownSound_01 = 0x4595D9,
		BunkerWallDownSound_01_Handled_ret = 0x459612,

		BunkerWallDownSound_02 = 0x459494,
		BunkerWallDownSound_02_Handled_ret = 0x4594CD

	};

	BuildingClass const* pThis = R->Origin() == BunkerWallDownSound_01 ?
		R->EDI<BuildingClass*>() : R->ESI<BuildingClass*>();

	BuildingTypeExt::PlayBunkerSound(pThis, R->Origin() == BunkerWallUpSound);

	switch (R->Origin())
	{
	case BunkerWallUpSound:
		return BunkerWallUpSound_Handled_ret;
	case BunkerWallDownSound_01:
		return BunkerWallDownSound_01_Handled_ret;
	case BunkerWallDownSound_02:
		return BunkerWallDownSound_02_Handled_ret;
	}
	return 0;
}

DEFINE_HOOK_AGAIN(0x4426DB, BuildingClass_ReceiveDamage_DisableDamageSound, 0x8)
DEFINE_HOOK_AGAIN(0x702777, BuildingClass_ReceiveDamage_DisableDamageSound, 0x8)
DEFINE_HOOK(0x70272E, BuildingClass_ReceiveDamage_DisableDamageSound, 0x8)
{
	enum
	{
		BuildingClass_TakeDamage_DamageSound = 0x4426DB,
		BuildingClass_TakeDamage_DamageSound_Handled_ret = 0x44270B,

		TechnoClass_TakeDamage_Building_DamageSound_01 = 0x702777,
		TechnoClass_TakeDamage_Building_DamageSound_01_Handled_ret = 0x7027AE,

		TechnoClass_TakeDamage_Building_DamageSound_02 = 0x70272E,
		TechnoClass_TakeDamage_Building_DamageSound_02_Handled_ret = 0x702765,

		Nothing = 0x0
	};

	GET(TechnoClass*, pThis, ESI);

	if (auto const pBuilding = specific_cast<BuildingClass*>(pThis))
	{
		auto const pExt = BuildingTypeExt::ExtMap.Find(pBuilding->Type);
		if (pExt && pExt->DisableDamageSound.Get())
			if (pExt && pExt->DisableDamageSound.Get())
			{
				switch (R->Origin())
				{
				case BuildingClass_TakeDamage_DamageSound:
					return BuildingClass_TakeDamage_DamageSound_Handled_ret;
				case TechnoClass_TakeDamage_Building_DamageSound_01:
					return TechnoClass_TakeDamage_Building_DamageSound_01_Handled_ret;
				case TechnoClass_TakeDamage_Building_DamageSound_02:
					return TechnoClass_TakeDamage_Building_DamageSound_02_Handled_ret;
				}
			}
	}
	return Nothing;
}

DEFINE_HOOK(0x44E85F, BuildingClass_Power_DegradeWithHealth, 0x7)
{
	enum
	{
		Handled = 0x44E86F
	};

	GET(BuildingClass*, pThis, ESI);
	GET_STACK(int, nPowMult, STACK_OFFSET(0xC, -0x4));

	auto const pTypeExt = BuildingTypeExt::ExtMap.Find(pThis->Type);

	R->EAX(Game::F2I(pTypeExt && !pTypeExt->Power_DegradeWithHealth.Get()
		? (nPowMult) : (nPowMult * pThis->GetHealthPercentage())));

	return Handled;
}

DEFINE_HOOK(0x6F34B7, TechnoClass_WhatWeaponShouldIUse_AllowAirstrike, 0x6)
{
	enum { SkipGameCode = 0x6F34BD };

	GET(BuildingTypeClass*, pThis, ECX);

	const auto pExt = BuildingTypeExt::ExtMap.Find(pThis);
	R->EAX(pExt->AllowAirstrike.Get(pThis->CanC4));

	return SkipGameCode;
}

DEFINE_HOOK(0x51EAF2, TechnoClass_WhatAction_AllowAirstrike, 0x6)
{
	enum { SkipGameCode = 0x51EAF8 };

	GET(BuildingTypeClass*, pThis, ESI);

	const auto pExt = BuildingTypeExt::ExtMap.Find(pThis);
	R->EAX(pExt->AllowAirstrike.Get(pThis->CanC4));

	return SkipGameCode;
}

