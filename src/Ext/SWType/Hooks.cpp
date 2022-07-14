#include "Body.h"

#include <SuperClass.h>
#include <BuildingClass.h>
#include <HouseClass.h>
#include <TechnoClass.h>

DEFINE_HOOK(0x6CC390, SuperClass_Launch, 0x6)
{
	GET(SuperClass* const, pSuper, ECX);
	GET_STACK(CellStruct const* const, pCell, 0x4);
	GET_STACK(bool const, isPlayer, 0x8);

	Debug::Log("[Phobos Launch] %s\n", pSuper->Type->get_ID());

	auto const handled = SWTypeExt::Activate(pSuper, *pCell, isPlayer);

	return handled ? 0x6CDE40 : 0;
}

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

//const BYTE Ignore[] =
//{
//	0x8B, 0x86, 0x20, 0x05, 0x00, 0x00,	// mov eax, [esi+520h]
//	0xEB, 0xA4, 0x0F, 0x45, 0x00		// jmp 0x450FA4
//};
//
//DEFINE_HOOK(0x450F37, BuildingClass_ProcessAnims_SuperWeaponA_IgnoreAres1, 0x9)
//{
//	GET(BuildingClass*, pThis, ESI);
//	GET(BuildingAnimStruct*, v43, EAX);
//
//	if (v43 && v43->Anim[0])
//		return 0x450F40;
//
//	R->EAX(pThis->Type);
//	return 0x450FA4;
//}
//
//DEFINE_HOOK(0x450F49, BuildingClass_ProcessAnims_SuperWeaponA_IgnoreAres2, 0x6)
//{
//	GET(BuildingClass*, pThis, ESI);
//	GET(int, v36, EAX);
//
//	if (v36 >= 0)
//	{
//		R->ECX(R->ESI());
//		return 0x450F4F;
//	}
//
//	R->EAX(pThis->Type);
//	return 0x450FA4;
//}
//
//DEFINE_HOOK(0x450F86, BuildingClass_ProcessAnims_SuperWeaponA_IgnoreAres3, 0x5)
//{
//	GET(BuildingClass*, pThis, ESI);
//	GET(BuildingAnimStruct*, v45, EAX);
//
//	if (v45 && v45->Anim[0])
//		return 0x450F8F;
//
//	return 0x450FA4;
//}
//
//DEFINE_HOOK(0x451069, BuildingClass_ProcessAnims_SuperWeaponB_IgnoreAres1, 0x8)
//{
//	GET(BuildingClass*, pThis, ESI);
//
//	if (pThis->Anims[14])
//		return 0x451071;
//
//	R->EAX(pThis->Owner);
//	return 0x451138;
//}
//
//DEFINE_HOOK(0x4510BC, BuildingClass_ProcessAnims_SuperWeaponB_IgnoreAres2, 0x9)
//{
//	GET(BuildingClass*, pThis, ESI);
//	GET(BuildingAnimStruct*, v53, EAX);
//
//	if (v53 && v53->Anim[0])
//		return 0x4510C5;
//
//	R->EAX(pThis->Owner);
//	return 0x451138;
//}
//
//DEFINE_HOOK(0x4510CE, BuildingClass_ProcessAnims_SuperWeaponB_IgnoreAres3, 0xA)
//{
//	GET(BuildingClass*, pThis, ESI);
//
//	if (pThis->Anims[16])
//		return 0x4510D8;
//
//	R->EAX(pThis->Owner);
//	return 0x451138;
//}
//
//DEFINE_HOOK(0x45111A, BuildingClass_ProcessAnims_SuperWeaponB_IgnoreAres4, 0x9)
//{
//	GET(BuildingClass*, pThis, ESI);
//	GET(BuildingAnimStruct*, v55, EAX);
//
//	if (v55 && v55->Anim[0])
//		return 0x451123;
//
//	R->EAX(pThis->Owner);
//	return 0x451138;
//}

HMODULE AresHandle = GetModuleHandle("Ares.dll");
