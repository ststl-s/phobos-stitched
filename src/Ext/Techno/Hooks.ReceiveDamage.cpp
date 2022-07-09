#include "Body.h"
#include <Ext/WarheadType/Body.h>
#include <Utilities/EnumFunctions.h>

args_ReceiveDamage* args;
TechnoClass* pThis;
TechnoTypeClass* pType;
TechnoExt::ExtData* pExt;
TechnoTypeExt::ExtData* pTypeExt;
WarheadTypeExt::ExtData* pWHExt;
bool bOriginIgnoreDefense;

DEFINE_HOOK(0x701900, TechnoClass_ReceiveDamage_BeforeAll, 0x6)
{
	GET(TechnoClass*, tmp_pThis, ECX);
	LEA_STACK(args_ReceiveDamage*, tmp_Args, 0x4);

	args = tmp_Args;
	pThis = tmp_pThis;
	pType = pThis->GetTechnoType();
	pExt = TechnoExt::ExtMap.Find(pThis);
	pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
	pWHExt = WarheadTypeExt::ExtMap.Find(args->WH);
	bOriginIgnoreDefense = args->IgnoreDefenses;

	enum { Nothing = 0x702D1F };

	if (!pWHExt->CanBeDodge.isset())
		pWHExt->CanBeDodge = RulesExt::Global()->Warheads_CanBeDodge;

	TechnoExt::ProcessAttackedWeapon(pThis, args, true);

	if (!args->IgnoreDefenses)
	{
		ShieldClass* const pShieldData = pExt->Shield.get();
		if (pShieldData != nullptr)
		{
			if (pShieldData->IsActive())
			{
				const int nDamageLeft = pShieldData->ReceiveDamage(args);

				if (nDamageLeft == 0)
					TechnoExt::ProcessAttackedWeapon(pThis, args, false);

				if (nDamageLeft >= 0)
					*args->Damage = nDamageLeft;
			}
		}
	}

	if (pWHExt->IgnoreDefense && !pThis->IsIronCurtained())
		args->IgnoreDefenses = true;

	return 0;
}

DEFINE_HOOK(0x70192B, TechnoClass_ReceiveDamage_BeforeCalculateArmor, 0x6)
{
	if (pWHExt->IgnoreArmorMultiplier || args->IgnoreDefenses || *args->Damage < 0)
		return 0x701A3B;

	return 0;
}

DEFINE_HOOK(0x7019D8, TechnoClass_ReceiveDamage_SkipLowDamageCheck, 0x5)
{
	if (ShieldClass* const pShieldData = pExt->Shield.get())
	{
		if (pShieldData->IsActive())
			return 0x7019E3;
	}

	// Restore overridden instructions
	return *args->Damage >= 1 ? 0x7019E3 : 0x7019DD;
}

//DEFINE_HOOK(0x701DCC, TechnoClass_ReceiveDamage_Before_Damage, 0x7)
//{
//	if (!bOriginIgnoreDefense && *args->Damage > 0
//		&& pTypeExt->AllowMinHealth.isset()
//		&& pTypeExt->AllowMinHealth > 0
//		&& pThis->Health - *args->Damage < pTypeExt->AllowMinHealth)
//	{
//		if (GeneralUtils::GetWarheadVersusArmor(args->WH, pType->Armor) >= 0.01)
//		{
//			*args->Damage = std::max(0, pThis->Health - pTypeExt->AllowMinHealth);
//		}
//	}
//
//	return 0;
//}

DEFINE_HOOK(0x5F53DD, ObjectClass_NoRelative, 0x8)
{
	GET(ObjectClass*, pObject, ESI);

	if (!(pObject->AbstractFlags & AbstractFlags::Techno))
	{
		R->EAX(pObject->GetType());
		return 0x5F53E5;
	}

	if (!bOriginIgnoreDefense && pTypeExt->AllowMinHealth.isset() && pTypeExt->AllowMinHealth.Get() > 0)
	{
		R->EBP(pType->Strength);

		//Ares Hook 0x5F53E5
		return 0x5F53EB;
	}

	R->EAX(pType);
	return 0x5F53E5;
}

DEFINE_HOOK(0x5F5416, ObjectClass_AllowMinHealth, 0x6)
{
	GET(ObjectClass*, pObject, ESI);

	if (!(pObject->AbstractFlags & AbstractFlags::Techno))
		return 0x5F5456;

	if (!pWHExt->IgnoreDefense && !pWHExt->IgnoreDamageLimit && !args->IgnoreDefenses)
	{
		R->ECX(*args->Damage);

		Vector2D<int> LimitMax = pExt->LimitDamage ? pExt->AllowMaxDamage.Get() : pTypeExt->AllowMaxDamage.Get();
		Vector2D<int> LimitMin = pExt->LimitDamage ? pExt->AllowMinDamage.Get() : pTypeExt->AllowMinDamage.Get();

		if (*args->Damage >= 0)
		{
			if (*args->Damage > LimitMax.X)
			{
				*args->Damage = LimitMax.X;
				R->ECX(*args->Damage);
			}
			else if (*args->Damage < LimitMin.X)
			{
				*args->Damage = 0;
				R->ECX(*args->Damage);
			}
		}
		else
		{
			if (*args->Damage < LimitMax.Y)
			{
				*args->Damage = LimitMax.Y;
				R->ECX(*args->Damage);
			}
			else if (*args->Damage > LimitMin.Y)
			{
				*args->Damage = 0;
				R->ECX(*args->Damage);
			}
		}
	}

	if (!pWHExt->IgnoreDefense && pWHExt->CanBeDodge && !args->IgnoreDefenses)
	{
		if (EnumFunctions::CanTargetHouse(pExt->CanDodge ? pExt->Dodge_Houses : pTypeExt->Dodge_Houses, args->SourceHouse, pThis->Owner))
		{
			if (pThis->GetHealthPercentage() <= (pExt->CanDodge ? pExt->Dodge_MaxHealthPercent : pTypeExt->Dodge_MaxHealthPercent) || pThis->GetHealthPercentage() >= (pExt->CanDodge ? pExt->Dodge_MinHealthPercent : pTypeExt->Dodge_MinHealthPercent))
			{
				R->ECX(*args->Damage);
				double dice = ScenarioClass::Instance->Random.RandomDouble();
				if ((pExt->CanDodge ? pExt->Dodge_Chance : pTypeExt->Dodge_Chance) >= dice)
				{
					if (pExt->CanDodge ? pExt->Dodge_Anim : pTypeExt->Dodge_Anim)
						GameCreate<AnimClass>(pExt->CanDodge ? pExt->Dodge_Anim : pTypeExt->Dodge_Anim, pThis->Location);

					*args->Damage = 0;
					R->ECX(*args->Damage);
				}
			}
		}
	}

	if (args->Attacker && pWHExt->AbsorbPercent > 0 && *args->Damage > 0 && !args->IgnoreDefenses)
	{
		if (!(!pWHExt->IgnoreDefense && pTypeExt->ImmuneToAbsorb))
		{
			int absorbdamage = -(*args->Damage * pWHExt->AbsorbPercent);
			if (pWHExt->AbsorbMax > 0 && abs(absorbdamage) > pWHExt->AbsorbMax)
				absorbdamage = -pWHExt->AbsorbMax;
			args->Attacker->ReceiveDamage(&absorbdamage, 0, RulesClass::Instance()->C4Warhead, args->Attacker, true, false, args->SourceHouse);
		}
	}

	if (!bOriginIgnoreDefense && pTypeExt->AllowMinHealth.isset() && pTypeExt->AllowMinHealth.Get() > 0)
	{
		R->ECX(*args->Damage);

		if (*args->Damage == 0)
			return 0x5F548C;

		if (*args->Damage > 0)
		{
			if (pThis->Health - *args->Damage < pTypeExt->AllowMinHealth)
			{
				*args->Damage = std::max(0, pThis->Health - pTypeExt->AllowMinHealth);
				R->ECX(*args->Damage);

				// Ares Hook 0x545456
				// No Culling
				return 0x5F5498;
			}
		}

		return 0x5F546A;
	}

	return 0;
	//return *args->Damage == 0 ? 0x5F548C : 0x5F5456;
}

DEFINE_HOOK(0x5F5498, ObjectClass_ReceiveDamage_AfterDamageCalculate, 0xC)
{
	GET(ObjectClass*, pObject, ESI);

	if (!(pObject->AbstractFlags & AbstractFlags::Techno))
		return 0;

	TechnoExt::ProcessAttackedWeapon(pThis, args, false);

	return 0;
}
