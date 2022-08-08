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
	std::deque<ExpressionAnalyzer::word> Expression;
	PhobosFixedString<0x20> Name;

	CustomArmor() : ArrayIndex(Array.size()), Expression()
	{
		Array.emplace_back(this);
	}

	CustomArmor(const char* pName) : Name(pName), ArrayIndex(Array.size()), Expression()
	{
		Array.emplace_back(this);
	}

	~CustomArmor()
	{
		auto it = std::find_if(Array.begin(), Array.end(),
			[this](std::unique_ptr<CustomArmor>& pArmor)
			{
				return pArmor.get() == this;
			});

		if (it != Array.end())
			Array.erase(it);
	}

	static CustomArmor* __fastcall Find(const char* pName);
	static int __fastcall FindIndex(const char* pName);
	static CustomArmor* __fastcall FindOrAllocate(const char* pName);
	static void LoadFromINIList(CCINIClass* pINI);
	static double GetVersus(WarheadTypeExt::ExtData* pWHExt, int armorIdx);

	// index >= 11
	static CustomArmor* __fastcall GetArmor(int armorIndex);

	static bool LoadGlobals(PhobosStreamReader& stm);
	static bool SaveGlobals(PhobosStreamWriter& stm);

	bool Save(PhobosStreamReader& stm);
	bool Load(PhobosStreamWriter& stm);
};
