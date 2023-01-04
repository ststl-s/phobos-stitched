#pragma once

#include "PhobosTrajectory.h"

class WaveTrajectoryType final : public PhobosTrajectoryType
{
public:
	WaveTrajectoryType() : PhobosTrajectoryType(TrajectoryFlag::Wave)
		, MaxHeight { 2000 }
		, MinHeight { 0 }
	{
	}

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual void Read(CCINIClass* const pINI, const char* pSection) override;
	Valueable<int> MaxHeight;
	Valueable<int> MinHeight;
};

class WaveTrajectory final : public PhobosTrajectory
{
public:
	WaveTrajectory() : PhobosTrajectory(TrajectoryFlag::Wave)
		, Fallen { false }
	{
	}

	WaveTrajectory(PhobosTrajectoryType* pType) : PhobosTrajectory(TrajectoryFlag::Wave)
		, Fallen { false }
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

	bool Fallen = false;
};
