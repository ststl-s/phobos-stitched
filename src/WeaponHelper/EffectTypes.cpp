#include "EffectTypes.h"

#include <Ext/WeaponType/Body.h>

WeaponLaserType::WeaponLaserType
(
		ColorStruct innerColor,
		ColorStruct outerColor,
		int duration,
		int thickness,
		ColorStruct outerSpread,
		bool isHouseColor,
		bool isSupported,
		bool fade
)
	: InnerColor(innerColor)
	, OuterColor(outerColor)
	, OuterSpread(outerSpread)
	, Duration(duration)
	, Thickness(thickness)
	, IsHouseColor(isHouseColor)
	, IsSupported(isSupported)
	, Fade(fade)
{
}

WeaponLaserType::WeaponLaserType(const WeaponTypeClass* pWeapon)
	: InnerColor(pWeapon->LaserInnerColor)
	, OuterColor(pWeapon->LaserOuterColor)
	, OuterSpread(pWeapon->LaserOuterSpread)
	, Duration(pWeapon->LaserDuration)
	, IsHouseColor(pWeapon->IsHouseColor)
{
	if (auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon))
	{
		this->Thickness = pWeaponExt->LaserThickness;
		this->IsSupported = pWeaponExt->IsSupported;
		this->Fade = pWeaponExt->LaserFade;
	}
}
