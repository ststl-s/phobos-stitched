#include "Aircraft.h"

#include <AircraftClass.h>

#include <Utilities/Macro.h>
#include <Utilities/Enum.h>

#include <Ext/Anim/Anim.h>
#include <Ext/Techno/Body.h>
#include <Ext/WeaponType/Body.h>

DEFINE_HOOK(0x417FF1, AircraftClass_Mission_Attack_StrafeShots, 0x6)
{
	GET(AircraftClass*, pThis, ESI);

	if (!TechnoExt::IsReallyAlive(pThis))
		return 0;

	if (pThis->MissionStatus < (int)AirAttackStatus::FireAtTarget2_Strafe
		|| pThis->MissionStatus >(int)AirAttackStatus::FireAtTarget5_Strafe)
	{
		return 0;
	}

	int weaponIndex = pThis->SelectWeapon(pThis->Target);

	if (!pThis->GetWeapon(weaponIndex))
		return 0;

	if (!pThis->GetWeapon(weaponIndex)->WeaponType)
		return 0;

	const auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pThis->GetWeapon(weaponIndex)->WeaponType);
	if (!pWeaponExt)
		return 0;

	int fireCount = pThis->MissionStatus - 4;

	if (fireCount > 1 && pWeaponExt->Strafing_Shots < fireCount)
	{
		if (!pThis->Ammo)
			pThis->unknown_bool_6D2 = false;

		pThis->MissionStatus = (int)AirAttackStatus::ReturnToBase;
	}

	return 0;
}

DEFINE_HOOK(0x418403, AircraftClass_Mission_Attack_FireAtTarget_BurstFix, 0x8)
{
	GET(AircraftClass*, pThis, ESI);

	if (!TechnoExt::IsReallyAlive(pThis))
		return 0;

	pThis->unknown_bool_6C8 = true;

	AircraftExt::FireBurst(pThis, pThis->Target, 0);

	return 0x418478;
}

DEFINE_HOOK(0x4186B6, AircraftClass_Mission_Attack_FireAtTarget2_BurstFix, 0x8)
{
	GET(AircraftClass*, pThis, ESI);

	if (!TechnoExt::IsReallyAlive(pThis))
		return 0;

	AircraftExt::FireBurst(pThis, pThis->Target, 0);

	return 0x4186D7;
}

DEFINE_HOOK(0x418805, AircraftClass_Mission_Attack_FireAtTarget2Strafe_BurstFix, 0x8)
{
	GET(AircraftClass*, pThis, ESI);

	if (!TechnoExt::IsReallyAlive(pThis))
		return 0;

	AircraftExt::FireBurst(pThis, pThis->Target, 1);

	return 0x418826;
}

DEFINE_HOOK(0x418914, AircraftClass_Mission_Attack_FireAtTarget3Strafe_BurstFix, 0x8)
{
	GET(AircraftClass*, pThis, ESI);

	if (!TechnoExt::IsReallyAlive(pThis))
		return 0;

	AircraftExt::FireBurst(pThis, pThis->Target, 2);

	return 0x418935;
}

DEFINE_HOOK(0x418A23, AircraftClass_Mission_Attack_FireAtTarget4Strafe_BurstFix, 0x8)
{
	GET(AircraftClass*, pThis, ESI);

	if (!TechnoExt::IsReallyAlive(pThis))
		return 0;

	AircraftExt::FireBurst(pThis, pThis->Target, 3);

	return 0x418A44;
}

DEFINE_HOOK(0x418B1F, AircraftClass_Mission_Attack_FireAtTarget5Strafe_BurstFix, 0x8)
{
	GET(AircraftClass*, pThis, ESI);

	if (!TechnoExt::IsReallyAlive(pThis))
		return 0;

	AircraftExt::FireBurst(pThis, pThis->Target, 4);

	return 0x418B40;
}

DEFINE_HOOK(0x414F47, AircraftClass_AI_TrailerInheritOwner, 0x6)
{
	GET(AircraftClass*, pThis, ESI);
	GET(AnimClass*, pAnim, EAX);

	if (!TechnoExt::IsReallyAlive(pThis))
		return 0;

	if (!pAnim)
		return 0;

	if (auto const pAnimExt = AnimExt::ExtMap.Find(pAnim))
	{
		pAnim->Owner = pThis->Owner;
		pAnimExt->Invoker = pThis;
	}

	return 0;
}

DEFINE_HOOK(0x415EEE, AircraftClass_Fire_KickOutPassenger, 0x6)
{
	GET(AircraftClass*, pThis, EDI);

	if (!TechnoExt::IsReallyAlive(pThis))
		return 0;

	if (pThis->Type->Passengers <= 0)
		return 0x415F08;

	int weaponIdx = 0;
	if (pThis->Target)
		weaponIdx = pThis->SelectWeapon(pThis->Target);

	if (weaponIdx <= -1)
		return 0;

	if (!pThis->GetWeapon(weaponIdx))
		return 0;

	if (!pThis->GetWeapon(weaponIdx)->WeaponType)
		return 0;

	const auto pWeaponType = pThis->GetWeapon(weaponIdx)->WeaponType;
	if (auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeaponType))
	{
		if (!pWeaponExt->KickOutPassenger.Get())
			return 0x415F08;
	}

	return 0;
}
