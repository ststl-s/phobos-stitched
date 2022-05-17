#pragma once
#include <Utilities/TemplateDef.h>
#include <Ext/Techno/Body.h>
#include <set>
#include <map>

class PhobosGlobal
{
public:
	std::multimap<int,TechnoClass*> Techno_HugeBar;

	bool Save(PhobosStreamWriter& stm);
	bool Load(PhobosStreamReader& stm);

	static bool SaveGlobals(PhobosStreamWriter& stm);
	static bool LoadGlobals(PhobosStreamReader& stm);

	static PhobosGlobal* Global();

	PhobosGlobal() :
		Techno_HugeBar()
	{ }
	~PhobosGlobal() = default;
	void InvalidatePointer(void* ptr, bool bRemoved) {};
	
private:
	template <typename T>
	bool Serialize(T& Stm);
	
	static PhobosGlobal GlobalObject;
};