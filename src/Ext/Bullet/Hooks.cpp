#include "Body.h"
#include <Ext/WeaponType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Misc/CaptureManager.h>

#include <AnimClass.h>
#include <TechnoClass.h>
#include <ScenarioClass.h>
#include <TacticalClass.h>

// has everything inited except SpawnNextAnim at this point
DEFINE_HOOK(0x466556, BulletClass_Init_SetLaserTrail, 0x6)
{
	GET(BulletClass*, pThis, ECX);

	if (!pThis->Type->Inviso)
		BulletExt::InitializeLaserTrails(pThis);

	return 0;
}

DEFINE_HOOK(0x4666F7, BulletClass_AI, 0x6)
{
	GET(BulletClass*, pThis, EBP);
	auto pBulletExt = BulletExt::ExtMap.Find(pThis);

	if (!pBulletExt)
		return 0;

	if (pBulletExt->ShouldIntercept)
	{
		pThis->Detonate(pThis->GetCoords());
		pThis->Limbo();
		pThis->UnInit();

		const auto pTechno = pThis->Owner;
		const bool isLimbo =
			pTechno &&
			pTechno->InLimbo &&
			pThis->WeaponType &&
			pThis->WeaponType->LimboLaunch;

		if (isLimbo)
		{
			pThis->SetTarget(nullptr);
			auto damage = pTechno->Health * 2;
			pTechno->SetLocation(pThis->GetCoords());
			pTechno->ReceiveDamage(&damage, 0, RulesClass::Instance->C4Warhead, nullptr, true, false, nullptr);
		}
	}

	if (pBulletExt->Intercepted)
		pBulletExt->ShouldIntercept = true;

	// LaserTrails update routine is in BulletClass::AI hook because BulletClass::Draw
	// doesn't run when the object is off-screen which leads to visual bugs - Kerbiter
	if (pBulletExt && pBulletExt->LaserTrails.size())
	{
		CoordStruct location = pThis->GetCoords();
		const BulletVelocity& velocity = pThis->Velocity;

		// We adjust LaserTrails to account for vanilla bug of drawing stuff one frame ahead.
		// Pretty meh solution but works until we fix the bug - Kerbiter
		CoordStruct drawnCoords
		{
			(int)(location.X + velocity.X),
			(int)(location.Y + velocity.Y),
			(int)(location.Z + velocity.Z)
		};

		for (auto const& trail : pBulletExt->LaserTrails)
		{
			// We insert initial position so the first frame of trail doesn't get skipped - Kerbiter
			// TODO move hack to BulletClass creation
			if (!trail->LastLocation.isset())
				trail->LastLocation = location;

			trail->Update(drawnCoords);
		}

	}

	return 0;
}

DEFINE_HOOK(0x4692BD, BulletClass_Logics_ApplyMindControl, 0x6)
{
	GET(BulletClass*, pThis, ESI);

	auto pTypeExt = WarheadTypeExt::ExtMap.Find(pThis->WH);
	auto pControlledAnimType = pTypeExt->MindControl_Anim.Get(RulesClass::Instance->ControlledAnimationType);
	auto pTechno = generic_cast<TechnoClass*>(pThis->Target);

	R->AL(CaptureManager::CaptureUnit(pThis->Owner->CaptureManager, pTechno, pControlledAnimType));

	return 0x4692D5;
}

DEFINE_HOOK(0x4671B9, BulletClass_AI_ApplyGravity, 0x6)
{
	GET(BulletTypeClass* const, pType, EAX);

	auto const nGravity = BulletTypeExt::GetAdjustedGravity(pType);
	__asm { fld nGravity };

	return 0x4671BF;
}

DEFINE_HOOK(0x6F7481, TechnoClass_Targeting_ApplyGravity, 0x6)
{
	GET(WeaponTypeClass* const, pWeaponType, EDX);

	auto const nGravity = BulletTypeExt::GetAdjustedGravity(pWeaponType->Projectile);
	__asm { fld nGravity };

	return 0x6F74A4;
}

DEFINE_HOOK(0x6FDAA6, TechnoClass_FireAngle_6FDA00_ApplyGravity, 0x5)
{
	GET(WeaponTypeClass* const, pWeaponType, EDI);

	auto const nGravity = BulletTypeExt::GetAdjustedGravity(pWeaponType->Projectile);
	__asm { fld nGravity };

	return 0x6FDACE;
}

DEFINE_HOOK(0x469211, BulletClass_Logics_MindControlAlternative1, 0x6)
{
	GET(BulletClass*, pBullet, ESI);

	if (!pBullet->Target)
		return 0;

	auto pBulletWH = pBullet->WH;
	auto pTarget = generic_cast<TechnoClass*>(pBullet->Target);

	if (pTarget
		&& pBullet->Owner
		&& pBulletWH
		&& pBulletWH->MindControl)
	{
		if (auto pTargetType = pTarget->GetTechnoType())
		{
			if (auto const pWarheadExt = WarheadTypeExt::ExtMap.Find(pBulletWH))
			{
				double currentHealthPerc = pTarget->GetHealthPercentage() * 100;

				if (pWarheadExt->MindContol_Threshhold < 0 || pWarheadExt->MindContol_Threshhold > 100)
					pWarheadExt->MindContol_Threshhold = 100;

				if (pWarheadExt->MindContol_Threshhold < 100
					&& pWarheadExt->MindContol_Damage.isset()
					&& pWarheadExt->MindContol_Warhead.isset()
					&& currentHealthPerc >= pWarheadExt->MindContol_Threshhold)
				{
					return 0x469343;
				}
			}
		}
	}

	return 0;
}

DEFINE_HOOK(0x469BD6, BulletClass_Logics_MindControlAlternative2, 0x6)
{
	GET(BulletClass*, pBullet, ESI);
	GET(AnimTypeClass*, pAnimType, EBX);

	if (!pBullet->Target)
		return 0;

	auto pBulletWH = pBullet->WH;
	auto pTarget = generic_cast<TechnoClass*>(pBullet->Target);

	if (pTarget
		&& pBullet->Owner
		&& pBulletWH
		&& pBulletWH->MindControl)
	{
		if (auto pTargetType = pTarget->GetTechnoType())
		{
			auto const pWarheadExt = WarheadTypeExt::ExtMap.Find(pBulletWH);
			double currentHealthPerc = pTarget->GetHealthPercentage() * 100;

			if (pWarheadExt && pWarheadExt->MindContol_Threshhold < 100
				&& pWarheadExt->MindContol_Damage.isset()
				&& pWarheadExt->MindContol_Warhead.isset()
				&& currentHealthPerc >= pWarheadExt->MindContol_Threshhold)
			{
				int damage = pWarheadExt->MindContol_Damage;
				WarheadTypeClass* pAltWarhead = pWarheadExt->MindContol_Warhead;
				auto pAttacker = pBullet->Owner;
				auto pAttackingHouse = pBullet->Owner->Owner;
				int realDamage = MapClass::GetTotalDamage(damage, pAltWarhead, pTargetType->Armor, 0);

				if (pTarget->Health <= realDamage && !pWarheadExt->MindContol_CanKill)
				{
					pTarget->Health = 100000000;
					realDamage = 1;
					pTarget->ReceiveDamage(&realDamage, 0, pAltWarhead, pAttacker, true, false, pAttackingHouse);
					pTarget->Health = 1;
				}
				else
				{
					pTarget->ReceiveDamage(&damage, 0, pAltWarhead, pAttacker, true, false, pAttackingHouse);
				}

				pAnimType = nullptr;

				// If the alternative Warhead have AnimList tag declared then use it
				if (pWarheadExt->MindContol_Warhead->AnimList.Count > 0)
				{
					int animListCount = pWarheadExt->MindContol_Warhead->AnimList.Count;
					auto const pAltWarheadExt = WarheadTypeExt::ExtMap.Find(pBulletWH);

					if (CellClass* pCell = MapClass::Instance->TryGetCellAt(pTarget->Location))
					{
						pAnimType = MapClass::SelectDamageAnimation(damage, pAltWarhead, pCell->LandType, pTarget->Location);
					}
				}

				R->EBX(pAnimType);
			}
		}
	}

	return 0;
}

DEFINE_HOOK(0x6FECB2, TechnoClass_FireAt_ApplyGravity, 0x6)
{
	GET(BulletTypeClass* const, pType, EAX);

	auto const nGravity = BulletTypeExt::GetAdjustedGravity(pType);
	__asm { fld nGravity };

	return 0x6FECD1;
}

DEFINE_HOOK(0x772A0A, WeaponTypeClass_SetSpeed_ApplyGravity, 0x6)
{
	GET(BulletTypeClass* const, pType, EAX);

	auto const nGravity = BulletTypeExt::GetAdjustedGravity(pType);
	__asm { fld nGravity };

	return 0x772A29;
}

DEFINE_HOOK(0x773087, WeaponTypeClass_GetSpeed_ApplyGravity, 0x6)
{
	GET(BulletTypeClass* const, pType, EAX);

	auto const nGravity = BulletTypeExt::GetAdjustedGravity(pType);
	__asm { fld nGravity };

	return 0x7730A3;
}

DEFINE_HOOK(0x46A3D6, BulletClass_Shrapnel_Forced, 0xA)
{
	enum { Shrapnel = 0x46A40C, Skip = 0x46ADCD };

	GET(BulletClass*, pBullet, EDI);

	auto const pData = BulletTypeExt::ExtMap.Find(pBullet->Type);

	if (auto const pObject = pBullet->GetCell()->FirstObject)
	{
		if (pObject->WhatAmI() != AbstractType::Building || pData->Shrapnel_AffectsBuildings)
			return Shrapnel;
	}
	else if (pData->Shrapnel_AffectsGround)
		return Shrapnel;

	return Skip;
}

DEFINE_HOOK(0x4690D4, BulletClass_Logics_ScreenShake, 0x6)
{
	enum { SkipShaking = 0x469130 };

	GET(WarheadTypeClass*, pWarhead, EAX);
	GET_BASE(CoordStruct*, pCoords, 0x8);

	if (auto const pExt = WarheadTypeExt::ExtMap.Find(pWarhead))
	{
		Point2D screenCoords;

		if (pExt->ShakeIsLocal && !TacticalClass::Instance->CoordsToClient(*pCoords, &screenCoords))
			return SkipShaking;
	}

	return 0;
}

DEFINE_HOOK(0x4690C1, BulletClass_Logics_DetachFromOwner, 0x8)
{
	GET(BulletClass*, pThis, ESI);

	if (pThis->Owner && pThis->WeaponType)
	{
		auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pThis->WeaponType);

		if (pWeaponExt->DetachedFromOwner)
			pThis->Owner = nullptr;
	}

	return 0;
}

DEFINE_HOOK(0x469008, BulletClass_Explode_Cluster, 0x8)
{
	enum { SkipGameCode = 0x469091 };

	GET(BulletClass*, pThis, ESI);
	GET_STACK(CoordStruct, origCoords, STACK_OFFS(0x3C, 0x30));

	if (pThis->Type->Cluster > 0)
	{
		if (auto const pTypeExt = BulletTypeExt::ExtMap.Find(pThis->Type))
		{
			int min = pTypeExt->Cluster_Scatter_Min.Get(Leptons(256));
			int max = pTypeExt->Cluster_Scatter_Max.Get(Leptons(512));
			auto coords = origCoords;

			for (int i = 0; i < pThis->Type->Cluster; i++)
			{
				pThis->Detonate(coords);

				if (!pThis->IsAlive)
					break;

				int distance = ScenarioClass::Instance->Random.RandomRanged(min, max);
				coords = MapClass::GetRandomCoordsNear(origCoords, distance, false);
			}
		}
	}

	return SkipGameCode;
}

DEFINE_HOOK(0x4687F8, BulletClass_Unlimbo_FlakScatter, 0x6)
{
	GET(BulletClass*, pThis, EBX);
	GET_STACK(float, mult, STACK_OFFS(0x5C, 0x44));

	if (pThis->WeaponType)
	{
		if (auto const pTypeExt = BulletTypeExt::ExtMap.Find(pThis->Type))
		{
			int default = RulesClass::Instance->BallisticScatter;
			int min = pTypeExt->BallisticScatter_Min.Get(Leptons(0));
			int max = pTypeExt->BallisticScatter_Max.Get(Leptons(default));

			int result = (mult * ScenarioClass::Instance->Random.RandomRanged(2 * min, 2 * max)) / pThis->WeaponType->Range;
			R->EAX(result);
		}
	}

	return 0;
}

DEFINE_HOOK(0x469D1A, BulletClass_Logics_Debris_Checks, 0x6)
{
	enum { SkipGameCode = 0x469EBA, SetDebrisCount = 0x469D36 };

	GET(BulletClass*, pThis, ESI);

	auto pWHExt = WarheadTypeExt::ExtMap.Find(pThis->WH);

	bool isLand = pThis->GetCell()->LandType != LandType::Water;

	if (!isLand && pWHExt->Debris_Conventional)
		return SkipGameCode;

	// Fix the debris count to be in range of Min, Max instead of Min, Max-1.
	R->EBX(ScenarioClass::Instance->Random.RandomRanged(pThis->WH->MinDebris, pThis->WH->MaxDebris));

	return SetDebrisCount;
}

DEFINE_HOOK(0x469E34, BulletClass_Logics_DebrisAnims, 0x5)
{
	enum { SkipGameCode = 0x469EBA };

	GET(BulletClass*, pThis, ESI);
	GET(int, debrisCount, EBX);

	auto pWHExt = WarheadTypeExt::ExtMap.Find(pThis->WH);
	auto debrisAnims = pWHExt->DebrisAnims.GetElements(RulesClass::Instance->MetallicDebris);

	if (debrisAnims.size() < 1)
		return SkipGameCode;

	while (debrisCount > 0)
	{
		int debrisIndex = ScenarioClass::Instance->Random.RandomRanged(0, debrisAnims.size() - 1);

		auto anim = GameCreate<AnimClass>(debrisAnims[debrisIndex], pThis->GetCoords());

		if (anim && pThis->Owner)
			anim->Owner = pThis->Owner->Owner;

		debrisCount--;
	}

	return SkipGameCode;
}