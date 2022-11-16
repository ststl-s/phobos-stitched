#include "Body.h"

#include <Ext/Side/Body.h>


DEFINE_HOOK_AGAIN(0x522790, TechnoClass_DefaultDisguise, 0x6) // InfantryClass_SetDisguise_DefaultDisguise
DEFINE_HOOK(0x6F421C, TechnoClass_DefaultDisguise, 0x6) // TechnoClass_DefaultDisguise
{
	enum { SetDisguise = 0x5227BF, DefaultDisguise = 0x6F4277 };

	GET(TechnoClass*, pThis, ESI);

	if (auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType()))
	{
		const auto pUnit = abstract_cast<UnitClass*>(pThis);

		if (R->Origin() == 0x522790 || !pUnit)
		{
			if (pTypeExt->DefaultDisguise.isset())
			{
				pThis->Disguise = pTypeExt->DefaultDisguise;
				pThis->Disguised = true;
				return R->Origin() == 0x522790 ? SetDisguise : DefaultDisguise;
			}
		}
		else
		{
			const auto pSideExt = pThis->Owner ?
				SideExt::ExtMap.Find(abstract_cast<SideClass*>(SideClass::Array->GetItemOrDefault(pThis->Owner->SideIndex))) : nullptr;

			pThis->Disguise = pTypeExt->DefaultVehicleDisguise.Get(pSideExt->VehicleDisguise.Get(pUnit->Type));
			pThis->Disguised = true;
			return DefaultDisguise;
		}
	}

	pThis->Disguised = false;

	return 0;
}

DEFINE_HOOK(0x7466DC, UnitClass_DisguiseAs_DisguiseAsVehicle, 0x6)
{
	GET(UnitClass*, pThis, EDI);
	GET(UnitClass*, pTarget, ESI);

	const bool targetDisguised = pTarget->IsDisguised();
	pThis->Disguise = targetDisguised ? pTarget->GetDisguise(true) : pTarget->Type;
	pThis->DisguisedAsHouse = targetDisguised ? pTarget->GetDisguiseHouse(true) : pTarget->Owner;
	pThis->Disguised = true;
	pThis->DisguiseCreationFrame = Unsorted::CurrentFrame;
	pThis->RadarTrackingFlash();

	return 0x746712;
}

#define CAN_BLINK_DISGUISE(pTechno) \
RulesExt::Global()->ShowAllyDisguiseBlinking && (HouseClass::IsCurrentPlayerObserver() || pTechno->Owner->IsAlliedWith(HouseClass::CurrentPlayer))

DEFINE_HOOK(0x70EE53, TechnoClass_IsClearlyVisibleTo_BlinkAllyDisguise1, 0xA)
{
	enum { SkipGameCode = 0x70EE6A, Return = 0x70EEEC };

	GET(TechnoClass*, pThis, ESI);
	GET(int, accum, EAX);

	if (CAN_BLINK_DISGUISE(pThis))
		return SkipGameCode;
	else if (accum && !pThis->Owner->IsControlledByCurrentPlayer())
		return Return;

	return SkipGameCode;
}

DEFINE_HOOK(0x70EE6A, TechnoClass_IsClearlyVisibleTo_BlinkAllyDisguise2, 0x6)
{
	enum { SkipCheck = 0x70EE79 };

	GET(TechnoClass*, pThis, ESI);

	if (CAN_BLINK_DISGUISE(pThis))
		return SkipCheck;

	return 0;
}

DEFINE_HOOK(0x7062F5, TechnoClass_DrawObject_BlinkAllyDisguise, 0x6)
{
	enum { SkipCheck = 0x706304 };

	GET(TechnoClass*, pThis, ESI);

	if (CAN_BLINK_DISGUISE(pThis))
		return SkipCheck;

	return 0;
}

DEFINE_HOOK(0x70EDAD, TechnoClass_DisguiseBlitFlags_BlinkAllyDisguise, 0x6)
{
	enum { SkipCheck = 0x70EDBC };

	GET(TechnoClass*, pThis, EDI);

	if (CAN_BLINK_DISGUISE(pThis))
		return SkipCheck;

	return 0;
}

DEFINE_HOOK(0x7060A9, TechnoClass_DrawObject_DisguisePalette, 0x6)
{
	enum { SkipGameCode = 0x7060CA };

	GET(TechnoClass*, pThis, ESI);

	LightConvertClass* convert = nullptr;

	auto const pType = pThis->IsDisguised() ? TechnoTypeExt::GetTechnoType(pThis->Disguise) : nullptr;
	int colorIndex = pThis->GetDisguiseHouse(true)->ColorSchemeIndex;

	if (pType && pType->Palette && pType->Palette->Count > 0)
		convert = pType->Palette->GetItem(colorIndex)->LightConvert;
	else
		convert = ColorScheme::Array->GetItem(colorIndex)->LightConvert;

	R->EBX(convert);

	return SkipGameCode;
}
