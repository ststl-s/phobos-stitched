#pragma once

#include <Ext/WeaponType/Body.h>
#include <BulletClass.h>

/*
https://github.com/ChrisLv-CN/YRDynamicPatcher-Kratos/blob/main/DynamicPatcher/Projects/Extension/Kraotos/Utilities/WeaponHelper.cs
author: https://github.com/ChrisLv-CN
*/

class SimulatedFireHelper
{
public:
	static BulletVelocity GetBulletVelocity(const CoordStruct& crdSrc, const CoordStruct& crdDest);

	static void FireWeaponTo
	(
		TechnoClass* pAttacker,
		TechnoClass* pOwner,
		AbstractClass* pTarget,
		WeaponTypeClass* pWeapon,
		const CoordStruct& flh,
		const CoordStruct& crdBulletSrc = CoordStruct::Empty,
		bool radialFire = false,
		int splitAngle = 180
	);

	static BulletClass* FireBulletTo
	(
		TechnoClass* pAttacker,
		TechnoClass* pOwner,
		AbstractClass* pTarget,
		WeaponTypeClass* pWeapon,
		const CoordStruct& crdSrc,
		const CoordStruct& crdDest,
		const BulletVelocity& bulletVelocity
	);

	static Vector3D<float> GetFLHAbsoluteOffset
	(
		const CoordStruct& flh,
		const DirStruct& dir,
		const CoordStruct& turretOffset = CoordStruct::Empty
	);

	static CoordStruct GetFLHAbsoluteCoords
	(
		TechnoClass* pTechno,
		const CoordStruct& flh,
		bool isOnTurret,
		int flipY,
		const CoordStruct& turretOffset = CoordStruct::Empty
	);

	static CoordStruct GetFLHAbsoluteCoords
	(
		const CoordStruct& coords,
		const CoordStruct& flh,
		const DirStruct& dir,
		const CoordStruct& turretOffset = CoordStruct::Empty
	);
	
private:
	static BulletClass* FireBullet
	(
		TechnoClass* pOwner,
		AbstractClass* pTarget,
		WeaponTypeClass* pWeapon,
		const CoordStruct& crdSrc,
		const CoordStruct& crdDest,
		const BulletVelocity& bulletVelocity
	);

	static Vector3D<float> GetFLHOffset(Matrix3D& matrix, const CoordStruct& flh);

	static void DrawBulletEffect
	(
		WeaponTypeClass* pWeapon,
		const CoordStruct& crdSrc,
		const CoordStruct& crdDest,
		TechnoClass* pOwner,
		AbstractClass* pTarget
	);

	static void AttachedParticleSystem
	(
		WeaponTypeClass* pWeapon,
		TechnoClass* pOwner,
		AbstractClass* pTarget,
		const CoordStruct& crdSrc,
		const CoordStruct& crdDest	
	);

	static void PlayReportSound(WeaponTypeClass* pWeapon, const CoordStruct& crdSrc);

	static void DrawWeaponAnim
	(
		TechnoClass* pAttacker,
		WeaponTypeClass* pWeapon,
		const CoordStruct& crdSrc,
		const CoordStruct& crdDest
	);
};

struct LaserType
{
	ColorStruct InnerColor;
	ColorStruct OuterColor;
	ColorStruct OuterSpread;
	int Duration;
	int Thickness;
	bool IsHouseColor;
	bool IsSupported;
	bool Fade;

	LaserType() = default;

	LaserType
	(
		ColorStruct innerColor,
		ColorStruct outerColor,
		ColorStruct outerSpread,
		int duration,
		int thickness,
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
	{ }

	LaserType(const WeaponTypeClass* pWeapon)
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
};
