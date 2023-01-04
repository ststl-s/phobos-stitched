#pragma once

#include "PhobosTrajectory.h"

class SpiralTrajectoryType final : public PhobosTrajectoryType
{
public:
	SpiralTrajectoryType() : PhobosTrajectoryType(TrajectoryFlag::Spiral)
		, MaxRadius { 128 }
		, Length { 256 }
		, Angel { 30 }
	{
	}

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual void Read(CCINIClass* const pINI, const char* pSection) override;
	double MaxRadius;
	double Length;
	double Angel;
};

class SpiralTrajectory final : public PhobosTrajectory
{
public:
	SpiralTrajectory() : PhobosTrajectory(TrajectoryFlag::Spiral)
		, CenterLocation { CoordStruct::Empty }
		, DirectionAngel { 0.0 }
		, CurrentRadius { 0.0 }
		, CurrentAngel { 0.0 }
		, close { false }
	{
	}

	SpiralTrajectory(PhobosTrajectoryType* pType) : PhobosTrajectory(TrajectoryFlag::Spiral)
		, CenterLocation { CoordStruct::Empty }
		, DirectionAngel { 0.0 }
		, CurrentRadius { 0.0 }
		, CurrentAngel { 0.0 }
		, close { false }
	{
	}

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual void OnUnlimbo(BulletClass* pBullet, CoordStruct* pCoord, BulletVelocity* pVelocity) override;
	virtual bool OnAI(BulletClass* pBullet) override;
	virtual void OnAIPreDetonate(BulletClass* pBullet) override;
	virtual void OnAIVelocity(BulletClass* pBullet, BulletVelocity* pSpeed, BulletVelocity* pPosition) override;
	virtual TrajectoryCheckReturnType OnAITargetCoordCheck(BulletClass* pBullet) override;
	virtual TrajectoryCheckReturnType OnAITechnoCheck(BulletClass* pBullet, TechnoClass* pTechno) override;

	CoordStruct CenterLocation;
	double DirectionAngel;
	double CurrentRadius;
	double CurrentAngel;
	bool close;
};
