#include "Body.h"

#include <HouseClass.h>

#include <Utilities/Macro.h>

CoordStruct* __fastcall SuperOn_CellClass_GetCenterCoords_Override(CellClass* pCell, DWORD _, CoordStruct& buffer)
{
	buffer = pCell->GetCoordsWithBridge();
	buffer.X += 126;
	buffer.Y += 126;
	return &buffer;
}

DEFINE_JUMP(CALL, 0x6DC201, GET_OFFSET(SuperOn_CellClass_GetCenterCoords_Override));

DEFINE_HOOK(0x4870D0, CellClass_IsSensorsOfHouse_Allied, 0x6)
{
	GET(CellClass*, pThis, ECX);
	GET_STACK(int, houseIdx, 0x4);

	HouseClass* pQueryHouse = HouseClass::Array->GetItem(houseIdx);

	if (pQueryHouse->IsObserver())
	{
		R->AL(true);
		return 0x4870DE;
	}

	for (const HouseClass* pHouse : *HouseClass::Array)
	{
		if (pHouse->IsAlliedWith(pQueryHouse) && pThis->Sensors_InclHouse(pHouse->ArrayIndex))
		{
			R->AL(true);
			return 0x4870DE;
		}
	}

	R->AL(false);
	return 0x4870DE;
}
