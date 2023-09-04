#include "Body.h"

#include <Utilities/Macro.h>
#include <Utilities/EnumFunctions.h>

#include <Ext/AnimType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/Techno/Body.h>

// Negi烈葱
// https://t.bilibili.com/651034625966080000
DEFINE_HOOK(0x639DD8, TechnoClass_WayPoint, 0x5)
{
	enum { CanUse = 0x639DDD, CannotUse = 0x639E03 };

	GET(TechnoClass*, pThis, ESI);

	if (pThis->WhatAmI() == AbstractType::Infantry || pThis->WhatAmI() == AbstractType::Unit)
		return CanUse;

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	auto pRulesExt = RulesExt::Global();
	bool GlobalAllow = (pRulesExt->AllowPlanningMode_Aircraft && pThis->WhatAmI() == AbstractType::Aircraft)
		|| (pRulesExt->AllowPlanningMode_Building && pThis->WhatAmI() == AbstractType::Building);

	if (pTypeExt->AllowPlanningMode.Get(GlobalAllow))
		return CanUse;
	else
		return CannotUse;
}

DEFINE_HOOK(0x6F64A9, TechnoClass_DrawHealthBar_Hide, 0x5)
{
	GET(TechnoClass*, pThis, ECX);
	auto pTypeData = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	if (pTypeData && pTypeData->HealthBar_Hide)
		return 0x6F6AB6;
	return 0;
}

DEFINE_HOOK(0x6F3C56, TechnoClass_Transform_6F3AD0_TurretMultiOffset, 0x0)
{
	LEA_STACK(Matrix3D*, mtx, STACK_OFFSET(0xD8, -0x90));
	GET(TechnoTypeClass*, technoType, EDX);

	TechnoTypeExt::ApplyTurretOffset(technoType, mtx);

	return 0x6F3C6D;
}

DEFINE_HOOK(0x6F3E6E, FootClass_firecoord_6F3D60_TurretMultiOffset, 0x0)
{
	LEA_STACK(Matrix3D*, mtx, STACK_OFFSET(0xCC, -0x90));
	GET(TechnoTypeClass*, technoType, EBP);

	TechnoTypeExt::ApplyTurretOffset(technoType, mtx);

	return 0x6F3E85;
}

DEFINE_HOOK(0x73B780, UnitClass_DrawVXL_TurretMultiOffset, 0x0)
{
	GET(TechnoTypeClass*, technoType, EAX);

	auto const pTypeData = TechnoTypeExt::ExtMap.Find(technoType);

	if (pTypeData && *pTypeData->TurretOffset.GetEx() == CoordStruct { 0, 0, 0 })
		return 0x73B78A;

	return 0x73B790;
}

DEFINE_HOOK(0x73BA4C, UnitClass_DrawVXL_TurretMultiOffset1, 0x0)
{
	LEA_STACK(Matrix3D*, mtx, STACK_OFFSET(0x1D0, -0x13C));
	GET(TechnoTypeClass*, technoType, EBX);

	double& factor = *reinterpret_cast<double*>(0xB1D008);

	TechnoTypeExt::ApplyTurretOffset(technoType, mtx, factor);

	return 0x73BA68;
}

DEFINE_HOOK(0x73C890, UnitClass_Draw_1_TurretMultiOffset, 0x0)
{
	LEA_STACK(Matrix3D*, mtx, 0x80);
	GET(TechnoTypeClass*, technoType, EAX);

	TechnoTypeExt::ApplyTurretOffset(technoType, mtx, 1.0 / 8);

	return 0x73C8B7;
}

DEFINE_HOOK(0x43E0C4, BuildingClass_Draw_43DA80_TurretMultiOffset, 0x0)
{
	LEA_STACK(Matrix3D*, mtx, 0x60);
	GET(TechnoTypeClass*, technoType, EDX);

	TechnoTypeExt::ApplyTurretOffset(technoType, mtx, 1.0 / 8);

	return 0x43E0E8;
}

DEFINE_HOOK(0x73D223, UnitClass_DrawIt_OreGath, 0x6)
{
	GET(UnitClass*, pThis, ESI);
	GET(int, nFacing, EDI);
	GET_STACK(RectangleStruct*, pBounds, STACK_OFFSET(0x50, 0x8));
	LEA_STACK(Point2D*, pLocation, STACK_OFFSET(0x50, -0x18));
	GET_STACK(int, nBrightness, STACK_OFFSET(0x50, 0x4));

	auto const pType = pThis->GetTechnoType();
	auto const pData = TechnoTypeExt::ExtMap.Find(pType);

	ConvertClass* pDrawer = FileSystem::ANIM_PAL;
	SHPStruct* pSHP = FileSystem::OREGATH_SHP;
	int idxFrame;

	auto idxTiberium = pThis->GetCell()->GetContainedTiberiumIndex();
	auto idxArray = pData->OreGathering_Tiberiums.size() > 0 ? pData->OreGathering_Tiberiums.IndexOf(idxTiberium) : 0;
	if (idxTiberium != -1 && idxArray != -1)
	{
		auto const pAnimType = pData->OreGathering_Anims.size() > 0 ? pData->OreGathering_Anims[idxArray] : nullptr;
		auto const nFramesPerFacing = pData->OreGathering_FramesPerDir.size() > 0 ? pData->OreGathering_FramesPerDir[idxArray] : 15;
		auto const pAnimExt = AnimTypeExt::ExtMap.Find(pAnimType);
		if (pAnimType)
		{
			pSHP = pAnimType->GetImage();
			if (auto const pPalette = pAnimExt->Palette.GetConvert())
				pDrawer = pPalette;
		}
		idxFrame = nFramesPerFacing * nFacing + (Unsorted::CurrentFrame + pThis->WalkedFramesSoFar) % nFramesPerFacing;
	}
	else
	{
		idxFrame = 15 * nFacing + (Unsorted::CurrentFrame + pThis->WalkedFramesSoFar) % 15;
	}

	DSurface::Temp->DrawSHP(
		pDrawer, pSHP, idxFrame, pLocation, pBounds,
		BlitterFlags::Flat | BlitterFlags::Alpha | BlitterFlags::Centered,
		0, pThis->GetZAdjustment() - 2, ZGradient::Ground, nBrightness,
		0, nullptr, 0, 0, 0
	);

	R->EBP(nBrightness);
	R->EBX(pBounds);

	return 0x73D28C;
}

DEFINE_HOOK(0x700C58, TechnoClass_CanPlayerMove_NoManualMove, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	// auto const& pExt = TechnoExt::ExtMap.Find(pThis);
	auto const& pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	bool noMove = pTypeExt && pTypeExt->NoManualMove;

	return noMove ? 0x700C62 : 0;
}

DEFINE_HOOK(0x73CF46, UnitClass_Draw_It_KeepUnitVisible, 0x6)
{
	GET(UnitClass*, pThis, ESI);

	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (pTypeExt->DeployingAnim_KeepUnitVisible && (pThis->Deploying || pThis->Undeploying))
		return 0x73CF62;

	return 0;
}

// Ares hooks in at 739B8A, this goes before it and skips it if needed.
DEFINE_HOOK(0x739B7C, UnitClass_Deploy_DeployDir, 0x6)
{
	enum { SkipAnim = 0x739C70, PlayAnim = 0x739B9E };

	GET(UnitClass*, pThis, ESI);

	if (!pThis->InAir)
	{
		if (pThis->Type->DeployingAnim)
		{
			if (TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType())->DeployingAnim_AllowAnyDirection)
				return PlayAnim;

			return 0;
		}

		pThis->Deployed = true;
	}

	return SkipAnim;
}

DEFINE_HOOK_AGAIN(0x739D8B, UnitClass_DeployUndeploy_DeployAnim, 0x5)
DEFINE_HOOK(0x739BA8, UnitClass_DeployUndeploy_DeployAnim, 0x5)
{
	enum { Deploy = 0x739C20, DeployUseUnitDrawer = 0x739C0A, Undeploy = 0x739E04, UndeployUseUnitDrawer = 0x739DEE };

	GET(UnitClass*, pThis, ESI);

	bool isDeploying = R->Origin() == 0x739BA8;

	if (auto const pExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType()))
	{
		if (auto const pAnim = GameCreate<AnimClass>(pThis->Type->DeployingAnim,
			pThis->Location, 0, 1, 0x600, 0,
			!isDeploying ? pExt->DeployingAnim_ReverseForUndeploy : false))
		{
			pThis->DeployAnim = pAnim;
			pAnim->SetOwnerObject(pThis);

			if (pExt->DeployingAnim_UseUnitDrawer)
				return isDeploying ? DeployUseUnitDrawer : UndeployUseUnitDrawer;
		}
		else
		{
			pThis->DeployAnim = nullptr;
		}
	}

	return isDeploying ? Deploy : Undeploy;
}

DEFINE_HOOK_AGAIN(0x739E81, UnitClass_DeployUndeploy_DeploySound, 0x6)
DEFINE_HOOK(0x739C86, UnitClass_DeployUndeploy_DeploySound, 0x6)
{
	enum { DeployReturn = 0x739CBF, UndeployReturn = 0x739EB8 };

	GET(UnitClass*, pThis, ESI);

	bool isDeploying = R->Origin() == 0x739C86;
	bool isDoneWithDeployUndeploy = isDeploying ? pThis->Deployed : !pThis->Deployed;

	if (isDoneWithDeployUndeploy)
		return 0; // Only play sound when done with deploying or undeploying.

	return isDeploying ? DeployReturn : UndeployReturn;
}

// Issue #503
// Author : Otamaa
DEFINE_HOOK(0x4AE670, DisplayClass_GetToolTip_EnemyUIName, 0x8)
{
	enum { SetUIName = 0x4AE678 };

	GET(ObjectClass*, pObject, ECX);

	auto pDecidedUIName = pObject->GetUIName();
	auto pFoot = generic_cast<FootClass*>(pObject);
	auto pTechnoType = pObject->GetTechnoType();

	if (pFoot && pTechnoType)
	{
		if (!pObject->IsDisguised())
		{
			bool IsAlly = true;
			bool IsCivilian = false;
			bool IsObserver = HouseClass::Observer || HouseClass::IsCurrentPlayerObserver();

			if (auto pOwnerHouse = pFoot->GetOwningHouse())
			{
				IsAlly = pOwnerHouse->IsAlliedWith(HouseClass::CurrentPlayer);
				IsCivilian = (pOwnerHouse == HouseClass::FindCivilianSide()) || pOwnerHouse->IsNeutral();
			}

			if (!IsAlly && !IsCivilian && !IsObserver)
			{
				auto pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pTechnoType);

				if (auto pEnemyUIName = pTechnoTypeExt->EnemyUIName.Get().Text)
					pDecidedUIName = pEnemyUIName;
			}
		}
		else if (pFoot->Disguise && static_cast<TechnoTypeClass*>(pFoot->Disguise))
		{
			const auto disguiseUIName = pFoot->Disguise->UIName;
			auto pOwnerHouse = pFoot->GetOwningHouse();

			if (pOwnerHouse && pOwnerHouse != HouseClass::CurrentPlayer)
			{
				bool IsAlly = pOwnerHouse->IsAlliedWith(HouseClass::CurrentPlayer);

				if (!IsAlly)
				{
					if (auto pDisguiseHouse = pFoot->DisguisedAsHouse)
					{
						if (pDisguiseHouse == HouseClass::CurrentPlayer ||
							(pDisguiseHouse->IsAlliedWith(HouseClass::CurrentPlayer) && RulesExt::Global()->ShowAllyDisguiseBlinking))
						{
							pDecidedUIName = disguiseUIName;
						}
						else if (auto pDisguiseTypeExt = TechnoTypeExt::ExtMap.Find(static_cast<TechnoTypeClass*>(pFoot->Disguise)))
						{
							if (auto pEnemyUIName = pDisguiseTypeExt->EnemyUIName.Get().Text)
								pDecidedUIName = pEnemyUIName;
						}
					}
					else
						pDecidedUIName = disguiseUIName;
				}
				else if (!RulesExt::Global()->ShowAllyDisguiseBlinking)
					pDecidedUIName = disguiseUIName;
			}
		}
	}

	R->EAX(pDecidedUIName);
	return SetUIName;
}

DEFINE_HOOK(0x702672, TechnoClass_ReceiveDamage_RevengeWeapon, 0x5)
{
	GET(TechnoClass*, pThis, ESI);
	GET_STACK(TechnoClass*, pSource, STACK_OFFSET(0xC4, 0x10));

	if (pSource)
	{
		auto const pExt = TechnoExt::ExtMap.Find(pThis);
		auto const pSourceExt = TechnoExt::ExtMap.Find(pSource);
		auto const pTypeExt = pExt->TypeExtData;

		if (pTypeExt && pTypeExt->RevengeWeapon.isset() &&
			EnumFunctions::CanTargetHouse(pTypeExt->RevengeWeapon_AffectsHouses, pThis->Owner, pSource->Owner))
		{
			WeaponTypeExt::DetonateAt(pTypeExt->RevengeWeapon.Get(), pSource, pThis);
		}

		for (const auto& pAE : pExt->AttachEffects)
		{
			bool active =
				pAE->Timer.InProgress()
				&& pAE->Delay_Timer.Expired()
				&& !pAE->Inlimbo
				&& !pAE->InLoopDelay
				&& !pAE->Timer.Completed();

			if (!active)
				continue;

			if (!pAE->Type->RevengeWeapon.isset())
				continue;

			if (EnumFunctions::CanTargetHouse(pAE->Type->RevengeWeapon_AffectsHouses, pThis->Owner, pSource->Owner))
				WeaponTypeExt::DetonateAt(pAE->Type->RevengeWeapon, pSource, pThis);
		}

		for (const auto& pAE : pSourceExt->AttachEffects)
		{
			bool active =
				pAE->Timer.InProgress()
				&& pAE->Delay_Timer.Expired()
				&& !pAE->Inlimbo
				&& !pAE->InLoopDelay
				&& !pAE->Timer.Completed();

			if (!active)
				continue;

			if (!pAE->Type->RevengeWeaponAttach.isset())
				continue;

			if (EnumFunctions::CanTargetHouse(pAE->Type->RevengeWeaponAttach_AffectsHouses, pThis->Owner, pSource->Owner))
				WeaponTypeExt::DetonateAt(pAE->Type->RevengeWeaponAttach, pSource, pThis);
		}
	}

	return 0;
}

// Patches TechnoClass::Kill_Cargo/KillPassengers (push ESI -> push EBP)
// Fixes recursive passenger kills not being accredited
// to proper techno but to their transports
DEFINE_PATCH(0x707CF2, 0x55);
