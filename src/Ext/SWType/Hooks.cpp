#include "Body.h"

#include <SuperClass.h>
#include <BuildingClass.h>
#include <HouseClass.h>
#include <TechnoClass.h>

DEFINE_HOOK(0x6CDE40, SuperClass_Place, 0x5)
{
	GET(SuperClass* const, pSuper, ECX);
	GET_STACK(CoordStruct const, coords, 0x230); // I think?

	if (auto const pSWExt = SWTypeExt::ExtMap.Find(pSuper->Type))
    {
		pSWExt->FireSuperWeapon(pSuper, pSuper->Owner, coords);

		pSWExt->FireSuperWeaponAnim(pSuper, pSuper->Owner);
    }
	return 0;
}