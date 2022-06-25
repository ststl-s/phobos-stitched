#include <InfantryClass.h>
#include <ScenarioClass.h>
#include <SpawnManagerClass.h>

#include "Body.h"

#include <Ext/TechnoType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Utilities/EnumFunctions.h>

inline void Func_LV5_1(TechnoClass* pThis, TechnoTypeClass* pType, TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt)
{
	TechnoExt::ApplyInterceptor(pThis, pExt, pTypeExt);
	TechnoExt::UpdateFireScript(pThis, pExt, pTypeExt);
	TechnoExt::EatPassengers(pThis, pExt, pTypeExt);
	TechnoExt::MovePassengerToSpawn(pThis, pTypeExt);
	TechnoExt::CheckIonCannonConditions(pThis, pExt, pTypeExt);
}

inline void Func_LV4_1(TechnoClass* pThis, TechnoTypeClass* pType, TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt)
{
	TechnoExt::SilentPassenger(pThis, pExt, pTypeExt);
	TechnoExt::Spawner_SameLoseTarget(pThis, pExt, pTypeExt);
	TechnoExt::ApplyPowered_KillSpawns(pThis, pTypeExt);
	TechnoExt::ApplySpawn_LimitRange(pThis, pTypeExt);
	TechnoExt::ApplyMindControlRangeLimit(pThis, pTypeExt);
}

inline void Func_LV4_2(TechnoClass* pThis, TechnoTypeClass* pType, TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt)
{
	// LaserTrails update routine is in TechnoClass::AI hook because TechnoClass::Draw
		// doesn't run when the object is off-screen which leads to visual bugs - Kerbiter
	for (auto const& trail : pExt->LaserTrails)
		trail->Update(TechnoExt::GetFLHAbsoluteCoords(pThis, trail->FLH, trail->IsOnTurret));
	TechnoExt::CheckDeathConditions(pThis, pExt, pTypeExt);
	if (pTypeExt->IsExtendGattling && !pType->IsGattling)
	{
		TechnoExt::SelectGattlingWeapon(pThis, pExt, pTypeExt);
		TechnoExt::TechnoGattlingCount(pThis, pExt, pTypeExt);
		TechnoExt::ResetGattlingCount(pThis, pExt, pTypeExt);
		TechnoExt::SetWeaponIndex(pThis, pExt);
	}
	TechnoExt::RunFireSelf(pThis, pExt, pTypeExt);
}

DEFINE_HOOK(0x6F9E50, TechnoClass_AI, 0x5)
{
	GET(TechnoClass*, pThis, ECX);
	TechnoTypeClass* pType = pThis->GetTechnoType();
	
	if (pType == nullptr)
		return 0;

	auto pExt = TechnoExt::ExtMap.Find(pThis);
	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

	if (pTypeExt->LV5_1)
		Func_LV5_1(pThis, pType, pExt, pTypeExt);

	if (pTypeExt->LV4_1)
		Func_LV4_1(pThis, pType, pExt, pTypeExt);

	if (pTypeExt->LV4_2)
		Func_LV4_2(pThis, pType, pExt, pTypeExt);

	if (pThis->IsMindControlled())
		TechnoExt::UpdateMindControlAnim(pThis, pExt);
	else if (pExt->MindControlRingAnimType != nullptr)
		pExt->MindControlRingAnimType = nullptr;
	
	if(pExt->setIonCannonType.isset())
		TechnoExt::RunIonCannonWeapon(pThis, pExt);
	
	if (pExt->setBeamCannon != nullptr)
		TechnoExt::RunBeamCannon(pThis, pExt);
	
	if (pExt->ConvertsOriginalType != pType)
		TechnoExt::ConvertsRecover(pThis, pExt);
	
	TechnoExt::CheckJJConvertConditions(pThis, pExt);
	TechnoExt::IsInROF(pThis, pExt);
	TechnoExt::ChangePassengersList(pThis, pExt);
	TechnoExt::DisableTurn(pThis, pExt);
	TechnoExt::CheckPaintConditions(pThis, pExt);
	TechnoExt::WeaponFacingTarget(pThis);
	TechnoExt::InfantryConverts(pThis, pTypeExt);
	TechnoExt::InitializeBuild(pThis, pExt, pTypeExt);
	TechnoExt::OccupantsWeaponChange(pThis, pExt);
	TechnoExt::OccupantsVeteranWeapon(pThis);

	if (!pTypeExt->AttackedWeapon.empty())
		TechnoExt::AttackedWeaponTimer(pExt);

	if (!pType->IsGattling && !pTypeExt->IsExtendGattling && !pType->IsChargeTurret)
		TechnoExt::VeteranWeapon(pThis, pExt, pTypeExt);
	
	return 0;
}

// YRDynamicPatcher-Kratos-0.7\DynamicPatcher\ExtensionHooks\TechnoExt.cs
DEFINE_HOOK(0x7063FF, TechnoClass_DrawSHP_Colour, 0x7)
{
	GET(TechnoClass*, pThis, ESI);

	auto pExt = TechnoExt::ExtMap.Find(pThis);

	if (pThis && pExt && pExt->AllowToPaint && pThis->WhatAmI() != AbstractType::Building)
	{
		auto Color = Drawing::RGB2DWORD(pExt->ColorToPaint);
		R->EAX(Color);
	}

	return 0;
}

// Ares-Version0A\src\Misc\Placeholders.cpp
DEFINE_HOOK(0x43D52D, BuildingClass_Draw_Tint, 0x5)
{
	GET(BuildingClass*, pThis, ESI);

	auto pExt = TechnoExt::ExtMap.Find(pThis);

	if (pThis && pExt && pExt->AllowToPaint)
	{
		auto Color = Drawing::RGB2DWORD(pExt->ColorToPaint);
		R->EDI(Color);
	}

	return 0;
}

// YRDynamicPatcher-Kratos-0.7\DynamicPatcher\ExtensionHooks\TechnoExt.cs
// Ares-Version0A\src\Misc\Placeholders.cpp
DEFINE_HOOK(0x73C15F, TechnoClass_DrawVXL_Tint, 0x7)
{
	GET(UnitClass*, pUnit, EBP);

	if (auto pThis = abstract_cast<TechnoClass*>(pUnit))
	{
		auto pExt = TechnoExt::ExtMap.Find(pThis);

		if (pExt && pExt->AllowToPaint)
		{
			auto Color = Drawing::RGB2DWORD(pExt->ColorToPaint);
			R->ESI(Color);
		}
	}

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

	TechnoExt::DeleteTheBuild(pThis);
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

	TechnoExt::InitializeShield(pThis);
	TechnoExt::InitializeLaserTrails(pThis);
	TechnoExt::InitializeAttachments(pThis);
	TechnoExt::InitialShowHugeHP(pThis);
	TechnoExt::InitializeJJConvert(pThis);
	TechnoExt::InitializeAttackedWeaponTimer(pThis);

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
				Vector2D<double> range = pTypeExt->InitialStrength_Cloning.Get();
				double percentage = range.X >= range.Y ? range.X : 
					(ScenarioClass::Instance->Random.RandomRanged(static_cast<int>(range.X * 100), static_cast<int>(range.Y * 100)) / 100.0);
				int strength = int(pTypeUnit->Strength * percentage);
				
				if (strength <= 0)
					strength = 1;

				pFoot->Health = strength;
				pFoot->EstimatedHealth = strength;
			}
		}
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
	GET_STACK(WarheadTypeClass*, pWarhead, STACK_OFFS(0xC4, -0xC));

	if (auto pExt = WarheadTypeExt::ExtMap.Find(pWarhead))
	{
		if (pExt->DecloakDamagedTargets.Get(RulesExt::Global()->Warheads_DecloakDamagedTargets))
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
				pTechnoExt->IsLeggedCyborg = true;
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

			if (pTechnoExt && pInf->Type->Cyborg && pInf->Crawling == true && !pTechnoExt->IsLeggedCyborg)
				pTechnoExt->IsLeggedCyborg = true;
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


DEFINE_HOOK(0x70A4FB, TechnoClass_Draw_Pips_SelfHealGain, 0x5)
{
	enum { SkipGameDrawing = 0x70A6C0 };

	GET(TechnoClass*, pThis, ECX);
	GET_STACK(Point2D*, pLocation, STACK_OFFS(0x74, -0x4));
	GET_STACK(RectangleStruct*, pBounds, STACK_OFFS(0x74, -0xC));

	TechnoExt::DrawSelfHealPips(pThis, pLocation, pBounds);

	return SkipGameDrawing;
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
	TechnoExt::SetWeaponROF(pThis, pWeapon);
	TechnoExt::SetGattlingCount(pThis, pThis->Target, pWeapon);
	return 0;
}

DEFINE_HOOK(0x6F534E, TechnoClass_DrawExtras_Insignia, 0x5)
{
	enum { SkipGameCode = 0x6F5388 };

	GET(TechnoClass*, pThis, EBP);
	GET_STACK(Point2D*, pLocation, STACK_OFFS(0x98, -0x4));
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
	GET_STACK(int, weaponIndex, STACK_OFFS(0x8, -0x4));

	int result = 0;
	auto pWeapon = pThis->GetWeapon(weaponIndex)->WeaponType;

	if (pWeapon)
	{
		result = pWeapon->Range;
		auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

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

DEFINE_HOOK(0x457C90, BuildingClass_IronCuratin, 0x6)
{
	GET(BuildingClass*, pThis, ECX);
	GET_STACK(HouseClass*, pSource, 0x8);
	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	
	if (pTypeExt->IronCurtain_Affect.isset())
	{
		if (pTypeExt->IronCurtain_Affect == IronCurtainAffects::Kill)
			R->EAX(pThis->ReceiveDamage(&pThis->Health, 0, RulesClass::Instance->C4Warhead, nullptr, true, false, pSource));
		else if (pTypeExt->IronCurtain_Affect == IronCurtainAffects::NoAffect)
			R->EAX(DamageState::Unaffected);
		else
			return 0;

		return 0x457CDB;
	}
	return 0;
}

DEFINE_HOOK(0x4DEAEE, FootClass_IronCurtain, 0x6)
{
	GET(FootClass*, pThis, ECX);
	GET_STACK(HouseClass*, pSource, 0x0);
	TechnoTypeClass* pType = pThis->GetTechnoType();
	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
	IronCurtainAffects ironAffect = IronCurtainAffects::Affect;

	if (pThis->WhatAmI() != AbstractType::Infantry)
		pSource = R->Stack<HouseClass*>(0x18);

	if (pType->Organic || pThis->WhatAmI() == AbstractType::Infantry)
	{
		if (pTypeExt->IronCurtain_Affect.isset())
			ironAffect = pTypeExt->IronCurtain_Affect.Get();
		else
			ironAffect = RulesExt::Global()->IronCurtainToOrganic.Get();
	}
	else
	{
		if (pTypeExt->IronCurtain_Affect.isset())
			ironAffect = pTypeExt->IronCurtain_Affect.Get();
	}

	if (ironAffect == IronCurtainAffects::Kill)
	{
		R->EAX(pThis->ReceiveDamage(&pThis->Health, 0, RulesClass::Instance->C4Warhead, nullptr, true, false, pSource));
	}
	else if (ironAffect == IronCurtainAffects::Affect)
	{
		R->ESI(pThis);
		return 0x4DEB38;
	}

	return 0x4DEBA2;
}

DEFINE_HOOK(0x522600, InfantryClass_IronCurtain, 0x6)
{
	GET(InfantryClass*, pThis, ECX);
	GET_STACK(int, nDuration, 0x4);
	GET_STACK(HouseClass*, pSource, 0x8);
	GET_STACK(bool, ForceShield, 0xC);
	pThis->FootClass::IronCurtain(nDuration, pSource, ForceShield);
	return 0x522639;
}

#pragma warning(pop) 