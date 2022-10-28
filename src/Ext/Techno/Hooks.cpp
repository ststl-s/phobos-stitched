#include "Body.h"

#include <InfantryClass.h>
#include <ScenarioClass.h>
#include <SpawnManagerClass.h>
#include <AnimClass.h>
#include <GameStrings.h>

#include <Utilities/EnumFunctions.h>

#include <Ext/House/Body.h>
#include <Ext/HouseType/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <New/Type/TemperatureTypeClass.h>

#include <Misc/GScreenDisplay.h>
#include <Misc/GScreenCreate.h>
#include <Misc/FlyingStrings.h>

inline void Subset_1(TechnoClass* pThis, TechnoTypeClass* pType, TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt)
{
	pExt->EatPassengers();
	pExt->CheckIonCannonConditions();
	pExt->UpdateAttackedWeaponTimer();
	pExt->ShieldPowered();
	TechnoExt::MovePassengerToSpawn(pThis, pTypeExt);
	TechnoExt::CurePassengers(pThis, pExt, pTypeExt);
}

inline void Subset_2(TechnoClass* pThis, TechnoTypeClass* pType, TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt)
{
	pExt->SilentPassenger();
	pExt->ApplySpawnSameLoseTarget();
	pExt->ApplyMobileRefinery();

	if (pTypeExt->Spawner_LimitRange)
		pExt->ApplySpawnLimitRange();

	if (pTypeExt->VeteranAnim != nullptr || pTypeExt->EliteAnim != nullptr)
		pExt->TechnoUpgradeAnim();

	TechnoExt::ApplyMindControlRangeLimit(pThis, pTypeExt);
}

inline void Subset_3(TechnoClass* pThis, TechnoTypeClass* pType, TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt)
{
	// LaserTrails update routine is in TechnoClass::AI hook because TechnoClass::Draw
	// doesn't run when the object is off-screen which leads to visual bugs - Kerbiter
	for (auto const& trail : pExt->LaserTrails)
	{
		if (pThis->CloakState == CloakState::Cloaked && !trail->Type->CloakVisible)
			continue;

		CoordStruct trailLoc = TechnoExt::GetFLHAbsoluteCoords(pThis, trail->FLH, trail->IsOnTurret);
		if (pThis->CloakState == CloakState::Uncloaking && !trail->Type->CloakVisible)
			trail->LastLocation = trailLoc;
		else
			trail->Update(trailLoc);
	}

	if (pTypeExt->IsExtendGattling && !pType->IsGattling)
	{
		//TechnoExt::SelectGattlingWeapon(pThis, pExt, pTypeExt);
		TechnoExt::TechnoGattlingCount(pThis, pExt, pTypeExt);
		TechnoExt::ResetGattlingCount(pThis, pExt, pTypeExt);
		TechnoExt::SetWeaponIndex(pThis, pExt);
	}

	pExt->ProcessFireSelf();

	if (!TechnoExt::IsReallyAlive(pThis))
		return;

	pExt->TeamAffect();
	pExt->PoweredUnitDown();
	pExt->PoweredUnit();
	pExt->PassengerProduct();
}

DEFINE_HOOK(0x6F9E50, TechnoClass_AI, 0x5)
{
	GET(TechnoClass*, pThis, ECX);

	TechnoTypeClass* pType = pThis->GetTechnoType();

	if (pType == nullptr)
		return 0;

	auto pExt = TechnoExt::ExtMap.Find(pThis);
	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

	if (pExt->TypeExtData == nullptr)
		pExt->TypeExtData = TechnoTypeExt::ExtMap.Find(pType);
	else if (pExt->TypeExtData->OwnerObject() != pType)
		pExt->UpdateTypeData(pType);

	if (pExt->CheckDeathConditions())
		return 0;

	pExt->UpdateShield();

	if (!TechnoExt::IsReallyAlive(pThis))
		return 0;

	pExt->CheckAttachEffects();

	if (!TechnoExt::IsReallyAlive(pThis))
		return 0;

	pExt->ProcessMoveDamage();

	if (!TechnoExt::IsReallyAlive(pThis))
		return 0;

	pExt->ProcessStopDamage();

	if (!TechnoExt::IsReallyAlive(pThis))
		return 0;

	pExt->IsInROF();
	pExt->CheckPaintConditions();
	pExt->InfantryConverts();
	pExt->RecalculateROT();
	pExt->ChangePassengersList();
	pExt->CheckJJConvertConditions();
	pExt->OccupantsWeaponChange();
	pExt->ApplyInterceptor();
//	pExt->OccupantsVeteranWeapon();
	pExt->UpdateDodge();
	pExt->ForgetFirer();
	pExt->UpdateDamageLimit();
	pExt->ShareWeaponRangeRecover();
	pExt->ShareWeaponRangeFire();
	pExt->CheckParachuted();

	pExt->IsInTunnel = false;

	if (pExt->AttachedGiftBox != nullptr)
	{
		pExt->AttachedGiftBox->AI();

		if (!TechnoExt::IsReallyAlive(pThis))
			return 0;
	}

	if (pExt->ConvertsOriginalType != pType)
		pExt->ConvertsRecover();

	if (pTypeExt->Subset_1)
		Subset_1(pThis, pType, pExt, pTypeExt);

	if (pTypeExt->Subset_2)
		Subset_2(pThis, pType, pExt, pTypeExt);

	if (!TechnoExt::IsReallyAlive(pThis))
		return 0;

	if (pTypeExt->Subset_3)
		Subset_3(pThis, pType, pExt, pTypeExt);

	if (pExt->setIonCannonType != nullptr)
		pExt->RunIonCannonWeapon();

	if (!TechnoExt::IsReallyAlive(pThis))
		return 0;

	if (pExt->setBeamCannon != nullptr)
		pExt->RunBeamCannon();

	if (!TechnoExt::IsReallyAlive(pThis))
		return 0;

	if (!pExt->Build_As_OnlyOne)
		TechnoExt::InitializeBuild(pThis, pExt, pTypeExt);

	TechnoExt::WeaponFacingTarget(pThis);
	TechnoExt::BuildingPassengerFix(pThis);
	TechnoExt::BuildingSpawnFix(pThis);
	TechnoExt::CheckTemperature(pThis);

	if (!pExt->InitialPayload && pThis->GetTechnoType()->Passengers > 0)
	{
		TechnoExt::PassengerFixed(pThis);
		TechnoExt::InitialPayloadFixed(pThis, pTypeExt);

		pExt->InitialPayload = true;
	}

	//if (!pTypeExt->IsExtendGattling && !pType->IsGattling && pType->Gunner)
	//	TechnoExt::SelectIFVWeapon(pThis, pExt, pTypeExt);

	if (!pExt->IsConverted && pThis->Passengers.NumPassengers > 0)
	{
		if (!pExt->ConvertPassenger)
			pExt->ConvertPassenger = pThis->Passengers.GetFirstPassenger();

		TechnoExt::CheckPassenger(pThis, pType, pExt, pTypeExt);

		pExt->IsConverted = true;
	}

	if (pExt->IsConverted && pThis->Passengers.NumPassengers <= 0)
	{
		TechnoExt::UnitConvert(pThis, pExt->OrignType, pExt->ConvertPassenger);

		pExt->IsConverted = false;
		pExt->ConvertPassenger = nullptr;
	}

	if (pExt->DelayedFire_Anim && !pThis->Target && pThis->GetCurrentMission() != Mission::Attack)
	{
		CDTimerClass* weaponReadyness = (CDTimerClass*)pThis->__DiskLaserTimer;
		weaponReadyness->Start(pThis->DiskLaserTimer.GetTimeLeft() + 5);

		// Reset Delayed fire animation
		pExt->DelayedFire_Anim = nullptr;
		pExt->DelayedFire_Anim_LoopCount = 0;
		pExt->DelayedFire_DurationTimer = -1;
	}

	return 0;
}

DEFINE_HOOK(0x51BAC7, InfantryClass_AI_Tunnel, 0x6)
{
	GET(InfantryClass*, pThis, ESI);

	auto pExt = TechnoExt::ExtMap.Find(pThis);
	pExt->UpdateOnTunnelEnter();

	return 0;
}

DEFINE_HOOK(0x7363B5, UnitClass_AI_Tunnel, 0x6)
{
	GET(UnitClass*, pThis, ESI);

	auto pExt = TechnoExt::ExtMap.Find(pThis);
	pExt->UpdateOnTunnelEnter();

	return 0;
}

DEFINE_HOOK(0x6F42F7, TechnoClass_Init_NewEntities, 0x2)
{
	GET(TechnoClass*, pThis, ESI);

	TechnoTypeClass* pType = pThis->GetTechnoType();
	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

	if (pThis->GetTechnoType() == nullptr || pTypeExt == nullptr)
		return 0;

	auto pExt = TechnoExt::ExtMap.Find(pThis);

	TechnoExt::FixManagers(pThis);

	for (size_t i = 0; i < TemperatureTypeClass::Array.size(); i++)
	{
		pExt->Temperature.emplace(i, pTypeExt->Temperature[i]);
		pExt->Temperature_HeatUpTimer.emplace(
			i,
			CDTimerClass(pTypeExt->Temperature_HeatUpFrame.count(i)
			? pTypeExt->Temperature_HeatUpFrame[i]
			: TemperatureTypeClass::Array[i]->HeatUp_Frame));
	}

	if (pTypeExt->PassengerProduct)
		pExt->PassengerProduct_Timer = pTypeExt->PassengerProduct_Rate;

	TechnoExt::InitializeShield(pThis);
	TechnoExt::InitializeLaserTrails(pThis);
	TechnoExt::InitializeAttachments(pThis);
	TechnoExt::InitializeHugeBar(pThis);
	TechnoExt::InitializeJJConvert(pThis);
	TechnoExt::InitialConvert(pThis, pExt, pTypeExt);

	return 0;
}

// Techno removed permanently
//DEFINE_HOOK(0x702050, TechnoClass_Destroyed, 0x6)
//{
//	GET(TechnoClass*, pThis, ESI);
//
//
//	return 0;
//}

DEFINE_HOOK(0x702E9D, TechnoClass_RegisterDestruction, 0x6)
{
	GET(TechnoClass*, pVictim, ESI);
	GET(TechnoClass*, pKiller, EDI);
	GET(int, cost, EBP);

	double giveExpMultiple = 1.0;
	double gainExpMultiple = 1.0;

	if(auto pVictimTypeExt = TechnoTypeExt::ExtMap.Find(pVictim->GetTechnoType()))
	{
		giveExpMultiple = pVictimTypeExt->Experience_VictimMultiple;
	}

	if (auto pKillerTypeExt = TechnoTypeExt::ExtMap.Find(pKiller->GetTechnoType()))
	{
		gainExpMultiple = pKillerTypeExt->Experience_KillerMultiple;
	}

	int finalCost = Game::F2I(cost * giveExpMultiple * gainExpMultiple);

	R->EBP(finalCost);

	return 0;
}

// Techno removed permanently
//DEFINE_HOOK(0x5F65F0, ObjectClass_Uninit, 0x6)
//{
//	GET(ObjectClass*, pThis, ECX);
//	TechnoClass* pTechno = abstract_cast<TechnoClass*>(pThis);
//
//	if (pTechno == nullptr)
//		return 0;
//
//	return 0;
//}

DEFINE_HOOK(0x6F6B1C, TechnoClass_Limbo, 0x6)
{
	GET(TechnoClass*, pThis, ESI);

	TechnoExt::LimboAttachments(pThis);

	return 0;
}

DEFINE_HOOK(0x6F6F20, TechnoClass_Unlimbo, 0x6)
{
	GET(TechnoClass*, pThis, ESI);

	TechnoExt::InitializeHugeBar(pThis);
	TechnoExt::UnlimboAttachments(pThis);

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

DEFINE_HOOK(0x443C81, BuildingClass_ExitObject_InitialClonedHealth, 0x7)
{
	GET(BuildingClass*, pBuilding, ESI);
	GET(FootClass*, pFoot, EDI);

	bool isCloner = false;

	if (pBuilding && pBuilding->Type->Cloning)
		isCloner = true;

	if (isCloner && pFoot)
	{
		if (auto pTypeExt = TechnoTypeExt::ExtMap.Find(pBuilding->GetTechnoType()))
		{
			if (auto pTypeUnit = pFoot->GetTechnoType())
			{
				double percentage = GeneralUtils::GetRangedRandomOrSingleValue(pTypeExt->InitialStrength_Cloning);
				int strength = static_cast<int>(pTypeUnit->Strength * percentage);

				if (strength <= 0)
					strength = 1;

				pFoot->Health = strength;
				pFoot->EstimatedHealth = strength;
			}
		}
	}

	return 0;
}

DEFINE_HOOK(0x6FD0B5, TechnoClass_RearmDelay_RandomDelay, 0x6)
{
	GET(WeaponTypeClass*, pWeapon, EDI);

	const auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);
	auto range = pWeaponExt->ROF_RandomDelay.Get(RulesExt::Global()->ROF_RandomDelay);

	R->EAX(GeneralUtils::GetRangedRandomOrSingleValue(range));
	return 0;
}

// Issue #271: Separate burst delay for weapon type
// Author: Starkku
DEFINE_HOOK(0x6FD05E, TechnoClass_RearmDelay_BurstDelays, 0x7)
{
	GET(TechnoClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EDI);

	const auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);
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
	GET_STACK(int, weaponIndex, STACK_OFFSET(0xD8, 0x8));
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
	GET_STACK(int, weaponIndex, STACK_OFFSET(0xD8, 0x8));
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
	REF_STACK(args_ReceiveDamage const, receiveDamageArgs, STACK_OFFSET(0xD0, 0x4));

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

// Author: Otamaa
DEFINE_HOOK(0x5223B3, InfantryClass_Approach_Target_DeployFireWeapon, 0x6)
{
	GET(InfantryClass*, pThis, ESI);
	R->EDI(pThis->Type->DeployFireWeapon == -1 ? pThis->SelectWeapon(pThis->Target) : pThis->Type->DeployFireWeapon);
	return 0x5223B9;
}

// Customizable OpenTopped Properties
// Author: Otamaa
DEFINE_HOOK(0x6F72D2, TechnoClass_IsCloseEnoughToTarget_OpenTopped_RangeBonus, 0xC)
{
	GET(TechnoClass* const, pThis, ESI);

	if (auto pTransport = pThis->Transporter)
	{
		if (auto pTypeExt = TechnoTypeExt::ExtMap.Find(pTransport->GetTechnoType()))
		{
			R->EAX(pTypeExt->OpenTopped_RangeBonus.Get(RulesClass::Instance->OpenToppedRangeBonus));
			return 0x6F72DE;
		}
	}

	return 0;
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

DEFINE_HOOK(0x702819, TechnoClass_ReceiveDamage_Decloak, 0xA)
{
	GET(TechnoClass* const, pThis, ESI);
	GET_STACK(WarheadTypeClass*, pWarhead, STACK_OFFSET(0xC4, 0xC));

	if (auto pExt = WarheadTypeExt::ExtMap.Find(pWarhead))
	{
		if (pExt->DecloakDamagedTargets.Get(RulesExt::Global()->Warheads_DecloakDamagedTargets))
			pThis->Uncloak(false);
	}

	return 0x702823;
}

DEFINE_HOOK(0x71067B, TechnoClass_EnterTransport_LaserTrails, 0x7)
{
	GET(TechnoClass*, pTechno, EDI);

	auto pTechnoExt = TechnoExt::ExtMap.Find(pTechno);

	if (pTechnoExt)
	{
		for (auto& pLaserTrail : pTechnoExt->LaserTrails)
		{
			pLaserTrail->Visible = false;
			pLaserTrail->LastLocation = { };
		}

		if (pTechno->WhatAmI() == AbstractType::Infantry)
		{
			auto const pInf = abstract_cast<InfantryClass*>(pTechno);
			auto const pInfType = abstract_cast<InfantryTypeClass*>(pTechno->GetTechnoType());

			if (pInfType->Cyborg && pInf->Crawling)
				pTechnoExt->IsLeggedCyborg = true;
		}

		if (pTechno->Transporter)
		{
			if (auto const pTransportTypeExt = TechnoTypeExt::ExtMap.Find(pTechno->Transporter->GetTechnoType()))
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
	GET(DamageState, eDamageState, EAX);

	if (pInf && eDamageState != DamageState::PostMortem)
	{
		if (pInf->Type->Cyborg)
		{
			auto pTechnoExt = TechnoExt::ExtMap.Find(pInf);

			if (pTechnoExt && pInf->Type->Cyborg && pInf->Crawling == true && !pTechnoExt->IsLeggedCyborg)
				pTechnoExt->IsLeggedCyborg = true;
		}
	}

	return 0;
}

DEFINE_HOOK(0x6F6CFE, TechnoClass_Unlimbo_LaserTrails, 0x6)
{
	GET(TechnoClass*, pTechno, ESI);

	if (auto pTechnoExt = TechnoExt::ExtMap.Find(pTechno))
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

// Update ammo rounds
DEFINE_HOOK(0x6FB086, TechnoClass_Reload_ReloadAmount, 0x8)
{
	GET(TechnoClass* const, pThis, ECX);

	TechnoExt::UpdateSharedAmmo(pThis);

	return 0;
}

DEFINE_HOOK(0x6FA793, TechnoClass_AI_SelfHealGain, 0x5)
{
	enum { SkipGameSelfHeal = 0x6FA941 };

	GET(TechnoClass*, pThis, ESI);

	TechnoExt::ApplyGainedSelfHeal(pThis);

	return SkipGameSelfHeal;
}

DEFINE_HOOK(0x6FD446, TechnoClass_LaserZap_IsSingleColor, 0x7)
{
	GET(WeaponTypeClass* const, pWeapon, ECX);
	GET(LaserDrawClass* const, pLaser, EAX);

	if (auto const pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon))
	{
		if (!pLaser->IsHouseColor && pWeaponExt->Laser_IsSingleColor)
			pLaser->IsHouseColor = true;
	}

	// Fixes drawing thick lasers for non-PrismSupport building-fired lasers.
	pLaser->IsSupported = pLaser->Thickness > 3;

	return 0;
}

DEFINE_HOOK(0x4F4583, Techno_HugeBar, 0x6)
{
	GScreenDisplay::UpdateAll();
	GScreenCreate::UpdateAll();
	RulesExt::RunAnim();
	TechnoExt::ProcessHugeBar();
	FlyingStrings::UpdateAllElectric();

	return 0;
}

DEFINE_HOOK(0x6F534E, TechnoClass_DrawExtras_Insignia, 0x5)
{
	enum { SkipGameCode = 0x6F5388 };

	GET(TechnoClass*, pThis, EBP);
	GET_STACK(Point2D*, pLocation, STACK_OFFSET(0x98, 0x4));
	GET(RectangleStruct*, pBounds, ESI);

	if (pThis->VisualCharacter(false, nullptr) != VisualType::Hidden)
		TechnoExt::DrawInsignia(pThis, pLocation, pBounds);

	return SkipGameCode;
}

DEFINE_HOOK(0x70EFE0, TechnoClass_GetMaxSpeed, 0x6)
{
	enum { SkipGameCode = 0x70EFF2 };

	GET(TechnoClass*, pThis, ECX);

	int maxSpeed = 0;

	if (pThis)
	{
		maxSpeed = pThis->GetTechnoType()->Speed;

		auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

		if (pTypeExt->UseDisguiseMovementSpeed && pThis->IsDisguised())
		{
			if (auto const pType = TechnoTypeExt::GetTechnoType(pThis->Disguise))
				maxSpeed = pType->Speed;
		}
	}

	R->EAX(maxSpeed);
	return SkipGameCode;
}

// Reimplements the game function with few changes / optimizations
DEFINE_HOOK(0x7012C2, TechnoClass_WeaponRange, 0x8)
{
	enum { ReturnResult = 0x70138F };

	GET(TechnoClass*, pThis, ECX);
	GET_STACK(int, weaponIndex, STACK_OFFSET(0x8, 0x4));

	int result = 0;
	auto pWeapon = pThis->GetWeapon(weaponIndex)->WeaponType;

	if (pWeapon)
	{
		result = pWeapon->Range;
		auto pExt = TechnoExt::ExtMap.Find(pThis);
		auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
		double dblRangeMultiplier = 1.0;

		for (const auto& pAE : pExt->AttachEffects)
		{
			if (!pAE->IsActive())
				continue;

			result += pAE->Type->Range;
			dblRangeMultiplier *= pAE->Type->Range_Multiplier;
		}

		result = Game::F2I(dblRangeMultiplier * result);

		if (pThis->GetTechnoType()->OpenTopped && !pTypeExt->OpenTopped_IgnoreRangefinding)
		{
			int smallestRange = INT32_MAX;
			auto pPassenger = pThis->Passengers.FirstPassenger;

			while (pPassenger && (pPassenger->AbstractFlags & AbstractFlags::Foot) != AbstractFlags::None)
			{
				int openTWeaponIndex = pPassenger->GetTechnoType()->OpenTransportWeapon;
				int tWeaponIndex = 0;

				if (openTWeaponIndex != -1)
					tWeaponIndex = openTWeaponIndex;
				else if (pPassenger->GetTechnoType()->TurretCount > 0)
					tWeaponIndex = pPassenger->CurrentWeaponNumber;

				WeaponTypeClass* pTWeapon = pPassenger->GetWeapon(tWeaponIndex)->WeaponType;

				if (pTWeapon)
				{
					if (pTWeapon->Range < smallestRange)
						smallestRange = pTWeapon->Range;
				}

				pPassenger = abstract_cast<FootClass*>(pPassenger->NextObject);
			}

			if (result > smallestRange)
				result = smallestRange;
		}
	}

	R->EBX(result);
	return ReturnResult;
}

// Basically a hack to make game and Ares pick laser properties from non-Primary weapons.
DEFINE_HOOK(0x70E1A5, TechnoClass_GetTurretWeapon_LaserWeapon, 0x6)
{
	enum { ReturnResult = 0x70E1C7, Continue = 0x70E1AB };

	GET(TechnoClass* const, pThis, ESI);

	if (pThis->WhatAmI() == AbstractType::Building)
	{
		if (auto const pExt = TechnoExt::ExtMap.Find(pThis))
		{
			if (!pExt->CurrentLaserWeaponIndex.empty())
			{
				auto weaponStruct = pThis->GetWeapon(pExt->CurrentLaserWeaponIndex.get());
				R->EAX(weaponStruct);
				return ReturnResult;
			}
		}
	}

	// Restore overridden instructions.
	R->EAX(pThis->GetTechnoType());
	return Continue;
}

DEFINE_HOOK(0x457C90, BuildingClass_IronCuratin, 0x6)
{
	GET(BuildingClass*, pThis, ECX);
	GET_STACK(HouseClass*, pSource, 0x8);

	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (pTypeExt->IronCurtain_Effect.isset())
	{
		switch (pTypeExt->IronCurtain_Effect)
		{
		case IronCurtainEffect::Kill:
		{
			R->EAX(pThis->TakeDamage(pThis->Health, pSource));

			return 0x457CDB;
		}break;
		case IronCurtainEffect::Ignore:
		{
			R->EAX(DamageState::Unaffected);

			return 0x457CDB;
		}break;
		default:
			break;
		}
	}

	return 0;
}

DEFINE_HOOK(0x4DEAEE, FootClass_IronCurtain, 0x6)
{
	GET(FootClass*, pThis, ECX);
	GET(TechnoTypeClass*, pType, EAX);
	GET_STACK(HouseClass*, pSource, STACK_OFFSET(0x10, 0x8));

	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
	IronCurtainEffect ironAffect;
	bool organic = pType->Organic || pThis->WhatAmI() == AbstractType::Infantry;

	if (organic)
	{
		ironAffect = pTypeExt->IronCurtain_Effect.Get(RulesExt::Global()->IronCurtain_EffectOnOrganics);
	}
	else
	{
		ironAffect = pTypeExt->IronCurtain_Effect.Get(IronCurtainEffect::Invulnerable);
	}

	switch (ironAffect)
	{
	case IronCurtainEffect::Invulnerable:
	{
		R->ESI(pThis);

		return 0x4DEB38;
	}break;
	case IronCurtainEffect::Ignore:
	{
		R->EAX(DamageState::Unaffected);
	}break;
	default:
	{
		R->EAX
		(
			pThis->TakeDamage
			(
				pThis->Health,
				pSource,
				nullptr,
				(organic
				? pTypeExt->IronCuratin_KillWarhead.Get(RulesExt::Global()->IronCurtain_KillOrganicsWarhead.Get(RulesClass::Instance->C4Warhead))
				: RulesClass::Instance->C4Warhead)
			)
		);
	}break;
	}

	return 0x4DEBA2;
}

DEFINE_HOOK(0x522600, InfantryClass_IronCurtain, 0x6)
{
	GET(InfantryClass*, pThis, ECX);
	GET_STACK(int, nDuration, 0x4);
	GET_STACK(HouseClass*, pSource, 0x8);
	GET_STACK(bool, ForceShield, 0xC);

	R->EAX(pThis->FootClass::IronCurtain(nDuration, pSource, ForceShield));

	return 0x522639;
}

DEFINE_HOOK(0x6B0B9C, SlaveManagerClass_Killed_DecideOwner, 0x6)
{
	enum { KillTheSlave = 0x6B0BDF, ChangeSlaveOwner = 0x6B0BB4 };

	GET(InfantryClass*, pSlave, ESI);

	if (const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pSlave->Type))
	{
		switch (pTypeExt->Slaved_OwnerWhenMasterKilled.Get())
		{
		case SlaveChangeOwnerType::Suicide:
			return KillTheSlave;

		case SlaveChangeOwnerType::Master:
			R->EAX(pSlave->Owner);
			return ChangeSlaveOwner;

		case SlaveChangeOwnerType::Neutral:
			if (auto pNeutral = HouseClass::FindNeutral())
			{
				R->EAX(pNeutral);
				return ChangeSlaveOwner;
			}

		default: // SlaveChangeOwnerType::Killer
			return 0x0;
		}
	}

	return 0x0;
}

DEFINE_HOOK(0x70A4FB, TechnoClass_Draw_Pips_SelfHealGain, 0x5)
{
	enum { SkipGameDrawing = 0x70A6C0 };

	GET(TechnoClass*, pThis, ECX);
	GET_STACK(Point2D*, pLocation, STACK_OFFSET(0x74, 0x4));
	GET_STACK(RectangleStruct*, pBounds, STACK_OFFSET(0x74, 0xC));

	TechnoExt::DrawSelfHealPips(pThis, pLocation, pBounds);

	return SkipGameDrawing;
}

// SellSound and EVA dehardcode
DEFINE_HOOK(0x449CC1, BuildingClass_Mission_Deconstruction_EVA_Sold_1, 0x6)
{
	enum { SkipVoxPlay = 0x449CEA };
	GET(BuildingClass*, pThis, EBP);

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);
	if (pTypeExt->EVA_Sold.isset())
	{
		if (pThis->IsOwnedByCurrentPlayer && !pThis->Type->UndeploysInto)
			VoxClass::PlayIndex(pTypeExt->EVA_Sold.Get());

		return SkipVoxPlay;
	}

	return 0x0;
}

DEFINE_HOOK(0x44AB22, BuildingClass_Mission_Deconstruction_EVA_Sold_2, 0x6)
{
	enum { SkipVoxPlay = 0x44AB3B };
	GET(BuildingClass*, pThis, EBP);

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);
	if (pTypeExt->EVA_Sold.isset())
	{
		if (pThis->IsOwnedByCurrentPlayer)
			VoxClass::PlayIndex(pTypeExt->EVA_Sold.Get());

		return SkipVoxPlay;
	}

	return 0x0;
}

DEFINE_HOOK(0x44A850, BuildingClass_Mission_Deconstruction_Sellsound, 0x6)
{
	enum { PlayVocLocally = 0x44A856 };
	GET(BuildingClass*, pThis, EBP);

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);

	if (pTypeExt->SellSound.isset())
	{
		R->ECX(pTypeExt->SellSound.Get());
		return PlayVocLocally;
	}

	return 0x0;
}

DEFINE_HOOK(0x4D9F8A, FootClass_Sell_Sellsound, 0x5)
{
	enum { SkipVoxVocPlay = 0x4D9FB5 };
	GET(FootClass*, pThis, ESI);

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	VoxClass::PlayIndex(pTypeExt->EVA_Sold.Get(VoxClass::FindIndex(GameStrings::EVA_UnitSold)));
	//WW used VocClass::PlayGlobal to play the SellSound, why did they do that?
	VocClass::PlayAt(pTypeExt->SellSound.Get(RulesClass::Instance->SellSound), pThis->Location);

	return SkipVoxVocPlay;
}

DEFINE_HOOK_AGAIN(0x703789, TechnoClass_CloakUpdateMCAnim, 0x6) // TechnoClass_Do_Cloak
DEFINE_HOOK(0x6FB9D7, TechnoClass_CloakUpdateMCAnim, 0x6)       // TechnoClass_Cloaking_AI
{
	GET(TechnoClass*, pThis, ESI);

	TechnoExt::UpdateMindControlAnim(pThis);

	return 0;
}

namespace Aircraft_KickOutPassengers
{
	FootClass* pFoot = nullptr;
	bool NeedParachuteNow = true;
}

DEFINE_HOOK(0x415DE3, AircraftClass_KickOutPassengers_SpawnParachuted, 0x5)
{
	GET(FootClass*, pFoot, ESI);
	GET_STACK(CoordStruct, coords, STACK_OFFSET(0x2C, -0x10));
	const int height = coords.Z - MapClass::Instance->GetCellFloorHeight(coords);

	if (const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pFoot->GetTechnoType()))
	{
		const int parachuteHeight = pTypeExt->Parachute_OpenHeight.Get(
					HouseTypeExt::ExtMap.Find(pFoot->Owner->Type)->Parachute_OpenHeight.Get(RulesExt::Global()->Parachute_OpenHeight));

		if (parachuteHeight && height > parachuteHeight)
		{
			if (const auto pExt = TechnoExt::ExtMap.Find(pFoot))
			{
				pExt->NeedParachute_Height = parachuteHeight;
				Aircraft_KickOutPassengers::pFoot = pFoot;
				Aircraft_KickOutPassengers::NeedParachuteNow = false;
			}
		}
	}

	return 0;
}

DEFINE_HOOK(0x5F5A58, ObjectClass_SpawnParachuted, 0x5)
{
	enum { IsBullet = 0x5F5A62, NotBullet = 0x5F5A9D, SkipParachute = 0x5F5B36 };

	GET(ObjectClass*, pThis, ESI);

	if (const auto pFoot = abstract_cast<FootClass*>(pThis))
	{
		if (Aircraft_KickOutPassengers::pFoot == pFoot && !Aircraft_KickOutPassengers::NeedParachuteNow)
		{
			Aircraft_KickOutPassengers::pFoot = nullptr;
			Aircraft_KickOutPassengers::NeedParachuteNow = true;
			R->EDI(0);
			return SkipParachute;
		}

		return NotBullet;
	}
	else
		return IsBullet;
}

#pragma warning(pop)

DEFINE_HOOK(0x703A09, TechnoClass_VisualCharacter_ObserverCloak, 0x7)
{
	enum { UseShadowyVisual = 0x703A5A };

	GET(TechnoClass*, pThis, ESI);

	// Allow observers to always see cloaked objects.
	if (HouseClass::IsCurrentPlayerObserver() && pThis->CloakState == CloakState::Cloaked)
		return UseShadowyVisual;

	return 0;
}
