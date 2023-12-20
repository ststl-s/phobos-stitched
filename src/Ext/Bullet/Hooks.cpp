#include "Body.h"

#include <Helpers/Macro.h>

#include <Ext/Anim/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>

#include <New/Armor/Armor.h>

#include <Misc/CaptureManager.h>

// has everything inited except SpawnNextAnim at this point
DEFINE_HOOK(0x466556, BulletClass_Init, 0x6)
{
	GET(BulletClass*, pThis, ECX);

	if (auto const pOwnerExt = TechnoExt::ExtMap.Find(pThis->Owner))
	{
		for (auto const& pAttachment : pOwnerExt->ChildAttachments)
		{
			if (!TechnoExt::IsReallyAlive(pAttachment->Child))
				continue;

			if (pAttachment->GetType()->InheritWeaponOwner_Parent)
				pThis->Owner = pAttachment->Child;
		}

		if (pOwnerExt->ParentAttachment && pOwnerExt->ParentAttachment->GetType()->InheritWeaponOwner)
			pThis->Owner = pOwnerExt->ParentAttachment->Parent;
	}

	if (auto const pExt = BulletExt::ExtMap.Find(pThis))
	{
		pExt->FirerHouse = pThis->Owner ? pThis->Owner->Owner : nullptr;
		pExt->TypeExtData = BulletTypeExt::ExtMap.Find(pThis->Type);
		pExt->CurrentStrength = pExt->TypeExtData->Strength_UseDamage ? pThis->Health : pExt->TypeExtData->Strength;
		pExt->Armor = pExt->TypeExtData->Armor.Get();

		if (auto const pFirerExt = TechnoExt::ExtMap.Find(pThis->Owner))
		{
			if (pFirerExt->SendPassenger)
			{
				pExt->Passenger = pFirerExt->SendPassenger;
				pFirerExt->SendPassenger = nullptr;

				pExt->SendPassengerMove = pFirerExt->SendPassengerMove;
				pFirerExt->SendPassengerMove = false;

				pExt->SendPassengerMoveHouse = pFirerExt->SendPassengerMoveHouse;
				pFirerExt->SendPassengerMoveHouse = AffectedHouse::Team;

				pExt->SendPassengerMoveHouse_IgnoreNeturalHouse = pFirerExt->SendPassengerMoveHouse_IgnoreNeturalHouse;
				pFirerExt->SendPassengerMoveHouse_IgnoreNeturalHouse = true;

				pExt->SendPassenger_Overlap = pFirerExt->SendPassengerOverlap;
				pFirerExt->SendPassengerOverlap = false;

				pExt->SendPassenger_Select = pFirerExt->SendPassengerSelect;
				pFirerExt->SendPassengerSelect = false;

				pExt->SendPassenger_UseParachute = pFirerExt->SendPassengerUseParachute;
				pFirerExt->SendPassengerUseParachute = true;

				if (pFirerExt->SendPassengerData)
				{
					pExt->CurrentStrength = pExt->Passenger->Health;
					pExt->Armor = static_cast<Armor>(TechnoExt::ExtMap.Find(pExt->Passenger)->GetArmorIdxWithoutShield());
					pFirerExt->SendPassengerData = false;
				}
			}
		}

		if (!pThis->Type->Inviso)
		{
			pExt->InitializeLaserTrails();

			if (pExt->TypeExtData->DetonateOnWay)
				pExt->DetonateOnWay_Timer.Start(pExt->TypeExtData->DetonateOnWay_Delay);
		}
	}

	return 0;
}

// Set in BulletClass::AI and guaranteed to be valid within it.
namespace BulletAITemp
{
	BulletExt::ExtData* ExtData;
	BulletTypeExt::ExtData* TypeExtData;
}

DEFINE_HOOK(0x4666F7, BulletClass_AI, 0x6)
{
	GET(BulletClass*, pThis, EBP);
	
	auto pBulletExt = BulletExt::ExtMap.Find(pThis);
	BulletAITemp::ExtData = pBulletExt;
	BulletAITemp::TypeExtData = pBulletExt->TypeExtData;

	if (const auto pTarget = abstract_cast<TechnoClass*>(pThis->Target))
	{
		auto pTargetExt = TechnoExt::ExtMap.Find(pTarget);
		if (pTargetExt->ParentAttachment && pTargetExt->ParentAttachment->GetType()->MoveTargetToParent)
			pThis->Target = pTargetExt->ParentAttachment->Parent;
	}

	if (pBulletExt->TypeExtData->DetonateOnWay)
	{
		auto weapon = pBulletExt->TypeExtData->DetonateOnWay_Weapon.Get(pThis->WeaponType);
		auto owner = pThis->Owner ? pThis->Owner : nullptr;

		if (pThis->Type->Inviso)
		{
			auto distance = pThis->TargetCoords.DistanceFrom(pThis->SourceCoords);

			for (size_t i = pBulletExt->TypeExtData->DetonateOnWay_LineDistance; i < distance; i += pBulletExt->TypeExtData->DetonateOnWay_LineDistance)
			{
				auto temp = i / distance;

				CoordStruct coords =
				{
					pThis->SourceCoords.X + static_cast<int>(temp * (pThis->TargetCoords.X - pThis->SourceCoords.X)),
					pThis->SourceCoords.Y + static_cast<int>(temp * (pThis->TargetCoords.Y - pThis->SourceCoords.Y)),
					pThis->SourceCoords.Z + static_cast<int>(temp * (pThis->TargetCoords.Z - pThis->SourceCoords.Z))
				};

				if (pBulletExt->TypeExtData->DetonateOnWay_OnCell)
				{
					auto cell = MapClass::Instance->TryGetCellAt(CellClass::Coord2Cell(coords));
					WeaponTypeExt::DetonateAt(weapon, cell, owner);
				}
				else
					WeaponTypeExt::DetonateAt(weapon, coords, owner);
			}
		}
		else if (pBulletExt->DetonateOnWay_Timer.Completed())
		{
			if (pBulletExt->TypeExtData->DetonateOnWay_OnCell)
			{
				auto cell = MapClass::Instance->TryGetCellAt(CellClass::Coord2Cell(pThis->Location));
				WeaponTypeExt::DetonateAt(weapon, cell, owner);
			}
			else
				WeaponTypeExt::DetonateAt(weapon, pThis->GetCoords(), owner);

			pBulletExt->DetonateOnWay_Timer.Start(pBulletExt->TypeExtData->DetonateOnWay_Delay);
		}
	}

	if (pBulletExt->InterceptedStatus == InterceptedStatus::Intercepted)
	{
		if (pBulletExt->Interfere)
		{
			if (!pBulletExt->Interfered)
			{
				if (pBulletExt->InterfereToSource)
				{
					if (TechnoExt::IsReallyAlive(pThis->Owner))
					{
						auto NowTarget = pThis->Owner;
						if (const auto NowOwner = abstract_cast<TechnoClass*>(pThis->Target))
						{
							pThis->Owner = NowOwner;
						}
						else
						{
							pThis->Owner = nullptr;
						}
						pThis->Target = NowTarget;
						pThis->TargetCoords = NowTarget->Location;
					}
					else
					{
						if (const auto NowOwner = abstract_cast<TechnoClass*>(pThis->Target))
						{
							pThis->Owner = NowOwner;
						}
						else
						{
							pThis->Owner = nullptr;
						}
						pThis->Target = nullptr;
						pThis->TargetCoords = pThis->SourceCoords;
					}
				}
				else if (pBulletExt->InterfereToSelf != nullptr)
				{
					pThis->Target = pBulletExt->InterfereToSelf;
					pThis->TargetCoords = pBulletExt->InterfereToSelf->Location;
				}
				else
				{
					double random = ScenarioClass::Instance()->Random.RandomRanged(0, static_cast<int>(pThis->Location.DistanceFrom(pThis->TargetCoords)));
					double theta = ScenarioClass::Instance()->Random.RandomDouble() * Math::TwoPi;

					CoordStruct NewTarget = { pThis->Location.X + static_cast<int>(random * Math::cos(theta)),
										pThis->Location.Y + static_cast<int>(random * Math::sin(theta)),
										0 };
					pThis->Target = nullptr;
					pThis->TargetCoords = NewTarget;
				}

				pThis->Velocity.X = static_cast<double>(pThis->TargetCoords.X - pThis->Location.X);
				pThis->Velocity.Y = static_cast<double>(pThis->TargetCoords.Y - pThis->Location.Y);
				pThis->Velocity.Z = static_cast<double>(pThis->TargetCoords.Z - pThis->Location.Z);
				pThis->Velocity *= 100 / pThis->Velocity.Magnitude();

				pBulletExt->Interfered = true;
			}
		}
		else
		{
			if (pBulletExt->DetonateOnInterception)
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
				pTechno->TakeDamage(damage);
			}

			if (pBulletExt->Passenger)
			{
				auto facing = static_cast<DirType>(ScenarioClass::Instance->Random.RandomRanged(0, 255));
				auto damage = pBulletExt->Passenger->Health * 2;
				pBulletExt->Passenger->Transporter = nullptr;
				++Unsorted::IKnowWhatImDoing;
				pBulletExt->Passenger->Unlimbo(pThis->GetCoords(), facing);
				--Unsorted::IKnowWhatImDoing;
				pBulletExt->Passenger->TakeDamage(damage);
			}
		}
	}

	if (pBulletExt->ShouldDirectional)
	{
		if (!pThis->Type->Inviso)
			pBulletExt->BulletDir = DirStruct((-1) * Math::atan2(pThis->Velocity.Y, pThis->Velocity.X));
	}

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

	//inviso=yes的抛射体只会Update一次，真方便。
	if (pThis->Type->Inviso && pThis->WeaponType != nullptr)
	{
		const auto pWeaponType = pThis->WeaponType;

		if (!pWeaponType->IsElectricBolt)
			BulletExt::DrawElectricLaserWeapon(pThis, pWeaponType);
	}

	return 0;
}

DEFINE_HOOK(0x466897, BulletClass_AI_Trailer, 0x6)
{
	enum { SkipGameCode = 0x4668BD };

	GET(BulletClass*, pThis, EBP);
	GET_STACK(CoordStruct, coords, STACK_OFFSET(0x1A8, -0x184));

	if (auto const pTrailerAnim = GameCreate<AnimClass>(pThis->Type->Trailer, coords, 1, 1))
	{
		auto const pTrailerAnimExt = AnimExt::ExtMap.Find(pTrailerAnim);
		auto const pOwner = pThis->Owner ? pThis->Owner->Owner : BulletAITemp::ExtData->FirerHouse;
		AnimExt::SetAnimOwnerHouseKind(pTrailerAnim, pOwner, nullptr, false, true);
		pTrailerAnimExt->SetInvoker(pThis->Owner);
	}

	return SkipGameCode;
}

// Inviso bullets behave differently in BulletClass::AI when their target is bullet and
// seemingly (at least partially) adopt characteristics of a vertical projectile.
// This is a potentially slightly hacky solution to that, as proper solution
// would likely require making sense of BulletClass::AI and ain't nobody got time for that.
DEFINE_HOOK(0x4668BD, BulletClass_AI_Interceptor_InvisoSkip, 0x6)
{
	enum { DetonateBullet = 0x467F9B };

	GET(BulletClass*, pThis, EBP);

	if (auto const pExt = BulletAITemp::ExtData)
	{
		if (pThis->Type->Inviso && pExt->IsInterceptor)
			return DetonateBullet;
	}

	return 0;
}

DEFINE_HOOK(0x466B67, BulletClass_AI_GetTargetCoords, 0x6)
{
	GET(BulletClass*, pThis, EBP);

	const auto pExt = BulletExt::ExtMap.Find(pThis);

	if (pThis->Target && pExt->TrackTarget)
		pThis->TargetCoords = *R->EAX<CoordStruct*>();
	else
		R->EAX(&pThis->TargetCoords);

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
				double currentHealthPerc = pTarget->GetHealthPercentage();
				bool flipComparations = pWarheadExt->MindControl_Threshold_Inverse;

				if (pWarheadExt->MindControl_Threshold < 0.0 || pWarheadExt->MindControl_Threshold > 1.0)
					pWarheadExt->MindControl_Threshold = flipComparations ? 0.0 : 1.0;
				bool skipMindControl = flipComparations ? (pWarheadExt->MindControl_Threshold > 0.0) : (pWarheadExt->MindControl_Threshold < 1.0);
				bool healthComparation = flipComparations ? (currentHealthPerc <= pWarheadExt->MindControl_Threshold) : (currentHealthPerc >= pWarheadExt->MindControl_Threshold);

				if (skipMindControl
					&& healthComparation
					&& pWarheadExt->MindControl_AlternateDamage.isset()
					&& pWarheadExt->MindControl_AlternateWarhead.isset())
				{
					int damage = pWarheadExt->MindControl_AlternateDamage;
					WarheadTypeClass* pAltWarhead = pWarheadExt->MindControl_AlternateWarhead;
					int realDamage = MapClass::GetTotalDamage(damage, pAltWarhead, pTargetType->Armor, 0);

					if (!pWarheadExt->MindControl_CanKill && pTarget->Health <= realDamage && realDamage > 1)
						pTarget->Health = realDamage;

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
			if (auto const pWarheadExt = WarheadTypeExt::ExtMap.Find(pBulletWH))
			{
				double currentHealthPerc = pTarget->GetHealthPercentage();
				bool flipComparations = pWarheadExt->MindControl_Threshold_Inverse;

				bool skipMindControl = flipComparations ? (pWarheadExt->MindControl_Threshold > 0.0) : (pWarheadExt->MindControl_Threshold < 1.0);
				bool healthComparation = flipComparations ? (currentHealthPerc <= pWarheadExt->MindControl_Threshold) : (currentHealthPerc >= pWarheadExt->MindControl_Threshold);

				if (skipMindControl
					&& healthComparation
					&& pWarheadExt->MindControl_AlternateDamage.isset()
					&& pWarheadExt->MindControl_AlternateWarhead.isset())
				{
					int damage = pWarheadExt->MindControl_AlternateDamage;
					WarheadTypeClass* pAltWarhead = pWarheadExt->MindControl_AlternateWarhead;
					auto pAttacker = pBullet->Owner;
					auto pAttackingHouse = pBullet->Owner->Owner;
					int realDamage = MapClass::GetTotalDamage(damage, pAltWarhead, pTargetType->Armor, 0);

					if (!pWarheadExt->MindControl_CanKill && pTarget->Health <= realDamage)
					{
						pTarget->Health += abs(realDamage);
						realDamage = 1;
						pTarget->TakeDamage(realDamage, pAttackingHouse, pAttacker, pAltWarhead);
						pTarget->Health = 1;
					}
					else
					{
						pTarget->TakeDamage(damage, pAttackingHouse, pAttacker, pAltWarhead);
					}

					pAnimType = nullptr;

					// If the alternative Warhead have AnimList tag declared then use it
					if (pWarheadExt->MindControl_AlternateWarhead->AnimList.Count > 0)
					{
						if (CellClass* pCell = MapClass::Instance->TryGetCellAt(pTarget->Location))
						{
							pAnimType = MapClass::SelectDamageAnimation(damage, pAltWarhead, pCell->LandType, pTarget->Location);
						}
					}

					R->EBX(pAnimType);
				}
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

DEFINE_HOOK(0x46A3D6, BulletClass_Shrapnel_Forced, 0xA)
{
	enum { Shrapnel = 0x46A40C, Skip = 0x46ADCD };

	GET(BulletClass*, pBullet, EDI);

	auto const pData = BulletTypeExt::ExtMap.Find(pBullet->Type);

	if (pData->Shrapnel_Chance < 1.0)
	{
		double dice = ScenarioClass::Instance->Random.RandomDouble();

		if (pData->Shrapnel_Chance < dice)
			return Skip;
	}

	if (auto const pObject = pBullet->GetCell()->FirstObject)
	{
		if (pObject->WhatAmI() != AbstractType::Building || pData->Shrapnel_AffectsBuildings)
			return Shrapnel;
	}
	else if (pData->Shrapnel_AffectsGround)
		return Shrapnel;

	return Skip;
}

DEFINE_HOOK(0x46A4ED, BulletClass_Shrapnel_Fix, 0x5)
{
	enum { Continue = 0x46A4F3, Skip = 0x46A8EA };

	GET(BulletClass*, pThis, EDI);
	GET(AbstractClass*, pTarget, EBP);

	if (BulletTypeExt::ExtMap.Find(pThis->Type)->Shrapnel_PriorityVersus)
	{
		const WarheadTypeClass* pWH = pThis->Type->ShrapnelWeapon->Warhead;

		if (const auto pTargetObj = abstract_cast<ObjectClass*>(pTarget))
		{
			if (const auto pTargetTechno = abstract_cast<TechnoClass*>(pTargetObj))
			{
				const auto pTechnoExt = TechnoExt::ExtMap.Find(pTargetTechno);

				if (CustomArmor::GetVersus(pWH, pTechnoExt->GetArmorIdx(pWH)) == 0.0)
					return Skip;
			}
			else if (CustomArmor::GetVersus(pWH, pTargetObj->GetType()->Armor) == 0.0)
			{
				return Skip;
			}

			if (pThis->Target == pTarget)
				return Skip;
		}
	}

	return Continue;
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

DEFINE_HOOK(0x469A75, BulletClass_Logics_DamageHouse, 0x7)
{
	GET(BulletClass*, pThis, ESI);
	GET(HouseClass*, pHouse, ECX);

	if (auto const pExt = BulletExt::ExtMap.Find(pThis))
	{
		if (!pHouse)
			R->ECX(pExt->FirerHouse);
	}

	return 0;
}

DEFINE_HOOK(0x469008, BulletClass_Explode_Cluster, 0x8)
{
	enum { SkipGameCode = 0x469091 };

	GET(BulletClass*, pThis, ESI);
	GET_STACK(CoordStruct, origCoords, STACK_OFFSET(0x3C, -0x30));

	if (pThis->Type->Cluster > 0)
	{
		if (auto const pTypeExt = BulletTypeExt::ExtMap.Find(pThis->Type))
		{
			int min = pTypeExt->ClusterScatter_Min.Get(Leptons(256));
			int max = pTypeExt->ClusterScatter_Max.Get(Leptons(512));
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
	GET_STACK(float, mult, STACK_OFFSET(0x5C, -0x44));

	if (pThis->WeaponType)
	{
		if (auto const pTypeExt = BulletTypeExt::ExtMap.Find(pThis->Type))
		{
			int defaultValue = RulesClass::Instance->BallisticScatter;
			int min = pTypeExt->BallisticScatter_Min.Get(Leptons(0));
			int max = pTypeExt->BallisticScatter_Max.Get(Leptons(defaultValue));

			int result = static_cast<int>((mult * ScenarioClass::Instance->Random.RandomRanged(2 * min, 2 * max)) / pThis->WeaponType->Range);
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

DEFINE_HOOK(0x468E9F, BulletClass_Logics_SnapOnTarget, 0x6)
{
	enum { NoSnap = 0x468FF4, ForceSnap = 0x468EC7 };

	GET(BulletClass*, pThis, ESI);

	if (pThis->Type->Inviso)
	{
		R->EAX(pThis->Type);
		return ForceSnap;
	}

	if (auto const pExt = BulletExt::ExtMap.Find(pThis))
	{
		if (pExt->Trajectory)
		{
			if (pExt->Trajectory->Flag == TrajectoryFlag::Straight && !pExt->SnappedToTarget)
				return NoSnap;
		}
	}

	return 0;
}

DEFINE_HOOK(0x468D3F, BulletClass_IsForcedToExplode_AirTarget, 0x6)
{
	enum { DontExplode = 0x468D73 };

	GET(BulletClass*, pThis, ESI);

	if (auto const pExt = BulletExt::ExtMap.Find(pThis))
	{
		if (pExt->Trajectory)
		{
			// Straight trajectory has its own proximity checks.
			if (pExt->Trajectory->Flag == TrajectoryFlag::Straight)
				return DontExplode;
		}
	}
	return 0;
}

DEFINE_HOOK(0x4690C1, BulletClass_Logics_DetonateOnAllMapObjects, 0x8)
{
	enum { ReturnFromFunction = 0x46A2FB };

	GET(BulletClass*, pThis, ESI);

	if (auto const pWHExt = WarheadTypeExt::ExtMap.Find(pThis->WH))
	{
		if (pWHExt->DetonateOnAllMapObjects && !pWHExt->WasDetonatedOnAllMapObjects)
		{
			pWHExt->WasDetonatedOnAllMapObjects = true;
			auto const pExt = BulletExt::ExtMap.Find(pThis);
			auto pOwner = pThis->Owner ? pThis->Owner->Owner : pExt->FirerHouse;

			auto tryDetonate = [pThis, pWHExt, pOwner](TechnoClass* pTechno)
			{
				if (pWHExt->EligibleForFullMapDetonation(pTechno, pOwner))
				{
					pThis->Target = pTechno;
					CoordStruct coords = pTechno->GetCoords();
					pThis->Detonate(coords);
				}
			};

			if ((pWHExt->DetonateOnAllMapObjects_AffectTargets & AffectedTarget::Aircraft) != AffectedTarget::None)
			{
				for (auto pTechno : *AircraftClass::Array)
					tryDetonate(pTechno);
			}

			if ((pWHExt->DetonateOnAllMapObjects_AffectTargets & AffectedTarget::Building) != AffectedTarget::None)
			{
				for (auto pTechno : *BuildingClass::Array)
					tryDetonate(pTechno);
			}

			if ((pWHExt->DetonateOnAllMapObjects_AffectTargets & AffectedTarget::Infantry) != AffectedTarget::None)
			{
				for (auto pTechno : *InfantryClass::Array)
					tryDetonate(pTechno);
			}

			if ((pWHExt->DetonateOnAllMapObjects_AffectTargets & AffectedTarget::Unit) != AffectedTarget::None)
			{
				for (auto pTechno : *UnitClass::Array)
					tryDetonate(pTechno);
			}

			pWHExt->WasDetonatedOnAllMapObjects = false;

			return ReturnFromFunction;
		}
	}

	return 0;
}

DEFINE_HOOK(0x467CCA, BulletClass_AI_TargetSnapChecks, 0x6)
{
	enum { SkipChecks = 0x467CDE };

	GET(BulletClass*, pThis, EBP);

	// Do not require Airburst=no to check target snapping for Inviso / Trajectory=Straight projectiles
	if (pThis->Type->Inviso)
	{
		R->EAX(pThis->Type);
		return SkipChecks;
	}
	else if (auto const pExt = BulletAITemp::ExtData)
	{
		if (pExt->Trajectory)
		{
			if (pExt->Trajectory->Flag == TrajectoryFlag::Straight)
			{
				R->EAX(pThis->Type);
				return SkipChecks;
			}
		}
	}

	return 0;
}

DEFINE_HOOK(0x468E61, BulletClass_Explode_TargetSnapChecks1, 0x6)
{
	enum { SkipChecks = 0x468E7B };

	GET(BulletClass*, pThis, ESI);

	// Do not require Airburst=no to check target snapping for Inviso / Trajectory=Straight projectiles
	if (pThis->Type->Inviso)
	{
		R->EAX(pThis->Type);
		return SkipChecks;
	}
	else if (pThis->Type->Arcing || pThis->Type->ROT > 0)
	{
		return 0;
	}
	else if (auto const pExt = BulletExt::ExtMap.Find(pThis))
	{
		if (pExt->Trajectory)
		{
			if (pExt->Trajectory->Flag == TrajectoryFlag::Straight)
			{
				R->EAX(pThis->Type);
				return SkipChecks;
			}
		}
	}

	return 0;
}

DEFINE_HOOK(0x468E9F, BulletClass_Explode_TargetSnapChecks2, 0x6)
{
	enum { SkipInitialChecks = 0x468EC7, SkipSetCoordinate = 0x468F23 };

	GET(BulletClass*, pThis, ESI);

	// Do not require EMEffect=no & Airburst=no to check target coordinate snapping for Inviso projectiles.
	if (pThis->Type->Inviso)
	{
		R->EAX(pThis->Type);
		return SkipInitialChecks;
	}
	else if (pThis->Type->Arcing || pThis->Type->ROT > 0)
	{
		return 0;
	}

	// Do not force Trajectory=Straight projectiles to detonate at target coordinates under certain circumstances.
	// Fixes issues with walls etc.
	if (auto const pExt = BulletExt::ExtMap.Find(pThis))
	{
		if (pExt->Trajectory)
		{
			if (pExt->Trajectory->Flag == TrajectoryFlag::Straight)
				return SkipSetCoordinate;
		}
	}

	return 0;
}

DEFINE_HOOK(0x46A290, BulletClass_Logics_ExtraWarheads, 0x5)
{
	GET(BulletClass*, pThis, ESI);
	GET_BASE(CoordStruct*, coords, 0x8);

	if (pThis->WeaponType)
	{
		auto const pWeaponExt = WeaponTypeExt::ExtMap.Find(pThis->WeaponType);
		int defaultDamage = pThis->WeaponType->Damage;

		for (size_t i = 0; i < pWeaponExt->ExtraWarheads.size(); i++)
		{
			auto const pWH = pWeaponExt->ExtraWarheads[i];
			auto const pHouse = pThis->Owner ? pThis->Owner->Owner : BulletExt::ExtMap.Find(pThis)->FirerHouse;
			int damage = defaultDamage;

			if (pWeaponExt->ExtraWarheads_DamageOverrides.size() > i)
				damage = pWeaponExt->ExtraWarheads_DamageOverrides[i];

			WarheadTypeExt::DetonateAt(pWH, *coords, pThis->Owner, damage, pHouse);
		}
	}

	return 0;
}

DEFINE_HOOK(0x469A3F, BulletClass_Detonate_Shrapnel, 0x6)
{
	GET(BulletClass*, pThis, ESI);
	auto pExt = BulletExt::ExtMap.Find(pThis);

	pExt->Shrapnel();

	return 0x469A56;
}

//namespace BulletTemp
//{
//	CoordStruct Buffer = CoordStruct::Empty;
//}
//
//DEFINE_HOOK(0x46870A, BulletClass_MoveTo_TargetCoords, 0x8)
//{
//	GET(BulletClass*, pBullet, EBX);
//
//	if (pBullet->Type->Inaccurate)
//	{
//		CoordStruct& buffer = BulletTemp::Buffer;
//		buffer = pBullet->Target->GetCenterCoords();
//		buffer += BulletExt::CalculateInaccurate(pBullet->Type);
//		R->EAX(&buffer);
//	}
//
//	return 0;
//}
