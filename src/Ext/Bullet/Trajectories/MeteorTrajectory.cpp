#include "MeteorTrajectory.h"

#include <ScenarioClass.h>

#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>

#include <New/Entity/LaserTrailClass.h>

#include <Utilities/TemplateDef.h>

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
		;

	return true;
}

bool MeteorTrajectory::Save(PhobosStreamWriter& Stm) const
{
	this->PhobosTrajectory::Save(Stm);

	Stm
		;

	return true;
}

void MeteorTrajectory::OnUnlimbo(BulletClass* pBullet, CoordStruct* pCoord, BulletVelocity* pVelocity)
{
	int range = static_cast<int>(this->GetTrajectoryType<MeteorTrajectoryType>(pBullet)->Range * 256);
	double angel = ScenarioClass::Instance()->Random.RandomDouble() * Math::TwoPi;
	double length = ScenarioClass::Instance()->Random.RandomRanged(-range, range);

	CoordStruct SourceLocation;
	SourceLocation.X = pBullet->TargetCoords.X + static_cast<int>(length * Math::cos(angel));
	SourceLocation.Y = pBullet->TargetCoords.Y + static_cast<int>(length * Math::sin(angel));
	SourceLocation.Z = pBullet->TargetCoords.Z + static_cast<int>(this->GetTrajectoryType<MeteorTrajectoryType>(pBullet)->Height);

	if (pBullet->Type->Inaccurate)
	{
		auto const pTypeExt = BulletTypeExt::ExtMap.Find(pBullet->Type);

		int ballisticScatter = RulesClass::Instance()->BallisticScatter;
		int scatterMax = pTypeExt->BallisticScatter_Max.isset() ? (int)(pTypeExt->BallisticScatter_Max.Get()) : ballisticScatter;
		int scatterMin = pTypeExt->BallisticScatter_Min.isset() ? (int)(pTypeExt->BallisticScatter_Min.Get()) : (scatterMax / 2);

		double random = ScenarioClass::Instance()->Random.RandomRanged(scatterMin, scatterMax);
		double theta = ScenarioClass::Instance()->Random.RandomDouble() * Math::TwoPi;

		CoordStruct offset
		{
			static_cast<int>(random * Math::cos(theta)),
			static_cast<int>(random * Math::sin(theta)),
			0
		};
		pBullet->TargetCoords += offset;
	}

	pBullet->Limbo();
	pBullet->Unlimbo(SourceLocation, static_cast<DirType>(0));

	pBullet->Velocity.X = static_cast<double>(pBullet->TargetCoords.X - SourceLocation.X);
	pBullet->Velocity.Y = static_cast<double>(pBullet->TargetCoords.Y - SourceLocation.Y);
	pBullet->Velocity.Z = static_cast<double>(pBullet->TargetCoords.Z - SourceLocation.Z);
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
}

TrajectoryCheckReturnType MeteorTrajectory::OnAITargetCoordCheck(BulletClass* pBullet)
{
	return TrajectoryCheckReturnType::ExecuteGameCheck; // Execute game checks.
}

TrajectoryCheckReturnType MeteorTrajectory::OnAITechnoCheck(BulletClass* pBullet, TechnoClass* pTechno)
{
	return TrajectoryCheckReturnType::ExecuteGameCheck; // Execute game checks.
}
