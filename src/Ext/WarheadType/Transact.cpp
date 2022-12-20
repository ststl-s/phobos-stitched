#include "Body.h"

#include <Utilities/Helpers.Alex.h>
#include <Misc/FlyingStrings.h>

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
	}

	return allValues;
}

void WarheadTypeExt::ExtData::TransactOnOneUnit(TechnoClass* pTarget, TechnoClass* pOwner, int targets)
{
	auto const pTargetType = pTarget ? pTarget->GetTechnoType() : nullptr;
	auto const pOwnerType = pOwner ? pOwner->GetTechnoType() : nullptr;
	std::vector<std::vector<int>> allValues = this->TransactGetSourceAndTarget(pTarget, pTargetType, pOwner, pOwnerType, targets);

	//		Experience
	if (pOwner == nullptr
		|| this->Transact_Experience_Value.isset()
		|| this->Transact_Experience_Veterancy.isset())
	{
		if (this->Transact_Experience_Veterancy.isset())
			VeterancyCustom(&pTarget->Veterancy, this->Transact_Experience_Veterancy.Get());
		else
			TransactOneValue(pTarget, pTargetType, this->Transact_Experience_Value.Get(), TransactValueType::Experience);
	}
	else
	{
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
	}

	//		Health
	for (size_t i = 0; i < allValues[4].size(); i++)
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

int WarheadTypeExt::ExtData::TransactMoneyOnOneUnit(TechnoClass* pTarget, TechnoClass* pOwner, int targets)
{
	auto const pTargetType = pTarget ? pTarget->GetTechnoType() : nullptr;
	auto const pOwnerType = pOwner ? pOwner->GetTechnoType() : nullptr;
	std::vector<std::vector<int>> allValues = this->TransactGetSourceAndTarget(pTarget, pTargetType, pOwner, pOwnerType, targets);
	int totalvalue = 0;
	//		Money
	for (size_t i = 0; i < allValues[2].size(); i++)
	{
		int sourceValue = allValues[2][i];
		int targetValue = allValues[3][i];

		// Transact (A loses B gains)
		if (sourceValue != 0 && targetValue != 0 && targetValue * sourceValue < 0)
		{
			int transactValue = abs(sourceValue) > abs(targetValue) ? abs(targetValue) : abs(sourceValue);

			if (sourceValue < 0)
			{
				transactValue = TransactOneValue(pOwner, pOwnerType, -transactValue, TransactValueType::Money);
				totalvalue += -transactValue;

				TransactOneValue(pTarget, pTargetType, transactValue, TransactValueType::Money);
				if (this->Transact_Money_Target_Display && transactValue != 0)
					FlyingStrings::AddMoneyString(transactValue, pTarget->Owner, this->Transact_Money_Target_Display_Houses, pTarget->Location, this->Transact_Money_Target_Display_Offset);
			}
			else
			{
				transactValue = TransactOneValue(pTarget, pTargetType, -transactValue, TransactValueType::Money);
				if (this->Transact_Money_Target_Display && transactValue != 0)
					FlyingStrings::AddMoneyString(-transactValue, pTarget->Owner, this->Transact_Money_Target_Display_Houses, pTarget->Location, this->Transact_Money_Target_Display_Offset);

				TransactOneValue(pOwner, pOwnerType, transactValue, TransactValueType::Money);
				totalvalue += transactValue;
			}
		}
		else
		{
			// Out-of-thin-air grants
			if (sourceValue != 0)
			{
				TransactOneValue(pOwner, pOwnerType, sourceValue, TransactValueType::Money);
				totalvalue += sourceValue;
			}

			if (targetValue != 0)
			{
				TransactOneValue(pTarget, pTargetType, targetValue, TransactValueType::Money);
				if (this->Transact_Money_Target_Display && targetValue != 0)
					FlyingStrings::AddMoneyString(targetValue, pTarget->Owner, this->Transact_Money_Target_Display_Houses, pTarget->Location, this->Transact_Money_Target_Display_Offset);
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
