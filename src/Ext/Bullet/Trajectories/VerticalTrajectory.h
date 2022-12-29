#pragma once

#include "PhobosTrajectory.h"

class VerticalTrajectoryType final : public PhobosTrajectoryType
{
public:
	VerticalTrajectoryType() : PhobosTrajectoryType(TrajectoryFlag::Vertical)
		, Height { 10000.0 }
	{
	}

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual void Read(CCINIClass* const pINI, const char* pSection) override;

	Valueable<double> Height;
};

class VerticalTrajectory final : public PhobosTrajectory
{
public:
	VerticalTrajectory() : PhobosTrajectory(TrajectoryFlag::Vertical)
		, IsFalling { false }
		, Height { 0.0 }
	{
	}

	VerticalTrajectory(PhobosTrajectoryType* pType) : PhobosTrajectory(TrajectoryFlag::Vertical)
		, IsFalling { false }
		, Height { 0.0 }
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

	bool IsFalling = false;
	double Height = 0.0;
};
#pragma once
