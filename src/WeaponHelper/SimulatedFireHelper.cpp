#include "SimulatedFireHelper.h"

#include "RadialFireHelper.h"

#include <Ext/Techno/Body.h>

BulletVelocity SimulatedFireHelper::GetBulletVelocity(const CoordStruct& crdSrc, const CoordStruct& crdDest)
{
	CoordStruct bulletFLH(1, 0, 0);
	DirStruct bulletDir(crdSrc, crdDest);
	Vector3D<float> bulletVelocity = GetFLHAbsoluteOffset(bulletFLH, bulletDir);

	return BulletVelocity(bulletVelocity.X, bulletVelocity.Y, bulletVelocity.Z);
}

Vector3D<float> SimulatedFireHelper::GetFLHAbsoluteOffset(const CoordStruct& flh, const DirStruct& dir, const CoordStruct& turretOffset)
{
	Vector3D<float> offset = Vector3D<float>::Empty;

	if (flh != CoordStruct::Empty)
	{
		Matrix3D matrix = Matrix3D();
		matrix.MakeIdentity();
		matrix.Translate(turretOffset.X, turretOffset.Y, turretOffset.Z);
		matrix.RotateZ(dir.radians());
		offset = GetFLHOffset(matrix, flh);
	}

	return offset;
}

Vector3D<float> SimulatedFireHelper::GetFLHOffset(Matrix3D& matrix, const CoordStruct& flh)
{
	// Step 4: apply FLH offset
	matrix.Translate(flh.X, flh.Y, flh.Z);
	Vector3D<float> result = Vector3D<float>::Empty;
	Matrix3D::MatrixMultiply(matrix, result);
	// Resulting FLH is mirrored along X axis, so we mirror it back - Kerbiter
	result.Y *= -1;

	return result;
}

CoordStruct SimulatedFireHelper::GetFLHAbsoluteCoords
(
	const CoordStruct& coords,
	const CoordStruct& flh,
	const DirStruct& dir,
	const CoordStruct& turretOffset = CoordStruct::Empty
)
{
	CoordStruct res = coords;

	if (flh != CoordStruct::Empty)
	{
		Vector3D<float> offset = GetFLHAbsoluteOffset(flh, dir, turretOffset);
		res += CoordStruct(offset.X, offset.Y, offset.Z);
	}

	return res;
}

CoordStruct SimulatedFireHelper::GetFLHAbsoluteCoords(TechnoClass* pTechno, const CoordStruct& flh, bool isOnTurret, int flipY, const CoordStruct& turretOffset)
{
	if (pTechno->WhatAmI() == AbstractType::Building)
	{
		// 建筑不能使用矩阵方法测算FLH
		return GetFLHAbsoluteCoords(pTechno->GetCoords(), flh, pTechno->PrimaryFacing.current(), turretOffset);
	}
	else
	{
		return TechnoExt::GetFLHAbsoluteCoords(pTechno, flh, isOnTurret);
	}
}

void SimulatedFireHelper::FireWeaponTo
(
	TechnoClass* pAttacker,
	TechnoClass* pOwner,
	AbstractClass* pTarget,
	WeaponTypeClass* pWeapon,
	const CoordStruct& flh,
	const CoordStruct& crdBulletSrc=CoordStruct::Empty,
	bool radialFire = false,
	int splitAngle = 180
)
{
	if (!TechnoExt::IsReallyAlive(pAttacker) || pTarget == nullptr || pWeapon == nullptr)
		return;
	
	CoordStruct crdDest = pTarget->GetCoords();
	// radial fire
	int burst = pWeapon->Burst;
	RadialFireHelper radialFireHelper(pAttacker, burst, splitAngle);
	int flipY = -1;
	for (int i = 0; i < burst; i++)
	{
		BulletVelocity bulletVelocity = BulletVelocity::Empty;

		if (radialFire)
		{
			flipY = (i < burst / 2.0) ? -1 : 1;
			bulletVelocity = radialFireHelper.GetBulletVelocity(i);
		}
		else
		{
			flipY *= -1;
		}

		CoordStruct crdSrc = crdBulletSrc == CoordStruct::Empty ? GetFLHAbsoluteCoords(pAttacker, flh, true, flipY) : crdBulletSrc;
		
		if (bulletVelocity == BulletVelocity::Empty)
		{
			bulletVelocity = GetBulletVelocity(crdSrc, crdDest);
		}

		BulletClass* pBullet = FireBulletTo(pAttacker, pOwner, pTarget, pWeapon, crdSrc, crdDest, bulletVelocity);
	}
}

BulletClass* SimulatedFireHelper::FireBulletTo
(
	TechnoClass* pAttacker,
	TechnoClass* pOwner,
	AbstractClass* pTarget,
	WeaponTypeClass* pWeapon,
	const CoordStruct& crdSrc,
	const CoordStruct& crdDest,
	const BulletVelocity& bulletVelocity
)
{
	if (pTarget == nullptr)
		return;

	if ((pTarget->AbstractFlags & AbstractFlags::Techno) && !TechnoExt::IsReallyAlive(static_cast<TechnoClass*>(pTarget)))
		return nullptr;

	// Fire weapon
	BulletClass* pBullet = FireBullet(pOwner, pTarget, pWeapon, crdSrc, crdDest, bulletVelocity);
	// Draw bullet effect
	DrawBulletEffect(pWeapon, crdSrc, crdDest, pOwner, pTarget);
	// Draw particle system
	AttachedParticleSystem(pWeapon, pOwner, pTarget, crdSrc, crdDest);
	// Play report sound
	PlayReportSound(pWeapon, crdSrc);
	// Draw weapon anim
	DrawWeaponAnim(pAttacker, pWeapon, crdSrc, crdDest);
	return pBullet;
}

BulletClass* FireBullet
	(
		TechnoClass* pOwner,
		AbstractClass* pTarget,
		WeaponTypeClass* pWeapon,
		const CoordStruct& crdSrc,
		const CoordStruct& crdDest,
		const BulletVelocity& bulletVelocity
	)
{
	double fireMult = 1;

	if (TechnoExt::IsReallyAlive(pOwner))
	{
		// check spawner
		if (pWeapon->Spawner && pOwner->SpawnManager != nullptr)
		{
			pOwner->SpawnManager->SetTarget(pTarget);

			return nullptr;
		}

		// check Abilities FIREPOWER
		fireMult = TechnoExt::GetDamageMultiplier(pOwner);
	}

	int damage = Game::F2I(pWeapon->Damage * fireMult);
	WarheadTypeClass* pWH = pWeapon->Warhead;
	int speed = pWeapon->GetSpeed(crdSrc.DistanceFrom(crdDest));
	bool bright = pWeapon->Bright; // 原游戏中弹头上的bright是无效的
	BulletClass* pBullet = nullptr;
	// 自己不能发射武器朝向自己
	// Pointer<TechnoClass> pRealAttacker = pTarget.Value != pAttacker.Value ? pAttacker : IntPtr.Zero;
	pBullet = pWeapon->Projectile->CreateBullet(pTarget, pOwner, damage, pWH, speed, bright);
	pBullet->WeaponType = pWeapon;
	// Logger.Log("{0}发射武器{1}，创建抛射体，目标类型{2}", pAttacker, pWeapon.Ref.Base.ID, pTarget.Ref.WhatAmI());

	// pBullet.Ref.SetTarget(pTarget);
	pBullet->MoveTo(crdSrc, bulletVelocity);

	if (pWeapon->Projectile->Inviso && !pWeapon->Projectile->Airburst)
	{
		pBullet->Detonate(crdDest);
		pBullet->UnInit();
	}

	return pBullet;
}

void SimulatedFireHelper::DrawBulletEffect
(
		WeaponTypeClass* pWeapon,
		const CoordStruct& crdSrc,
		const CoordStruct& crdDest,
		TechnoClass* pOwner,
		AbstractClass* pTarget
)
{
	//Laser
	if (pWeapon->IsLaser)
	{

		LaserType laserType(pWeapon);
		ColorStruct houseColor { 0,0,0 };
		if (pWeapon->IsHouseColor && pOwner != nullptr)
		{
			houseColor = pOwner->Owner->LaserColor;
		}
		
		BulletEffectHelper.DrawLine(sourcePos, targetPos, laserType, houseColor);
	}
	// IsRadBeam
	if (pWeapon.Ref.IsRadBeam)
	{
		RadBeamType radBeamType = RadBeamType.RadBeam;
		if (!pWeapon.Ref.Warhead.IsNull && pWeapon.Ref.Warhead.Ref.Temporal)
		{
			radBeamType = RadBeamType.Temporal;
		}
		BeamType beamType = new BeamType(radBeamType);
		BulletEffectHelper.DrawBeam(sourcePos, targetPos, beamType);
		// RadBeamType beamType = RadBeamType.RadBeam;
		// ColorStruct beamColor = RulesClass.Global().RadColor;
		// if (!pWeapon.Ref.Warhead.IsNull && pWeapon.Ref.Warhead.Ref.Temporal)
		// {
		//     beamType = RadBeamType.Temporal;
		//     beamColor = RulesClass.Global().ChronoBeamColor;
		// }
		// Pointer<RadBeam> pRadBeam = RadBeam.Allocate(beamType);
		// if (!pRadBeam.IsNull)
		// {
		//     pRadBeam.Ref.SetCoordsSource(sourcePos);
		//     pRadBeam.Ref.SetCoordsTarget(targetPos);
		//     pRadBeam.Ref.Color = beamColor;
		//     pRadBeam.Ref.Period = 15;
		//     pRadBeam.Ref.Amplitude = 40.0;
		// }
	}
	//IsElectricBolt
	if (pWeapon.Ref.IsElectricBolt)
	{
		if (!pAttacker.IsNull && !pTarget.IsNull)
		{
			BulletEffectHelper.DrawBolt(pAttacker, pTarget, pWeapon, sourcePos);
		}
		else
		{
			BulletEffectHelper.DrawBolt(sourcePos, targetPos, pWeapon.Ref.IsAlternateColor);
		}
	}
}
