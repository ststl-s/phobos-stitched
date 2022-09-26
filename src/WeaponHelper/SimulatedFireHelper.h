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
