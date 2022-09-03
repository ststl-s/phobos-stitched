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
	PhobosFixedString<0x20> Name;

	CustomArmor() : ArrayIndex(Array.size()), Expression()
	{
		Array.emplace_back(this);
	}

	CustomArmor(const char* pName) : Name(pName), ArrayIndex(Array.size()), Expression()
	{
		Array.emplace_back(this);
	}

	~CustomArmor() = default;

	static CustomArmor* __fastcall Find(const char* pName);
	static int __fastcall FindIndex(const char* pName);
	static CustomArmor* __fastcall FindOrAllocate(const char* pName);
	static void LoadFromINIList(CCINIClass* pINI);
	static double GetVersus(WarheadTypeExt::ExtData* pWHExt, int armorIdx);
	static double GetVersus(WarheadTypeClass* pWH, int armorIdx);
	static double GetVersus(WarheadTypeClass* pWH, Armor armor);

	// index >= 11
	static CustomArmor* __fastcall GetArmor(int armorIndex);

	static void Clear();
	static bool LoadGlobals(PhobosStreamReader& stm);
	static bool SaveGlobals(PhobosStreamWriter& stm);

	bool Save(PhobosStreamReader& stm);
	bool Load(PhobosStreamWriter& stm);
};
