#include "WaveTrajectory.h"

#include <ScenarioClass.h>

#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>

#include <New/Entity/LaserTrailClass.h>

#include <Utilities/TemplateDef.h>

bool WaveTrajectoryType::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->PhobosTrajectoryType::Load(Stm, false);
	Stm
		.Process(this->MaxHeight, false)
		.Process(this->MinHeight, false)
		;

	return true;
}

bool WaveTrajectoryType::Save(PhobosStreamWriter& Stm) const
{
	this->PhobosTrajectoryType::Save(Stm);
	Stm
		.Process(this->MaxHeight)
		.Process(this->MinHeight)
		;

	return true;
}


void WaveTrajectoryType::Read(CCINIClass* const pINI, const char* pSection)
{
	if (!pINI->GetSection(pSection))
		return;

	this->PhobosTrajectoryType::Read(pINI, pSection);

	INI_EX exINI(pINI);

	this->MaxHeight.Read(exINI, pSection, "Trajectory.Wave.MaxHeight");
	this->MinHeight.Read(exINI, pSection, "Trajectory.Wave.MinHeight");
}

bool WaveTrajectory::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->PhobosTrajectory::Load(Stm, false);

	Stm
		.Process(this->Fallen, false)
		;

	return true;
}

bool WaveTrajectory::Save(PhobosStreamWriter& Stm) const
{
	this->PhobosTrajectory::Save(Stm);

	Stm
		.Process(this->Fallen)
		;

	return true;
}

void WaveTrajectory::OnUnlimbo(BulletClass* pBullet, CoordStruct* pCoord, BulletVelocity* pVelocity)
{
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

	pBullet->Velocity.X = static_cast<double>(pBullet->TargetCoords.X - pBullet->SourceCoords.X);
	pBullet->Velocity.Y = static_cast<double>(pBullet->TargetCoords.Y - pBullet->SourceCoords.Y);
	pBullet->Velocity.Z = static_cast<double>(pBullet->TargetCoords.Z - pBullet->SourceCoords.Z);
	pBullet->Velocity *= this->GetTrajectorySpeed(pBullet) / pBullet->Velocity.Magnitude();
}

bool WaveTrajectory::OnAI(BulletClass* pBullet)
{
	if (!this->Fallen)
	{
		int zDelta = pBullet->TargetCoords.Z - pBullet->SourceCoords.Z;
		int maxHeight = this->GetTrajectoryType<WaveTrajectoryType>(pBullet)->MaxHeight + zDelta;
		int minHeight = this->GetTrajectoryType<WaveTrajectoryType>(pBullet)->MinHeight + zDelta;

		int currHeight = ScenarioClass::Instance->Random.RandomRanged(minHeight, maxHeight);

		if (currHeight != 0)
			pBullet->Location.Z = pBullet->SourceCoords.Z + currHeight;

		CoordStruct bullet = pBullet->Location;
		bullet.Z = 0;
		CoordStruct target = pBullet->TargetCoords;
		target.Z = 0;

		if (bullet.DistanceFrom(target) < 100)
		{
			CoordStruct end = { pBullet->TargetCoords.X, pBullet->TargetCoords.Y,pBullet->Location.Z };
			pBullet->SetLocation(end);
			this->Fallen = true;
		}
	}
	else
	{
		pBullet->SetLocation(pBullet->TargetCoords);
	}

	// If the projectile is close enough to the target then explode it
	double closeEnough = pBullet->TargetCoords.DistanceFrom(pBullet->Location);
	if (closeEnough < 100)
	{
		auto pBulletExt = BulletExt::ExtMap.Find(pBullet);

		if (pBulletExt && pBulletExt->LaserTrails.size())
			pBulletExt->LaserTrails.clear();

		return true;
	}
	return false;
}

void WaveTrajectory::OnAIPreDetonate(BulletClass* pBullet)
{
}

void WaveTrajectory::OnAIVelocity(BulletClass* pBullet, BulletVelocity* pSpeed, BulletVelocity* pPosition)
{
	pSpeed->Z += BulletTypeExt::GetAdjustedGravity(pBullet->Type); // We don't want to take the gravity into account
}

TrajectoryCheckReturnType WaveTrajectory::OnAITargetCoordCheck(BulletClass* pBullet)
{
	return TrajectoryCheckReturnType::ExecuteGameCheck; // Execute game checks.
}

TrajectoryCheckReturnType WaveTrajectory::OnAITechnoCheck(BulletClass* pBullet, TechnoClass* pTechno)
{
	return TrajectoryCheckReturnType::SkipGameCheck; // Bypass game checks entirely.
}
