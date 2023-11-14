#pragma once

#include <FootClass.h>

class FootExt
{
public:
	using base_type = FootClass;

	static bool __fastcall EnterInfantry(FootClass* pThis, InfantryClass* pInf);
};
