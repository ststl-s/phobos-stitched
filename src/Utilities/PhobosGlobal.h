#pragma once
#include <Utilities/TemplateDef.h>
#include <Ext/Techno/Body.h>
#include <set>
#include <map>

class PhobosGlobal
{
public:
	std::multimap<int, TechnoClass*> Techno_HugeBar;
	std::map<int, std::set<TriggerClass*>> RandomTriggerPool;
	TechnoClass* GenericStand;

	bool Save(PhobosStreamWriter& stm);
	bool Load(PhobosStreamReader& stm);

	static bool SaveGlobals(PhobosStreamWriter& stm);
	static bool LoadGlobals(PhobosStreamReader& stm);

	static PhobosGlobal* Global();

	PhobosGlobal() :
		Techno_HugeBar()
		, RandomTriggerPool()
		, GenericStand(nullptr)
	{
	}

	~PhobosGlobal() = default;

	static void Clear();
	static void PointerGotInvalid(void* ptr, bool bRemoved) { }

	TechnoClass* GetGenericStand();
	void Reset();

private:
	template <typename T>
	bool Serialize(T& stm);

	template <typename T>
	static bool SerializeGlobal(T& stm);

	template <typename T>
	static bool ProcessTechnoType(T& stm);

	template <typename T>
	static bool ProcessTechno(T& stm);

	template <typename T>
	static bool ProcessWarhead(T& stm);

	template <typename T>
	static bool ProcessSWType(T& stm);
	
	static PhobosGlobal GlobalObject;
};
