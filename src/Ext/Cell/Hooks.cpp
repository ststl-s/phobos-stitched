#include "Body.h"

#include <Utilities/Macro.h>

CoordStruct* __fastcall SuperOn_CellClass_GetCenterCoords_Override(CellClass* pCell, DWORD _, CoordStruct& buffer)
{
	buffer = pCell->GetCoordsWithBridge();
	buffer.X += 126;
	buffer.Y += 126;
	return &buffer;
}

DEFINE_JUMP(CALL, 0x60C201, GET_OFFSET(SuperOn_CellClass_GetCenterCoords_Override));
