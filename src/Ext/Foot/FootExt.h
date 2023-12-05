#pragma once
#include <FootClass.h>

#include <vector>

class FootExt
{
public:
	using base_type = FootClass;

	static bool __fastcall EnterInfantry(FootClass* pThis, InfantryClass* pInf);
	static void PutFootsTo(const std::vector<FootClass*> foots, const CellStruct& cell, bool overlap);
	static void PutFootsTo(const std::vector<FootClass*> foots, const CoordStruct& location, bool overlap);
};
