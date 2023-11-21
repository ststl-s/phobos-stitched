#include "Body.h"

#include <HouseClass.h>

#include <Utilities/Macro.h>

void __fastcall HouseClass_UpdateSuperWeaponOwned(HouseClass* pThis)
{
	pThis->UpdateSuperWeaponOwned();
}

DEFINE_JUMP(CALL, 0x43BEF0, GET_OFFSET(HouseClass_UpdateSuperWeaponOwned));
DEFINE_JUMP(CALL, 0x451700, GET_OFFSET(HouseClass_UpdateSuperWeaponOwned));
DEFINE_JUMP(CALL, 0x451739, GET_OFFSET(HouseClass_UpdateSuperWeaponOwned));
DEFINE_JUMP(CALL, 0x4F92F6, GET_OFFSET(HouseClass_UpdateSuperWeaponOwned));
DEFINE_JUMP(CALL, 0x508DDB, GET_OFFSET(HouseClass_UpdateSuperWeaponOwned));

void __fastcall HouseClass_UpdateSuperWeaponUnavailable(HouseClass* pThis)
{
	pThis->UpdateSuperWeaponUnavailable();
}

DEFINE_JUMP(CALL, 0x4409EF, GET_OFFSET(HouseClass_UpdateSuperWeaponUnavailable));
DEFINE_JUMP(CALL, 0x4F92FD, GET_OFFSET(HouseClass_UpdateSuperWeaponUnavailable));
