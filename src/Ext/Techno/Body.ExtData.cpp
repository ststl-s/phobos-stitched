#include "Body.h"

#include <JumpjetLocomotionClass.h>

#include <Ext/Anim/Body.h>
#include <Ext/Abstract/AbstractExt.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/House/Body.h>
#include <Ext/HouseType/Body.h>
#include <Ext/SWType/Body.h>

#include <Misc/FlyingStrings.h>

#include <Utilities/EnumFunctions.h>
#include <Utilities/GeneralUtils.h>
#include <Utilities/Helpers.Alex.h>

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

void TechnoExt::ExtData::InvalidatePointer(void* ptr, bool removed)
{
	if (ptr == nullptr)
		return;

	for (auto& pAE : AttachEffects)
	{
		pAE->InvalidatePointer(ptr, removed);
	}

	if (ShareWeaponRangeTarget == ptr)
	{
		ShareWeaponRangeTarget = nullptr;
	}

	if (removed)
	{
		for (auto const& pAttachment : ChildAttachments)
			pAttachment->InvalidatePointer(ptr);

		if (ProcessingConvertsAnim == ptr)
		{
			ProcessingConvertsAnim = nullptr;
			Convert(this->OwnerObject(), this->ConvertsTargetType, this->Convert_DetachedBuildLimit);
		}

		if (PreFireAnim == ptr)
		{
			PreFireAnim = nullptr;
			PreFireFinish = false;
			OwnerObject()->DiskLaserTimer.Start(-1);
		}
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
	TechnoClass* pTechno = this->OwnerObject();

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

	if (pTypeExt->Convert_Deploy != nullptr)
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

void TechnoExt::ExtData::ShareWeaponRangeTurn()
{
	TechnoClass* pThis = OwnerObject();

	if (!TechnoExt::IsReallyAlive(pThis))
		return;

	if (ObjectClass* pObject = abstract_cast<ObjectClass*>(ShareWeaponRangeTarget))
	{
		if (!TechnoExt::IsReallyAlive(pObject))
			return;
	}
	else 
	{
		if (!AbstractExt::IsAbstract(ShareWeaponRangeTarget))
			return;
	}

	if (pThis->HasTurret())
	{
		if (pThis->TurretFacing().GetFacing<32>() != ShareWeaponRangeFacing.GetFacing<32>())
		{
			pThis->SecondaryFacing.SetDesired(ShareWeaponRangeFacing);
			pThis->PrimaryFacing.SetDesired(ShareWeaponRangeFacing);
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
					if (pBeamCannonWeapon->Anim.Count > 0)
					{
						if (pBeamCannonWeapon->Anim.Count >= 8)
						{
							auto anim = GameCreate<AnimClass>(pBeamCannonWeapon->Anim.GetItem(pThis->GetRealFacing().GetFacing<8>()), TechnoExt::GetFLHAbsoluteCoords(pThis, pWeaponExt->BeamCannon_FLH[i], pThis->HasTurret()));
							anim->SetOwnerObject(pThis);
						}
						else
						{
							auto anim = GameCreate<AnimClass>(pBeamCannonWeapon->Anim.GetItem(0), TechnoExt::GetFLHAbsoluteCoords(pThis, pWeaponExt->BeamCannon_FLH[i], pThis->HasTurret()));
							anim->SetOwnerObject(pThis);
						}
					}

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

	if (!IsActivePower(pThis))
		return;

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

	for (const auto& pAE : this->GetActiveAE())
	{
		if (pAE->Type->DisableWeapon_Category & DisableWeaponCate::Self)
			return;
	}

	for (size_t i = 0;
		TechnoExt::IsReallyAlive(pThis)
		&& i < vWeapons.size();
		i++)
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

void TechnoExt::ExtData::CheckJJConvertConditions()
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

void TechnoExt::ExtData::KeepGuard()
{
	TechnoClass* pThis = OwnerObject();
	auto const pTypeExt = TypeExtData;

	if (AttackWeapon && pThis->Target)
	{
		if (pThis->DistanceFrom(pThis->Target) > AttackWeapon->Range)
		{
			pThis->ForceMission(Mission::Stop);
			pThis->Guard();
		}
	}
}

void TechnoExt::ExtData::ForgetFirer()
{
	if (Attacker != nullptr
		&& Attacker_Weapon != nullptr)
	{
		bool AttackerCheck = WeaponTypeExt::ExtMap.Find(Attacker_Weapon)->OnlyAllowOneFirer_ResetImmediately ? (Attacker->IsAlive && (Attacker->GetCurrentMission() == Mission::Attack) && (Attacker->Target == OwnerObject())) : true;

		if (Attacker_Count > 0 && AttackerCheck)
		{
			Attacker_Count--;
		}
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
				{
					pThis->Deactivate();
					pThis->QueueMission(Mission::Stop, true);
				}
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
					if (!pThis->Owner->IsHumanPlayer)
						pThis->QueueMission(RulesExt::Global()->ReactivateAIRecoverMission, true);
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
			{
				pThis->Deactivate();
				pThis->QueueMission(Mission::Stop, true);
			}
		}
		else
		{
			if (pThis->Deactivated)
			{
				pThis->Reactivate();
				if (!pThis->Owner->IsHumanPlayer)
					pThis->QueueMission(RulesExt::Global()->ReactivateAIRecoverMission, true);
			}
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
	if (pThis->GetTechnoType()->Trainable)
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

		for (auto pTechnoType : *TechnoTypeClass::Array)
		{
			auto pTypeExt = TechnoTypeExt::ExtMap.Find(pTechnoType);
			if (!pTypeExt->IsExtendAcademy)
				continue;

			const auto& vTechnos = HouseExt::GetOwnedTechno(pThis->Owner, pTechnoType);
			for (size_t i = 0; i < vTechnos.size(); i++)
			{
				if (!vTechnos[i]->IsInPlayfield)
					continue;

				if (pTypeExt->Academy_Powered)
				{
					if (vTechnos[i]->WhatAmI() == AbstractType::Building)
					{
						if (!IsActivePower(vTechnos[i]))
							continue;
					}
					else
					{
						if (!IsActive(vTechnos[i]))
							continue;
					}
				}

				switch (pThis->WhatAmI())
				{
				case AbstractType::Infantry:
					if (!pTypeExt->Academy_InfantryVeterancy_Types.empty())
					{
						auto& vTypes = pTypeExt->Academy_InfantryVeterancy_Types;
						auto it = std::find(vTypes.begin(), vTypes.end(), pThis->GetTechnoType());

						if (it == vTypes.end())
							continue;
					}

					if (!pTypeExt->Academy_InfantryVeterancy_Ignore.empty())
					{
						auto& vTypes = pTypeExt->Academy_InfantryVeterancy_Ignore;
						auto it = std::find(vTypes.begin(), vTypes.end(), pThis->GetTechnoType());

						if (it != vTypes.end())
							continue;
					}

					if (pTypeExt->Academy_InfantryVeterancy_Cumulative)
						Veterancy += pTypeExt->Academy_InfantryVeterancy;
					else if (Veterancy < pTypeExt->Academy_InfantryVeterancy)
						Veterancy = pTypeExt->Academy_InfantryVeterancy;

					break;
				case AbstractType::Aircraft:
					if (!pTypeExt->Academy_AircraftVeterancy_Types.empty())
					{
						auto& vTypes = pTypeExt->Academy_AircraftVeterancy_Types;
						auto it = std::find(vTypes.begin(), vTypes.end(), pThis->GetTechnoType());

						if (it == vTypes.end())
							continue;
					}

					if (!pTypeExt->Academy_AircraftVeterancy_Ignore.empty())
					{
						auto& vTypes = pTypeExt->Academy_AircraftVeterancy_Ignore;
						auto it = std::find(vTypes.begin(), vTypes.end(), pThis->GetTechnoType());

						if (it != vTypes.end())
							continue;
					}

					if (pTypeExt->Academy_AircraftVeterancy_Cumulative)
						Veterancy += pTypeExt->Academy_AircraftVeterancy;
					else if (Veterancy < pTypeExt->Academy_AircraftVeterancy)
						Veterancy = pTypeExt->Academy_AircraftVeterancy;

					break;
				case AbstractType::Unit:
					if (pThis->GetTechnoType()->Organic)
					{
						if (!pTypeExt->Academy_InfantryVeterancy_Types.empty())
						{
							auto& vTypes = pTypeExt->Academy_InfantryVeterancy_Types;
							auto it = std::find(vTypes.begin(), vTypes.end(), pThis->GetTechnoType());

							if (it == vTypes.end())
								continue;
						}

						if (!pTypeExt->Academy_InfantryVeterancy_Ignore.empty())
						{
							auto& vTypes = pTypeExt->Academy_InfantryVeterancy_Ignore;
							auto it = std::find(vTypes.begin(), vTypes.end(), pThis->GetTechnoType());

							if (it != vTypes.end())
								continue;
						}

						if (pTypeExt->Academy_InfantryVeterancy_Cumulative)
							Veterancy += pTypeExt->Academy_InfantryVeterancy;
						else if (Veterancy < pTypeExt->Academy_InfantryVeterancy)
							Veterancy = pTypeExt->Academy_InfantryVeterancy;
					}
					else if (pThis->GetTechnoType()->ConsideredAircraft)
					{
						if (!pTypeExt->Academy_AircraftVeterancy_Types.empty())
						{
							auto& vTypes = pTypeExt->Academy_AircraftVeterancy_Types;
							auto it = std::find(vTypes.begin(), vTypes.end(), pThis->GetTechnoType());

							if (it == vTypes.end())
								continue;
						}

						if (!pTypeExt->Academy_AircraftVeterancy_Ignore.empty())
						{
							auto& vTypes = pTypeExt->Academy_AircraftVeterancy_Ignore;
							auto it = std::find(vTypes.begin(), vTypes.end(), pThis->GetTechnoType());

							if (it != vTypes.end())
								continue;
						}

						if (pTypeExt->Academy_AircraftVeterancy_Cumulative)
							Veterancy += pTypeExt->Academy_AircraftVeterancy;
						else if (Veterancy < pTypeExt->Academy_AircraftVeterancy)
							Veterancy = pTypeExt->Academy_AircraftVeterancy;
					}
					else if (pThis->GetTechnoType()->Naval)
					{
						if (!pTypeExt->Academy_NavalVeterancy_Types.empty())
						{
							auto& vTypes = pTypeExt->Academy_NavalVeterancy_Types;
							auto it = std::find(vTypes.begin(), vTypes.end(), pThis->GetTechnoType());

							if (it == vTypes.end())
								continue;
						}

						if (!pTypeExt->Academy_NavalVeterancy_Ignore.empty())
						{
							auto& vTypes = pTypeExt->Academy_NavalVeterancy_Ignore;
							auto it = std::find(vTypes.begin(), vTypes.end(), pThis->GetTechnoType());

							if (it != vTypes.end())
								continue;
						}

						if (pTypeExt->Academy_NavalVeterancy_Cumulative)
							Veterancy += pTypeExt->Academy_NavalVeterancy;
						else if (Veterancy < pTypeExt->Academy_NavalVeterancy)
							Veterancy = pTypeExt->Academy_NavalVeterancy;
					}
					else
					{
						if (!pTypeExt->Academy_VehicleVeterancy_Types.empty())
						{
							auto& vTypes = pTypeExt->Academy_VehicleVeterancy_Types;
							auto it = std::find(vTypes.begin(), vTypes.end(), pThis->GetTechnoType());

							if (it == vTypes.end())
								continue;
						}

						if (!pTypeExt->Academy_VehicleVeterancy_Ignore.empty())
						{
							auto& vTypes = pTypeExt->Academy_VehicleVeterancy_Ignore;
							auto it = std::find(vTypes.begin(), vTypes.end(), pThis->GetTechnoType());

							if (it != vTypes.end())
								continue;
						}

						if (pTypeExt->Academy_VehicleVeterancy_Cumulative)
							Veterancy += pTypeExt->Academy_VehicleVeterancy;
						else if (Veterancy < pTypeExt->Academy_VehicleVeterancy)
							Veterancy = pTypeExt->Academy_VehicleVeterancy;
					}
					break;
				case AbstractType::Building:
					if (!pTypeExt->Academy_BuildingVeterancy_Types.empty())
					{
						auto& vTypes = pTypeExt->Academy_BuildingVeterancy_Types;
						auto it = std::find(vTypes.begin(), vTypes.end(), pThis->GetTechnoType());

						if (it == vTypes.end())
							continue;
					}

					if (!pTypeExt->Academy_BuildingVeterancy_Ignore.empty())
					{
						auto& vTypes = pTypeExt->Academy_BuildingVeterancy_Ignore;
						auto it = std::find(vTypes.begin(), vTypes.end(), pThis->GetTechnoType());

						if (it != vTypes.end())
							continue;
					}

					if (pTypeExt->Academy_BuildingVeterancy_Cumulative)
						Veterancy += pTypeExt->Academy_BuildingVeterancy;
					else if (Veterancy < pTypeExt->Academy_BuildingVeterancy)
						Veterancy = pTypeExt->Academy_BuildingVeterancy;

					break;
				default:
					break;
				}
			}
		}

		if (Veterancy > 0)
		{
			VeterancyStruct* vstruct = &pThis->Veterancy;
			if (Veterancy >= 2)
				vstruct->SetElite();
			else
			{
				vstruct->Add(-Veterancy);
				if (vstruct->IsNegative())
				{
					vstruct->Reset();
					vstruct->Add(Veterancy);
				}
				else
					vstruct->Add(Veterancy);
			}
		}
	}
}

void TechnoExt::ExtData::TechnoAcademyReset()
{
	if (AcademyReset)
		return;

	TechnoClass* pThis = OwnerObject();
	const auto pTypeExt = this->TypeExtData;
	if (pTypeExt->IsExtendAcademy && pTypeExt->Academy_Immediately)
	{
		if (pThis->IsInPlayfield)
			return;

		if (pTypeExt->Academy_Powered)
		{
			if (pThis->WhatAmI() == AbstractType::Building)
			{
				if (!IsActivePower(pThis))
					return;
			}
			else
			{
				if (!IsActive(pThis))
					return;
			}
		}

		for (auto pTechnoType : *TechnoTypeClass::Array)
		{
			if (!pTechnoType->Trainable)
				continue;

			const auto& vTechnos = HouseExt::GetOwnedTechno(pThis->Owner, pTechnoType);
			for (size_t i = 0; i < vTechnos.size(); i++)
			{
				double Veterancy = 0;
				switch (vTechnos[i]->WhatAmI())
				{
				case AbstractType::Infantry:
					if (!pTypeExt->Academy_InfantryVeterancy_Types.empty())
					{
						auto& vTypes = pTypeExt->Academy_InfantryVeterancy_Types;
						auto it = std::find(vTypes.begin(), vTypes.end(), pThis->GetTechnoType());

						if (it == vTypes.end())
							continue;
					}

					if (!pTypeExt->Academy_InfantryVeterancy_Ignore.empty())
					{
						auto& vTypes = pTypeExt->Academy_InfantryVeterancy_Ignore;
						auto it = std::find(vTypes.begin(), vTypes.end(), pThis->GetTechnoType());

						if (it != vTypes.end())
							continue;
					}

					Veterancy = pTypeExt->Academy_InfantryVeterancy;

					break;
				case AbstractType::Aircraft:
					if (!pTypeExt->Academy_AircraftVeterancy_Types.empty())
					{
						auto& vTypes = pTypeExt->Academy_AircraftVeterancy_Types;
						auto it = std::find(vTypes.begin(), vTypes.end(), pThis->GetTechnoType());

						if (it == vTypes.end())
							continue;
					}

					if (!pTypeExt->Academy_AircraftVeterancy_Ignore.empty())
					{
						auto& vTypes = pTypeExt->Academy_AircraftVeterancy_Ignore;
						auto it = std::find(vTypes.begin(), vTypes.end(), pThis->GetTechnoType());

						if (it != vTypes.end())
							continue;
					}

					Veterancy = pTypeExt->Academy_AircraftVeterancy;

					break;
				case AbstractType::Unit:
					if (pThis->GetTechnoType()->Organic)
					{
						if (!pTypeExt->Academy_InfantryVeterancy_Types.empty())
						{
							auto& vTypes = pTypeExt->Academy_InfantryVeterancy_Types;
							auto it = std::find(vTypes.begin(), vTypes.end(), pThis->GetTechnoType());

							if (it == vTypes.end())
								continue;
						}

						if (!pTypeExt->Academy_InfantryVeterancy_Ignore.empty())
						{
							auto& vTypes = pTypeExt->Academy_InfantryVeterancy_Ignore;
							auto it = std::find(vTypes.begin(), vTypes.end(), pThis->GetTechnoType());

							if (it != vTypes.end())
								continue;
						}

						Veterancy = pTypeExt->Academy_InfantryVeterancy;
					}
					else if (pThis->GetTechnoType()->ConsideredAircraft)
					{
						if (!pTypeExt->Academy_AircraftVeterancy_Types.empty())
						{
							auto& vTypes = pTypeExt->Academy_AircraftVeterancy_Types;
							auto it = std::find(vTypes.begin(), vTypes.end(), pThis->GetTechnoType());

							if (it == vTypes.end())
								continue;
						}

						if (!pTypeExt->Academy_AircraftVeterancy_Ignore.empty())
						{
							auto& vTypes = pTypeExt->Academy_AircraftVeterancy_Ignore;
							auto it = std::find(vTypes.begin(), vTypes.end(), pThis->GetTechnoType());

							if (it != vTypes.end())
								continue;
						}

						Veterancy = pTypeExt->Academy_AircraftVeterancy;
					}
					else if (pThis->GetTechnoType()->Naval)
					{
						if (!pTypeExt->Academy_NavalVeterancy_Types.empty())
						{
							auto& vTypes = pTypeExt->Academy_NavalVeterancy_Types;
							auto it = std::find(vTypes.begin(), vTypes.end(), pThis->GetTechnoType());

							if (it == vTypes.end())
								continue;
						}

						if (!pTypeExt->Academy_NavalVeterancy_Ignore.empty())
						{
							auto& vTypes = pTypeExt->Academy_NavalVeterancy_Ignore;
							auto it = std::find(vTypes.begin(), vTypes.end(), pThis->GetTechnoType());

							if (it != vTypes.end())
								continue;
						}

						Veterancy = pTypeExt->Academy_NavalVeterancy;
					}
					else
					{
						if (!pTypeExt->Academy_VehicleVeterancy_Types.empty())
						{
							auto& vTypes = pTypeExt->Academy_VehicleVeterancy_Types;
							auto it = std::find(vTypes.begin(), vTypes.end(), pThis->GetTechnoType());

							if (it == vTypes.end())
								continue;
						}

						if (!pTypeExt->Academy_VehicleVeterancy_Ignore.empty())
						{
							auto& vTypes = pTypeExt->Academy_VehicleVeterancy_Ignore;
							auto it = std::find(vTypes.begin(), vTypes.end(), pThis->GetTechnoType());

							if (it != vTypes.end())
								continue;
						}

						Veterancy = pTypeExt->Academy_VehicleVeterancy;
					}
					break;
				case AbstractType::Building:
					if (!pTypeExt->Academy_BuildingVeterancy_Types.empty())
					{
						auto& vTypes = pTypeExt->Academy_BuildingVeterancy_Types;
						auto it = std::find(vTypes.begin(), vTypes.end(), pThis->GetTechnoType());

						if (it == vTypes.end())
							continue;
					}

					if (!pTypeExt->Academy_BuildingVeterancy_Ignore.empty())
					{
						auto& vTypes = pTypeExt->Academy_BuildingVeterancy_Ignore;
						auto it = std::find(vTypes.begin(), vTypes.end(), pThis->GetTechnoType());

						if (it != vTypes.end())
							continue;
					}

					Veterancy = pTypeExt->Academy_BuildingVeterancy;

					break;
				default:
					break;
				}

				if (pTypeExt->Academy_Immediately_Addition)
				{
					VeterancyStruct* vstruct = &vTechnos[i]->Veterancy;
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
						VeterancyStruct* vstruct = &vTechnos[i]->Veterancy;
						if (Veterancy >= 2)
							vstruct->SetElite();
						else
						{
							vstruct->Add(-Veterancy);
							if (vstruct->IsNegative())
							{
								vstruct->Reset();
								vstruct->Add(Veterancy);
							}
							else
								vstruct->Add(Veterancy);
						}
					}
				}
			}
		}
	}
	AcademyReset = true;
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

void TechnoExt::ExtData::MoveChangeLocomotor()
{
	TechnoClass* pThis = OwnerObject();
	auto const pTypeExt = TypeExtData;
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

void TechnoExt::ExtData::TemporalTeamCheck()
{
	TechnoClass* pThis = OwnerObject();

	if (TemporalTarget->IsReallyAlive())
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

void TechnoExt::ExtData::SetSyncDeathOwner()
{
	TechnoClass* pThis = OwnerObject();

	for (int i = 0; i < pThis->CaptureManager->ControlNodes.Count; i++)
	{
		auto pTechnoExt = TechnoExt::ExtMap.Find(pThis->CaptureManager->ControlNodes[i]->Techno);
		if (pTechnoExt->SyncDeathOwner != pThis)
			pTechnoExt->SyncDeathOwner = pThis;
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
			(pTypeExt->MobileRefinery_TransRate > 0 && Unsorted::CurrentFrame % pTypeExt->MobileRefinery_TransRate) ||
			!TechnoExt::IsActive(pThis))
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
				auto bonus = (pThis->Owner->NumOrePurifiers * RulesClass::Instance->PurifierBonus) + 1.0;
				int value = static_cast<int>(amount * tibValue * pTypeExt->MobileRefinery_CashMultiplier * bonus);
				int addition = HouseExt::CheckOrePurifier(pThis->Owner, value);
				value += addition;
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
	return this->ArmorReplaced && SessionClass::IsSingleplayer()
		? this->ReplacedArmorIdx
		: static_cast<int>(this->OwnerObject()->GetTechnoType()->Armor);
}

void TechnoExt::ExtData::CheckParachuted()
{
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
					return InfantryOnWaterFix(pThis);
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

bool TechnoExt::ExtData::IsDeployed()
{
	const auto pThis = this->OwnerObject();

	if (pThis->WhatAmI() == AbstractType::Infantry)
	{
		if (auto pInf = abstract_cast<InfantryClass*>(pThis))
		{
			if (pInf->IsDeployed())
				return true;
		}
	}
	else
	{
		if (pThis->CurrentMission == Mission::Unload)
			return true;
	}

	return false;
}

void TechnoExt::ExtData::BackwarpUpdate()
{
	const auto pThis = this->OwnerObject();

	if (BackwarpLocation == CoordStruct::Empty)
	{
		BackwarpLocation = pThis->GetCoords();
		BackwarpHealth = pThis->Health;
		BackwarpTimer.Start(TypeExtData->Backwarp_Delay);
		BackwarpColdDown.Start(TypeExtData->Backwarp_ChargeTime);
	}
	else
	{
		if (BackwarpWarpOutTimer.InProgress())
		{
			if (BackwarpTimer.InProgress() && BackwarpColdDown.InProgress())
			{
				BackwarpTimer.Pause();
				BackwarpColdDown.Pause();

				if (!pThis->WarpingOut)
					pThis->WarpingOut = true;
			}
		}
		else
		{
			if (BackwarpTimer.Completed())
			{
				BackwarpLocation = pThis->GetCoords();
				BackwarpHealth = pThis->Health;
				BackwarpTimer.Start(TypeExtData->Backwarp_Delay);
			}

			if (BackwarpTimer.Expired() && BackwarpColdDown.Expired())
			{
				BackwarpTimer.Resume();
				BackwarpColdDown.Resume();

				if (pThis->WarpingOut)
					pThis->WarpingOut = false;
			}
		}
	}
}

void TechnoExt::ExtData::Backwarp()
{
	const auto pThis = this->OwnerObject();

	if (pThis->GetCurrentMission() == Mission::Unload)
	{
		if (BackwarpColdDown.Completed())
		{
			AnimClass* WarpOut = GameCreate<AnimClass>(TypeExtData->Backwarp_WarpOutAnim.Get(RulesClass::Instance()->WarpOut), pThis->GetCoords());
			WarpOut->Owner = pThis->Owner;

			VocClass::PlayAt(TypeExtData->Backwarp_WarpOutSound.Get(RulesClass::Instance->ChronoOutSound), pThis->GetCoords());

			if (TypeExtData->Backwarp_Health)
				pThis->Health = BackwarpHealth;

			if (pThis->GetCoords() != BackwarpLocation)
			{
				bool selected = pThis->IsSelected;

				const auto pType = TypeExtData->OwnerObject();

				CellClass* pCell = nullptr;
				CellStruct nCell;

				bool allowBridges = pType->SpeedType != SpeedType::Float;
				nCell = MapClass::Instance->NearByLocation(CellClass::Coord2Cell(BackwarpLocation),
					pType->SpeedType, -1, pType->MovementZone, false, 1, 1, true,
					false, false, allowBridges, CellStruct::Empty, false, false);
				pCell = MapClass::Instance->TryGetCellAt(nCell);

				if (pCell != nullptr)
					BackwarpLocation = pCell->GetCoordsWithBridge();
				else
					BackwarpLocation.Z = MapClass::Instance->GetCellFloorHeight(BackwarpLocation);

				FootClass* pFoot = abstract_cast<FootClass*>(pThis);
				CellStruct cellDest = CellClass::Coord2Cell(BackwarpLocation);
				pThis->Limbo();
				ILocomotion* pLoco = pFoot->Locomotor.release();
				pFoot->Locomotor.reset(LocomotionClass::CreateInstance(pType->Locomotor).release());
				pFoot->Locomotor->Link_To_Object(pFoot);
				pLoco->Release();
				++Unsorted::IKnowWhatImDoing;
				pThis->Unlimbo(BackwarpLocation, pThis->PrimaryFacing.Current().GetDir());
				--Unsorted::IKnowWhatImDoing;

				if (pThis->IsInAir())
					TechnoExt::FallenDown(pThis);

				if (selected)
					pThis->Select();
			}

			AnimClass* WarpIn = GameCreate<AnimClass>(TypeExtData->Backwarp_WarpInAnim.Get(RulesClass::Instance()->WarpIn), pThis->GetCoords());
			WarpIn->Owner = pThis->Owner;

			VocClass::PlayAt(TypeExtData->Backwarp_WarpInSound.Get(RulesClass::Instance->ChronoInSound), pThis->GetCoords());

			if (TypeExtData->Backwarp_WarpOutTime > 0)
			{
				pThis->WarpingOut = true;
				BackwarpWarpOutTimer.Start(TypeExtData->Backwarp_WarpOutTime);
			}

			BackwarpTimer.Start(TypeExtData->Backwarp_Delay);
			BackwarpColdDown.Start(TypeExtData->Backwarp_ChargeTime);
		}
	}

	if (auto pUnit = abstract_cast<UnitClass*>(pThis))
	{
		if (pUnit->Deployed)
			pUnit->Undeploy();
	}
}

void TechnoExt::ExtData::UpdateStrafingLaser()
{
	const auto pThis = this->OwnerObject();

	auto createLaser = [pThis](std::unique_ptr<StrafingLaserClass>& pStrafingLaser, CoordStruct target, int duration, bool fades)
	{
		CoordStruct source = TechnoExt::GetFLHAbsoluteCoords(pThis, pStrafingLaser->FLH, pThis->GetTechnoType()->Turret);
		auto innerColor = pStrafingLaser->Type->IsHouseColor.Get() ? pThis->Owner->Color : pStrafingLaser->Type->InnerColor.Get();
		auto outercolor = pStrafingLaser->Type->OuterColor.Get();
		auto outerspread = pStrafingLaser->Type->OuterSpread.Get();

		LaserDrawClass* pLaser = GameCreate<LaserDrawClass>(
			source,
			target,
			innerColor,
			outercolor,
			outerspread,
			duration);

		pLaser->Fades = fades;
		pLaser->IsHouseColor = (pStrafingLaser->Type->IsHouseColor.Get() || pStrafingLaser->Type->IsSingleColor.Get()) ? true : false;
		pLaser->IsSupported = pStrafingLaser->Type->IsSupported.Get(pStrafingLaser->Type->Thickness.Get() > 3) ? true : false;
		pLaser->Thickness = pStrafingLaser->Type->Thickness.Get();

		return pLaser;
	};

	auto deleteStrafingLaser =
		[this](std::unique_ptr<StrafingLaserClass>& pStrafingLaser)
		{
			auto it = std::find(this->StrafingLasers.cbegin(), this->StrafingLasers.cend(), pStrafingLaser);
			if (it != this->StrafingLasers.end())
			{
				pStrafingLaser = nullptr;
				this->StrafingLasers.erase(it);
			}
		};

	if (!TechnoExt::IsReallyAlive(pThis))
	{
		for (auto& pStrafingLaser : this->StrafingLasers)
			deleteStrafingLaser(pStrafingLaser);
	}

	for (auto& pStrafingLaser : this->StrafingLasers)
	{
		if (!pStrafingLaser)
		{
			deleteStrafingLaser(pStrafingLaser);
			continue;
		}

		int timer = pStrafingLaser->Type->Timer.Get();
		int x = static_cast<int>((pStrafingLaser->TargetFLH.X - pStrafingLaser->SourceFLH.X) / double(timer));
		int y = static_cast<int>((pStrafingLaser->TargetFLH.Y - pStrafingLaser->SourceFLH.Y) / double(timer));
		int z = static_cast<int>((pStrafingLaser->TargetFLH.Z - pStrafingLaser->SourceFLH.Z) / double(timer));
		int timeLeft = Unsorted::CurrentFrame - pStrafingLaser->CurrentFrame;

		CoordStruct coord
		{
			pStrafingLaser->SourceFLH.X + x * timeLeft,
			pStrafingLaser->SourceFLH.Y + y * timeLeft,
			pStrafingLaser->SourceFLH.Z + z * timeLeft

		};

		if (pStrafingLaser->InGround)
		{
			const auto nCell = CellClass::Coord2Cell(coord);
			const auto pCell = MapClass::Instance->TryGetCellAt(nCell);
			if (pCell)
			{
				coord.Z = pCell->GetCoordsWithBridge().Z;
			}
		}

		if (!TechnoExt::IsActive(pThis))
		{
			createLaser(pStrafingLaser, coord, pStrafingLaser->Type->Duration.Get(), true);
			deleteStrafingLaser(pStrafingLaser);
			continue;
		}

		if (pThis->Target != pStrafingLaser->Target)
		{
			createLaser(pStrafingLaser, coord, pStrafingLaser->Type->Duration.Get(), true);
			deleteStrafingLaser(pStrafingLaser);
			continue;
		}

		if (timeLeft > timer)
		{
			if (const auto pWeapon = pStrafingLaser->Type->DetonateWeapon.Get())
			{
				int damage = TechnoExt::GetCurrentDamage(pWeapon->Damage, abstract_cast<FootClass*>(pThis));
				WeaponTypeExt::DetonateAt(pWeapon, pStrafingLaser->TargetFLH, pThis, damage);
			}

			auto pLaser = createLaser(pStrafingLaser, pStrafingLaser->TargetFLH, pStrafingLaser->Type->Duration.Get(), true);
			pLaser->Thickness = pStrafingLaser->Type->EndThickness.Get(pLaser->Thickness);

			deleteStrafingLaser(pStrafingLaser);
			continue;
		}
		else
		{
			createLaser(pStrafingLaser, coord, 1, false);

			if (timeLeft % pStrafingLaser->Type->Weapon_Delay.Get() == 0)
			{
				if (const auto pWeapon = pStrafingLaser->Type->Weapon.Get())
				{
					int damage = TechnoExt::GetCurrentDamage(pWeapon->Damage, abstract_cast<FootClass*>(pThis));
					WeaponTypeExt::DetonateAt(pWeapon, coord, pThis, damage);
				}
			}
		}
	}
}

void TechnoExt::ExtData::SetNeedConvert(TechnoTypeClass* pTargetType, bool detachedBuildLimit, AnimTypeClass* pAnimType)
{
	this->ConvertsTargetType = pTargetType;
	this->Convert_DetachedBuildLimit = detachedBuildLimit;

	if (pAnimType != nullptr)
	{
		AnimClass* pAnim = GameCreate<AnimClass>(pAnimType, this->OwnerObject()->GetCoords());
		this->ProcessingConvertsAnim = pAnim;
	}
	else
	{
		Convert(this->OwnerObject(), pTargetType, detachedBuildLimit);
	}
}

void TechnoExt::ExtData::ApplySpawnsTiberium()
{
	TechnoClass* pThis = OwnerObject();

	if (pThis->WhatAmI() == AbstractType::Building)
	{
		if (!TechnoExt::IsActivePower(pThis))
			return;
	}
	else
	{
		if (!TechnoExt::IsActive(pThis))
			return;
	}

	const auto pTypeExt = this->TypeExtData;

	if (!pTypeExt->TiberiumSpawner ||
		(pTypeExt->TiberiumSpawner_SpawnRate > 0 && Unsorted::CurrentFrame % pTypeExt->TiberiumSpawner_SpawnRate) ||
		pTypeExt->TiberiumSpawner_Types.empty())
		return;

	CellStruct cell;

	if (pThis->WhatAmI() == AbstractType::Building)
	{
		cell = CellClass::Coord2Cell(pThis->GetCenterCoords());
	}
	else
	{
		auto const pFoot = abstract_cast<FootClass*>(pThis);
		if (locomotion_cast<JumpjetLocomotionClass*>(pFoot->Locomotor))
			cell = pFoot->CurrentJumpjetMapCoords;
		else
			cell = pFoot->CurrentMapCoords;
	}

	std::vector<CellClass*> Cells;

	if (pTypeExt->TiberiumSpawner_Range.Get().Y)
	{
		for (int x = -abs(pTypeExt->TiberiumSpawner_Range.Get().X); x <= abs(pTypeExt->TiberiumSpawner_Range.Get().X); x++)
		{
			for (int y = -abs(pTypeExt->TiberiumSpawner_Range.Get().Y); y <= abs(pTypeExt->TiberiumSpawner_Range.Get().Y); y++)
			{

				CoordStruct flh = { 0,0,0 };
				flh.X = x * Unsorted::LeptonsPerCell;
				flh.Y = y * Unsorted::LeptonsPerCell;

				CoordStruct coords = TechnoExt::GetFLHAbsoluteCoords(pThis, flh, false);
				coords.Z = CellClass::Cell2Coord(cell).Z;

				Cells.push_back(MapClass::Instance->TryGetCellAt(coords));
			}
		}
	}
	else
	{
		for (CellSpreadEnumerator it(pTypeExt->TiberiumSpawner_Range.Get().X); it; ++it)
		{
			auto const pCell = MapClass::Instance->GetCellAt(*it + cell);
			Cells.push_back(pCell);
		}
	}

	if (Cells.empty())
		return;

	if (pTypeExt->TiberiumSpawner_AffectAllCell)
	{
		for (size_t i = 0; i < Cells.size(); i++)
		{
			auto const pCell = Cells[i];
			CoordStruct pos = pCell->GetCenterCoords();

			if (!pCell)
				continue;

			size_t Chooseidx = ScenarioClass::Instance->Random.RandomRanged(0, pTypeExt->TiberiumSpawner_Types.size() - 1);
			int Tiberiumidx = pTypeExt->TiberiumSpawner_Types[Chooseidx];
			int tibValue = TiberiumClass::Array->GetItem(Tiberiumidx)->Value;

			if (pCell->CanTiberiumGerminate(TiberiumClass::Array->GetItem(Tiberiumidx)) || pCell->GetContainedTiberiumIndex() == Tiberiumidx)
			{
				if (pCell->GetContainedTiberiumIndex() == Tiberiumidx && TiberiumClass::Array->GetItem(Tiberiumidx)->GrowthPercentage <= 0)
					continue;

				int value = pTypeExt->TiberiumSpawner_Values.size() > Chooseidx ?
					pTypeExt->TiberiumSpawner_Values[Chooseidx] :
					tibValue * 3;

				int maxValue = pTypeExt->TiberiumSpawner_MaxValues.size() > Chooseidx ?
					pTypeExt->TiberiumSpawner_MaxValues[Chooseidx] :
					tibValue * 12;

				int tValue = pCell->GetContainedTiberiumValue();
				if (tValue >= tibValue * 12)
					continue;

				if (tValue + value > maxValue)
					value = maxValue - tValue;

				if (value <= 0)
					continue;
				else if (value > tibValue * 12)
					value = tibValue * 12;

				int tAmount = static_cast<int>(value * 1.0 / tibValue);

				if (tAmount >= 12)
				{
					pCell->IncreaseTiberium(Tiberiumidx, 11);
					pCell->IncreaseTiberium(Tiberiumidx, tAmount - 11);
				}
				else
					pCell->IncreaseTiberium(Tiberiumidx, tAmount);

				if (!pTypeExt->TiberiumSpawner_SpawnAnims.empty())
				{
					AnimTypeClass* pAnimType = nullptr;
					if (pTypeExt->TiberiumSpawner_SpawnAnim_RandomPick)
					{
						pAnimType = pTypeExt->TiberiumSpawner_SpawnAnims
							[ScenarioClass::Instance->Random.RandomRanged(0, pTypeExt->TiberiumSpawner_SpawnAnims.size() - 1)];
					}
					else
						pAnimType = pTypeExt->TiberiumSpawner_SpawnAnims.size() > Chooseidx ?
						pTypeExt->TiberiumSpawner_SpawnAnims[Chooseidx] :
						pTypeExt->TiberiumSpawner_SpawnAnims[0];

					if (pAnimType)
					{
						if (auto pAnim = GameCreate<AnimClass>(pAnimType, pos))
							pAnim->Owner = pThis->Owner;
					}
				}
			}
		}

		if (!pTypeExt->TiberiumSpawner_Anims.empty())
		{
			AnimTypeClass* pAnimType = nullptr;
			int facing = pThis->PrimaryFacing.Current().GetValue<3>();

			if (facing >= 7)
				facing = 0;
			else
				facing++;

			switch (pTypeExt->TiberiumSpawner_Anims.size())
			{
			case 1:
				pAnimType = pTypeExt->TiberiumSpawner_Anims[0];
				break;
			case 8:
				pAnimType = pTypeExt->TiberiumSpawner_Anims[facing];
				break;
			default:
				pAnimType = pTypeExt->TiberiumSpawner_Anims
					[ScenarioClass::Instance->Random.RandomRanged(0, pTypeExt->TiberiumSpawner_Anims.size() - 1)];
				break;
			}

			if (pAnimType)
			{
				if (auto pAnim = GameCreate<AnimClass>(pAnimType, pThis->Location))
				{
					pAnim->Owner = pThis->Owner;

					if (pTypeExt->TiberiumSpawner_AnimMove)
						pAnim->SetOwnerObject(pThis);
				}
			}
		}
	}
	else
	{
		size_t Chooseidx = ScenarioClass::Instance->Random.RandomRanged(0, pTypeExt->TiberiumSpawner_Types.size() - 1);
		int Tiberiumidx = pTypeExt->TiberiumSpawner_Types[Chooseidx];
		int tibValue = TiberiumClass::Array->GetItem(Tiberiumidx)->Value;

		std::vector<CellClass*> AllowCells;
		for (size_t i = 0; i < Cells.size(); i++)
		{
			auto const pAllowCell = Cells[i];

			if (!pAllowCell ||
				!(pAllowCell->CanTiberiumGerminate(TiberiumClass::Array->GetItem(Tiberiumidx)) || pAllowCell->GetContainedTiberiumIndex() == Tiberiumidx) ||
				pAllowCell->GetContainedTiberiumValue() >= tibValue * 12 ||
				(pAllowCell->GetContainedTiberiumIndex() == Tiberiumidx && TiberiumClass::Array->GetItem(Tiberiumidx)->GrowthPercentage <= 0))
				continue;

			AllowCells.push_back(pAllowCell);
		}

		if (AllowCells.empty())
			return;

		CellClass* pCell = AllowCells[ScenarioClass::Instance->Random.RandomRanged(0, AllowCells.size() - 1)];
		CoordStruct pos = pCell->GetCenterCoords();

		int value = pTypeExt->TiberiumSpawner_Values.size() > Chooseidx ?
			pTypeExt->TiberiumSpawner_Values[Chooseidx] :
			tibValue * 3;

		int maxValue = pTypeExt->TiberiumSpawner_MaxValues.size() > Chooseidx ?
			pTypeExt->TiberiumSpawner_MaxValues[Chooseidx] :
			tibValue * 12;

		int tValue = pCell->GetContainedTiberiumValue();
		if (tValue + value > maxValue)
			value = maxValue - tValue;

		if (value <= 0)
			return;
		else if (value > tibValue * 12)
			value = tibValue * 12;

		int tAmount = static_cast<int>(value * 1.0 / tibValue);

		if (tAmount >= 12)
		{
			pCell->IncreaseTiberium(Tiberiumidx, 11);
			pCell->IncreaseTiberium(Tiberiumidx, tAmount - 11);
		}
		else
			pCell->IncreaseTiberium(Tiberiumidx, tAmount);

		if (!pTypeExt->TiberiumSpawner_Anims.empty())
		{
			AnimTypeClass* pAnimType = nullptr;
			int facing = pThis->PrimaryFacing.Current().GetValue<3>();

			if (facing >= 7)
				facing = 0;
			else
				facing++;

			switch (pTypeExt->TiberiumSpawner_Anims.size())
			{
			case 1:
				pAnimType = pTypeExt->TiberiumSpawner_Anims[0];
				break;
			case 8:
				pAnimType = pTypeExt->TiberiumSpawner_Anims[facing];
				break;
			default:
				pAnimType = pTypeExt->TiberiumSpawner_Anims
					[ScenarioClass::Instance->Random.RandomRanged(0, pTypeExt->TiberiumSpawner_Anims.size() - 1)];
				break;
			}

			if (pAnimType)
			{
				if (auto pAnim = GameCreate<AnimClass>(pAnimType, pThis->Location))
				{
					pAnim->Owner = pThis->Owner;

					if (pTypeExt->TiberiumSpawner_AnimMove)
						pAnim->SetOwnerObject(pThis);
				}
			}
		}

		if (!pTypeExt->TiberiumSpawner_SpawnAnims.empty())
		{
			AnimTypeClass* pAnimType = nullptr;
			if (pTypeExt->TiberiumSpawner_SpawnAnim_RandomPick)
			{
				pAnimType = pTypeExt->TiberiumSpawner_SpawnAnims
					[ScenarioClass::Instance->Random.RandomRanged(0, pTypeExt->TiberiumSpawner_SpawnAnims.size() - 1)];
			}
			else
				pAnimType = pTypeExt->TiberiumSpawner_SpawnAnims.size() > Chooseidx ?
				pTypeExt->TiberiumSpawner_SpawnAnims[Chooseidx] :
				pTypeExt->TiberiumSpawner_SpawnAnims[0];

			if (pAnimType)
			{
				if (auto pAnim = GameCreate<AnimClass>(pAnimType, pos))
					pAnim->Owner = pThis->Owner;
			}
		}
	}
}

void TechnoExt::ExtData::CheckRopeConnection()
{
	const auto pThis = this->OwnerObject();
	const auto pExt = this;

	if (pThis->IsFallingDown)
	{
		if (TechnoExt::IsReallyAlive(pExt->RopeConnection_Vehicle))
		{
			auto unitcoord = pExt->RopeConnection_Vehicle->GetCoords();
			auto coord = pThis->GetCoords();

			GameCreate<LaserDrawClass>
				(
					unitcoord,
					coord,
					ColorStruct { 60,60,60, },
					ColorStruct { 0,0,0, },
					ColorStruct { 0,0,0, },
					2
				);

			unitcoord.X += 1;
			coord.X += 1;

			GameCreate<LaserDrawClass>
				(
					unitcoord,
					coord,
					ColorStruct { 10,10,10, },
					ColorStruct { 0,0,0, },
					ColorStruct { 0,0,0, },
					2
				);
		}
	}
	else
	{
		pThis->OnBridge = pThis->GetCell()->ContainsBridge();
		pExt->RopeConnection = false;
		pExt->RopeConnection_Vehicle = nullptr;
		pExt->CheckRopeConnection_Alive();
	}
}

void TechnoExt::ExtData::CheckRopeConnection_Alive()
{
	const auto pThis = this->OwnerObject();
	const auto pType = pThis->GetTechnoType();
	const auto pCell = pThis->GetCell();
	const auto pSpeedType = pType->SpeedType;

	if (!pCell->Tile_Is_Water())
	{
		if (pSpeedType == SpeedType::FloatBeach || pSpeedType == SpeedType::Float)
		{
			TechnoExt::KillSelf(pThis, AutoDeathBehavior::Kill);
		}

		if (pCell->Passability)
		{
			TechnoExt::KillSelf(pThis, AutoDeathBehavior::Kill);
		}
	}
	else
	{
		if (!pCell->ContainsBridge())
		{
			if (pSpeedType != SpeedType::Amphibious && pSpeedType != SpeedType::FloatBeach && pSpeedType != SpeedType::Float &&
				pSpeedType != SpeedType::Hover && pSpeedType != SpeedType::Winged)
			{
				if (pType->WhatAmI() == AbstractType::InfantryType)
				{
					VocClass::PlayAt(pType->DieSound.GetItem(0), pThis->GetCoords());
				}

				int count = RulesClass::Instance->SplashList.Count - 1;
				int seed = ScenarioClass::Instance->Random.RandomRanged(0, count);

				if (const auto pAnim = GameCreate<AnimClass>(RulesClass::Instance->SplashList.GetItem(seed), pThis->GetCoords()))
				{
					pAnim->Owner = pThis->Owner;
					if (const auto pAnimExt = AnimExt::ExtMap.Find(pAnim))
					{
						pAnimExt->Invoker = pThis;
					}
				}

				TechnoExt::KillSelf(pThis, AutoDeathBehavior::Vanish);
			}
		}
		else
		{
			if (pSpeedType == SpeedType::FloatBeach || pSpeedType == SpeedType::Float)
			{
				TechnoExt::KillSelf(pThis, AutoDeathBehavior::Vanish);
			}
		}
	}
}

void TechnoExt::ExtData::AttachmentsAirFix()
{
	if (this->ParentInAir == OwnerObject()->IsInAir())
		return;

	for (auto const& pAttachment : this->ChildAttachments)
	{
		if (!pAttachment->Child)
			continue;

		bool selected = pAttachment->Child->IsSelected;
		pAttachment->Limbo();
		pAttachment->Unlimbo();
		// if (pAttachment->Child->WhatAmI() != AbstractType::Building)
			// ChangeLocomotorTo(pAttachment->Child, LocomotionClass::CLSIDs::Hover.get());
		//pAttachment->Child->InAir = pThis->IsInAir();
		if (selected)
		{
			pAttachment->Child->Select();
		}
	}

	this->ParentInAir = OwnerObject()->IsInAir();
}

void TechnoExt::ExtData::CheckPassenger()
{
	TechnoClass* pTechno = OwnerObject();
	const TechnoTypeClass* PassType = pTechno->Passengers.GetFirstPassenger()->GetTechnoType();
	const auto pTypeExt = this->TypeExtData;

	if (std::find(this->Convert_Passengers.cbegin(), this->Convert_Passengers.cend(), PassType) == this->Convert_Passengers.cend())
		return;

	Nullable<TechnoTypeClass*> ChangeType;

	for (size_t i = 0; i < pTypeExt->Convert_Passengers.size(); i++)
	{
		TechnoTypeClass* Passenger = this->Convert_Passengers[i];

		if (Passenger == PassType)
		{
			ChangeType = this->Convert_Types[i];
			break;
		}
	}

	if (!ChangeType)
		ChangeType = this->Convert_Types[0];

	if (!ChangeType)
		return;

	TechnoExt::UnitConvert(pTechno, ChangeType, pTechno->Passengers.GetFirstPassenger());
}

void TechnoExt::ExtData::SelectSW()
{
	TechnoClass* pTechno = OwnerObject();
	const auto pHouse = pTechno->Owner;
	const auto pTypeExt = this->TypeExtData;

	if (!pHouse->IsCurrentPlayer())
		return;

	if (Phobos::ToSelectSW)
	{
		const auto idxSW = pTypeExt->SuperWeapon_Quick[this->SWIdx];
		auto pSW = pHouse->Supers.GetItem(idxSW);

		if (pSW)
		{
			MapClass::UnselectAll();
			pSW->SetReadiness(true);
			Unsorted::CurrentSWType = idxSW;
		}

		this->SWIdx++;

		if (this->SWIdx > pTypeExt->SuperWeapon_Quick.size() - 1)
			this->SWIdx = 0;

		Phobos::ToSelectSW = false;
	}

	if (Unsorted::CurrentSWType == -1)
	{
		this->SWIdx = 0;
	}
}
