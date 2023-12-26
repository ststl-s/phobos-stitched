#include "Body.h"

#include <Helpers/Macro.h>

#include <Ext/Techno/Body.h>

#include <Misc/FlyingStrings.h>

#include <New/Entity/LaserTrailClass.h>

#include <Utilities/Helpers.Alex.h>

// Add or substract experience for real
int AddExpCustom(VeterancyStruct* vstruct, int targetCost, int exp)
{
	double toBeAdded = (double)exp / (targetCost * RulesClass::Instance->VeteranRatio);
	// Used in experience transfer to get the actual amount substracted
	int transffered = (int)(Math::min(vstruct->Veterancy, abs(toBeAdded))
		* (targetCost * RulesClass::Instance->VeteranRatio));
	// Don't do anything when current exp at 0
	if (exp < 0 && transffered <= 0)
	{
		vstruct->Reset();
		transffered = 0;
	}
	else
	{
		vstruct->Add(toBeAdded);
	}
	// Prevent going above elite level of 2.0
	if (vstruct->IsElite())
	{
		vstruct->SetElite();
	}
	return transffered;
}

void VeterancyCustom(VeterancyStruct* vstruct, int target)
{
	switch (target)
	{
	case 0:
		vstruct->SetRookie(false);
		break;
	case 1:
		vstruct->SetVeteran(true);
		break;
	default:
		vstruct->SetElite(true);
		break;
	}
}

int AddMoneyCustom(HouseClass* house, int houseMoney, int money)
{
	int transffered = abs(money);
	if (money < 0)
	{
		if (houseMoney >= transffered)
			house->TransactMoney(money);
		else
		{
			transffered = abs(houseMoney);
			house->TransactMoney(-houseMoney);
		}
	}
	else
		house->TransactMoney(money);
	return transffered;
}

int AddHealthCustom(TechnoClass* target, int totalHealth, int health)
{
	int transffered = abs(health);
	if (health < 0)
	{
		if (target->Health < transffered)
			transffered = abs(target->Health);

		target->TakeDamage(transffered, target->Owner);
	}
	else
	{
		if (totalHealth - target->Health < transffered)
			transffered = totalHealth - target->Health;

		target->TakeDamage(-transffered, target->Owner);
	}
	return transffered;
}

int WarheadTypeExt::ExtData::TransactOneValue(TechnoClass* pTechno, TechnoTypeClass* pTechnoType, int transactValue, TransactValueType valueType)
{
	int transferred = 0;

	if (pTechno == nullptr)
		return 0;

	switch (valueType)
	{
	case TransactValueType::Experience:
		transferred = AddExpCustom(&pTechno->Veterancy,
			pTechnoType->GetActualCost(pTechno->Owner), transactValue);
		break;
	case TransactValueType::Money:
		transferred = AddMoneyCustom(pTechno->Owner, pTechno->Owner->Available_Money(), transactValue);
		break;
	case TransactValueType::Health:
		transferred = AddHealthCustom(pTechno, pTechnoType->Strength, transactValue);
		break;
	default:
		break;
	}

	return transferred;
}

int WarheadTypeExt::ExtData::TransactGetValue(TechnoClass* pTarget, TechnoClass* pOwner, int flat, double percent, boolean calcFromTarget, int targetValue, int ownerValue)
{
	int flatValue = 0, percentValue = 0;

	// Flat
	flatValue = flat;

	// Percent
	if (!CLOSE_ENOUGH(percent, 0.0))
	{
		if (calcFromTarget && pTarget)
			percentValue = (int)(targetValue * percent);
		else if (!calcFromTarget)
			percentValue = (int)(ownerValue * percent);
	}

	return abs(percentValue) > abs(flatValue) ? percentValue : flatValue;
}

std::vector<std::vector<int>> WarheadTypeExt::ExtData::TransactGetSourceAndTarget(TechnoClass* pTarget, TechnoTypeClass* pTargetType, TechnoClass* pOwner, TechnoTypeClass* pOwnerType, int targets)
{
	std::vector<std::vector<int>> allValues;
	std::vector<int> sourceValues;
	std::vector<int> targetValues;

	// SOURCE
	//		Experience
	if (pTarget != nullptr && pTargetType != nullptr)
	{
		int sourceExp = pOwner ? this->TransactGetValue(pTarget, pOwner,
			this->Transact_Experience_Source_Flat, this->Transact_Money_Source_Percent,
			this->Transact_Money_Source_Percent_CalcFromTarget,
			pTargetType->GetActualCost(pTarget->Owner), pOwnerType->GetActualCost(pOwner->Owner)) : 0;
		sourceValues.push_back(sourceExp / targets);
		//		Others ...
		allValues.push_back(sourceValues);//0
		sourceValues.clear();
	}

	// TARGET
	//		Experience
	if (pOwner != nullptr && pOwnerType != nullptr)
	{
		int targetExp = pTarget ? this->TransactGetValue(pOwner, pTarget,
			this->Transact_Experience_Target_Flat, this->Transact_Experience_Target_Percent,
			this->Transact_Experience_Target_Percent_CalcFromSource,
			pOwnerType->GetActualCost(pOwner->Owner), pTargetType->GetActualCost(pTarget->Owner)) : 0;
		targetValues.push_back(targetExp / targets);
		//		Others ...
		allValues.push_back(targetValues);//1
		targetValues.clear();
	}

	// SOURCE
	//		Money
	if (pTarget != nullptr && pTargetType != nullptr)
	{
		int sourceMoney = pOwner ? this->TransactGetValue(pTarget, pOwner,
			this->Transact_Money_Source_Flat, this->Transact_Money_Source_Percent,
			this->Transact_Money_Source_Percent_CalcFromTarget,
			pTargetType->GetActualCost(pTarget->Owner), pOwnerType->GetActualCost(pOwner->Owner)) : 0;
		sourceValues.push_back(sourceMoney / targets);
		//		Others ...
		allValues.push_back(sourceValues);//2
		sourceValues.clear();
	}

	// TARGET
	//		Money
	if (pOwner != nullptr && pOwnerType != nullptr)
	{
		int targetMoney = pTarget ? this->TransactGetValue(pOwner, pTarget,
			this->Transact_Money_Target_Flat, this->Transact_Money_Target_Percent,
			this->Transact_Money_Target_Percent_CalcFromSource,
			pOwnerType->GetActualCost(pOwner->Owner), pTargetType->GetActualCost(pTarget->Owner)) : 0;
		targetValues.push_back(targetMoney / targets);
		//		Others ...
		allValues.push_back(targetValues);//3
		targetValues.clear();
	}

	// SOURCE
	//		Health
	if (pTarget != nullptr && pTargetType != nullptr)
	{
		int sourceHealth = pOwner ? this->TransactGetValue(pTarget, pOwner,
			this->Transact_Health_Source_Flat, this->Transact_Health_Source_Percent,
			this->Transact_Health_Source_Percent_CalcFromTarget,
			this->Transact_Health_Target_Percent_UseCurrentHealth ? pTarget->Health : pTargetType->Strength,
			this->Transact_Health_Source_Percent_UseCurrentHealth ? pOwner->Health : pOwnerType->Strength) : 0;
		sourceValues.push_back(sourceHealth / targets);
		//		Others ...
		allValues.push_back(sourceValues);//4
		sourceValues.clear();
	}

	// TARGET
	//		Health
	if (pOwner != nullptr && pOwnerType != nullptr)
	{
		int targetHealth = pTarget ? this->TransactGetValue(pOwner, pTarget,
			this->Transact_Health_Target_Flat, this->Transact_Health_Target_Percent,
			this->Transact_Health_Target_Percent_CalcFromSource,
			this->Transact_Health_Source_Percent_UseCurrentHealth ? pOwner->Health : pOwnerType->Strength,
			this->Transact_Health_Target_Percent_UseCurrentHealth ? pTarget->Health : pTargetType->Strength) : 0;
		targetValues.push_back(targetHealth / targets);
		//		Others ...
		allValues.push_back(targetValues);//5
		targetValues.clear();
	}

	return allValues;
}

void WarheadTypeExt::ExtData::TransactOnOneUnit(TechnoClass* pTarget, TechnoClass* pOwner, int targets)
{
	if (!TechnoExt::IsReallyAlive(pTarget))
		return;

	auto const pTargetType = pTarget ? pTarget->GetTechnoType() : nullptr;

	if (pOwner == nullptr
		   || this->Transact_Experience_Value.isset()
		   || this->Transact_Experience_Percent.isset()
		   || this->Transact_Experience_Veterancy.isset())
	{
		if (this->Transact_Experience_Veterancy.isset())
			VeterancyCustom(&pTarget->Veterancy, this->Transact_Experience_Veterancy.Get());
		else if (this->Transact_Experience_Percent.isset())
			TransactOneValue(pTarget, pTargetType, (int)(this->Transact_Experience_Percent.Get() * pTargetType->GetActualCost(pTarget->Owner)), TransactValueType::Experience);
		else
			TransactOneValue(pTarget, pTargetType, this->Transact_Experience_Value.Get(), TransactValueType::Experience);
	}

	if (!TechnoExt::IsReallyAlive(pOwner))
		return;

	auto const pOwnerType = pOwner ? pOwner->GetTechnoType() : nullptr;
	std::vector<std::vector<int>> allValues = this->TransactGetSourceAndTarget(pTarget, pTargetType, pOwner, pOwnerType, targets);

	//Experience
	for (size_t i = 0; i < allValues[0].size(); i++)
	{
		int sourceValue = allValues[0][i];
		int targetValue = allValues[1][i];

		// Transact (A loses B gains)
		if (sourceValue != 0 && targetValue != 0 && targetValue * sourceValue < 0)
		{
			int transactValue = abs(sourceValue) > abs(targetValue) ? abs(targetValue) : abs(sourceValue);

			if (sourceValue < 0)
			{
				transactValue = TransactOneValue(pOwner, pOwnerType, -transactValue, TransactValueType::Experience);
				TransactOneValue(pTarget, pTargetType, transactValue, TransactValueType::Experience);
			}
			else
			{
				transactValue = TransactOneValue(pTarget, pTargetType, -transactValue, TransactValueType::Experience);
				TransactOneValue(pOwner, pOwnerType, transactValue, TransactValueType::Experience);
			}
		}
		else
		{
			// Out-of-thin-air grants
			if (sourceValue != 0)
				TransactOneValue(pOwner, pOwnerType, sourceValue, TransactValueType::Experience);

			if (targetValue != 0)
				TransactOneValue(pTarget, pTargetType, targetValue, TransactValueType::Experience);
		}
	}

	if (allValues.size() >= 6)
	{
		//		Health
		for (size_t i = 0; i < allValues[4].size() && i < allValues[5].size(); i++)
		{
			int sourceValue = allValues[4][i];
			int targetValue = allValues[5][i];

			// Transact (A loses B gains)
			if (sourceValue != 0 && targetValue != 0 && targetValue * sourceValue < 0)
			{
				int transactValue = abs(sourceValue) > abs(targetValue) ? abs(targetValue) : abs(sourceValue);

				if (sourceValue < 0)
				{
					transactValue = TransactOneValue(pOwner, pOwnerType, -transactValue, TransactValueType::Health);
					TransactOneValue(pTarget, pTargetType, transactValue, TransactValueType::Health);
				}
				else
				{
					transactValue = TransactOneValue(pTarget, pTargetType, -transactValue, TransactValueType::Health);
					TransactOneValue(pOwner, pOwnerType, transactValue, TransactValueType::Health);
				}
			}
			else
			{
				// Out-of-thin-air grants
				if (sourceValue != 0)
					TransactOneValue(pOwner, pOwnerType, sourceValue, TransactValueType::Health);

				if (targetValue != 0)
					TransactOneValue(pTarget, pTargetType, targetValue, TransactValueType::Health);
			}
		}
	}
}

int WarheadTypeExt::ExtData::TransactMoneyOnOneUnit(TechnoClass* pTarget, TechnoClass* pOwner, int targets)
{
	if (!TechnoExt::IsReallyAlive(pOwner) || !TechnoExt::IsReallyAlive(pTarget))
		return 0;

	auto const pTargetType = pTarget->GetTechnoType();
	auto const pOwnerType = pOwner->GetTechnoType();
	std::vector<std::vector<int>> allValues = this->TransactGetSourceAndTarget(pTarget, pTargetType, pOwner, pOwnerType, targets);
	int totalvalue = 0;
	bool allowweapon = false;

	//		Money
	for (size_t i = 0; i < allValues[2].size() && i < allValues[3].size(); i++)
	{
		int sourceValue = allValues[2][i];
		int targetValue = allValues[3][i];

		// Transact (A loses B gains)
		if (sourceValue != 0 && targetValue != 0 && targetValue * sourceValue < 0)
		{
			int transactValue = abs(sourceValue) > abs(targetValue) ? abs(targetValue) : abs(sourceValue);

			if (sourceValue < 0)
			{
				if (this->Transact_Money_Source_CheckMoney)
				{
					if (transactValue > pOwner->Owner->Available_Money())
						transactValue = 0;
				}

				transactValue = TransactOneValue(pOwner, pOwnerType, -transactValue, TransactValueType::Money);
				totalvalue += -transactValue;
				if (transactValue != 0)
				{
					if (this->Transact_Money_WeaponOnlyCheckOneSide)
					{
						allowweapon = true;
					}
					else
					{
						if (this->Transact_Money_Source_Weapon)
						{
							auto weaponowner = pTarget ? pTarget : pOwner;
							WeaponTypeExt::DetonateAt(this->Transact_Money_Source_Weapon, pOwner, weaponowner);
						}
					}
				}

				TransactOneValue(pTarget, pTargetType, transactValue, TransactValueType::Money);
				if (transactValue != 0)
				{
					if (this->Transact_Money_Target_Display)
						FlyingStrings::AddMoneyString(transactValue, pTarget->Owner, this->Transact_Money_Target_Display_Houses, pTarget->Location, this->Transact_Money_Target_Display_Offset);
					if (this->Transact_Money_WeaponOnlyCheckOneSide)
					{
						allowweapon = true;
					}
					else
					{
						if (this->Transact_Money_Target_Weapon)
						{
							auto weaponowner = pOwner ? pOwner : pTarget;
							WeaponTypeExt::DetonateAt(this->Transact_Money_Target_Weapon, pTarget, weaponowner);
						}
					}
				}
			}
			else
			{
				if (this->Transact_Money_Target_CheckMoney)
				{
					if (transactValue > pTarget->Owner->Available_Money())
						transactValue = 0;
				}

				transactValue = TransactOneValue(pTarget, pTargetType, -transactValue, TransactValueType::Money);
				if (transactValue != 0)
				{
					if (this->Transact_Money_Target_Display)
						FlyingStrings::AddMoneyString(-transactValue, pTarget->Owner, this->Transact_Money_Target_Display_Houses, pTarget->Location, this->Transact_Money_Target_Display_Offset);

					if (this->Transact_Money_WeaponOnlyCheckOneSide)
					{
						allowweapon = true;
					}
					else
					{
						if (this->Transact_Money_Target_Weapon)
						{
							auto weaponowner = pOwner ? pOwner : pTarget;
							WeaponTypeExt::DetonateAt(this->Transact_Money_Target_Weapon, pTarget, weaponowner);
						}
					}
				}

				TransactOneValue(pOwner, pOwnerType, transactValue, TransactValueType::Money);
				totalvalue += transactValue;
				if (transactValue != 0)
				{
					if (this->Transact_Money_WeaponOnlyCheckOneSide)
					{
						allowweapon = true;
					}
					else
					{
						if (this->Transact_Money_Source_Weapon)
						{
							auto weaponowner = pTarget ? pTarget : pOwner;
							WeaponTypeExt::DetonateAt(this->Transact_Money_Source_Weapon, pOwner, weaponowner);
						}
					}
				}
			}
		}
		else
		{
			// Out-of-thin-air grants
			if (sourceValue != 0)
			{
				if (this->Transact_Money_Source_CheckMoney)
				{
					if (sourceValue < 0 && abs(sourceValue) > pOwner->Owner->Available_Money())
						sourceValue = 0;
				}

				int truevalue = TransactOneValue(pOwner, pOwnerType, sourceValue, TransactValueType::Money);
				if (sourceValue > 0)
					totalvalue += truevalue;
				else
					totalvalue -= truevalue;

				if (truevalue != 0)
				{
					if (this->Transact_Money_WeaponOnlyCheckOneSide)
					{
						allowweapon = true;
					}
					else
					{
						if (this->Transact_Money_Source_Weapon)
						{
							auto weaponowner = pTarget ? pTarget : pOwner;
							WeaponTypeExt::DetonateAt(this->Transact_Money_Source_Weapon, pOwner, weaponowner);
						}
					}
				}
			}

			if (targetValue != 0)
			{
				if (this->Transact_Money_Target_CheckMoney)
				{
					if (targetValue < 0 && abs(targetValue) > pTarget->Owner->Available_Money())
						targetValue = 0;
				}

				int truevalue = TransactOneValue(pTarget, pTargetType, targetValue, TransactValueType::Money);
				if (truevalue != 0)
				{
					if (this->Transact_Money_Target_Display)
					{
						if (targetValue > 0)
							FlyingStrings::AddMoneyString(truevalue, pTarget->Owner, this->Transact_Money_Target_Display_Houses, pTarget->Location, this->Transact_Money_Target_Display_Offset);
						else
							FlyingStrings::AddMoneyString(-truevalue, pTarget->Owner, this->Transact_Money_Target_Display_Houses, pTarget->Location, this->Transact_Money_Target_Display_Offset);
					}

					if (this->Transact_Money_WeaponOnlyCheckOneSide)
					{
						allowweapon = true;
					}
					else
					{
						if (this->Transact_Money_Target_Weapon)
						{
							auto weaponowner = pOwner ? pOwner : pTarget;
							WeaponTypeExt::DetonateAt(this->Transact_Money_Target_Weapon, pTarget, weaponowner);
						}
					}
				}
			}
		}

		if (allowweapon)
		{
			if (this->Transact_Money_Target_Weapon)
			{
				auto weaponowner = pOwner ? pOwner : pTarget;
				WeaponTypeExt::DetonateAt(this->Transact_Money_Target_Weapon, pTarget, weaponowner);
			}

			if (this->Transact_Money_Source_Weapon)
			{
				auto weaponowner = pTarget ? pTarget : pOwner;
				WeaponTypeExt::DetonateAt(this->Transact_Money_Source_Weapon, pOwner, weaponowner);
			}
		}
	}

	return totalvalue;
}

void WarheadTypeExt::ExtData::TransactOnAllUnits(HouseClass* pHouse, const CoordStruct coords, const float cellSpread, TechnoClass* pOwner, WarheadTypeExt::ExtData* pWHTypeExt)
{
	int targets = 1;
	int sourcemoney = 0;

	if (this->Transact_SpreadAmongTargets)
		targets = Helpers::Alex::getCellSpreadItems(coords, cellSpread, true).size();

	for (auto pTarget : Helpers::Alex::getCellSpreadItems(coords, cellSpread, true))
	{
		if (pWHTypeExt->CanTargetHouse(pHouse, pTarget))
		{
			sourcemoney += TransactMoneyOnOneUnit(pTarget, pOwner, targets);
			TransactOnOneUnit(pTarget, pOwner, targets);
		}
	}

	if (sourcemoney != 0 && this->Transact_Money_Source_Display)
		FlyingStrings::AddMoneyString(sourcemoney, pOwner->Owner, this->Transact_Money_Target_Display_Houses, pOwner->Location, this->Transact_Money_Target_Display_Offset);
}
