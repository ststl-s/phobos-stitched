#include "MeteorTrajectory.h"

#include <Ext/BulletType/Body.h>

bool MeteorTrajectoryType::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->PhobosTrajectoryType::Load(Stm, false);
	Stm
		.Process(this->Height, false)
		.Process(this->Range, false)
		;
	return true;
}

bool MeteorTrajectoryType::Save(PhobosStreamWriter& Stm) const
{
	this->PhobosTrajectoryType::Save(Stm);
	Stm
		.Process(this->Height)
		.Process(this->Range)
		;
	return true;
}

void MeteorTrajectoryType::Read(CCINIClass* const pINI, const char* pSection)
{
	if (!pINI->GetSection(pSection))
		return;

	this->PhobosTrajectoryType::Read(pINI, pSection);

	INI_EX exINI(pINI);

	this->Height.Read(exINI, pSection, "Trajectory.Meteor.Height");
	this->Range.Read(exINI, pSection, "Trajectory.Meteor.Range");
}

bool MeteorTrajectory::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->PhobosTrajectory::Load(Stm, false);

	Stm
		.Process(this->IsSet, false)
		;

	return true;
}

bool MeteorTrajectory::Save(PhobosStreamWriter& Stm) const
{
	this->PhobosTrajectory::Save(Stm);

	Stm
		.Process(this->IsSet)
		;

	return true;
}

void MeteorTrajectory::OnUnlimbo(BulletClass* pBullet, CoordStruct* pCoord, BulletVelocity* pVelocity)
{
	int range = static_cast<int>(this->GetTrajectoryType<MeteorTrajectoryType>(pBullet)->Range * 256);
	this->SourceLocation.X = pBullet->TargetCoords.X + ScenarioClass::Instance()->Random.RandomRanged(-range, range);
	this->SourceLocation.Y = pBullet->TargetCoords.Y + ScenarioClass::Instance()->Random.RandomRanged(-range, range);
	this->SourceLocation.Z = pBullet->TargetCoords.Z + static_cast<int>(this->GetTrajectoryType<MeteorTrajectoryType>(pBullet)->Height);

	pBullet->Velocity.X = static_cast<double>(pBullet->TargetCoords.X - this->SourceLocation.X);
	pBullet->Velocity.Y = static_cast<double>(pBullet->TargetCoords.Y - this->SourceLocation.Y);
	pBullet->Velocity.Z = static_cast<double>(pBullet->TargetCoords.Z - this->SourceLocation.Z);
	pBullet->Velocity *= this->GetTrajectorySpeed(pBullet) / pBullet->Velocity.Magnitude();
}

bool MeteorTrajectory::OnAI(BulletClass* pBullet)
{
	if (pBullet->TargetCoords.DistanceFrom(pBullet->Location) < this->DetonationDistance)
		return true;

	return false;
}

void MeteorTrajectory::OnAIPreDetonate(BulletClass* pBullet)
{
}

void MeteorTrajectory::OnAIVelocity(BulletClass* pBullet, BulletVelocity* pSpeed, BulletVelocity* pPosition)
{
	pSpeed->Z += BulletTypeExt::GetAdjustedGravity(pBullet->Type);
	if (!this->IsSet)
	{
		pBullet->Limbo();
		pBullet->Unlimbo(this->SourceLocation, static_cast<DirType>(0));
		pPosition->X = this->SourceLocation.X;
		pPosition->Y = this->SourceLocation.Y;
		pPosition->Z = this->SourceLocation.Z;
		this->IsSet = true;
	}

}

TrajectoryCheckReturnType MeteorTrajectory::OnAITargetCoordCheck(BulletClass* pBullet)
{
	return TrajectoryCheckReturnType::ExecuteGameCheck; // Execute game checks.
}

TrajectoryCheckReturnType MeteorTrajectory::OnAITechnoCheck(BulletClass* pBullet, TechnoClass* pTechno)
{
	return TrajectoryCheckReturnType::ExecuteGameCheck; // Execute game checks.
}
