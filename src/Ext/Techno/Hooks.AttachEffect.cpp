#include <Ext/Techno/Body.h>

// ROF
DEFINE_HOOK(0x6FD1F1, TechnoClass_GetROF, 0x5)
{
	GET(TechnoClass*, pThis, ESI);
	GET(int, iROF, EBP);

	TechnoExt::ExtData* pExt = TechnoExt::ExtMap.Find(pThis);
	int iROFBuff = 0;

	for (auto& pAE: pExt->AttachEffects)
	{
		iROF = static_cast<int>(iROF * pAE->Type->ROF_Multiplier);
		iROF = std::max(iROF, 1);
		iROFBuff += pAE->Type->ROF;
	}

	iROF += iROFBuff;
	iROF = std::max(iROF, 1);
	R->EBP(iROF);

	return 0;
}

// FirePower
DEFINE_HOOK(0x46B050, BulletTypeClass_CreateBullet, 0x6)
{
	GET_STACK(TechnoClass*, pOwner, 0x4);
	REF_STACK(int, iDamage, 0x8);

	if (pOwner == nullptr)
		return 0;

	TechnoExt::ExtData* pOwnerExt = TechnoExt::ExtMap.Find(pOwner);
	int iDamageBuff = 0;
	
	for (auto& pAE : pOwnerExt->AttachEffects)
	{
		iDamage = static_cast<int>(iDamage * pAE->Type->FirePower_Multiplier);
		iDamageBuff += pAE->Type->FirePower;
	}

	iDamage += iDamageBuff;

	return 0;
}

// Speed
DEFINE_HOOK(0x4DB221, FootClass_GetCurrentSpeed, 0x5)
{
	GET(FootClass*, pThis, ESI);
	GET(int, iSpeed, EDI);

	TechnoExt::ExtData* pExt = TechnoExt::ExtMap.Find(pThis);
	int iSpeedBuff = 0;

	for (auto& pAE : pExt->AttachEffects)
	{
		iSpeed *= static_cast<int>(pAE->Type->Speed_Multiplier);
		iSpeedBuff += pAE->Type->Speed;
	}

	iSpeed += iSpeedBuff;
	R->EDI(iSpeed);

	if (pThis->WhatAmI() != AbstractType::Unit)
		return 0x4DB23E;

	return 0x4DB226;
}

// DisableWeapon
DEFINE_HOOK(0x6FC0B0, TechnoClass_GetFireError, 0x8)
{
	GET(TechnoClass*, pThis, ECX);

	TechnoExt::ExtData* pExt = TechnoExt::ExtMap.Find(pThis);

	for (auto& pAE : pExt->AttachEffects)
	{
		if (pAE->Type->DisableWeapon)
		{
			R->EAX(FireError::CANT);
			return 0x6FC0EB;
		}
	}

	return 0;
}
