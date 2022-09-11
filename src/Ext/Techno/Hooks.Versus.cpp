#include <Ext/Techno/Body.h>

#include <New/Armor/Armor.h>

#include <Utilities/Macro.h>

DEFINE_HOOK(0x708AF1, TechnoClass_ShouldRetaliate_Versus, 0x6)
{
	enum { Should = 0x708B0B, Not = 0x708B17 };

	GET(WeaponTypeClass*, pWeapon, ESI);
	GET(TechnoClass*, pAttacker, EBP);

	auto pAttackerExt = TechnoExt::ExtMap.Find(pAttacker);
	int armorIdx = pAttackerExt->GetArmorIdx(pWeapon);
	auto pWHExt = WarheadTypeExt::ExtMap.Find(pWeapon->Warhead);

	if (pWHExt->Versus_Retaliate.count(armorIdx) && !pWHExt->Versus_Retaliate[armorIdx])
		return Not;

	double versus = CustomArmor::GetVersus(pWeapon->Warhead, armorIdx);

	if (fabs(versus) <= 0.01)
		return Not;

	return Should;
}

DEFINE_HOOK(0x6FCB64, TechnoClass_CanFire_Versus, 0x6)
{
	enum { CanFire = 0x6FCB8D, CanNotFire = 0x6FCB7E };

	GET(WarheadTypeClass*, pWH, EDI);
	GET(TechnoClass*, pTarget, EBP);

	auto pTargetExt = TechnoExt::ExtMap.Find(pTarget);
	double versus = CustomArmor::GetVersus(pWH, pTargetExt->GetArmorIdx(pWH));

	if (versus == 0.0)
		return CanNotFire;

	return CanFire;
}

DEFINE_HOOK(0x6F7D31, TechnoClass_CanAutoTargetObject_Versus, 0x6)
{
	enum { CanTarget = 0x6F7D55, CanNotTarget = 0x6F894F };

	GET(WeaponTypeClass*, pWeapon, EBP);
	GET(TechnoClass*, pTarget, ESI);

	auto pTargetExt = TechnoExt::ExtMap.Find(pTarget);
	int armorIdx = pTargetExt->GetArmorIdx(pWeapon);
	auto pWHExt = WarheadTypeExt::ExtMap.Find(pWeapon->Warhead);

	if (pWHExt->Versus_PassiveAcquire.count(armorIdx) && !pWHExt->Versus_PassiveAcquire[armorIdx])
		return CanNotTarget;
	
	double versus = CustomArmor::GetVersus(pWeapon->Warhead, armorIdx);

	if (fabs(versus) <= 0.02)
		return CanNotTarget;

	return CanTarget;
}

DEFINE_HOOK(0x70CE96, TechnoClass_EvalThreatRating_Versus1, 0x6)
{
	GET(TechnoClass*, pThis, EDI);
	GET(TechnoClass*, pTarget, ESI);
	GET(WeaponTypeClass*, pWeapon, EAX);
	GET_STACK(double, dbl_unknown, 0x18);

	auto pExt = TechnoExt::ExtMap.Find(pThis);
	double tmp = dbl_unknown * CustomArmor::GetVersus(pWeapon->Warhead, pExt->GetArmorIdx(pWeapon)) * (pTarget->Target == pThis ? -1 : 1);

	R->Stack(0x10, tmp);

	return 0x70CED2;
}

DEFINE_HOOK(0x70CF39, TechnoClass_EvalThreatRating, 0x7)
{
	GET(WeaponTypeClass*, pWeapon, EBX);
	GET(TechnoClass*, pTarget, ESI);
	GET_STACK(double, val, 0x30);
	REF_STACK(double, v46, 0x10);

	if (auto pTargetExt = TechnoExt::ExtMap.Find(pTarget))
	{
		v46 = CustomArmor::GetVersus(pWeapon->Warhead, pTargetExt->GetArmorIdx(pWeapon)) * val + v46;
	}

	return 0x70CF58;
}

DEFINE_HOOK(0x48922F, MapClass_GetTotalDamage, 0x6)
{
	GET(WarheadTypeClass*, pWH, EDI);
	GET(int, damage, ESI);
	GET(int, armorIdx, EDX);

	double versus = CustomArmor::GetVersus(pWH, armorIdx);
	double totalDamage = damage * versus;

	R->EAX(Game::F2I(totalDamage));

	return 0x489249;
}
