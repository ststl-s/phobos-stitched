#pragma once

#include "PhobosTrajectory.h"

class ArtilleryTrajectoryType final : public PhobosTrajectoryType
{
public:
	ArtilleryTrajectoryType() : PhobosTrajectoryType(TrajectoryFlag::Artillery)
		, MaxHeight { 2000.0 }
		, DistanceToHeight { true }
		, DistanceToHeight_Multiplier { 0.2 }
	{ }

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual void Read(CCINIClass* const pINI, const char* pSection) override;
	double MaxHeight;
	bool DistanceToHeight;
	double DistanceToHeight_Multiplier;
};

class ArtilleryTrajectory final : public PhobosTrajectory
{
public:
	ArtilleryTrajectory() : PhobosTrajectory(TrajectoryFlag::Artillery)
		, InitialTargetLocation { CoordStruct::Empty }
		, InitialSourceLocation { CoordStruct::Empty }
		, CenterLocation { CoordStruct::Empty }
		, Height { 0 }
		, Init { false }
	{}

	ArtilleryTrajectory(PhobosTrajectoryType* pType) : PhobosTrajectory(TrajectoryFlag::Artillery)
		, InitialTargetLocation { CoordStruct::Empty }
		, InitialSourceLocation { CoordStruct::Empty }
		, CenterLocation { CoordStruct::Empty }
		, Height { 0 }
		, Init { false }
	{}

	virtual bool Load(PhobosStreamReader& Stm, bool RegisterForChange) override;
	virtual bool Save(PhobosStreamWriter& Stm) const override;

	virtual void OnUnlimbo(BulletClass* pBullet, CoordStruct* pCoord, BulletVelocity* pVelocity) override;
	virtual bool OnAI(BulletClass* pBullet) override;
	virtual void OnAIPreDetonate(BulletClass* pBullet) override;
	virtual void OnAIVelocity(BulletClass* pBullet, BulletVelocity* pSpeed, BulletVelocity* pPosition) override;
	virtual TrajectoryCheckReturnType OnAITargetCoordCheck(BulletClass* pBullet) override;
	virtual TrajectoryCheckReturnType OnAITechnoCheck(BulletClass* pBullet, TechnoClass* pTechno) override;

	CoordStruct InitialTargetLocation;
	CoordStruct InitialSourceLocation;
	CoordStruct CenterLocation;
	double Height;
	bool Init;
};
