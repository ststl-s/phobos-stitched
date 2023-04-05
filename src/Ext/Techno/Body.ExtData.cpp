#include "Body.h"

#include <Utilities/EnumFunctions.h>
#include <Utilities/Helpers.Alex.h>

#include <JumpjetLocomotionClass.h>

#include <Misc/FlyingStrings.h>

#include <Ext/BuildingType/Body.h>
#include <Ext/House/Body.h>
#include <Ext/HouseType/Body.h>
#include <Ext/SWType/Body.h>

void __fastcall TechnoExt::ExtData::UpdateTypeData(const TechnoTypeClass* currentType)
{
	auto const pThis = this->OwnerObject();

	if (this->LaserTrails.size())
		this->LaserTrails.clear();

	this->TypeExtData = TechnoTypeExt::ExtMap.Find(currentType);

	// Recreate Laser Trails
	for (auto const& entry : this->TypeExtData->LaserTrailData)
	{
		if (auto const pLaserType = LaserTrailTypeClass::Array[entry.Type].get())
		{
			this->LaserTrails.push_back(std::make_unique<LaserTrailClass>(
				pLaserType, pThis->Owner, entry.FLH, entry.IsOnTurret));
		}
	}

	// Reset Shield
	// This part should have been done by UpdateShield

	// Reset AutoDeath Timer
	if (this->AutoDeathTimer.HasStarted())
		this->AutoDeathTimer.Stop();

	// Reset PassengerDeletion Timer - TODO : unchecked
	if (this->PassengerDeletionTimer.IsTicking() && this->TypeExtData->PassengerDeletion_Rate <= 0)
	{
		this->PassengerDeletionCountDown = -1;
		this->PassengerDeletionTimer.Stop();
	}
}

void TechnoExt::ExtData::UpdateShield()
{
	// Set current shield type if it is not set.
	if (this->CurrentShieldType && this->CurrentShieldType->Strength <= 0 && this->TypeExtData->ShieldType->Strength)
		this->CurrentShieldType = TypeExtData->ShieldType;

	// Create shield class instance if it does not exist.
	if (this->CurrentShieldType && this->CurrentShieldType->Strength && this->Shield == nullptr)
	{
		this->Shield = std::make_unique<ShieldClass>(this->OwnerObject());
	}

	if (const auto pShieldData = this->Shield.get())
		pShieldData->AI();
}

void TechnoExt::ExtData::ApplyInterceptor()
{
	bool interceptor = this->TypeExtData->Interceptor.Get();
	TechnoClass* pTechno = this->OwnerObject();

	if (interceptor)
	{
		const InterceptorTypeClass* pInterceptorType = this->TypeExtData->InterceptorType.get();
		bool interceptor_Rookie = pInterceptorType->Rookie.Get(true);
		bool interceptor_Veteran = pInterceptorType->Veteran.Get(true);
		bool interceptor_Elite = pInterceptorType->Elite.Get(true);

		if (pTechno->Veterancy.IsRookie() && !interceptor_Rookie
			|| pTechno->Veterancy.IsVeteran() && !interceptor_Veteran
			|| pTechno->Veterancy.IsElite() && !interceptor_Elite)
			return;

		if (!pTechno->Target && !(pTechno->WhatAmI() == AbstractType::Aircraft && pTechno->GetHeight() <= 0))
		{
			BulletClass* pTargetBullet = nullptr;

			std::vector<BulletClass*> vBullets(std::move(GeneralUtils::GetCellSpreadBullets(pTechno->Location, pInterceptorType->GuardRange.Get(pTechno))));

			for (auto const pBullet : vBullets)
			{
				auto pBulletTypeExt = BulletTypeExt::ExtMap.Find(pBullet->Type);
				auto pBulletExt = BulletExt::ExtMap.Find(pBullet);

				if (!pBulletTypeExt || !pBulletTypeExt->Interceptable || pBulletExt->InterceptedStatus == InterceptedStatus::Intercepted)
					continue;

				if (pBulletTypeExt->Armor.isset())
				{
					int weaponIndex = pTechno->SelectWeapon(pBullet);
					if (pInterceptorType->UseStageWeapon)
						weaponIndex = pTechno->CurrentWeaponNumber;
					auto pWeapon = pTechno->GetWeapon(weaponIndex)->WeaponType;

					if (pInterceptorType->WeaponType.Get(pTechno).WeaponType != nullptr)
						pWeapon = pInterceptorType->WeaponType.Get(pTechno).WeaponType;

					if (pWeapon == nullptr || !pWeapon->Projectile->AA)
						continue;

					double versus = GeneralUtils::GetWarheadVersusArmor(pWeapon->Warhead, pBulletTypeExt->Armor.Get());

					if (versus == 0.0)
						continue;
				}

				const auto& minguardRange = pInterceptorType->MinimumGuardRange.Get(pTechno);

				auto distance = pBullet->Location.DistanceFrom(pTechno->Location);

				if (distance < minguardRange)
					continue;

				auto bulletOwner = pBullet->Owner ? pBullet->Owner->Owner : pBulletExt->FirerHouse;

				if (EnumFunctions::CanTargetHouse(pInterceptorType->CanTargetHouses.Get(), pTechno->Owner, bulletOwner))
				{
					pTargetBullet = pBullet;

					if (pBulletExt->InterceptedStatus == InterceptedStatus::Targeted)
						continue;

					break;
				}
			}

			if (pTargetBullet)
				pTechno->SetTarget(pTargetBullet);
		}
	}
}

void TechnoExt::ExtData::ApplySpawnLimitRange()
{
	TechnoClass* pTechno = OwnerObject();

	if (auto const pManager = pTechno->SpawnManager)
	{
		auto pTechnoType = pTechno->GetTechnoType();
		int weaponRange = 0;
		int weaponRangeExtra = TypeExtData->Spawner_ExtraLimitRange * Unsorted::LeptonsPerCell;
		auto setWeaponRange = [&weaponRange](WeaponTypeClass* pWeaponType)
		{
			if (pWeaponType && pWeaponType->Spawner && pWeaponType->Range > weaponRange)
				weaponRange = pWeaponType->Range;
		};

		setWeaponRange(pTechnoType->Weapon[0].WeaponType);
		setWeaponRange(pTechnoType->Weapon[1].WeaponType);
		setWeaponRange(pTechnoType->EliteWeapon[0].WeaponType);
		setWeaponRange(pTechnoType->EliteWeapon[1].WeaponType);

		weaponRange += weaponRangeExtra;

		if (pManager->Target && (pTechno->DistanceFrom(pManager->Target) > weaponRange))
			pManager->ResetTarget();
	}
}

void TechnoExt::ExtData::SilentPassenger()
{
	TechnoClass* pThis = OwnerObject();
	auto const pTypeExt = TypeExtData;

	if (pTypeExt->SilentPassenger)
	{
		if (pThis->Passengers.NumPassengers > 0)
		{
			FootClass* pPassenger = pThis->Passengers.GetFirstPassenger();

			if (AllowPassengerToFire)
			{
				if (AllowFireCount)
				{
					if (pThis->GetCurrentMission() != Mission::Attack)
						AllowFireCount = 0;
					else
					{
						while (pPassenger)
						{
							pPassenger->ForceMission(Mission::Attack);
							pPassenger->SetTarget(pThis->Target);
							if (auto const pManager = pPassenger->SpawnManager)
							{
								pManager->SetTarget(pThis->Target);
							}
							pPassenger = static_cast<FootClass*>(pPassenger->NextObject);
						}
						AllowFireCount--;
					}
				}
				else
				{
					AllowPassengerToFire = false;
				}
			}
			else
			{
				while (pPassenger)
				{
					pPassenger->ForceMission(Mission::Stop);
					pPassenger->Guard();

					if (auto const pManager = pPassenger->SpawnManager)
					{
						pManager->ResetTarget();
					}

					pPassenger = static_cast<FootClass*>(pPassenger->NextObject);
				}
			}
		}
	}
}

void TechnoExt::ExtData::ApplySpawnSameLoseTarget()
{
	if (TypeExtData->Spawner_SameLoseTarget.Get())
	{
		SpawnManagerClass* pManager = OwnerObject()->SpawnManager;

		if (pManager != nullptr)
		{
			if (SpawneLoseTarget)
				pManager->ResetTarget();
			else
				SpawneLoseTarget = true;
		}
	}
}

void TechnoExt::ExtData::ConvertsRecover()
{
	TechnoClass* pThis = OwnerObject();
	if (ConvertsOriginalType != pThis->GetTechnoType())
	{
		if (ConvertsCounts > 0)
		{
			ConvertsCounts--;
		}
		else if (ConvertsCounts == 0)
		{
			Convert(pThis, ConvertsOriginalType, Convert_DetachedBuildLimit);

			if (ConvertsAnim != nullptr)
			{
				AnimClass* pAnim = GameCreate<AnimClass>(ConvertsAnim, pThis->GetCoords());
				pAnim->SetOwnerObject(pThis);
			}

			ConvertsAnim = nullptr;
			ConvertsCounts--;
			ConvertsOriginalType = pThis->GetTechnoType();
		}
	}
}

void TechnoExt::ExtData::InfantryConverts()
{
	TechnoClass* pThis = OwnerObject();
	auto const pTypeExt = TypeExtData;

	if (pTypeExt->Convert_Deploy != nullptr && pThis->GetCurrentMission() == Mission::Unload)
	{
		if (pTypeExt->Convert_DeployAnim != nullptr)
		{
			AnimClass* pAnim = GameCreate<AnimClass>(pTypeExt->Convert_DeployAnim, pThis->Location);
			pAnim->SetOwnerObject(pThis);
			pAnim->Owner = pThis->Owner;
		}

		if (auto pInf = abstract_cast<InfantryClass*>(pThis))
		{
			Convert(pThis, pTypeExt->Convert_Deploy);
			return;
		}
	}
}

void TechnoExt::ExtData::RecalculateROT()
{
	TechnoClass* pThis = OwnerObject();
	auto const pTypeExt = TypeExtData;
	if (pThis->WhatAmI() != AbstractType::Unit && pThis->WhatAmI() != AbstractType::Aircraft && pThis->WhatAmI() != AbstractType::Building)
		return;

	if (pThis->WhatAmI() == AbstractType::Building && pTypeExt->EMPulseCannon)
		return;

	bool disable = DisableTurnCount > 0;

	if (disable)
		--DisableTurnCount;

	TechnoTypeClass* pType = pThis->GetTechnoType();
	double dblROTMultiplier = 1.0 * !disable;
	int iROTBuff = 0;

	for (auto& pAE : AttachEffects)
	{
		if (!pAE->IsActive())
			continue;

		dblROTMultiplier *= pAE->Type->ROT_Multiplier;
		iROTBuff += pAE->Type->ROT;
	}

	int iROT_Primary = static_cast<int>(pType->ROT * dblROTMultiplier) + iROTBuff;
	int iROT_Secondary = static_cast<int>(TypeExtData->TurretROT.Get(pType->ROT) * dblROTMultiplier) + iROTBuff;
	iROT_Primary = std::max(iROT_Primary, 0);
	iROT_Secondary = std::max(iROT_Secondary, 0);
	pThis->PrimaryFacing.SetROT(iROT_Primary == 0 ? 1 : static_cast<short>(iROT_Primary));
	pThis->SecondaryFacing.SetROT(iROT_Secondary == 0 ? 1 : static_cast<short>(iROT_Secondary));

	if (FacingInitialized && iROT_Primary == 0)
		pThis->PrimaryFacing.SetCurrent(LastSelfFacing);

	if (FacingInitialized && iROT_Secondary == 0)
		pThis->SecondaryFacing.SetCurrent(LastTurretFacing);

	LastSelfFacing = pThis->PrimaryFacing.Current();
	LastTurretFacing = pThis->SecondaryFacing.Current();
	FacingInitialized = true;
}

void TechnoExt::ExtData::DisableTurnInfantry()
{
	TechnoClass* pThis = OwnerObject();
	if (pThis->WhatAmI() != AbstractType::Infantry)
		return;

	bool disable = DisableTurnCount > 0;

	if (disable)
	{
		pThis->PrimaryFacing.SetCurrent(LastSelfFacing);
		--DisableTurnCount;
	}
	else
	{
		LastSelfFacing = pThis->PrimaryFacing.Current();
	}
}

void TechnoExt::ExtData::UpdateDodge()
{
	if (DodgeDuration > 0)
	{
		DodgeDuration--;
	}
	else
	{
		CanDodge = false;
		Dodge_Anim = nullptr;
		Dodge_Chance = 0.0;
		Dodge_Houses = AffectedHouse::All;
		Dodge_MaxHealthPercent = 1.0;
		Dodge_MinHealthPercent = 0.0;
		Dodge_OnlyDodgePositiveDamage = true;
	}
}

void TechnoExt::ExtData::UpdateDamageLimit()
{
	if (LimitDamageDuration > 0)
	{
		LimitDamageDuration--;
	}
	else
	{
		LimitDamage = false;
		AllowMaxDamage = Vector2D<int> { MAX(int), MIN(int) };
		AllowMinDamage = Vector2D<int> { MIN(int), MAX(int) };
	}
}

void TechnoExt::ExtData::ProcessMoveDamage()
{
	TechnoClass* pThis = OwnerObject();

	if (pThis->WhatAmI() == AbstractType::Building)
		return;

	auto const pTypeExt = TypeExtData;

	if (MoveDamage_Duration > 0)
	{
		if (LastLocation != pThis->Location)
		{
			LastLocation = pThis->Location;

			if (MoveDamage_Count > 0)
			{
				MoveDamage_Count--;
			}
			else
			{
				MoveDamage_Count = MoveDamage_Delay;

				if (MoveDamage != 0)
				{
					pThis->TakeDamage
					(
						MoveDamage,
						pThis->Owner,
						nullptr,
						MoveDamage_Warhead == nullptr ? RulesClass::Instance->C4Warhead : MoveDamage_Warhead
					);
				}

				if (MoveDamage_Anim)
				{
					if (auto pAnim = GameCreate<AnimClass>(MoveDamage_Anim, pThis->Location))
					{
						pAnim->SetOwnerObject(pThis);
						pAnim->Owner = pThis->Owner;
					}
				}
			}
		}
		else if (MoveDamage_Count > 0)
		{
			MoveDamage_Count--;
		}

		MoveDamage_Duration--;
	}
	else
	{
		if (LastLocation != pThis->Location)
		{
			LastLocation = pThis->Location;

			if (MoveDamage_Count > 0)
			{
				MoveDamage_Count--;
			}
			else
			{
				MoveDamage_Count = pTypeExt->MoveDamage_Delay;

				if (pTypeExt->MoveDamage != 0)
				{
					pThis->TakeDamage
					(
						pTypeExt->MoveDamage,
						pThis->Owner,
						nullptr,
						pTypeExt->MoveDamage_Warhead.Get(RulesClass::Instance->C4Warhead)
					);
				}

				if (pTypeExt->MoveDamage_Anim.isset())
				{
					if (auto pAnim = GameCreate<AnimClass>(pTypeExt->MoveDamage_Anim, pThis->Location))
					{
						pAnim->SetOwnerObject(pThis);
						pAnim->Owner = pThis->Owner;
					}
				}
			}
		}
		else if (MoveDamage_Count > 0)
		{
			MoveDamage_Count--;
		}
	}
}

void TechnoExt::ExtData::ProcessStopDamage()
{
	TechnoClass* pThis = OwnerObject();

	if (pThis->WhatAmI() == AbstractType::Building)
		return;

	auto const pTypeExt = TypeExtData;

	if (StopDamage_Duration > 0)
	{
		if (LastLocation == pThis->Location)
		{
			if (StopDamage_Count > 0)
			{
				StopDamage_Count--;
			}
			else
			{
				StopDamage_Count = StopDamage_Delay;

				if (StopDamage != 0)
				{
					pThis->TakeDamage
					(
						StopDamage,
						pThis->Owner,
						nullptr,
						StopDamage_Warhead == nullptr ? RulesClass::Instance->C4Warhead : StopDamage_Warhead
					);
				}

				if (StopDamage_Anim != nullptr)
				{
					if (auto pAnim = GameCreate<AnimClass>(StopDamage_Anim, pThis->Location))
					{
						pAnim->SetOwnerObject(pThis);
						pAnim->Owner = pThis->Owner;
					}
				}
			}
		}
		else
		{
			if (StopDamage_Count > 0)
				StopDamage_Count--;

			LastLocation = pThis->Location;
		}

		StopDamage_Duration--;
	}
	else
	{
		if (LastLocation == pThis->Location)
		{
			if (StopDamage_Count > 0)
			{
				StopDamage_Count--;
			}
			else
			{
				StopDamage_Count = pTypeExt->StopDamage_Delay;

				if (pTypeExt->StopDamage)
				{
					pThis->TakeDamage
					(
						pTypeExt->StopDamage,
						pThis->Owner,
						nullptr,
						pTypeExt->StopDamage_Warhead.Get(RulesClass::Instance->C4Warhead)
					);
				}

				if (pTypeExt->StopDamage_Anim != nullptr)
				{
					if (auto pAnim = GameCreate<AnimClass>(pTypeExt->StopDamage_Anim, pThis->Location))
					{
						pAnim->SetOwnerObject(pThis);
						pAnim->Owner = pThis->Owner;
					}
				}
			}
		}
		else
		{
			if (StopDamage_Count > 0)
				StopDamage_Count--;

			LastLocation = pThis->Location;
		}
	}
}

void TechnoExt::ShareWeaponRangeFire(TechnoClass* pThis, AbstractClass* pTarget)
{
	auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	auto const pExt = TechnoExt::ExtMap.Find(pThis);

	CoordStruct& source = pThis->Location;
	CoordStruct target = pTarget->GetCoords();
	DirStruct tgtDir = DirStruct { Math::atan2(source.Y - target.Y, target.X - source.X) };

	if (!pThis->GetWeapon(pTypeExt->WeaponRangeShare_UseWeapon)->WeaponType->OmniFire)
	{
		if (pThis->HasTurret())
		{
			if (pThis->TurretFacing().GetFacing<32>() != tgtDir.GetFacing<32>())
			{
				pThis->SecondaryFacing.SetDesired(tgtDir);
				pExt->ShareWeaponRangeTarget = pTarget;
				pExt->ShareWeaponRangeFacing = tgtDir;
				return;
			}
		}
		else
		{
			if (pThis->GetRealFacing().GetFacing<32>() != tgtDir.GetFacing<32>())
			{
				pThis->PrimaryFacing.SetDesired(tgtDir);
				pExt->ShareWeaponRangeTarget = pTarget;
				pExt->ShareWeaponRangeFacing = tgtDir;
				return;
			}
		}
	}

	auto locomotor = pThis->GetTechnoType()->Locomotor;
	ChangeLocomotorTo(pThis, LocomotionClass::CLSIDs::Jumpjet.get());
	pThis->SetTarget(pTarget);
	pThis->ForceMission(Mission::Attack);

	BulletClass* pBullet = pThis->TechnoClass::Fire(pTarget, pTypeExt->WeaponRangeShare_UseWeapon);

	if (pBullet != nullptr)
		pBullet->Owner = pThis;

	ChangeLocomotorTo(pThis, locomotor);
	pThis->Target = nullptr;
	pThis->ForceMission(Mission::Stop);
	pThis->Guard();
	if (pExt->ShareWeaponRangeTarget != nullptr)
	{
		pExt->ShareWeaponRangeTarget = nullptr;
	}
}

void TechnoExt::ExtData::ShareWeaponRangeTurn()
{
	TechnoClass* pThis = OwnerObject();
	if (pThis->HasTurret())
	{
		if (pThis->TurretFacing().GetFacing<32>() != ShareWeaponRangeFacing.GetFacing<32>())
		{
			pThis->SecondaryFacing.SetDesired(ShareWeaponRangeFacing);
		}
		else
		{
			ShareWeaponRangeFire(pThis, ShareWeaponRangeTarget);
		}
	}
	else
	{
		if (pThis->GetRealFacing().GetFacing<32>() != ShareWeaponRangeFacing.GetFacing<32>())
		{
			pThis->PrimaryFacing.SetDesired(ShareWeaponRangeFacing);
		}
		else
		{
			ShareWeaponRangeFire(pThis, ShareWeaponRangeTarget);
		}
	}
}

void TechnoExt::ExtData::TeamAffect()
{
	TechnoClass* pThis = OwnerObject();
	auto const pTypeExt = TypeExtData;
	if (!TeamAffectUnits.empty())
		TeamAffectUnits.clear();

	if (pTypeExt->TeamAffect && pTypeExt->TeamAffect_Range > 0)
	{
		int TeamUnitNumber = 0;
		if (pTypeExt->TeamAffect_Technos.empty())
		{
			for (auto pTeamUnit : Helpers::Alex::getCellSpreadItems(pThis->GetCoords(), pTypeExt->TeamAffect_Range, true))
			{
				if (pTeamUnit != pThis
					&& EnumFunctions::CanTargetHouse(pTypeExt->TeamAffect_Houses, pThis->Owner, pTeamUnit->Owner)
					&& EnumFunctions::IsTechnoEligible(pTeamUnit, pTypeExt->TeamAffect_Targets)
					&& TechnoExt::IsActivePower(pTeamUnit))
				{
					TeamAffectUnits.emplace_back(pTeamUnit);
					TeamUnitNumber++;
				}

				if (pTypeExt->TeamAffect_MaxNumber >= pTypeExt->TeamAffect_Number && TeamUnitNumber == pTypeExt->TeamAffect_MaxNumber)
					break;
			}

			if (TeamUnitNumber >= pTypeExt->TeamAffect_Number)
			{
				if (TeamAffectCount > 0)
				{
					TeamAffectCount--;
				}
				else
				{
					if (pTypeExt->TeamAffect_Weapon.Get())
					{
						WeaponTypeExt::DetonateAt(pTypeExt->TeamAffect_Weapon, pThis, pThis);
						TeamAffectCount = pTypeExt->TeamAffect_ROF.isset() ? pTypeExt->TeamAffect_ROF : pTypeExt->TeamAffect_Weapon->ROF;
					}
				}

				if (pTypeExt->TeamAffect_Anim.isset() && TeamAffectAnim == nullptr)
				{
					TeamAffectAnim = GameCreate<AnimClass>(pTypeExt->TeamAffect_Anim, pThis->GetCoords());
					TeamAffectAnim->SetOwnerObject(pThis);
					TeamAffectAnim->RemainingIterations = 0xFFU;
					TeamAffectAnim->Owner = pThis->GetOwningHouse();
				}

				TeamAffectActive = true;
				return;
			}
			else
			{
				if (TeamAffectCount > 0)
					TeamAffectCount--;

				if (TeamAffectAnim != nullptr)
				{
					TeamAffectAnim->UnInit();
					TeamAffectAnim = nullptr;
				}
			}

			if (TeamAffectActive)
			{
				TeamAffectActive = false;

				if (TeamAffectLoseEfficacyCount > 0)
					TeamAffectLoseEfficacyCount--;
				else
				{
					if (pTypeExt->TeamAffect_LoseEfficacyWeapon.Get())
					{
						WeaponTypeExt::DetonateAt(pTypeExt->TeamAffect_LoseEfficacyWeapon, pThis, pThis);
						TeamAffectLoseEfficacyCount = pTypeExt->TeamAffect_LoseEfficacyROF.isset() ? pTypeExt->TeamAffect_LoseEfficacyROF : pTypeExt->TeamAffect_LoseEfficacyWeapon->ROF;
					}
				}
			}
		}
		else
		{
			for (auto pTeamUnit : Helpers::Alex::getCellSpreadItems(pThis->GetCoords(), pTypeExt->TeamAffect_Range, true))
			{
				if (pTeamUnit != pThis
					&& EnumFunctions::CanTargetHouse(pTypeExt->TeamAffect_Houses, pThis->Owner, pTeamUnit->Owner)
					&& TechnoExt::IsActivePower(pTeamUnit))
				{
					if (pTypeExt->TeamAffect_Technos.Contains(pTeamUnit->GetTechnoType()))
					{
						TeamAffectUnits.emplace_back(pTeamUnit);
						TeamUnitNumber++;
					}
				}

				if (pTypeExt->TeamAffect_MaxNumber >= pTypeExt->TeamAffect_Number && TeamUnitNumber == pTypeExt->TeamAffect_MaxNumber)
					break;
			}

			if (TeamUnitNumber >= pTypeExt->TeamAffect_Number)
			{
				if (TeamAffectCount > 0)
				{
					TeamAffectCount--;
				}
				else
				{
					if (pTypeExt->TeamAffect_Weapon.Get())
					{
						WeaponTypeExt::DetonateAt(pTypeExt->TeamAffect_Weapon, pThis, pThis);
						TeamAffectCount = pTypeExt->TeamAffect_ROF.isset() ? pTypeExt->TeamAffect_ROF : pTypeExt->TeamAffect_Weapon->ROF;
					}
				}

				if (pTypeExt->TeamAffect_Anim.isset() && TeamAffectAnim == nullptr)
				{
					TeamAffectAnim = GameCreate<AnimClass>(pTypeExt->TeamAffect_Anim, pThis->GetCoords());
					TeamAffectAnim->SetOwnerObject(pThis);
					TeamAffectAnim->RemainingIterations = 0xFFU;
					TeamAffectAnim->Owner = pThis->GetOwningHouse();
				}

				TeamAffectActive = true;

				return;
			}
			else
			{
				if (TeamAffectCount > 0)
					TeamAffectCount--;

				if (TeamAffectAnim != nullptr)
				{
					TeamAffectAnim->UnInit();
					TeamAffectAnim = nullptr;
				}
			}

			if (TeamAffectActive)
			{
				TeamAffectActive = false;

				if (TeamAffectLoseEfficacyCount > 0)
				{
					TeamAffectLoseEfficacyCount--;
				}
				else
				{
					if (pTypeExt->TeamAffect_LoseEfficacyWeapon)
					{
						WeaponTypeExt::DetonateAt(pTypeExt->TeamAffect_LoseEfficacyWeapon, pThis, pThis);
						TeamAffectLoseEfficacyCount =
							pTypeExt->TeamAffect_LoseEfficacyROF.isset() ?
							pTypeExt->TeamAffect_LoseEfficacyROF :
							pTypeExt->TeamAffect_LoseEfficacyWeapon->ROF;
					}
				}
			}
		}
	}
}

void TechnoExt::ExtData::EatPassengers()
{
	TechnoClass* pTechno = OwnerObject();

	if (!TechnoExt::IsActive(pTechno))
		return;

	if (TypeExtData->PassengerDeletion_Rate > 0 || TypeExtData->PassengerDeletion_UseCostAsRate)
	{
		if (pTechno->Passengers.NumPassengers > 0)
		{
			FootClass* pPassenger = pTechno->Passengers.GetFirstPassenger();

			if (PassengerDeletionCountDown < 0)
			{
				int timerLength = 0;

				if (TypeExtData->PassengerDeletion_UseCostAsRate)
				{
					// Use passenger cost as countdown.
					timerLength = static_cast<int>(pPassenger->GetTechnoType()->Cost * TypeExtData->PassengerDeletion_CostMultiplier);

					if (TypeExtData->PassengerDeletion_Rate > 0)
						timerLength = std::min(timerLength, TypeExtData->PassengerDeletion_Rate.Get());
				}
				else
				{
					// Use explicit rate optionally multiplied by unit size as countdown.
					timerLength = TypeExtData->PassengerDeletion_Rate;
					if (TypeExtData->PassengerDeletion_Rate_SizeMultiply && pPassenger->GetTechnoType()->Size > 1.0)
						timerLength *= static_cast<int>(pPassenger->GetTechnoType()->Size + 0.5);
				}

				PassengerDeletionCountDown = timerLength;
				PassengerDeletionTimer.Start(timerLength);
			}
			else
			{
				if (PassengerDeletionTimer.Completed())
				{
					ObjectClass* pLastPassenger = nullptr;

					// Passengers are designed as a FIFO queue but being implemented as a list
					while (pPassenger->NextObject)
					{
						pLastPassenger = pPassenger;
						pPassenger = abstract_cast<FootClass*>(pPassenger->NextObject);
					}

					if (pLastPassenger)
						pLastPassenger->NextObject = nullptr;
					else
						pTechno->Passengers.FirstPassenger = nullptr;

					--pTechno->Passengers.NumPassengers;

					if (pPassenger)
					{
						VocClass::PlayAt(TypeExtData->PassengerDeletion_ReportSound, pTechno->GetCoords(), nullptr);

						if (TypeExtData->PassengerDeletion_Anim.isset())
						{
							const auto pAnimType = TypeExtData->PassengerDeletion_Anim.Get();
							if (auto const pAnim = GameCreate<AnimClass>(pAnimType, pTechno->Location))
							{
								pAnim->SetOwnerObject(pTechno);
								pAnim->Owner = pTechno->Owner;
							}
						}

						// Check if there is money refund
						if (TypeExtData->PassengerDeletion_Soylent)
						{
							// Refund money to the Attacker
							int nMoneyToGive = pPassenger->GetTechnoType()->GetRefund(pPassenger->Owner, true);
							nMoneyToGive = (int)(nMoneyToGive * TypeExtData->PassengerDeletion_SoylentMultiplier);

							// Is allowed the refund of friendly units?
							if (!TypeExtData->PassengerDeletion_SoylentFriendlies && pPassenger->Owner->IsAlliedWith(pTechno))
								nMoneyToGive = 0;

							if (nMoneyToGive > 0)
							{
								pTechno->Owner->GiveMoney(nMoneyToGive);

								if (TypeExtData->PassengerDeletion_DisplaySoylent)
								{
									FlyingStrings::AddMoneyString
									(
										nMoneyToGive,
										pTechno->Owner,
										TypeExtData->PassengerDeletion_DisplaySoylentToHouses,
										pTechno->Location,
										TypeExtData->PassengerDeletion_DisplaySoylentOffset
									);
								}
							}
						}

						// Handle gunner change.
						if (pTechno->GetTechnoType()->Gunner)
						{
							if (auto const pFoot = abstract_cast<FootClass*>(pTechno))
							{
								pFoot->RemoveGunner(pPassenger);

								if (pTechno->Passengers.NumPassengers > 0)
									pFoot->ReceiveGunner(pTechno->Passengers.FirstPassenger);
							}
						}

						pPassenger->KillPassengers(pTechno);
						pPassenger->RegisterDestruction(pTechno);
						pPassenger->UnInit();
					}

					PassengerDeletionTimer.Stop();
					PassengerDeletionCountDown = -1;
				}
			}
		}
		else
		{
			PassengerDeletionTimer.Stop();
			PassengerDeletionCountDown = -1;
		}
	}
}

void TechnoExt::ExtData::ChangePassengersList()
{
	if (AllowChangePassenger)
	{
		std::vector<FootClass*> tempPassengerList = PassengerList;
		PassengerList.clear();

		for (size_t i = 0; i < tempPassengerList.size() - 1; i++)
		{
			PassengerList.emplace_back(tempPassengerList[i + 1]);
		}

		std::vector<CoordStruct> tempPassengerlocationList = PassengerlocationList;
		PassengerlocationList.clear();

		for (size_t i = 0; i < tempPassengerlocationList.size() - 1; i++)
		{
			PassengerlocationList.emplace_back(tempPassengerlocationList[i + 1]);
		}

		AllowCreatPassenger = true;
		AllowChangePassenger = false;
	}
}

void TechnoExt::ExtData::UpdateOnTunnelEnter()
{
	if (!this->IsInTunnel)
	{
		if (const auto pShieldData = this->Shield.get())
			pShieldData->SetAnimationVisibility(false);

		for (auto& pLaserTrail : this->LaserTrails)
		{
			pLaserTrail->Visible = false;
			pLaserTrail->LastLocation = { };
		}

		this->IsInTunnel = true;
	}
}

void TechnoExt::ExtData::IsInROF()
{
	if (ROFCount > 0)
	{
		InROF = true;
		ROFCount--;
	}
	else
	{
		InROF = false;
		IsChargeROF = false;
	}
}

// TODO : Wrap into a new entity
bool TechnoExt::ExtData::CheckDeathConditions()
{
	auto const pTypeExt = this->TypeExtData;

	if (!pTypeExt->AutoDeath_Behavior.isset())
		return false;

	auto const pThis = this->OwnerObject();
	auto const pType = pThis->GetTechnoType();

	// Self-destruction must be enabled
	const auto howToDie = pTypeExt->AutoDeath_Behavior.Get();

	// Death if no ammo
	if (pType->Ammo > 0 && pThis->Ammo <= 0 && pTypeExt->AutoDeath_OnAmmoDepletion)
	{
		TechnoExt::KillSelf(pThis, howToDie);
		return true;
	}

	// Death if countdown ends
	if (pTypeExt->AutoDeath_AfterDelay > 0)
	{
		if (!this->AutoDeathTimer.HasStarted())
		{
			this->AutoDeathTimer.Start(pTypeExt->AutoDeath_AfterDelay);
		}
		else if (!pThis->Transporter && this->AutoDeathTimer.Completed())
		{
			TechnoExt::KillSelf(pThis, howToDie);
			return true;
		}

	}

	if (pTypeExt->AutoDeath_OnPassengerDepletion && pType->Passengers > 0)
	{
		if (pThis->Passengers.NumPassengers > 0)
		{
			if (this->AutoDeathTimer_Passenger.HasStarted())
			{
				this->AutoDeathTimer_Passenger.Resume();
				this->AutoDeathTimer_Passenger.Stop();
			}
		}
		else
		{
			if (!this->AutoDeathTimer_Passenger.HasStarted())
			{
				this->AutoDeathTimer_Passenger.Start(pTypeExt->AutoDeath_OnPassengerDepletion_Delay);
			}
			else if (!pThis->Transporter && this->AutoDeathTimer_Passenger.Completed())
			{
				TechnoExt::KillSelf(pThis, howToDie);
				return true;
			}
		}
	}

	auto existTechnoTypes = [pThis](const ValueableVector<TechnoTypeClass*>& vTypes, AffectedHouse affectedHouse, bool any)
	{
		auto existSingleType = [pThis, affectedHouse](const TechnoTypeClass* pType)
		{
			for (HouseClass* pHouse : *HouseClass::Array)
			{
				if (EnumFunctions::CanTargetHouse(affectedHouse, pThis->Owner, pHouse)
					&& pHouse->CountOwnedAndPresent(pType) > 0)
					return true;
			}

			return false;
		};

		return any
			? std::any_of(vTypes.begin(), vTypes.end(), existSingleType)
			: std::all_of(vTypes.begin(), vTypes.end(), existSingleType);
	};

	// death if listed technos don't exist
	if (!pTypeExt->AutoDeath_TechnosDontExist.empty())
	{
		if (!existTechnoTypes(pTypeExt->AutoDeath_TechnosDontExist, pTypeExt->AutoDeath_TechnosDontExist_Houses, !pTypeExt->AutoDeath_TechnosDontExist_Any))
		{
			TechnoExt::KillSelf(pThis, howToDie);

			return true;
		}
	}

	// death if listed technos exist
	if (!pTypeExt->AutoDeath_TechnosExist.empty())
	{
		if (existTechnoTypes(pTypeExt->AutoDeath_TechnosExist, pTypeExt->AutoDeath_TechnosExist_Houses, pTypeExt->AutoDeath_TechnosExist_Any))
		{
			TechnoExt::KillSelf(pThis, howToDie);

			return true;
		}
	}

	return false;
}

void TechnoExt::ExtData::CheckIonCannonConditions()
{
	IonCannonTypeClass* pIonCannonType = nullptr;
	auto const pTypeExt = TypeExtData;
	TechnoClass* pThis = OwnerObject();

	pIonCannonType = pTypeExt->IonCannonType.Get();

	if (pIonCannonType == nullptr)
		return;

	if (pIonCannonType->IonCannon_Radius >= 0)
	{
		if (!(IonCannon_Rate > 0))
		{
			if (IonCannon_setRadius)
			{
				IonCannon_Radius = pIonCannonType->IonCannon_Radius;
				IonCannon_RadiusReduce = pIonCannonType->IonCannon_RadiusReduce;
				IonCannon_Angle = pIonCannonType->IonCannon_Angle;
				IonCannon_Scatter_Max = pIonCannonType->IonCannon_Scatter_Max;
				IonCannon_Scatter_Min = pIonCannonType->IonCannon_Scatter_Min;
				IonCannon_Duration = pIonCannonType->IonCannon_Duration;
				IonCannon_setRadius = false;
			}

			if (IonCannon_Radius >= 0 && !IonCannon_Stop)
			{
				CoordStruct center = pThis->GetCoords();

				WeaponTypeClass* pIonCannonWeapon = nullptr;
				if (pIonCannonType->IonCannon_Weapon.isset())
				{
					pIonCannonWeapon = pIonCannonType->IonCannon_Weapon.Get();
				}
				else
				{
					pIonCannonWeapon = pThis->GetWeapon(0)->WeaponType;
				}

				int angleDelta = 360 / pIonCannonType->IonCannon_Lines;
				for (int angle = IonCannon_StartAngle; angle < IonCannon_StartAngle + 360; angle += angleDelta)
				{
					int ScatterX = ScenarioClass::Instance->Random(IonCannonWeapon_Scatter_Min, IonCannonWeapon_Scatter_Max);
					int ScatterY = ScenarioClass::Instance->Random(IonCannonWeapon_Scatter_Min, IonCannonWeapon_Scatter_Max);

					if (ScenarioClass::Instance->Random(0, 1))
						ScatterX = -ScatterX;

					if (ScenarioClass::Instance->Random(0, 1))
						ScatterY = -ScatterY;

					CoordStruct pos =
					{
						center.X + static_cast<int>(IonCannon_Radius * cos(angle * 3.14 / 180)) + ScatterX,
						center.Y + static_cast<int>(IonCannon_Radius * sin(angle * 3.14 / 180)) + ScatterY,
						0
					};
					CoordStruct posAir = pos + CoordStruct { 0, 0, pIonCannonType->IonCannon_LaserHeight };
					CoordStruct posAirEle = pos + CoordStruct { 0, 0, pIonCannonType->IonCannon_EleHeight };
					auto pCell = MapClass::Instance->TryGetCellAt(pos);

					if (pCell)
						pos.Z = pCell->GetCoordsWithBridge().Z;
					else
						pos.Z = MapClass::Instance->GetCellFloorHeight(pos);

					if (!(pIonCannonType->IonCannon_DrawLaserWithFire && IonCannon_ROF > 0))
					{
						if (pIonCannonType->IonCannon_DrawLaser)
						{
							LaserDrawClass* pLaser = GameCreate<LaserDrawClass>
								(
									posAir,
									pos,
									pIonCannonType->IonCannon_InnerColor,
									pIonCannonType->IonCannon_OuterColor,
									pIonCannonType->IonCannon_OuterSpread,
									pIonCannonType->IonCannon_LaserDuration
								);

							pLaser->Thickness = pIonCannonType->IonCannon_Thickness; // only respected if IsHouseColor
							pLaser->IsHouseColor = true;
							// pLaser->IsSupported = this->Type->IsIntense;
						}
					}

					if (!(pIonCannonType->IonCannon_DrawEBoltWithFire && IonCannon_ROF > 0))
					{
						if (pIonCannonType->IonCannon_DrawEBolt) // Uses laser
						{
							if (auto const pEBolt = GameCreate<EBolt>())
								pEBolt->Fire(posAirEle, pos, 0);
						}
					}

					if (IonCannon_ROF <= 0)
					{
						WeaponTypeExt::DetonateAt(pIonCannonWeapon, pos, pThis);
					}
				}

				if (IonCannon_ROF > 0)
					IonCannon_ROF--;
				else
					IonCannon_ROF = pIonCannonType->IonCannon_ROF;

				if (IonCannon_RadiusReduce <= pIonCannonType->IonCannon_RadiusReduceMax
					&& IonCannon_RadiusReduce >= pIonCannonType->IonCannon_RadiusReduceMin)
					IonCannon_RadiusReduce += pIonCannonType->IonCannon_RadiusReduceAcceleration;

				if (IonCannon_Angle <= pIonCannonType->IonCannon_AngleMax && IonCannon_Angle >= pIonCannonType->IonCannon_AngleMin)
					IonCannon_Angle += pIonCannonType->IonCannon_AngleAcceleration;

				if (IonCannon_Scatter_Max <= pIonCannonType->IonCannon_Scatter_Max_IncreaseMax
					&& IonCannon_Scatter_Max >= pIonCannonType->IonCannon_Scatter_Max_IncreaseMin)
					IonCannon_Scatter_Max += pIonCannonType->IonCannon_Scatter_Max_Increase;

				if (IonCannon_Scatter_Min <= pIonCannonType->IonCannon_Scatter_Min_IncreaseMax
					&& IonCannon_Scatter_Min >= pIonCannonType->IonCannon_Scatter_Min_IncreaseMin)
					IonCannon_Scatter_Min += pIonCannonType->IonCannon_Scatter_Min_Increase;

				IonCannon_Radius -= IonCannon_RadiusReduce;
				IonCannon_StartAngle -= IonCannon_Angle;

				if (pIonCannonType->IonCannon_MaxRadius >= 0)
				{
					if (IonCannon_Radius > pIonCannonType->IonCannon_MaxRadius)
						IonCannon_Stop = true;
				}

				if (pIonCannonType->IonCannon_MinRadius >= 0)
				{
					if (IonCannon_Radius < pIonCannonType->IonCannon_MinRadius)
						IonCannon_Stop = true;
				}

				if (pIonCannonType->IonCannon_Duration >= 0)
				{
					if (IonCannon_Duration > 0)
						IonCannon_Duration--;
					else
						IonCannon_Stop = true;
				}
			}
			else
			{
				if (pIonCannonType->IonCannon_FireOnce)
				{
					pThis->TakeDamage(pThis->Health);
				}
				else
				{
					IonCannon_setRadius = true;
					IonCannon_Stop = false;
					IonCannon_Rate = pIonCannonType->IonCannon_Rate;
				}
			}
		}
		else
		{
			IonCannon_Rate--;
		}
	}
}

void TechnoExt::ExtData::RunIonCannonWeapon()
{
	WeaponTypeClass* const pWeapon = setIonCannonWeapon;
	IonCannonTypeClass* pIonCannonType = setIonCannonType;

	if (pIonCannonType != nullptr && pIonCannonType->IonCannon_Radius >= 0)
	{
		if (IonCannonWeapon_setRadius)
		{
			IonCannonWeapon_Radius = pIonCannonType->IonCannon_Radius.Get();
			IonCannonWeapon_RadiusReduce = pIonCannonType->IonCannon_RadiusReduce.Get();
			IonCannonWeapon_Angle = pIonCannonType->IonCannon_Angle.Get();
			IonCannonWeapon_Scatter_Max = pIonCannonType->IonCannon_Scatter_Max.Get();
			IonCannonWeapon_Scatter_Min = pIonCannonType->IonCannon_Scatter_Min.Get();
			IonCannonWeapon_Duration = pIonCannonType->IonCannon_Duration.Get();
			IonCannonWeapon_setRadius = false;
		}

		CoordStruct target = IonCannonWeapon_Target;

		if (IonCannonWeapon_Radius >= 0 && !IonCannonWeapon_Stop)
		{

			WeaponTypeClass* pIonCannonWeapon = pIonCannonType->IonCannon_Weapon.Get(pWeapon);
			int angleDelta = 360 / pIonCannonType->IonCannon_Lines;

			for (int angle = IonCannonWeapon_StartAngle;
				angle < IonCannonWeapon_StartAngle + 360;
				angle += angleDelta)
			{
				int ScatterX = (ScenarioClass::Instance->Random() % (IonCannonWeapon_Scatter_Max - IonCannonWeapon_Scatter_Min + 1)) + IonCannonWeapon_Scatter_Min;
				int ScatterY = (ScenarioClass::Instance->Random() % (IonCannonWeapon_Scatter_Max - IonCannonWeapon_Scatter_Min + 1)) + IonCannonWeapon_Scatter_Min;
				CoordStruct pos =
				{
					target.X + static_cast<int>(IonCannonWeapon_Radius * cos(angle * acos(-1) / 180)) + ScatterX,
					target.Y + static_cast<int>(IonCannonWeapon_Radius * sin(angle * acos(-1) / 180)) + ScatterY,
					0
				};
				CoordStruct posAir = pos + CoordStruct { 0, 0, pIonCannonType->IonCannon_LaserHeight.Get() };
				CoordStruct posAirEle = pos + CoordStruct { 0, 0, pIonCannonType->IonCannon_EleHeight.Get() };
				auto pCell = MapClass::Instance->TryGetCellAt(pos);

				if (ScenarioClass::Instance->Random() & 1)
					ScatterX = -ScatterX;

				if (ScenarioClass::Instance->Random() & 1)
					ScatterY = -ScatterY;

				if (pCell)
					pos.Z = pCell->GetCoordsWithBridge().Z;
				else
					pos.Z = MapClass::Instance->GetCellFloorHeight(pos);

				if (!(pIonCannonType->IonCannon_DrawLaserWithFire.Get() && IonCannonWeapon_ROF > 0))
				{
					if (pIonCannonType->IonCannon_DrawLaser.Get())
					{
						LaserDrawClass* pLaser = GameCreate<LaserDrawClass>
							(
								posAir,
								pos,
								pIonCannonType->IonCannon_InnerColor,
								pIonCannonType->IonCannon_OuterColor,
								pIonCannonType->IonCannon_OuterSpread,
								pIonCannonType->IonCannon_LaserDuration
							);
						// only respected if IsHouseColor
						pLaser->Thickness = pIonCannonType->IonCannon_Thickness.Get();
						pLaser->IsHouseColor = true;
						// pLaser->IsSupported = this->Type->IsIntense;
					}
				}

				if (!(pIonCannonType->IonCannon_DrawEBoltWithFire.Get() && IonCannonWeapon_ROF > 0))
				{
					if (pIonCannonType->IonCannon_DrawEBolt) // Uses laser
					{
						EBolt* pEBolt = GameCreate<EBolt>();

						if (pEBolt != nullptr)
							pEBolt->Fire(posAirEle, pos, 0);
					}
				}

				if (IonCannonWeapon_ROF <= 0)
					WeaponTypeExt::DetonateAt(pIonCannonWeapon, pos, OwnerObject());
			}

			if (IonCannonWeapon_ROF > 0)
				IonCannonWeapon_ROF--;
			else
				IonCannonWeapon_ROF = pIonCannonType->IonCannon_ROF.Get();

			if (IonCannonWeapon_RadiusReduce <= pIonCannonType->IonCannon_RadiusReduceMax.Get()
				&& IonCannonWeapon_RadiusReduce >= pIonCannonType->IonCannon_RadiusReduceMin.Get())
			{
				IonCannonWeapon_RadiusReduce += pIonCannonType->IonCannon_RadiusReduceAcceleration.Get();
			}

			if (IonCannonWeapon_Angle <= pIonCannonType->IonCannon_AngleMax.Get()
				&& IonCannonWeapon_Angle >= pIonCannonType->IonCannon_AngleMin.Get())
			{
				IonCannonWeapon_Angle += pIonCannonType->IonCannon_AngleAcceleration.Get();
			}

			if (IonCannonWeapon_Scatter_Max <= pIonCannonType->IonCannon_Scatter_Max_IncreaseMax.Get()
				&& IonCannonWeapon_Scatter_Max >= pIonCannonType->IonCannon_Scatter_Max_IncreaseMin.Get())
			{
				IonCannonWeapon_Scatter_Max += pIonCannonType->IonCannon_Scatter_Max_Increase.Get();
			}

			if (IonCannonWeapon_Scatter_Min <= pIonCannonType->IonCannon_Scatter_Min_IncreaseMax.Get()
				&& IonCannonWeapon_Scatter_Min >= pIonCannonType->IonCannon_Scatter_Min_IncreaseMin.Get())
			{
				IonCannonWeapon_Scatter_Min += pIonCannonType->IonCannon_Scatter_Min_Increase.Get();
			}

			IonCannonWeapon_Radius -= IonCannonWeapon_RadiusReduce;
			IonCannonWeapon_StartAngle -= IonCannonWeapon_Angle;

			if (pIonCannonType->IonCannon_MaxRadius.Get() >= 0)
			{
				if (IonCannonWeapon_Radius > pIonCannonType->IonCannon_MaxRadius.Get())
					IonCannonWeapon_Stop = true;
			}

			if (pIonCannonType->IonCannon_MinRadius >= 0)
			{
				if (IonCannonWeapon_Radius < pIonCannonType->IonCannon_MinRadius.Get())
					IonCannonWeapon_Stop = true;
			}

			if (pIonCannonType->IonCannon_Duration.Get() >= 0)
			{
				if (IonCannonWeapon_Duration > 0)
					IonCannonWeapon_Duration--;
				else
					IonCannonWeapon_Stop = true;
			}
		}
		else
		{
			IonCannonWeapon_setRadius = true;
			IonCannonWeapon_Stop = true;
		}
	}
}

void TechnoExt::ExtData::RunBeamCannon()
{
	TechnoClass* pThis = OwnerObject();
	WeaponTypeClass* const pWeapon = setBeamCannon;
	auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);

	if (pWeaponExt->IsBeamCannon && pWeaponExt->BeamCannon_Length >= 0)
	{
		if (BeamCannon_setLength)
		{
			BeamCannon_Length = 0;
			BeamCannon_LengthIncrease = pWeaponExt->BeamCannon_LengthIncrease;
			BeamCannon_setLength = false;
		}

		CoordStruct target = BeamCannon_Target;
		std::vector<CoordStruct> center = BeamCannon_Center;
		std::vector<CoordStruct> firepos;

		if (abs(BeamCannon_Length) <= pWeaponExt->BeamCannon_Length && !BeamCannon_Stop)
		{

			WeaponTypeClass* pBeamCannonWeapon = pWeaponExt->BeamCannonWeapon.Get(pWeapon);
			for (int i = 0; i < pWeaponExt->BeamCannon_Burst; i++)
			{
				CoordStruct pos =
				{
					center[i].X + static_cast<int>((BeamCannon_Length)
						* cos(atan2(target.Y - center[i].Y , target.X - center[i].X))),

					center[i].Y + static_cast<int>((BeamCannon_Length)
						* sin(atan2(target.Y - center[i].Y , target.X - center[i].X))),

					0
				};

				CellClass* pCell = MapClass::Instance->TryGetCellAt(pos);

				if (pCell)
					pos.Z = pCell->GetCoordsWithBridge().Z;
				else
					pos.Z = MapClass::Instance->GetCellFloorHeight(pos);

				firepos.emplace_back(pos);

				CoordStruct posAir = pos + CoordStruct { 0, 0, pWeaponExt->BeamCannon_LaserHeight };
				CoordStruct posAirEle = pos + CoordStruct { 0, 0, pWeaponExt->BeamCannon_EleHeight };

				if (pWeaponExt->BeamCannon_DrawFromSelf)
				{
					posAir = TechnoExt::GetFLHAbsoluteCoords(pThis, pWeaponExt->BeamCannon_FLH[i], pThis->HasTurret());
					posAirEle = TechnoExt::GetFLHAbsoluteCoords(pThis, pWeaponExt->BeamCannon_FLH[i], pThis->HasTurret());
				}

				if (pWeaponExt->BeamCannon_DrawLaser)
				{
					LaserDrawClass* pLaser =
						GameCreate<LaserDrawClass>
						(
							posAir,
							pos,
							pWeaponExt->BeamCannon_InnerColor,
							pWeaponExt->BeamCannon_OuterColor,
							pWeaponExt->BeamCannon_OuterSpread,
							pWeaponExt->BeamCannon_Duration
						);
					// only respected if IsHouseColor
					pLaser->Thickness = pWeaponExt->BeamCannon_Thickness;
					pLaser->IsHouseColor = true;
					// pLaser->IsSupported = this->Type->IsIntense;
				}

				if (pWeaponExt->BeamCannon_DrawEBolt)
				{
					EBolt* pEBolt = GameCreate<EBolt>();

					if (pEBolt != nullptr)
						pEBolt->Fire(posAirEle, pos, 0);
				}
			}

			if (BeamCannon_ROF > 0)
			{
				BeamCannon_ROF--;
			}
			else
			{
				for (int i = 0; i < pWeaponExt->BeamCannon_Burst; i++)
				{
					WeaponTypeExt::DetonateAt(pBeamCannonWeapon, firepos[i], pThis);
				}

				BeamCannon_ROF = pWeaponExt->BeamCannon_ROF;
			}

			if (BeamCannon_LengthIncrease <= pWeaponExt->BeamCannon_LengthIncreaseMax
				&& BeamCannon_LengthIncrease >= pWeaponExt->BeamCannon_LengthIncreaseMin)
			{
				BeamCannon_LengthIncrease += pWeaponExt->BeamCannon_LengthIncreaseAcceleration;
			}
			BeamCannon_Length += BeamCannon_LengthIncrease;
		}
		else
		{
			BeamCannon_setLength = true;
			BeamCannon_Stop = true;
			BeamCannon_Center.clear();
		}
	}
}

void TechnoExt::ExtData::BeamCannonLockFacing()
{
	if (!BeamCannon_Stop)
	{
		if (DisableTurnCount < 2)
		{
			if (DisableTurnCount == 1)
				DisableTurnCount += 1;
			else
				DisableTurnCount += 2;
		}
	}
}

void TechnoExt::ExtData::ProcessFireSelf()
{
	TechnoClass* pThis = OwnerObject();
	const ValueableVector<WeaponTypeClass*>& vWeapons = TypeExtData->FireSelf_Weapon.Get(pThis);
	const ValueableVector<int>& vROF = TypeExtData->FireSelf_ROF.Get(pThis);

	if (vWeapons.empty())
		return;

	std::vector<CDTimerClass>& vTimers = FireSelf_Timers;

	while (vTimers.size() < vWeapons.size())
	{
		size_t idx = vTimers.size();
		int iROF = idx < vROF.size() ? vROF[idx] : vWeapons[idx]->ROF;
		vTimers.emplace_back(TypeExtData->FireSelf_Immediately.Get(pThis) ? 0 : iROF);
	}

	for (size_t i = 0; i < vWeapons.size(); i++)
	{
		if (vTimers[i].Completed())
		{
			int iROF = i < vROF.size() ? vROF[i] : vWeapons[i]->ROF;
			WeaponTypeExt::DetonateAt(vWeapons[i], pThis, pThis);

			if (!IsReallyAlive(pThis))
				return;

			vTimers[i].Start(iROF);
		}
	}
}

void TechnoExt::ExtData::CheckPaintConditions()
{
	if (AllowToPaint)
	{
		if (Paint_Count > 0)
		{
			Paint_Count--;
			Paint_FramesPassed++;
		}
		else
		{
			AllowToPaint = false;
			Paint_IsDiscoColor = false;
			Paint_FramesPassed = 0;
		}

		if (Paint_IsDiscoColor)
		{
			auto& colors = Paint_Colors;

			int transitionCycle = (Paint_FramesPassed / Paint_TransitionDuration) % colors.size();
			int currentColorIndex = transitionCycle;
			int nextColorIndex = (transitionCycle + 1) % colors.size();
			double blendingCoef = (Paint_FramesPassed % Paint_TransitionDuration) / static_cast<double>(Paint_TransitionDuration);
			ColorToPaint =
			{
				(BYTE)(colors[currentColorIndex].R * (1 - blendingCoef) + colors[nextColorIndex].R * blendingCoef),
				(BYTE)(colors[currentColorIndex].G * (1 - blendingCoef) + colors[nextColorIndex].G * blendingCoef),
				(BYTE)(colors[currentColorIndex].B * (1 - blendingCoef) + colors[nextColorIndex].B * blendingCoef)
			};
		}
	}
}

void TechnoExt::ExtData::CheckJJConvertConditions()
{
	if (NeedConvertWhenLanding)
	{
		TechnoClass* pThis = OwnerObject();

		if (!JJ_Landed)
		{
			if (pThis->CurrentMission == Mission::Unload)
			{
				Convert(pThis, LandingType);
				JJ_Landed = true;
			}
		}
		else
		{
			if (pThis->CurrentMission == Mission::Move)
			{
				Convert(pThis, FloatingType);
				JJ_Landed = false;
			}
		}
	}
}

void TechnoExt::ExtData::OccupantsWeaponChange()
{
	if (auto const pBuilding = abstract_cast<BuildingClass*>(OwnerObject()))
	{
		if (pBuilding->Occupants.Count > 0)
		{
			int count = pBuilding->Occupants.Count;
			int rofix = 0;
			auto pBuildingTypeExt = BuildingTypeExt::ExtMap.Find(pBuilding->Type);
			auto ROFMultiplier = pBuildingTypeExt->BuildingOccupyROFMult.isset() ? pBuildingTypeExt->BuildingOccupyROFMult : RulesClass::Instance()->OccupyROFMultiplier;

			if (pBuilding->Target)
			{
				for (int i = 0; i < count; i++)
				{
					if (pBuilding->GetFireErrorWithoutRange(pBuilding->Target, 0) == FireError::OK)
						rofix++;

					if (pBuilding->FiringOccupantIndex == pBuilding->Occupants.Count - 1)
						pBuilding->FiringOccupantIndex = 0;
					else
						pBuilding->FiringOccupantIndex++;
				}
			}

			while (pBuilding->GetFireErrorWithoutRange(pBuilding->Target, 0) == FireError::ILLEGAL && count > 0 && pBuilding->GetCurrentMission() == Mission::Attack)
			{
				if (pBuilding->FiringOccupantIndex == pBuilding->Occupants.Count - 1)
					pBuilding->FiringOccupantIndex = 0;
				else
					pBuilding->FiringOccupantIndex++;

				count--;
			}

			if (rofix > 0)
				BuildingROFFix = static_cast<int>(pBuilding->GetWeapon(0)->WeaponType->ROF / (rofix * ROFMultiplier));
		}
	}
}

void TechnoExt::OccupantsWeapon(TechnoClass* pThis, TechnoExt::ExtData* pExt)
{
	if (pThis->WhatAmI() != AbstractType::Building)
		return;

	auto const pBuilding = abstract_cast<BuildingClass*>(pThis);
	if (pBuilding->CanOccupyFire() && pBuilding->Occupants.Count > 0 && pBuilding->FiringOccupantIndex >= 0 && pBuilding->FiringOccupantIndex < pBuilding->Occupants.Count)
	{
		auto pInf = pBuilding->Occupants.GetItem(pBuilding->FiringOccupantIndex);
		if (IsReallyAlive(pInf))
		{
			auto pInfType = pInf->GetTechnoType();
			if (auto pInfTypeExt = TechnoTypeExt::ExtMap.Find(pInfType))
			{
				pExt->CurrtenWeapon = pInfTypeExt->OccupyWeapons.Get(pInf).WeaponType;
			}
			else
			{
				pExt->CurrtenWeapon = nullptr;
			}
		}
		else
		{
			pExt->CurrtenWeapon = nullptr;
		}
	}
	else
	{
		pExt->CurrtenWeapon = nullptr;
	}
}

void TechnoExt::BuildingWeaponChange(TechnoClass* pThis, TechnoExt::ExtData* pExt, TechnoTypeExt::ExtData* pTypeExt)
{
	if (pThis->WhatAmI() != AbstractType::Building)
		return;

	auto const pBuilding = abstract_cast<BuildingClass*>(pThis);
	if (pBuilding->CanOccupyFire())
	{
		if (pExt->CurrtenWeapon)
		{
			pThis->GetWeapon(0)->WeaponType = pExt->CurrtenWeapon;
		}
		else
		{
			pThis->GetWeapon(0)->WeaponType = pTypeExt->Weapons.Get(0, pThis).WeaponType;
		}
	}
}

void TechnoExt::ExtData::KeepGuard()
{
	TechnoClass* pThis = OwnerObject();
	auto const pTypeExt = TypeExtData;

	if (pTypeExt->LimitedAttackRange)
	{
		if (AttackWeapon && pThis->Target)
		{
			if (pThis->DistanceFrom(pThis->Target) > AttackWeapon->Range)
			{
				pThis->ForceMission(Mission::Stop);
				pThis->Guard();
			}
		}
	}
}

void TechnoExt::ExtData::ForgetFirer()
{
	if (Attacker != nullptr || Attacker_Weapon != nullptr)
	{
		bool AttackerCheck = WeaponTypeExt::ExtMap.Find(Attacker_Weapon)->OnlyAllowOneFirer_ResetImmediately ? (Attacker->IsAlive && (Attacker->GetCurrentMission() == Mission::Attack) && (Attacker->Target == OwnerObject())) : true;
		if (Attacker_Count > 0 && AttackerCheck)
			Attacker_Count--;
		else
		{
			Attacker = nullptr;
			Attacker_Weapon = nullptr;
		}
	}
	else
	{
		Attacker_Count = 0;
	}
}

void TechnoExt::ExtData::ShieldPowered()
{
	if (Shield != nullptr && !Shield->GetType()->PoweredTechnos.empty())
	{
		ShieldTypeClass* const pShieldType = Shield->GetType();
		TechnoClass* pThis = OwnerObject();
		HouseClass* pHouse = pThis->Owner;
		bool power = !pShieldType->PoweredTechnos_Any;

		for (TechnoTypeClass* pPowerType : Shield->GetType()->PoweredTechnos)
		{
			const auto& vTechnos = HouseExt::GetOwnedTechno(pHouse, pPowerType);

			if (pShieldType->PoweredTechnos_Any)
			{
				if (std::any_of(vTechnos.begin(), vTechnos.end(), IsActivePower))
				{
					power = true;
					break;
				}
			}
			else
			{
				power &= std::any_of(vTechnos.begin(), vTechnos.end(), IsActivePower);
			}
		}

		if (!power)
			Shield->BreakShield();
	}
}

void TechnoExt::ExtData::PoweredUnit()
{
	auto const pTypeExt = TypeExtData;

	if (!pTypeExt->PoweredUnitBy.empty())
	{
		TechnoClass* pThis = OwnerObject();
		bool power = !pTypeExt->PoweredUnitBy_Any;
		HouseClass* pHouse = pThis->Owner;

		for (TechnoTypeClass* pType : pTypeExt->PoweredUnitBy)
		{
			const auto& vTechnos = HouseExt::GetOwnedTechno(pHouse, pType);

			if (pTypeExt->PoweredUnitBy_Any)
			{
				if (std::any_of(vTechnos.begin(), vTechnos.end(), IsActivePower))
				{
					power = true;
					break;
				}
			}
			else
			{
				power &= std::any_of(vTechnos.begin(), vTechnos.end(), IsActivePower);
			}
		}

		if (!power)
			InLosePower = true;

		LosePower = !power;
	}
}

void TechnoExt::ExtData::PoweredUnitDown()
{
	auto const pTypeExt = TypeExtData;

	if (!pTypeExt->PoweredUnitBy.empty())
	{
		TechnoClass* pThis = OwnerObject();

		if (LosePower)
		{
			if (pThis->WhatAmI() == AbstractType::Building)
			{
				auto const pBuilding = abstract_cast<BuildingClass*>(pThis);
				if (pBuilding->IsPowerOnline())
					pBuilding->GoOffline();
			}
			else
			{
				if (!pThis->Deactivated)
					pThis->Deactivate();
			}

			if (LosePowerParticleCount > 0)
			{
				LosePowerParticleCount--;
			}
			else
			{
				int randomX = ScenarioClass::Instance->Random.RandomRanged
				(
					pTypeExt->PoweredUnitBy_ParticleSystemXOffset.Get().X,
					pTypeExt->PoweredUnitBy_ParticleSystemXOffset.Get().Y
				);
				int randomY = ScenarioClass::Instance->Random.RandomRanged
				(
					pTypeExt->PoweredUnitBy_ParticleSystemYOffset.Get().X,
					pTypeExt->PoweredUnitBy_ParticleSystemYOffset.Get().Y
				);
				CoordStruct location = { pThis->GetCoords().X + randomX, pThis->GetCoords().Y + randomY, pThis->GetCoords().Z };
				ParticleSystemTypeClass* pParticleSystemType =
					pTypeExt->PoweredUnitBy_ParticleSystem.isset() ?
					pTypeExt->PoweredUnitBy_ParticleSystem :
					RulesClass::Instance()->DefaultSparkSystem;
				GameCreate<ParticleSystemClass>(pParticleSystemType, location, nullptr, nullptr, CoordStruct::Empty, nullptr);
				LosePowerParticleCount = pTypeExt->PoweredUnitBy_ParticleSystemSpawnDelay;
			}

			if (LosePowerAnim == nullptr)
			{
				AnimTypeClass* pSparkles =
					pTypeExt->PoweredUnitBy_Sparkles.isset() ?
					pTypeExt->PoweredUnitBy_Sparkles :
					RulesClass::Instance()->EMPulseSparkles;
				LosePowerAnim = GameCreate<AnimClass>(pSparkles, pThis->GetCoords());
				LosePowerAnim->SetOwnerObject(pThis);
				LosePowerAnim->RemainingIterations = 0xFFU;
				LosePowerAnim->Owner = pThis->GetOwningHouse();
			}
		}
		else
		{
			if (pThis->WhatAmI() == AbstractType::Building)
			{
				auto const pBuilding = abstract_cast<BuildingClass*>(pThis);
				if (!pBuilding->IsPowerOnline() && InLosePower)
				{
					pBuilding->GoOnline();
					InLosePower = false;
				}
			}
			else
			{
				if (pThis->Deactivated && InLosePower)
				{
					pThis->Reactivate();
					InLosePower = false;
				}
			}

			if (LosePowerParticleCount > 0)
				LosePowerParticleCount = 0;

			if (LosePowerAnim != nullptr)
			{
				LosePowerAnim->UnInit();
				LosePowerAnim = nullptr;
			}
		}
	}
}

void TechnoExt::ExtData::PoweredUnitDeactivate()
{
	auto const pTypeExt = TypeExtData;
	TechnoClass* pThis = OwnerObject();

	if (pTypeExt->Powered && pThis->WhatAmI() != AbstractType::Building)
	{
		if (pThis->Owner->PowerDrain > pThis->Owner->PowerOutput)
		{
			if (!pThis->Deactivated)
				pThis->Deactivate();
		}
		else
		{
			if (pThis->Deactivated)
				pThis->Reactivate();
		}
	}
}

void TechnoExt::ExtData::TechnoUpgradeAnim()
{
	TechnoClass* pThis = OwnerObject();
	const auto pTypeExt = this->TypeExtData;

	if (this->CurrentRank == Rank::Invalid)
		this->CurrentRank = pThis->Veterancy.GetRemainingLevel();

	Rank rank = pThis->Veterancy.GetRemainingLevel();

	if (rank != this->CurrentRank)
	{
		this->CurrentRank = rank;

		if (this->CurrentRank == Rank::Elite && pTypeExt->EliteAnim.Get(RulesExt::Global()->EliteAnim) != nullptr)
		{
			if (auto const pAnim = GameCreate<AnimClass>(pTypeExt->EliteAnim.Get(RulesExt::Global()->EliteAnim), pThis->Location))
			{
				pAnim->SetOwnerObject(pThis);
				pAnim->Owner = pThis->Owner;
			}
		}
		else if (CurrentRank == Rank::Veteran && pTypeExt->VeteranAnim.Get(RulesExt::Global()->VeteranAnim) != nullptr)
		{
			if (auto const pAnim = GameCreate<AnimClass>(pTypeExt->VeteranAnim.Get(RulesExt::Global()->VeteranAnim), pThis->Location))
			{
				pAnim->SetOwnerObject(pThis);
				pAnim->Owner = pThis->Owner;
			}
		}
	}
}

void TechnoExt::ExtData::TechnoAcademy()
{
	TechnoClass* pThis = OwnerObject();
	if (pThis->GetTechnoType()->Trainable && !AcademyUpgraded)
	{
		double Veterancy = 0;

		auto pHouseExt = HouseExt::ExtMap.Find(pThis->Owner);
		switch (pThis->WhatAmI())
		{
		case AbstractType::Aircraft:
			for (size_t i = 0; i < pHouseExt->AircraftVeterancyTypes.size(); i++)
			{
				if (pThis->GetTechnoType() == pHouseExt->AircraftVeterancyTypes[i])
				{
					Veterancy = pHouseExt->AircraftVeterancy[i];
					break;
				}
			}
			break;
		case AbstractType::Building:
			for (size_t i = 0; i < pHouseExt->BuildingVeterancyTypes.size(); i++)
			{
				if (pThis->GetTechnoType() == pHouseExt->BuildingVeterancyTypes[i])
				{
					Veterancy = pHouseExt->BuildingVeterancy[i];
					break;
				}
			}
			break;
		case AbstractType::Infantry:
			for (size_t i = 0; i < pHouseExt->InfantryVeterancyTypes.size(); i++)
			{
				if (pThis->GetTechnoType() == pHouseExt->InfantryVeterancyTypes[i])
				{
					Veterancy = pHouseExt->InfantryVeterancy[i];
					break;
				}
			}
			break;
		case AbstractType::Unit:
			if (pThis->GetTechnoType()->Organic)
			{
				for (size_t i = 0; i < pHouseExt->InfantryVeterancyTypes.size(); i++)
				{
					if (pThis->GetTechnoType() == pHouseExt->InfantryVeterancyTypes[i])
					{
						Veterancy = pHouseExt->InfantryVeterancy[i];
						break;
					}
				}
			}
			else if (pThis->GetTechnoType()->ConsideredAircraft)
			{
				for (size_t i = 0; i < pHouseExt->AircraftVeterancyTypes.size(); i++)
				{
					if (pThis->GetTechnoType() == pHouseExt->AircraftVeterancyTypes[i])
					{
						Veterancy = pHouseExt->AircraftVeterancy[i];
						break;
					}
				}
			}
			else if (pThis->GetTechnoType()->Naval)
			{
				for (size_t i = 0; i < pHouseExt->NavalVeterancyTypes.size(); i++)
				{
					if (pThis->GetTechnoType() == pHouseExt->NavalVeterancyTypes[i])
					{
						Veterancy = pHouseExt->NavalVeterancy[i];
						break;
					}
				}
			}
			else
			{
				for (size_t i = 0; i < pHouseExt->VehicleVeterancyTypes.size(); i++)
				{
					if (pThis->GetTechnoType() == pHouseExt->VehicleVeterancyTypes[i])
					{
						Veterancy = pHouseExt->VehicleVeterancy[i];
						break;
					}
				}
			}
			break;
		default:
			break;
		}

		for (auto pTechno : *TechnoClass::Array)
		{
			auto pTypeExt = TechnoTypeExt::ExtMap.Find(pTechno->GetTechnoType());
			if (pTechno->Owner == pThis->Owner && pTypeExt->IsExtendAcademy)
			{
				if (pTypeExt->Academy_Powered)
				{
					if (pTechno->WhatAmI() == AbstractType::Building)
					{
						if (!IsActivePower(pTechno))
						{
							continue;
						}
					}
					else
					{
						if (!IsActive(pTechno))
						{
							continue;
						}
					}
				}

				if ((pThis->WhatAmI() == AbstractType::Infantry) ||
					((pThis->WhatAmI() == AbstractType::Unit) && pThis->GetTechnoType()->Organic))
				{
					bool AcademyAllow = false;
					if (!pTypeExt->Academy_InfantryVeterancy_Types.empty())
					{
						for (TechnoTypeClass* pType : pTypeExt->Academy_InfantryVeterancy_Types)
						{
							if (pThis->GetTechnoType() == pType)
							{
								AcademyAllow = true;
								break;
							}
						}
					}
					else
					{
						AcademyAllow = true;
					}

					if (!pTypeExt->Academy_InfantryVeterancy_Ignore.empty())
					{
						for (TechnoTypeClass* pType : pTypeExt->Academy_InfantryVeterancy_Ignore)
						{
							if (pThis->GetTechnoType() == pType)
							{
								AcademyAllow = false;
								break;
							}
						}
					}

					if (AcademyAllow)
					{
						if (pTypeExt->Academy_InfantryVeterancy_Cumulative)
							Veterancy += pTypeExt->Academy_InfantryVeterancy;
						else if (Veterancy < pTypeExt->Academy_InfantryVeterancy)
							Veterancy = pTypeExt->Academy_InfantryVeterancy;
					}
				}
				else if ((pThis->WhatAmI() == AbstractType::Aircraft) ||
					((pThis->WhatAmI() == AbstractType::Unit) && pThis->GetTechnoType()->ConsideredAircraft))
				{
					bool AcademyAllow = false;
					if (!pTypeExt->Academy_AircraftVeterancy_Types.empty())
					{
						for (TechnoTypeClass* pType : pTypeExt->Academy_AircraftVeterancy_Types)
						{
							if (pThis->GetTechnoType() == pType)
							{
								AcademyAllow = true;
								break;
							}
						}
					}
					else
					{
						AcademyAllow = true;
					}

					if (!pTypeExt->Academy_AircraftVeterancy_Ignore.empty())
					{
						for (TechnoTypeClass* pType : pTypeExt->Academy_AircraftVeterancy_Ignore)
						{
							if (pThis->GetTechnoType() == pType)
							{
								AcademyAllow = false;
								break;
							}
						}
					}

					if (AcademyAllow)
					{
						if (pTypeExt->Academy_AircraftVeterancy_Cumulative)
							Veterancy += pTypeExt->Academy_AircraftVeterancy;
						else if (Veterancy < pTypeExt->Academy_AircraftVeterancy)
							Veterancy = pTypeExt->Academy_AircraftVeterancy;
					}
				}
				else if ((pThis->WhatAmI() == AbstractType::Unit) && !pThis->GetTechnoType()->Naval)
				{
					bool AcademyAllow = false;
					if (!pTypeExt->Academy_VehicleVeterancy_Types.empty())
					{
						for (TechnoTypeClass* pType : pTypeExt->Academy_VehicleVeterancy_Types)
						{
							if (pThis->GetTechnoType() == pType)
							{
								AcademyAllow = true;
								break;
							}
						}
					}
					else
					{
						AcademyAllow = true;
					}

					if (!pTypeExt->Academy_VehicleVeterancy_Ignore.empty())
					{
						for (TechnoTypeClass* pType : pTypeExt->Academy_VehicleVeterancy_Ignore)
						{
							if (pThis->GetTechnoType() == pType)
							{
								AcademyAllow = false;
								break;
							}
						}
					}

					if (AcademyAllow)
					{
						if (pTypeExt->Academy_VehicleVeterancy_Cumulative)
							Veterancy += pTypeExt->Academy_VehicleVeterancy;
						else if (Veterancy < pTypeExt->Academy_VehicleVeterancy)
							Veterancy = pTypeExt->Academy_VehicleVeterancy;
					}
				}
				else if ((pThis->WhatAmI() == AbstractType::Unit) && pThis->GetTechnoType()->Naval)
				{
					bool AcademyAllow = false;
					if (!pTypeExt->Academy_NavalVeterancy_Types.empty())
					{
						for (TechnoTypeClass* pType : pTypeExt->Academy_NavalVeterancy_Types)
						{
							if (pThis->GetTechnoType() == pType)
							{
								AcademyAllow = true;
								break;
							}
						}
					}
					else
					{
						AcademyAllow = true;
					}

					if (!pTypeExt->Academy_NavalVeterancy_Ignore.empty())
					{
						for (TechnoTypeClass* pType : pTypeExt->Academy_NavalVeterancy_Ignore)
						{
							if (pThis->GetTechnoType() == pType)
							{
								AcademyAllow = false;
								break;
							}
						}
					}

					if (AcademyAllow)
					{
						if (pTypeExt->Academy_NavalVeterancy_Cumulative)
							Veterancy += pTypeExt->Academy_NavalVeterancy;
						else if (Veterancy < pTypeExt->Academy_NavalVeterancy)
							Veterancy = pTypeExt->Academy_NavalVeterancy;
					}
				}
				else if (pThis->WhatAmI() == AbstractType::Building)
				{
					bool AcademyAllow = false;
					if (!pTypeExt->Academy_BuildingVeterancy_Types.empty())
					{
						for (TechnoTypeClass* pType : pTypeExt->Academy_BuildingVeterancy_Types)
						{
							if (pThis->GetTechnoType() == pType)
							{
								AcademyAllow = true;
								break;
							}
						}
					}
					else
					{
						AcademyAllow = true;
					}

					if (!pTypeExt->Academy_BuildingVeterancy_Ignore.empty())
					{
						for (TechnoTypeClass* pType : pTypeExt->Academy_BuildingVeterancy_Ignore)
						{
							if (pThis->GetTechnoType() == pType)
							{
								AcademyAllow = false;
								break;
							}
						}
					}

					if (AcademyAllow)
					{
						if (pTypeExt->Academy_BuildingVeterancy_Cumulative)
							Veterancy += pTypeExt->Academy_BuildingVeterancy;
						else if (Veterancy < pTypeExt->Academy_BuildingVeterancy)
							Veterancy = pTypeExt->Academy_BuildingVeterancy;
					}
				}
			}
		}

		if (Veterancy > 0)
		{
			VeterancyStruct* vstruct = &pThis->Veterancy;
			if (Veterancy >= 2)
			{
				vstruct->SetElite();
			}
			else
			{
				vstruct->Add(-Veterancy);
				if (vstruct->IsNegative())
				{
					vstruct->Reset();
					vstruct->Add(Veterancy);
				}
				else
				{
					vstruct->Add(Veterancy);
				}
			}
		}

		AcademyUpgraded = true;
	}
}

void TechnoExt::ExtData::TechnoAcademyReset()
{
	TechnoClass* pThis = OwnerObject();
	const auto pTypeExt = this->TypeExtData;
	if (pTypeExt->IsExtendAcademy && pTypeExt->Academy_Immediately && !AcademyReset)
	{
		if (pTypeExt->Academy_Powered)
		{
			if (pThis->WhatAmI() == AbstractType::Building)
			{
				if (!IsActivePower(pThis))
				{
					return;
				}
			}
			else
			{
				if (!IsActive(pThis))
				{
					return;
				}
			}
		}

		for (auto pTechno : *TechnoClass::Array)
		{
			if (pTechno->Owner == pThis->Owner && pTechno->GetTechnoType()->Trainable)
			{
				double Veterancy = 0;
				if ((pTechno->WhatAmI() == AbstractType::Infantry) ||
					((pTechno->WhatAmI() == AbstractType::Unit) && pTechno->GetTechnoType()->Organic))
				{
					bool AcademyAllow = false;
					if (!pTypeExt->Academy_InfantryVeterancy_Types.empty())
					{
						for (TechnoTypeClass* pType : pTypeExt->Academy_InfantryVeterancy_Types)
						{
							if (pTechno->GetTechnoType() == pType)
							{
								AcademyAllow = true;
								break;
							}
						}
					}
					else
					{
						AcademyAllow = true;
					}

					if (!pTypeExt->Academy_InfantryVeterancy_Ignore.empty())
					{
						for (TechnoTypeClass* pType : pTypeExt->Academy_InfantryVeterancy_Ignore)
						{
							if (pTechno->GetTechnoType() == pType)
							{
								AcademyAllow = false;
								break;
							}
						}
					}

					if (AcademyAllow)
						Veterancy = pTypeExt->Academy_InfantryVeterancy;
				}
				else if ((pTechno->WhatAmI() == AbstractType::Aircraft) ||
					((pTechno->WhatAmI() == AbstractType::Unit) && pTechno->GetTechnoType()->ConsideredAircraft))
				{
					bool AcademyAllow = false;
					if (!pTypeExt->Academy_AircraftVeterancy_Types.empty())
					{
						for (TechnoTypeClass* pType : pTypeExt->Academy_AircraftVeterancy_Types)
						{
							if (pTechno->GetTechnoType() == pType)
							{
								AcademyAllow = true;
								break;
							}
						}
					}
					else
					{
						AcademyAllow = true;
					}

					if (!pTypeExt->Academy_AircraftVeterancy_Ignore.empty())
					{
						for (TechnoTypeClass* pType : pTypeExt->Academy_AircraftVeterancy_Ignore)
						{
							if (pTechno->GetTechnoType() == pType)
							{
								AcademyAllow = false;
								break;
							}
						}
					}

					if (AcademyAllow)
						Veterancy = pTypeExt->Academy_AircraftVeterancy;
				}
				else if ((pTechno->WhatAmI() == AbstractType::Unit) && !pTechno->GetTechnoType()->Naval)
				{
					bool AcademyAllow = false;
					if (!pTypeExt->Academy_VehicleVeterancy_Types.empty())
					{
						for (TechnoTypeClass* pType : pTypeExt->Academy_VehicleVeterancy_Types)
						{
							if (pTechno->GetTechnoType() == pType)
							{
								AcademyAllow = true;
								break;
							}
						}
					}
					else
					{
						AcademyAllow = true;
					}

					if (!pTypeExt->Academy_VehicleVeterancy_Ignore.empty())
					{
						for (TechnoTypeClass* pType : pTypeExt->Academy_VehicleVeterancy_Ignore)
						{
							if (pTechno->GetTechnoType() == pType)
							{
								AcademyAllow = false;
								break;
							}
						}
					}

					if (AcademyAllow)
						Veterancy = pTypeExt->Academy_VehicleVeterancy;
				}
				else if ((pTechno->WhatAmI() == AbstractType::Unit) && pTechno->GetTechnoType()->Naval)
				{
					bool AcademyAllow = false;
					if (!pTypeExt->Academy_NavalVeterancy_Types.empty())
					{
						for (TechnoTypeClass* pType : pTypeExt->Academy_NavalVeterancy_Types)
						{
							if (pTechno->GetTechnoType() == pType)
							{
								AcademyAllow = true;
								break;
							}
						}
					}
					else
					{
						AcademyAllow = true;
					}

					if (!pTypeExt->Academy_NavalVeterancy_Ignore.empty())
					{
						for (TechnoTypeClass* pType : pTypeExt->Academy_NavalVeterancy_Ignore)
						{
							if (pTechno->GetTechnoType() == pType)
							{
								AcademyAllow = false;
								break;
							}
						}
					}

					if (AcademyAllow)
						Veterancy += pTypeExt->Academy_NavalVeterancy;
				}
				else if (pTechno->WhatAmI() == AbstractType::Building)
				{
					bool AcademyAllow = false;
					if (!pTypeExt->Academy_BuildingVeterancy_Types.empty())
					{
						for (TechnoTypeClass* pType : pTypeExt->Academy_BuildingVeterancy_Types)
						{
							if (pTechno->GetTechnoType() == pType)
							{
								AcademyAllow = true;
								break;
							}
						}
					}
					else
					{
						AcademyAllow = true;
					}

					if (!pTypeExt->Academy_BuildingVeterancy_Ignore.empty())
					{
						for (TechnoTypeClass* pType : pTypeExt->Academy_BuildingVeterancy_Ignore)
						{
							if (pTechno->GetTechnoType() == pType)
							{
								AcademyAllow = false;
								break;
							}
						}
					}

					if (AcademyAllow)
						Veterancy = pTypeExt->Academy_BuildingVeterancy;
				}

				if (pTypeExt->Academy_Immediately_Addition)
				{
					VeterancyStruct* vstruct = &pTechno->Veterancy;
					if (Veterancy < 0)
					{
						vstruct->Add(Veterancy);
						if (vstruct->IsNegative())
							vstruct->Reset();
					}
					else
					{
						vstruct->Add(Veterancy);
						if (vstruct->IsElite())
							vstruct->SetElite();
					}
				}
				else
				{
					if (Veterancy > 0)
					{
						VeterancyStruct* vstruct = &pTechno->Veterancy;
						if (Veterancy >= 2)
						{
							vstruct->SetElite();
						}
						else
						{
							vstruct->Add(-Veterancy);
							if (vstruct->IsNegative())
							{
								vstruct->Reset();
								vstruct->Add(Veterancy);
							}
							else
							{
								vstruct->Add(Veterancy);
							}
						}
					}
				}
			}
		}

		AcademyReset = true;
	}
}

void TechnoExt::ExtData::ControlConverts()
{
	TechnoClass* pThis = OwnerObject();
	auto const pTypeExt = TypeExtData;

	if (pThis->Owner->IsControlledByHuman())
	{
		if (pTypeExt->Convert_Player)
		{
			Convert(pThis, pTypeExt->Convert_Player);
		}
	}
	else if (strcmp(pThis->Owner->PlainName,"Computer") == 0)
	{
		if (pTypeExt->Convert_AI)
		{
			Convert(pThis, pTypeExt->Convert_AI);
		}
	}
	else
	{
		if (pTypeExt->Convert_Netural)
		{
			Convert(pThis, pTypeExt->Convert_Netural);
		}
	}
}

void TechnoExt::ExtData::MoveConverts()
{
	TechnoClass* pThis = OwnerObject();
	auto const pTypeExt = TypeExtData;

	switch (pThis->WhatAmI())
	{
	case AbstractType::Infantry:
	{
		if (const auto pTechno = abstract_cast<InfantryClass*>(pThis))
		{
			if (pTechno->GetCurrentSpeed() > 0)
			{
				if (pTypeExt->Convert_Move != nullptr && pThis->GetTechnoType() != pTypeExt->Convert_Move)
					Convert(pThis, pTypeExt->Convert_Move);
			}
			else
			{
				if (pTypeExt->Convert_Stand != nullptr && pThis->GetTechnoType() != pTypeExt->Convert_Stand)
					Convert(pThis, pTypeExt->Convert_Stand);
			}
		}
	}break;
	case AbstractType::Unit:
	{
		if (const auto pTechno = abstract_cast<UnitClass*>(pThis))
		{
			if (pTechno->GetCurrentSpeed() > 0)
			{
				if (pTypeExt->Convert_Move != nullptr && pThis->GetTechnoType() != pTypeExt->Convert_Move)
					Convert(pThis, pTypeExt->Convert_Move);
			}
			else
			{
				if (pTypeExt->Convert_Stand != nullptr && pThis->GetTechnoType() != pTypeExt->Convert_Stand)
					Convert(pThis, pTypeExt->Convert_Stand);
			}
		}
	}break;
	case AbstractType::Aircraft:
	{
		if (const auto pTechno = abstract_cast<AircraftClass*>(pThis))
		{
			if (pTechno->GetCurrentSpeed() > 0)
			{
				if (pTypeExt->Convert_Move != nullptr && pThis->GetTechnoType() != pTypeExt->Convert_Move)
					Convert(pThis, pTypeExt->Convert_Move);
			}
			else
			{
				if (pTypeExt->Convert_Stand != nullptr && pThis->GetTechnoType() != pTypeExt->Convert_Stand)
					Convert(pThis, pTypeExt->Convert_Stand);
			}
		}
	}break;
	default:
		break;
	}
}

void TechnoExt::ReturnMoney(TechnoClass* pThis, HouseClass* pHouse, CoordStruct pLocation)
{
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	int money = 0;
	if (pTypeExt->ReturnMoney != 0)
		money = pTypeExt->ReturnMoney;
	else
		money = static_cast<int>(pThis->GetTechnoType()->GetActualCost(pHouse) * pTypeExt->ReturnMoney_Percentage);

	if (money != 0)
	{
		pHouse->TransactMoney(money);
		if (pTypeExt->ReturnMoney_Display)
			FlyingStrings::AddMoneyString(money, pHouse, pTypeExt->ReturnMoney_Display_Houses, pLocation, pTypeExt->ReturnMoney_Display_Offset);
	}
}

void TechnoExt::ExtData::DeployAttachEffect()
{
	TechnoClass* pThis = OwnerObject();
	if (auto const pInfantry = abstract_cast<InfantryClass*>(OwnerObject()))
	{
		if (pInfantry->IsDeployed())
		{
			const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
			if (DeployAttachEffectsCount > 0)
			{
				DeployAttachEffectsCount--;
			}
			else
			{
				for (size_t i = 0; i < pTypeExt->DeployAttachEffects.size(); i++)
					AttachEffect(pThis, pThis, pTypeExt->DeployAttachEffects[i]);
				DeployAttachEffectsCount = pTypeExt->DeployAttachEffects_Delay;
			}
		}
		else if (DeployAttachEffectsCount > 0)
			DeployAttachEffectsCount--;
	}
}

void TechnoExt::ExtData::AttachEffectNext()
{
	TechnoClass* pThis = OwnerObject();
	if (NextAttachEffects.size() > 0)
	{
		for (size_t i = 0; i < NextAttachEffects.size(); i++)
		{
			AttachEffect(pThis, NextAttachEffectsOwner, NextAttachEffects[i]);
		}
		NextAttachEffects.clear();
		NextAttachEffectsOwner = nullptr;
	}
}

void TechnoExt::ExtData::MoveChangeLocomotor()
{
	TechnoClass* pThis = OwnerObject();
	auto const pTypeExt = TypeExtData;
	if (pTypeExt->Locomotor_Change)
	{
		FootClass* pFoot = abstract_cast<FootClass*>(pThis);
		AbstractClass* target;
		bool isattcking = false;
		if (pThis->Target)
		{
			target = pThis->Target;
			isattcking = true;
		}
		else
		{
			target = pFoot->Destination;
		}

		if (!HasChangeLocomotor)
		{
			auto mission = pThis->GetCurrentMission();
			if (!pThis->IsWarpingIn() && !pThis->IsInAir() && pThis->GetHeight() >= 0)
			{
				CellClass* pCell = MapClass::Instance->TryGetCellAt(CellClass::Coord2Cell(pThis->Location));

				CoordStruct crdDest;

				if (pCell != nullptr)
				{
					crdDest = pCell->GetCoordsWithBridge();
				}
				else
				{
					crdDest = pThis->Location;
					crdDest.Z = MapClass::Instance->GetCellFloorHeight(crdDest);
				}

				crdDest.Z += pThis->GetHeight();

				auto facing = pThis->GetRealFacing();
				bool selected = pThis->IsSelected;

				if ((pThis->DistanceFrom(target) >= pTypeExt->Locomotor_ChangeMinRange) && (pThis->DistanceFrom(target) <= pTypeExt->Locomotor_ChangeMaxRange))
				{
					if (IsTypeLocomotor)
					{
						pThis->ForceMission(Mission::Stop);
						pThis->Limbo();
						switch (pTypeExt->Locomotor_ChangeTo)
						{
						case Locomotors::Drive:
							ChangeLocomotorTo(pThis, LocomotionClass::CLSIDs::Drive.get());
							break;
						case Locomotors::Droppod:
							ChangeLocomotorTo(pThis, LocomotionClass::CLSIDs::Droppod.get());
							break;
						case Locomotors::Fly:
							ChangeLocomotorTo(pThis, LocomotionClass::CLSIDs::Fly.get());
							break;
						case Locomotors::Hover:
							ChangeLocomotorTo(pThis, LocomotionClass::CLSIDs::Hover.get());
							break;
						case Locomotors::Jumpjet:
							ChangeLocomotorTo(pThis, LocomotionClass::CLSIDs::Jumpjet.get());
							break;
						case Locomotors::Mech:
							ChangeLocomotorTo(pThis, LocomotionClass::CLSIDs::Mech.get());
							break;
						case Locomotors::Rocket:
							ChangeLocomotorTo(pThis, LocomotionClass::CLSIDs::Rocket.get());
							break;
						case Locomotors::Ship:
							ChangeLocomotorTo(pThis, LocomotionClass::CLSIDs::Ship.get());
							break;
						case Locomotors::Teleport:
							ChangeLocomotorTo(pThis, LocomotionClass::CLSIDs::Teleport.get());
							break;
						case Locomotors::Tunnel:
							ChangeLocomotorTo(pThis, LocomotionClass::CLSIDs::Tunnel.get());
							break;
						case Locomotors::Walk:
							ChangeLocomotorTo(pThis, LocomotionClass::CLSIDs::Walk.get());
							break;
						default:
							break;
						}
						++Unsorted::IKnowWhatImDoing;
						pThis->Unlimbo(crdDest, facing.GetDir());
						--Unsorted::IKnowWhatImDoing;
						if (selected)
						{
							pThis->Select();
						}

						if (isattcking)
						{
							pThis->SetTarget(target);
							pFoot->ReceiveCommand(pThis, RadioCommand::RequestAttack, target);
						}
						else
						{
							pFoot->ReceiveCommand(pThis, RadioCommand::RequestMoveTo, target);
						}

						pThis->ForceMission(mission);

						IsTypeLocomotor = false;
					}
				}
				else if (!IsTypeLocomotor)
				{
					pThis->ForceMission(Mission::Stop);
					pThis->Limbo();
					ChangeLocomotorTo(pThis, pThis->GetTechnoType()->Locomotor);
					++Unsorted::IKnowWhatImDoing;
					pThis->Unlimbo(crdDest, facing.GetDir());
					--Unsorted::IKnowWhatImDoing;
					if (selected)
					{
						pThis->Select();
					}

					if (isattcking)
					{
						pThis->SetTarget(target);
						pFoot->ReceiveCommand(pThis, RadioCommand::RequestAttack, target);
					}
					else
					{
						pFoot->ReceiveCommand(pThis, RadioCommand::RequestMoveTo, target);
					}

					pThis->ForceMission(mission);

					IsTypeLocomotor = true;
				}
			}

			ChangeLocomotorTarget = target;
			HasChangeLocomotor = true;
		}
		else
		{
			if (target != ChangeLocomotorTarget)
			{
				HasChangeLocomotor = false;
			}
		}
	}
}

void TechnoExt::ExtData::DisableBeSelect()
{
	TechnoClass* pThis = OwnerObject();
	if (pThis->IsSelected)
	{
		for (auto& pAE : AttachEffects)
		{
			if (!pAE->IsActive())
				continue;

			if (pAE->Type->DisableBeSelect)
			{
				pThis->Deselect();
				break;
			}
		}
	}
}

void TechnoExt::ExtData::TemporalTeamCheck()
{
	TechnoClass* pThis = OwnerObject();

	if (TemporalTarget)
	{
		if (IsReallyAlive(TemporalTarget))
		{
			auto pTargetExt = TechnoExt::ExtMap.Find(TemporalTarget);

			if (pTargetExt->TemporalTarget)
				pTargetExt->TemporalTeamCheck();

			if (IsActivePower(pThis) && pThis->TemporalImUsing)
			{
				if (pThis->TemporalImUsing->Target && pThis->TemporalImUsing->Target == TemporalTarget)
				{
					std::vector<TechnoClass*> TempTeam;
					std::vector<TechnoClass*> CheckTeam;
					pTargetExt->IsTemporalTarget = true;

					bool HasOwner = false;
					for (auto pOwner : pTargetExt->TemporalOwner)
					{
						if (pOwner == pThis)
						{
							HasOwner = true;
							break;
						}
					}
					if (!HasOwner)
					{
						pTargetExt->TemporalOwner.emplace_back(pThis);
					}

					for (auto pTarget : TemporalTeam)
					{
						if (!IsReallyAlive(pTarget))
							continue;

						auto pEachTargetExt = TechnoExt::ExtMap.Find(pTarget);
						bool HasStand = false;
						for (auto pStand : pEachTargetExt->TemporalStand)
						{
							if (auto pStandExt = TechnoExt::ExtMap.Find(pStand))
							{
								if (pStandExt->TemporalStandFirer == pThis)
								{
									HasStand = true;
									break;
								}
							}
						}

						if (!HasStand)
						{
							auto TempStand = abstract_cast<TechnoClass*>(pThis->GetTechnoType()->CreateObject(pThis->Owner));
							TempStand->Limbo();
							TempStand->TemporalImUsing = GameCreate<TemporalClass>(TempStand);
							TempStand->TemporalImUsing->Fire(pTarget);
							TempStand->TemporalImUsing->WarpRemaining = pTarget->Health * 10;
							TempStand->TemporalImUsing->WarpPerStep = pThis->TemporalImUsing->WarpPerStep;

							auto pStandExt = TechnoExt::ExtMap.Find(TempStand);
							pStandExt->TemporalStandTarget = pTarget;
							pStandExt->TemporalStandFirer = pThis;
							pStandExt->TemporalStandOwner = TemporalTarget;
							HouseExt::ExtMap.Find(pThis->Owner)->TemporalStands.emplace_back(TempStand);
							pEachTargetExt->TemporalStand.emplace_back(TempStand);
						}

						TempTeam.emplace_back(pTarget);

						if (!pEachTargetExt->IsTemporalTarget)
							CheckTeam.emplace_back(pTarget);
					}

					TemporalTeam = TempTeam;

					if (CheckTeam.size() > 0 && pThis->TemporalImUsing->WarpRemaining <= pThis->TemporalImUsing->WarpPerStep)
					{
						pThis->TemporalImUsing->WarpRemaining += (15 * pThis->TemporalImUsing->WarpPerStep);
					}
				}
				else
				{
					for (size_t j = 0; j < pTargetExt->TemporalOwner.size(); j++)
					{
						if (pTargetExt->TemporalOwner[j] == pThis)
						{
							for (auto pTarget : TemporalTeam)
							{
								if (!IsReallyAlive(pTarget))
									continue;

								auto pEachTargetExt = TechnoExt::ExtMap.Find(pTarget);

								for (size_t i = 0; i < pEachTargetExt->TemporalStand.size(); i++)
								{
									if (auto pStandExt = TechnoExt::ExtMap.Find(pEachTargetExt->TemporalStand[i]))
									{
										if (pStandExt->TemporalStandFirer == pThis)
										{
											if (IsReallyAlive(pEachTargetExt->TemporalStand[i]))
											{
												pEachTargetExt->TemporalStand[i]->SetOwningHouse(HouseClass::FindCivilianSide(), false);
												KillSelf(pEachTargetExt->TemporalStand[i], AutoDeathBehavior::Vanish);
											}
											pEachTargetExt->TemporalStand.erase(pEachTargetExt->TemporalStand.begin() + i);
											break;
										}
									}
								}
							}

							pTargetExt->IsTemporalTarget = false;
							TemporalTeam.clear();
							TemporalTarget = nullptr;
							pTargetExt->TemporalOwner.erase(pTargetExt->TemporalOwner.begin() + j);
						}
						else
						{
							if (!IsReallyAlive(pTargetExt->TemporalOwner[j]))
								pTargetExt->TemporalOwner.erase(pTargetExt->TemporalOwner.begin() + j);
						}
					}
				}
			}
			else
			{
				for (size_t j = 0; j < pTargetExt->TemporalOwner.size(); j++)
				{
					if (pTargetExt->TemporalOwner[j] == pThis)
						pTargetExt->TemporalOwner.erase(pTargetExt->TemporalOwner.begin() + j);
					else
					{
						if (!IsReallyAlive(pTargetExt->TemporalOwner[j]))
							pTargetExt->TemporalOwner.erase(pTargetExt->TemporalOwner.begin() + j);
					}
				}

				for (auto pTarget : TemporalTeam)
				{
					if (!IsReallyAlive(pTarget))
						continue;

					auto pEachTargetExt = TechnoExt::ExtMap.Find(pTarget);

					for (size_t i = 0; i < pEachTargetExt->TemporalStand.size(); i++)
					{
						auto pStandExt = TechnoExt::ExtMap.Find(pEachTargetExt->TemporalStand[i]);
						if (pStandExt->TemporalStandFirer == pThis)
						{
							if (IsReallyAlive(pEachTargetExt->TemporalStand[i]))
							{
								pEachTargetExt->TemporalStand[i]->SetOwningHouse(HouseClass::FindCivilianSide(), false);
								KillSelf(pEachTargetExt->TemporalStand[i], AutoDeathBehavior::Vanish);
							}
							pEachTargetExt->TemporalStand.erase(pEachTargetExt->TemporalStand.begin() + i);
							break;
						}
					}
				}
				pTargetExt->IsTemporalTarget = false;
				TemporalTeam.clear();
				TemporalTarget = nullptr;
			}
		}
	}
}

void TechnoExt::ExtData::SetSyncDeathOwner()
{
	TechnoClass* pThis = OwnerObject();
	if (pThis->CaptureManager && TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType())->MindControl_SyncDeath)
	{
		for (int i = 0; i < pThis->CaptureManager->ControlNodes.Count; i++)
		{
			auto pTechnoExt = TechnoExt::ExtMap.Find(pThis->CaptureManager->ControlNodes[i]->Techno);
			if (pTechnoExt->SyncDeathOwner != pThis)
				pTechnoExt->SyncDeathOwner = pThis;
		}
	}
}

void TechnoExt::ExtData::DeathWithSyncDeathOwner()
{
	TechnoClass* pThis = OwnerObject();
	if (SyncDeathOwner)
	{
		if (!IsReallyAlive(SyncDeathOwner))
			KillSelf(pThis, AutoDeathBehavior::Kill);
		else
		{
			if (pThis->MindControlledBy)
			{
				if (pThis->MindControlledBy != SyncDeathOwner)
					SyncDeathOwner = nullptr;
			}
			else
				SyncDeathOwner = nullptr;
		}
	}
}

void TechnoExt::ExtData::ApplyMobileRefinery()
{
	TechnoClass* pThis = OwnerObject();

	if (pThis->AbstractFlags & AbstractFlags::Foot)
	{
		const auto pTypeExt = this->TypeExtData;

		if (!pTypeExt->MobileRefinery ||
			(pTypeExt->MobileRefinery_TransRate > 0 && Unsorted::CurrentFrame % pTypeExt->MobileRefinery_TransRate))
			return;

		size_t cellCount = Math::max(pTypeExt->MobileRefinery_FrontOffset.size(), pTypeExt->MobileRefinery_LeftOffset.size());

		if (!cellCount)
			cellCount = 1;

		CoordStruct flh = { 0,0,0 };

		for (size_t idx = 0; idx < cellCount; idx++)
		{
			flh.X = static_cast<int>(pTypeExt->MobileRefinery_FrontOffset.size() > idx ? pTypeExt->MobileRefinery_FrontOffset[idx] * Unsorted::LeptonsPerCell : 0);
			flh.Y = static_cast<int>(pTypeExt->MobileRefinery_LeftOffset.size() > idx ? pTypeExt->MobileRefinery_LeftOffset[idx] * Unsorted::LeptonsPerCell : 0);
			CoordStruct pos = TechnoExt::GetFLHAbsoluteCoords(pThis, flh, false);
			CellClass* pCell = MapClass::Instance->TryGetCellAt(pos);

			if (!pCell)
				return;

			auto loc = pCell->GetCoords();
			loc.Z = pThis->Location.Z;
			int tValue = pCell->GetContainedTiberiumValue();

			if (tValue)
			{
				int tibValue = TiberiumClass::Array->GetItem(pCell->GetContainedTiberiumIndex())->Value;
				int tAmount = static_cast<int>(tValue * 1.0 / tibValue);
				int amount = pTypeExt->MobileRefinery_AmountPerCell ? Math::min(pTypeExt->MobileRefinery_AmountPerCell, tAmount) : tAmount;
				pCell->ReduceTiberium(amount);
				int value = static_cast<int>(amount * tibValue * pTypeExt->MobileRefinery_CashMultiplier);
				pThis->Owner->TransactMoney(value);

				if (pTypeExt->MobileRefinery_Display)
				{
					Point2D location = { 0,0 };
					TacticalClass::Instance->CoordsToScreen(&location, &loc);
					location -= TacticalClass::Instance->TacticalPos;
					RectangleStruct rect = DSurface::Composite->GetRect();
					RectangleStruct bound = { location.X, location.Y, 10, 12 };

					if (bound.X > 0 && bound.X + bound.Width < rect.Width &&
						bound.Y > 0 && bound.Y + bound.Height < rect.Height - 32)
					{
						ColorStruct color = pTypeExt->MobileRefinery_DisplayColor;
						wchar_t moneyStr[0x20];
						swprintf_s(moneyStr, L"%ls%ls%d", L"+", Phobos::UI::CostLabel, value);
						FlyingStrings::Add(moneyStr, loc, color);
					}
				}

				if (!pTypeExt->MobileRefinery_Anims.empty())
				{
					AnimTypeClass* pAnimType = nullptr;
					int facing = pThis->PrimaryFacing.Current().GetValue<3>();

					if (facing >= 7)
						facing = 0;
					else
						facing++;

					switch (pTypeExt->MobileRefinery_Anims.size())
					{
					case 1:
						pAnimType = pTypeExt->MobileRefinery_Anims[0];
						break;
					case 8:
						pAnimType = pTypeExt->MobileRefinery_Anims[facing];
						break;
					default:
						pAnimType = pTypeExt->MobileRefinery_Anims
							[ScenarioClass::Instance->Random.RandomRanged(0, pTypeExt->MobileRefinery_Anims.size() - 1)];
						break;
					}

					if (pAnimType)
					{
						if (auto pAnim = GameCreate<AnimClass>(pAnimType, pos))
						{
							pAnim->Owner = pThis->Owner;

							if (pTypeExt->MobileRefinery_AnimMove)
								pAnim->SetOwnerObject(pThis);
						}
					}
				}
			}
		}
	}
}

void TechnoExt::ExtData::UpdateAttackedWeaponTimer()
{
	for (int& iTime : AttackedWeapon_Timer)
	{
		if (iTime > 0)
			iTime--;
	}
}

void TechnoExt::ExtData::CheckAttachEffects()
{
	TechnoClass* pThis = this->OwnerObject();

	if (!TechnoExt::IsReallyAlive(pThis))
		return;

	if (!AttachEffects_Initialized)
	{
		TechnoTypeExt::ExtData* pTypeExt = this->TypeExtData;
		HouseTypeClass* pHouseType = pThis->Owner->Type;

		for (const auto pAEType : pTypeExt->AttachEffects)
		{
			AttachEffect(pThis, pThis, pAEType);
		}

		if (const auto pAEType = HouseTypeExt::GetAttachEffectOnInit(pHouseType, pThis))
			AttachEffect(pThis, pThis, pAEType);

		this->AttachEffects_Initialized = true;
	}

	this->AttachEffects.erase
	(
		std::remove_if
		(
			this->AttachEffects.begin(),
			this->AttachEffects.end(),
			[](const std::unique_ptr<AttachEffectClass>& pAE)
			{
				return pAE == nullptr
					|| pAE->Timer.Completed()
					|| pAE->Type->DiscardAfterHits > 0 && pAE->AttachOwnerAttackedCounter >= pAE->Type->DiscardAfterHits
					;
			}
		)
		, this->AttachEffects.end()
				);

	bool armorReplaced = false;
	bool armorReplaced_Shield = false;
	bool decloak = false;
	bool cloakable = SessionClass::IsSingleplayer() ? (TechnoExt::CanICloakByDefault(pThis) || this->Crate_Cloakable) : false;

	for (const auto& pAE : this->AttachEffects)
	{
		pAE->Update();

		if (!TechnoExt::IsReallyAlive(pThis))
			return;

		if (pAE->IsActive())
		{
			if (pAE->Type->ReplaceArmor.isset())
			{
				this->ReplacedArmorIdx = pAE->Type->ReplaceArmor.Get();
				armorReplaced = true;
			}

			if (pAE->Type->ReplaceArmor_Shield.isset() && this->Shield != nullptr)
			{
				this->Shield->ReplaceArmor(pAE->Type->ReplaceArmor_Shield.Get());
				armorReplaced_Shield = true;
			}

			cloakable |= pAE->Type->Cloak;
			decloak |= pAE->Type->Decloak;
		}
	}

	if (!TechnoExt::IsReallyAlive(pThis))
		return;

	this->ArmorReplaced = armorReplaced;

	if (Shield != nullptr)
		Shield->SetArmorReplaced(armorReplaced_Shield);

	if (SessionClass::IsSingleplayer())
		pThis->Cloakable = cloakable && !decloak;
}

void TechnoExt::ExtData::PassengerProduct()
{
	const auto pTypeExt = this->TypeExtData;

	if (pTypeExt->PassengerProduct)
	{
		--PassengerProduct_Timer;

		if (this->PassengerProduct_Timer <= 0 && !pTypeExt->PassengerProduct_Type.empty())
		{
			this->PassengerProduct_Timer = pTypeExt->PassengerProduct_Rate;
			TechnoClass* pThis = this->OwnerObject();
			const TechnoTypeClass* pType = pThis->GetTechnoType();

			if (pTypeExt->PassengerProduct_RandomPick)
			{
				std::vector<int> vIdx;
				int size = static_cast<int>(pTypeExt->PassengerProduct_Type.size());

				for (int i = 0; i < pTypeExt->PassengerProduct_Amount; i++)
				{
					vIdx.emplace_back(ScenarioClass::Instance->Random.RandomRanged(0, size - 1));
				}

				for (int idx : vIdx)
				{
					TechnoTypeClass* pPassengerType = pTypeExt->PassengerProduct_Type[idx];
					int passengerSize = pTypeExt->Passengers_BySize ? static_cast<int>(pPassengerType->Size) : 1;

					if (pThis->Passengers.GetTotalSize() + passengerSize > pType->Passengers)
						break;

					if (FootClass* pPassenger = abstract_cast<FootClass*>(pPassengerType->CreateObject(pThis->Owner)))
					{
						pThis->AddPassenger(pPassenger);
						pPassenger->Transporter = pThis;

						if (pType->OpenTopped)
							pThis->EnteredOpenTopped(pPassenger);
					}
				}
			}
			else
			{
				TechnoTypeClass* pPassengerType = pTypeExt->PassengerProduct_Type[0];
				int passengerSize = pTypeExt->Passengers_BySize ? static_cast<int>(pPassengerType->Size) : 1;

				for (int i = 0; i < pTypeExt->PassengerProduct_Amount; i++)
				{
					if (pThis->Passengers.GetTotalSize() + passengerSize > pType->Passengers)
						break;

					if (FootClass* pPassenger = abstract_cast<FootClass*>(pPassengerType->CreateObject(pThis->Owner)))
					{
						pThis->AddPassenger(pPassenger);
						pPassenger->Transporter = pThis;

						if (pType->OpenTopped)
							pThis->EnteredOpenTopped(pPassenger);
					}
				}
			}
		}
	}
}

int __fastcall TechnoExt::ExtData::GetArmorIdx(const WeaponTypeClass* pWeapon) const
{
	if (pWeapon == nullptr)
		return this->GetArmorIdxWithoutShield();

	return GetArmorIdx(pWeapon->Warhead);
}

int __fastcall TechnoExt::ExtData::GetArmorIdx(const WarheadTypeClass* pWH) const
{
	if (this == nullptr)
		return 0;

	if (auto pShield = this->Shield.get())
	{
		if (pShield->CanBePenetrated(pWH))
			return this->GetArmorIdxWithoutShield();

		if (pShield->IsActive())
			return pShield->GetArmorIndex();
	}

	return this->GetArmorIdxWithoutShield();
}

int TechnoExt::ExtData::GetArmorIdxWithoutShield() const
{
	return this->ArmorReplaced && !SessionClass::IsSingleplayer()
		? this->ReplacedArmorIdx
		: static_cast<int>(this->OwnerObject()->GetTechnoType()->Armor);
}

void TechnoExt::ExtData::CheckParachuted()
{
	if (this->NeedParachute_Height <= 0)
		return;

	const auto pThis = this->OwnerObject();
	auto coords = pThis->GetCoords();

	if (coords.Z - MapClass::Instance->GetCellFloorHeight(coords) - this->NeedParachute_Height > 0)
		return;

	coords.Z += 75;
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	const auto parachuteAnim = pTypeExt->Parachute_Anim.Get(
	HouseTypeExt::ExtMap.Find(pThis->Owner->Type)->Parachute_Anim.Get(RulesClass::Instance->Parachute));

	if (const auto parachute = GameCreate<AnimClass>(parachuteAnim, coords))
	{
		pThis->Parachute = parachute;
		parachute->SetOwnerObject(pThis);
		parachute->LightConvert = pThis->GetRemapColour();
		parachute->TintColor = pThis->GetCell()->Intensity_Normal;
	}

	this->NeedParachute_Height = 0;
}

void TechnoExt::ExtData::ShouldSinking()
{
	if (WasFallenDown)
	{
		const auto pThis = this->OwnerObject();
		auto pCell = pThis->GetCell();

		if (!pThis->IsInAir())
		{
			if (pCell->Tile_Is_Water() &&
				!pThis->IsOnBridge() &&
				!(pThis->GetTechnoType()->SpeedType == SpeedType::Hover ||
				pThis->GetTechnoType()->SpeedType == SpeedType::Winged ||
				pThis->GetTechnoType()->SpeedType == SpeedType::Float ||
				pThis->GetTechnoType()->SpeedType == SpeedType::Amphibious))
			{
				if (pCell->OverlayTypeIndex >= 0)
				{
					if (OverlayTypeClass::Array->GetItem(pCell->OverlayTypeIndex)->LandType != LandType::Water)
					{
						WasFallenDown = false;

						if (UnitFallWeapon)
						{
							auto location = pThis->IsOnBridge() ? pThis->GetCell()->GetCoordsWithBridge() : pThis->GetCell()->GetCoords();
							WeaponTypeExt::DetonateAt(UnitFallWeapon, location, pThis);
							UnitFallWeapon = nullptr;
						}

						if (UnitFallDestory)
						{
							auto location = pThis->IsOnBridge() ? pThis->GetCell()->GetCoordsWithBridge() : pThis->GetCell()->GetCoords();
							pThis->Limbo();
							pThis->IsFallingDown = false;
							WasFallenDown = false;
							CurrtenFallRate = 0;
							pThis->FallRate = CurrtenFallRate;
							pThis->Unlimbo(location, pThis->GetRealFacing().GetDir());
							KillSelf(pThis, AutoDeathBehavior::Kill);
						}

						return;
					}
				}

				if (pThis->WhatAmI() != AbstractType::Building)
				{
					if (pThis->WhatAmI() == AbstractType::Infantry)
					{
						InfantryOnWaterFix(pThis);
					}
					else
					{
						pThis->IsSinking = true;
					}
				}
			}
			else
			{
				WasFallenDown = false;

				if (UnitFallWeapon)
				{
					auto location = pThis->IsOnBridge() ? pThis->GetCell()->GetCoordsWithBridge() : pThis->GetCell()->GetCoords();
					WeaponTypeExt::DetonateAt(UnitFallWeapon, location, pThis);
					UnitFallWeapon = nullptr;
				}

				if (UnitFallDestory)
				{
					auto location = pThis->IsOnBridge() ? pThis->GetCell()->GetCoordsWithBridge() : pThis->GetCell()->GetCoords();
					pThis->Limbo();
					pThis->IsFallingDown = false;
					WasFallenDown = false;
					CurrtenFallRate = 0;
					pThis->FallRate = CurrtenFallRate;
					pThis->Unlimbo(location, pThis->GetRealFacing().GetDir());
					KillSelf(pThis, AutoDeathBehavior::Kill);
				}
			}
		}
		else
		{
			if (UnitFallDestory)
			{
				if (pThis->GetHeight() < UnitFallDestoryHeight)
				{
					if (UnitFallWeapon)
					{
						WeaponTypeExt::DetonateAt(UnitFallWeapon, pThis->Location, pThis);
						UnitFallWeapon = nullptr;
					}

					pThis->Limbo();
					pThis->IsFallingDown = false;
					WasFallenDown = false;
					CurrtenFallRate = 0;
					pThis->FallRate = CurrtenFallRate;
					pThis->Unlimbo(pThis->Location, pThis->GetRealFacing().GetDir());
					KillSelf(pThis, AutoDeathBehavior::Kill);
				}
			}
		}
	}
}

void TechnoExt::ExtData::AntiGravity()
{
	if (AntiGravityType == nullptr)
		return;

	const auto pThis = OwnerObject();
	auto const pWarheadExt = WarheadTypeExt::ExtMap.Find(AntiGravityType);
	pThis->UnmarkAllOccupationBits(pThis->GetCoords());

	if (WasOnAntiGravity)
	{
		if (!OnAntiGravity)
		{
			int FallRate = abs(pWarheadExt->AntiGravity_FallRate.Get(TypeExtData->FallRate_NoParachute));
			int FallRateMax = abs(pWarheadExt->AntiGravity_FallRateMax.Get(TypeExtData->FallRate_NoParachuteMax.Get(RulesClass::Instance->NoParachuteMaxFallRate)));

			if (CurrtenFallRate > -FallRateMax)
				CurrtenFallRate -= FallRate;
			else
				CurrtenFallRate = -FallRateMax;

			pThis->FallRate = CurrtenFallRate;
		}

		WasFallenDown = true;

		if (!pThis->IsFallingDown)
		{
			if (!pThis->IsInAir())
			{
				if (pWarheadExt->AntiGravity_Anim)
				{
					auto location = pThis->IsOnBridge() ? pThis->GetCell()->GetCoordsWithBridge() : pThis->GetCell()->GetCoords();
					auto pAnim = GameCreate<AnimClass>(pWarheadExt->AntiGravity_Anim, location);
					pAnim->Owner = pThis->Owner;
				}

				int damage = pWarheadExt->AntiGravity_FallDamage;
				int addon = CurrtenFallRate < 0 ? abs(CurrtenFallRate) : 0;
				damage += static_cast<int>(pWarheadExt->AntiGravity_FallDamage_Factor * addon);
				auto warhead = pWarheadExt->AntiGravity_FallDamage_Warhead.Get(RulesClass::Instance->C4Warhead);
				if (damage != 0)
					pThis->TakeDamage(damage, pThis->Owner, pThis, warhead);
			}

			WasOnAntiGravity = false;
			OnAntiGravity = false;
			AntiGravityType = nullptr;
			CurrtenFallRate = 0;
			return;
		}
	}

	if (OnAntiGravity)
	{
		if (pThis->FallRate >= 0)
		{
			if (pThis->GetHeight() < pWarheadExt->AntiGravity_Height)
			{
				if (!pThis->IsFallingDown)
					pThis->IsFallingDown = true;

				int RiseRate = abs(pWarheadExt->AntiGravity_RiseRate.Get(TypeExtData->FallRate_NoParachute));
				int RiseRateMax = abs(pWarheadExt->AntiGravity_RiseRateMax.Get(TypeExtData->FallRate_NoParachuteMax.Get(RulesClass::Instance->NoParachuteMaxFallRate)));

				if (CurrtenFallRate < RiseRateMax)
					CurrtenFallRate += RiseRate;
				else
					CurrtenFallRate = RiseRateMax;

				pThis->FallRate = CurrtenFallRate;
			}
			else
			{
				if (!pThis->IsFallingDown)
					pThis->IsFallingDown = true;

				int FallRate = abs(pWarheadExt->AntiGravity_FallRate.Get(TypeExtData->FallRate_NoParachute));
				int FallRateMax = abs(pWarheadExt->AntiGravity_FallRateMax.Get(TypeExtData->FallRate_NoParachuteMax.Get(RulesClass::Instance->NoParachuteMaxFallRate)));

				if (CurrtenFallRate > -FallRateMax)
					CurrtenFallRate -= FallRate;
				else
					CurrtenFallRate = -FallRateMax;

				pThis->FallRate = CurrtenFallRate;

				if (!pWarheadExt->AntiGravity_ConnectSW.empty())
				{
					std::vector<SuperClass*> SWlist;
					for (int swIdx : pWarheadExt->AntiGravity_ConnectSW)
					{
						SuperClass* pSuper = AntiGravityOwner->Supers[swIdx];
						SWTypeExt::ExtData* pSWTypeExt = SWTypeExt::ExtMap.Find(pSuper->Type);
						if (strcmp(pSWTypeExt->TypeID.data(), "UnitFall") == 0)
							SWlist.emplace_back(pSuper);
					}

					if (!SWlist.empty())
					{
						auto pHouseExt = HouseExt::ExtMap.Find(AntiGravityOwner);
						int addon = ScenarioClass::Instance->Random.RandomRanged(0, abs(pWarheadExt->AntiGravity_ConnectSW_DefermentRandomMax));

						pHouseExt->UnitFallTechnos.emplace_back(pThis);
						pHouseExt->UnitFallConnects.emplace_back(SWlist);
						pHouseExt->UnitFallAnims.emplace_back(pWarheadExt->AntiGravity_ConnectSW_Anim);
						pHouseExt->UnitFallDeferments.emplace_back(abs(pWarheadExt->AntiGravity_ConnectSW_Deferment) + addon);
						pHouseExt->UnitFallFacings.emplace_back(pWarheadExt->AntiGravity_ConnectSW_Facing);
						pHouseExt->UnitFallHeights.emplace_back(pWarheadExt->AntiGravity_ConnectSW_Height);
						pHouseExt->UnitFallMissions.emplace_back(pWarheadExt->AntiGravity_ConnectSW_Mission);
						pHouseExt->UnitFallOwners.emplace_back(pWarheadExt->AntiGravity_ConnectSW_Owner);
						pHouseExt->UnitFallRandomFacings.emplace_back(pWarheadExt->AntiGravity_ConnectSW_RandomFacing);
						pHouseExt->UnitFallUseParachutes.emplace_back(pWarheadExt->AntiGravity_ConnectSW_UseParachute);
						pHouseExt->UnitFallAlwaysFalls.emplace_back(pWarheadExt->AntiGravity_ConnectSW_AlwaysFall);
						pHouseExt->UnitFallCells.emplace_back(CellStruct::Empty);
						pHouseExt->UnitFallReallySWs.emplace_back(nullptr);
						pHouseExt->UnitFallTechnoOwners.emplace_back(pThis->Owner);

						UnitFallWeapon = pWarheadExt->AntiGravity_ConnectSW_Weapon;
						UnitFallDestory = pWarheadExt->AntiGravity_ConnectSW_Destory;
						UnitFallDestoryHeight = pWarheadExt->AntiGravity_ConnectSW_DestoryHeight;

						if (pWarheadExt->AntiGravity_Anim)
						{
							auto pAnim = GameCreate<AnimClass>(pWarheadExt->AntiGravity_Anim, pThis->Location);
							pAnim->Owner = pThis->Owner;
						}

						pThis->IsFallingDown = false;

						WasOnAntiGravity = false;
						OnAntiGravity = false;

						AntiGravityType = nullptr;
						CurrtenFallRate = 0;
						pThis->FallRate = CurrtenFallRate;

						pThis->Limbo();
						pThis->SetOwningHouse(HouseClass::FindCivilianSide(), false);

						return;
					}
				}

				if (pWarheadExt->AntiGravity_Destory)
				{
					if (pWarheadExt->AntiGravity_Anim)
					{
						auto pAnim = GameCreate<AnimClass>(pWarheadExt->AntiGravity_Anim, pThis->Location);
						pAnim->Owner = pThis->Owner;
					}

					pThis->IsFallingDown = false;

					KillSelf(pThis, AutoDeathBehavior::Vanish);
				}

				OnAntiGravity = false;
				WasOnAntiGravity = true;
			}
		}
		else
		{
			if (!pThis->IsFallingDown)
				pThis->IsFallingDown = true;

			int RiseRate = abs(pWarheadExt->AntiGravity_RiseRate.Get(TypeExtData->FallRate_NoParachute));
			int RiseRateMax = abs(pWarheadExt->AntiGravity_RiseRateMax.Get(TypeExtData->FallRate_NoParachuteMax.Get(RulesClass::Instance->NoParachuteMaxFallRate)));

			if (CurrtenFallRate < RiseRateMax)
				CurrtenFallRate += RiseRate;
			else
				CurrtenFallRate = RiseRateMax;

			pThis->FallRate = CurrtenFallRate;
		}
	}
}

void TechnoExt::ExtData::PlayLandAnim()
{
	const auto pThis = OwnerObject();
	if (pThis->InLimbo)
		return;

	if (Landed)
	{
		if (pThis->IsInAir())
			Landed = false;
	}
	else
	{
		if (!pThis->IsInAir())
		{
			Landed = true;
			if (TypeExtData->LandAnim)
			{
				auto location = pThis->IsOnBridge() ? pThis->GetCell()->GetCoordsWithBridge() : pThis->GetCell()->GetCoords();
				auto pAnim = GameCreate<AnimClass>(TypeExtData->LandAnim, location);
				pAnim->Owner = pThis->Owner;
			}
		}
	}
}
