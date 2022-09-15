#pragma once
#include <Utilities/Template.h>
#include <Ext/Techno/Body.h>
#include <set>
#include <map>

class PhobosGlobal
{
public:
	std::multimap<int, TechnoClass*, std::greater<int>> Techno_HugeBar;
	std::map<int, std::vector<TriggerClass*>> RandomTriggerPool;
	std::vector<QueuedSW> MultipleSWFirer_Queued;
	TechnoClass* GenericStand;
	std::unordered_map<int, int> TriggerType_HouseMultiplayer;

	bool Save(PhobosStreamWriter& stm);
	bool Load(PhobosStreamReader& stm);

	static bool SaveGlobals(PhobosStreamWriter& stm);
	static bool LoadGlobals(PhobosStreamReader& stm);

	inline static PhobosGlobal* Global()
	{
		return &GlobalObject;
	}

	PhobosGlobal() :
		Techno_HugeBar()
		, RandomTriggerPool()
		, MultipleSWFirer_Queued()
		, GenericStand(nullptr)
		, TriggerType_HouseMultiplayer()
	{
	}

	~PhobosGlobal() = default;

	static void Clear();
	static void PointerGotInvalid(void* ptr, bool removed);
	void InvalidatePointer(void* ptr, bool removed);

	TechnoClass* GetGenericStand();
	void CheckSuperQueued();

	void Reset();

private:
	template <typename T>
	bool Serialize(T& stm);

	static PhobosGlobal GlobalObject;
};
