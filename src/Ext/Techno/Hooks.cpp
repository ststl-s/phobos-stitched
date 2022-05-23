#include <InfantryClass.h>
#include <ScenarioClass.h>
#include <SpawnManagerClass.h>

#include "Body.h"

#include <Ext/TechnoType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Utilities/EnumFunctions.h>

DEFINE_HOOK(0x6F9E50, TechnoClass_AI, 0x5)
{
	GET(TechnoClass*, pThis, ECX);
	TechnoTypeClass* pType = pThis->GetTechnoType();
	
	if (pType == nullptr)
		return 0;

	auto pExt = TechnoExt::ExtMap.Find(pThis);
	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
	
	//Phobos and PR
	TechnoExt::ApplyMindControlRangeLimit(pThis);
	TechnoExt::ApplyInterceptor(pThis);
	TechnoExt::ApplyPowered_KillSpawns(pThis);
	TechnoExt::ApplySpawn_LimitRange(pThis);
	TechnoExt::CheckDeathConditions(pThis);
	TechnoExt::EatPassengers(pThis);
	TechnoExt::UpdateMindControlAnim(pThis);
	TechnoExt::ForceJumpjetTurnToTarget(pThis);//TODO: move to locomotor processing
	TechnoExt::MCVLocoAIFix(pThis);
	TechnoExt::HarvesterLocoFix(pThis);

	//stitched
	TechnoExt::CheckIonCannonConditions(pThis);
	TechnoExt::RunIonCannonWeapon(pThis, pExt);
	TechnoExt::RunBeamCannon(pThis, pExt);
	TechnoExt::ChangePassengersList(pThis, pExt);
	TechnoExt::MovePassengerToSpawn(pThis, pTypeExt);
	TechnoExt::SilentPassenger(pThis, pExt, pTypeExt);
	TechnoExt::Spawner_SameLoseTarget(pThis, pExt, pTypeExt);
	TechnoExt::RunFireSelf(pThis, pExt, pTypeExt);
	TechnoExt::UpdateFireScript(pThis, pExt, pTypeExt);

	// LaserTrails update routine is in TechnoClass::AI hook because TechnoClass::Draw
	// doesn't run when the object is off-screen which leads to visual bugs - Kerbiter
	for (auto const& trail : pExt->LaserTrails)
		trail->Update(TechnoExt::GetFLHAbsoluteCoords(pThis, trail->FLH, trail->IsOnTurret));

	return 0;
}

//DEFINE_HOOK(0x710460, TechnoClass_Destroyed_EraseHugeHP, 0x6)
// pThis <- ECX
//old hook unused when infantry destoryed
//borrowed from YRDynamicPatcher-Kratos
DEFINE_HOOK(0x702050, TechnoClass_Destroyed, 0x6)
{//this hook borrowed from TechnoAttachments
	GET(TechnoClass*, pThis, ESI);
	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	TechnoExt::EraseHugeHP(pThis, pTypeExt);
	TechnoExt::HandleHostDestruction(pThis);
	TechnoExt::Destoryed_EraseAttachment(pThis);
	
	return 0;
}

DEFINE_HOOK(0x6F6B1C, TechnoClass_Limbo, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	TechnoTypeClass* pType = pThis->GetTechnoType();
	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
	TechnoExt::EraseHugeHP(pThis, pTypeExt);
	return 0;
}

DEFINE_HOOK(0x6F6F20, TechnoClass_Unlimbo, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	TechnoExt::InitialShowHugeHP(pThis);
	return 0;
}

DEFINE_HOOK(0x6F42F7, TechnoClass_Init_NewEntities, 0x2)
{
	GET(TechnoClass*, pThis, ESI);
	
	if (pThis->GetTechnoType() == nullptr)
		return 0;

	//Debug::Log("[TechnoClass] Init Techno address[0x%X]\n", pThis);

	TechnoExt::InitializeShield(pThis);
	TechnoExt::InitializeLaserTrails(pThis);
	TechnoExt::InitializeAttachments(pThis);
	TechnoExt::InitialShowHugeHP(pThis);

	//Debug::Log("[TechnoClass] Finish Init Techno address[0x%X]\n", pThis);

	return 0;
}

DEFINE_HOOK(0x702E4E, TechnoClass_Save_Killer_Techno, 0x6)
{
	GET(TechnoClass*, pKiller, EDI);
	GET(TechnoClass*, pVictim, ECX);

	if (pKiller && pVictim)
		TechnoExt::ObjectKilledBy(pVictim, pKiller);

	return 0;
}

DEFINE_HOOK_AGAIN(0x7355C0, TechnoClass_Init_InitialStrength, 0x6) // UnitClass_Init
DEFINE_HOOK_AGAIN(0x517D69, TechnoClass_Init_InitialStrength, 0x6) // InfantryClass_Init
DEFINE_HOOK_AGAIN(0x442C7B, TechnoClass_Init_InitialStrength, 0x6) // BuildingClass_Init
DEFINE_HOOK(0x414057, TechnoClass_Init_InitialStrength, 0x6)       // AircraftClass_Init
{
	GET(TechnoClass*, pThis, ESI);

	if (R->Origin() != 0x517D69)
	{
		if (auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType()))
		{
			if (R->Origin() != 0x442C7B)
				R->EAX(pTypeExt->InitialStrength.Get(R->EAX<int>()));
			else
				R->ECX(pTypeExt->InitialStrength.Get(R->ECX<int>()));
		}
	}
	else if (auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType()))
	{
		auto strength = pTypeExt->InitialStrength.Get(R->EDX<int>());
		pThis->Health = strength;
		pThis->EstimatedHealth = strength;
	}

	return 0;
}

// Issue #271: Separate burst delay for weapon type
// Author: Starkku
DEFINE_HOOK(0x6FD05E, TechnoClass_Rearm_Delay_BurstDelays, 0x7)
{
	GET(TechnoClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EDI);
	auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);
	int burstDelay = -1;

	if (pWeaponExt->Burst_Delays.size() > (unsigned)pThis->CurrentBurstIndex)
		burstDelay = pWeaponExt->Burst_Delays[pThis->CurrentBurstIndex - 1];
	else if (pWeaponExt->Burst_Delays.size() > 0)
		burstDelay = pWeaponExt->Burst_Delays[pWeaponExt->Burst_Delays.size() - 1];

	if (burstDelay >= 0)
	{
		R->EAX(burstDelay);
		return 0x6FD099;
	}

	// Restore overridden instructions
	GET(int, idxCurrentBurst, ECX);
	return idxCurrentBurst <= 0 || idxCurrentBurst > 4 ? 0x6FD084 : 0x6FD067;
}

DEFINE_HOOK(0x6F3B37, TechnoClass_Transform_6F3AD0_BurstFLH_1, 0x7)
{
	GET(TechnoClass*, pThis, EBX);
	GET_STACK(int, weaponIndex, STACK_OFFS(0xD8, -0x8));
	bool FLHFound = false;
	CoordStruct FLH = CoordStruct::Empty;

	FLH = TechnoExt::GetBurstFLH(pThis, weaponIndex, FLHFound);

	if (!FLHFound)
	{
		if (auto pInf = abstract_cast<InfantryClass*>(pThis))
			FLH = TechnoExt::GetSimpleFLH(pInf, weaponIndex, FLHFound);
	}

	if (FLHFound)
	{
		R->ECX(FLH.X);
		R->EBP(FLH.Y);
		R->EAX(FLH.Z);
	}

	return 0;
}

DEFINE_HOOK(0x6F3C88, TechnoClass_Transform_6F3AD0_BurstFLH_2, 0x6)
{
	GET(TechnoClass*, pThis, EBX);
	GET_STACK(int, weaponIndex, STACK_OFFS(0xD8, -0x8));
	bool FLHFound = false;

	TechnoExt::GetBurstFLH(pThis, weaponIndex, FLHFound);

	if (FLHFound)
		R->EAX(0);

	return 0;
}

// Issue #237 NotHuman additional animations support
// Author: Otamaa
#pragma warning(push)
#pragma warning(disable : 4067)
DEFINE_HOOK(0x518505, InfantryClass_TakeDamage_NotHuman, 0x4)
{
	GET(InfantryClass* const, pThis, ESI);
	REF_STACK(args_ReceiveDamage const, receiveDamageArgs, STACK_OFFS(0xD0, -0x4));

	// Die1-Die5 sequences are offset by 10
	constexpr auto Die = [](int x) { return x + 10; };

	int resultSequence = Die(1);
	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (pTypeExt->NotHuman_RandomDeathSequence.Get())
		resultSequence = ScenarioClass::Instance->Random.RandomRanged(Die(1), Die(5));

	if (receiveDamageArgs.WH)
	{
		if (auto const pWarheadExt = WarheadTypeExt::ExtMap.Find(receiveDamageArgs.WH))
		{
			int whSequence = pWarheadExt->NotHuman_DeathSequence.Get();
			if (whSequence > 0)
				resultSequence = Math::min(Die(whSequence), Die(5));
		}
	}

	R->ECX(pThis);
	pThis->PlayAnim(static_cast<Sequence>(resultSequence), true);

	return 0x518515;
}

DEFINE_HOOK(0x5218F3, InfantryClass_WhatWeaponShouldIUse_DeployFireWeapon, 0x6)
{
	GET(TechnoTypeClass*, pType, ECX);

	if (pType->DeployFireWeapon == -1)
		return 0x52194E;

	return 0;
}

// Customizable OpenTopped Properties
// Author: Otamaa

DEFINE_HOOK(0x6F72D2, TechnoClass_IsCloseEnoughToTarget_OpenTopped_RangeBonus, 0xC)
{
	GET(TechnoClass* const, pThis, ESI);

	if (auto pTransport = pThis->Transporter)
	{
		if (auto pExt = TechnoTypeExt::ExtMap.Find(pTransport->GetTechnoType()))
		{
			R->EAX(pExt->OpenTopped_RangeBonus.Get(RulesClass::Instance->OpenToppedRangeBonus));
			return 0x6F72DE;
		}
	}

	return 0;
}

DEFINE_HOOK(0x6FE43B, TechnoClass_Fire_OpenTopped_DmgMult, 0x8)
{
	enum { ApplyDamageMult = 0x6FE45A, ContinueCheck = 0x6FE460 };

	GET(TechnoClass* const, pThis, ESI);

	//replacing whole check due to `fild`
	if (pThis->InOpenToppedTransport)
	{
		GET_STACK(int, nDamage, STACK_OFFS(0xB0, 0x84));
		float nDamageMult = static_cast<float>(RulesClass::Instance->OpenToppedDamageMultiplier);

		if (auto pTransport = pThis->Transporter)
		{
			if (auto pExt = TechnoTypeExt::ExtMap.Find(pTransport->GetTechnoType()))
			{
				//it is float isnt it YRPP ? , check tomson26 YR-IDB !
				nDamageMult = pExt->OpenTopped_DamageMultiplier.Get(nDamageMult);
			}
		}

		R->EAX(Game::F2I(nDamage * nDamageMult));
		return ApplyDamageMult;
	}

	return ContinueCheck;
}

DEFINE_HOOK(0x71A82C, TemporalClass_AI_Opentopped_WarpDistance, 0xC)
{
	GET(TemporalClass* const, pThis, ESI);

	if (auto pTransport = pThis->Owner->Transporter)
	{
		if (auto pExt = TechnoTypeExt::ExtMap.Find(pTransport->GetTechnoType()))
		{
			R->EDX(pExt->OpenTopped_WarpDistance.Get(RulesClass::Instance->OpenToppedWarpDistance));
			return 0x71A838;
		}
	}

	return 0;
}

DEFINE_HOOK(0x7098B9, TechnoClass_TargetSomethingNearby_AutoFire, 0x6)
{
	GET(TechnoClass* const, pThis, ESI);

	if (auto pExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType()))
	{
		if (pExt->AutoFire)
		{
			if (pExt->AutoFire_TargetSelf)
				pThis->SetTarget(pThis);
			else
				pThis->SetTarget(pThis->GetCell());

			return 0x7099B8;
		}
	}

	return 0;
}

DEFINE_HOOK(0x6FE19A, TechnoClass_FireAt_AreaFire, 0x6)
{
	enum { DoNotFire = 0x6FE4E7, SkipSetTarget = 0x6FE1D5 };

	GET(TechnoClass* const, pThis, ESI);
	GET(CellClass* const, pCell, EAX);
	GET_STACK(WeaponTypeClass*, pWeaponType, STACK_OFFS(0xB0, 0x70));

	if (auto pExt = WeaponTypeExt::ExtMap.Find(pWeaponType))
	{
		if (pExt->AreaFire_Target == AreaFireTarget::Random)
		{
			auto const range = pWeaponType->Range / 256.0;

			std::vector<CellStruct> adjacentCells = GeneralUtils::AdjacentCellsInRange(static_cast<size_t>(range + 0.99));
			size_t size = adjacentCells.size();

			for (unsigned int i = 0; i < size; i++)
			{
				int rand = ScenarioClass::Instance->Random.RandomRanged(0, size - 1);
				unsigned int cellIndex = (i + rand) % size;
				CellStruct tgtPos = pCell->MapCoords + adjacentCells[cellIndex];
				CellClass* tgtCell = MapClass::Instance->GetCellAt(tgtPos);

				if (EnumFunctions::AreCellAndObjectsEligible(tgtCell, pExt->CanTarget, pExt->CanTargetHouses, pThis->Owner, true))
				{
					R->EAX(tgtCell);
					return 0;
				}
			}

			return DoNotFire;
		}
		else if (pExt->AreaFire_Target == AreaFireTarget::Self)
		{
			if (!EnumFunctions::AreCellAndObjectsEligible(pThis->GetCell(), pExt->CanTarget, pExt->CanTargetHouses, nullptr, false))
				return DoNotFire;

			R->EAX(pThis);
			return SkipSetTarget;
		}

		if (!EnumFunctions::AreCellAndObjectsEligible(pCell, pExt->CanTarget, pExt->CanTargetHouses, nullptr, false))
			return DoNotFire;
	}

	return 0;
}

DEFINE_HOOK(0x702819, TechnoClass_ReceiveDamage_Decloak, 0xA)
{
	GET(TechnoClass* const, pThis, ESI);
	GET_STACK(WarheadTypeClass*, pWarhead, STACK_OFFS(0xC4, -0xC));

	if (auto pExt = WarheadTypeExt::ExtMap.Find(pWarhead))
	{
		if (pExt->DecloakDamagedTargets)
			pThis->Uncloak(false);
	}

	return 0x702823;
}

DEFINE_HOOK(0x73DE90, UnitClass_SimpleDeployer_TransferLaserTrails, 0x6)
{
	GET(UnitClass*, pUnit, ESI);

	auto pTechnoExt = TechnoExt::ExtMap.Find(pUnit);
	auto pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pUnit->GetTechnoType());

	if (pTechnoExt && pTechnoTypeExt)
	{
		if (pTechnoExt->LaserTrails.size())
			pTechnoExt->LaserTrails.clear();

		for (auto const& entry : pTechnoTypeExt->LaserTrailData)
		{
			if (auto const pLaserType = LaserTrailTypeClass::Array[entry.Type].get())
			{
				pTechnoExt->LaserTrails.push_back(std::make_unique<LaserTrailClass>(
					pLaserType, pUnit->Owner, entry.FLH, entry.IsOnTurret));
			}
		}
	}

	return 0;
}

DEFINE_HOOK(0x71067B, TechnoClass_EnterTransport_LaserTrails, 0x7)
{
	GET(TechnoClass*, pTechno, EDI);

	auto pTechnoExt = TechnoExt::ExtMap.Find(pTechno);
	auto pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pTechno->GetTechnoType());

	if (pTechnoExt && pTechnoTypeExt)
	{
		for (auto& pLaserTrail : pTechnoExt->LaserTrails)
		{
			pLaserTrail->Visible = false;
			pLaserTrail->LastLocation = { };
		}

		if (pTechno->WhatAmI() == AbstractType::Infantry)
		{
			InfantryClass* pInf = abstract_cast<InfantryClass*>(pTechno);
			InfantryTypeClass* pInfType = abstract_cast<InfantryTypeClass*>(pTechno->GetTechnoType());

			if (pInfType->Cyborg && pInf->Crawling == true)
			{
				pTechnoExt->IsLeggedCyborg = true;
			}
		}

		if (pTechno->Transporter)
		{
			if (auto pTransportTypeExt = TechnoTypeExt::ExtMap.Find(pTechno->Transporter->GetTechnoType()))
			{
				if (pTransportTypeExt->CanRepairCyborgLegs)
					pTechnoExt->IsLeggedCyborg = false;
			}
		}
	}

	return 0;
}

DEFINE_HOOK(0x518047, TechnoClass_Destroyed_IsCyborg, 0x5)
{
	GET(InfantryClass*, pInf, ESI);
	GET(DamageState, ds, EAX);

	if (pInf && ds != DamageState::PostMortem)
	{
		if (pInf->Type->Cyborg)
		{
			auto pTechnoExt = TechnoExt::ExtMap.Find(pInf);

			if (pTechnoExt && pInf->Type->Cyborg
				&& pInf->Crawling == true
				&& !pTechnoExt->IsLeggedCyborg)
			{
				pTechnoExt->IsLeggedCyborg = true;
			}
		}
	}

	return 0;
}

DEFINE_HOOK(0x5F4F4E, ObjectClass_Unlimbo_LaserTrails, 0x7)
{
	GET(TechnoClass*, pTechno, ECX);

	auto pTechnoExt = TechnoExt::ExtMap.Find(pTechno);
	if (pTechnoExt)
	{
		for (auto& pLaserTrail : pTechnoExt->LaserTrails)
		{
			pLaserTrail->LastLocation = { };
			pLaserTrail->Visible = true;
		}

		// Fix legless Cyborgs when leave transports
		if (pTechnoExt->IsLeggedCyborg)
		{
			InfantryClass* soldier = abstract_cast<InfantryClass*>(pTechno);

			soldier->SequenceAnim = Sequence::Prone;
			soldier->Crawling = true;
		}
	}

	return 0;
}

DEFINE_HOOK(0x6F3428, TechnoClass_GetWeapon_ForceWeapon, 0x6)
{
	GET(TechnoClass*, pTechno, ECX);

	if (pTechno && pTechno->Target)
	{
		auto pTechnoType = pTechno->GetTechnoType();
		if (!pTechnoType)
			return 0;

		auto pTarget = abstract_cast<TechnoClass*>(pTechno->Target);
		if (!pTarget)
			return 0;

		auto pTargetType = pTarget->GetTechnoType();
		if (!pTargetType)
			return 0;

		if (auto pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pTechnoType))
		{
			if (pTechnoTypeExt->ForceWeapon_Naval_Decloaked >= 0
				&& pTargetType->Cloakable && pTargetType->Naval
				&& pTarget->CloakState == CloakState::Uncloaked)
			{
				R->EAX(pTechnoTypeExt->ForceWeapon_Naval_Decloaked);
				return 0x6F37AF;
			}
		}
	}

	return 0;
}

// Update ammo rounds
DEFINE_HOOK(0x6FB086, TechnoClass_Reload_ReloadAmount, 0x8)
{
	GET(TechnoClass* const, pThis, ECX);

	TechnoExt::UpdateSharedAmmo(pThis);

	return 0;
}

DEFINE_HOOK(0x6FA793, TechnoClass_AI_SelfHealGain, 0x5)
{
	enum { Infantry = 0x6FA8D2, Unit = 0x6FA7E2, None = 0x6FA941, GameChecks = 0x6FA7B2 };

	GET(TechnoClass*, pThis, ESI);

	bool skipSelfHeal = pThis->Health >= pThis->GetTechnoType()->Strength;

	if (!pThis->Health)
		skipSelfHeal = true;

	R->BL(skipSelfHeal);

	if (auto const pExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType()))
	{
		if (pExt->SelfHealGainType.isset())
		{
			if (!skipSelfHeal)
			{
				if (pExt->SelfHealGainType.Get() == SelfHealGainType::Infantry)
					return Infantry;
				else if (pExt->SelfHealGainType.Get() == SelfHealGainType::Units)
					return Unit;
			}

			return None;
		}
	}

	return GameChecks;
}


DEFINE_HOOK(0x70A4FB, TechnoClass_Draw_Pips_SelfHealGain, 0x5)
{
	enum { SkipGameDrawing = 0x70A6C0 };

	GET(TechnoClass*, pThis, ECX);
	GET_STACK(Point2D*, pLocation, STACK_OFFS(0x74, -0x4));
	GET_STACK(RectangleStruct*, pBounds, STACK_OFFS(0x74, -0xC));

	TechnoExt::DrawSelfHealPips(pThis, TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType()), pLocation, pBounds);

	return SkipGameDrawing;
}

DEFINE_HOOK(0x6FF43F, TechnoClass_FireAt_FeedbackWeapon, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EBX);

	if (auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon))
	{
		if (pWeaponExt->FeedbackWeapon.isset())
		{
			auto fbWeapon = pWeaponExt->FeedbackWeapon.Get();

			if (pThis->InOpenToppedTransport && !fbWeapon->FireInTransport)
				return 0;

			WeaponTypeExt::DetonateAt(fbWeapon, pThis, pThis);
		}
	}

	return 0;
}

DEFINE_HOOK(0x6FD446, TechnoClass_FireLaser_IsSingleColor, 0x7)
{
	GET(WeaponTypeClass* const, pWeapon, ECX);
	GET(LaserDrawClass* const, pLaser, EAX);

	if (auto const pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon))
	{
		if (!pLaser->IsHouseColor && pWeaponExt->Laser_IsSingleColor)
			pLaser->IsHouseColor = true;

	}

	return 0;
}

DEFINE_HOOK(0x6B7265, SpawnManagerClass_AI_UpdateTimer, 0x6)
{
	GET(SpawnManagerClass* const, pThis, ESI);

	if (pThis->Owner && pThis->Status == SpawnManagerStatus::Launching && pThis->CountDockedSpawns() != 0)
	{
		if (auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Owner->GetTechnoType()))
		{
			if (pTypeExt->Spawner_DelayFrames.isset()) { }
			R->EAX(std::min(pTypeExt->Spawner_DelayFrames.Get(), 10));
		}
	}

	return 0;
}

DEFINE_HOOK_AGAIN(0x6B73BE, SpawnManagerClass_AI_SpawnTimer, 0x6)
DEFINE_HOOK(0x6B73AD, SpawnManagerClass_AI_SpawnTimer, 0x5)
{
	GET(SpawnManagerClass* const, pThis, ESI);

	if (pThis->Owner)
	{
		if (auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Owner->GetTechnoType()))
		{
			if (pTypeExt->Spawner_DelayFrames.isset())
				R->ECX(pTypeExt->Spawner_DelayFrames.Get());
		}
	}

	return 0;
}

DEFINE_HOOK(0x701DFF, TechnoClass_ReceiveDamage_FlyingStrings, 0x7)
{
	GET(TechnoClass* const, pThis, ESI);
	GET(int* const, pDamage, EBX);

	if (Phobos::Debug_DisplayDamageNumbers && *pDamage)
		TechnoExt::DisplayDamageNumberString(pThis, *pDamage, false);

	if (*pDamage)
        TechnoExt::ReceiveDamageAnim(pThis, *pDamage);

	return 0;
}

DEFINE_HOOK_AGAIN(0x6F7EC2, TechnoClass_ThreatEvals_OpenToppedOwner, 0x6)
DEFINE_HOOK(0x6F8FD7, TechnoClass_ThreatEvals_OpenToppedOwner, 0x5)
{
	enum { SkipCheckOne = 0x6F8FDC, SkipCheckTwo = 0x6F7EDA };

	bool isFirstHook = R->Origin() == 0x6F8FD7;

	TechnoClass* pThis = nullptr;

	if (isFirstHook)
		pThis = R->ESI<TechnoClass*>();
	else
		pThis = R->EDI<TechnoClass*>();

	auto pTransport = pThis->Transporter;

	if (pTransport)
	{
		if (auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pTransport->GetTechnoType()))
		{
			if (pTypeExt->Passengers_ChangeOwnerWithTransport)
				return isFirstHook ? SkipCheckOne : SkipCheckTwo;
		}
	}

	return 0;
}

DEFINE_HOOK(0x701881, TechnoClass_ChangeHouse_Passengers, 0x5)
{
	GET(TechnoClass*, pThis, ESI);

	if (auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType()))
	{
		if (pTypeExt->Passengers_ChangeOwnerWithTransport && pThis->Passengers.NumPassengers > 0)
		{
			FootClass* pPassenger = pThis->Passengers.GetFirstPassenger();

			if (pPassenger)
				pPassenger->SetOwningHouse(pThis->Owner, false);

			while (pPassenger->NextObject)
			{
				pPassenger = static_cast<FootClass*>(pPassenger->NextObject);

				if (pPassenger)
					pPassenger->SetOwningHouse(pThis->Owner, false);
			}
		}
	}

	return 0;
}

DEFINE_HOOK(0x4F4583, Techno_Run_HugeHP, 0x6)
{
	//this hook borrow from phobos.cpp.
	//phobos.cpp use this hook show develop warning
	TechnoExt::RunHugeHP();
	return 0;
}

DEFINE_HOOK(0x6FDD50, Techno_Before_Fire, 0x6)
{
	GET(TechnoClass*, pThis, ECX);
	TechnoExt::AddFireScript(pThis);
	if (pThis->Target == nullptr)
		return 0;
	WeaponTypeClass* pWeapon = pThis->GetWeapon(pThis->SelectWeapon(pThis->Target))->WeaponType;
	TechnoExt::RunBlinkWeapon(pThis, pThis->Target, pWeapon);
	TechnoExt::IonCannonWeapon(pThis, pThis->Target, pWeapon);
	TechnoExt::BeamCannon(pThis, pThis->Target, pWeapon);
	TechnoExt::FirePassenger(pThis, pThis->Target, pWeapon);
	TechnoExt::AllowPassengerToFire(pThis, pThis->Target, pWeapon);
	TechnoExt::SpawneLoseTarget(pThis);
	return 0;
}

#pragma warning(pop) 