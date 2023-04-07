#include "Armor.h"

#include <Utilities/TemplateDef.h>

#include <Ext/Techno/Body.h>

/*
None = 0,
Flak = 1,
Plate = 2,
Light = 3,
Medium = 4,
Heavy = 5,
Wood = 6,
Steel = 7,
Concrete = 8,
Special_1 = 9,
Special_2 = 10
*/
const char* CustomArmor::BaseArmorName[] =
{
	"none",
	"flak",
	"plate",
	"light",
	"medium",
	"heavy",
	"wood",
	"steel",
	"concrete",
	"special_1",
	"special_2"
};

const int CustomArmor::BaseArmorNumber = sizeof(BaseArmorName) / sizeof(const char*);

std::vector<std::unique_ptr<CustomArmor>> CustomArmor::Array;

CustomArmor* __fastcall CustomArmor::Find(const char* pName)
{
	auto it = std::find_if(Array.begin(), Array.end(),
		[pName](std::unique_ptr<CustomArmor>& pArmor)
		{
			return _strcmpi(pArmor->Name, pName) == 0;
		});

	if (it != Array.end())
	{
		return it->get();
	}

	return nullptr;
}

int __fastcall CustomArmor::FindIndex(const char* pName)
{
	auto ptr = std::find_if(BaseArmorName, BaseArmorName + BaseArmorNumber,
		[pName](const char* pArmorName)
		{
			return _strcmpi(pArmorName, pName) == 0;
		});

	if (ptr - BaseArmorName < BaseArmorNumber)
	{
		return ptr - BaseArmorName;
	}

	auto it = std::find_if(Array.begin(), Array.end(),
		[pName](std::unique_ptr<CustomArmor>& pArmor)
		{
			return _strcmpi(pArmor->Name, pName) == 0;
		});

	if (it != Array.end())
	{
		return it - Array.begin() + BaseArmorNumber;
	}

	return -1;
}

CustomArmor* __fastcall CustomArmor::FindOrAllocate(const char* pName)
{
	auto it = std::find_if(Array.begin(), Array.end(),
		[pName](std::unique_ptr<CustomArmor>& pArmor)
		{
			return _strcmpi(pArmor->Name, pName) == 0;
		});

	if (it != Array.end())
	{
		return it->get();
	}

	return new CustomArmor(pName);
}

void CustomArmor::LoadFromINIList(CCINIClass* pINI)
{
	const char* pSection = "ArmorTypes";
	int len = pINI->GetKeyCount(pSection);
	INI_EX exINI(pINI);

	for (int i = 0; i < len; ++i)
	{
		const char* pName = pINI->GetKeyName(pSection, i);
		exINI.ReadString(pSection, pName);

		if (FindIndex(pName) >= 0)
		{
			continue;
		}

		std::string expression = exINI.value();
		std::vector<ExpressionAnalyzer::word> expressionWords
		(
			std::move
			(
				ExpressionAnalyzer::InfixToPostfixWords(expression, [pSection, pName, exINI](const std::string& name)
					{
						int idx = FindIndex(name.c_str());

						if (idx == -1)
						{
							Debug::INIParseFailed(pSection, pName, exINI.value(), "Expecting a registered ArmorType");
							return std::string("0");
						}
						else
						{
							return std::to_string(idx);
						}
					})
			)
		);

		if (expressionWords.empty())
		{
			Debug::INIParseFailed(pSection, pName, exINI.value(), "Expression is invalid");
		}

		CustomArmor* pArmor = FindOrAllocate(pName);
		pArmor->Expression = std::move(expressionWords);
	}
}

double __fastcall CustomArmor::GetVersus(WarheadTypeExt::ExtData* pWHExt, int armorIdx)
{
	if (armorIdx < BaseArmorNumber)
	{
		if (armorIdx < 0)
			return 0.0;

		return pWHExt->Verses[armorIdx];
	}

	// if (pWHExt->Versus.count(armorIdx - BaseArmorNumber))
	if (pWHExt->Versus_HasValue[armorIdx - BaseArmorNumber])
		return pWHExt->Versus[armorIdx - BaseArmorNumber];

	if (armorIdx - BaseArmorNumber > static_cast<int>(Array.size()))
	{
		Debug::Log("[Develop Error] Invalid armor index{%d}", armorIdx);

		return 0.0;
	}

	return ExpressionAnalyzer::CalculatePostfixExpression
	(Array[armorIdx - BaseArmorNumber]->Expression,
		[pWHExt](const std::string& sIdx)
		{
		int idx = atoi(sIdx.c_str());
		return GetVersus(pWHExt, idx);
		});

	/*
	return pWHExt->Versus[armorIdx - BaseArmorNumber] =
		ExpressionAnalyzer::CalculatePostfixExpression
		(Array[armorIdx - BaseArmorNumber]->Expression,
			[pWHExt](const std::string& sIdx)
			{
				int idx = atoi(sIdx.c_str());
				return GetVersus(pWHExt, idx);
			}
	);
	*/
}

double __fastcall CustomArmor::GetVersus(const WarheadTypeClass* pWH, int armorIdx)
{
	auto pWHExt = WarheadTypeExt::ExtMap.Find(pWH);

	return GetVersus(pWHExt, armorIdx);
}

double __fastcall CustomArmor::GetVersus(const WarheadTypeClass* pWH, Armor armor)
{
	return GetVersus(pWH, static_cast<int>(armor));
}

CustomArmor* __fastcall CustomArmor::GetArmor(int armorIndex)
{
	if (armorIndex < BaseArmorNumber || armorIndex - BaseArmorNumber >= static_cast<int>(Array.size()))
	{
		return nullptr;
	}

	return Array[armorIndex - BaseArmorNumber].get();
}

const char* __fastcall CustomArmor::GetArmorName(int armorIndex)
{
	if (0 <= armorIndex && armorIndex < BaseArmorNumber)
		return BaseArmorName[armorIndex];

	if (0 <= armorIndex && armorIndex - 11 < static_cast<int>(Array.size()))
		return Array[armorIndex - 11]->Name;

	return "CustomArmor::ErrorType::out_of_range";
}

void CustomArmor::Debug(const TechnoClass* pAttacker, const AbstractClass* pTarget, const WeaponTypeClass* pWeapon)
{
	if (const auto pObj = abstract_cast<const ObjectClass*>(pTarget))
	{
		const WarheadTypeClass* pWH = pWeapon->Warhead;

		if (const auto pTechno = abstract_cast<const TechnoClass*>(pTarget))
		{
			const auto pTechnoExt = TechnoExt::ExtMap.Find(pTechno);
			const int armorIdx = pTechnoExt->GetArmorIdx(pWH);
			const double versus = GetVersus(pWH, armorIdx);
			const char* armorName = GetArmorName(armorIdx);
			const std::string expression = armorIdx < BaseArmorNumber ? "" : GetArmor(armorIdx)->GetExpression();
			Debug::Log("[" __FUNCTION__ "] Attacker{%s}, Target{%s}, Weapon{%s}, WH{%s}\n"
				"Versus{%.3lf}, Armor{%s}, Expression{%s}\n",
				pAttacker->get_ID(), pTechno->get_ID(), pWeapon->get_ID(), pWH->get_ID(),
				versus, armorName, expression.c_str());
		}
		else
		{
			const int armorIdx = static_cast<int>(pObj->GetType()->Armor);
			const double versus = GetVersus(pWH, armorIdx);
			const char* armorName = GetArmorName(armorIdx);
			const std::string expression = armorIdx < BaseArmorNumber ? "" : GetArmor(armorIdx)->GetExpression();
			Debug::Log("[" __FUNCTION__ "] Attacker{%s}, Target{%s}, Weapon{%s}, WH{%s}\n"
				"Versus{%.3lf}, Armor{%s}, Expression{%s}\n",
				pAttacker->get_ID(), pObj->GetType()->get_ID(), pWeapon->get_ID(), pWH->get_ID(),
				versus, armorName, expression.c_str());
		}
	}
}

const std::string CustomArmor::GetExpression() const
{
	std::string result;

	for (const auto& word : this->Expression)
	{
		if (word.IsOperator)
		{
			result += word.Item;
		}
		else
		{
			int idx = atoi(word.Item.c_str());
			result += GetArmorName(idx);
		}
	}

	return result;
}

void CustomArmor::Clear()
{
	Array.clear();
}

bool CustomArmor::LoadGlobals(PhobosStreamReader& stm)
{
	stm.Process(Array);
	return stm.Success();
}

bool CustomArmor::SaveGlobals(PhobosStreamWriter& stm)
{
	stm.Process(Array);
	return stm.Success();
}

bool CustomArmor::Load(PhobosStreamReader& stm, bool registerForChange)
{
	stm.Process(this->Expression);
	stm.Process(this->Name);
	return stm.Success();
}

bool CustomArmor::Save(PhobosStreamWriter& stm) const
{
	stm.Process(this->Expression);
	stm.Process(this->Name);
	return stm.Success();
}

DEFINE_HOOK(0x475430, ReadArmorType, 0x5)
{
	GET(CCINIClass*, pINI, ECX);
	GET_STACK(const char*, pSection, 0x4);
	GET_STACK(const char*, pKey, 0x8);
	GET_STACK(int, defaultIdx, 0xC);

	INI_EX exINI(pINI);
	exINI.ReadString(pSection, pKey);
	int armorIdx = CustomArmor::FindIndex(exINI.value());
	R->EAX(armorIdx == -1 ? defaultIdx : armorIdx);

	return 0;
}
