#include "Body.h"

#include <Helpers/Macro.h>

#include <CellClass.h>
#include <ConvertClass.h>

DEFINE_HOOK(0x544E70, IsometricTileTypeClass_Init_Drawer, 0x8)
{
	GET(CellClass*, pCell, ESI); // Luckily, pCell is just ESI, so we don't need other hooks to set it

	GET(int, Red, ECX);
	GET(int, Green, EDX);
	GET_STACK(int, Blue, 0x4);

	auto pData = IsometricTileTypeExt::ExtMap.Find(IsometricTileTypeClass::Array->GetItemOrDefault(pCell->IsoTileTypeIndex));
	if (pData != nullptr)
	{
		LightConvertClass* tmp = IsometricTileTypeExt::InitDrawer(IsometricTileTypeExt::LoadedPalettesLookUp[pData->Tileset], Red, Green, Blue);
		R->EAX(tmp);
		return 0x544FDE;
	}
	return 0;
}

DEBUG_HOOK(0x548DC1, EIP_0x548DC6, 0x5)
{
	Debug::Log("0xAA10C0[0x%X],EAX[0x%X],EDX[%d]\n", *reinterpret_cast<int*>(0xAA10C0), R->EAX(), R->EDX());
	return 0;
}