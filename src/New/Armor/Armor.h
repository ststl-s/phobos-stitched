#pragma once
#include <WarheadTypeClass.h>

#include <Utilities/Template.h>
#include <Utilities/Constructs.h>
#include <Utilities/ExpressionAnalyzer.h>

#include <Ext/WarheadType/Body.h>

class CustomArmor
{
public:

	static const char* BaseArmorName[];
	static const int BaseArmorNumber;
	static std::vector<std::unique_ptr<CustomArmor>> Array;

	const int ArrayIndex;
	std::vector<ExpressionAnalyzer::word> Expression;
	char Name[0x32] = "";

	CustomArmor() : ArrayIndex(Array.size()), Expression()
	{
		Array.emplace_back(this);
	}

	CustomArmor(const char* pName) : ArrayIndex(Array.size()), Expression()
	{
		strcpy_s(this->Name, pName);
		Array.emplace_back(this);
	}

	~CustomArmor() = default;

	inline std::string ExpressionString() const
	{
		std::string result;

		for (const auto& word : Expression)
		{
			result += word.Item;
		}

		return result;
	}

	static CustomArmor* __fastcall Find(const char* pName);
	static int __fastcall FindIndex(const char* pName);
	static CustomArmor* __fastcall FindOrAllocate(const char* pName);
	static void LoadFromINIList(CCINIClass* pINI);
	static double __fastcall GetVersus(WarheadTypeExt::ExtData* pWHExt, int armorIdx);
	static double __fastcall GetVersus(const WarheadTypeClass* pWH, int armorIdx);
	static double __fastcall GetVersus(const WarheadTypeClass* pWH, Armor armor);

	// index >= 11
	static CustomArmor* __fastcall GetArmor(int armorIndex);

	static const char* __fastcall GetArmorName(int armorIndex);

	static void Clear();
	static bool LoadGlobals(PhobosStreamReader& stm);
	static bool SaveGlobals(PhobosStreamWriter& stm);

	bool Load(PhobosStreamReader& stm, bool registerForChange);
	bool Save(PhobosStreamWriter& stm) const;
};
