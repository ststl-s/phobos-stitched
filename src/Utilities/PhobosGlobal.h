#pragma once
#include <Utilities/TemplateDef.h>
#include <Ext/Techno/Body.h>
#include <set>

class PhobosGlobal
{
public:
	std::set<std::pair<int, TechnoClass*>> Techno_HugeBar;

	void Save(PhobosStreamWriter& stm);
	void Load(PhobosStreamReader& stm);

	static void SaveGlobal(PhobosStreamWriter& stm);
	static void LoadGlobal(PhobosStreamReader& stm);

	static PhobosGlobal* Global();
	
private:
	template <typename T>
	void Serialize(T& Stm);
	
	static PhobosGlobal GlobalObject;
};