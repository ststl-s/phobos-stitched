#include "Body.h"

#include <Utilities/EnumFunctions.h>
#include <Utilities/Macro.h>

#include <Ext/TEvent/Body.h>

DEFINE_HOOK(0x701900, TechnoClass_ReceiveDamage_BeforeAll, 0x6)
{
	GET(TechnoClass*, pThis, ECX);
	LEA_STACK(args_ReceiveDamage*, args, 0x4);

	const auto pExt = TechnoExt::ExtMap.Find(pThis);
	const auto pWHExt = WarheadTypeExt::ExtMap.Find(args->WH);
	bool attackedWeaponDisabled = false;

	for (const auto& pAE : pExt->AttachEffects)
	{
		if (!pAE->IsActive())
			continue;

		if (pAE->Type->DisableWeapon && (pAE->Type->DisableWeapon_Category & DisableWeaponCate::Attacked))
		{
			attackedWeaponDisabled = true;
			break;
		}
	}

	if (!pWHExt->CanBeDodge.isset())
		pWHExt->CanBeDodge = RulesExt::Global()->Warheads_CanBeDodge;

	if (!attackedWeaponDisabled)
		TechnoExt::ProcessAttackedWeapon(pThis, args, true);

	if (!args->IgnoreDefenses)
	{
		if (args->Attacker && args->Attacker->GetTechnoType() && args->WH)
		{
			double damageMultiplier = 1.0;
			if (pExt->ReceiveDamageMultiplier != 1.0)
			{
				damageMultiplier *= pExt->ReceiveDamageMultiplier;
				pExt->ReceiveDamageMultiplier = 1.0;
			}

			*args->Damage = static_cast<int>(*args->Damage * damageMultiplier);
		}

		ShieldClass* const pShieldData = pExt->Shield.get();
		if (pShieldData != nullptr)
		{
			if (pShieldData->IsActive())
			{
				const int nDamageLeft = pShieldData->ReceiveDamage(args);

				if (nDamageLeft == 0)
					TechnoExt::ProcessAttackedWeapon(pThis, args, false);

				if (nDamageLeft >= 0)
				{
					*args->Damage = nDamageLeft;

					if (auto pTag = pThis->AttachedTag)
						pTag->RaiseEvent(static_cast<TriggerEvent>(PhobosTriggerEvent::ShieldBroken), pThis, CellStruct::Empty);
				}
			}
		}
	}

	if (pWHExt->IgnoreDefense && !pThis->IsIronCurtained())
		args->IgnoreDefenses = true;

	return 0;
}

DEFINE_HOOK(0x7019D8, TechnoClass_ReceiveDamage_SkipLowDamageCheck, 0x5)
{
	GET(TechnoClass*, pThis, ESI);
	LEA_STACK(args_ReceiveDamage*, args, STACK_OFFSET(0xC4, 0x4));

	const auto pExt = TechnoExt::ExtMap.Find(pThis);

	if (ShieldClass* const pShieldData = pExt->Shield.get())
	{
		if (pShieldData->IsActive())
			return 0x7019E3;
	}

	// Restore overridden instructions
	return *args->Damage >= 1 ? 0x7019E3 : 0x7019DD;
}

DEFINE_HOOK(0x5F53DD, ObjectClass_NoRelative, 0x8)
{
	GET(ObjectClass*, pObject, ESI);
	LEA_STACK(args_ReceiveDamage*, args, STACK_OFFSET(0x24, 0x4));

	if (TechnoClass* pThis = abstract_cast<TechnoClass*>(pObject))
	{
		const TechnoTypeClass* pType = pThis->GetTechnoType();
		const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

		if (!args->IgnoreDefenses && pTypeExt->AllowMinHealth > 0)
		{
			R->EBP(pType->Strength);

			//Ares Hook 0x5F53E5
			return 0x5F53F3;
		}
	}

	R->EAX(pObject->GetType());

	return 0x5F53E5;
}

DEFINE_HOOK(0x5F53ED, ObjectClass_ReceiveDamage_DisableComplieroptimize, 0x6)
{
	LEA_STACK(args_ReceiveDamage*, args, STACK_OFFSET(0x24, 0x4));

	return args->IgnoreDefenses ? 0x5F5416 : 0x5F53F3;
}

DEFINE_HOOK(0x5F53F3, ObjectClass_ReceiveDamage_CalculateDamage, 0x6)
{
	GET(ObjectClass*, pObject, ESI);
	LEA_STACK(args_ReceiveDamage*, args, STACK_OFFSET(0x24, 0x4));

	if (TechnoClass* pThis = abstract_cast<TechnoClass*>(pObject))
	{
		const auto pExt = TechnoExt::ExtMap.Find(pThis);
		const auto pWHExt = WarheadTypeExt::ExtMap.Find(args->WH);
		*args->Damage = MapClass::GetTotalDamage(*args->Damage, args->WH, static_cast<Armor>(pExt->GetArmorIdxWithoutShield()), args->DistanceToEpicenter);

		if (pWHExt->DistanceDamage && args->Attacker)
		{
			auto range = args->Attacker->DistanceFrom(pThis);
			auto add = (pWHExt->DistanceDamage_Add) * (range / ((pWHExt->DistanceDamage_Add_Factor) * 256));
			auto multiply = pow((pWHExt->DistanceDamage_Multiply), (range / ((pWHExt->DistanceDamage_Multiply_Factor) * 256)));

			int changedamage = static_cast<int>((*args->Damage + add) * multiply) - *args->Damage;

			if (changedamage > pWHExt->DistanceDamage_Max)
			{
				changedamage = pWHExt->DistanceDamage_Max;
			}

			if (changedamage < pWHExt->DistanceDamage_Min)
			{
				changedamage = pWHExt->DistanceDamage_Min;
			}

			*args->Damage += changedamage;
		}

		if (!pWHExt->IgnoreArmorMultiplier && !args->IgnoreDefenses && *args->Damage > 0)
		{
			double dblArmorMultiplier = 1.0;

			for (auto& pAE : pExt->AttachEffects)
			{
				if (!pAE->IsActive())
					continue;

				if (pAE->Type->Armor_Multiplier <= 1e-5)
				{
					*args->Damage = 0;

					return 0x5F5416;
				}

				dblArmorMultiplier *= pAE->Type->Armor_Multiplier;
			}

			*args->Damage = Game::F2I(*args->Damage / dblArmorMultiplier);
		}

		return 0x5F5416;
	}

	return 0;
}

DEFINE_HOOK(0x5F5416, ObjectClass_AfterDamageCalculate, 0x6)
{
	GET(ObjectClass*, pObject, ESI);
	LEA_STACK(args_ReceiveDamage*, args, STACK_OFFSET(0x24, 0x4));

	if (!(pObject->AbstractFlags & AbstractFlags::Techno))
	{
		// against compiler optimization
		*reinterpret_cast<DWORD*>(&args->IgnoreDefenses) = pObject->GetType()->Strength;
		return 0x5F5456;
	}

	TechnoClass* pThis = static_cast<TechnoClass*>(pObject);
	auto pExt = TechnoExt::ExtMap.Find(pThis);
	const TechnoTypeClass* pType = pThis->GetTechnoType();
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
	const auto pWHExt = WarheadTypeExt::ExtMap.Find(args->WH);

	if (!args->IgnoreDefenses && !pWHExt->IgnoreArmorMultiplier)
	{
		int armorBuff = 0;

		for (auto& pAE : pExt->AttachEffects)
		{
			if (!pAE->IsActive())
				continue;

			armorBuff += pAE->Type->Armor;
		}

		if (*args->Damage > 0)
			*args->Damage -= std::min(*args->Damage, armorBuff);
	}

	if (!args->IgnoreDefenses && !pWHExt->IgnoreDamageLimit)
	{
		Vector2D<int> LimitMax = pExt->LimitDamage ? pExt->AllowMaxDamage : pTypeExt->AllowMaxDamage.Get();
		Vector2D<int> LimitMin = pExt->LimitDamage ? pExt->AllowMinDamage : pTypeExt->AllowMinDamage.Get();

		if (*args->Damage >= 0)
		{
			if (*args->Damage > LimitMax.X)
				*args->Damage = LimitMax.X;
			else if (*args->Damage < LimitMin.X)
				*args->Damage = 0;
		}
		else
		{
			if (*args->Damage < LimitMax.Y)
				*args->Damage = LimitMax.Y;
			else if (*args->Damage > LimitMin.Y)
				*args->Damage = 0;
		}
	}

	if (!args->IgnoreDefenses && pWHExt->CanBeDodge)
	{
		if (EnumFunctions::CanTargetHouse(pExt->CanDodge ? pExt->Dodge_Houses : pTypeExt->Dodge_Houses, pThis->Owner, args->SourceHouse))
		{
			if (pThis->GetHealthPercentage() <= (pExt->CanDodge ? pExt->Dodge_MaxHealthPercent : pTypeExt->Dodge_MaxHealthPercent) || pThis->GetHealthPercentage() >= (pExt->CanDodge ? pExt->Dodge_MinHealthPercent : pTypeExt->Dodge_MinHealthPercent))
			{
				bool damagecheck = pExt->CanDodge ? pExt->Dodge_OnlyDodgePositiveDamage : pTypeExt->Dodge_OnlyDodgePositiveDamage;

				if (damagecheck ? *args->Damage > 0 : true )
				{
					double dice = ScenarioClass::Instance->Random.RandomDouble();
					if ((pExt->CanDodge ? pExt->Dodge_Chance : pTypeExt->Dodge_Chance) >= dice)
					{
						if (pExt->CanDodge ? pExt->Dodge_Anim : pTypeExt->Dodge_Anim)
						{
							if (auto const pAnim = GameCreate<AnimClass>(pExt->CanDodge ? pExt->Dodge_Anim : pTypeExt->Dodge_Anim, pThis->Location))
							{
								pAnim->SetOwnerObject(pThis);
								pAnim->Owner = pThis->Owner;
							}
						}

						*args->Damage = 0;
					}
				}
			}
		}
	}

	if (!args->IgnoreDefenses && args->Attacker && pWHExt->AbsorbPercent > 0 && *args->Damage > 0)
	{
		if (!(!pWHExt->IgnoreDefense && pTypeExt->ImmuneToAbsorb))
		{
			int absorbdamage = -static_cast<int>(*args->Damage * pWHExt->AbsorbPercent);
			if (pWHExt->AbsorbMax > 0 && abs(absorbdamage) > pWHExt->AbsorbMax)
				absorbdamage = -pWHExt->AbsorbMax;
			args->Attacker->TakeDamage(absorbdamage, args->SourceHouse, args->Attacker);
		}
	}

	if (!args->IgnoreDefenses && pTypeExt->TeamAffect_ShareDamage && pExt->TeamAffectActive && !pExt->TeamAffectUnits.empty())
	{
		args_ReceiveDamage tmpArgs = *args;
		int damage = *args->Damage;
		if (pTypeExt->TeamAffect_ShareDamagePercent < -1e-6)
		{
			*tmpArgs.Damage = Game::F2I(static_cast<double>(damage) / (pExt->TeamAffectUnits.size() + 1.0));
			TechnoExt::ReceiveShareDamage(pThis, &tmpArgs, pExt->TeamAffectUnits);
			*args->Damage = Game::F2I(static_cast<double>(damage) / (pExt->TeamAffectUnits.size() + 1.0));
		}
		else
		{
			*tmpArgs.Damage = Game::F2I(static_cast<double>(damage * pTypeExt->TeamAffect_ShareDamagePercent) / pExt->TeamAffectUnits.size());
			TechnoExt::ReceiveShareDamage(pThis, &tmpArgs, pExt->TeamAffectUnits);
			*args->Damage = Game::F2I(damage * (1.0 - pTypeExt->TeamAffect_ShareDamagePercent));
		}
	}

	//----------------------------------------------------------------------------------------------------------------------
	// args->IgnoreDefense is invalid

	bool ignoreDefenses = args->IgnoreDefenses;
	// against compiler optimization
	*reinterpret_cast<DWORD*>(&args->IgnoreDefenses) = pObject->GetType()->Strength;

	int allowMinHealth = pTypeExt->AllowMinHealth;

	for (const auto& pAE : pExt->AttachEffects)
	{
		if (!pAE->IsActive())
			continue;

		allowMinHealth = std::max(allowMinHealth, pAE->Type->AllowMinHealth.Get());
	}

	if (!ignoreDefenses && allowMinHealth > 0)
	{
		R->ECX(*args->Damage);

		if (*args->Damage == 0)
			return 0x5F548C;

		if (*args->Damage > 0)
		{
			if (pThis->Health - *args->Damage < allowMinHealth)
			{
				*args->Damage = std::max(0, pThis->Health - allowMinHealth);
				R->ECX(*args->Damage);

				// Ares Hook 0x545456
				// No Culling
				return 0x5F5498;
			}
		}

		return 0x5F546A;
	}

	return 0;
}

DEFINE_HOOK(0x5F5498, ObjectClass_ReceiveDamage_AfterDamageCalculate, 0xC)
{
	GET(ObjectClass*, pObject, ESI);
	LEA_STACK(args_ReceiveDamage*, args, STACK_OFFSET(0x24, 0x4));

	if (TechnoClass* pThis = abstract_cast<TechnoClass*>(pObject))
	{
		auto pExt = TechnoExt::ExtMap.Find(pThis);
		bool attackedWeaponDisabled = false;

		for (auto& pAE : pExt->AttachEffects)
		{
			if (!pAE->IsActive())
				continue;

			if (pAE->Type->DisableWeapon && (pAE->Type->DisableWeapon_Category & DisableWeaponCate::Attacked))
			{
				attackedWeaponDisabled = true;
				break;
			}
		}

		if (!attackedWeaponDisabled)
		{
			TechnoExt::ProcessAttackedWeapon(pThis, args, false);

			for (auto& pAE : pExt->AttachEffects)
			{
				if (!pAE->IsActive())
					continue;

				pAE->AttachOwnerAttackedBy(args->Attacker);
			}
		}
	}

	return 0;
}

DEFINE_HOOK(0x701DFF, TechnoClass_ReceiveDamage_FlyingStrings, 0x7)
{
	GET(TechnoClass*, pThis, ESI);
	LEA_STACK(args_ReceiveDamage*, args, STACK_OFFSET(0xC4, 0x4));
	GET(DamageState, state, EAX);

	if (Phobos::Debug_DisplayDamageNumbers && *args->Damage)
		TechnoExt::DisplayDamageNumberString(pThis, *args->Damage, false);

	if (*args->Damage)
		TechnoExt::ReceiveDamageAnim(pThis, *args->Damage);

	if (pThis->Health == 0)
		state = DamageState::NowDead;

	if (state == DamageState::NowYellow || state == DamageState::NowRed)
	{
		if (args->WH->Sparky)
		{
			const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
			const auto pBld = abstract_cast<BuildingClass*>(pThis);
			const auto pAnimTypes = pBld ? pTypeExt->OnFire.GetElements(RulesExt::Global()->OnFire) : pTypeExt->OnFire;

			if (const int size = pAnimTypes.size())
			{
				if (pBld)
				{
					const int width = pBld->Type->GetFoundationWidth();
					const int height = pBld->Type->GetFoundationHeight(false);
					auto cellAt = CellClass::Coord2Cell(pThis->Location);
					const short selfY = cellAt.Y;

					for (int x = 0; x < width; x++)
					{
						cellAt.Y = selfY;

						for (int y = 0; y < height; y++)
						{
							if (auto pCell = MapClass::Instance->GetCellAt(cellAt))
							{
								auto pAnimType = pAnimTypes[ScenarioClass::Instance->Random.RandomRanged(0, size - 1)];

								if (auto pAnim = GameCreate<AnimClass>(pAnimType, MapClass::Instance->GetRandomCoordsNear(pCell->GetCellCoords(), 96, false)))
									pAnim->SetOwnerObject(pThis);
							}

							cellAt.Y++;
						}

						cellAt.X++;
					}
				}
				else
				{
					const auto pAnimType = pAnimTypes[ScenarioClass::Instance->Random.RandomRanged(0, size - 1)];

					if (auto pAnim = GameCreate<AnimClass>(pAnimType, MapClass::Instance->GetRandomCoordsNear(pThis->Location, 96, false)))
						pAnim->SetOwnerObject(pThis);
				}
			}
		}
	}

	return 0;
}

DEFINE_JUMP(LJMP, 0x44270B, 0x4428FE) // Skip Origin Building OnFire Code

DEFINE_HOOK(0x70265F, TechnoClass_ReceiveDamage_Explodes, 0x6)
{
	enum { SkipKillingPassengers = 0x702669 };

	GET(TechnoClass*, pThis, ESI);

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (!pTypeExt->Explodes_KillPassengers)
		return SkipKillingPassengers;

	return 0;
}
