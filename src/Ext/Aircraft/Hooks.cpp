#include "Body.h"

#include <AircraftClass.h>

#include <Utilities/Macro.h>
#include <Utilities/Enum.h>

#include <Ext/Anim/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/WeaponType/Body.h>

DEFINE_HOOK(0x414BB0, AircraftClass_Update, 0x5)
{
	GET(AircraftClass*, pThis, ECX);

	if (!pThis->IsReallyAlive())
		return 0;

	auto pExt = AircraftExt::ExtMap.Find(pThis);

	if (pExt->TypeExtData != AircraftTypeExt::ExtMap.Find(pThis->Type))
		pExt->TypeExtData = AircraftTypeExt::ExtMap.Find(pThis->Type);

	auto pTypeExt = pExt->TypeExtData;

	if (pTypeExt->Fighter_AreaGuard && pThis->Owner->IsControlledByHuman())
		pExt->Aircraft_AreaGuard();

	if (pExt->CurrentTarget != pThis->Target)
	{
		pExt->AircraftClass_SetTargetFix();
		pExt->CurrentTarget = pThis->Target;
	}

	return 0;
}

DEFINE_HOOK(0x41B7F0, AircraftClass_Is_Strafe, 0x6)
{
	GET_STACK(IFlyControl*, pThis__shifted_0x6C0, 0x4);

	enum { retn = 0x41B83B };

	AircraftClass* pThis = reinterpret_cast<AircraftClass*>(reinterpret_cast<int>(pThis__shifted_0x6C0) - 0x6C0);

	if (pThis->CurrentMission == Mission::Attack
		&& pThis->Target != nullptr)
	{
		int weaponIdx = pThis->SelectWeapon(pThis->Target);
		WeaponStruct* pWeapon = pThis->GetWeapon(weaponIdx);

		if (pWeapon && pWeapon->WeaponType)
		{
			WeaponTypeClass* pWeaponType = pWeapon->WeaponType;
			const auto pWeaponTypeExt = WeaponTypeExt::ExtMap.Find(pWeaponType);

			if (pWeaponType->Projectile->ROT <= 1 && !pWeaponType->Projectile->Inviso
				|| pWeaponTypeExt->Strafing_Shots.isset())
			{
				R->EAX(true);
				return retn;
			}
		}

		R->EAX(false);
		return retn;
	}

	WeaponStruct* pWeapon = pThis->GetWeapon(0);

	if (pWeapon && pWeapon->WeaponType)
	{
		R->EAX(pWeapon->WeaponType->Projectile->ROT <= 1
			&& !pWeapon->WeaponType->Projectile->Inviso);
	}

	return retn;
}

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

	AircraftExt::ExtData* pExt = AircraftExt::ExtMap.Find(pThis);

	if (pExt->Strafe_FireCount > 0)
		pThis->MissionStatus = (int)AirAttackStatus::FireAtTarget3_Strafe;

	if (pExt->Strafe_FireCount < 0 && pThis->MissionStatus == (int)AirAttackStatus::FireAtTarget2_Strafe)
		pExt->Strafe_FireCount = 0;

	if (pExt->Strafe_FireCount >= pWeaponExt->Strafing_Shots.Get(5))
	{
		if (!pThis->Ammo)
			pThis->unknown_bool_6D2 = false;

		pExt->Strafe_FireCount = -1;
		pThis->MissionStatus = (int)AirAttackStatus::ReturnToBase;
	}

	++pExt->Strafe_FireCount;

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

DEFINE_HOOK(0x414F10, AircraftClass_AI_Trailer, 0x5)
{
	enum { SkipGameCode = 0x414F47 };

	GET(AircraftClass*, pThis, ESI);
	GET_STACK(CoordStruct, coords, STACK_OFFSET(0x40, -0xC));

	if (auto const pTrailerAnim = GameCreate<AnimClass>(pThis->Type->Trailer, coords, 1, 1))
	{
		auto const pTrailerAnimExt = AnimExt::ExtMap.Find(pTrailerAnim);
		AnimExt::SetAnimOwnerHouseKind(pTrailerAnim, pThis->Owner, nullptr, false, true);
		pTrailerAnimExt->SetInvoker(pThis);
	}

	return SkipGameCode;
}

DEFINE_HOOK(0x415EEE, AircraftClass_Fire_KickOutPassenger, 0x6)
{
	GET(AircraftClass*, pThis, EDI);
	GET_BASE(int, weaponIdx, 0xC);

	if (!TechnoExt::IsReallyAlive(pThis))
		return 0;

	if (pThis->Type->Passengers <= 0)
		return 0x415F08;

	WeaponStruct* pWeapon = pThis->GetWeapon(weaponIdx);

	if (!pWeapon || !pWeapon->WeaponType)
		return 0;

	const auto pWeaponType = pWeapon->WeaponType;

	if (auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeaponType))
	{
		if (pWeaponExt->KickOutPassenger.isset() && !pWeaponExt->KickOutPassenger)
			return 0x415F08;
	}

	return 0;
}
