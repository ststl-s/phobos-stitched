#include <Ext/Techno/Body.h>

// ROF
DEFINE_HOOK(0x6FD1F1, TechnoClass_GetROF, 0x5)
{
	GET(int, iROF, EBP);
	GET(TechnoClass*, pThis, ESI);

	TechnoExt::ExtData* pExt = TechnoExt::ExtMap.Find(pThis);

	for (auto& pAE: pExt->AttachEffects)
	{
		iROF = static_cast<int>(iROF * pAE->Type->ROF_Multiplier);
		iROF = std::max(iROF, 1);
	}

	for (auto& pAE : pExt->AttachEffects)
	{
		iROF += pAE->Type->ROF;
		iROF = std::max(iROF, 1);
	}

	R->EBP(iROF);

	return 0;
}

// FirePower
DEFINE_HOOK(0x46B050, BulletTypeClass_CreateBullet, 0x6)
{
	GET_STACK(TechnoClass*, pOwner, 0x4);
	LEA_STACK(int*, pDamage, 0x8);

	TechnoExt::ExtData* pOwnerExt = TechnoExt::ExtMap.Find(pOwner);
	
	for (auto& pAE : pOwnerExt->AttachEffects)
	{
		*pDamage = static_cast<int>(*pDamage * pAE->Type->FirePower_Multiplier);
	}

	for (auto& pAE : pOwnerExt->AttachEffects)
	{
		*pDamage += pAE->Type->FirePower;
	}

	return 0;
}
