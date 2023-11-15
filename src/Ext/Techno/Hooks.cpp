#include "Body.h"

#include <GameStrings.h>
#include <DriveLocomotionClass.h>

#include <Ext/BuildingType/Body.h>
#include <Ext/HouseType/Body.h>
#include <Ext/TerrainType/Body.h>

#include <Misc/FlyingStrings.h>
#include <Misc/GScreenCreate.h>
#include <Misc/GScreenDisplay.h>

#include <New/Type/TemperatureTypeClass.h>

#include <Utilities/Macro.h>
#include <Utilities/EnumFunctions.h>
#include <Utilities/GeneralUtils.h>

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
	pExt->TechnoAcademyReset();

	if (pTypeExt->Spawner_LimitRange)
		pExt->ApplySpawnLimitRange();

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
		TechnoExt::SelectGattlingWeapon(pThis, pExt, pTypeExt);
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
	pExt->PoweredUnitDeactivate();
	pExt->PassengerProduct();
}

DEFINE_HOOK(0x6F9E50, TechnoClass_AI, 0x5)
{
	GET(TechnoClass*, pThis, ECX);

	//enum { retn = 0x6FB004 };
	enum { retn = 0 };

	if (!TechnoExt::IsReallyAlive(pThis))
		return retn;

	TechnoTypeClass* pType = pThis->GetTechnoType();
	auto pExt = TechnoExt::ExtMap.Find(pThis);
	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

	if (pExt->TypeExtData == nullptr)
		pExt->TypeExtData = TechnoTypeExt::ExtMap.Find(pType);
	else if (pExt->TypeExtData->OwnerObject() != pType)
		pExt->UpdateTypeData(pType);

	if (pExt->CheckDeathConditions())
		return retn;

	pExt->UpdateShield();

	if (!TechnoExt::IsReallyAlive(pThis))
		return retn;

	pExt->CheckAttachEffects();
	pExt->DeployAttachEffect();
	pExt->AttachEffectNext();

	if (!TechnoExt::IsReallyAlive(pThis))
		return retn;

	pExt->AntiGravity();

	if (!TechnoExt::IsReallyAlive(pThis))
		return retn;

	pExt->CheckPaintConditions();
	pExt->InfantryConverts();
	pExt->RecalculateROT();
	pExt->CheckJJConvertConditions();
	pExt->OccupantsWeaponChange();
	pExt->ApplyInterceptor();
	pExt->ForgetFirer();
	pExt->UpdateDamageLimit();
	pExt->CheckParachuted();
	pExt->MoveConverts();
	pExt->MoveChangeLocomotor();
	pExt->DisableBeSelect();
	pExt->KeepGuard();
	pExt->TemporalTeamCheck();
	pExt->SetSyncDeathOwner();
	pExt->DeathWithSyncDeathOwner();
	pExt->PlayLandAnim();
	pExt->Aircraft_AreaGuard();
	pExt->BackwarpUpdate();
	pExt->Backwarp();
	pExt->UpdateStrafingLaser();

	pExt->IsInTunnel = false;
	
	if (pExt->AttachedGiftBox != nullptr)
	{
		pExt->AttachedGiftBox->AI(pTypeExt);

		if (!TechnoExt::IsReallyAlive(pThis))
			return retn;
	}

	if (pExt->ConvertsOriginalType != pType)
		pExt->ConvertsRecover();

	if (pTypeExt->Subset_1)
		Subset_1(pThis, pType, pExt, pTypeExt);

	if (pTypeExt->Subset_2)
		Subset_2(pThis, pType, pExt, pTypeExt);

	if (!TechnoExt::IsReallyAlive(pThis))
		return retn;

	if (pTypeExt->Subset_3)
		Subset_3(pThis, pType, pExt, pTypeExt);

	if (pExt->setIonCannonType != nullptr)
		pExt->RunIonCannonWeapon();

	if (!TechnoExt::IsReallyAlive(pThis))
		return retn;

	if (pExt->setBeamCannon != nullptr)
	{
		pExt->RunBeamCannon();
		pExt->BeamCannonLockFacing();
	}

	if (!TechnoExt::IsReallyAlive(pThis))
		return retn;

	// if (!pExt->Build_As_OnlyOne)
		// TechnoExt::InitializeBuild(pThis, pExt, pTypeExt);

	pExt->TechnoUpgradeAnim();

	if (pExt->ShareWeaponRangeTarget != nullptr)
	{
		pExt->ShareWeaponRangeTurn();
	}

	TechnoExt::WeaponFacingTarget(pThis);
	TechnoExt::BuildingPassengerFix(pThis);
	TechnoExt::BuildingSpawnFix(pThis);
	TechnoExt::CheckTemperature(pThis);
	TechnoExt::AttachmentsAirFix(pThis);
	TechnoExt::MoveTargetToChild(pThis);
	TechnoExt::FallRateFix(pThis);

	if (!TechnoExt::IsReallyAlive(pThis))
		return retn;

	if (!pTypeExt->IsExtendGattling && !pType->IsGattling && pType->Gunner)
		TechnoExt::SelectIFVWeapon(pThis, pExt, pTypeExt);

	TechnoExt::OccupantsWeapon(pThis, pExt);
	TechnoExt::BuildingWeaponChange(pThis, pExt, pTypeExt);

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

	if (pExt->Deployed)
	{
		if (!pExt->IsDeployed())
			pExt->Deployed = false;
	}

	if (!pTypeExt->SuperWeapon_Quick.empty())
	{
		TechnoExt::SelectSW(pThis, pTypeExt);
	}

	if (pExt->CurrentTarget != pThis->Target)
	{
		pExt->AircraftClass_SetTargetFix();
		pExt->CurrentTarget = pThis->Target;
	}

	if (pExt->FinishSW)
	{
		pExt->FinishSW = false;
		pThis->Guard();
	}

	if (pExt->WasFallenDown)
	{
		pExt->ShouldSinking();

		if (!TechnoExt::IsReallyAlive(pThis))
			return retn;
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
	pExt->TypeExtData = pTypeExt;
	pExt->ControlConverts();

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

	if (pExt->LastOwner == nullptr)
		pExt->LastOwner = pThis->Owner;

	TechnoExt::InitializeShield(pThis);
	TechnoExt::InitializeLaserTrails(pThis);
	TechnoExt::InitializeAttachments(pThis);
	TechnoExt::InitializeHugeBar(pThis);
	TechnoExt::InitializeJJConvert(pThis);
	TechnoExt::InitialConvert(pThis, pExt, pTypeExt);
	GiftBoxClass::InitializeGiftBox(pThis);

	if (pTypeExt->Message_Creat.isset())
	{
		if (EnumFunctions::CanTargetHouse(pTypeExt->Message_Creat_ShowHouses, pThis->Owner, HouseClass::CurrentPlayer))
		{
			MessageListClass::Instance->PrintMessage(pTypeExt->Message_Creat.Get().Text, RulesClass::Instance->MessageDelay, pThis->Owner->ColorSchemeIndex);
		}
	}

	pExt->TechnoAcademy();

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

	if (auto pKillerExt = TechnoExt::ExtMap.Find(pKiller))
	{
		if (pKillerExt->ParentAttachment)
		{
			int parentCost = static_cast<int>(pKillerExt->ParentAttachment->GetType()->Experience_ParentModifier * finalCost);
			pKillerExt->ParentAttachment->Parent->Veterancy.Add(parentCost);
			if (pKillerExt->ParentAttachment->Parent->Veterancy.IsElite())
			{
				pKillerExt->ParentAttachment->Parent->Veterancy.SetElite();
			}
		}

		for (auto const& pAttachment : pKillerExt->ChildAttachments)
		{
			int childCost = static_cast<int>(pAttachment->GetType()->Experience_ChildModifier * finalCost);
			pAttachment->Child->Veterancy.Add(childCost);
			if (pAttachment->Child->Veterancy.IsElite())
			{
				pAttachment->Child->Veterancy.SetElite();
			}
		}
	}

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

	auto pExt = TechnoExt::ExtMap.Find(pThis);

	if (!pExt->InitialPayload && pThis->GetTechnoType()->Passengers > 0)
	{
		TechnoExt::InitialPayloadFixed(pThis);
		TechnoExt::PassengerFixed(pThis);

		pExt->InitialPayload = true;
	}
	
	return 0;
}

DEFINE_HOOK(0x702E4E, TechnoClass_Save_Killer_Techno, 0x6)
{
	GET(TechnoClass*, pKiller, EDI);
	GET(TechnoClass*, pVictim, ECX);

	if (pKiller && pVictim)
		TechnoExt::ObjectKilledBy(pVictim, pKiller);

	if (pVictim)
	{

		if (const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pVictim->GetTechnoType()))
		{
			if (pTypeExt->Message_Death.isset())
			{
				if (EnumFunctions::CanTargetHouse(pTypeExt->Message_Death_ShowHouses, pVictim->Owner, HouseClass::CurrentPlayer))
				{
					MessageListClass::Instance->PrintMessage(pTypeExt->Message_Death.Get().Text, RulesClass::Instance->MessageDelay, pVictim->Owner->ColorSchemeIndex);
				}
			}
		}
	}

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

	if (auto const pInf = abstract_cast<InfantryClass*>(R->EDI<FootClass*>()))
	{
		if (pBuilding && pBuilding->Type->Cloning)
		{
			if (auto pTypeExt = BuildingTypeExt::ExtMap.Find(pBuilding->Type))
			{
				double percentage = GeneralUtils::GetRangedRandomOrSingleValue(pTypeExt->InitialStrength_Cloning);
				int strength = Math::clamp(static_cast<int>(pInf->Type->Strength * percentage), 1, pInf->Type->Strength);

				pInf->Health = strength;
				pInf->EstimatedHealth = strength;
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

namespace BurstFLHTemp
{
	bool FLHFound;
}

DEFINE_HOOK(0x6F3B37, TechnoClass_Transform_6F3AD0_BurstFLH_1, 0x7)
{
	GET(TechnoClass*, pThis, EBX);
	GET_STACK(int, weaponIndex, STACK_OFFSET(0xD8, 0x8));
	bool FLHFound = false;
	CoordStruct FLH = CoordStruct::Empty;

	FLH = TechnoExt::GetBurstFLH(pThis, weaponIndex, FLHFound);
	BurstFLHTemp::FLHFound = FLHFound;

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
	GET_STACK(int, weaponIndex, STACK_OFFSET(0xD8, 0x8));

	if (BurstFLHTemp::FLHFound || weaponIndex < 0)
		R->EAX(0);

	BurstFLHTemp::FLHFound = false;

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

DEFINE_HOOK(0x6FA735, TechnoClass_AI_ShouldSelfHealOneStep, 0x6)
{
	GET(TechnoClass*, pThis, ESI);

	auto pExt = TechnoExt::ExtMap.Find(pThis);

	for (const auto& pAE : pExt->AttachEffects)
	{
		if (!pAE->IsActive())
			continue;

		if (pAE->Type->ForbiddenSelfHeal)
			return 0x6FAFFD;
	}

	return TechnoExt::IsActive(pThis) && pThis->ShouldSelfHealOneStep() ? 0x6FA751 : 0x6FA793;
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
	GScreenDisplay::UpdateSW();
	GScreenCreate::UpdateAll();
	RulesExt::RunAnim();
	TechnoExt::ProcessHugeBar();

	return 0;
}

DEFINE_HOOK(0x6F534E, TechnoClass_DrawExtras_Insignia, 0x5)
{
	enum { SkipGameCode = 0x6F5388 };

	GET(TechnoClass*, pThis, EBP);
	GET_STACK(const Point2D*, pLocation, STACK_OFFSET(0x98, 0x4));
	GET(const RectangleStruct*, pBounds, ESI);

	if (pThis->VisualCharacter(false, nullptr) != VisualType::Hidden)
		TechnoExt::DrawInsignia(pThis, *pLocation, *pBounds);

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

// Fix slaves cannot always suicide due to armor multiplier or something
DEFINE_PATCH(0x6B0BF7,
	0x6A, 0x01  // push 1       // ignoreDefense=false->true
);

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

DEFINE_HOOK(0x5F6CD0, ObjectClass_IsCrushable, 0x6)
{
	enum { SkipGameCode = 0x5F6D3C };

	GET(ObjectClass*, pThis, ECX);
	GET_STACK(TechnoClass*, pTechno, STACK_OFFSET(0x8, -0x4));
	bool canCrush = false;

	if (pThis && pTechno && pThis != pTechno)
	{
		if (pThis->AbstractFlags & AbstractFlags::Techno)
		{
			if (pThis->AbstractFlags & AbstractFlags::Foot)
			{
				const auto pThisFoot = static_cast<FootClass*>(pThis);

				if (TechnoExt::IsReallyAlive(pThisFoot) && !pTechno->Owner->IsAlliedWith(pThisFoot) && !pThisFoot->IsIronCurtained())
				{
					const auto pExt = TechnoTypeExt::ExtMap.Find(pThisFoot->GetTechnoType());
					const auto pTechnoExt = TechnoTypeExt::ExtMap.Find(pTechno->GetTechnoType());
					const auto pInf = abstract_cast<InfantryClass*>(pThisFoot);
					const int crushableLevel = pInf && pInf->IsDeployed() ? pExt->DeployCrushableLevel.Get(pThisFoot) : pExt->CrushableLevel.Get(pThisFoot);

					canCrush = pTechnoExt->CrushLevel.Get(pTechno) > crushableLevel;
				}
			}
		}
		else if (const auto pThisTerrain = abstract_cast<TerrainClass*>(pThis))
		{
			if (!pThisTerrain->Type->SpawnsTiberium && !pThisTerrain->Type->Immune)
			{
				const auto pExt = TerrainTypeExt::ExtMap.Find(pThisTerrain->Type);
				const auto pTechnoExt = TechnoTypeExt::ExtMap.Find(pTechno->GetTechnoType());

				canCrush = pTechnoExt->CrushLevel.Get(pTechno) > pExt->CrushableLevel;
			}
		}
	}

	R->EAX(canCrush);

	return SkipGameCode;
}

namespace CrushTemp
{
	TechnoTypeExt::ExtData* UpdatePositionExt = nullptr;
	TechnoTypeExt::ExtData* Sub4B0F20Ext = nullptr;
}

DEFINE_HOOK(0x73B002, UnitClass_UpdatePosition_CrusherTerrain, 0x6)
{
	GET(UnitClass*, pThis, EBP);
	GET(CellClass*, pCell, EDI);

	TechnoTypeExt::ExtData* pTypeExt = nullptr;
	auto pObj = pCell->FirstObject;
	bool crush = false;

	while (pObj)
	{
		if (const auto pTerrain = abstract_cast<TerrainClass*>(pObj))
		{
			const auto pTType = pTerrain->Type;

			if (!pTType->SpawnsTiberium && !pTType->Immune)
			{
				if (!pTypeExt)
					pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);

				if (pTypeExt->CrushLevel.Get(pThis) > TerrainTypeExt::ExtMap.Find(pTerrain->Type)->CrushableLevel)
				{
					VocClass::PlayAt(pTType->CrushSound, pThis->Location);
					TerrainTypeExt::Remove(pTerrain);
					crush = true;
				}
			}
		}

		pObj = pObj->NextObject;
	}

	if (crush)
	{
		pThis->RockingForwardsPerFrame += 0.02f;
		pThis->unknown_bool_6B5 = false;
	}

	CrushTemp::UpdatePositionExt = pTypeExt;

	R->EAX(pCell->OverlayTypeIndex);
	return pCell->OverlayTypeIndex != -1 ? 0x73B00A : 0x73B074;
}

DEFINE_HOOK(0x4B1999, DriveLocomotionClass_4B0F20_CrusherTerrain, 0x6)
{
	GET(DriveLocomotionClass*, pLoco, EBP);
	GET(CellClass*, pCell, EBX);

	const auto pLinkedTo = pLoco->LinkedTo;

	if (!pLinkedTo->GetTechnoType()->TiltsWhenCrushes)
		return 0;

	TechnoTypeExt::ExtData* pFootExt = nullptr;
	const auto pTerrain = [pLinkedTo, &pFootExt](CellClass* pCell)->TerrainClass*
	{
		ObjectClass* pObject = pCell->FirstObject;

		while (pObject)
		{
			if (const auto pTerrain = abstract_cast<TerrainClass*>(pObject))
			{
				const auto pTType = pTerrain->Type;

				if (!pTType->SpawnsTiberium && !pTType->Immune)
				{
					if (!pFootExt)
						pFootExt = TechnoTypeExt::ExtMap.Find(pLinkedTo->GetTechnoType());

					if (pFootExt->CrushLevel.Get(pLinkedTo) > TerrainTypeExt::ExtMap.Find(pTType)->CrushableLevel)
						return pTerrain;
				}
			}

			pObject = pObject->NextObject;
		}

		return nullptr;
	}(pCell);

	if (pTerrain)
		pLinkedTo->RockingForwardsPerFrame = -0.05f;

	CrushTemp::Sub4B0F20Ext = pFootExt;

	R->EAX(pCell->OverlayTypeIndex);
	return pCell->OverlayTypeIndex != -1 ? 0x4B19A1 : 0x4B1A04;
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

DEFINE_HOOK(0x62A02F, ParasiteClass_AI, 0x6)
{
	enum { Continue = 0, SkipAfter = 0x62A015 };

	GET(ParasiteClass*, pPara, ESI);
	GET(FootClass*, pVictim, ECX);
	GET(WeaponTypeClass*, pWeapon, EDI);

	if (!MapClass::GetTotalDamage(100, pWeapon->Warhead, pVictim->GetType()->Armor, 0))
	{
		pPara->ExitUnit();
		return SkipAfter;
	}

	if (const auto pExt = TechnoExt::ExtMap.Find(pVictim))
	{
		if (const auto pShieldData = pExt->Shield.get())
		{
			if (pShieldData->IsActive() && !pShieldData->CanBeTargeted(pWeapon))
			{
				pPara->ExitUnit();
				return SkipAfter;
			}
		}
	}

	return 0;
}

DEFINE_HOOK(0x62A0D3, ParasiteClass_AI_ParticleSystem, 0x5)
{
	enum { SkipGameCode = 0x62A108 };

	GET(ParasiteClass*, pPara, ESI);
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pPara->Owner->GetTechnoType());

	if (!pTypeExt->Parasite_NoParticleSystem)
	{
		if (const auto pParticle = pTypeExt->Parasite_ParticleSystem.Get(RulesClass::Instance->DefaultSparkSystem))
			GameCreate<ParticleSystemClass>(pParticle, pPara->Victim->Location, nullptr, nullptr, CoordStruct::Empty, nullptr);
	}

	return SkipGameCode;
}

DEFINE_HOOK(0x62A20E, ParasiteClass_AI_RockVictim, 0x8)
{
	enum { Continue = 0, Skip = 0x62A222 };

	GET(ParasiteClass*, pPara, ESI);

	if (pPara->Victim->IsAttackedByLocomotor)
		return Skip;

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pPara->Owner->GetTechnoType());

	return pTypeExt->Parasite_NoRock ? Skip : Continue;
}

DEFINE_HOOK(0x62A240, ParasiteClass_AI_AttachEffect, 0x6)
{
	GET(ParasiteClass*, pPara, ESI);
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pPara->Owner->GetTechnoType());

	if (!pTypeExt->Parasite_AttachEffects.empty())
	{
		for (auto pAE : pTypeExt->Parasite_AttachEffects)
			TechnoExt::AttachEffect(pPara->Victim, pPara->Owner, pAE);
	}

	return 0;
}

DEFINE_HOOK(0x62A16A, ParasiteClass_AI_Anim, 0x5)
{
	GET(AnimClass*, pAnim, EAX);
	GET(ParasiteClass*, pPara, ESI);

	if (pAnim)
	{
		pAnim->SetOwnerObject(pPara->Victim);
		pAnim->Owner = pPara->Owner->GetOwningHouse();
	}

	return 0;
}

DEFINE_HOOK(0x62AB88, ParasiteClass_CanExistOnVictimCell_Terrain, 0x5)
{
	enum { SkipGameCode = 0x62ABA5, ReturnZero = 0x62ABA0 };

	GET(ParasiteClass*, pThis, ESI);

	const auto pTerrain = [](CellClass* pCell)->TerrainClass*
	{
		for (ObjectClass* pObject = pCell->FirstObject; pObject; pObject = pObject->NextObject)
		{
			const auto pTerrain = abstract_cast<TerrainClass*>(pObject);

			if (pTerrain && !TerrainTypeExt::ExtMap.Find(pTerrain->Type)->IsPassable)
				return pTerrain;
		}

		return nullptr;
	}(pThis->Victim->GetCell());

	return pTerrain ? ReturnZero : SkipGameCode;
}

DEFINE_HOOK(0x62ABB6, ParasiteClass_CanExistOnVictimCell_LandType, 0x5)
{
	enum { ContinueIn = 0x62ABC5, Skip = 0x62AC0F };

	GET(ParasiteClass*, pThis, ESI);
	GET(LandType, land, EAX);

	return GroundType::Array[static_cast<int>(land)].Cost[static_cast<int>(pThis->Owner->GetTechnoType()->SpeedType)] > 0.0 ? Skip : ContinueIn;
}

DEFINE_HOOK(0x703A09, TechnoClass_VisualCharacter_CloakVisibility, 0x7)
{
	enum { UseShadowyVisual = 0x703A5A, CheckMutualAlliance = 0x703A16 };

	// Allow observers to always see cloaked objects.
	// Skip IsCampaign check (confirmed being useless from Mental Omega mappers)
	if (HouseClass::IsCurrentPlayerObserver())
		return UseShadowyVisual;

	return CheckMutualAlliance;
}

DEFINE_HOOK(0x45455B, BuildingClass_VisualCharacter_CloakVisibility, 0x5)
{
	enum { UseShadowyVisual = 0x45452D, CheckMutualAlliance = 0x454564 };

	if (HouseClass::IsCurrentPlayerObserver())
		return UseShadowyVisual;

	return CheckMutualAlliance;
}

//建筑可建造范围钩子，作者：烈葱（NetsuNegi） -  我直接单推烈葱！！
DEFINE_HOOK(0x4A8FCC, MapClass_CanBuildingTypeBePlacedHere, 0x5)
{
	enum { Continue = 0x4A8FD1, CanPlaceHere = 0x4A902C };

	GET(CellClass*, pCell, ECX);
	GET_STACK(int, HouseIdx, STACK_OFFSET(0x30, 0x8));

	const auto pUnit = pCell->GetUnit(false) ? abstract_cast<TechnoClass*>(pCell->GetUnit(false)) :
		(pCell->Jumpjet) ? abstract_cast<TechnoClass*>(pCell->Jumpjet) : nullptr;

	if (pUnit != nullptr)
	{
		if (TechnoExt::CheckCanBuildUnitType(pUnit, HouseIdx))
		{
			R->Stack(STACK_OFFSET(0x30, 0xC), true);
			return CanPlaceHere;
		}
	}

	const auto pInf = pCell->GetInfantry(false) ? abstract_cast<TechnoClass*>(pCell->GetInfantry(false)) :
		(pCell->Jumpjet) ? abstract_cast<TechnoClass*>(pCell->Jumpjet) : nullptr;

	if (pInf != nullptr)
	{
		if (TechnoExt::CheckCanBuildUnitType(pInf, HouseIdx))
		{
			R->Stack(STACK_OFFSET(0x30, 0xC), true);
			return CanPlaceHere;
		}
	}

	R->EAX(pCell->GetBuilding());
	return Continue;
}

DEFINE_HOOK(0x6FA167, TechnoClass_AI_DrainMoney, 0x5)
{
	enum { SkipGameCode = 0x6FA1C5 };

	GET(TechnoClass*, pThis, ESI);
	const auto pSource = pThis->DrainingMe;
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pSource->GetTechnoType());
	auto pRules = RulesClass::Instance();

	if (Unsorted::CurrentFrame % pTypeExt->DrainMoneyFrameDelay.Get(pRules->DrainMoneyFrameDelay) == 0)
	{
		if (int amount = pTypeExt->DrainMoneyAmount.Get(pRules->DrainMoneyAmount))
		{
			if (amount > 0)
				amount = Math::min(amount, pThis->Owner->Available_Money());
			else
				amount = Math::max(amount, -pSource->Owner->Available_Money());

			if (amount)
			{
				pThis->Owner->TransactMoney(-amount);
				pSource->Owner->TransactMoney(amount);

				if (pTypeExt->DrainMoney_Display)
				{
					auto displayCoords = pTypeExt->DrainMoney_Display_AtFirer ? pSource->Location : pThis->Location;
					FlyingStrings::AddMoneyString(amount, pSource->Owner, pTypeExt->DrainMoney_Display_Houses, displayCoords, pTypeExt->DrainMoney_Display_Offset);
				}
			}
		}
	}

	return SkipGameCode;
}

DEFINE_HOOK(0x70FDF5, TechnoClass_DrawDrainAnimation_Custom, 0x6)
{
	enum { SkipGameCode = 0x70FDFB };

	GET(TechnoClass*, pThis, ESI);
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	R->EDX(pTypeExt->DrainAnimationType.Get(RulesClass::Instance->DrainAnimationType));

	return SkipGameCode;
}

bool __fastcall TechnoClass_IsReadyToCloak_Wrapper(TechnoClass* pTechno)
{
	bool withROF = pTechno->TechnoClass::IsReadyToCloak();

	if (!withROF)
	{
		const auto pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pTechno->GetTechnoType());

		if (!pTechnoTypeExt->Cloakable_IgnoreROF.Get(RulesExt::Global()->CloakIgnoreROF))
			return false;

		AbstractClass* pTarget = pTechno->Target;
		pTechno->Target = nullptr;
		int timeLeft = pTechno->DiskLaserTimer.TimeLeft;
		pTechno->DiskLaserTimer.TimeLeft = 0;
		bool withoutROF = pTechno->TechnoClass::IsReadyToCloak();
		pTechno->DiskLaserTimer.TimeLeft = timeLeft;
		pTechno->Target = pTarget;

		return withoutROF;
	}

	return withROF;
}

DEFINE_JUMP(VTABLE, 0x7E2544, GET_OFFSET(TechnoClass_IsReadyToCloak_Wrapper)); // AircraftClass
DEFINE_JUMP(VTABLE, 0x7E8F34, GET_OFFSET(TechnoClass_IsReadyToCloak_Wrapper)); // FootClass
DEFINE_JUMP(VTABLE, 0x7EB2F8, GET_OFFSET(TechnoClass_IsReadyToCloak_Wrapper)); // InfantryClass
DEFINE_JUMP(VTABLE, 0x7F4C00, GET_OFFSET(TechnoClass_IsReadyToCloak_Wrapper)); // TechnoClass
DEFINE_JUMP(VTABLE, 0x7F5F10, GET_OFFSET(TechnoClass_IsReadyToCloak_Wrapper)); // UnitClass
DEFINE_JUMP(CALL, 0x457779, GET_OFFSET(TechnoClass_IsReadyToCloak_Wrapper))    // BuildingClass

static void __stdcall DrawALine(int srcX, int srcY, int srcZ, int destX, int destY, int destZ, int nColor, bool unknown_a, bool unknown_b)
{
	JMP_STD(0x7049C0);
}

void __declspec(naked) _FootClass_DrawActionLines_Attack_SkipCall()
{
	ADD_ESP(0x24);
	JMP(0x4DC1A0);
}

void __declspec(naked) _FootClass_DrawActionLines_Move_SkipCall()
{
	ADD_ESP(0x24);
	JMP(0x4DC328);
}

DEFINE_HOOK(0x4DC19B, FootClass_DrawActionLines_Attack, 0x5)
{
	GET(FootClass*, pFoot, ESI);
	GET_STACK(int, SrcX, 0x0);
	GET_STACK(int, SrcY, 0x4);
	GET_STACK(int, SrcZ, 0x8);
	GET_STACK(int, DestX, 0xC);
	GET_STACK(int, DestY, 0x10);
	GET_STACK(int, DestZ, 0x14);
	GET_STACK(int, nColor, 0x18);
	GET_STACK(bool, Unknown_bool_a8, 0x1C);
	GET_STACK(bool, Unknown_bool_a9, 0x20);

	if (pFoot)
	{
		const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pFoot->GetTechnoType());

		if (auto pWeapon = pTypeExt->Line_Attack_Weapon.Get())
		{
			LaserDrawClass* pLaser = pFoot->CreateLaser(pFoot, 0, pWeapon, CoordStruct{ DestX, DestY, DestZ });
			pLaser->IsHouseColor = true;
		}
		else if (pTypeExt->Line_Attack_Dashed.Get())
		{
			Point2D pointStart = TacticalClass::Instance->CoordsToClient(CoordStruct{ SrcX, SrcY, SrcZ });
			Point2D pointEnd = TacticalClass::Instance->CoordsToClient(CoordStruct{ DestX, DestY, DestZ });
			if (ClipLine(&pointStart, &pointEnd, &DSurface::ViewBounds))
			{
				bool PatternAttack = false;
				DSurface::Composite->DrawDashedLine(&pointStart, &pointEnd, Drawing::RGB_To_Int(pTypeExt->Line_Attack_Color.Get()), &PatternAttack, 0);
			}
		}
		else
			DrawALine(SrcX, SrcY, SrcZ, DestX, DestY, DestZ, nColor, Unknown_bool_a8, Unknown_bool_a9);
	}

	return (int)_FootClass_DrawActionLines_Attack_SkipCall;
}

DEFINE_HOOK(0x4DC323, FootClass_DrawActionLines_Move, 0x5)
{
	GET(FootClass*, pFoot, ESI);
	GET_STACK(int, SrcX, 0x0);
	GET_STACK(int, SrcY, 0x4);
	GET_STACK(int, SrcZ, 0x8);
	GET_STACK(int, DestX, 0xC);
	GET_STACK(int, DestY, 0x10);
	GET_STACK(int, DestZ, 0x14);
	GET_STACK(int, nColor, 0x18);
	GET_STACK(bool, Unknown_bool_a8, 0x1C);
	GET_STACK(bool, Unknown_bool_a9, 0x20);

	if (pFoot)
	{
		const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pFoot->GetTechnoType());

		if (auto pWeapon = pTypeExt->Line_Move_Weapon.Get())
		{
			LaserDrawClass* pLaser = pFoot->CreateLaser(pFoot, 0, pWeapon, CoordStruct{ DestX, DestY, DestZ });
			pLaser->IsHouseColor = true;
		}
		else if (pTypeExt->Line_Move_Dashed.Get())
		{
			Point2D pointStart = TacticalClass::Instance->CoordsToClient(CoordStruct{ SrcX, SrcY, SrcZ });
			Point2D pointEnd = TacticalClass::Instance->CoordsToClient(CoordStruct{ DestX, DestY, DestZ });
			if (ClipLine(&pointStart, &pointEnd, &DSurface::ViewBounds))
			{
				bool PatternMove = false;
				DSurface::Composite->DrawDashedLine(&pointStart, &pointEnd, Drawing::RGB_To_Int(pTypeExt->Line_Move_Color.Get()), &PatternMove, 0);
			}
		}
		else
			DrawALine(SrcX, SrcY, SrcZ, DestX, DestY, DestZ, nColor, Unknown_bool_a8, Unknown_bool_a9);
	}

	return (int)_FootClass_DrawActionLines_Move_SkipCall;
}

DEFINE_HOOK(0x73E411, UnitClass_Mission_Unload_DumpAmount, 0x7)
{
	GET(UnitClass*, pThis, ESI);
	GET(int, tibIdx, EBP);
	REF_STACK(CoordStruct, var, STACK_OFFSET(0x80, -0x6C));
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);
	float dumpAmount = pThis->Tiberium.GetAmount(tibIdx);

	if (pTypeExt->HarvesterDumpAmount > 0)
		dumpAmount = std::min(dumpAmount, pTypeExt->HarvesterDumpAmount.Get());

	var.Y = *(int*)&dumpAmount;

	return 0x73E42B;
}

DEFINE_JUMP(LJMP, 0x4850CB, 0x4850F2)

DEFINE_JUMP(LJMP, 0x54D4C0, 0x54D531)

DEFINE_HOOK(0x7043B9, TechnoClass_GetZAdjustment_NthLink,0x6)
{
	GET(TechnoClass*, pNthLink, EAX);

	if (pNthLink == nullptr)
		return 0x7043E1;

	return 0;
}
