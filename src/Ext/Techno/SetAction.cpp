#include "Body.h"

#include <InfantryClass.h>
#include <InputManagerClass.h>

bool TechnoExt::EngineerAllowAttack(InfantryClass* pThis, TechnoClass* pTechno, Action& action)
{
	action = Action::None;

	if (!TechnoExt::IsActive(pThis))
	{
		return false;
	}

	if (!pThis->Type->Engineer)
	{
		return false;
	}

	if (!TechnoExt::IsReallyAlive(pTechno))
	{
		return false;
	}

	auto const pExt = TechnoExt::ExtMap.Find(pThis->ConvertTechno());
	if (!pExt)
	{
		return false;
	}

	auto const pTypeExt = pExt->TypeExtData;
	if (!pTypeExt)
	{
		return false;
	}

	if (MapClass::Instance->IsLocationShrouded(pTechno->GetCoords()))
		return false;

	bool canCapture = false;
	bool canC4 = false;

	if (auto const pBld = abstract_cast<BuildingClass*>(pTechno))
	{
		canCapture = pBld->Type->Capturable && !pBld->Type->BridgeRepairHut;
		canC4 = pBld->Type->CanC4;
	}

	if (!InputManagerClass::Instance->IsForceFireKeyPressed())
	{
		if (pThis->Owner == pTechno->Owner || pThis->Owner->Allies.Contains(pTechno->Owner))
		{
			if (!pThis->Type->AttackCursorOnFriendlies)
			{
				return false;
			}
		}
		else
		{
			if (canCapture)
			{
				return false;
			}
		}
	}
	else
	{
		if (InputManagerClass::Instance->IsForceMoveKeyPressed())
		{
			return false;
		}
	}

	if (canC4 &&
		pThis->Type->C4)
	{
		action = Action::Sabotage;
		return true;
	}
	else if (pTypeExt->Engineer_UseSecondary.Get())
	{
		action = Action::Attack;
		return TechnoExt::EngineerAllowAttack_GetMultiWeapons(pThis->ConvertTechno(), pTypeExt, pTechno);
	}

	return false;
}

bool TechnoExt::EngineerAllowAttack_CanBombDisarm(TechnoClass* pThis, TechnoTypeExt::ExtData* pTypeExt, TechnoClass* pTechno)
{
	if (!TechnoExt::IsReallyAlive(pThis) || !TechnoExt::IsReallyAlive(pTechno))
		return false;

	if (!pThis->IsEngineer())
		return true;

	if (!pTypeExt || !pTypeExt->Engineer_UseSecondary.Get())
	{
		return false;
	}

	auto const Primary = pThis->GetWeapon(0) ? pThis->GetWeapon(0)->WeaponType : nullptr;
	auto const Secondary = pThis->GetWeapon(1) ? pThis->GetWeapon(1)->WeaponType : nullptr;

	if (!Primary || !Secondary)
	{
		return false;
	}

	if (pTechno->AttachedBomb)
	{
		if (!Primary->Warhead)
			return false;

		if (auto const pWHExt = WarheadTypeExt::ExtMap.Find(Primary->Warhead))
		{
			if (Primary->Warhead->BombDisarm &&
			   pWHExt->CanTargetHouse(pThis->Owner, pTechno))
			{
				return false;
			}
			else
			{
				if (Secondary && Secondary->Warhead && !Secondary->Warhead->BombDisarm)
					return true;
				else
					return false;
			}
		}
		else
		{
			return false;
		}
	}
	else
	{
		if (!Secondary->Warhead)
			return false;
		
		if (Secondary->Warhead->BombDisarm)
		{
			return false;
		}
		else
		{
			return true;
		}
	}
}

bool TechnoExt::EngineerAllowAttack_GetMultiWeapons(TechnoClass* pThis, TechnoTypeExt::ExtData* pTypeExt, TechnoClass* pTechno)
{
	if (!TechnoExt::IsReallyAlive(pThis) || !TechnoExt::IsReallyAlive(pTechno))
	{
		return false;
	}

	if (pThis->IsEngineer() && !TechnoExt::EngineerAllowAttack_CanBombDisarm(pThis, pTypeExt, pTechno))
	{
		return false;
	}

	int fireIndex = pThis->SelectWeapon(pTechno);
	auto fireError = pThis->GetFireErrorWithoutRange(pTechno, fireIndex);

	if (fireError == FireError::NONE ||
		fireError == FireError::ILLEGAL ||
		fireError == FireError::CANT)
	{
		return false;
	}

	return true;
}

bool TechnoExt::EngineerAllowEnterBuilding(InfantryClass* pThis, BuildingClass* pBuilding, Action& action)
{
	action = Action::None;

	if (!TechnoExt::IsActive(pThis))
	{
		return false;
	}

	if (!TechnoExt::IsReallyAlive(pBuilding))
	{
		return false;
	}

	if (MapClass::Instance->IsLocationShrouded(pBuilding->GetCoords()))
	{
		return false;
	}

	if ((pThis->Owner != pBuilding->Owner &&
		!pThis->Owner->Allies.Contains(pBuilding->Owner)))
	{
		if (pBuilding->Type->Capturable && !pBuilding->Type->BridgeRepairHut)
		{
			action = Action::Capture;
			return true;
		}
		else
		{
			return false;
		}
	}

	if (!InputManagerClass::Instance->IsForceMoveKeyPressed())
	{
		if (pBuilding->AttachedBomb &&
			pThis->GetWeapon(0) &&
			pThis->GetWeapon(0)->WeaponType &&
			pThis->GetWeapon(0)->WeaponType->Warhead &&
			pThis->GetWeapon(0)->WeaponType->Warhead->BombDisarm)
		{
			return false;
		}

		if (!pBuilding->Type->InfantryAbsorb ||
			pBuilding->Type->Passengers <= 0 ||
			!pBuilding->Type->Repairable)
		{
			return false;
		}

		if (pBuilding->Health < pBuilding->Type->Strength)
		{
			return false;
		}
	}

	double size = pThis->GetTechnoType()->Size;
	int count = pBuilding->Type->Passengers - pBuilding->Passengers.NumPassengers;

	if (size <= pBuilding->Type->SizeLimit && size <= count)
	{
		action = Action::Enter;
	}
	else
	{
		action = Action::NoEnter;
	}

	return true;
}
