#include "BuildingExt.h"

#include <Helpers/Macro.h>

#include <Ext/House/Body.h>
#include <Ext/HouseType/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/SWType/Body.h>

#include <Misc/FlyingStrings.h>

#include <Utilities/EnumFunctions.h>
#include <Utilities/TemplateDef.h>

BuildingExt::ExtContainer BuildingExt::ExtMap;

void BuildingExt::ExtData::DisplayRefund()
{
	const auto pThis = this->OwnerObject();

	if (this->AccumulatedIncome &&
		Unsorted::CurrentFrame % 15 == 0 &&
		(RulesExt::Global()->DisplayIncome_AllowAI ||
		pThis->Owner->IsControlledByHuman()))
	{
		FlyingStrings::AddMoneyString(
			this->AccumulatedIncome,
			this->OwnerObject()->Owner,
			this->TypeExtData->DisplayIncome_Houses.Get(RulesExt::Global()->DisplayIncome_Houses.Get()),
			this->OwnerObject()->GetRenderCoords(),
			this->TypeExtData->DisplayIncome_Offset
		);

		this->AccumulatedIncome = 0;
	}
}

bool BuildingExt::ExtData::HasSuperWeapon(const int index, const bool withUpgrades) const
{
	const auto pThis = this->OwnerObject();
	const auto pExt = BuildingTypeExt::ExtMap.Find(pThis->Type);

	const auto count = pExt->GetSuperWeaponCount();
	for (auto i = 0; i < count; ++i)
	{
		const auto idxSW = pExt->GetSuperWeaponIndex(i, pThis->Owner);

		if (idxSW == index)
			return true;
	}

	if (withUpgrades)
	{
		for (auto const& pUpgrade : pThis->Upgrades)
		{
			if (const auto pUpgradeExt = BuildingTypeExt::ExtMap.Find(pUpgrade))
			{
				const auto countUpgrade = pUpgradeExt->GetSuperWeaponCount();
				for (auto i = 0; i < countUpgrade; ++i)
				{
					const auto idxSW = pUpgradeExt->GetSuperWeaponIndex(i, pThis->Owner);

					if (idxSW == index)
						return true;
				}
			}
		}
	}

	return false;
}

void BuildingExt::StoreTiberium(BuildingClass* pThis, float amount, int idxTiberiumType, int idxStorageTiberiumType)
{
	auto const pDepositableTiberium = TiberiumClass::Array->GetItem(idxStorageTiberiumType);
	float depositableTiberiumAmount = 0.0f; // Number of 'bails' that will be stored.
	auto const pTiberium = TiberiumClass::Array->GetItem(idxTiberiumType);

	if (amount > 0.0)
	{
		if (auto pBuildingType = pThis->Type)
		{
			if (auto const pExt = BuildingTypeExt::ExtMap.Find(pBuildingType))
			{
				if (pExt->Refinery_UseStorage)
				{
					// Store Tiberium in structures
					depositableTiberiumAmount = (amount * pTiberium->Value) / pDepositableTiberium->Value;
					pThis->Owner->GiveTiberium(depositableTiberiumAmount, idxStorageTiberiumType);
				}
			}
		}
	}
}

void BuildingExt::UpdatePrimaryFactoryAI(BuildingClass* pThis)
{
	auto pOwner = pThis->Owner;

	if (!pOwner || pOwner->ProducingAircraftTypeIndex < 0)
		return;

	auto BuildingExt = BuildingExt::ExtMap.Find(pThis);
	if (!BuildingExt)
		return;

	AircraftTypeClass* pAircraft = AircraftTypeClass::Array->GetItem(pOwner->ProducingAircraftTypeIndex);
	FactoryClass* currFactory = pOwner->GetFactoryProducing(pAircraft);
	DynamicVectorClass<BuildingClass*> airFactoryBuilding;
	BuildingClass* newBuilding = nullptr;

	// Update what is the current air factory for future comparisons
	if (BuildingExt->CurrentAirFactory)
	{
		int nDocks = 0;
		if (BuildingExt->CurrentAirFactory->Type)
			nDocks = BuildingExt->CurrentAirFactory->Type->NumberOfDocks;

		int nOccupiedDocks = CountOccupiedDocks(BuildingExt->CurrentAirFactory);

		if (nOccupiedDocks < nDocks)
			currFactory = BuildingExt->CurrentAirFactory->Factory;
		else
			BuildingExt->CurrentAirFactory = nullptr;
	}

	// Obtain a list of air factories for optimizing the comparisons
	for (auto pBuilding : pOwner->Buildings)
	{
		if (pBuilding->Type->Factory == AbstractType::AircraftType)
		{
			if (!currFactory && pBuilding->Factory)
				currFactory = pBuilding->Factory;

			airFactoryBuilding.AddItem(pBuilding);
		}
	}

	if (BuildingExt->CurrentAirFactory)
	{
		for (auto pBuilding : airFactoryBuilding)
		{
			if (pBuilding == BuildingExt->CurrentAirFactory)
			{
				BuildingExt->CurrentAirFactory->Factory = currFactory;
				BuildingExt->CurrentAirFactory->IsPrimaryFactory = true;
			}
			else
			{
				pBuilding->IsPrimaryFactory = false;

				if (pBuilding->Factory)
					pBuilding->Factory->AbandonProduction();
			}
		}

		return;
	}

	if (!currFactory)
		return;

	for (auto pBuilding : airFactoryBuilding)
	{
		int nDocks = pBuilding->Type->NumberOfDocks;
		int nOccupiedDocks = CountOccupiedDocks(pBuilding);

		if (nOccupiedDocks < nDocks)
		{
			if (!newBuilding)
			{
				newBuilding = pBuilding;
				newBuilding->Factory = currFactory;
				newBuilding->IsPrimaryFactory = true;
				BuildingExt->CurrentAirFactory = newBuilding;

				continue;
			}
		}

		pBuilding->IsPrimaryFactory = false;

		if (pBuilding->Factory)
			pBuilding->Factory->AbandonProduction();
	}

	return;
}

int BuildingExt::CountOccupiedDocks(BuildingClass* pBuilding)
{
	if (!pBuilding)
		return 0;

	int nOccupiedDocks = 0;

	if (pBuilding->RadioLinks.IsAllocated)
	{
		for (auto i = 0; i < pBuilding->RadioLinks.Capacity; ++i)
		{
			if (auto const pLink = pBuilding->GetNthLink(i))
				nOccupiedDocks++;
		}
	}

	return nOccupiedDocks;
}

bool BuildingExt::HasFreeDocks(BuildingClass* pBuilding)
{
	if (!pBuilding)
		return false;

	if (pBuilding->Type->Factory == AbstractType::AircraftType)
	{
		int nDocks = pBuilding->Type->NumberOfDocks;
		int nOccupiedDocks = BuildingExt::CountOccupiedDocks(pBuilding);

		if (nOccupiedDocks < nDocks)
			return true;
		else
			return false;
	}

	return false;
}

bool BuildingExt::CanGrindTechno(BuildingClass* pBuilding, TechnoClass* pTechno)
{
	if (!pBuilding->Type->Grinding || (pTechno->WhatAmI() != AbstractType::Infantry && pTechno->WhatAmI() != AbstractType::Unit))
		return false;

	if ((pBuilding->Type->InfantryAbsorb || pBuilding->Type->UnitAbsorb) &&
		(pTechno->WhatAmI() == AbstractType::Infantry && !pBuilding->Type->InfantryAbsorb ||
			pTechno->WhatAmI() == AbstractType::Unit && !pBuilding->Type->UnitAbsorb))
	{
		return false;
	}

	if (const auto pExt = BuildingTypeExt::ExtMap.Find(pBuilding->Type))
	{
		if (pBuilding->Owner == pTechno->Owner && !pExt->Grinding_AllowOwner)
			return false;

		if (pBuilding->Owner != pTechno->Owner && pBuilding->Owner->IsAlliedWith(pTechno) && !pExt->Grinding_AllowAllies)
			return false;

		if (pExt->Grinding_AllowTypes.size() > 0 && !pExt->Grinding_AllowTypes.Contains(pTechno->GetTechnoType()))
			return false;

		if (pExt->Grinding_DisallowTypes.size() > 0 && pExt->Grinding_DisallowTypes.Contains(pTechno->GetTechnoType()))
			return false;
	}

	return true;
}

bool BuildingExt::DoGrindingExtras(BuildingClass* pBuilding, TechnoClass* pTechno, int refund)
{
	if (const auto pExt = BuildingExt::ExtMap.Find(pBuilding))
	{
		const auto pTypeExt = pExt->TypeExtData;

		if (pTypeExt->DisplayIncome.Get(RulesExt::Global()->DisplayIncome.Get()) && refund)
		{
			pExt->AccumulatedIncome += refund;
		}

		if (pTypeExt->Grinding_Weapon.isset()
			&& Unsorted::CurrentFrame >= pExt->GrindingWeapon_LastFiredFrame + pTypeExt->Grinding_Weapon.Get()->ROF)
		{
			TechnoExt::FireWeaponAtSelf(pBuilding, pTypeExt->Grinding_Weapon.Get());
			pExt->GrindingWeapon_LastFiredFrame = Unsorted::CurrentFrame;
		}

		if (pTypeExt->Grinding_Sound.isset())
		{
			VocClass::PlayAt(pTypeExt->Grinding_Sound.Get(), pTechno->GetCoords());
			return true;
		}
	}

	return false;
}

bool __fastcall BuildingExt::HasSWType(BuildingClass* pThis, int swIdx)
{
	if (!TechnoExt::IsActivePower(pThis))
		return false;

	if (pThis->HasSuperWeapon(swIdx))
		return true;

	const auto pTypeExt = BuildingTypeExt::ExtMap.Find(pThis->Type);

	if (pTypeExt->SuperWeapons.Contains(swIdx))
		return true;

	for (auto pUPType : pThis->Upgrades)
	{
		if (pUPType == nullptr)
			continue;

		const auto pUPTypeExt = BuildingTypeExt::ExtMap.Find(pUPType);

		if (pUPTypeExt->SuperWeapons.Contains(swIdx))
			return true;
	}

	return false;
}

// Building only or allow units too?
void BuildingExt::ExtData::ApplyPoweredKillSpawns()
{
	auto const pThis = this->OwnerObject();

	if (this->TypeExtData->Powered_KillSpawns && pThis->Type->Powered && !pThis->IsPowerOnline())
	{
		if (auto pManager = pThis->SpawnManager)
		{
			pManager->ResetTarget();
			for (auto pItem : pManager->SpawnedNodes)
			{
				if (pItem->Status == SpawnNodeStatus::Attacking || pItem->Status == SpawnNodeStatus::Returning)
				{
					pItem->Techno->TakeDamage(pItem->Techno->Health);
				}
			}
		}
	}
}

bool BuildingExt::HandleInfiltrate(BuildingClass* pBuilding, HouseClass* pInfiltratorHouse)
{
	BuildingTypeExt::ExtData* pTypeExt = BuildingTypeExt::ExtMap.Find(pBuilding->Type);
	BuildingExt::ExtData* pExt = BuildingExt::ExtMap.Find(pBuilding);

	if (!pTypeExt->SpyEffect_Custom)
		return false;

	auto pVictimHouse = pBuilding->Owner;
	HouseExt::ExtData* pVictimExt = HouseExt::ExtMap.Find(pVictimHouse);
	HouseExt::ExtData* pInfiltratorExt = HouseExt::ExtMap.Find(pInfiltratorHouse);
	if (pInfiltratorHouse != pVictimHouse)
	{
		// I assume you were not launching for real, Morton

		auto launchTheSWHere = [pBuilding](SuperClass* const pSuper, HouseClass* const pHouse)
		{
			int oldstart = pSuper->RechargeTimer.StartTime;
			int oldleft = pSuper->RechargeTimer.TimeLeft;
			pSuper->SetReadiness(true);
			pSuper->Launch(CellClass::Coord2Cell(pBuilding->Location), pHouse->IsCurrentPlayer());
			pSuper->Reset();
			pSuper->RechargeTimer.StartTime = oldstart;
			pSuper->RechargeTimer.TimeLeft = oldleft;
		};

		if (pTypeExt->SpyEffect_VictimSuperWeapon.isset())
		{
			if (const auto pSuper = pVictimHouse->Supers.GetItem(pTypeExt->SpyEffect_VictimSuperWeapon.Get()))
				launchTheSWHere(pSuper, pVictimHouse);
		}

		if (pTypeExt->SpyEffect_InfiltratorSuperWeapon.isset())
		{
			if (const auto pSuper = pInfiltratorHouse->Supers.GetItem(pTypeExt->SpyEffect_InfiltratorSuperWeapon.Get()))
				launchTheSWHere(pSuper, pInfiltratorHouse);
		}

		if (pTypeExt->SpyEffect_StolenMoneyAmount != 0 || pTypeExt->SpyEffect_StolenMoneyPercentage != 0)
		{
			int money = 0;
			if (pTypeExt->SpyEffect_StolenMoneyAmount != 0 && pTypeExt->SpyEffect_StolenMoneyPercentage != 0)
			{
				if ((pTypeExt->SpyEffect_StolenMoneyAmount * pTypeExt->SpyEffect_StolenMoneyPercentage < 0) ||
					(pTypeExt->SpyEffect_StolenMoneyAmount < 0 && pTypeExt->SpyEffect_StolenMoneyPercentage < 0))
				{
					money = -static_cast<int>(abs(pInfiltratorHouse->Available_Money() * pTypeExt->SpyEffect_StolenMoneyPercentage));
					if (abs(money) > abs(pTypeExt->SpyEffect_StolenMoneyAmount))
						money = -abs(pTypeExt->SpyEffect_StolenMoneyAmount);

					if (abs(money) > pInfiltratorHouse->Available_Money())
						money = -pInfiltratorHouse->Available_Money();

					pInfiltratorHouse->TransactMoney(money);
					pVictimHouse->TransactMoney(-money);
				}
				else
				{
					money = static_cast<int>(pVictimHouse->Available_Money() * pTypeExt->SpyEffect_StolenMoneyPercentage);
					if (money > pTypeExt->SpyEffect_StolenMoneyAmount)
						money = pTypeExt->SpyEffect_StolenMoneyAmount;

					if (money > pVictimHouse->Available_Money())
						money = pVictimHouse->Available_Money();
				}
			}
			else if (pTypeExt->SpyEffect_StolenMoneyAmount != 0)
			{
				money = pTypeExt->SpyEffect_StolenMoneyAmount;

				if (pTypeExt->SpyEffect_StolenMoneyAmount < 0)
				{
					if (abs(money) > pInfiltratorHouse->Available_Money())
						money = -pInfiltratorHouse->Available_Money();

					pInfiltratorHouse->TransactMoney(money);
					pVictimHouse->TransactMoney(-money);
				}
				else
				{
					if (money > pVictimHouse->Available_Money())
						money = pVictimHouse->Available_Money();
				}
			}
			else if (pTypeExt->SpyEffect_StolenMoneyPercentage != 0)
			{
				if (pTypeExt->SpyEffect_StolenMoneyPercentage < 0)
				{
					money = static_cast<int>(pInfiltratorHouse->Available_Money() * pTypeExt->SpyEffect_StolenMoneyPercentage);
					pInfiltratorHouse->TransactMoney(money);
					pVictimHouse->TransactMoney(-money);
				}
				else
					money = static_cast<int>(pVictimHouse->Available_Money() * pTypeExt->SpyEffect_StolenMoneyPercentage);
			}

			if (pTypeExt->SpyEffect_StolenMoneyDisplay && money != 0)
				FlyingStrings::AddMoneyString(-money, pVictimHouse, pTypeExt->SpyEffect_StolenMoneyDisplay_Houses, pBuilding->GetCenterCoords(), pTypeExt->SpyEffect_StolenMoneyDisplay_Offset);
		}

		if (pTypeExt->SpyEffect_BuildingOfflineDuration > 0)
			pExt->OfflineTimer = pTypeExt->SpyEffect_BuildingOfflineDuration;

		if (pTypeExt->SpyEffect_InfantryDeactivate_Duration > 0)
		{
			pVictimExt->DeactivateInfantry_Duration.emplace_back(pTypeExt->SpyEffect_InfantryDeactivate_Duration);
			pVictimExt->DeactivateInfantry_Types.emplace_back(pTypeExt->SpyEffect_InfantryDeactivate_Types);
			pVictimExt->DeactivateInfantry_Ignore.emplace_back(pTypeExt->SpyEffect_InfantryDeactivate_Ignore);
		}

		if (pTypeExt->SpyEffect_VehicleDeactivate_Duration > 0)
		{
			pVictimExt->DeactivateVehicle_Duration.emplace_back(pTypeExt->SpyEffect_VehicleDeactivate_Duration);
			pVictimExt->DeactivateVehicle_Types.emplace_back(pTypeExt->SpyEffect_VehicleDeactivate_Types);
			pVictimExt->DeactivateVehicle_Ignore.emplace_back(pTypeExt->SpyEffect_VehicleDeactivate_Ignore);
		}

		if (pTypeExt->SpyEffect_NavalDeactivate_Duration > 0)
		{
			pVictimExt->DeactivateNaval_Duration.emplace_back(pTypeExt->SpyEffect_NavalDeactivate_Duration);
			pVictimExt->DeactivateNaval_Types.emplace_back(pTypeExt->SpyEffect_NavalDeactivate_Types);
			pVictimExt->DeactivateNaval_Ignore.emplace_back(pTypeExt->SpyEffect_NavalDeactivate_Ignore);
		}

		if (pTypeExt->SpyEffect_AircraftDeactivate_Duration > 0)
		{
			pVictimExt->DeactivateAircraft_Duration.emplace_back(pTypeExt->SpyEffect_AircraftDeactivate_Duration);
			pVictimExt->DeactivateAircraft_Types.emplace_back(pTypeExt->SpyEffect_AircraftDeactivate_Types);
			pVictimExt->DeactivateAircraft_Ignore.emplace_back(pTypeExt->SpyEffect_AircraftDeactivate_Ignore);
		}

		if (pTypeExt->SpyEffect_BuildingDeactivate_Duration > 0)
		{
			pVictimExt->DeactivateBuilding_Duration.emplace_back(pTypeExt->SpyEffect_BuildingDeactivate_Duration);
			pVictimExt->DeactivateBuilding_Types.emplace_back(pTypeExt->SpyEffect_BuildingDeactivate_Types);
			pVictimExt->DeactivateBuilding_Ignore.emplace_back(pTypeExt->SpyEffect_BuildingDeactivate_Ignore);
		}

		if (pTypeExt->SpyEffect_DefenseDeactivate_Duration > 0)
		{
			pVictimExt->DeactivateDefense_Duration.emplace_back(pTypeExt->SpyEffect_DefenseDeactivate_Duration);
			pVictimExt->DeactivateDefense_Types.emplace_back(pTypeExt->SpyEffect_DefenseDeactivate_Types);
			pVictimExt->DeactivateDefense_Ignore.emplace_back(pTypeExt->SpyEffect_DefenseDeactivate_Ignore);
		}

		if (pTypeExt->SpyEffect_InfantryVeterancy_Reset)
		{
			if (pTypeExt->SpyEffect_Veterancy_AffectBuildingOwner)
				HouseExt::TechnoUpgrade(pVictimHouse, 0,
					pTypeExt->SpyEffect_InfantryVeterancy_Types, pTypeExt->SpyEffect_InfantryVeterancy_Ignore,
					AffectedTechnoType::Infantry, pTypeExt->SpyEffect_InfantryVeterancy_Cumulative);
			else
				HouseExt::TechnoUpgrade(pInfiltratorHouse, 0,
					pTypeExt->SpyEffect_InfantryVeterancy_Types, pTypeExt->SpyEffect_InfantryVeterancy_Ignore,
					AffectedTechnoType::Infantry, pTypeExt->SpyEffect_InfantryVeterancy_Cumulative);
		}
		else if (pTypeExt->SpyEffect_InfantryVeterancy != 0)
		{
			if (pTypeExt->SpyEffect_Veterancy_AffectBuildingOwner)
				HouseExt::TechnoUpgrade(pVictimHouse, pTypeExt->SpyEffect_InfantryVeterancy,
					pTypeExt->SpyEffect_InfantryVeterancy_Types, pTypeExt->SpyEffect_InfantryVeterancy_Ignore,
					AffectedTechnoType::Infantry, pTypeExt->SpyEffect_InfantryVeterancy_Cumulative);
			else
				HouseExt::TechnoUpgrade(pInfiltratorHouse, pTypeExt->SpyEffect_InfantryVeterancy,
					pTypeExt->SpyEffect_InfantryVeterancy_Types, pTypeExt->SpyEffect_InfantryVeterancy_Ignore,
					AffectedTechnoType::Infantry, pTypeExt->SpyEffect_InfantryVeterancy_Cumulative);
		}

		if (pTypeExt->SpyEffect_VehicleVeterancy_Reset)
		{
			if (pTypeExt->SpyEffect_Veterancy_AffectBuildingOwner)
				HouseExt::TechnoUpgrade(pVictimHouse, 0,
					pTypeExt->SpyEffect_VehicleVeterancy_Types, pTypeExt->SpyEffect_VehicleVeterancy_Ignore,
					AffectedTechnoType::Units, pTypeExt->SpyEffect_VehicleVeterancy_Cumulative);
			else
				HouseExt::TechnoUpgrade(pInfiltratorHouse,0,
					pTypeExt->SpyEffect_VehicleVeterancy_Types, pTypeExt->SpyEffect_VehicleVeterancy_Ignore,
					AffectedTechnoType::Units, pTypeExt->SpyEffect_VehicleVeterancy_Cumulative);
		}
		else if (pTypeExt->SpyEffect_VehicleVeterancy != 0)
		{
			if (pTypeExt->SpyEffect_Veterancy_AffectBuildingOwner)
				HouseExt::TechnoUpgrade(pVictimHouse, pTypeExt->SpyEffect_VehicleVeterancy,
					pTypeExt->SpyEffect_VehicleVeterancy_Types, pTypeExt->SpyEffect_VehicleVeterancy_Ignore,
					AffectedTechnoType::Units, pTypeExt->SpyEffect_VehicleVeterancy_Cumulative);
			else
				HouseExt::TechnoUpgrade(pInfiltratorHouse, pTypeExt->SpyEffect_VehicleVeterancy,
					pTypeExt->SpyEffect_VehicleVeterancy_Types, pTypeExt->SpyEffect_VehicleVeterancy_Ignore,
					AffectedTechnoType::Units, pTypeExt->SpyEffect_VehicleVeterancy_Cumulative);
		}

		if (pTypeExt->SpyEffect_NavalVeterancy_Reset)
		{
			if (pTypeExt->SpyEffect_Veterancy_AffectBuildingOwner)
				HouseExt::TechnoUpgrade(pVictimHouse, 0,
					pTypeExt->SpyEffect_NavalVeterancy_Types, pTypeExt->SpyEffect_NavalVeterancy_Ignore,
					AffectedTechnoType::Naval, pTypeExt->SpyEffect_InfantryVeterancy_Cumulative);
			else
				HouseExt::TechnoUpgrade(pInfiltratorHouse, 0,
					pTypeExt->SpyEffect_NavalVeterancy_Types, pTypeExt->SpyEffect_NavalVeterancy_Ignore,
					AffectedTechnoType::Naval, pTypeExt->SpyEffect_InfantryVeterancy_Cumulative);
		}
		else if (pTypeExt->SpyEffect_NavalVeterancy != 0)
		{
			if (pTypeExt->SpyEffect_Veterancy_AffectBuildingOwner)
				HouseExt::TechnoUpgrade(pVictimHouse, pTypeExt->SpyEffect_NavalVeterancy,
					pTypeExt->SpyEffect_NavalVeterancy_Types, pTypeExt->SpyEffect_NavalVeterancy_Ignore,
					AffectedTechnoType::Naval, pTypeExt->SpyEffect_InfantryVeterancy_Cumulative);
			else
				HouseExt::TechnoUpgrade(pInfiltratorHouse, pTypeExt->SpyEffect_NavalVeterancy,
					pTypeExt->SpyEffect_NavalVeterancy_Types, pTypeExt->SpyEffect_NavalVeterancy_Ignore,
					AffectedTechnoType::Naval, pTypeExt->SpyEffect_InfantryVeterancy_Cumulative);
		}

		if (pTypeExt->SpyEffect_AircraftVeterancy_Reset)
		{
			if (pTypeExt->SpyEffect_Veterancy_AffectBuildingOwner)
				HouseExt::TechnoUpgrade(pVictimHouse, 0,
					pTypeExt->SpyEffect_AircraftVeterancy_Types, pTypeExt->SpyEffect_AircraftVeterancy_Ignore,
					AffectedTechnoType::Aircraft, pTypeExt->SpyEffect_AircraftVeterancy_Cumulative);
			else
				HouseExt::TechnoUpgrade(pInfiltratorHouse, 0,
					pTypeExt->SpyEffect_AircraftVeterancy_Types, pTypeExt->SpyEffect_AircraftVeterancy_Ignore,
					AffectedTechnoType::Aircraft, pTypeExt->SpyEffect_AircraftVeterancy_Cumulative);
		}
		else if (pTypeExt->SpyEffect_AircraftVeterancy != 0)
		{
			if (pTypeExt->SpyEffect_Veterancy_AffectBuildingOwner)
				HouseExt::TechnoUpgrade(pVictimHouse, pTypeExt->SpyEffect_AircraftVeterancy,
					pTypeExt->SpyEffect_AircraftVeterancy_Types, pTypeExt->SpyEffect_AircraftVeterancy_Ignore,
					AffectedTechnoType::Aircraft, pTypeExt->SpyEffect_AircraftVeterancy_Cumulative);
			else
				HouseExt::TechnoUpgrade(pInfiltratorHouse, pTypeExt->SpyEffect_AircraftVeterancy,
					pTypeExt->SpyEffect_AircraftVeterancy_Types, pTypeExt->SpyEffect_AircraftVeterancy_Ignore,
					AffectedTechnoType::Aircraft, pTypeExt->SpyEffect_AircraftVeterancy_Cumulative);
		}

		if (pTypeExt->SpyEffect_BuildingVeterancy_Reset)
		{
			if (pTypeExt->SpyEffect_Veterancy_AffectBuildingOwner)
				HouseExt::TechnoUpgrade(pVictimHouse, 0,
					pTypeExt->SpyEffect_BuildingVeterancy_Types, pTypeExt->SpyEffect_BuildingVeterancy_Ignore,
					AffectedTechnoType::Buildings, pTypeExt->SpyEffect_BuildingVeterancy_Cumulative);
			else
				HouseExt::TechnoUpgrade(pInfiltratorHouse, 0,
					pTypeExt->SpyEffect_BuildingVeterancy_Types, pTypeExt->SpyEffect_BuildingVeterancy_Ignore,
					AffectedTechnoType::Buildings, pTypeExt->SpyEffect_BuildingVeterancy_Cumulative);
		}
		else if (pTypeExt->SpyEffect_BuildingVeterancy != 0)
		{
			if (pTypeExt->SpyEffect_Veterancy_AffectBuildingOwner)
				HouseExt::TechnoUpgrade(pVictimHouse, pTypeExt->SpyEffect_BuildingVeterancy,
					pTypeExt->SpyEffect_BuildingVeterancy_Types, pTypeExt->SpyEffect_BuildingVeterancy_Ignore,
					AffectedTechnoType::Buildings, pTypeExt->SpyEffect_BuildingVeterancy_Cumulative);
			else
				HouseExt::TechnoUpgrade(pInfiltratorHouse, pTypeExt->SpyEffect_BuildingVeterancy,
					pTypeExt->SpyEffect_BuildingVeterancy_Types, pTypeExt->SpyEffect_BuildingVeterancy_Ignore,
					AffectedTechnoType::Buildings, pTypeExt->SpyEffect_BuildingVeterancy_Cumulative);
		}

		if (!pTypeExt->SpyEffect_SuperWeaponTypes.empty())
		{
			for (size_t i = 0; i < pTypeExt->SpyEffect_SuperWeaponTypes.size(); i++)
			{
				bool inhouseext = false;
				bool inhousedelay = false;
				for (size_t j = 0; j < pInfiltratorExt->SpySuperWeaponTypes.size(); j++)
				{
					if (pInfiltratorExt->SpySuperWeaponTypes[j] == pTypeExt->SpyEffect_SuperWeaponTypes[i])
					{
						inhouseext = true;

						int delay;
						if (pInfiltratorExt->SpySuperWeaponDelay.size() >= j)
							delay = pInfiltratorExt->SpySuperWeaponDelay[j];
						else
							delay = 0;

						if (delay > 0)
							inhousedelay = true;
						else
							pInfiltratorExt->SpySuperWeaponDelay[j] = pTypeExt->SpyEffect_SuperWeaponTypes_Delay.size() > i ?
							pTypeExt->SpyEffect_SuperWeaponTypes_Delay[i] : 0;
						break;
					}
				}

				if (inhousedelay)
					continue;

				SuperClass* pSuper = pInfiltratorHouse->Supers[pTypeExt->SpyEffect_SuperWeaponTypes[i]];
				if (pTypeExt->SpyEffect_SuperWeaponTypes_Permanent[i])
					pInfiltratorExt->SW_Permanents[pSuper->Type->ArrayIndex] = true;

				if (pSuper->Granted && pTypeExt->SpyEffect_SuperWeaponTypes_AffectOwned[i])
					pSuper->RechargeTimer.TimeLeft -= pSuper->RechargeTimer.TimeLeft;
				else
				{
					bool granted;
					granted = pSuper->Grant(true, true, false);
					if (granted)
					{
						if (MouseClass::Instance->AddCameo(AbstractType::Special, pTypeExt->SpyEffect_SuperWeaponTypes[i]))
							MouseClass::Instance->RepaintSidebar(1);
					}
				}

				if (!inhouseext)
				{
					pInfiltratorExt->SpySuperWeaponTypes.emplace_back(pTypeExt->SpyEffect_SuperWeaponTypes[i]);
					int dealy;
					if (pTypeExt->SpyEffect_SuperWeaponTypes_Delay.size() > i)
						dealy = pTypeExt->SpyEffect_SuperWeaponTypes_Delay[i];
					else
						dealy = 0;
					pInfiltratorExt->SpySuperWeaponDelay.emplace_back(dealy);
				}
			}
		}

		if (!pTypeExt->SpyEffect_RechargeSuperWeaponTypes.empty())
		{
			for (size_t i = 0; i < pTypeExt->SpyEffect_RechargeSuperWeaponTypes.size(); i++)
			{
				SuperClass* pSuper = pVictimHouse->Supers[pTypeExt->SpyEffect_RechargeSuperWeaponTypes[i]];
				int time;
				if (pTypeExt->SpyEffect_RechargeSuperWeaponTypes_Duration.size() > i)
					time = static_cast<int>(pTypeExt->SpyEffect_RechargeSuperWeaponTypes_Duration[i]);
				else
					time = 0;

				if (abs(time) <= 1)
					time = static_cast<int>(pSuper->Type->RechargeTime * time);

				if (pTypeExt->SpyEffect_RechargeSuperWeaponTypes_SetPercentage.size() > i)
				{
					if (pTypeExt->SpyEffect_RechargeSuperWeaponTypes_SetPercentage[i] > 0)
						pSuper->RechargeTimer.TimeLeft = Game::F2I(pSuper->Type->RechargeTime * pTypeExt->SpyEffect_RechargeSuperWeaponTypes_SetPercentage[i]);
				}
				pSuper->RechargeTimer.TimeLeft += time;

				if (SWTypeExt::ExtMap.Find(pSuper->Type)->SW_Cumulative)
				{
					int count;
					if (pTypeExt->SpyEffect_RechargeSuperWeaponTypes_CumulativeCount.size() > i)
						count = pTypeExt->SpyEffect_RechargeSuperWeaponTypes_CumulativeCount[i];
					else
						count = 0;

					if (pVictimExt->SuperWeaponCumulativeCount[i] - count >= 0)
						pVictimExt->SuperWeaponCumulativeCount[i] -= count;
					else
						pVictimExt->SuperWeaponCumulativeCount[i] = 0;
				}
			}
		}

		if (pTypeExt->SpyEffect_RevealSightDuration != 0)
		{
			int range = pTypeExt->SpyEffect_RevealSightRange != 0 ? pTypeExt->SpyEffect_RevealSightRange : pBuilding->Type->Sight;

			if (pTypeExt->SpyEffect_RevealSightDuration < 0)
			{
				CoordStruct coords = pBuilding->GetCenterCoords();

				if (range < 0)
					MapClass::Instance->Reveal(pInfiltratorHouse);
				else
					MapClass::Instance->RevealArea1(&coords , range, pInfiltratorHouse, CellStruct::Empty, 0, 0, 0, 1);

			}
			else
			{
				bool hasreveal = false;
				for (size_t i = 0; i < pExt->RevealSightHouses.size(); i++)
				{
					if (pExt->RevealSightHouses[i] == pInfiltratorHouse)
					{
						pExt->RevealSightRanges[i] = range;
						pExt->RevealSightPermanents[i] = pTypeExt->SpyEffect_RevealSightPermanent;
						pExt->RevealSightTimers[i].Start(pTypeExt->SpyEffect_RevealSightDuration);
						hasreveal = true;
						break;
					}
				}

				if (!hasreveal)
				{
					pExt->RevealSightHouses.emplace_back(pInfiltratorHouse);
					pExt->RevealSightRanges.emplace_back(range);
					pExt->RevealSightPermanents.emplace_back(pTypeExt->SpyEffect_RevealSightPermanent);

					CDTimerClass timer;
					timer.Start(pTypeExt->SpyEffect_RevealSightDuration);
					pExt->RevealSightTimers.emplace_back(timer);
				}
			}
		}

		if (pTypeExt->SpyEffect_RadarJamDuration != 0)
		{
			if (pTypeExt->SpyEffect_RadarJamDuration > 0)
			{
				pVictimHouse->RadarBlackoutTimer.Start(pTypeExt->SpyEffect_RadarJamDuration);
			}
			else
			{
				pInfiltratorHouse->RadarBlackoutTimer.Start(abs(pTypeExt->SpyEffect_RadarJamDuration));
			}
		}

		if (pTypeExt->SpyEffect_PowerOutageDuration < 0)
			pInfiltratorHouse->PowerBlackoutTimer.Start(abs(pTypeExt->SpyEffect_PowerOutageDuration));

		if (pTypeExt->SpyEffect_GapRadarDuration != 0)
		{
			if (pTypeExt->SpyEffect_GapRadarDuration > 0)
			{
				pVictimHouse->ReshroudMap();
				pVictimExt->GapRadarTimer.Start(pTypeExt->SpyEffect_GapRadarDuration);
			}
			else
			{
				pInfiltratorHouse->ReshroudMap();
				pInfiltratorExt->GapRadarTimer.Start(abs(pTypeExt->SpyEffect_GapRadarDuration));
			}
		}

		if (pTypeExt->SpyEffect_RevealRadarSightDuration != 0)
		{
			if (pTypeExt->SpyEffect_RevealRadarSightDuration > 0)
			{
				bool hasbuilding = false;
				for (size_t i = 0; i < pVictimExt->RevealRadarSightBuildings.size(); i++)
				{
					if (pVictimExt->RevealRadarSightBuildings[i] == pBuilding)
					{
						hasbuilding = true;
						pVictimExt->KeepRevealRadarSights[i] = pTypeExt->SpyEffect_KeepRevealRadarSight;
						pVictimExt->RevealRadarSightBuildingOwners[i] = pVictimHouse;
						pVictimExt->RevealRadarSightOwners[i] = pInfiltratorHouse;
						pVictimExt->RevealRadarSightPermanents[i] = pTypeExt->SpyEffect_RevealRadarSightPermanent;
						pVictimExt->RevealRadarSights_Aircraft[i] = pTypeExt->SpyEffect_RevealRadarSight_Aircraft;
						pVictimExt->RevealRadarSights_Building[i] = pTypeExt->SpyEffect_RevealRadarSight_Building;
						pVictimExt->RevealRadarSights_Infantry[i] = pTypeExt->SpyEffect_RevealRadarSight_Infantry;
						pVictimExt->RevealRadarSights_Unit[i] = pTypeExt->SpyEffect_RevealRadarSight_Unit;
						pVictimExt->RevealRadarSightTimers[i].Start(pTypeExt->SpyEffect_RevealRadarSightDuration);
						break;
					}
				}

				if (!hasbuilding)
				{
					pVictimExt->KeepRevealRadarSights.emplace_back(pTypeExt->SpyEffect_KeepRevealRadarSight);
					pVictimExt->RevealRadarSightBuildingOwners.emplace_back(pVictimHouse);
					pVictimExt->RevealRadarSightBuildings.emplace_back(pBuilding);
					pVictimExt->RevealRadarSightOwners.emplace_back(pInfiltratorHouse);
					pVictimExt->RevealRadarSightPermanents.emplace_back(pTypeExt->SpyEffect_RevealRadarSightPermanent);
					pVictimExt->RevealRadarSights_Aircraft.emplace_back(pTypeExt->SpyEffect_RevealRadarSight_Aircraft);
					pVictimExt->RevealRadarSights_Building.emplace_back(pTypeExt->SpyEffect_RevealRadarSight_Building);
					pVictimExt->RevealRadarSights_Infantry.emplace_back(pTypeExt->SpyEffect_RevealRadarSight_Infantry);
					pVictimExt->RevealRadarSights_Unit.emplace_back(pTypeExt->SpyEffect_RevealRadarSight_Unit);

					CDTimerClass timer;
					timer.Start(pTypeExt->SpyEffect_RevealRadarSightDuration);
					pVictimExt->RevealRadarSightTimers.emplace_back(timer);
				}
			}
			else
			{
				bool hasbuilding = false;
				for (size_t i = 0; i < pInfiltratorExt->RevealRadarSightBuildings.size(); i++)
				{
					if (pInfiltratorExt->RevealRadarSightBuildings[i] == pBuilding)
					{
						hasbuilding = true;
						pInfiltratorExt->KeepRevealRadarSights[i] = pTypeExt->SpyEffect_KeepRevealRadarSight;
						pInfiltratorExt->RevealRadarSightBuildingOwners[i] = pVictimHouse;
						pInfiltratorExt->RevealRadarSightOwners[i] = pVictimHouse;
						pInfiltratorExt->RevealRadarSightPermanents[i] = pTypeExt->SpyEffect_RevealRadarSightPermanent;
						pInfiltratorExt->RevealRadarSights_Aircraft[i] = pTypeExt->SpyEffect_RevealRadarSight_Aircraft;
						pInfiltratorExt->RevealRadarSights_Building[i] = pTypeExt->SpyEffect_RevealRadarSight_Building;
						pInfiltratorExt->RevealRadarSights_Infantry[i] = pTypeExt->SpyEffect_RevealRadarSight_Infantry;
						pInfiltratorExt->RevealRadarSights_Unit[i] = pTypeExt->SpyEffect_RevealRadarSight_Unit;
						pInfiltratorExt->RevealRadarSightTimers[i].Start(abs(pTypeExt->SpyEffect_RevealRadarSightDuration));
						break;
					}
				}

				if (!hasbuilding)
				{
					pInfiltratorExt->KeepRevealRadarSights.emplace_back(pTypeExt->SpyEffect_KeepRevealRadarSight);
					pInfiltratorExt->RevealRadarSightBuildingOwners.emplace_back(pVictimHouse);
					pInfiltratorExt->RevealRadarSightBuildings.emplace_back(pBuilding);
					pInfiltratorExt->RevealRadarSightOwners.emplace_back(pVictimHouse);
					pInfiltratorExt->RevealRadarSightPermanents.emplace_back(pTypeExt->SpyEffect_RevealRadarSightPermanent);
					pInfiltratorExt->RevealRadarSights_Aircraft.emplace_back(pTypeExt->SpyEffect_RevealRadarSight_Aircraft);
					pInfiltratorExt->RevealRadarSights_Building.emplace_back(pTypeExt->SpyEffect_RevealRadarSight_Building);
					pInfiltratorExt->RevealRadarSights_Infantry.emplace_back(pTypeExt->SpyEffect_RevealRadarSight_Infantry);
					pInfiltratorExt->RevealRadarSights_Unit.emplace_back(pTypeExt->SpyEffect_RevealRadarSight_Unit);

					CDTimerClass timer;
					timer.Start(abs(pTypeExt->SpyEffect_RevealRadarSightDuration));
					pInfiltratorExt->RevealRadarSightTimers.emplace_back(timer);
				}
			}
		}

		if (pTypeExt->SpyEffect_InfantryCostBonus_Reset)
		{
			if (pTypeExt->SpyEffect_CostBonus_AffectBuildingOwner)
				HouseExt::TechnoCostBonusReset(pVictimHouse,
					pTypeExt->SpyEffect_InfantryCostBonus_Types,
					pTypeExt->SpyEffect_InfantryCostBonus_Ignore,
					AffectedTechnoType::Infantry);
			else
				HouseExt::TechnoCostBonusReset(pInfiltratorHouse,
					pTypeExt->SpyEffect_InfantryCostBonus_Types,
					pTypeExt->SpyEffect_InfantryCostBonus_Ignore,
					AffectedTechnoType::Infantry);
		}
		else if (pTypeExt->SpyEffect_InfantryCostBonus != 1.0)
		{
			if (pTypeExt->SpyEffect_CostBonus_AffectBuildingOwner)
				HouseExt::TechnoCostBonus(pVictimHouse,
					pTypeExt->SpyEffect_InfantryCostBonus,
					pTypeExt->SpyEffect_InfantryCostBonus_Types,
					pTypeExt->SpyEffect_InfantryCostBonus_Ignore,
					AffectedTechnoType::Infantry,
					pTypeExt->SpyEffect_InfantryCostBonus_Max,
					pTypeExt->SpyEffect_InfantryCostBonus_Min);
			else
				HouseExt::TechnoCostBonus(pInfiltratorHouse,
					pTypeExt->SpyEffect_InfantryCostBonus,
					pTypeExt->SpyEffect_InfantryCostBonus_Types,
					pTypeExt->SpyEffect_InfantryCostBonus_Ignore,
					AffectedTechnoType::Infantry,
					pTypeExt->SpyEffect_InfantryCostBonus_Max,
					pTypeExt->SpyEffect_InfantryCostBonus_Min);
		}

		if (pTypeExt->SpyEffect_UnitsCostBonus_Reset)
		{
			if (pTypeExt->SpyEffect_CostBonus_AffectBuildingOwner)
				HouseExt::TechnoCostBonusReset(pVictimHouse,
					pTypeExt->SpyEffect_UnitsCostBonus_Types,
					pTypeExt->SpyEffect_UnitsCostBonus_Ignore,
					AffectedTechnoType::Units);
			else
				HouseExt::TechnoCostBonusReset(pInfiltratorHouse,
					pTypeExt->SpyEffect_UnitsCostBonus_Types,
					pTypeExt->SpyEffect_UnitsCostBonus_Ignore,
					AffectedTechnoType::Units);
		}
		else if (pTypeExt->SpyEffect_UnitsCostBonus != 1.0)
		{
			if (pTypeExt->SpyEffect_CostBonus_AffectBuildingOwner)
				HouseExt::TechnoCostBonus(pVictimHouse,
					pTypeExt->SpyEffect_UnitsCostBonus,
					pTypeExt->SpyEffect_UnitsCostBonus_Types,
					pTypeExt->SpyEffect_UnitsCostBonus_Ignore,
					AffectedTechnoType::Units,
					pTypeExt->SpyEffect_UnitsCostBonus_Max,
					pTypeExt->SpyEffect_UnitsCostBonus_Min);
			else
				HouseExt::TechnoCostBonus(pInfiltratorHouse,
					pTypeExt->SpyEffect_UnitsCostBonus,
					pTypeExt->SpyEffect_UnitsCostBonus_Types,
					pTypeExt->SpyEffect_UnitsCostBonus_Ignore,
					AffectedTechnoType::Units,
					pTypeExt->SpyEffect_UnitsCostBonus_Max,
					pTypeExt->SpyEffect_UnitsCostBonus_Min);
		}

		if (pTypeExt->SpyEffect_NavalCostBonus_Reset)
		{
			if (pTypeExt->SpyEffect_CostBonus_AffectBuildingOwner)
				HouseExt::TechnoCostBonusReset(pVictimHouse,
					pTypeExt->SpyEffect_NavalCostBonus_Types,
					pTypeExt->SpyEffect_NavalCostBonus_Ignore,
					AffectedTechnoType::Naval);
			else
				HouseExt::TechnoCostBonusReset(pInfiltratorHouse,
					pTypeExt->SpyEffect_NavalCostBonus_Types,
					pTypeExt->SpyEffect_NavalCostBonus_Ignore,
					AffectedTechnoType::Naval);
		}
		else if (pTypeExt->SpyEffect_NavalCostBonus != 1.0)
		{
			if (pTypeExt->SpyEffect_CostBonus_AffectBuildingOwner)
				HouseExt::TechnoCostBonus(pVictimHouse,
					pTypeExt->SpyEffect_NavalCostBonus,
					pTypeExt->SpyEffect_NavalCostBonus_Types,
					pTypeExt->SpyEffect_NavalCostBonus_Ignore,
					AffectedTechnoType::Naval,
					pTypeExt->SpyEffect_NavalCostBonus_Max,
					pTypeExt->SpyEffect_NavalCostBonus_Min);
			else
				HouseExt::TechnoCostBonus(pInfiltratorHouse,
					pTypeExt->SpyEffect_NavalCostBonus,
					pTypeExt->SpyEffect_NavalCostBonus_Types,
					pTypeExt->SpyEffect_NavalCostBonus_Ignore,
					AffectedTechnoType::Naval,
					pTypeExt->SpyEffect_NavalCostBonus_Max,
					pTypeExt->SpyEffect_NavalCostBonus_Min);
		}

		if (pTypeExt->SpyEffect_AircraftCostBonus_Reset)
		{
			if (pTypeExt->SpyEffect_CostBonus_AffectBuildingOwner)
				HouseExt::TechnoCostBonusReset(pVictimHouse,
					pTypeExt->SpyEffect_AircraftCostBonus_Types,
					pTypeExt->SpyEffect_AircraftCostBonus_Ignore,
					AffectedTechnoType::Aircraft);
			else
				HouseExt::TechnoCostBonusReset(pInfiltratorHouse,
					pTypeExt->SpyEffect_AircraftCostBonus_Types,
					pTypeExt->SpyEffect_AircraftCostBonus_Ignore,
					AffectedTechnoType::Aircraft);
		}
		else if (pTypeExt->SpyEffect_AircraftCostBonus != 1.0)
		{
			if (pTypeExt->SpyEffect_CostBonus_AffectBuildingOwner)
				HouseExt::TechnoCostBonus(pVictimHouse,
					pTypeExt->SpyEffect_AircraftCostBonus,
					pTypeExt->SpyEffect_AircraftCostBonus_Types,
					pTypeExt->SpyEffect_AircraftCostBonus_Ignore,
					AffectedTechnoType::Aircraft,
					pTypeExt->SpyEffect_AircraftCostBonus_Max,
					pTypeExt->SpyEffect_AircraftCostBonus_Min);
			else
				HouseExt::TechnoCostBonus(pInfiltratorHouse,
					pTypeExt->SpyEffect_AircraftCostBonus,
					pTypeExt->SpyEffect_AircraftCostBonus_Types,
					pTypeExt->SpyEffect_AircraftCostBonus_Ignore,
					AffectedTechnoType::Aircraft,
					pTypeExt->SpyEffect_AircraftCostBonus_Max,
					pTypeExt->SpyEffect_AircraftCostBonus_Min);
		}

		if (pTypeExt->SpyEffect_BuildingsCostBonus_Reset)
		{
			if (pTypeExt->SpyEffect_CostBonus_AffectBuildingOwner)
				HouseExt::TechnoCostBonusReset(pVictimHouse,
					pTypeExt->SpyEffect_BuildingsCostBonus_Types,
					pTypeExt->SpyEffect_BuildingsCostBonus_Ignore,
					AffectedTechnoType::Buildings);
			else
				HouseExt::TechnoCostBonusReset(pInfiltratorHouse,
					pTypeExt->SpyEffect_BuildingsCostBonus_Types,
					pTypeExt->SpyEffect_BuildingsCostBonus_Ignore,
					AffectedTechnoType::Buildings);
		}
		else if (pTypeExt->SpyEffect_BuildingsCostBonus != 1.0)
		{
			if (pTypeExt->SpyEffect_CostBonus_AffectBuildingOwner)
				HouseExt::TechnoCostBonus(pVictimHouse,
					pTypeExt->SpyEffect_BuildingsCostBonus,
					pTypeExt->SpyEffect_BuildingsCostBonus_Types,
					pTypeExt->SpyEffect_BuildingsCostBonus_Ignore,
					AffectedTechnoType::Buildings,
					pTypeExt->SpyEffect_BuildingsCostBonus_Max,
					pTypeExt->SpyEffect_BuildingsCostBonus_Min);
			else
				HouseExt::TechnoCostBonus(pInfiltratorHouse,
					pTypeExt->SpyEffect_BuildingsCostBonus,
					pTypeExt->SpyEffect_BuildingsCostBonus_Types,
					pTypeExt->SpyEffect_BuildingsCostBonus_Ignore,
					AffectedTechnoType::Buildings,
					pTypeExt->SpyEffect_BuildingsCostBonus_Max,
					pTypeExt->SpyEffect_BuildingsCostBonus_Min);
		}

		if (pTypeExt->SpyEffect_DefensesCostBonus_Reset)
		{
			if (pTypeExt->SpyEffect_CostBonus_AffectBuildingOwner)
				HouseExt::TechnoCostBonusReset(pVictimHouse,
					pTypeExt->SpyEffect_DefensesCostBonus_Types,
					pTypeExt->SpyEffect_DefensesCostBonus_Ignore,
					AffectedTechnoType::Defenses);
			else
				HouseExt::TechnoCostBonusReset(pInfiltratorHouse,
					pTypeExt->SpyEffect_DefensesCostBonus_Types,
					pTypeExt->SpyEffect_DefensesCostBonus_Ignore,
					AffectedTechnoType::Defenses);
		}
		else if (pTypeExt->SpyEffect_DefensesCostBonus != 1.0)
		{
			if (pTypeExt->SpyEffect_CostBonus_AffectBuildingOwner)
				HouseExt::TechnoCostBonus(pVictimHouse,
					pTypeExt->SpyEffect_DefensesCostBonus,
					pTypeExt->SpyEffect_DefensesCostBonus_Types,
					pTypeExt->SpyEffect_DefensesCostBonus_Ignore,
					AffectedTechnoType::Defenses,
					pTypeExt->SpyEffect_DefensesCostBonus_Max,
					pTypeExt->SpyEffect_DefensesCostBonus_Min);
			else
				HouseExt::TechnoCostBonus(pInfiltratorHouse,
					pTypeExt->SpyEffect_DefensesCostBonus,
					pTypeExt->SpyEffect_DefensesCostBonus_Types,
					pTypeExt->SpyEffect_DefensesCostBonus_Ignore,
					AffectedTechnoType::Defenses,
					pTypeExt->SpyEffect_DefensesCostBonus_Max,
					pTypeExt->SpyEffect_DefensesCostBonus_Min);
		}

		if (pTypeExt->SpyEffect_PurifierBonus_Reset)
		{
			if (pTypeExt->SpyEffect_PurifierBonus_AffectBuildingOwner)
				pVictimExt->OrePurifierBonus = 0.0;
			else
				pInfiltratorExt->OrePurifierBonus = 0.0;
		}
		else if (pTypeExt->SpyEffect_PurifierBonus != 0.0)
		{
			if (pTypeExt->SpyEffect_PurifierBonus_AffectBuildingOwner)
				pVictimExt->OrePurifierBonus += pTypeExt->SpyEffect_PurifierBonus;
			else
				pInfiltratorExt->OrePurifierBonus += pTypeExt->SpyEffect_PurifierBonus;
		}

		if (pTypeExt->SpyEffect_Anim && pTypeExt->SpyEffect_Anim_Duration != 0)
		{
			pExt->SpyEffectAnim = GameCreate<AnimClass>(pTypeExt->SpyEffect_Anim, pBuilding->GetCoords());
			pExt->SpyEffectAnim->SetOwnerObject(pBuilding);
			pExt->SpyEffectAnim->RemainingIterations = 0xFFU;
			pExt->SpyEffectAnim->Owner = pVictimHouse;

			pExt->SpyEffectAnimDuration = pTypeExt->SpyEffect_Anim_Duration;
			pExt->SpyEffectAnimDisplayHouses = pTypeExt->SpyEffect_Anim_DisplayHouses;
		}

		if (pTypeExt->SpyEffect_Messages.size() > 0)
		{
			for (size_t i = 0; i < pTypeExt->SpyEffect_Messages.size(); i++)
			{
				if (pTypeExt->SpyEffect_Message_ShowOwners[i] == ShowMessageHouse::All ||
					(pTypeExt->SpyEffect_Message_ShowOwners[i] == ShowMessageHouse::Invoker && HouseClass::CurrentPlayer == pInfiltratorHouse) ||
					(pTypeExt->SpyEffect_Message_ShowOwners[i] == ShowMessageHouse::Victim && HouseClass::CurrentPlayer == pVictimHouse))
				{
					int color = 0;
					if (pTypeExt->SpyEffect_Message_ColorTypes[i] == ShowMessageHouse::Invoker)
						color = pInfiltratorHouse->ColorSchemeIndex;
					else if (pTypeExt->SpyEffect_Message_ColorTypes[i] == ShowMessageHouse::Victim)
						color = pVictimHouse->ColorSchemeIndex;
					else
						color = HouseClass::CurrentPlayer->ColorSchemeIndex;

					MessageListClass::Instance->PrintMessage(pTypeExt->SpyEffect_Messages[i].Text, RulesClass::Instance->MessageDelay, color);
				}
			}
		}

		if (pTypeExt->SpyEffect_CaptureDelay != 0)
		{
			if (!pExt->CaptureTimer.HasStarted())
			{
				if (pTypeExt->SpyEffect_CaptureDelay > 0)
				{
					pExt->CaptureTimer.Start(pTypeExt->SpyEffect_CaptureDelay);
					pExt->CaptureOwner = pInfiltratorHouse;
					if (pTypeExt->SpyEffect_CaptureCount > 0)
					{
						pExt->OriginalOwner = pVictimHouse;
						pExt->CaptureCount = pTypeExt->SpyEffect_CaptureCount;
					}
				}
				else
				{
					pExt->CaptureTimer.Start(static_cast<int>(RulesClass::Instance->C4Delay));
					pExt->CaptureOwner = pInfiltratorHouse;
					if (pTypeExt->SpyEffect_CaptureCount > 0)
					{
						pExt->OriginalOwner = pVictimHouse;
						pExt->CaptureCount = pTypeExt->SpyEffect_CaptureCount;
					}
				}
			}
		}

		if (pTypeExt->SpyEffect_SellDelay != 0 && pBuilding->Type->LoadBuildup())
		{
			if (!pExt->SellTimer.HasStarted())
			{
				if (pTypeExt->SpyEffect_SellDelay > 0)
					pExt->SellTimer.Start(pTypeExt->SpyEffect_SellDelay);
				else
					pExt->SellTimer.Start(static_cast<int>(RulesClass::Instance->C4Delay));
			}
		}
	}

	for (auto const& pUpgrade : pBuilding->Upgrades)
	{
		if (const auto pUpgradeExt = BuildingTypeExt::ExtMap.Find(pUpgrade))
		{
			HandleInfiltrateUpgrades(pBuilding, pInfiltratorHouse, pUpgradeExt);
		}
	}

	return true;
}

bool BuildingExt::HandleInfiltrateUpgrades(BuildingClass* pBuilding, HouseClass* pInfiltratorHouse, BuildingTypeExt::ExtData* pTypeExt)
{
	BuildingExt::ExtData* pExt = BuildingExt::ExtMap.Find(pBuilding);

	if (!pTypeExt->SpyEffect_Custom)
		return false;

	auto pVictimHouse = pBuilding->Owner;
	HouseExt::ExtData* pVictimExt = HouseExt::ExtMap.Find(pVictimHouse);
	HouseExt::ExtData* pInfiltratorExt = HouseExt::ExtMap.Find(pInfiltratorHouse);
	if (pInfiltratorHouse != pVictimHouse)
	{
		// I assume you were not launching for real, Morton

		auto launchTheSWHere = [pBuilding](SuperClass* const pSuper, HouseClass* const pHouse)
		{
			int oldstart = pSuper->RechargeTimer.StartTime;
			int oldleft = pSuper->RechargeTimer.TimeLeft;
			pSuper->SetReadiness(true);
			pSuper->Launch(CellClass::Coord2Cell(pBuilding->Location), pHouse->IsCurrentPlayer());
			pSuper->Reset();
			pSuper->RechargeTimer.StartTime = oldstart;
			pSuper->RechargeTimer.TimeLeft = oldleft;
		};

		if (pTypeExt->SpyEffect_VictimSuperWeapon.isset())
		{
			if (const auto pSuper = pVictimHouse->Supers.GetItem(pTypeExt->SpyEffect_VictimSuperWeapon.Get()))
				launchTheSWHere(pSuper, pVictimHouse);
		}

		if (pTypeExt->SpyEffect_InfiltratorSuperWeapon.isset())
		{
			if (const auto pSuper = pInfiltratorHouse->Supers.GetItem(pTypeExt->SpyEffect_InfiltratorSuperWeapon.Get()))
				launchTheSWHere(pSuper, pInfiltratorHouse);
		}

		if (pTypeExt->SpyEffect_StolenMoneyAmount != 0 || pTypeExt->SpyEffect_StolenMoneyPercentage != 0)
		{
			int money = 0;
			if (pTypeExt->SpyEffect_StolenMoneyAmount != 0 && pTypeExt->SpyEffect_StolenMoneyPercentage != 0)
			{
				if ((pTypeExt->SpyEffect_StolenMoneyAmount * pTypeExt->SpyEffect_StolenMoneyPercentage < 0) ||
					(pTypeExt->SpyEffect_StolenMoneyAmount < 0 && pTypeExt->SpyEffect_StolenMoneyPercentage < 0))
				{
					money = -static_cast<int>(abs(pInfiltratorHouse->Available_Money() * pTypeExt->SpyEffect_StolenMoneyPercentage));
					if (abs(money) > abs(pTypeExt->SpyEffect_StolenMoneyAmount))
						money = -abs(pTypeExt->SpyEffect_StolenMoneyAmount);

					if (abs(money) > pInfiltratorHouse->Available_Money())
						money = -pInfiltratorHouse->Available_Money();
				}
				else
				{
					money = static_cast<int>(pVictimHouse->Available_Money() * pTypeExt->SpyEffect_StolenMoneyPercentage);
					if (money > pTypeExt->SpyEffect_StolenMoneyAmount)
						money = pTypeExt->SpyEffect_StolenMoneyAmount;

					if (money > pVictimHouse->Available_Money())
						money = pVictimHouse->Available_Money();
				}
			}
			else if (pTypeExt->SpyEffect_StolenMoneyAmount != 0)
			{
				money = pTypeExt->SpyEffect_StolenMoneyAmount;

				if (pTypeExt->SpyEffect_StolenMoneyAmount < 0)
				{
					if (abs(money) > pInfiltratorHouse->Available_Money())
						money = -pInfiltratorHouse->Available_Money();
				}
				else
				{
					if (money > pVictimHouse->Available_Money())
						money = pVictimHouse->Available_Money();
				}
			}
			else if (pTypeExt->SpyEffect_StolenMoneyPercentage != 0)
			{
				if (pTypeExt->SpyEffect_StolenMoneyPercentage < 0)
					money = static_cast<int>(pInfiltratorHouse->Available_Money() * pTypeExt->SpyEffect_StolenMoneyPercentage);
				else
					money = static_cast<int>(pVictimHouse->Available_Money() * pTypeExt->SpyEffect_StolenMoneyPercentage);
			}

			pInfiltratorHouse->TransactMoney(money);
			pVictimHouse->TransactMoney(-money);

			if (pTypeExt->SpyEffect_StolenMoneyDisplay && money != 0)
				FlyingStrings::AddMoneyString(-money, pVictimHouse, pTypeExt->SpyEffect_StolenMoneyDisplay_Houses, pBuilding->GetCenterCoords(), pTypeExt->SpyEffect_StolenMoneyDisplay_Offset);
		}

		if (pTypeExt->SpyEffect_BuildingOfflineDuration > 0)
			pExt->OfflineTimer = pTypeExt->SpyEffect_BuildingOfflineDuration;

		if (pTypeExt->SpyEffect_InfantryDeactivate_Duration > 0)
		{
			pVictimExt->DeactivateInfantry_Duration.emplace_back(pTypeExt->SpyEffect_InfantryDeactivate_Duration);
			pVictimExt->DeactivateInfantry_Types.emplace_back(pTypeExt->SpyEffect_InfantryDeactivate_Types);
			pVictimExt->DeactivateInfantry_Ignore.emplace_back(pTypeExt->SpyEffect_InfantryDeactivate_Ignore);
		}

		if (pTypeExt->SpyEffect_VehicleDeactivate_Duration > 0)
		{
			pVictimExt->DeactivateVehicle_Duration.emplace_back(pTypeExt->SpyEffect_VehicleDeactivate_Duration);
			pVictimExt->DeactivateVehicle_Types.emplace_back(pTypeExt->SpyEffect_VehicleDeactivate_Types);
			pVictimExt->DeactivateVehicle_Ignore.emplace_back(pTypeExt->SpyEffect_VehicleDeactivate_Ignore);
		}

		if (pTypeExt->SpyEffect_NavalDeactivate_Duration > 0)
		{
			pVictimExt->DeactivateNaval_Duration.emplace_back(pTypeExt->SpyEffect_NavalDeactivate_Duration);
			pVictimExt->DeactivateNaval_Types.emplace_back(pTypeExt->SpyEffect_NavalDeactivate_Types);
			pVictimExt->DeactivateNaval_Ignore.emplace_back(pTypeExt->SpyEffect_NavalDeactivate_Ignore);
		}

		if (pTypeExt->SpyEffect_AircraftDeactivate_Duration > 0)
		{
			pVictimExt->DeactivateAircraft_Duration.emplace_back(pTypeExt->SpyEffect_AircraftDeactivate_Duration);
			pVictimExt->DeactivateAircraft_Types.emplace_back(pTypeExt->SpyEffect_AircraftDeactivate_Types);
			pVictimExt->DeactivateAircraft_Ignore.emplace_back(pTypeExt->SpyEffect_AircraftDeactivate_Ignore);
		}

		if (pTypeExt->SpyEffect_BuildingDeactivate_Duration > 0)
		{
			pVictimExt->DeactivateBuilding_Duration.emplace_back(pTypeExt->SpyEffect_BuildingDeactivate_Duration);
			pVictimExt->DeactivateBuilding_Types.emplace_back(pTypeExt->SpyEffect_BuildingDeactivate_Types);
			pVictimExt->DeactivateBuilding_Ignore.emplace_back(pTypeExt->SpyEffect_BuildingDeactivate_Ignore);
		}

		if (pTypeExt->SpyEffect_DefenseDeactivate_Duration > 0)
		{
			pVictimExt->DeactivateDefense_Duration.emplace_back(pTypeExt->SpyEffect_DefenseDeactivate_Duration);
			pVictimExt->DeactivateDefense_Types.emplace_back(pTypeExt->SpyEffect_DefenseDeactivate_Types);
			pVictimExt->DeactivateDefense_Ignore.emplace_back(pTypeExt->SpyEffect_DefenseDeactivate_Ignore);
		}

		if (pTypeExt->SpyEffect_InfantryVeterancy_Reset)
		{
			if (pTypeExt->SpyEffect_Veterancy_AffectBuildingOwner)
				HouseExt::TechnoUpgrade(pVictimHouse, 0,
					pTypeExt->SpyEffect_InfantryVeterancy_Types, pTypeExt->SpyEffect_InfantryVeterancy_Ignore,
					AffectedTechnoType::Infantry, pTypeExt->SpyEffect_InfantryVeterancy_Cumulative);
			else
				HouseExt::TechnoUpgrade(pInfiltratorHouse, 0,
					pTypeExt->SpyEffect_InfantryVeterancy_Types, pTypeExt->SpyEffect_InfantryVeterancy_Ignore,
					AffectedTechnoType::Infantry, pTypeExt->SpyEffect_InfantryVeterancy_Cumulative);
		}
		else if (pTypeExt->SpyEffect_InfantryVeterancy != 0)
		{
			if (pTypeExt->SpyEffect_Veterancy_AffectBuildingOwner)
				HouseExt::TechnoUpgrade(pVictimHouse, pTypeExt->SpyEffect_InfantryVeterancy,
					pTypeExt->SpyEffect_InfantryVeterancy_Types, pTypeExt->SpyEffect_InfantryVeterancy_Ignore,
					AffectedTechnoType::Infantry, pTypeExt->SpyEffect_InfantryVeterancy_Cumulative);
			else
				HouseExt::TechnoUpgrade(pInfiltratorHouse, pTypeExt->SpyEffect_InfantryVeterancy,
					pTypeExt->SpyEffect_InfantryVeterancy_Types, pTypeExt->SpyEffect_InfantryVeterancy_Ignore,
					AffectedTechnoType::Infantry, pTypeExt->SpyEffect_InfantryVeterancy_Cumulative);
		}

		if (pTypeExt->SpyEffect_VehicleVeterancy_Reset)
		{
			if (pTypeExt->SpyEffect_Veterancy_AffectBuildingOwner)
				HouseExt::TechnoUpgrade(pVictimHouse, 0,
					pTypeExt->SpyEffect_VehicleVeterancy_Types, pTypeExt->SpyEffect_VehicleVeterancy_Ignore,
					AffectedTechnoType::Units, pTypeExt->SpyEffect_VehicleVeterancy_Cumulative);
			else
				HouseExt::TechnoUpgrade(pInfiltratorHouse, 0,
					pTypeExt->SpyEffect_VehicleVeterancy_Types, pTypeExt->SpyEffect_VehicleVeterancy_Ignore,
					AffectedTechnoType::Units, pTypeExt->SpyEffect_VehicleVeterancy_Cumulative);
		}
		else if (pTypeExt->SpyEffect_VehicleVeterancy != 0)
		{
			if (pTypeExt->SpyEffect_Veterancy_AffectBuildingOwner)
				HouseExt::TechnoUpgrade(pVictimHouse, pTypeExt->SpyEffect_VehicleVeterancy,
					pTypeExt->SpyEffect_VehicleVeterancy_Types, pTypeExt->SpyEffect_VehicleVeterancy_Ignore,
					AffectedTechnoType::Units, pTypeExt->SpyEffect_VehicleVeterancy_Cumulative);
			else
				HouseExt::TechnoUpgrade(pInfiltratorHouse, pTypeExt->SpyEffect_VehicleVeterancy,
					pTypeExt->SpyEffect_VehicleVeterancy_Types, pTypeExt->SpyEffect_VehicleVeterancy_Ignore,
					AffectedTechnoType::Units, pTypeExt->SpyEffect_VehicleVeterancy_Cumulative);
		}

		if (pTypeExt->SpyEffect_NavalVeterancy_Reset)
		{
			if (pTypeExt->SpyEffect_Veterancy_AffectBuildingOwner)
				HouseExt::TechnoUpgrade(pVictimHouse, 0,
					pTypeExt->SpyEffect_NavalVeterancy_Types, pTypeExt->SpyEffect_NavalVeterancy_Ignore,
					AffectedTechnoType::Naval, pTypeExt->SpyEffect_InfantryVeterancy_Cumulative);
			else
				HouseExt::TechnoUpgrade(pInfiltratorHouse, 0,
					pTypeExt->SpyEffect_NavalVeterancy_Types, pTypeExt->SpyEffect_NavalVeterancy_Ignore,
					AffectedTechnoType::Naval, pTypeExt->SpyEffect_InfantryVeterancy_Cumulative);
		}
		else if (pTypeExt->SpyEffect_NavalVeterancy != 0)
		{
			if (pTypeExt->SpyEffect_Veterancy_AffectBuildingOwner)
				HouseExt::TechnoUpgrade(pVictimHouse, pTypeExt->SpyEffect_NavalVeterancy,
					pTypeExt->SpyEffect_NavalVeterancy_Types, pTypeExt->SpyEffect_NavalVeterancy_Ignore,
					AffectedTechnoType::Naval, pTypeExt->SpyEffect_InfantryVeterancy_Cumulative);
			else
				HouseExt::TechnoUpgrade(pInfiltratorHouse, pTypeExt->SpyEffect_NavalVeterancy,
					pTypeExt->SpyEffect_NavalVeterancy_Types, pTypeExt->SpyEffect_NavalVeterancy_Ignore,
					AffectedTechnoType::Naval, pTypeExt->SpyEffect_InfantryVeterancy_Cumulative);
		}

		if (pTypeExt->SpyEffect_AircraftVeterancy_Reset)
		{
			if (pTypeExt->SpyEffect_Veterancy_AffectBuildingOwner)
				HouseExt::TechnoUpgrade(pVictimHouse, 0,
					pTypeExt->SpyEffect_AircraftVeterancy_Types, pTypeExt->SpyEffect_AircraftVeterancy_Ignore,
					AffectedTechnoType::Aircraft, pTypeExt->SpyEffect_AircraftVeterancy_Cumulative);
			else
				HouseExt::TechnoUpgrade(pInfiltratorHouse, 0,
					pTypeExt->SpyEffect_AircraftVeterancy_Types, pTypeExt->SpyEffect_AircraftVeterancy_Ignore,
					AffectedTechnoType::Aircraft, pTypeExt->SpyEffect_AircraftVeterancy_Cumulative);
		}
		else if (pTypeExt->SpyEffect_AircraftVeterancy != 0)
		{
			if (pTypeExt->SpyEffect_Veterancy_AffectBuildingOwner)
				HouseExt::TechnoUpgrade(pVictimHouse, pTypeExt->SpyEffect_AircraftVeterancy,
					pTypeExt->SpyEffect_AircraftVeterancy_Types, pTypeExt->SpyEffect_AircraftVeterancy_Ignore,
					AffectedTechnoType::Aircraft, pTypeExt->SpyEffect_AircraftVeterancy_Cumulative);
			else
				HouseExt::TechnoUpgrade(pInfiltratorHouse, pTypeExt->SpyEffect_AircraftVeterancy,
					pTypeExt->SpyEffect_AircraftVeterancy_Types, pTypeExt->SpyEffect_AircraftVeterancy_Ignore,
					AffectedTechnoType::Aircraft, pTypeExt->SpyEffect_AircraftVeterancy_Cumulative);
		}

		if (pTypeExt->SpyEffect_BuildingVeterancy_Reset)
		{
			if (pTypeExt->SpyEffect_Veterancy_AffectBuildingOwner)
				HouseExt::TechnoUpgrade(pVictimHouse, 0,
					pTypeExt->SpyEffect_BuildingVeterancy_Types, pTypeExt->SpyEffect_BuildingVeterancy_Ignore,
					AffectedTechnoType::Buildings, pTypeExt->SpyEffect_BuildingVeterancy_Cumulative);
			else
				HouseExt::TechnoUpgrade(pInfiltratorHouse, 0,
					pTypeExt->SpyEffect_BuildingVeterancy_Types, pTypeExt->SpyEffect_BuildingVeterancy_Ignore,
					AffectedTechnoType::Buildings, pTypeExt->SpyEffect_BuildingVeterancy_Cumulative);
		}
		else if (pTypeExt->SpyEffect_BuildingVeterancy != 0)
		{
			if (pTypeExt->SpyEffect_Veterancy_AffectBuildingOwner)
				HouseExt::TechnoUpgrade(pVictimHouse, pTypeExt->SpyEffect_BuildingVeterancy,
					pTypeExt->SpyEffect_BuildingVeterancy_Types, pTypeExt->SpyEffect_BuildingVeterancy_Ignore,
					AffectedTechnoType::Buildings, pTypeExt->SpyEffect_BuildingVeterancy_Cumulative);
			else
				HouseExt::TechnoUpgrade(pInfiltratorHouse, pTypeExt->SpyEffect_BuildingVeterancy,
					pTypeExt->SpyEffect_BuildingVeterancy_Types, pTypeExt->SpyEffect_BuildingVeterancy_Ignore,
					AffectedTechnoType::Buildings, pTypeExt->SpyEffect_BuildingVeterancy_Cumulative);
		}

		if (!pTypeExt->SpyEffect_SuperWeaponTypes.empty())
		{
			for (size_t i = 0; i < pTypeExt->SpyEffect_SuperWeaponTypes.size(); i++)
			{
				bool inhouseext = false;
				bool inhousedelay = false;
				for (size_t j = 0; j < pInfiltratorExt->SpySuperWeaponTypes.size(); j++)
				{
					if (pInfiltratorExt->SpySuperWeaponTypes[j] == pTypeExt->SpyEffect_SuperWeaponTypes[i])
					{
						inhouseext = true;

						int delay;
						if (pInfiltratorExt->SpySuperWeaponDelay.size() >= j)
							delay = pInfiltratorExt->SpySuperWeaponDelay[j];
						else
							delay = 0;

						if (delay > 0)
							inhousedelay = true;
						else
							pInfiltratorExt->SpySuperWeaponDelay[j] = pTypeExt->SpyEffect_SuperWeaponTypes_Delay.size() > i ?
							pTypeExt->SpyEffect_SuperWeaponTypes_Delay[i] : 0;
						break;
					}
				}

				if (inhousedelay)
					continue;

				SuperClass* pSuper = pInfiltratorHouse->Supers[pTypeExt->SpyEffect_SuperWeaponTypes[i]];
				if (pTypeExt->SpyEffect_SuperWeaponTypes_Permanent[i])
					pInfiltratorExt->SW_Permanents[pSuper->Type->ArrayIndex] = true;

				if (pSuper->Granted && pTypeExt->SpyEffect_SuperWeaponTypes_AffectOwned[i])
					pSuper->RechargeTimer.TimeLeft -= pSuper->RechargeTimer.TimeLeft;
				else
				{
					bool granted;
					granted = pSuper->Grant(true, true, false);
					if (granted)
					{
						if (MouseClass::Instance->AddCameo(AbstractType::Special, pTypeExt->SpyEffect_SuperWeaponTypes[i]))
							MouseClass::Instance->RepaintSidebar(1);
					}
				}

				if (!inhouseext)
				{
					pInfiltratorExt->SpySuperWeaponTypes.emplace_back(pTypeExt->SpyEffect_SuperWeaponTypes[i]);
					int dealy;
					if (pTypeExt->SpyEffect_SuperWeaponTypes_Delay.size() > i)
						dealy = pTypeExt->SpyEffect_SuperWeaponTypes_Delay[i];
					else
						dealy = 0;
					pInfiltratorExt->SpySuperWeaponDelay.emplace_back(dealy);
				}
			}
		}

		if (!pTypeExt->SpyEffect_RechargeSuperWeaponTypes.empty())
		{
			for (size_t i = 0; i < pTypeExt->SpyEffect_RechargeSuperWeaponTypes.size(); i++)
			{
				SuperClass* pSuper = pVictimHouse->Supers[pTypeExt->SpyEffect_RechargeSuperWeaponTypes[i]];
				int time;
				if (pTypeExt->SpyEffect_RechargeSuperWeaponTypes_Duration.size() > i)
					time = static_cast<int>(pTypeExt->SpyEffect_RechargeSuperWeaponTypes_Duration[i]);
				else
					time = 0;

				if (abs(time) <= 1)
					time = static_cast<int>(pSuper->Type->RechargeTime * time);

				if (pTypeExt->SpyEffect_RechargeSuperWeaponTypes_SetPercentage.size() > i)
				{
					if (pTypeExt->SpyEffect_RechargeSuperWeaponTypes_SetPercentage[i] > 0)
						pSuper->RechargeTimer.TimeLeft = Game::F2I(pSuper->Type->RechargeTime * pTypeExt->SpyEffect_RechargeSuperWeaponTypes_SetPercentage[i]);
				}
				pSuper->RechargeTimer.TimeLeft += time;

				if (SWTypeExt::ExtMap.Find(pSuper->Type)->SW_Cumulative)
				{
					int count;
					if (pTypeExt->SpyEffect_RechargeSuperWeaponTypes_CumulativeCount.size() > i)
						count = pTypeExt->SpyEffect_RechargeSuperWeaponTypes_CumulativeCount[i];
					else
						count = 0;

					if (pVictimExt->SuperWeaponCumulativeCount[i] - count >= 0)
						pVictimExt->SuperWeaponCumulativeCount[i] -= count;
					else
						pVictimExt->SuperWeaponCumulativeCount[i] = 0;
				}
			}
		}

		if (pTypeExt->SpyEffect_RevealSightDuration != 0)
		{
			int range = pTypeExt->SpyEffect_RevealSightRange != 0 ? pTypeExt->SpyEffect_RevealSightRange : pBuilding->Type->Sight;

			if (pTypeExt->SpyEffect_RevealSightDuration < 0)
			{
				CoordStruct coords = pBuilding->GetCenterCoords();

				if (range < 0)
					MapClass::Instance->Reveal(pInfiltratorHouse);
				else
					MapClass::Instance->RevealArea1(&coords, range, pInfiltratorHouse, CellStruct::Empty, 0, 0, 0, 1);

			}
			else
			{
				bool hasreveal = false;
				for (size_t i = 0; i < pExt->RevealSightHouses.size(); i++)
				{
					if (pExt->RevealSightHouses[i] == pInfiltratorHouse)
					{
						pExt->RevealSightRanges[i] = range;
						pExt->RevealSightPermanents[i] = pTypeExt->SpyEffect_RevealSightPermanent;
						pExt->RevealSightTimers[i].Start(pTypeExt->SpyEffect_RevealSightDuration);
						hasreveal = true;
						break;
					}
				}

				if (!hasreveal)
				{
					pExt->RevealSightHouses.emplace_back(pInfiltratorHouse);
					pExt->RevealSightRanges.emplace_back(range);
					pExt->RevealSightPermanents.emplace_back(pTypeExt->SpyEffect_RevealSightPermanent);

					CDTimerClass timer;
					timer.Start(pTypeExt->SpyEffect_RevealSightDuration);
					pExt->RevealSightTimers.emplace_back(timer);
				}
			}
		}

		if (pTypeExt->SpyEffect_RadarJamDuration != 0)
		{
			if (pTypeExt->SpyEffect_RadarJamDuration > 0)
			{
				pVictimHouse->RadarBlackoutTimer.Start(pTypeExt->SpyEffect_RadarJamDuration);
			}
			else
			{
				pInfiltratorHouse->RadarBlackoutTimer.Start(abs(pTypeExt->SpyEffect_RadarJamDuration));
			}
		}

		if (pTypeExt->SpyEffect_PowerOutageDuration != 0)
		{
			if (pTypeExt->SpyEffect_PowerOutageDuration > 0)
			{
				pVictimHouse->PowerBlackoutTimer.Start(pTypeExt->SpyEffect_PowerOutageDuration);
			}
			else
			{
				pInfiltratorHouse->PowerBlackoutTimer.Start(abs(pTypeExt->SpyEffect_PowerOutageDuration));
			}
		}

		if (pTypeExt->SpyEffect_GapRadarDuration != 0)
		{
			if (pTypeExt->SpyEffect_GapRadarDuration > 0)
			{
				pVictimHouse->ReshroudMap();
				pVictimExt->GapRadarTimer.Start(pTypeExt->SpyEffect_GapRadarDuration);
			}
			else
			{
				pInfiltratorHouse->ReshroudMap();
				pInfiltratorExt->GapRadarTimer.Start(abs(pTypeExt->SpyEffect_GapRadarDuration));
			}
		}

		if (pTypeExt->SpyEffect_RevealRadarSightDuration != 0)
		{
			if (pTypeExt->SpyEffect_RevealRadarSightDuration > 0)
			{
				bool hasbuilding = false;
				for (size_t i = 0; i < pVictimExt->RevealRadarSightBuildings.size(); i++)
				{
					if (pVictimExt->RevealRadarSightBuildings[i] == pBuilding)
					{
						hasbuilding = true;
						pVictimExt->KeepRevealRadarSights[i] = pTypeExt->SpyEffect_KeepRevealRadarSight;
						pVictimExt->RevealRadarSightBuildingOwners[i] = pVictimHouse;
						pVictimExt->RevealRadarSightOwners[i] = pInfiltratorHouse;
						pVictimExt->RevealRadarSightPermanents[i] = pTypeExt->SpyEffect_RevealRadarSightPermanent;
						pVictimExt->RevealRadarSights_Aircraft[i] = pTypeExt->SpyEffect_RevealRadarSight_Aircraft;
						pVictimExt->RevealRadarSights_Building[i] = pTypeExt->SpyEffect_RevealRadarSight_Building;
						pVictimExt->RevealRadarSights_Infantry[i] = pTypeExt->SpyEffect_RevealRadarSight_Infantry;
						pVictimExt->RevealRadarSights_Unit[i] = pTypeExt->SpyEffect_RevealRadarSight_Unit;
						pVictimExt->RevealRadarSightTimers[i].Start(pTypeExt->SpyEffect_RevealRadarSightDuration);
						break;
					}
				}

				if (!hasbuilding)
				{
					pVictimExt->KeepRevealRadarSights.emplace_back(pTypeExt->SpyEffect_KeepRevealRadarSight);
					pVictimExt->RevealRadarSightBuildingOwners.emplace_back(pVictimHouse);
					pVictimExt->RevealRadarSightBuildings.emplace_back(pBuilding);
					pVictimExt->RevealRadarSightOwners.emplace_back(pInfiltratorHouse);
					pVictimExt->RevealRadarSightPermanents.emplace_back(pTypeExt->SpyEffect_RevealRadarSightPermanent);
					pVictimExt->RevealRadarSights_Aircraft.emplace_back(pTypeExt->SpyEffect_RevealRadarSight_Aircraft);
					pVictimExt->RevealRadarSights_Building.emplace_back(pTypeExt->SpyEffect_RevealRadarSight_Building);
					pVictimExt->RevealRadarSights_Infantry.emplace_back(pTypeExt->SpyEffect_RevealRadarSight_Infantry);
					pVictimExt->RevealRadarSights_Unit.emplace_back(pTypeExt->SpyEffect_RevealRadarSight_Unit);

					CDTimerClass timer;
					timer.Start(pTypeExt->SpyEffect_RevealRadarSightDuration);
					pVictimExt->RevealRadarSightTimers.emplace_back(timer);
				}
			}
			else
			{
				bool hasbuilding = false;
				for (size_t i = 0; i < pInfiltratorExt->RevealRadarSightBuildings.size(); i++)
				{
					if (pInfiltratorExt->RevealRadarSightBuildings[i] == pBuilding)
					{
						hasbuilding = true;
						pInfiltratorExt->KeepRevealRadarSights[i] = pTypeExt->SpyEffect_KeepRevealRadarSight;
						pInfiltratorExt->RevealRadarSightBuildingOwners[i] = pVictimHouse;
						pInfiltratorExt->RevealRadarSightOwners[i] = pVictimHouse;
						pInfiltratorExt->RevealRadarSightPermanents[i] = pTypeExt->SpyEffect_RevealRadarSightPermanent;
						pInfiltratorExt->RevealRadarSights_Aircraft[i] = pTypeExt->SpyEffect_RevealRadarSight_Aircraft;
						pInfiltratorExt->RevealRadarSights_Building[i] = pTypeExt->SpyEffect_RevealRadarSight_Building;
						pInfiltratorExt->RevealRadarSights_Infantry[i] = pTypeExt->SpyEffect_RevealRadarSight_Infantry;
						pInfiltratorExt->RevealRadarSights_Unit[i] = pTypeExt->SpyEffect_RevealRadarSight_Unit;
						pInfiltratorExt->RevealRadarSightTimers[i].Start(abs(pTypeExt->SpyEffect_RevealRadarSightDuration));
						break;
					}
				}

				if (!hasbuilding)
				{
					pInfiltratorExt->KeepRevealRadarSights.emplace_back(pTypeExt->SpyEffect_KeepRevealRadarSight);
					pInfiltratorExt->RevealRadarSightBuildingOwners.emplace_back(pVictimHouse);
					pInfiltratorExt->RevealRadarSightBuildings.emplace_back(pBuilding);
					pInfiltratorExt->RevealRadarSightOwners.emplace_back(pVictimHouse);
					pInfiltratorExt->RevealRadarSightPermanents.emplace_back(pTypeExt->SpyEffect_RevealRadarSightPermanent);
					pInfiltratorExt->RevealRadarSights_Aircraft.emplace_back(pTypeExt->SpyEffect_RevealRadarSight_Aircraft);
					pInfiltratorExt->RevealRadarSights_Building.emplace_back(pTypeExt->SpyEffect_RevealRadarSight_Building);
					pInfiltratorExt->RevealRadarSights_Infantry.emplace_back(pTypeExt->SpyEffect_RevealRadarSight_Infantry);
					pInfiltratorExt->RevealRadarSights_Unit.emplace_back(pTypeExt->SpyEffect_RevealRadarSight_Unit);

					CDTimerClass timer;
					timer.Start(abs(pTypeExt->SpyEffect_RevealRadarSightDuration));
					pInfiltratorExt->RevealRadarSightTimers.emplace_back(timer);
				}
			}
		}

		if (pTypeExt->SpyEffect_InfantryCostBonus_Reset)
		{
			if (pTypeExt->SpyEffect_CostBonus_AffectBuildingOwner)
				HouseExt::TechnoCostBonusReset(pVictimHouse,
					pTypeExt->SpyEffect_InfantryCostBonus_Types,
					pTypeExt->SpyEffect_InfantryCostBonus_Ignore,
					AffectedTechnoType::Infantry);
			else
				HouseExt::TechnoCostBonusReset(pInfiltratorHouse,
					pTypeExt->SpyEffect_InfantryCostBonus_Types,
					pTypeExt->SpyEffect_InfantryCostBonus_Ignore,
					AffectedTechnoType::Infantry);
		}
		else if (pTypeExt->SpyEffect_InfantryCostBonus != 1.0)
		{
			if (pTypeExt->SpyEffect_CostBonus_AffectBuildingOwner)
				HouseExt::TechnoCostBonus(pVictimHouse,
					pTypeExt->SpyEffect_InfantryCostBonus,
					pTypeExt->SpyEffect_InfantryCostBonus_Types,
					pTypeExt->SpyEffect_InfantryCostBonus_Ignore,
					AffectedTechnoType::Infantry,
					pTypeExt->SpyEffect_InfantryCostBonus_Max,
					pTypeExt->SpyEffect_InfantryCostBonus_Min);
			else
				HouseExt::TechnoCostBonus(pInfiltratorHouse,
					pTypeExt->SpyEffect_InfantryCostBonus,
					pTypeExt->SpyEffect_InfantryCostBonus_Types,
					pTypeExt->SpyEffect_InfantryCostBonus_Ignore,
					AffectedTechnoType::Infantry,
					pTypeExt->SpyEffect_InfantryCostBonus_Max,
					pTypeExt->SpyEffect_InfantryCostBonus_Min);
		}

		if (pTypeExt->SpyEffect_UnitsCostBonus_Reset)
		{
			if (pTypeExt->SpyEffect_CostBonus_AffectBuildingOwner)
				HouseExt::TechnoCostBonusReset(pVictimHouse,
					pTypeExt->SpyEffect_UnitsCostBonus_Types,
					pTypeExt->SpyEffect_UnitsCostBonus_Ignore,
					AffectedTechnoType::Units);
			else
				HouseExt::TechnoCostBonusReset(pInfiltratorHouse,
					pTypeExt->SpyEffect_UnitsCostBonus_Types,
					pTypeExt->SpyEffect_UnitsCostBonus_Ignore,
					AffectedTechnoType::Units);
		}
		else if (pTypeExt->SpyEffect_UnitsCostBonus != 1.0)
		{
			if (pTypeExt->SpyEffect_CostBonus_AffectBuildingOwner)
				HouseExt::TechnoCostBonus(pVictimHouse,
					pTypeExt->SpyEffect_UnitsCostBonus,
					pTypeExt->SpyEffect_UnitsCostBonus_Types,
					pTypeExt->SpyEffect_UnitsCostBonus_Ignore,
					AffectedTechnoType::Units,
					pTypeExt->SpyEffect_UnitsCostBonus_Max,
					pTypeExt->SpyEffect_UnitsCostBonus_Min);
			else
				HouseExt::TechnoCostBonus(pInfiltratorHouse,
					pTypeExt->SpyEffect_UnitsCostBonus,
					pTypeExt->SpyEffect_UnitsCostBonus_Types,
					pTypeExt->SpyEffect_UnitsCostBonus_Ignore,
					AffectedTechnoType::Units,
					pTypeExt->SpyEffect_UnitsCostBonus_Max,
					pTypeExt->SpyEffect_UnitsCostBonus_Min);
		}

		if (pTypeExt->SpyEffect_NavalCostBonus_Reset)
		{
			if (pTypeExt->SpyEffect_CostBonus_AffectBuildingOwner)
				HouseExt::TechnoCostBonusReset(pVictimHouse,
					pTypeExt->SpyEffect_NavalCostBonus_Types,
					pTypeExt->SpyEffect_NavalCostBonus_Ignore,
					AffectedTechnoType::Naval);
			else
				HouseExt::TechnoCostBonusReset(pInfiltratorHouse,
					pTypeExt->SpyEffect_NavalCostBonus_Types,
					pTypeExt->SpyEffect_NavalCostBonus_Ignore,
					AffectedTechnoType::Naval);
		}
		else if (pTypeExt->SpyEffect_NavalCostBonus != 1.0)
		{
			if (pTypeExt->SpyEffect_CostBonus_AffectBuildingOwner)
				HouseExt::TechnoCostBonus(pVictimHouse,
					pTypeExt->SpyEffect_NavalCostBonus,
					pTypeExt->SpyEffect_NavalCostBonus_Types,
					pTypeExt->SpyEffect_NavalCostBonus_Ignore,
					AffectedTechnoType::Naval,
					pTypeExt->SpyEffect_NavalCostBonus_Max,
					pTypeExt->SpyEffect_NavalCostBonus_Min);
			else
				HouseExt::TechnoCostBonus(pInfiltratorHouse,
					pTypeExt->SpyEffect_NavalCostBonus,
					pTypeExt->SpyEffect_NavalCostBonus_Types,
					pTypeExt->SpyEffect_NavalCostBonus_Ignore,
					AffectedTechnoType::Naval,
					pTypeExt->SpyEffect_NavalCostBonus_Max,
					pTypeExt->SpyEffect_NavalCostBonus_Min);
		}

		if (pTypeExt->SpyEffect_AircraftCostBonus_Reset)
		{
			if (pTypeExt->SpyEffect_CostBonus_AffectBuildingOwner)
				HouseExt::TechnoCostBonusReset(pVictimHouse,
					pTypeExt->SpyEffect_AircraftCostBonus_Types,
					pTypeExt->SpyEffect_AircraftCostBonus_Ignore,
					AffectedTechnoType::Aircraft);
			else
				HouseExt::TechnoCostBonusReset(pInfiltratorHouse,
					pTypeExt->SpyEffect_AircraftCostBonus_Types,
					pTypeExt->SpyEffect_AircraftCostBonus_Ignore,
					AffectedTechnoType::Aircraft);
		}
		else if (pTypeExt->SpyEffect_AircraftCostBonus != 1.0)
		{
			if (pTypeExt->SpyEffect_CostBonus_AffectBuildingOwner)
				HouseExt::TechnoCostBonus(pVictimHouse,
					pTypeExt->SpyEffect_AircraftCostBonus,
					pTypeExt->SpyEffect_AircraftCostBonus_Types,
					pTypeExt->SpyEffect_AircraftCostBonus_Ignore,
					AffectedTechnoType::Aircraft,
					pTypeExt->SpyEffect_AircraftCostBonus_Max,
					pTypeExt->SpyEffect_AircraftCostBonus_Min);
			else
				HouseExt::TechnoCostBonus(pInfiltratorHouse,
					pTypeExt->SpyEffect_AircraftCostBonus,
					pTypeExt->SpyEffect_AircraftCostBonus_Types,
					pTypeExt->SpyEffect_AircraftCostBonus_Ignore,
					AffectedTechnoType::Aircraft,
					pTypeExt->SpyEffect_AircraftCostBonus_Max,
					pTypeExt->SpyEffect_AircraftCostBonus_Min);
		}

		if (pTypeExt->SpyEffect_BuildingsCostBonus_Reset)
		{
			if (pTypeExt->SpyEffect_CostBonus_AffectBuildingOwner)
				HouseExt::TechnoCostBonusReset(pVictimHouse,
					pTypeExt->SpyEffect_BuildingsCostBonus_Types,
					pTypeExt->SpyEffect_BuildingsCostBonus_Ignore,
					AffectedTechnoType::Buildings);
			else
				HouseExt::TechnoCostBonusReset(pInfiltratorHouse,
					pTypeExt->SpyEffect_BuildingsCostBonus_Types,
					pTypeExt->SpyEffect_BuildingsCostBonus_Ignore,
					AffectedTechnoType::Buildings);
		}
		else if (pTypeExt->SpyEffect_BuildingsCostBonus != 1.0)
		{
			if (pTypeExt->SpyEffect_CostBonus_AffectBuildingOwner)
				HouseExt::TechnoCostBonus(pVictimHouse,
					pTypeExt->SpyEffect_BuildingsCostBonus,
					pTypeExt->SpyEffect_BuildingsCostBonus_Types,
					pTypeExt->SpyEffect_BuildingsCostBonus_Ignore,
					AffectedTechnoType::Buildings,
					pTypeExt->SpyEffect_BuildingsCostBonus_Max,
					pTypeExt->SpyEffect_BuildingsCostBonus_Min);
			else
				HouseExt::TechnoCostBonus(pInfiltratorHouse,
					pTypeExt->SpyEffect_BuildingsCostBonus,
					pTypeExt->SpyEffect_BuildingsCostBonus_Types,
					pTypeExt->SpyEffect_BuildingsCostBonus_Ignore,
					AffectedTechnoType::Buildings,
					pTypeExt->SpyEffect_BuildingsCostBonus_Max,
					pTypeExt->SpyEffect_BuildingsCostBonus_Min);
		}

		if (pTypeExt->SpyEffect_DefensesCostBonus_Reset)
		{
			if (pTypeExt->SpyEffect_CostBonus_AffectBuildingOwner)
				HouseExt::TechnoCostBonusReset(pVictimHouse,
					pTypeExt->SpyEffect_DefensesCostBonus_Types,
					pTypeExt->SpyEffect_DefensesCostBonus_Ignore,
					AffectedTechnoType::Defenses);
			else
				HouseExt::TechnoCostBonusReset(pInfiltratorHouse,
					pTypeExt->SpyEffect_DefensesCostBonus_Types,
					pTypeExt->SpyEffect_DefensesCostBonus_Ignore,
					AffectedTechnoType::Defenses);
		}
		else if (pTypeExt->SpyEffect_DefensesCostBonus != 1.0)
		{
			if (pTypeExt->SpyEffect_CostBonus_AffectBuildingOwner)
				HouseExt::TechnoCostBonus(pVictimHouse,
					pTypeExt->SpyEffect_DefensesCostBonus,
					pTypeExt->SpyEffect_DefensesCostBonus_Types,
					pTypeExt->SpyEffect_DefensesCostBonus_Ignore,
					AffectedTechnoType::Defenses,
					pTypeExt->SpyEffect_DefensesCostBonus_Max,
					pTypeExt->SpyEffect_DefensesCostBonus_Min);
			else
				HouseExt::TechnoCostBonus(pInfiltratorHouse,
					pTypeExt->SpyEffect_DefensesCostBonus,
					pTypeExt->SpyEffect_DefensesCostBonus_Types,
					pTypeExt->SpyEffect_DefensesCostBonus_Ignore,
					AffectedTechnoType::Defenses,
					pTypeExt->SpyEffect_DefensesCostBonus_Max,
					pTypeExt->SpyEffect_DefensesCostBonus_Min);
		}

		if (pTypeExt->SpyEffect_PurifierBonus_Reset)
		{
			if (pTypeExt->SpyEffect_PurifierBonus_AffectBuildingOwner)
				pVictimExt->OrePurifierBonus = 0.0;
			else
				pInfiltratorExt->OrePurifierBonus = 0.0;
		}
		else if (pTypeExt->SpyEffect_PurifierBonus != 0.0)
		{
			if (pTypeExt->SpyEffect_PurifierBonus_AffectBuildingOwner)
				pVictimExt->OrePurifierBonus += pTypeExt->SpyEffect_PurifierBonus;
			else
				pInfiltratorExt->OrePurifierBonus += pTypeExt->SpyEffect_PurifierBonus;
		}

		if (pTypeExt->SpyEffect_Anim && pTypeExt->SpyEffect_Anim_Duration != 0)
		{
			pExt->SpyEffectAnim = GameCreate<AnimClass>(pTypeExt->SpyEffect_Anim, pBuilding->GetCoords());
			pExt->SpyEffectAnim->SetOwnerObject(pBuilding);
			pExt->SpyEffectAnim->RemainingIterations = 0xFFU;
			pExt->SpyEffectAnim->Owner = pVictimHouse;

			pExt->SpyEffectAnimDuration = pTypeExt->SpyEffect_Anim_Duration;
			pExt->SpyEffectAnimDisplayHouses = pTypeExt->SpyEffect_Anim_DisplayHouses;
		}

		if (pTypeExt->SpyEffect_Messages.size() > 0)
		{
			for (size_t i = 0; i < pTypeExt->SpyEffect_Messages.size(); i++)
			{
				if (pTypeExt->SpyEffect_Message_ShowOwners[i] == ShowMessageHouse::All ||
					(pTypeExt->SpyEffect_Message_ShowOwners[i] == ShowMessageHouse::Invoker && HouseClass::CurrentPlayer == pInfiltratorHouse) ||
					(pTypeExt->SpyEffect_Message_ShowOwners[i] == ShowMessageHouse::Victim && HouseClass::CurrentPlayer == pVictimHouse))
				{
					MessageListClass::Instance->PrintMessage(pTypeExt->SpyEffect_Messages[i].Text, RulesClass::Instance->MessageDelay, HouseClass::CurrentPlayer->ColorSchemeIndex);
				}
			}
		}

		if (pTypeExt->SpyEffect_CaptureDelay != 0)
		{
			if (!pExt->CaptureTimer.HasStarted())
			{
				if (pTypeExt->SpyEffect_CaptureDelay > 0)
				{
					pExt->CaptureTimer.Start(pTypeExt->SpyEffect_CaptureDelay);
					pExt->CaptureOwner = pInfiltratorHouse;
					if (pTypeExt->SpyEffect_CaptureCount > 0)
					{
						pExt->OriginalOwner = pVictimHouse;
						pExt->CaptureCount = pTypeExt->SpyEffect_CaptureCount;
					}
				}
				else
				{
					pExt->CaptureTimer.Start(static_cast<int>(RulesClass::Instance->C4Delay));
					pExt->CaptureOwner = pInfiltratorHouse;
					if (pTypeExt->SpyEffect_CaptureCount > 0)
					{
						pExt->OriginalOwner = pVictimHouse;
						pExt->CaptureCount = pTypeExt->SpyEffect_CaptureCount;
					}
				}
			}
		}

		if (pTypeExt->SpyEffect_SabotageDelay != 0)
		{
			if (!pExt->SabotageTimer.HasStarted())
			{
				if (pTypeExt->SpyEffect_SabotageDelay > 0)
					pExt->SabotageTimer.Start(pTypeExt->SpyEffect_SabotageDelay);
				else
					pExt->SabotageTimer.Start(static_cast<int>(RulesClass::Instance->C4Delay));
			}
		}

		if (pTypeExt->SpyEffect_SellDelay != 0 && pBuilding->Type->LoadBuildup())
		{
			if (!pExt->SellTimer.HasStarted())
			{
				if (pTypeExt->SpyEffect_SellDelay > 0)
					pExt->SellTimer.Start(pTypeExt->SpyEffect_SellDelay);
				else
					pExt->SellTimer.Start(static_cast<int>(RulesClass::Instance->C4Delay));
			}
		}
	}

	return true;
}

void BuildingExt::ExtData::BuildingPowered()
{
	auto const pThis = this->OwnerObject();

	if (this->OfflineTimer > 0)
	{
		if (pThis->IsPowerOnline())
			pThis->GoOffline();

		this->OfflineTimer--;
	}
	else if(this->OfflineTimer == 0)
	{
		if (!pThis->IsPowerOnline())
			pThis->GoOnline();

		this->OfflineTimer--;
	}
}

void BuildingExt::ExtData::CaptureBuilding()
{
	auto const pThis = this->OwnerObject();
	if (this->CaptureTimer.Completed() && this->CaptureCount >= 0)
	{
		pThis->SetOwningHouse(this->CaptureOwner);
		this->CaptureTimer.Stop();
		if (this->CaptureCount > 0)
		{
			this->CaptureTimer.Start(this->CaptureCount);
			this->CaptureCount = -1;
			this->SellingForbidden = true;
		}
		else
			this->CaptureOwner = nullptr;
	}
	else if (this->CaptureTimer.HasStarted() && this->CaptureCount < 0)
	{
		if (this->CaptureTimer.Completed())
		{
			this->CaptureTimer.Stop();
			auto owner = this->OriginalOwner->Defeated ? HouseClass::FindCivilianSide() : this->OriginalOwner;
			pThis->SetOwningHouse(owner, false);
			this->OriginalOwner = nullptr;
			this->CaptureOwner = nullptr;
			this->CaptureCount = 0;
			this->SellingForbidden = false;
		}
		else if (this->CaptureOwner != pThis->Owner)
		{
			this->CaptureTimer.Resume();
			this->CaptureTimer.Stop();
			this->OriginalOwner = nullptr;
			this->CaptureOwner = nullptr;
			this->CaptureCount = 0;
			this->SellingForbidden = false;
		}
	}
}

void BuildingExt::ExtData::ForbidSell()
{
	auto const pThis = this->OwnerObject();
	if (pThis->GetCurrentMission() == Mission::Selling)
	{
		if (this->SellingForbidden)
		{
			pThis->ForceMission(Mission::Stop);
			pThis->Guard();
		}
		else
		{
			TechnoExt::ExtMap.Find(pThis)->MoneyReturn_Sold = true;

			if (!(pThis->Type->UndeploysInto != nullptr && pThis->Focus != nullptr))
			{
				if (!this->SellWeaponDetonated && this->TypeExtData->SellWeapon)
				{
					WeaponTypeExt::DetonateAt(this->TypeExtData->SellWeapon, pThis, pThis);
					this->SellWeaponDetonated = true;
				}
			}
		}
	}
}

void BuildingExt::ExtData::SabotageBuilding()
{
	auto const pThis = this->OwnerObject();
	if (this->SabotageTimer.Completed())
		TechnoExt::KillSelf(pThis, AutoDeathBehavior::Kill);
}

void BuildingExt::ExtData::SellBuilding()
{
	auto const pThis = this->OwnerObject();
	if (this->SellTimer.Completed())
	{
		this->SellingForbidden = false;
		TechnoExt::KillSelf(pThis, AutoDeathBehavior::Sell);
	}
}

void BuildingExt::ExtData::RevealSight()
{
	auto const pThis = this->OwnerObject();
	for (size_t i = 0; i < RevealSightHouses.size(); i++)
	{
		if (RevealSightHouses[i]->Defeated ||
			(!RevealSightTimers[i].InProgress() && !RevealSightPermanents[i]))
		{
			RevealSightHouses.erase(RevealSightHouses.begin() + i);
			RevealSightRanges.erase(RevealSightRanges.begin() + i);
			RevealSightTimers.erase(RevealSightTimers.begin() + i);
		}
		else
		{
			CoordStruct coords = pThis->GetCenterCoords();
			if (RevealSightRanges[i] < 0)
				MapClass::Instance->Reveal(RevealSightHouses[i]);
			else
				MapClass::Instance->RevealArea1(&coords, RevealSightRanges[i], RevealSightHouses[i], CellStruct::Empty, 0, 0, 0, 1);
		}
	}
}

void BuildingExt::ExtData::SpyEffectAnimCheck()
{
	if (SpyEffectAnim)
	{
		if (SpyEffectAnim->Owner != OwnerObject()->Owner)
			SpyEffectAnim->Owner = OwnerObject()->Owner;

		if (EnumFunctions::CanTargetHouse(SpyEffectAnimDisplayHouses, SpyEffectAnim->Owner, HouseClass::CurrentPlayer))
		{
			if (SpyEffectAnim->Invisible)
				SpyEffectAnim->Invisible = false;
		}
		else
		{
			if (!SpyEffectAnim->Invisible)
				SpyEffectAnim->Invisible = true;
		}

		if (SpyEffectAnimDuration > 0)
			SpyEffectAnimDuration--;
		else if (SpyEffectAnimDuration == 0)
		{
			SpyEffectAnim->UnInit();
			SpyEffectAnim = nullptr;
			SpyEffectAnimDisplayHouses = AffectedHouse::All;
		}
	}
}

void BuildingExt::ExtData::AutoRepairCheck()
{
	if (!Phobos::AutoRepair)
		return;

	auto pBid = OwnerObject();

	if (!pBid->Owner->IsCurrentPlayer() || !pBid->Type->ClickRepairable)
		return;

	if (pBid->GetHealthPercentage() != 1 && !pBid->IsBeingRepaired)
	{
		pBid->IsBeingRepaired = true;
	}
}

void BuildingExt::ExtData::OccupantsWeaponChange()
{
	BuildingClass* pBuilding = OwnerObject();

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

void __fastcall BuildingExt::BuildingPassengerFix(BuildingClass* pThis)
{
	if (pThis->GetWeapon(0)->WeaponType != nullptr)
	{
		if (pThis->Passengers.NumPassengers == 0 && pThis->GetCurrentMission() == Mission::Unload)
		{
			pThis->ForceMission(Mission::Stop);
			pThis->Target = nullptr;
			pThis->ForceMission(Mission::Guard);
		}
	}
}

void __fastcall BuildingExt::BuildingSpawnFix(BuildingClass* pThis)
{
	auto pManager = pThis->SpawnManager;

	for (auto pItem : pManager->SpawnedNodes)
	{
		if (pItem->Status == SpawnNodeStatus::Returning
			&& pItem->Techno != nullptr
			&& pItem->Techno->GetHeight() == 0)
		{
			auto FoundationX = pThis->Type->GetFoundationWidth();
			auto FoundationY = pThis->Type->GetFoundationHeight(true);

			if (FoundationX < 0)
				FoundationX = 0;

			if (FoundationY < 0)
				FoundationY = 0;

			auto adjust = pThis->GetCoords() - CoordStruct { (FoundationX - 1) * 128, (FoundationY - 1) * 128 };

			pItem->Techno->SetLocation(adjust);
		}
	}
}

void BuildingExt::ExtData::SelectIFVWeapon()
{
	BuildingClass* pThis = OwnerObject();
	const auto pType = pThis->Type;
	//const auto pTypeExt = this->TypeExtData;
	const auto pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pType);

	if (pTechnoTypeExt->IsExtendGattling)
		return;

	this->IFVTurrets = pTechnoTypeExt->Turrets;

	auto& turrets = this->IFVTurrets;

	if (pThis->Passengers.NumPassengers > 0)
	{
		auto pFirstType = pThis->Passengers.FirstPassenger->GetTechnoType();

		if (pFirstType->IFVMode < pType->WeaponCount)
			this->IFVMode = pFirstType->IFVMode;
		else
			this->IFVMode = 0;

		if (pFirstType->IFVMode < pType->TurretCount)
			pThis->CurrentTurretNumber = turrets[pFirstType->IFVMode].GetItem(0);
		else
			pThis->CurrentTurretNumber = turrets[0].GetItem(0);
	}
	else
	{
		this->IFVMode = 0;
		pThis->CurrentTurretNumber = turrets[0].GetItem(0);
	}

	pThis->UpdatePlacement(PlacementType::Redraw);
}

void BuildingExt::ExtData::OccupantsWeapon()
{
	auto const pBuilding = OwnerObject();

	if (pBuilding->CanOccupyFire()
		&& pBuilding->Occupants.Count > 0
		&& pBuilding->FiringOccupantIndex >= 0
		&& pBuilding->FiringOccupantIndex < pBuilding->Occupants.Count)
	{
		auto pInf = pBuilding->Occupants.GetItem(pBuilding->FiringOccupantIndex);

		if (pInf->IsReallyAlive())
		{
			auto pInfType = pInf->GetTechnoType();

			if (auto pInfTypeExt = TechnoTypeExt::ExtMap.Find(pInfType))
			{
				this->CurrtenWeapon = pInfTypeExt->OccupyWeapons.Get(pInf).WeaponType;
			}
			else
			{
				this->CurrtenWeapon = nullptr;
			}
		}
		else
		{
			this->CurrtenWeapon = nullptr;
		}
	}
	else
	{
		this->CurrtenWeapon = nullptr;
	}
}

void BuildingExt::ExtData::BuildingWeaponChange()
{
	auto pBuilding = OwnerObject();
	const auto pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pBuilding->Type);

	if (pBuilding->CanOccupyFire())
	{
		if (this->CurrtenWeapon)
		{
			pBuilding->GetWeapon(0)->WeaponType = this->CurrtenWeapon;
		}
		else
		{
			pBuilding->GetWeapon(0)->WeaponType = pTechnoTypeExt->Weapons.Get(0, pBuilding).WeaponType;
		}
	}
}

// =============================
// load / save

template <typename T>
void BuildingExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->TypeExtData)
		.Process(this->TechnoExtData)
		.Process(this->DeployedTechno)
		.Process(this->LimboID)
		.Process(this->GrindingWeapon_LastFiredFrame)
		.Process(this->CurrentAirFactory)
		.Process(this->AccumulatedIncome)
		.Process(this->OfflineTimer)
		.Process(this->SellTimer)
		.Process(this->CaptureTimer)
		.Process(this->CaptureCount)
		.Process(this->CaptureOwner)
		.Process(this->OriginalOwner)
		.Process(this->SellingForbidden)
		.Process(this->RevealSightHouses)
		.Process(this->RevealSightRanges)
		.Process(this->RevealSightTimers)
		.Process(this->RevealSightPermanents)
		.Process(this->SpyEffectAnim)
		.Process(this->SpyEffectAnimDuration)
		.Process(this->SpyEffectAnimDisplayHouses)
		.Process(this->SellWeaponDetonated)
		.Process(this->OverPowerLevel)
		.Process(this->BuildingROFFix)
		.Process(this->IFVTurrets)
		.Process(this->IFVMode)
		.Process(this->CurrtenWeapon)
		;
}

void BuildingExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<BuildingClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void BuildingExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<BuildingClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool BuildingExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Success();
}

bool BuildingExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Success();
}

// =============================
// container

BuildingExt::ExtContainer::ExtContainer() : Container("BuildingClass") { }

BuildingExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x43BCBD, BuildingClass_CTOR, 0x6)
{
	GET(BuildingClass*, pItem, ESI);

	auto const pExt = BuildingExt::ExtMap.TryAllocate(pItem);

	if (pExt)
	{
		pExt->TypeExtData = BuildingTypeExt::ExtMap.Find(pItem->Type);
		pExt->TechnoExtData = TechnoExt::ExtMap.Find(pItem);
	}

	return 0;
}

DEFINE_HOOK(0x43C022, BuildingClass_DTOR, 0x6)
{
	GET(BuildingClass*, pItem, ESI);

	BuildingExt::ExtMap.Remove(pItem);

	return 0;
}

DEFINE_HOOK_AGAIN(0x454190, BuildingClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x453E20, BuildingClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(BuildingClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	BuildingExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x45417E, BuildingClass_Load_Suffix, 0x5)
{
	BuildingExt::ExtMap.LoadStatic();

	return 0;
}

DEFINE_HOOK(0x454244, BuildingClass_Save_Suffix, 0x7)
{
	BuildingExt::ExtMap.SaveStatic();

	return 0;
}

// Removes setting otherwise unused field (0x6FC) in BuildingClass when building has airstrike applied on it so that it can safely be used to store BuildingExt pointer.
DEFINE_JUMP(LJMP, 0x41D9FB, 0x41DA05);
