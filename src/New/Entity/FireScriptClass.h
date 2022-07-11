#pragma once
#include <GeneralStructures.h>
#include <LaserDrawClass.h>
#include <HouseClass.h>
#include <Utilities/TemplateDef.h>

#include <Ext/WeaponType/Body.h>
#include <New/Type/FireScriptTypeClass.h>

#include <vector>


class FireScriptClass
{
public:
	FireScriptTypeClass* Type;
	TechnoClass* Techno;
	int CurrentLine;
	int CurrentFrame;
	int TotalFrame;
	ValueableVector<Vector2D<int>> NowCycleTime;
	CoordStruct LocWhenFire;

	FireScriptClass(FireScriptTypeClass* Type, TechnoClass* pThis, CoordStruct LocWhenFire, int CurrentLine = 0, int CurrentFrame = 0, int TotalFrame = 0)
		:Type(Type), Techno(pThis), LocWhenFire(LocWhenFire), CurrentLine(CurrentLine), CurrentFrame(CurrentFrame), TotalFrame(TotalFrame), NowCycleTime()
	{
	}
	~FireScriptClass() { }
	void ProcessScript(bool SelfCenter);

	void InvalidatePointer(void* ptr) { };

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	bool Save(PhobosStreamWriter& Stm) const;

private:
	template <typename T>
	bool Serialize(T& Stm);
};