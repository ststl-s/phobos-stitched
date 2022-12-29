#pragma once

#include "PhobosTrajectory.h"

class MeteorTrajectoryType final : public PhobosTrajectoryType
{
public:
	MeteorTrajectoryType() : PhobosTrajectoryType(TrajectoryFlag::Meteor)
		, Height { 10000.0 }
		, Range { 30.0 }
	{
	}

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual void Read(CCINIClass* const pINI, const char* pSection) override;

	Valueable<double> Height;
	Valueable<double> Range;
};

class MeteorTrajectory final : public PhobosTrajectory
{
public:
	MeteorTrajectory() : PhobosTrajectory(TrajectoryFlag::Meteor)
		, IsSet { false }
		, SourceLocation { CoordStruct::Empty }
	{
	}

	MeteorTrajectory(PhobosTrajectoryType* pType) : PhobosTrajectory(TrajectoryFlag::Meteor)
		, IsSet { false }
		, SourceLocation { CoordStruct::Empty }
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

	bool IsSet = false;
	CoordStruct SourceLocation;
};
#pragma once
