#include <Utilities/EnumFunctions.h>

#include <Ext/Techno/Body.h>

// ROF
DEFINE_HOOK(0x6FD1F1, TechnoClass_GetROF, 0x5)
{
	GET(TechnoClass*, pThis, ESI);
	GET(int, iROF, EBP);

	TechnoExt::ExtData* pExt = nullptr;

	if (pThis->Transporter != nullptr)
		pExt = TechnoExt::ExtMap.Find(pThis->Transporter);
	else
		pExt = TechnoExt::ExtMap.Find(pThis);

	int iROFBuff = 0;
	double dblMultiplier = 1.0;

	if (pExt->BuildingROFFix > 0)
	{
		iROF = pExt->BuildingROFFix;
		pExt->BuildingROFFix = -1;
	}

	for (const auto& pAE: pExt->AttachEffects)
	{
		if (!pAE->IsActive())
			continue;

		dblMultiplier *= pAE->Type->ROF_Multiplier;
		iROFBuff += pAE->Type->ROF;
	}

	iROF = Game::F2I(iROF * dblMultiplier);
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

	const TechnoExt::ExtData* pTechnoExt = nullptr;

	if (pOwner->Transporter != nullptr)
		pTechnoExt = TechnoExt::ExtMap.Find(pOwner->Transporter);
	else
		pTechnoExt = TechnoExt::ExtMap.Find(pOwner);

	if (pOwner == nullptr || pTechnoExt == nullptr)
		return 0;

	double dblMultiplier = 1.0;
	int iDamageBuff = 0;
	
	for (const auto& pAE : pTechnoExt->AttachEffects)
	{
		if (!pAE->IsActive())
			continue;

		dblMultiplier *= pAE->Type->FirePower_Multiplier;
		iDamageBuff += pAE->Type->FirePower;
	}

	iDamage = Game::F2I(iDamage * dblMultiplier);
	iDamage += iDamageBuff;

	return 0;
}

// Speed
DEFINE_HOOK(0x4DB221, FootClass_GetCurrentSpeed, 0x5)
{
	GET(FootClass*, pThis, ESI);
	GET(int, iSpeed, EDI);

	TechnoExt::ExtData* pExt = TechnoExt::ExtMap.Find(pThis);

	double dblMultiplier = 1.0;
	int iSpeedBuff = 0;

	for (const auto& pAE : pExt->AttachEffects)
	{
		if (!pAE->IsActive())
		{
			continue;
		}

		dblMultiplier *= pAE->Type->Speed_Multiplier;
		iSpeedBuff += pAE->Type->Speed;
	}

	iSpeedBuff = iSpeedBuff * 256 / 100;
	iSpeed = Game::F2I(iSpeed * dblMultiplier);
	iSpeed += iSpeedBuff;
	iSpeed = std::max(0, iSpeed);
	iSpeed = std::min(256, iSpeed);
	R->EDI(iSpeed);

	if (pThis->WhatAmI() != AbstractType::Unit)
		return 0x4DB23E;

	return 0x4DB226;
}

// Range
DEFINE_HOOK(0x6F7248, TechnoClass_InRange, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EBX);

	int range = pWeapon->Range;
	double dblRangeMultiplier = 1.0;
	auto pExt = TechnoExt::ExtMap.Find(pThis);

	for (const auto& pAE : pExt->AttachEffects)
	{
		if (!pAE->IsActive())
			continue;

		range += pAE->Type->Range;
		dblRangeMultiplier *= pAE->Type->Range_Multiplier;
	}

	range = Game::F2I(range * dblRangeMultiplier);
	R->EDI(range);

	return 0x6F724E;
}

// DisableWeapon
DEFINE_HOOK(0x6FC0B0, TechnoClass_GetFireError, 0x8)
{
	GET(TechnoClass*, pThis, ECX);
	GET_STACK(int, weaponIdx, 0x8);

	if (pThis->Transporter != nullptr)
	{
		TechnoExt::ExtData* pExt = TechnoExt::ExtMap.Find(pThis->Transporter);

		for (const auto& pAE : pExt->AttachEffects)
		{
			if (!pAE->IsActive())
				continue;

			if (pAE->Type->DisableWeapon && (pAE->Type->DisableWeapon_Category & DisableWeaponCate::Passenger))
			{
				R->EAX(FireError::CANT);
				return 0x6FC0EB;
			}
		}
	}
	else
	{
		TechnoExt::ExtData* pExt = TechnoExt::ExtMap.Find(pThis);

		for (const auto& pAE : pExt->AttachEffects)
		{
			if (!pAE->IsActive())
				continue;

			if (pAE->Type->DisableWeapon)
			{
				if (EnumFunctions::IsWeaponDisabled(pThis, pAE->Type->DisableWeapon_Category, weaponIdx))
				{
					R->EAX(FireError::CANT);
					return 0x6FC0EB;
				}
			}
		}
	}

	return 0;
}

DEFINE_HOOK(0x70D690, TechnoClass_FireDeathWeapon_Supress, 0x7)
{
	GET(TechnoClass*, pThis, ECX);

	TechnoExt::ExtData* pExt = TechnoExt::ExtMap.Find(pThis);

	for (const auto& pAE : pExt->AttachEffects)
	{
		if (!pAE->IsActive())
			continue;

		if (pAE->Type->DisableWeapon && (pAE->Type->DisableWeapon_Category & DisableWeaponCate::Death))
		{
			return 0x70D796;
		}
	}

	return 0;
}

DEFINE_HOOK(0x702583, TechnoClass_ReceiveDamage_NowDead_Explode, 0x6)
{
	GET(TechnoClass*, pThis, ESI);

	auto pExt = TechnoExt::ExtMap.Find(pThis);
	bool forceExplode = false;
	
	for (const auto& pAE : pExt->AttachEffects)
	{
		if (!pAE->IsActive())
			continue;

		forceExplode |= pAE->Type->ForceExplode;

		if (forceExplode)
			break;
	}

	if (forceExplode)
		return 0x702603;

	return 0;
}


DEFINE_HOOK(0x70D724, TechnoClass_FireDeathWeapon_ReplaceDeathWeapon, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EDI);

	R->EBP(R->EAX());
	auto pExt = TechnoExt::ExtMap.Find(pThis);

	for (const auto& pAE : pExt->AttachEffects)
	{
		if (!pAE->IsActive())
			continue;

		if (pAE->Type->ReplaceDeathWeapon.isset())
			pWeapon = pAE->Type->ReplaceDeathWeapon;
	}

	R->EDI(pWeapon);

	return pWeapon == nullptr ? 0x70D72A : 0x70D735;
}

//immune to mindcontrol
DEFINE_HOOK(0x471C90, CaptureManagerClass_CanCapture_AttachEffect, 0x6)
{
	GET_STACK(TechnoClass*, pTarget, 0x4);

	if (auto pTargetExt = TechnoExt::ExtMap.Find(pTarget))
	{
		for (const auto& pAE : pTargetExt->AttachEffects)
		{
			if (!pAE->IsActive())
				continue;

			if (pAE->Type->ImmuneMindControl)
			{
				R->EAX(false);

				return 0x471D39;
			}
		}
	}

	return 0;
}

//HideImage
//DEFINE_HOOK_AGAIN(0x43D290,TechnoClass_Draw_HideImage,0x5)	//Building
DEFINE_HOOK_AGAIN(0x73CEC0,TechnoClass_Draw_HideImage,0x5)	//Unit
DEFINE_HOOK_AGAIN(0x4144B0,TechnoClass_Draw_HideImage,0x5)	//Aircraft
DEFINE_HOOK(0x518F90, TechnoClass_Draw_HideImage, 0x7)	//Infantry
{
	GET(TechnoClass*, pThis, ECX);

	auto pExt = TechnoExt::ExtMap.Find(pThis);

	for (const auto& pAE : pExt->AttachEffects)
	{
		if (!pAE->IsActive())
			continue;

		if (pAE->Type->HideImage)
		{
			switch (pThis->WhatAmI())
			{
			//case AbstractType::Building:
				//static_cast<BuildingClass*>(pThis)->DestroyNthAnim(BuildingAnimSlot::All);
				//return 0x43DA73;
			case AbstractType::Unit:
				return 0x73D446;
			case AbstractType::Infantry:
				return 0x519626;
			case AbstractType::Aircraft:
				return 0x4149FE;
			default:
				break;
			}
		}
	}

	return 0;
}

DEFINE_HOOK(0x5184FF, InfantryClass_ReceiveDamage_InfDeathAnim, 0x6)
{
	GET(InfantryClass*, pThis, ESI);
	GET(InfantryTypeClass*, pType, EAX);
	LEA_STACK(args_ReceiveDamage*, args, STACK_OFFSET(0xD0, 0x4));

	enum { NotHuman = 0x518505, AnimOverriden = 0x5185F1, AresCode = 0x5185C8 };

	if (pType->NotHuman)
		return NotHuman;

	const auto pExt = TechnoExt::ExtMap.Find(pThis);

	AnimTypeClass* pAnimType = nullptr;

	for (const auto& pAE : pExt->AttachEffects)
	{
		if (!pAE->IsActive())
			continue;

		if (pAE->Type->InfDeathAnim != nullptr)
			pAnimType = pAE->Type->InfDeathAnim;
	}

	if (pAnimType != nullptr)
	{
		AnimClass* pAnim = GameCreate<AnimClass>(pAnimType, pThis->Location);
		pAnim->Owner = args->SourceHouse;

		if (pAnim->Owner != nullptr)
		{
			pAnim->LightConvert = ColorScheme::Array->GetItem(pAnim->Owner->ColorSchemeIndex)->LightConvert;
			pAnim->LightConvertIndex = pAnim->Owner->ColorSchemeIndex;
		}

		return AnimOverriden;
	}

	return AresCode;
}
