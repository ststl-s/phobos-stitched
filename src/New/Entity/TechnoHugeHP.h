#pragma once

#include <set>

#ifndef TechnoTypeExt
#include <Ext/TechnoType/Body.h>
#endif

#ifndef Techno_Huge_HP

struct Techno_With_Type
{
	TechnoClass* pThis;
	TechnoTypeExt::ExtData* pTypeExt;
	Techno_With_Type() :pThis(nullptr), pTypeExt(nullptr) { }
	Techno_With_Type(TechnoClass* pThis, TechnoTypeExt::ExtData* pTypeExt) :pThis(pThis), pTypeExt(pTypeExt) { }
	bool operator < (const Techno_With_Type& s)const
	{
		return pTypeExt->HugeHP_Priority.Get() < s.pTypeExt->HugeHP_Priority.Get();
	}
	bool operator > (const Techno_With_Type& s)const
	{
		return pTypeExt->HugeHP_Priority.Get() > s.pTypeExt->HugeHP_Priority.Get();
	}
	bool operator == (const Techno_With_Type& s)const
	{
		return pTypeExt == s.pTypeExt && pThis == s.pThis;
	}
};

DynamicVectorClass<Techno_With_Type> Techno_Huge_HP;
#endif