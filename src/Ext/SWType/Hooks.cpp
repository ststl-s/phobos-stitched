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

DEFINE_HOOK(0x6CB5D2, SuperClass_Grant_AddToShowTimer, 0x9)
{
	GET(SuperClass*, pThis, ESI);

	enum { SkipGameCode = 0x6CB63E };

	if (pThis->Type->ShowTimer && !pThis->Owner->Type->MultiplayPassive)
	{
		SuperClass::ShowTimers->AddItem(pThis);

		const auto pTypeExt = SWTypeExt::ExtMap.Find(pThis->Type);
		int priority = pTypeExt->SW_Priority;
		int size = SuperClass::ShowTimers->Count;

		for (int i = 0; i < size; i++)
		{
			int otherPriority = SWTypeExt::ExtMap.Find(SuperClass::ShowTimers->GetItem(i)->Type)->SW_Priority;

			if (priority > otherPriority)
			{
				std::swap(SuperClass::ShowTimers->Items[i], SuperClass::ShowTimers->Items[size - 1]);

				for (int j = i + 1; j < size - 1; j++)
				{
					std::swap(SuperClass::ShowTimers->Items[j], SuperClass::ShowTimers->Items[size - 1]);
				}

				break;
			}
		}
	}

	return SkipGameCode;
}
