#include <Utilities/EnumFunctions.h>

#include <Ext/Techno/Body.h>
#include <Ext/House/Body.h>
#include <Ext/Anim/Body.h>
#include <Ext/AnimType/Body.h>

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

	if (pExt->ParentAttachment && pExt->ParentAttachment->GetType()->InheritStateEffects)
	{
		auto pParentExt = TechnoExt::ExtMap.Find(pExt->ParentAttachment->Parent);
		for (const auto& pAE : pParentExt->AttachEffects)
		{
			if (!pAE->IsActive())
				continue;

			dblMultiplier *= pAE->Type->ROF_Multiplier;
			iROFBuff += pAE->Type->ROF;
		}
	}

	for (auto const& pAttachment : pExt->ChildAttachments)
	{
		if (pAttachment->GetType()->InheritStateEffects_Parent)
		{
			auto pChildExt = TechnoExt::ExtMap.Find(pAttachment->Child);
			for (const auto& pAE : pChildExt->AttachEffects)
			{
				if (!pAE->IsActive())
					continue;

				dblMultiplier *= pAE->Type->ROF_Multiplier;
				iROFBuff += pAE->Type->ROF;
			}
		}
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

	if (pTechnoExt->ParentAttachment && pTechnoExt->ParentAttachment->GetType()->InheritStateEffects)
	{
		auto pParentExt = TechnoExt::ExtMap.Find(pTechnoExt->ParentAttachment->Parent);
		for (const auto& pAE : pParentExt->AttachEffects)
		{
			if (!pAE->IsActive())
				continue;

			dblMultiplier *= pAE->Type->FirePower_Multiplier;
			iDamageBuff += pAE->Type->FirePower;
		}
	}

	for (auto const& pAttachment : pTechnoExt->ChildAttachments)
	{
		if (pAttachment->GetType()->InheritStateEffects_Parent)
		{
			auto pChildExt = TechnoExt::ExtMap.Find(pAttachment->Child);
			for (const auto& pAE : pChildExt->AttachEffects)
			{
				if (!pAE->IsActive())
					continue;

				dblMultiplier *= pAE->Type->FirePower_Multiplier;
				iDamageBuff += pAE->Type->FirePower;
			}
		}
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

	if (pExt->ParentAttachment && pExt->ParentAttachment->GetType()->InheritStateEffects)
	{
		auto pParentExt = TechnoExt::ExtMap.Find(pExt->ParentAttachment->Parent);
		for (const auto& pAE : pParentExt->AttachEffects)
		{
			if (!pAE->IsActive())
				continue;

			dblMultiplier *= pAE->Type->Speed_Multiplier;
			iSpeedBuff += pAE->Type->Speed;
		}
	}

	for (auto const& pAttachment : pExt->ChildAttachments)
	{
		if (pAttachment->GetType()->InheritStateEffects_Parent)
		{
			auto pChildExt = TechnoExt::ExtMap.Find(pAttachment->Child);
			for (const auto& pAE : pChildExt->AttachEffects)
			{
				if (!pAE->IsActive())
					continue;

				dblMultiplier *= pAE->Type->Speed_Multiplier;
				iSpeedBuff += pAE->Type->Speed;
			}
		}
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

	if (pExt->ParentAttachment && pExt->ParentAttachment->GetType()->InheritStateEffects)
	{
		auto pParentExt = TechnoExt::ExtMap.Find(pExt->ParentAttachment->Parent);
		for (const auto& pAE : pParentExt->AttachEffects)
		{
			if (!pAE->IsActive())
				continue;

			range += pAE->Type->Range;
			dblRangeMultiplier *= pAE->Type->Range_Multiplier;
		}
	}

	for (auto const& pAttachment : pExt->ChildAttachments)
	{
		if (pAttachment->GetType()->InheritStateEffects_Parent)
		{
			auto pChildExt = TechnoExt::ExtMap.Find(pAttachment->Child);
			for (const auto& pAE : pChildExt->AttachEffects)
			{
				if (!pAE->IsActive())
					continue;

				range += pAE->Type->Range;
				dblRangeMultiplier *= pAE->Type->Range_Multiplier;
			}
		}
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

		if (pExt->ParentAttachment && pExt->ParentAttachment->GetType()->InheritStateEffects)
		{
			auto pParentExt = TechnoExt::ExtMap.Find(pExt->ParentAttachment->Parent);
			for (const auto& pAE : pParentExt->AttachEffects)
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

		for (auto const& pAttachment : pExt->ChildAttachments)
		{
			if (pAttachment->GetType()->InheritStateEffects_Parent)
			{
				auto pChildExt = TechnoExt::ExtMap.Find(pAttachment->Child);
				for (const auto& pAE : pChildExt->AttachEffects)
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

	if (pExt->ParentAttachment && pExt->ParentAttachment->GetType()->InheritStateEffects)
	{
		auto pParentExt = TechnoExt::ExtMap.Find(pExt->ParentAttachment->Parent);
		for (const auto& pAE : pParentExt->AttachEffects)
		{
			if (!pAE->IsActive())
				continue;

			if (pAE->Type->DisableWeapon && (pAE->Type->DisableWeapon_Category & DisableWeaponCate::Death))
			{
				return 0x70D796;
			}
		}
	}

	for (auto const& pAttachment : pExt->ChildAttachments)
	{
		if (pAttachment->GetType()->InheritStateEffects_Parent)
		{
			auto pChildExt = TechnoExt::ExtMap.Find(pAttachment->Child);
			for (const auto& pAE : pChildExt->AttachEffects)
			{
				if (!pAE->IsActive())
					continue;

				if (pAE->Type->DisableWeapon && (pAE->Type->DisableWeapon_Category & DisableWeaponCate::Death))
				{
					return 0x70D796;
				}
			}
		}
	}

	return 0;
}

DEFINE_HOOK(0x702583, TechnoClass_ReceiveDamage_NowDead_Explode, 0x6)
{
	GET(TechnoClass*, pThis, ESI);

	auto pExt = TechnoExt::ExtMap.Find(pThis);
	bool forceExplode = false;

	if (pExt->UnitDeathAnim != nullptr && (pThis->WhatAmI() == AbstractType::Unit || pThis->WhatAmI() == AbstractType::Aircraft))
	{
		if (auto const pAnim = GameCreate<AnimClass>(pExt->UnitDeathAnim, pThis->Location))
		{
			AnimExt::SetAnimOwnerHouseKind(pAnim, pExt->UnitDeathAnimOwner, pThis->Owner, true);
			auto pAnimExt = AnimExt::ExtMap.Find(pAnim);
			pAnimExt->DeathUnitFacing = static_cast<short>(pThis->PrimaryFacing.Current().GetFacing<256>());
			pAnimExt->FromDeathUnit = true;
			if (pThis->HasTurret())
			{
				pAnimExt->DeathUnitHasTurret = true;
				pAnimExt->DeathUnitTurretFacing = pThis->SecondaryFacing.Current();
			}
		}
	}

	for (const auto& pAE : pExt->AttachEffects)
	{
		if (!pAE->IsActive())
			continue;

		forceExplode |= pAE->Type->ForceExplode;

		if (forceExplode)
			break;
	}

	if (pExt->ParentAttachment && pExt->ParentAttachment->GetType()->InheritStateEffects)
	{
		auto pParentExt = TechnoExt::ExtMap.Find(pExt->ParentAttachment->Parent);
		for (const auto& pAE : pParentExt->AttachEffects)
		{
			if (!pAE->IsActive())
				continue;

			forceExplode |= pAE->Type->ForceExplode;

			if (forceExplode)
				break;
		}
	}

	for (auto const& pAttachment : pExt->ChildAttachments)
	{
		if (pAttachment->GetType()->InheritStateEffects_Parent)
		{
			auto pChildExt = TechnoExt::ExtMap.Find(pAttachment->Child);
			for (const auto& pAE : pChildExt->AttachEffects)
			{
				if (!pAE->IsActive())
					continue;

				forceExplode |= pAE->Type->ForceExplode;

				if (forceExplode)
					break;
			}
		}
	}

	if (forceExplode)
		return 0x702603;

	return 0;
}

DEFINE_HOOK(0x70D724, TechnoClass_FireDeathWeapon_ReplaceDeathWeapon, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EDI);

	enum { FireDeathWeapon = 0x70D735, SkipDeathWeapon = 0x70D72A };

	if (!SessionClass::IsSingleplayer())
	{
		R->EBP(R->EAX());
		return pWeapon == nullptr ? SkipDeathWeapon : FireDeathWeapon;
	}

	auto pExt = TechnoExt::ExtMap.Find(pThis);

	if (pExt->ParentAttachment && pExt->ParentAttachment->GetType()->InheritStateEffects)
	{
		auto pParentExt = TechnoExt::ExtMap.Find(pExt->ParentAttachment->Parent);
		for (const auto& pAE : pParentExt->AttachEffects)
		{
			if (!pAE->IsActive())
				continue;

			if (pAE->Type->ReplaceDeathWeapon.isset())
				pWeapon = pAE->Type->ReplaceDeathWeapon;
		}
	}

	for (auto const& pAttachment : pExt->ChildAttachments)
	{
		if (pAttachment->GetType()->InheritStateEffects_Parent)
		{
			auto pChildExt = TechnoExt::ExtMap.Find(pAttachment->Child);
			for (const auto& pAE : pChildExt->AttachEffects)
			{
				if (!pAE->IsActive())
					continue;

				if (pAE->Type->ReplaceDeathWeapon.isset())
					pWeapon = pAE->Type->ReplaceDeathWeapon;
			}
		}
	}

	for (const auto& pAE : pExt->AttachEffects)
	{
		if (!pAE->IsActive())
			continue;

		if (pAE->Type->ReplaceDeathWeapon.isset())
			pWeapon = pAE->Type->ReplaceDeathWeapon;
	}

	R->EDI(pWeapon);
	R->EBP(pWeapon ? Game::F2I(pWeapon->Damage * pThis->GetTechnoType()->DeathWeaponDamageModifier) : 0);

	return pWeapon == nullptr ? SkipDeathWeapon : FireDeathWeapon;
}

//immune to mindcontrol
DEFINE_HOOK(0x471C90, CaptureManagerClass_CanCapture_AttachEffect, 0x6)
{
	GET(CaptureManagerClass*, pThis, ECX);
	GET_STACK(TechnoClass*, pTarget, 0x4);

	enum { SkipGameCode = 0x471D39 };

	if (!TechnoExt::IsReallyAlive(pTarget))
	{
		R->EAX(false);

		return SkipGameCode;
	}

	if (pTarget->Passengers.NumPassengers > 0)
	{
		for (
			FootClass* pPassenger = abstract_cast<FootClass*>(pTarget->Passengers.GetFirstPassenger());
			pPassenger != nullptr && pPassenger->Transporter == pThis->Owner;
			pPassenger = abstract_cast<FootClass*>(pPassenger->NextObject)
			)
		{
			auto pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pPassenger->GetTechnoType());

			if (pTechnoTypeExt->VehicleImmuneToMindControl)
			{
				R->EAX(false);

				return SkipGameCode;
			}
		}
	}

	if (auto pTargetExt = TechnoExt::ExtMap.Find(pTarget))
	{
		for (const auto& pAE : pTargetExt->AttachEffects)
		{
			if (!pAE->IsActive())
				continue;

			if (pAE->Type->ImmuneMindControl)
			{
				R->EAX(false);

				return SkipGameCode;
			}
		}

		if (pTargetExt->ParentAttachment && pTargetExt->ParentAttachment->GetType()->InheritStateEffects)
		{
			auto pParentExt = TechnoExt::ExtMap.Find(pTargetExt->ParentAttachment->Parent);
			for (const auto& pAE : pParentExt->AttachEffects)
			{
				if (!pAE->IsActive())
					continue;

				if (pAE->Type->ImmuneMindControl)
				{
					R->EAX(false);

					return SkipGameCode;
				}
			}
		}

		for (auto const& pAttachment : pTargetExt->ChildAttachments)
		{
			if (pAttachment->GetType()->InheritStateEffects_Parent)
			{
				auto pChildExt = TechnoExt::ExtMap.Find(pAttachment->Child);
				for (const auto& pAE : pChildExt->AttachEffects)
				{
					if (!pAE->IsActive())
						continue;

					if (pAE->Type->ImmuneMindControl)
					{
						R->EAX(false);

						return SkipGameCode;
					}
				}
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

	if (pExt->ParentAttachment && pExt->ParentAttachment->GetType()->InheritStateEffects)
	{
		auto pParentExt = TechnoExt::ExtMap.Find(pExt->ParentAttachment->Parent);
		for (const auto& pAE : pParentExt->AttachEffects)
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
	}

	for (auto const& pAttachment : pExt->ChildAttachments)
	{
		if (pAttachment->GetType()->InheritStateEffects_Parent)
		{
			auto pChildExt = TechnoExt::ExtMap.Find(pAttachment->Child);
			for (const auto& pAE : pChildExt->AttachEffects)
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
		}
	}

	if (pExt->WasOnAntiGravity && pExt->AntiGravityType)
	{
		auto pWHExt = WarheadTypeExt::ExtMap.Find(pExt->AntiGravityType);
		if (pWHExt->AntiGravity_Destory)
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

	if (pExt->ParentAttachment && pExt->ParentAttachment->GetType()->InheritStateEffects)
	{
		auto pParentExt = TechnoExt::ExtMap.Find(pExt->ParentAttachment->Parent);
		for (const auto& pAE : pParentExt->AttachEffects)
		{
			if (!pAE->IsActive())
				continue;

			if (pAE->Type->InfDeathAnim != nullptr)
				pAnimType = pAE->Type->InfDeathAnim;
		}
	}

	for (auto const& pAttachment : pExt->ChildAttachments)
	{
		if (pAttachment->GetType()->InheritStateEffects_Parent)
		{
			auto pChildExt = TechnoExt::ExtMap.Find(pAttachment->Child);
			for (const auto& pAE : pChildExt->AttachEffects)
			{
				if (!pAE->IsActive())
					continue;

				if (pAE->Type->InfDeathAnim != nullptr)
					pAnimType = pAE->Type->InfDeathAnim;
			}
		}
	}

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

DEFINE_HOOK(0x482956, CellClass_CrateBeingCollected_Cloak, 0xA)
{
	GET(int, distance, EAX);

	if (distance < RulesClass::Instance->CrateRadius)
	{
		GET(TechnoClass*, pThis, ECX);

		if (const auto pExt = TechnoExt::ExtMap.Find(pThis))
			pExt->Crate_Cloakable = true;
	}

	return 0;
}
