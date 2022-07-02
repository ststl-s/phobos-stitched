#include "Body.h"
#include <Ext/WarheadType/Body.h>

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
	if (pWHExt->IgnoreArmorMultiplier)
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

DEFINE_HOOK(0x701DCC, TechnoClass_ReceiveDamage_Before_Damage, 0x7)
{
	if (!bOriginIgnoreDefense && pThis->Health - *args->Damage < pTypeExt->AllowMinHealth)
		*args->Damage = pThis->Health - pTypeExt->AllowMinHealth;

	return 0;
}

DEFINE_HOOK(0x5F5498, ObjectClass_ReceiveDamage_AfterDamageCalculate, 0xC)
{
	GET(ObjectClass*, pObject, ESI);

	if (!(pObject->AbstractFlags & AbstractFlags::Techno))
		return 0;

	TechnoExt::ProcessAttackedWeapon(pThis, args, false);

	return 0;
}