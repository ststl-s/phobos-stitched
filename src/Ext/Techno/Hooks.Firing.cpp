#include "Body.h"

#include <ScenarioClass.h>

#include <Utilities/EnumFunctions.h>

#include <Ext/Bullet/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/Techno/Body.h>

#include <New/Armor/Armor.h>

DEFINE_HOOK(0x70E140, TechnoClass_GetWeapon, 0x6)
{
	GET(TechnoClass*, pThis, ECX);
	GET_STACK(int, weaponIdx, 0x4);

	enum { retn = 0x70E192 };

	if (weaponIdx < 0)
	{
		R->EAX(NULL);

		return retn;
	}

	auto pExt = TechnoExt::ExtMap.Find(pThis);
	TechnoTypeClass* pType = pThis->GetTechnoType();
	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
	const WeaponStruct* pWeapon = &pTypeExt->Weapons.Get(weaponIdx, pThis);

	if (pThis->InOpenToppedTransport)
	{
		if (pTypeExt->WeaponInTransport.Get(pThis).WeaponType != nullptr)
		{
			pWeapon = &pTypeExt->WeaponInTransport.Get(pThis);
			R->EAX(pWeapon);

			return retn;
		}
	}

	bool IsDeploy = (pThis->WhatAmI() == AbstractType::Infantry) ? IsDeploy = abstract_cast<InfantryClass*>(pThis)->IsDeployed()
		: (pThis->CurrentMission == Mission::Unload) ? IsDeploy = true : IsDeploy = false;

	if (pType->DeployFire && (pType->DeployFireWeapon >= -1 && pType->DeployFireWeapon <=1) && IsDeploy)
	{
		if (pTypeExt->NewDeployWeapon.Get(pThis).WeaponType != nullptr)
		{
			pWeapon = &pTypeExt->WeaponInTransport.Get(pThis);
			R->EAX(pWeapon);

			return retn;
		}
	}

	for (const auto& pAE : pExt->AttachEffects)
	{
		if (!pAE->IsActive())
			continue;

		if (const WeaponStruct* pWeaponReplace = pAE->GetReplaceWeapon(weaponIdx))
			pWeapon = pWeaponReplace;
	}

	R->EAX(pWeapon);

	return retn;
}

// Weapon Selection

DEFINE_HOOK(0x6F3339, TechnoClass_WhatWeaponShouldIUse_Interceptor, 0x8)
{
	enum { SkipGameCode = 0x6F3341, ReturnValue = 0x6F3406 };

	GET(TechnoClass*, pThis, ESI);
	GET_STACK(AbstractClass*, pTarget, STACK_OFFS(0x18, -0x4));

	if (pThis && pTarget && pTarget->WhatAmI() == AbstractType::Bullet)
	{
		if (const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType()))
		{
			if (pTypeExt->Interceptor)
			{
				R->EAX(pTypeExt->Interceptor_Weapon);
				return ReturnValue;
			}
		}
	}

	// Restore overridden instructions.
	R->EAX(pThis->GetTechnoType());

	return SkipGameCode;
}

DEFINE_HOOK(0x6F33CD, TechnoClass_WhatWeaponShouldIUse_ForceFire, 0x6)
{
	enum { Secondary = 0x6F3745 };

	GET(TechnoClass*, pThis, ESI);
	GET_STACK(AbstractClass*, pTarget, STACK_OFFS(0x18, -0x4));

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

DEFINE_HOOK(0x6F36DB, TechnoClass_WhatWeaponShouldIUse, 0x8)
{
	GET(TechnoClass*, pThis, ESI);
	GET(TechnoClass*, pTargetTechno, EBP);
	GET_STACK(AbstractClass*, pTarget, STACK_OFFS(0x18, -0x4));

	enum { Primary = 0x6F37AD, Secondary = 0x6F3745, FurtherCheck = 0x6F3754, OriginalCheck = 0x6F36E3 };

	CellClass* targetCell = nullptr;

	// Ignore target cell for airborne technos.
	if (!pTargetTechno || !pTargetTechno->IsInAir())
	{
		if (const auto pCell = abstract_cast<CellClass*>(pTarget))
			targetCell = pCell;
		else if (const auto pObject = abstract_cast<ObjectClass*>(pTarget))
			targetCell = pObject->GetCell();
	}

	if (const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType()))
	{
		if (const auto pSecondary = pThis->GetWeapon(1))
		{
			if (const auto pSecondaryExt = WeaponTypeExt::ExtMap.Find(pSecondary->WeaponType))
			{
				if ((targetCell && !EnumFunctions::IsCellEligible(targetCell, pSecondaryExt->CanTarget, true)) ||
					(pTargetTechno && (!EnumFunctions::IsTechnoEligible(pTargetTechno, pSecondaryExt->CanTarget) ||
						!EnumFunctions::CanTargetHouse(pSecondaryExt->CanTargetHouses, pThis->Owner, pTargetTechno->Owner) ||
						pSecondaryExt->OnlyAllowOneFirer && TechnoExt::ExtMap.Find(pTargetTechno)->Attacker != nullptr && pThis != TechnoExt::ExtMap.Find(pTargetTechno)->Attacker)) ||
					(pThis->Passengers.NumPassengers == 0 && pSecondaryExt->PassengerDeletion))
				{
					return Primary;
				}

				if (const auto pPrimaryExt = WeaponTypeExt::ExtMap.Find(pThis->GetWeapon(0)->WeaponType))
				{
					if (pTypeExt->NoSecondaryWeaponFallback && !TechnoExt::CanFireNoAmmoWeapon(pThis, 1))
						return Primary;

					if ((targetCell && !EnumFunctions::IsCellEligible(targetCell, pPrimaryExt->CanTarget, true)) ||
						(pTargetTechno && (!EnumFunctions::IsTechnoEligible(pTargetTechno, pPrimaryExt->CanTarget) ||
							!EnumFunctions::CanTargetHouse(pPrimaryExt->CanTargetHouses, pThis->Owner, pTargetTechno->Owner) ||
							pPrimaryExt->OnlyAllowOneFirer && TechnoExt::ExtMap.Find(pTargetTechno)->Attacker != nullptr && pThis != TechnoExt::ExtMap.Find(pTargetTechno)->Attacker)) ||
						(pThis->Passengers.NumPassengers == 0 && pPrimaryExt->PassengerDeletion))
					{
						return Secondary;
					}

					if (pTypeExt->DeterminedByRange)
					{
						auto closeweapon = Primary;
						auto furtherweapon = Secondary;
						auto pCloseWeapon = pThis->GetWeapon(0)->WeaponType;
						auto pFurtherWeapon = pThis->GetWeapon(1)->WeaponType;

						if (pTargetTechno != nullptr)
						{
							TechnoExt::ExtData* pTargetTechnoExt = TechnoExt::ExtMap.Find(pTargetTechno);

							double versusClose = CustomArmor::GetVersus(pCloseWeapon->Warhead, pTargetTechnoExt->GetArmorIdx(pCloseWeapon));
							double versusFurther = CustomArmor::GetVersus(pFurtherWeapon->Warhead, pTargetTechnoExt->GetArmorIdx(pFurtherWeapon));

							if (versusFurther == 0.0)
								return Primary;

							if (versusClose == 0.0)
								return Secondary;
						}

						if (pThis->GetWeapon(0)->WeaponType->Range > pSecondary->WeaponType->Range)
						{
							closeweapon = Secondary;
							furtherweapon = Primary;
							pCloseWeapon = pThis->GetWeapon(1)->WeaponType;
							pFurtherWeapon = pThis->GetWeapon(0)->WeaponType;
						}

						int ChangeRange = pFurtherWeapon->MinimumRange;
						int ChangeRangeExtra = pTypeExt->DeterminedByRange_ExtraRange * 256;
						ChangeRange += ChangeRangeExtra;

						auto pExt = TechnoExt::ExtMap.Find(pThis);

						if (pTypeExt->DeterminedByRange_MainWeapon == 0)
						{
							if (!pExt->InROF && pThis->GetWeapon(1)->WeaponType->Range >= pThis->DistanceFrom(pTarget) && pThis->GetWeapon(1)->WeaponType->MinimumRange <= pThis->DistanceFrom(pTarget))
								return Secondary;

							return Primary;
						}

						if (pTypeExt->DeterminedByRange_MainWeapon == 1)
						{
							if (!pExt->InROF && pThis->GetWeapon(0)->WeaponType->Range >= pThis->DistanceFrom(pTarget) && pThis->GetWeapon(0)->WeaponType->MinimumRange <= pThis->DistanceFrom(pTarget))
								return Primary;

							return Secondary;
						}

						if (pThis->DistanceFrom(pTarget) <= ChangeRange)
							return closeweapon;
						else
							return furtherweapon;
					}
				}
			}
		}

		if (!pTargetTechno)
			return Primary;


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

	if (pTargetTechno != nullptr)
	{
		TechnoExt::ExtData* pTargetExt = TechnoExt::ExtMap.Find(pTargetTechno);
		WeaponStruct* primary = pThis->GetWeapon(0);
		WeaponStruct* secondary = pThis->GetWeapon(1);

		if (secondary != nullptr &&
			secondary->WeaponType != nullptr &&
			CustomArmor::GetVersus(secondary->WeaponType->Warhead, pTargetExt->GetArmorIdx(secondary->WeaponType->Warhead)) == 0.0)
			return Primary;

		if (primary != nullptr &&
			primary->WeaponType != nullptr &&
			CustomArmor::GetVersus(primary->WeaponType->Warhead, pTargetExt->GetArmorIdx(primary->WeaponType->Warhead)) != 0.0)
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
			CustomArmor::GetVersus(secondary->WeaponType->Warhead, pObjectType->Armor) == 0.0)
			return Primary;

		if (primary != nullptr &&
			primary->WeaponType != nullptr &&
			CustomArmor::GetVersus(primary->WeaponType->Warhead, pObjectType->Armor) != 0.0)
			return FurtherCheck;

		return Secondary;
	}

	return OriginalCheck;
}

DEFINE_HOOK(0x6F3436, TechnoClass_SelectGattlingWeapon, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(AbstractClass*, pTarget, EBP);

	enum { Odd = 0x6F346A, Even = 0x6F345C, Origin = 0 };
	int gattlingStage = pThis->CurrentGattlingStage;
	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (pTypeExt->Gattling_SelectWeaponByVersus)
	{
		WeaponTypeClass* pWeaponOdd = pThis->GetWeapon(gattlingStage << 1)->WeaponType;
		WeaponTypeClass* pWeaponEven = pThis->GetWeapon(gattlingStage << 1 | 1)->WeaponType;

		if (auto pTargetTechno = abstract_cast<TechnoClass*>(pTarget))
		{
			R->ESI(gattlingStage);

			auto pTargetExt = TechnoExt::ExtMap.Find(pTargetTechno);
			if (pTargetTechno->IsInAir())
			{
				if (!pWeaponEven->Projectile->AA)
					return Odd;

				if (CustomArmor::GetVersus(pWeaponEven->Warhead, pTargetExt->GetArmorIdx(pWeaponOdd)) != 0.0)
					return Even;

				if (CustomArmor::GetVersus(pWeaponOdd->Warhead, pTargetExt->GetArmorIdx(pWeaponEven)) == 0.0)
					return Even;

				return Odd;
			}
			else
			{
				if (!pWeaponOdd->Projectile->AG)
					return Even;

				if (CustomArmor::GetVersus(pWeaponOdd->Warhead, pTargetExt->GetArmorIdx(pWeaponOdd)) != 0.0)
					return Odd;

				if (CustomArmor::GetVersus(pWeaponEven->Warhead, pTargetExt->GetArmorIdx(pWeaponEven)) == 0.0)
					return Odd;

				return Even;
			}
		}
		else
		{
			R->ESI(gattlingStage);

			if (auto pTargetObject = abstract_cast<ObjectClass*>(pTarget))
			{
				ObjectTypeClass* pTargetType = pTargetObject->GetType();

				if (GeneralUtils::GetWarheadVersusArmor(pWeaponOdd->Warhead, pTargetType->Armor) != 0.0)
					return Odd;

				if (GeneralUtils::GetWarheadVersusArmor(pWeaponEven->Warhead, pTargetType->Armor) == 0.0)
					return Odd;

				return Even;
			}
			else
			{
				return Odd;
			}
		}
	}

	return Origin;
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
	GET_STACK(AbstractClass*, pTarget, STACK_OFFS(0x20, -0x4));
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
		const auto pTechno = abstract_cast<TechnoClass*>(pTarget);

		CellClass* targetCell = nullptr;

		if ((pThis->Passengers.NumPassengers == 0) && pWeaponExt->PassengerDeletion)
			return CannotFire;

		// Ignore target cell for airborne technos.
		if (!pTechno || !pTechno->IsInAir())
		{
			if (const auto pCell = abstract_cast<CellClass*>(pTarget))
				targetCell = pCell;
			else if (const auto pObject = abstract_cast<ObjectClass*>(pTarget))
				targetCell = pObject->GetCell();
		}

		if (targetCell)
		{
			if (!EnumFunctions::IsCellEligible(targetCell, pWeaponExt->CanTarget, true))
			{
				return CannotFire;
			}
		}

		if (pTechno)
		{
			if (!EnumFunctions::IsTechnoEligible(pTechno, pWeaponExt->CanTarget) ||
				!EnumFunctions::CanTargetHouse(pWeaponExt->CanTargetHouses, pThis->Owner, pTechno->Owner) ||
				(pWeaponExt->OnlyAllowOneFirer && TechnoExt::ExtMap.Find(pTechno)->Attacker != nullptr && pThis != TechnoExt::ExtMap.Find(pTechno)->Attacker))
			{
				return CannotFire;
			}
		}
	}

	if (pWH->MindControl)
	{
		TechnoClass* pTechno = abstract_cast<TechnoClass*>(pTarget);

		if (pTechno != nullptr)
		{
			if (!pThis->CaptureManager->CanCapture(pTechno))
				return CannotFire;

			if (pTechno->Passengers.NumPassengers > 0)
			{
				for (
					FootClass* pPassenger = pTechno->Passengers.GetFirstPassenger();
					pPassenger != nullptr;
					pPassenger = abstract_cast<FootClass*>(pPassenger->NextObject)
					)
				{
					auto pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pPassenger->GetTechnoType());

					if (pTechnoTypeExt->VehicleImmuneToMindControl.Get())
						return CannotFire;
				}
			}
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


DEFINE_HOOK(0x6FDD50, Techno_Before_Fire, 0x6)
{
	GET(TechnoClass*, pThis, ECX);
	GET_STACK(AbstractClass*, pTarget, 0x4);
	GET_STACK(int, idxWeapon, 0x8);

	WeaponTypeClass* pWeapon = pThis->GetWeapon(idxWeapon)->WeaponType;

	if (pWeapon == nullptr)
		return 0;

	auto pExt = TechnoExt::ExtMap.Find(pThis);
	bool disableAttach = false;

	for (const auto& pAE : pExt->AttachEffects)
	{
		if (pAE->Type->DisableWeapon && (pAE->Type->DisableWeapon_Category & DisableWeaponCate::Attach))
		{
			disableAttach = true;
			break;
		}
	}

	if (!disableAttach)
		WeaponTypeExt::ProcessAttachWeapons(pWeapon, pThis, pTarget);

	TechnoExt::IonCannonWeapon(pThis, pTarget, pWeapon);
	TechnoExt::BeamCannon(pThis, pTarget, pWeapon);
	TechnoExt::FirePassenger(pThis, pTarget, pWeapon);
	TechnoExt::AllowPassengerToFire(pThis, pTarget, pWeapon);
	TechnoExt::SpawneLoseTarget(pThis);
	TechnoExt::SetWeaponROF(pThis, pWeapon);
	TechnoExt::SetGattlingCount(pThis, pTarget, pWeapon);
	TechnoExt::ShareWeaponRange(pThis, pTarget, pWeapon);

	if (pTarget->AbstractFlags & AbstractFlags::Techno)
		TechnoExt::RememeberFirer(pThis, pTarget, pWeapon);

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
	GET_STACK(BulletClass* const, pBullet, STACK_OFFS(0xB0, 0x74));

	auto const pSourceTypeExt = TechnoTypeExt::ExtMap.Find(pSource->GetTechnoType());
	bool interceptor = pSourceTypeExt->Interceptor;
	if (interceptor)
	{
		bool interceptor_Rookie = pSourceTypeExt->Interceptor_Rookie.Get(true);
		bool interceptor_Veteran = pSourceTypeExt->Interceptor_Veteran.Get(true);
		bool interceptor_Elite = pSourceTypeExt->Interceptor_Elite.Get(true);

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
				int successProbability = pSourceTypeExt->Interceptor_Success;

				if (!pSource->Veterancy.IsRookie())
				{
					if (pSource->Veterancy.IsVeteran())
					{
						if (pSourceTypeExt->Interceptor_VeteranSuccess >= 0)
						{
							successProbability = pSourceTypeExt->Interceptor_VeteranSuccess;
						}
					}
					else
					{
						if (pSource->Veterancy.IsElite())
						{
							if (pSourceTypeExt->Interceptor_EliteSuccess >= 0)
							{
								successProbability = pSourceTypeExt->Interceptor_EliteSuccess;
							}
							else
							{
								if (pSourceTypeExt->Interceptor_VeteranSuccess >= 0)
								{
									successProbability = pSourceTypeExt->Interceptor_VeteranSuccess;
								}
							}
						}
					}
				}
				else
				{
					if (pSourceTypeExt->Interceptor_RookieSuccess >= 0)
					{
						successProbability = pSourceTypeExt->Interceptor_RookieSuccess;
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

DEFINE_HOOK(0x6FC689, TechnoClass_CanFire_LandNavalTarget, 0x6)
{
	enum { DisallowFiring = 0x6FC86A };

	GET(TechnoClass*, pThis, ESI);
	GET_STACK(AbstractClass*, pTarget, STACK_OFFS(0x20, -0x4));

	const auto pType = pThis->GetTechnoType();
	auto pCell = abstract_cast<CellClass*>(pTarget);

	if (pCell)
	{
		if (pType->NavalTargeting == 6 &&
			(pCell->LandType == LandType::Water || pCell->LandType == LandType::Beach))
		{
			return DisallowFiring;
		}
	}
	else if (const auto pTerrain = abstract_cast<TerrainClass*>(pTarget))
	{
		pCell = pTerrain->GetCell();

		if (pType->LandTargeting == 1 && pCell->LandType != LandType::Water && pCell->LandType != LandType::Beach)
			return DisallowFiring;
		else if (pType->NavalTargeting == 6 && (pCell->LandType == LandType::Water || pCell->LandType == LandType::Beach))
			return DisallowFiring;
	}

	return 0;
}
