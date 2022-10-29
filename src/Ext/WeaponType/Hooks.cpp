#include "Body.h"

#include <Ext/Scenario/Body.h>
#include <Ext/BulletType/Body.h>

DEFINE_HOOK(0x772A0A, WeaponTypeClass_SetSpeed_ApplyGravity, 0x6)
{
	GET(BulletTypeClass* const, pType, EAX);

	auto const nGravity = BulletTypeExt::GetAdjustedGravity(pType);
	__asm { fld nGravity };

	return 0x772A29;
}

DEFINE_HOOK(0x773087, WeaponTypeClass_GetSpeed_ApplyGravity, 0x6)
{
	GET(BulletTypeClass* const, pType, EAX);

	auto const nGravity = BulletTypeExt::GetAdjustedGravity(pType);
	__asm { fld nGravity };

	return 0x7730A3;
}

#pragma region Fix_WW_Strength_ReceiveDamage_C4Warhead_Misuses

// Suicide=yes behavior on WeaponTypes
DEFINE_HOOK(0x6FDDCA, TechnoClass_Fire_Suicide, 0xA)
{
	GET(TechnoClass* const, pThis, ESI);

	pThis->ReceiveDamage(&pThis->Health, 0, RulesClass::Instance->C4Warhead,
		nullptr, true, false, pThis->Owner);

	return 0x6FDE03;
}

// Kill the vxl unit when flipped over
DEFINE_HOOK(0x70BC6F, TechnoClass_UpdateRigidBodyKinematics_KillFlipped, 0xA)
{
	GET(TechnoClass* const, pThis, ESI);

	auto const pFlipper = pThis->DirectRockerLinkedUnit;
	pThis->ReceiveDamage(&pThis->Health, 0, RulesClass::Instance->C4Warhead,
		nullptr, true, false, pFlipper ? pFlipper->Owner : nullptr);

	return 0x70BCA4;
}

#ifdef REWORK_AFTER_RELEASE_V03

// What is this? why the fuck 10*Strength damage to itself
DEFINE_HOOK(0x4425C0, BuildingClass_ReceiveDamage_MaybeKillRadioLinks, 0x6)
{
	GET(TechnoClass* const, pRadio, EAX);

	pRadio->ReceiveDamage(&pRadio->Health, 0, RulesClass::Instance->C4Warhead,
		nullptr, true, true, nullptr);

	return 0x4425F4;
}

DEFINE_HOOK(0x501477, HouseClass_IHouse_AllToHunt_KillMCInsignificant, 0xA)
{
	GET(TechnoClass* const, pItem, ESI);

	pItem->ReceiveDamage(&pItem->Health, 0, RulesClass::Instance->C4Warhead,
		nullptr, true, true, nullptr);

	return 0x50150E;
}

// Something unfinished for later
DEFINE_HOOK(0x7187D2, TeleportLocomotionClass_7187A0_IronCurtainFuckMeUp, 0x8)
{
	GET(FootClass* const, pOwner, ECX);
	pOwner->ReceiveDamage(&pOwner->Health, 0, RulesClass::Instance->C4Warhead,
		nullptr, true, false, nullptr);
	return 0x71880A;
}
//718B1E

#endif
#pragma endregion Fix_WW_Strength_ReceiveDamage_C4Warhead_Misuse
