#include "Body.h"
#include <New/Armor/Armor.h>
#include <Utilities/GeneralUtils.h>
#include <Misc/CaptureManager.h>
#include <Utilities/EnumFunctions.h>

bool TechnoExt::ExtData::SelectSpecialWeapon(AbstractClass* pTarget)
{
	const auto pThis = this->OwnerObject();
	const auto pExt = this;
	const auto pTypeExt = pExt->TypeExtData;

	if (!TechnoExt::IsReallyAlive(pThis) || !pExt || !pTypeExt)
		return false;

	auto SetZero = [&]()
	{
		if (pExt->TargetType >= 0)
			pExt->TargetType = -1;

		if (pExt->TargetType_FireIdx >= 0)
			pExt->TargetType_FireIdx = -1;
	};

	if (!pTarget || abstract_cast<CellClass*>(pTarget))
	{
		if (pExt->CurrentTarget)
			pExt->CurrentTarget = nullptr;

		SetZero();
		return false;
	}

	if (!pTypeExt->UseWeapons.Get() || pThis->GetTechnoType()->WeaponCount <= 0)
	{
		if (pExt->CurrentTarget)
			pExt->CurrentTarget = nullptr;

		SetZero();
		return false;
	}

	if (pThis->GetTechnoType()->IsGattling ||
		pThis->GetTechnoType()->IsChargeTurret ||
		pTypeExt->IsExtendGattling.Get() ||
		(pThis->GetTechnoType()->Gunner && pThis->GetTechnoType()->TurretCount > 0))
	{
		if (pExt->CurrentTarget)
			pExt->CurrentTarget = nullptr;

		SetZero();
		return false;
	}

	if (pExt->CurrentTarget == pTarget)
		return true;
	else
		pExt->CurrentTarget = pTarget;

	pExt->TargetType_FireIdx = pThis->SelectWeapon(pTarget);
	if (pExt->TargetType_FireIdx < 0)
	{
		SetZero();
		return false;
	}

	ValueableVector<int>* WeaponIdx = nullptr;
	if (const auto pTechno = abstract_cast<TechnoClass*>(pTarget))
	{
		if (!TechnoExt::IsReallyAlive(pTechno))
		{
			SetZero();
			return false;
		}

		if (pExt->TargetType_FireIdx == 0)
		{
			switch (pTechno->WhatAmI())
			{
			case AbstractType::Infantry:
				WeaponIdx = pTechno->IsInAir() ? &pTypeExt->NewWeapon_Primary_Infantry_AIR :
					&pTypeExt->NewWeapon_Primary_Infantry;
				break;
			case AbstractType::Unit:
				WeaponIdx = pTechno->IsInAir() ? &pTypeExt->NewWeapon_Primary_Unit_AIR :
					&pTypeExt->NewWeapon_Primary_Unit;
				break;
			case AbstractType::Aircraft:
				WeaponIdx = pTechno->IsInAir() ? &pTypeExt->NewWeapon_Primary_Aircraft_AIR :
					&pTypeExt->NewWeapon_Primary_Aircraft;
				break;
			case AbstractType::Building:
				WeaponIdx = &pTypeExt->NewWeapon_Primary_Building;
				break;
			default:
				break;
			}
		}
		else if (pExt->TargetType_FireIdx == 1)
		{
			switch (pTechno->WhatAmI())
			{
			case AbstractType::Infantry:
				WeaponIdx = pTechno->IsInAir() ? &pTypeExt->NewWeapon_Secondary_Infantry_AIR :
					&pTypeExt->NewWeapon_Secondary_Infantry;
				break;
			case AbstractType::Unit:
				WeaponIdx = pTechno->IsInAir() ? &pTypeExt->NewWeapon_Secondary_Unit_AIR :
					&pTypeExt->NewWeapon_Secondary_Unit;
				break;
			case AbstractType::Aircraft:
				WeaponIdx = pTechno->IsInAir() ? &pTypeExt->NewWeapon_Secondary_Aircraft_AIR :
					&pTypeExt->NewWeapon_Secondary_Aircraft;
				break;
			case AbstractType::Building:
				WeaponIdx = &pTypeExt->NewWeapon_Secondary_Building;
				break;
			default:
				break;
			}
		}
	}

	if (!WeaponIdx || WeaponIdx->empty())
	{
		if (pExt->TargetType_FireIdx == 0)
		{
			WeaponIdx = &pTypeExt->NewWeapon_Primary_All;
		}
		else if (pExt->TargetType_FireIdx == 1)
		{
			WeaponIdx = &pTypeExt->NewWeapon_Secondary_All;
		}
	}

	if (!WeaponIdx || WeaponIdx->empty())
	{
		SetZero();
		return false;
	}

	bool getWeapon = false;
	for (int i = 0; i < WeaponIdx->size(); i++)
	{
		int weaponIdx = WeaponIdx->at(i);
		if (weaponIdx <= 0 || pThis->GetTechnoType()->WeaponCount < weaponIdx)
			continue;

		int idx = weaponIdx - 1;
		const auto pNewWeapon = &pTypeExt->NewWeapons.Get(idx, pThis);
		if (!pNewWeapon || !pNewWeapon->WeaponType)
			continue;

		if (!pExt->CheckSpecialWeapon(pTarget, pNewWeapon->WeaponType))
			continue;

		pExt->TargetType = idx;
		getWeapon = true;
		break;
	}

	if (!getWeapon)
	{
		SetZero();
	}

	return getWeapon;
}

bool TechnoExt::ExtData::CheckSpecialWeapon(AbstractClass* pTarget, WeaponTypeClass* pNewWeapon)
{
	const auto pThis = this->OwnerObject();
	if (!TechnoExt::IsReallyAlive(pThis))
		return false;

	if (!pNewWeapon || !pNewWeapon->Projectile || !pNewWeapon->Warhead)
		return false;

	if (pThis->InOpenToppedTransport)
	{
		if (!pNewWeapon->FireInTransport)
			return false;
	}

	if (pNewWeapon->MinimumRange >= 0 &&
		pThis->DistanceFrom(pTarget) < pNewWeapon->MinimumRange)
		return false;

	if (const auto pWHExt = WarheadTypeExt::ExtMap.Find(pNewWeapon->Warhead))
	{
		const int nMoney = pWHExt->TransactMoney;
		if (nMoney < 0 && pThis->Owner->Available_Money() < -nMoney)
			return false;
	}

	const auto pTechno = abstract_cast<TechnoClass*>(pTarget);

	if (const auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pNewWeapon))
	{
		if (pThis->WhatAmI() == AbstractType::Unit &&
			pWeaponExt->KickOutPassenger.isset() &&
			pWeaponExt->KickOutPassenger)
		{
			if (pThis->GetTechnoType()->Passengers <= 0 || pThis->Passengers.NumPassengers <= 0)
				return false;
		}

		CellClass* pTargetCell = nullptr;

		if (TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType())->LimitedAttackRange)
		{
			TechnoExt::ExtMap.Find(pThis)->AttackWeapon = pNewWeapon;
			if (pThis->DistanceFrom(pTarget) > (pNewWeapon->Range))
				return false;
		}

		if ((pThis->Passengers.NumPassengers == 0) && pWeaponExt->PassengerDeletion)
			return false;

		// Ignore target cell for airborne technos.
		if (!pTechno || !pTechno->IsInAir())
		{
			if (const auto pCell = abstract_cast<CellClass*>(pTarget))
				pTargetCell = pCell;
			else if (const auto pObject = abstract_cast<ObjectClass*>(pTarget))
				pTargetCell = pObject->GetCell();
		}

		if (pTargetCell)
		{
			if (!EnumFunctions::IsCellEligible(pTargetCell, pWeaponExt->CanTarget, true))
				return false;
		}

		if (pTechno)
		{
			const auto pTargetExt = TechnoExt::ExtMap.Find(pTechno);

			for (const auto pAE : pTargetExt->GetActiveAE())
			{
				if (pAE->Type->DisableBeTarget)
					return false;
			}

			if (!EnumFunctions::IsTechnoEligible(pTechno, pWeaponExt->CanTarget) ||
				!EnumFunctions::CanTargetHouse(pWeaponExt->CanTargetHouses, pThis->Owner, pTechno->Owner))
			{
				return false;
			}

			if (!pWeaponExt->HasRequiredAttachedEffects(pTechno, pThis))
				return false;
		}
	}

	if (pTechno)
	{
		if (!TechnoExt::IsReallyAlive(pTechno))
			return false;

		if (pNewWeapon->Warhead->ElectricAssault)
		{
			if (pThis->Owner != pTechno->Owner && !pThis->Owner->Allies.Contains(pTechno->Owner))
				return false;

			if (const auto pBld = abstract_cast<BuildingClass*>(pTechno))
			{
				if (!pBld->Type->Overpowerable)
					return false;
			}
			else
			{
				return false;
			}
		}

		if (pNewWeapon->Warhead->IvanBomb)
		{
			if (pTechno->AttachedBomb)
				return false;
		}

		if (pNewWeapon->Warhead->BombDisarm && !pTechno->AttachedBomb)
		{
			return false;
		}

		if (pNewWeapon->Warhead->Airstrike &&
			pTechno->GetTechnoType()->WhatAmI() != AbstractType::UnitType &&
			pTechno->GetTechnoType()->WhatAmI() != AbstractType::BuildingType)
		{
			return false;
		}

		if (pNewWeapon->Warhead->MindControl)
		{
			if (pTechno != nullptr)
			{
				if (!CaptureManager::CanCapture(pThis->CaptureManager, pTechno))
					return false;
			}
		}

		if (auto pTargetExt = TechnoExt::ExtMap.Find(pTechno))
		{
			if (fabs(CustomArmor::GetVersus(pNewWeapon->Warhead, pTargetExt->GetArmorIdx(pNewWeapon->Warhead)) < 1e-6))
				return false;
		}
	}
	else
	{
		if (pNewWeapon->Warhead->ElectricAssault)
		{
			return false;
		}

		if (pNewWeapon->Warhead->IvanBomb)
		{
			return false;
		}

		if (pNewWeapon->Warhead->BombDisarm)
		{
			return false;
		}

		if (pNewWeapon->Warhead->Airstrike)
		{
			return false;
		}

		if (pNewWeapon->Warhead->MindControl)
		{
			return false;
		}

		if (auto const pObj = abstract_cast<ObjectClass*>(pTarget))
		{
			if (fabs(CustomArmor::GetVersus(pNewWeapon->Warhead, pObj->GetType()->Armor)) < 1e-6)
			{
				return false;
			}
		}
	}

	return true;
}
