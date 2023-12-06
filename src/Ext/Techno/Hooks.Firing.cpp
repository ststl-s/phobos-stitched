#include "Body.h"

#include <Ext/Building/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/Techno/AresExtData.h>

#include <Utilities/EnumFunctions.h>
#include <Utilities/Macro.h>

#include <Misc/CaptureManager.h>
#include <Misc/DrawLaser.h>

#include <New/Armor/Armor.h>

DEFINE_HOOK(0x70E140, TechnoClass_GetWeapon, 0x6)
{
	GET(TechnoClass*, pThis, ECX);
	GET_STACK(int, weaponIdx, 0x4);

	enum { retn = 0x70E192 };

	auto pExt = TechnoExt::ExtMap.Find(pThis);

	if (weaponIdx < 0 || pExt == nullptr)
	{
		R->EAX(NULL);

		return retn;
	}

	TechnoTypeClass* pType = pThis->GetTechnoType();
	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
	const WeaponStruct* pWeapon = &pTypeExt->Weapons.Get(weaponIdx, pThis);

	if (pExt->CurrentFiringSW != nullptr)
	{
		const WeaponStruct* empulseWeapon = nullptr;

		if (pTypeExt->EMPulse_Weapons.count(pExt->CurrentFiringSW->Type->GetArrayIndex()))
		{
			empulseWeapon = &pTypeExt->EMPulse_Weapons[pExt->CurrentFiringSW->Type->GetArrayIndex()].Get(pThis);
		}

		if (empulseWeapon == nullptr || empulseWeapon->WeaponType == nullptr)
			R->EAX(&pTypeExt->Weapons.Get(0, pThis));
		else
			R->EAX(empulseWeapon);

		return retn;
	}

	if (pThis->WhatAmI() == AbstractType::Building && pType->Gunner)
	{
		pWeapon = &pTypeExt->Weapons.Get(pExt->IFVMode, pThis);
		R->EAX(pWeapon);
	}

	if (pTypeExt->IsExtendGattling && !pType->IsGattling)
	{
		if (pTypeExt->Gattling_Charge && !pTypeExt->Gattling_Cycle)
		{
			pWeapon = &pTypeExt->Weapons.Get(0, pThis);

			if (pThis->GetCurrentMission() == Mission::Unload)
			{
				pWeapon = &pTypeExt->Weapons.Get(pExt->GattlingWeaponIndex, pThis);
				pExt->HasCharged = true;
				pExt->IsCharging = false;
				pThis->ForceMission(Mission::Stop);
				pThis->ForceMission(Mission::Attack);
				pThis->SetTarget(pExt->AttackTarget);
			}

			if (pThis->GetCurrentMission() == Mission::Attack)
			{
				auto maxValue = pExt->GattlingStages[pType->WeaponStages - 1].GetItem(0);;
				if (pExt->GattlingCount >= maxValue)
				{
					pWeapon = &pTypeExt->Weapons.Get(pExt->GattlingWeaponIndex, pThis);
					pExt->HasCharged = true;
					pExt->IsCharging = false;
				}
				else if (!pExt->HasCharged)
				{
					pExt->IsCharging = true;
				}
			}
		}
		else
		{
			pWeapon = &pTypeExt->Weapons.Get(pExt->GattlingWeaponIndex, pThis);
		}

		R->EAX(pWeapon);
	}

	if (pThis->InOpenToppedTransport)
	{
		if (pTypeExt->WeaponInTransport.Get(pThis).WeaponType != nullptr)
		{
			pWeapon = &pTypeExt->WeaponInTransport.Get(pThis);
			R->EAX(pWeapon);

			return retn;
		}
	}

	if (pType->DeployFire && (pType->DeployFireWeapon >= -1 && pType->DeployFireWeapon <=1) && pExt->IsDeployed())
	{
		if (pTypeExt->NewDeployWeapon.Get(pThis).WeaponType != nullptr)
		{
			pWeapon = &pTypeExt->NewDeployWeapon.Get(pThis);
			R->EAX(pWeapon);

			return retn;
		}
	}

	if (!pType->IsGattling
		&& !pType->IsChargeTurret
		&& !pType->Gunner
		&& pExt->TargetType > 0
		&& pTypeExt->UseNewWeapon
		&& pTypeExt->NewWeapon_FireIndex == weaponIdx)
	{
		switch (pExt->TargetType)
		{
		case 1:
		{
			if (pTypeExt->NewWeapon_Infantry.Get(pThis).WeaponType != nullptr)
			{
				if (const WeaponStruct* pNewWeapon = &pTypeExt->NewWeapon_Infantry.Get(pThis))
					pWeapon = pNewWeapon;
			}
		}break;
		case 2:
		{
			if (pTypeExt->NewWeapon_Unit.Get(pThis).WeaponType != nullptr)
			{
				if (const WeaponStruct* pNewWeapon = &pTypeExt->NewWeapon_Unit.Get(pThis))
					pWeapon = pNewWeapon;
			}
		}break;
		case 3:
		{
			if (pTypeExt->NewWeapon_Aircraft.Get(pThis).WeaponType != nullptr)
			{
				if (const WeaponStruct* pNewWeapon = &pTypeExt->NewWeapon_Aircraft.Get(pThis))
					pWeapon = pNewWeapon;
			}
		}break;
		case 4:
		{
			if (pTypeExt->NewWeapon_Building.Get(pThis).WeaponType != nullptr)
			{
				if (const WeaponStruct* pNewWeapon = &pTypeExt->NewWeapon_Building.Get(pThis))
					pWeapon = pNewWeapon;
			}
		}break;
		case 5:
		{
			if (pTypeExt->NewWeapon_Infantry_AIR.Get(pThis).WeaponType != nullptr)
			{
				if (const WeaponStruct* pNewWeapon = &pTypeExt->NewWeapon_Infantry_AIR.Get(pThis))
					pWeapon = pNewWeapon;
			}
		}break;
		case 6:
		{
			if (pTypeExt->NewWeapon_Unit_AIR.Get(pThis).WeaponType != nullptr)
			{
				if (const WeaponStruct* pNewWeapon = &pTypeExt->NewWeapon_Unit_AIR.Get(pThis))
					pWeapon = pNewWeapon;
			}
		}break;
		case 7:
		{
			if (pTypeExt->NewWeapon_Aircraft_AIR.Get(pThis).WeaponType != nullptr)
			{
				if (const WeaponStruct* pNewWeapon = &pTypeExt->NewWeapon_Aircraft_AIR.Get(pThis))
					pWeapon = pNewWeapon;
			}
		}break;
		default:
			break;
		}
	}

	if (auto const pBld = abstract_cast<BuildingClass*>(pThis))
	{
		if (pBld->Type->Overpowerable && pBld->IsOverpowered)
		{
			auto pBldExt = BuildingExt::ExtMap.Find(pBld);
			auto pBldTypeExt = BuildingTypeExt::ExtMap.Find(pBld->Type);
			for (size_t i = 0; i < pBldTypeExt->Overpower_ChargeLevel.size(); i++)
			{
				if (pBldExt->OverPowerLevel >= pBldTypeExt->Overpower_ChargeLevel[i])
				{
					if (const WeaponStruct* pOverpowerWeapon = &pBldTypeExt->Overpower_ChargeLevel_Weapon[i].Get(pThis))
						pWeapon = pOverpowerWeapon;
				}
			}
		}
	}

	for (const auto& pAE : pExt->GetActiveAE())
	{
		if (!pAE->Type->ReplaceWeapon)
			continue;

		if (const WeaponStruct* pWeaponReplace = pAE->GetReplaceWeapon(weaponIdx))
		{
			pWeapon = pWeaponReplace;
			break;
		}
	}

	if (pThis->Target && pThis->Target->WhatAmI() == AbstractType::Bullet)
	{
		if (pTypeExt->Interceptor && pTypeExt->InterceptorType->WeaponType.Get(pThis).WeaponType != nullptr)
			pWeapon = &pTypeExt->InterceptorType->WeaponType.Get(pThis);
	}

	R->EAX(pWeapon);

	return retn;
}

DEFINE_HOOK(0x6F3360, TechnoClass_WhatWeaponShoueldIUse_MultipleTurrets, 0x6)
{
	GET(TechnoTypeClass*, pType, EAX);

	return pType->Gunner ? 0 : 0x6F3379;

}

// Weapon Selection
DEFINE_HOOK(0x6F3339, TechnoClass_WhatWeaponShouldIUse_Interceptor, 0x8)
{
	enum { SkipGameCode = 0x6F3341, ReturnValue = 0x6F3406 };

	GET(TechnoClass*, pThis, ESI);
	GET_STACK(AbstractClass*, pTarget, STACK_OFFSET(0x18, 0x4));

	if (pThis)
	{
		if (pTarget && pTarget->WhatAmI() == AbstractType::Bullet)
		{
			if (const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType()))
			{
				if (pTypeExt->Interceptor)
				{
					if (pTypeExt->InterceptorType->UseStageWeapon)
						R->EAX(pThis->CurrentWeaponNumber);
					else
						R->EAX(pTypeExt->InterceptorType->Weapon);

					return ReturnValue;
				}
			}
		}

		// Restore overridden instructions.
		R->EAX(pThis->GetTechnoType());

	}

	return SkipGameCode;
}

DEFINE_HOOK(0x6F33CD, TechnoClass_WhatWeaponShouldIUse_ForceFire, 0x6)
{
	enum { Secondary = 0x6F3745 };

	GET(TechnoClass*, pThis, ESI);
	GET_STACK(AbstractClass*, pTarget, STACK_OFFSET(0x18, 0x4));

	if (const auto pCell = abstract_cast<CellClass*>(pTarget))
	{
		if (const auto pPrimaryExt = WeaponTypeExt::ExtMap.Find(pThis->GetWeapon(0)->WeaponType))
		{
			if (pThis->GetWeapon(1)->WeaponType && !EnumFunctions::IsCellEligible(pCell, pPrimaryExt->CanTarget, true))
				return Secondary;
		}
	}

	return 0;
}

DEFINE_HOOK(0x6F3428, TechnoClass_WhatWeaponShouldIUse_ForceWeapon, 0x6)
{
	enum { UseWeaponIndex = 0x6F37AF };

	GET(TechnoClass*, pTechno, ECX);

	if (pTechno && pTechno->Target)
	{
		auto pTarget = abstract_cast<TechnoClass*>(pTechno->Target);

		if (!pTarget)
			return 0;

		int forceWeaponIndex = -1;

		if (auto pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pTechno->GetTechnoType()))
		{
			auto pTargetType = pTarget->GetTechnoType();

			if (pTechnoTypeExt->ForceWeapon_Naval_Decloaked >= 0 &&
				pTargetType->Cloakable && pTargetType->Naval &&
				pTarget->CloakState == CloakState::Uncloaked)
			{
				forceWeaponIndex = pTechnoTypeExt->ForceWeapon_Naval_Decloaked;
			}
			else if (pTechnoTypeExt->ForceWeapon_Cloaked >= 0 &&
				pTarget->CloakState == CloakState::Cloaked)
			{
				forceWeaponIndex = pTechnoTypeExt->ForceWeapon_Cloaked;
			}
			else if (pTechnoTypeExt->ForceWeapon_Disguised >= 0 &&
				pTarget->IsDisguised())
			{
				forceWeaponIndex = pTechnoTypeExt->ForceWeapon_Disguised;
			}
			else if (pTechnoTypeExt->ForceWeapon_UnderEMP >= 0)
			{
				if (auto pTargetTypeExt = TechnoTypeExt::ExtMap.Find(pTargetType))
				{
					if ((pTarget->IsUnderEMP()
						|| (pTargetTypeExt->ImmuneToEMP.isset() && pTargetTypeExt->ImmuneToEMP.Get() == true)
						|| (!pTargetTypeExt->ImmuneToEMP.isset() &&
							((pTarget->WhatAmI() == AbstractType::Infantry && !static_cast<InfantryTypeClass*>(pTargetType)->Cyborg) ||
								((pTarget->WhatAmI() == AbstractType::Unit || pTarget->WhatAmI() == AbstractType::Aircraft) && pTargetType->Organic)))))
					{
						forceWeaponIndex = pTechnoTypeExt->ForceWeapon_UnderEMP;
					}
				}
			}
		}

		if (forceWeaponIndex >= 0)
		{
			R->EAX(forceWeaponIndex);
			return UseWeaponIndex;
		}
	}

	return 0;
}

DEFINE_HOOK(0x6F36DB, TechnoClass_WhatWeaponShouldIUse, 0x8)
{
	GET(TechnoClass*, pThis, ESI);
	GET(TechnoClass*, pTargetTechno, EBP);
	GET_STACK(AbstractClass*, pTarget, STACK_OFFSET(0x18, 0x4));

	enum { Primary = 0x6F37AD, Secondary = 0x6F3745, FurtherCheck = 0x6F3754, OriginalCheck = 0x6F36E3 };

	if ((pTarget->AbstractFlags & AbstractFlags::Object) != AbstractFlags::None
		&& !TechnoExt::IsReallyAlive(static_cast<ObjectClass*>(pTarget)))
		return Primary;

	if (const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType()))
	{
		int weaponIndex = TechnoExt::PickWeaponIndex(pThis, pTargetTechno, pTarget, 0, 1, !pTypeExt->NoSecondaryWeaponFallback);

		if (weaponIndex != -1)
		{
			return weaponIndex == 1 ? Secondary : Primary;
		}

		if (const auto pTargetExt = TechnoExt::ExtMap.Find(pTargetTechno))
		{
			if (const auto pShield = pTargetExt->Shield.get())
			{
				if (pShield->IsActive())
				{
					if (pThis->GetWeapon(1) && !(pTypeExt->NoSecondaryWeaponFallback && !TechnoExt::CanFireNoAmmoWeapon(pThis, 1)))
					{
						if (!pShield->CanBeTargeted(pThis->GetWeapon(0)->WeaponType))
							return Secondary;
						else
							return FurtherCheck;
					}

					return Primary;
				}
			}
		}
	}

	const auto pExt = TechnoExt::ExtMap.Find(pThis);
	if (!pTargetTechno)
	{
		pExt->TargetType = 0;
		return Primary;
	}

	if (pTargetTechno != nullptr)
	{
		TechnoExt::ExtData* pTargetExt = TechnoExt::ExtMap.Find(pTargetTechno);
		WeaponStruct* primary = pThis->GetWeapon(0);
		WeaponStruct* secondary = pThis->GetWeapon(1);

		if (secondary != nullptr &&
			secondary->WeaponType != nullptr &&
			fabs(CustomArmor::GetVersus(secondary->WeaponType->Warhead, pTargetExt->GetArmorIdx(secondary->WeaponType->Warhead))) < 1e-6)
			return Primary;

		if (primary != nullptr &&
			primary->WeaponType != nullptr &&
			fabs(CustomArmor::GetVersus(primary->WeaponType->Warhead, pTargetExt->GetArmorIdx(primary->WeaponType->Warhead)) >= 1e-6))
			return FurtherCheck;

		return Secondary;
	}
	else if (auto pTargetObject = abstract_cast<ObjectClass*>(pTarget))
	{
		WeaponStruct* primary = pThis->GetWeapon(0);
		WeaponStruct* secondary = pThis->GetWeapon(1);
		ObjectTypeClass* pObjectType = pTargetObject->GetType();

		if (secondary != nullptr &&
			secondary->WeaponType != nullptr &&
			fabs(CustomArmor::GetVersus(secondary->WeaponType->Warhead, pObjectType->Armor)) < 1e-6)
			return Primary;

		if (primary != nullptr &&
			primary->WeaponType != nullptr &&
			fabs(CustomArmor::GetVersus(primary->WeaponType->Warhead, pObjectType->Armor)) >= 1e-6)
			return Primary;

		return Secondary;
	}
	else
	{
		return Primary;
	}
}

DEFINE_HOOK(0x6F37EB, TechnoClass_WhatWeaponShouldIUse_AntiAir, 0x6)
{
	enum { Primary = 0x6F37AD, Secondary = 0x6F3807 };

	GET_STACK(AbstractClass*, pTarget, STACK_OFFSET(0x18, 0x4));
	GET_STACK(WeaponTypeClass*, pWeapon, STACK_OFFSET(0x18, -0x4));
	GET(WeaponTypeClass*, pSecWeapon, EAX);

	const auto pTargetTechno = abstract_cast<TechnoClass*>(pTarget);

	if (!pWeapon->Projectile->AA && pSecWeapon->Projectile->AA && pTargetTechno != nullptr && pTargetTechno->IsInAir())
		return Secondary;

	return Primary;
}

DEFINE_HOOK(0x6F3432, TechnoClass_WhatWeaponShouldIUse_Gattling, 0xA)
{
	enum { ReturnValue = 0x6F37AF };

	GET(TechnoClass*, pThis, ESI);
	GET_STACK(AbstractClass*, pTarget, STACK_OFFSET(0x18, 0x4));

	const auto pTargetTechno = abstract_cast<TechnoClass*>(pTarget);
	int oddWeaponIndex = 2 * pThis->CurrentGattlingStage;
	int evenWeaponIndex = oddWeaponIndex + 1;
	int chosenWeaponIndex = oddWeaponIndex;
	int eligibleWeaponIndex = TechnoExt::PickWeaponIndex(pThis, pTargetTechno, pTarget, oddWeaponIndex, evenWeaponIndex, true);

	if (eligibleWeaponIndex != -1)
	{
		chosenWeaponIndex = eligibleWeaponIndex;
	}
	else if (pTargetTechno)
	{
		const WeaponStruct* pWeaponStructOdd = pThis->GetWeapon(oddWeaponIndex);
		const WeaponStruct* pWeaponStructEven = pThis->GetWeapon(evenWeaponIndex);
		auto const pWeaponOdd = pWeaponStructOdd->WeaponType;
		auto const pWeaponEven = pWeaponStructEven->WeaponType;
		bool skipRemainingChecks = false;

		if (!pWeaponStructOdd || !pWeaponOdd)
		{
			Debug::Log("[Error] TechnoType{%s} has no Weaponn%d!\n", pThis->get_ID(), oddWeaponIndex + 1);
		}

		if (!pWeaponStructEven || !pWeaponEven)
		{
			Debug::Log("[Error] TechnoType{%s} has no Weapon%d!\n", pThis->get_ID(), evenWeaponIndex + 1);
		}

		if (const auto pTargetExt = TechnoExt::ExtMap.Find(pTargetTechno))
		{
			if (const auto pShield = pTargetExt->Shield.get())
			{
				if (pShield->IsActive() && !pShield->CanBeTargeted(pWeaponOdd))
				{
					chosenWeaponIndex = evenWeaponIndex;
					skipRemainingChecks = true;
				}
			}
		}

		if (!skipRemainingChecks)
		{
			if (fabs(GeneralUtils::GetWarheadVersusArmor(pWeaponOdd->Warhead, pTargetTechno->GetTechnoType()->Armor)) < 1e-6)
			{
				chosenWeaponIndex = evenWeaponIndex;
			}
			else
			{
				auto pCell = pTargetTechno->GetCell();
				bool isOnWater = (pCell->LandType == LandType::Water || pCell->LandType == LandType::Beach) && !pTargetTechno->IsInAir();

				if (!pTargetTechno->OnBridge && isOnWater)
				{
					int navalTargetWeapon = pThis->SelectNavalTargeting(pTargetTechno);

					if (navalTargetWeapon == 2)
						chosenWeaponIndex = evenWeaponIndex;
				}
				else if ((pTargetTechno->IsInAir() && !pWeaponOdd->Projectile->AA && pWeaponEven->Projectile->AA) ||
					!pTargetTechno->IsInAir() && pThis->GetTechnoType()->LandTargeting == LandTargetingType::Land_Secondary)
				{
					chosenWeaponIndex = evenWeaponIndex;
				}
			}
		}
	}

	R->EAX(chosenWeaponIndex);
	return ReturnValue;
}

DEFINE_HOOK(0x6FC689, TechnoClass_CanFire_LandNavalTarget, 0x6)
{
	enum { DisallowFiring = 0x6FC86A };

	GET(TechnoClass*, pThis, ESI);
	GET_STACK(AbstractClass*, pTarget, STACK_OFFSET(0x20, 0x4));

	const auto pType = pThis->GetTechnoType();
	auto pCell = abstract_cast<CellClass*>(pTarget);

	if (pCell)
	{
		if (pType->NavalTargeting == NavalTargetingType::Naval_None &&
			(pCell->LandType == LandType::Water || pCell->LandType == LandType::Beach))
		{
			return DisallowFiring;
		}
	}
	else if (const auto pTerrain = abstract_cast<TerrainClass*>(pTarget))
	{
		pCell = pTerrain->GetCell();

		if (pType->LandTargeting == LandTargetingType::Land_Not_OK &&
			pCell->LandType != LandType::Water && pCell->LandType != LandType::Beach)
		{
			return DisallowFiring;
		}
		else if (pType->NavalTargeting == NavalTargetingType::Naval_None &&
			(pCell->LandType == LandType::Water || pCell->LandType == LandType::Beach))
		{
			return DisallowFiring;
		}
	}

	return 0;
}

DEFINE_HOOK(0x5218F3, InfantryClass_WhatWeaponShouldIUse_DeployFireWeapon, 0x6)
{
	GET(TechnoTypeClass*, pType, ECX);

	if (pType->DeployFireWeapon == -1)
		return 0x52194E;

	return 0;
}

// Pre-Firing Checks
DEFINE_HOOK(0x6FC339, TechnoClass_CanFire, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EDI);
	GET_STACK(AbstractClass*, pTarget, STACK_OFFSET(0x20, 0x4));
	// Checking for nullptr is not required here, since the game has already executed them before calling the hook  -- Belonit
	const auto pWH = pWeapon->Warhead;
	enum { CannotFire = 0x6FCB7E };

	if (const auto pWHExt = WarheadTypeExt::ExtMap.Find(pWH))
	{
		const int nMoney = pWHExt->TransactMoney;
		if (nMoney < 0 && pThis->Owner->Available_Money() < -nMoney)
			return CannotFire;
	}

	if (const auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon))
	{
		if(pThis->GetTechnoType()->WhatAmI() == AbstractType::UnitType
			&& pWeaponExt->KickOutPassenger.isset()
			&& pWeaponExt->KickOutPassenger)
		{
			if (pThis->GetTechnoType()->Passengers <= 0 || pThis->Passengers.NumPassengers <= 0)
				return CannotFire;
		}

		const auto pTechno = abstract_cast<TechnoClass*>(pTarget);

		CellClass* pTargetCell = nullptr;

		if (TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType())->LimitedAttackRange)
		{
			TechnoExt::ExtMap.Find(pThis)->AttackWeapon = pWeapon;
			if (pThis->DistanceFrom(pTarget) > (pWeapon->Range))
				return CannotFire;
		}

		if ((pThis->Passengers.NumPassengers == 0) && pWeaponExt->PassengerDeletion)
			return CannotFire;

		// Ignore target cell for airborne technos.
		if (!pTechno || !pTechno->IsInAir())
		{
			if (const auto pCell = abstract_cast<CellClass*>(pTarget))
				pTargetCell = pCell;
			else if (const auto pObject = abstract_cast<ObjectClass*>(pTarget))
				pTargetCell = pObject->GetCell();

			const auto pExt = TechnoExt::ExtMap.Find(pThis);
			pExt->TargetType = 0;
		}

		if (pTargetCell)
		{
			if (!EnumFunctions::IsCellEligible(pTargetCell, pWeaponExt->CanTarget, true))
				return CannotFire;
		}

		if (pTechno)
		{
			const auto pExt = TechnoExt::ExtMap.Find(pThis);
			const auto pTargetExt = TechnoExt::ExtMap.Find(pTechno);

			for (const auto pAE : pTargetExt->GetActiveAE())
			{
				if (pAE->Type->DisableBeTarget)
					return CannotFire;
			}

			if (pExt->TypeExtData && pExt->TypeExtData->UseNewWeapon.Get())
			{
				if (pTechno->WhatAmI() == AbstractType::Infantry)
				{
					if (pTechno->IsInAir())
						pExt->TargetType = 5;
					else
						pExt->TargetType = 1;
				}
				if (pTechno->WhatAmI() == AbstractType::Unit)
				{
					if (pTechno->IsInAir())
						pExt->TargetType = 6;
					else
						pExt->TargetType = 2;
				}
				if (pTechno->WhatAmI() == AbstractType::Aircraft)
				{
					if (pTechno->IsInAir())
						pExt->TargetType = 7;
					else
						pExt->TargetType = 3;
				}
				if (pTechno->WhatAmI() == AbstractType::Building)
				{
					pExt->TargetType = 4;
				}
			}
			else
				pExt->TargetType = 0;

			if (!EnumFunctions::IsTechnoEligible(pTechno, pWeaponExt->CanTarget) ||
				!EnumFunctions::CanTargetHouse(pWeaponExt->CanTargetHouses, pThis->Owner, pTechno->Owner))
			{
				return CannotFire;
			}

			if (!pWeaponExt->HasRequiredAttachedEffects(pTechno, pThis))
				return CannotFire;
		}
	}

	if (pWH->MindControl)
	{
		TechnoClass* pTechno = abstract_cast<TechnoClass*>(pTarget);

		if (pTechno != nullptr)
		{
			if (!CaptureManager::CanCapture(pThis->CaptureManager, pTechno))
				return CannotFire;
		}
	}

	return 0;
}

DEFINE_HOOK(0x6FC587, TechnoClass_CanFire_OpenTopped, 0x6)
{
	enum { DisallowFiring = 0x6FC86A };

	GET(TechnoClass*, pThis, ESI);

	if (auto const pTransport = pThis->Transporter)
	{
		if (auto pTypeExt = TechnoTypeExt::ExtMap.Find(pTransport->GetTechnoType()))
		{
			if (pTransport->Deactivated && !pTypeExt->OpenTopped_AllowFiringIfDeactivated)
				return DisallowFiring;
		}
	}

	return 0;
}

DEFINE_HOOK(0x6FCA26, TechnoClass_CanFire_ShouldDecloak, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EBX);
	const auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);
	const auto pExt = TechnoExt::ExtMap.Find(pThis);
	bool decloakToFire = pWeaponExt->DecloakToFire.Get(RulesExt::Global()->DecloakToFire);

	for (const auto pAE : pExt->GetActiveAE())
	{
		if (pAE->Type->DecloakToFire.isset())
		{
			decloakToFire = pAE->Type->DecloakToFire;
			break;
		}
	}

	R->AL(decloakToFire);

	return 0x6FCA2C;
}

DEFINE_HOOK(0x6FDD50, Techno_Before_Fire, 0x6)
{
	GET(TechnoClass*, pThis, ECX);
	GET_STACK(AbstractClass*, pTarget, 0x4);
	GET_STACK(int, idxWeapon, 0x8);

	WeaponTypeClass* pWeapon = pThis->GetWeapon(idxWeapon)->WeaponType;

	if (pWeapon == nullptr)
		return 0;

	// CustomArmor::Debug(pThis, pTarget, pWeapon);

	auto pExt = TechnoExt::ExtMap.Find(pThis);
	bool disableAttach = false;

	for (auto pAE : pExt->GetActiveAE())
	{
		if (pAE->Type->DisableWeapon && (pAE->Type->DisableWeapon_Category & DisableWeaponCate::Attach))
		{
			disableAttach = true;
		}

		++pAE->AttachOwnerShoots;
	}

	if (!disableAttach)
	{
		auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);
		auto pWHExt = WarheadTypeExt::ExtMap.Find(pWeapon->Warhead);

		WeaponTypeExt::ProcessAttachWeapons(pWeapon, pThis, pTarget);

		if (pWeaponExt->ExtraBurst_Spread)
			WeaponTypeExt::ProcessExtraBrustSpread(pWeapon, pThis, pTarget);
		else
			WeaponTypeExt::ProcessExtraBrust(pWeapon, pThis, pTarget);

		if (pWeaponExt->Ammo < 0)
			TechnoExt::ChangeAmmo(pThis, pWeaponExt->Ammo);

		TechnoExt::IonCannonWeapon(pThis, pTarget, pWeapon);
		TechnoExt::BeamCannon(pThis, pTarget, pWeapon);

		if (pWeaponExt->SelfTransport && pThis->WhatAmI() != AbstractType::Building)
			TechnoExt::FireSelf(pThis, pWeaponExt);
		else
			TechnoExt::FirePassenger(pThis, pWeaponExt);

		if (pWeapon->Warhead->Temporal && pWHExt->Temporal_CellSpread > 0)
		{
			if (auto pTargetTechno = abstract_cast<TechnoClass*>(pTarget))
			{
				TechnoExt::SetTemporalTeam(pThis, pTargetTechno, pWHExt);
			}
		}
	}

	TechnoExt::AllowPassengerToFire(pThis, pTarget, pWeapon);
	TechnoExt::SpawneLoseTarget(pThis);
	TechnoExt::SetGattlingCount(pThis, pTarget, pWeapon);
	TechnoExt::ShareWeaponRange(pThis, pTarget, pWeapon);
	TechnoExt::KickOutPassenger(pThis, idxWeapon);

	if (pTarget->AbstractFlags & AbstractFlags::Techno)
		TechnoExt::RememeberFirer(pThis, pTarget, pWeapon);

	if (const auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon))
	{
		pWeaponExt->AddStrafingLaser(pThis, pTarget, idxWeapon);
	}

	return 0;
}

DEFINE_HOOK(0x6FDD6F, TechnoClass_Fire_AfterGetWeapon, 0x8)
{
	GET(TechnoClass*, pThis, ESI);

	if (const auto pExt = TechnoExt::ExtMap.Find(pThis))
	{
		if (pExt->CurrentFiringSW != nullptr)
		{
			pExt->CurrentFiringSW = nullptr;
			pExt->FinishSW = true;
		}
	}

	return 0;
}

// Weapon Firing

DEFINE_HOOK(0x6FE43B, TechnoClass_FireAt_OpenToppedDmgMult, 0x8)
{
	enum { ApplyDamageMult = 0x6FE45A, ContinueCheck = 0x6FE460 };

	GET(TechnoClass* const, pThis, ESI);

	//replacing whole check due to `fild`
	if (pThis->InOpenToppedTransport)
	{
		GET_STACK(int, nDamage, STACK_OFFSET(0xB0, -0x84));
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

DEFINE_HOOK(0x6FE19A, TechnoClass_FireAt_AreaFire, 0x6)
{
	enum { DoNotFire = 0x6FE4E7, SkipSetTarget = 0x6FE1D5 };

	GET(TechnoClass* const, pThis, ESI);
	GET(CellClass* const, pCell, EAX);
	GET_STACK(WeaponTypeClass*, pWeaponType, STACK_OFFSET(0xB0, -0x70));

	if (auto pExt = WeaponTypeExt::ExtMap.Find(pWeaponType))
	{
		if (pExt->AreaFire_Target == AreaFireTarget::Random)
		{
			auto const range = pWeaponType->Range / static_cast<double>(Unsorted::LeptonsPerCell);

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

DEFINE_HOOK(0x6FF43F, TechnoClass_FireAt_FeedbackWeapon, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EBX);

	auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);

	if (pWeaponExt)
	{
		if (pWeaponExt->FeedbackWeapon.isset())
		{
			auto fbWeapon = pWeaponExt->FeedbackWeapon.Get();

			if (pThis->InOpenToppedTransport && !fbWeapon->FireInTransport)
				return 0;

			auto pExt = TechnoExt::ExtMap.Find(pThis);

			for (const auto& pAE : pExt->AttachEffects)
			{
				if (pAE->Type->DisableWeapon && (pAE->Type->DisableWeapon_Category & DisableWeaponCate::Feedback))
					return 0;
			}

			WeaponTypeExt::DetonateAt(fbWeapon, pThis, pThis);
		}
	}

	return 0;
}

DEFINE_HOOK(0x6FF660, TechnoClass_FireAt_Interceptor, 0x6)
{
	GET(TechnoClass* const, pSource, ESI);
	GET_BASE(AbstractClass* const, pTarget, 0x8);
	GET(WeaponTypeClass* const, pWeaponType, EBX);
	GET_STACK(BulletClass* const, pBullet, STACK_OFFSET(0xB0, -0x74));

	auto const pSourceTypeExt = TechnoTypeExt::ExtMap.Find(pSource->GetTechnoType());
	bool interceptor = pSourceTypeExt->Interceptor;
	if (interceptor)
	{
		const InterceptorTypeClass* pInterceptorType = pSourceTypeExt->InterceptorType.get();
		bool interceptor_Rookie = pInterceptorType->Rookie.Get(true);
		bool interceptor_Veteran = pInterceptorType->Veteran.Get(true);
		bool interceptor_Elite = pInterceptorType->Elite.Get(true);

		if (pSource->Veterancy.IsRookie() && !interceptor_Rookie)
			interceptor = false;

		if (pSource->Veterancy.IsVeteran() && !interceptor_Veteran)
			interceptor = false;

		if (pSource->Veterancy.IsElite() && !interceptor_Elite)
			interceptor = false;

		if (!interceptor)
			return 0;

		if (auto const pTargetObject = specific_cast<BulletClass* const>(pTarget))
		{
			if (auto const pBulletExt = BulletExt::ExtMap.Find(pBullet))
			{
				int probability = ScenarioClass::Instance->Random.RandomRanged(1, 100);
				int successProbability = pInterceptorType->Success;

				if (!pSource->Veterancy.IsRookie())
				{
					if (pSource->Veterancy.IsVeteran())
					{
						if (pInterceptorType->VeteranSuccess >= 0)
						{
							successProbability = pInterceptorType->VeteranSuccess;
						}
					}
					else
					{
						if (pSource->Veterancy.IsElite())
						{
							if (pInterceptorType->EliteSuccess >= 0)
							{
								successProbability = pInterceptorType->EliteSuccess;
							}
							else
							{
								if (pInterceptorType->VeteranSuccess >= 0)
								{
									successProbability = pInterceptorType->VeteranSuccess;
								}
							}
						}
					}
				}
				else
				{
					if (pInterceptorType->RookieSuccess >= 0)
					{
						successProbability = pInterceptorType->RookieSuccess;
					}
				}

				//Debug::Log("DEBUG: Interceptor: %d\% <= %d\% ??? R:%d V:%d E:%d\n", probability, successProbability, pSource->Veterancy.IsRookie(), pSource->Veterancy.IsVeteran(), pSource->Veterancy.IsElite());
				if (probability <= successProbability)
				{
					//Debug::Log("DEBUG: Intercepted projectile!\n");
					pBulletExt->IsInterceptor = true;
					pBulletExt->InterceptedStatus = InterceptedStatus::Targeted;

					// If using Inviso projectile, can intercept bullets right after firing.
					if (pTargetObject->IsAlive && pWeaponType->Projectile->Inviso)
					{
						if (auto const pWHExt = WarheadTypeExt::ExtMap.Find(pWeaponType->Warhead))
							pWHExt->InterceptBullets(pSource, pWeaponType, pTargetObject->Location);
					}
				}
			}
		}
	}

	return 0;
}

DEFINE_HOOK_AGAIN(0x6FF660, TechnoClass_FireAt_ToggleLaserWeaponIndex, 0x6)
DEFINE_HOOK(0x6FF4CC, TechnoClass_FireAt_ToggleLaserWeaponIndex, 0x6)
{
	GET(TechnoClass* const, pThis, ESI);
	GET(WeaponTypeClass* const, pWeapon, EBX);
	GET_BASE(int, weaponIndex, 0xC);

	if (pThis->WhatAmI() == AbstractType::Building && pWeapon->IsLaser)
	{
		if (auto const pExt = TechnoExt::ExtMap.Find(pThis))
		{
			if (pExt->CurrentLaserWeaponIndex.empty())
				pExt->CurrentLaserWeaponIndex = weaponIndex;
			else
				pExt->CurrentLaserWeaponIndex.clear();
		}
	}

	return 0;
}

//DEFINE_HOOK(0x6F858F, TechnoClass_CanAutoTarget_BuildingOut1, 0x6)
//{
//	GET(TechnoClass*, pThis, EDI);
//	GET(TechnoClass*, pTarget, ESI);
//
//	enum { CannotTarget = 0x6F85F8, NextIf = 0x6F860C, FarIf = 0x6F866D };
//
//	if (FootClass* pFoot = abstract_cast<FootClass*>(pThis))
//	{
//		if (pFoot->Team != nullptr
//			|| !pFoot->Owner->ControlledByHuman()
//			|| pTarget->IsStrange()
//			|| pTarget->WhatAmI() != AbstractType::Building
//			|| pTarget->GetTurretWeapon() && pTarget->GetTurretWeapon()->WeaponType != nullptr && pTarget->GetThreatValue())
//		{
//			//game code
//			if (!pThis->IsEngineer())
//				return FarIf;
//
//			return NextIf;
//		}
//		else
//		{
//			if (!pThis->IsEngineer())
//			{
//				//dehardcode
//				if (pTarget->WhatAmI() == AbstractType::Building && pTarget->GetThreatValue())
//				{
//					return FarIf;
//				}
//
//				return CannotTarget;
//			}
//
//			return NextIf;
//		}
//	}
//
//	return NextIf;
//}
//
//DEFINE_HOOK(0x6F889B, TechnoClass_CanAutoTarget_BuildingOut2, 0xA)
//{
//	GET(TechnoClass*, pTarget, ESI);
//
//	enum { CannotTarget = 0x6F894F, GameCode = 0x6F88BF };
//
//	if (pTarget->WhatAmI() != AbstractType::Building)
//		return CannotTarget;
//
//	WeaponStruct* pWeapon = pTarget->GetTurretWeapon();
//
//	if (pWeapon == nullptr || pWeapon->WeaponType == nullptr)
//	{
//		if (pTarget->GetThreatValue())
//			return GameCode;
//
//		return CannotTarget;
//	}
//
//	return GameCode;
//}

DEFINE_HOOK(0x6FDD93, TechnoClass_FireAt_DelayedFire, 0x6) // or 0x6FDD99  , 0x6
{
	GET(WeaponTypeClass*, pWeaponType, EBX);
	GET(TechnoClass*, pThis, ESI);

	enum { continueFireAt = 0, skipFireAt = 0x6FDE03 };

	if (!pThis)
		return continueFireAt;

	if (auto pWeaponTypeExt = WeaponTypeExt::ExtMap.Find(pWeaponType))
	{
		if (auto pExt = TechnoExt::ExtMap.Find(pThis))
		{
			if (pWeaponTypeExt->DelayedFire_Anim_LoopCount <= 0)
				return continueFireAt;

			if (!pWeaponTypeExt->DelayedFire_Anim.isset())
				return continueFireAt;

			if (pWeaponTypeExt->DelayedFire_DurationTimer.Get() > 0 && pExt->DelayedFire_DurationTimer < 0)
				pExt->DelayedFire_DurationTimer = pWeaponTypeExt->DelayedFire_DurationTimer.Get();

			AnimTypeClass* pDelayedFireAnimType = pWeaponTypeExt->DelayedFire_Anim.isset() ? pWeaponTypeExt->DelayedFire_Anim.Get() : nullptr;
			bool hasValidDelayedFireAnimType = pDelayedFireAnimType ? true : false;

			if (!hasValidDelayedFireAnimType)
			{
				pExt->DelayedFire_Anim = nullptr;
				pExt->DelayedFire_Anim_LoopCount = 0;
				pExt->DelayedFire_DurationTimer = -1;

				return continueFireAt;
			}

			bool isDelayedFireAnimPlaying = pExt->DelayedFire_Anim ? true : false;
			bool hasDeployAnimFinished = (isDelayedFireAnimPlaying && (pExt->DelayedFire_Anim->Animation.Value >= pDelayedFireAnimType->End + pDelayedFireAnimType->Start - 1)) ? true : false;

			if (!isDelayedFireAnimPlaying)
			{
				// Create the DelayedFire animation & stop the Fire process
				TechnoTypeClass* pThisType = pThis->GetTechnoType();
				int weaponIndex = pThis->CurrentWeaponNumber;
				//int weaponIndex = pThis->SelectWeapon(pThis->Target);
				CoordStruct animLocation = pThis->Location;

				if (pWeaponTypeExt->DelayedFire_Anim_UseFLH)
					animLocation = TechnoExt::GetFLHAbsoluteCoords(pThis, pThisType->Weapon[weaponIndex].FLH, pThis->HasTurret());//pThisType->Weapon[weaponIndex].FLH;

				if (auto pAnim = GameCreate<AnimClass>(pDelayedFireAnimType, animLocation))//pThis->Location))//animLocation))
				{
					pExt->DelayedFire_Anim = pAnim;
					pExt->DelayedFire_Anim->SetOwnerObject(pThis);
					pExt->DelayedFire_Anim_LoopCount++;
				}
				else
				{
					Debug::Log("ERROR! DelayedFire animation [%s] -> %s can't be created.\n", pThis->GetTechnoType()->ID, pDelayedFireAnimType->ID);
					pExt->DelayedFire_Anim = nullptr;
					pExt->DelayedFire_Anim_LoopCount = 0;
					pExt->DelayedFire_DurationTimer = -1;

					return continueFireAt;
				}
			}
			else
			{
				if (pWeaponTypeExt->DelayedFire_DurationTimer.Get() > 0)
				{
					pExt->DelayedFire_DurationTimer--;

					if (pExt->DelayedFire_DurationTimer <= 0)
					{
						pExt->DelayedFire_Anim = nullptr;
						pExt->DelayedFire_Anim_LoopCount = 0;
						pExt->DelayedFire_DurationTimer = -1;

						return continueFireAt;
					}
				}

				if (hasDeployAnimFinished)
				{
					// DelayedFire animation finished but it can repeat more times, if set
					pExt->DelayedFire_Anim = nullptr;

					if (pExt->DelayedFire_Anim_LoopCount >= pWeaponTypeExt->DelayedFire_Anim_LoopCount && pWeaponTypeExt->DelayedFire_Anim_LoopCount > 0)
					{
						pExt->DelayedFire_Anim_LoopCount = 0;
						pExt->DelayedFire_DurationTimer = -1;

						return continueFireAt;
					}
				}
			}

			return skipFireAt;
		}
	}

	return continueFireAt;
}

DEFINE_HOOK(0x6F3AF9, TechnoClass_GetFLH_GetAlternateFLH, 0x6)
{
	GET(TechnoClass*, pThis, EBX);
	GET(int, weaponIdx, ESI);

	enum { SkipGameCode = 0x6F3B37 };

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	weaponIdx = -weaponIdx - 1;

	const CoordStruct& flh =
		weaponIdx < static_cast<int>(pTypeExt->AlternateFLHs.size())
		? pTypeExt->AlternateFLHs[weaponIdx]
		: CoordStruct::Empty;

	R->ECX(flh.X);
	R->EBP(flh.Y);
	R->EAX(flh.Z);

	return SkipGameCode;
}

// Feature: Allow Units using AlternateFLHs - by Trsdy
// I don't want to rewrite something new, so I use the Infantry one directly
// afaik it didn't check infantry-specific stuff here
// and neither Ares nor Phobos messed up with it so far, even that crawling flh one was in TechnoClass
DEFINE_JUMP(VTABLE, 0x7F5D20, 0x523250); // Redirect UnitClass::GetFLH to InfantryClass::GetFLH (used to be TechnoClass::GetFLH)

// 覆盖Ares的部分，为创建出来的LaserDrawClass添加一个跟踪，虽然不知道它会不会RE（汗）。
// 感谢烈葱提供的原始代码以及ststl姐姐寻找的覆盖地址。飞星？他负责当黑奴搬砖（迫真）。
DEFINE_HOOK(0x6FF4CC, TechnoClass_Fire_CreateLaser, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(TechnoClass* const, pTarget, EDI);
	GET(WeaponTypeClass*, pWeapon, EBX);

	if (!pWeapon->IsLaser)
		return 0x6FF57D;

	auto const idxWeapon = R->Base<int>(0xC); // don't use stack offsets - function uses on-the-fly stack realignments which mean offsets are not constants
	auto const pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);
	int const Thickness = pWeaponExt->Laser_Thickness;

	if (auto const pBld = abstract_cast<BuildingClass*>(pThis))
	{
		auto const pTWeapon = pBld->GetTurretWeapon()->WeaponType;

		if (auto const pLaser = pBld->CreateLaser(pTarget, idxWeapon, pTWeapon, CoordStruct::Empty))
		{
			//default thickness for buildings.
			//this was 3 for PrismType (rising to 5 for supported prism) but no idea what it was for non-PrismType - setting to 3 for all BuildingTypes now.
			if (Thickness == -1)
			{
				pLaser->Thickness = 3;
			}
			else
			{
				pLaser->Thickness = Thickness;
			}

			auto const pBldTypeData = BuildingTypeExt::ExtMap.Find(pBld->Type);

			if (pBldTypeData->PrismForwarding.CanAttack())
			{
				//is a prism tower
				if (pBld->SupportingPrisms > 0)
				{
					//Ares sets this to the longest backward chain
					//is being supported... so increase beam intensity
					if (pBldTypeData->PrismForwarding.Intensity < 0)
					{
						pLaser->Thickness -= pBldTypeData->PrismForwarding.Intensity; //add on absolute intensity
					}
					else if (pBldTypeData->PrismForwarding.Intensity > 0)
					{
						pLaser->Thickness += (pBldTypeData->PrismForwarding.Intensity * pBld->SupportingPrisms);
					}

					// always supporting
					pLaser->IsSupported = true;
				}
			}

			// 使用CoordStruct::Empty是因为我不懂那个获取开火坐标的机制，于是就不跟踪了。
			if (pWeaponExt->IsTrackingLaser.Get())
				DrawLaser::AddTrackingLaser(pLaser, pLaser->Duration, pThis, pTarget, CoordStruct::Empty, pThis->GetTechnoType()->Turret);
		}
	}
	else
	{
		if (auto const pLaser = pThis->CreateLaser(pTarget, idxWeapon, pWeapon, CoordStruct::Empty))
		{
			if (Thickness == -1)
			{
				pLaser->Thickness = 2;
			}
			else
			{
				pLaser->Thickness = Thickness;

				// required for larger Thickness to work right
				pLaser->IsSupported = (Thickness > 3);
			}

			// 使用CoordStruct::Empty的理由同上。
			if (pWeaponExt->IsTrackingLaser.Get())
				DrawLaser::AddTrackingLaser(pLaser, pLaser->Duration, pThis, pTarget, CoordStruct::Empty, pThis->GetTechnoType()->Turret);
		}
	}

	return 0x6FF656;
}

DEFINE_HOOK_AGAIN(0x44ACF0, TechnoClass_UpdateFiring, 0x6)	//BuildingClass::Mission_Attack
DEFINE_HOOK_AGAIN(0x417FE0, TechnoClass_UpdateFiring, 0x6)	//AircraftClass::Mission_Attack
DEFINE_HOOK_AGAIN(0x736DF0, TechnoClass_UpdateFiring, 0x7)	//UnitClass::UpdateFiring
DEFINE_HOOK(0x5206B0, TechnoClass_UpdateFiring, 0x6)		//InfantryClass::UpdateFiring
{
	GET(TechnoClass*, pThis, ECX);

	if (pThis->Target == nullptr)
		return 0;

	AbstractClass* pTarget = pThis->Target;
	int weaponIdx = pThis->SelectWeapon(pTarget);

	const WeaponStruct* pWeapon = pThis->GetWeapon(weaponIdx);

	if (pWeapon == nullptr || pWeapon->WeaponType == nullptr)
		return 0;

	const WeaponTypeClass* pWeaponType = pWeapon->WeaponType;
	const WeaponTypeExt::ExtData* pWeaponTypeExt = WeaponTypeExt::ExtMap.Find(pWeaponType);

	if (pWeaponTypeExt->AttachWeapons.empty())
		return 0;

	FireError fireError = pThis->GetFireError(pTarget, weaponIdx, true);

	if (fireError != FireError::OK && fireError != FireError::REARM)
		return 0;

	const int weaponArrayIndex = pWeaponType->GetArrayIndex();
	TechnoExt::ExtData* pExt = TechnoExt::ExtMap.Find(pThis);
	std::vector<CDTimerClass>& vTimers = pExt->AttachWeapon_Timers[weaponArrayIndex];
	const std::vector<CoordStruct>& FLHs = pWeaponTypeExt->AttachWeapons_FLH;
	
	for (size_t i = 0; i < pWeaponTypeExt->AttachWeapons.size(); i++)
	{
		if (i >= pWeaponTypeExt->AttachWeapons_DetachedFire.size())
			break;

		if (pWeaponTypeExt->AttachWeapons[i] == pWeaponType
			|| !pWeaponTypeExt->AttachWeapons_DetachedFire.at(i))
			continue;

		while (i >= vTimers.size())
			vTimers.emplace_back(CDTimerClass(-1));

		if (!vTimers[i].Completed())
			continue;

		WeaponTypeClass* pAttachWeapon = pWeaponTypeExt->AttachWeapons[i];

		int rofBuff;
		double rofMulti = pExt->GetAEROFMul(&rofBuff) * pThis->Owner->ROFMultiplier * pThis->AresExtData->ROFMultiplier;

		vTimers[i].Start(Game::F2I(pAttachWeapon->ROF * rofMulti) + rofBuff);

		CoordStruct FLH = i >= FLHs.size() ? FLHs[i] : CoordStruct::Empty;

		if (pWeaponTypeExt->AttachWeapons_Burst_InvertL && pWeaponType->Burst > 1 && pThis->CurrentBurstIndex & 1)
			FLH.Y = -FLH.Y;

		WeaponStruct weapon(pAttachWeapon, FLH);
		TechnoExt::SimulatedFire(pThis, weapon, pTarget);
	}

	return 0;
}
