#include "Body.h"

#include <SuperClass.h>
#include <BuildingClass.h>
#include <HouseClass.h>
#include <TechnoClass.h>

#include <Misc/PhobosGlobal.h>

DEFINE_HOOK(0x6CC390, SuperClass_Launch, 0x6)
{
	GET(SuperClass* const, pSuper, ECX);
	GET_STACK(CellStruct const* const, pCell, 0x4);
	GET_STACK(bool const, isPlayer, 0x8);

	Debug::Log("[Phobos Launch] %s\n", pSuper->Type->get_ID());

	auto const handled = SWTypeExt::Activate(pSuper, *pCell, isPlayer);

	return handled ? 0x6CDE40 : 0;
}

//Ares hooked at 0x6CC390 and jumped to 0x6CDE40
// If a super is not handled by Ares however, we do it at the original entry point
DEFINE_HOOK_AGAIN(0x6CC390, SuperClass_Place_FireExt, 0x6)
DEFINE_HOOK(0x6CDE40, SuperClass_Place_FireExt, 0x4)
{
	GET(SuperClass* const, pSuper, ECX);
	GET_STACK(CellStruct const* const, pCell, 0x4);
	GET_STACK(bool const, isPlayer, 0x8);

	SWTypeExt::FireSuperWeaponExt(pSuper, *pCell);

	return 0;
}

DEFINE_HOOK(0x50B1CA, SuperClass_After_Update, 0x6)
{
	PhobosGlobal::Global()->CheckSuperQueued();

	return 0;
}
