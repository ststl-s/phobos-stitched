#include "ArtilleryTrajectory.h"

#include <ScenarioClass.h>

#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>

#include <New/Entity/LaserTrailClass.h>

#include <Utilities/TemplateDef.h>

bool ArtilleryTrajectoryType::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->PhobosTrajectoryType::Load(Stm, false);

	Stm
		.Process(this->MaxHeight, false)
		.Process(this->DistanceToHeight, false)
		.Process(this->DistanceToHeight_Multiplier, false)
		;

	return true;
}

bool ArtilleryTrajectoryType::Save(PhobosStreamWriter& Stm) const
{
	this->PhobosTrajectoryType::Save(Stm);

	Stm
		.Process(this->MaxHeight)
		.Process(this->DistanceToHeight)
		.Process(this->DistanceToHeight_Multiplier)
		;

	return true;
}

void ArtilleryTrajectoryType::Read(CCINIClass* const pINI, const char* pSection)
{
	if (!pINI->GetSection(pSection))
		return;

	this->PhobosTrajectoryType::Read(pINI, pSection);

	INI_EX exINI(pINI);

	this->MaxHeight.Read(exINI, pSection, "Trajectory.Artillery.MaxHeight");
	this->DistanceToHeight.Read(exINI, pSection, "Trajectory.Artillery.DistanceToHeight");
	this->DistanceToHeight_Multiplier.Read(exINI, pSection, "Trajectory.Artillery.Multiplier");
}

bool ArtilleryTrajectory::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->PhobosTrajectory::Load(Stm, false);

	Stm
		.Process(this->InitialSourceLocation, false)
		.Process(this->FullDistance, false)
		.Process(this->A, false)
		.Process(this->B, false)
		.Process(this->C, false)
		;

	return true;
}

bool ArtilleryTrajectory::Save(PhobosStreamWriter& Stm) const
{
	this->PhobosTrajectory::Save(Stm);

	Stm
		.Process(this->InitialSourceLocation)
		.Process(this->FullDistance)
		.Process(this->A)
		.Process(this->B)
		.Process(this->C)
		;

	return true;
}

void ArtilleryTrajectory::OnUnlimbo(BulletClass* pBullet, CoordStruct* pCoord, BulletVelocity* pVelocity)
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

	auto InitialTargetLocation = pBullet->TargetCoords;
	InitialTargetLocation.Z = 0;

	this->InitialSourceLocation = pBullet->SourceCoords;
	this->InitialSourceLocation.Z = 0;

	this->FullDistance = InitialTargetLocation.DistanceFrom(this->InitialSourceLocation);

	double Height = 0;

	if (this->GetTrajectoryType<ArtilleryTrajectoryType>(pBullet)->DistanceToHeight)
	{
		double height = (this->GetTrajectoryType<ArtilleryTrajectoryType>(pBullet)->DistanceToHeight_Multiplier * this->FullDistance) + pBullet->TargetCoords.Z;

		if (this->GetTrajectoryType<ArtilleryTrajectoryType>(pBullet)->MaxHeight > 0 && height > (this->GetTrajectoryType<ArtilleryTrajectoryType>(pBullet)->MaxHeight + pBullet->TargetCoords.Z))
			height = this->GetTrajectoryType<ArtilleryTrajectoryType>(pBullet)->MaxHeight + pBullet->TargetCoords.Z;

		if (height > pBullet->SourceCoords.Z)
			Height = height;
	}
	else
	{
		Height = this->GetTrajectoryType<ArtilleryTrajectoryType>(pBullet)->MaxHeight + pBullet->TargetCoords.Z;
	}

	if (Height < pBullet->SourceCoords.Z)
		Height = pBullet->SourceCoords.Z;

	this->C = pBullet->SourceCoords.Z;
	this->A = (2 * (pBullet->TargetCoords.Z - (2 * Height) + this->C)) / pow(this->FullDistance, 2);
	this->B = ((4 * Height) - pBullet->TargetCoords.Z - (3 * this->C)) / this->FullDistance;

	pBullet->Velocity.X = static_cast<double>(pBullet->TargetCoords.X - pBullet->SourceCoords.X);
	pBullet->Velocity.Y = static_cast<double>(pBullet->TargetCoords.Y - pBullet->SourceCoords.Y);
	pBullet->Velocity.Z = static_cast<double>(this->B * (this->FullDistance / 2) + this->C);
	pBullet->Velocity *= this->GetTrajectorySpeed(pBullet) / pBullet->Velocity.Magnitude();
}

bool ArtilleryTrajectory::OnAI(BulletClass* pBullet)
{
	if (!BulletExt::ExtMap.Find(pBullet)->Interfered)
	{
		CoordStruct bulletCoords = pBullet->Location;
		bulletCoords.Z = 0;
		double currentBulletDistance = this->InitialSourceLocation.DistanceFrom(bulletCoords);

		double currHeight = this->A * pow(currentBulletDistance, 2) + this->B * currentBulletDistance + this->C;
		pBullet->Location.Z = static_cast<int>(currHeight);
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

void ArtilleryTrajectory::OnAIPreDetonate(BulletClass* pBullet)
{
	if (!BulletExt::ExtMap.Find(pBullet)->Interfered)
	{
		CoordStruct bulletCoords = pBullet->Location;
		bulletCoords.Z = 0;
		if (this->InitialSourceLocation.DistanceFrom(bulletCoords) > this->FullDistance)
		{
			pBullet->SetLocation(pBullet->TargetCoords);
		}
	}
}

void ArtilleryTrajectory::OnAIVelocity(BulletClass* pBullet, BulletVelocity* pSpeed, BulletVelocity* pPosition)
{
	pSpeed->Z += BulletTypeExt::GetAdjustedGravity(pBullet->Type); // We don't want to take the gravity into account
}

TrajectoryCheckReturnType ArtilleryTrajectory::OnAITargetCoordCheck(BulletClass* pBullet)
{
	return TrajectoryCheckReturnType::ExecuteGameCheck; // Execute game checks.
}

TrajectoryCheckReturnType ArtilleryTrajectory::OnAITechnoCheck(BulletClass* pBullet, TechnoClass* pTechno)
{
	return TrajectoryCheckReturnType::SkipGameCheck; // Bypass game checks entirely.
}
