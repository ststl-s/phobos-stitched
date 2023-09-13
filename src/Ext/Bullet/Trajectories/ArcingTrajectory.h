#pragma once

#include "PhobosTrajectory.h"

class ArcingTrajectoryType final : public PhobosTrajectoryType
{
public:
	ArcingTrajectoryType() : PhobosTrajectoryType(TrajectoryFlag::Arcing)
		, Elevation { 0.0 }
		, Lobber { false }
	{}

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual void Read(CCINIClass* const pINI, const char* pSection) override;

	Valueable<double> Elevation;
	Valueable<bool> Lobber;
};

class ArcingTrajectory final : public PhobosTrajectory
{
public:
	ArcingTrajectory() : PhobosTrajectory(TrajectoryFlag::Arcing)
		, OverRange { false }
	{}

	ArcingTrajectory(PhobosTrajectoryType* pType) : PhobosTrajectory(TrajectoryFlag::Arcing)
		, OverRange { false }
	{}

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual void OnUnlimbo(BulletClass* pBullet, CoordStruct* pCoord, BulletVelocity* pVelocity) override;
	virtual bool OnAI(BulletClass* pBullet) override;
	virtual void OnAIPreDetonate(BulletClass* pBullet) override;
	virtual void OnAIVelocity(BulletClass* pBullet, BulletVelocity* pSpeed, BulletVelocity* pPosition) override;
	virtual TrajectoryCheckReturnType OnAITargetCoordCheck(BulletClass* pBullet) override;
	virtual TrajectoryCheckReturnType OnAITechnoCheck(BulletClass* pBullet, TechnoClass* pTechno) override;

	bool OverRange;
};
