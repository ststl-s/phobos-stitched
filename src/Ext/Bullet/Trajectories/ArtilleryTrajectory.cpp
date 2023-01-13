#include "ArtilleryTrajectory.h"
#include <Ext/BulletType/Body.h>
#include <Ext/Bullet/Body.h>
#include <ScenarioClass.h>

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
	this->MaxHeight = pINI->ReadDouble(pSection, "Trajectory.Artillery.MaxHeight", 2000);
	this->DistanceToHeight = pINI->ReadDouble(pSection, "Trajectory.Artillery.DistanceToHeight", true);
	this->DistanceToHeight_Multiplier = pINI->ReadDouble(pSection, "Trajectory.Artillery.Multiplier", 0.2);
}

bool ArtilleryTrajectory::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->PhobosTrajectory::Load(Stm, false);

	Stm
		.Process(this->InitialTargetLocation, false)
		.Process(this->InitialSourceLocation, false)
		.Process(this->CenterLocation, false)
		.Process(this->Height, false)
		.Process(this->Init, false)
		;

	return true;
}

bool ArtilleryTrajectory::Save(PhobosStreamWriter& Stm) const
{
	this->PhobosTrajectory::Save(Stm);

	Stm
		.Process(this->InitialTargetLocation)
		.Process(this->InitialSourceLocation)
		.Process(this->CenterLocation)
		.Process(this->Height)
		.Process(this->Init)
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

	this->InitialTargetLocation = pBullet->TargetCoords;
	this->InitialSourceLocation = pBullet->SourceCoords;
	this->CenterLocation = pBullet->Location;

	this->Height = this->GetTrajectoryType<ArtilleryTrajectoryType>(pBullet)->MaxHeight + this->InitialTargetLocation.Z;

	if (this->Height < pBullet->SourceCoords.Z)
		this->Height = pBullet->SourceCoords.Z;

	CoordStruct TempTargetLocation = pBullet->TargetCoords;
	TempTargetLocation.Z = 0;
	CoordStruct TempSourceLocation = pBullet->SourceCoords;
	TempSourceLocation.Z = 0;
	double distance = TempTargetLocation.DistanceFrom(TempSourceLocation);
	double fix = (((this->Height - pBullet->TargetCoords.Z) * distance) / (2 * this->Height - pBullet->SourceCoords.Z - pBullet->TargetCoords.Z)) / (this->Height - pBullet->TargetCoords.Z);

	double DirectionAngel = Math::atan2(pBullet->TargetCoords.Y - pBullet->SourceCoords.Y, pBullet->TargetCoords.X - pBullet->SourceCoords.X) + (Math::Pi / 2);

	this->InitialTargetLocation.X += static_cast<int>((pBullet->TargetCoords.Z * fix * Math::cos(DirectionAngel)));
	this->InitialTargetLocation.Y += static_cast<int>((pBullet->TargetCoords.Z * fix * Math::sin(DirectionAngel)));
	this->InitialTargetLocation.Z = 0;

	this->InitialSourceLocation.X -= static_cast<int>((pBullet->SourceCoords.Z * fix * Math::cos(DirectionAngel)));
	this->InitialSourceLocation.Y -= static_cast<int>((pBullet->SourceCoords.Z * fix * Math::sin(DirectionAngel)));
	this->InitialSourceLocation.Z = 0;

	pBullet->Velocity.X = static_cast<double>(pBullet->TargetCoords.X - pBullet->SourceCoords.X);
	pBullet->Velocity.Y = static_cast<double>(pBullet->TargetCoords.Y - pBullet->SourceCoords.Y);
	pBullet->Velocity.Z = static_cast<double>(pBullet->TargetCoords.Z - pBullet->SourceCoords.Z);
	pBullet->Velocity *= this->GetTrajectorySpeed(pBullet) / pBullet->Velocity.Magnitude();
}

bool ArtilleryTrajectory::OnAI(BulletClass* pBullet)
{
	if (!BulletExt::ExtMap.Find(pBullet)->Interfered)
	{
		if (this->GetTrajectoryType<ArtilleryTrajectoryType>(pBullet)->DistanceToHeight)
		{
			CoordStruct TempTargetLocation = pBullet->TargetCoords;
			TempTargetLocation.Z = 0;
			CoordStruct TempSourceLocation = pBullet->SourceCoords;
			TempSourceLocation.Z = 0;
			double distance = TempTargetLocation.DistanceFrom(TempSourceLocation);
			double height = (this->GetTrajectoryType<ArtilleryTrajectoryType>(pBullet)->DistanceToHeight_Multiplier * distance) + pBullet->TargetCoords.Z;

			if (this->GetTrajectoryType<ArtilleryTrajectoryType>(pBullet)->MaxHeight > 0 && height > (this->GetTrajectoryType<ArtilleryTrajectoryType>(pBullet)->MaxHeight + pBullet->TargetCoords.Z))
				height = this->GetTrajectoryType<ArtilleryTrajectoryType>(pBullet)->MaxHeight + pBullet->TargetCoords.Z;

			if (height > pBullet->SourceCoords.Z)
				this->Height = height;
		}

		CoordStruct bulletCoords = pBullet->Location;
		bulletCoords.Z = 0;
		CoordStruct initialTargetLocation = this->InitialTargetLocation;
		CoordStruct initialSourceLocation = this->InitialSourceLocation;

		double fullInitialDistance = initialSourceLocation.DistanceFrom(initialTargetLocation);// +(double)zDelta;
		double halfInitialDistance = fullInitialDistance / 2;
		double currentBulletDistance = initialSourceLocation.DistanceFrom(bulletCoords);

		// Trajectory angle
		int sinDecimalTrajectoryAngle = 90;
		double sinRadTrajectoryAngle = Math::sin(Math::deg2rad(sinDecimalTrajectoryAngle));

		// Angle of the projectile in the current location
		double angle = (currentBulletDistance * sinDecimalTrajectoryAngle) / halfInitialDistance;
		double sinAngle = Math::sin(Math::deg2rad(angle));

		// Height of the flying projectile in the current location
		double currHeight = (sinAngle * this->Height) / sinRadTrajectoryAngle;

		CoordStruct source = pBullet->SourceCoords;
		source.Z = 0;
		double SourceDistance = this->InitialSourceLocation.DistanceFrom(source);
		double fixangle = (SourceDistance * sinDecimalTrajectoryAngle) / halfInitialDistance;
		double fixsinAngle = Math::sin(Math::deg2rad(fixangle));
		double heightfix = pBullet->SourceCoords.Z - (fixsinAngle * this->Height) / sinRadTrajectoryAngle;

		pBullet->Location.Z = pBullet->TargetCoords.Z + static_cast<int>(currHeight + heightfix);

		if (!this->Init)
		{
			auto pExt = BulletExt::ExtMap.Find(pBullet);
			pExt->LaserTrails.clear();
			pBullet->Limbo();
			pBullet->Unlimbo(pBullet->SourceCoords, static_cast<DirType>(0));
			if (auto pTypeExt = BulletTypeExt::ExtMap.Find(pBullet->Type))
			{
				auto pThis = pExt->OwnerObject();
				auto pOwner = pThis->Owner ? pThis->Owner->Owner : nullptr;

				for (auto const& idxTrail : pTypeExt->LaserTrail_Types)
				{
					if (auto const pLaserType = LaserTrailTypeClass::Array[idxTrail].get())
					{
						pExt->LaserTrails.push_back(
							std::make_unique<LaserTrailClass>(pLaserType, pOwner));
					}
				}
			}
			this->Init = true;
		}
		else
		{
			this->CenterLocation.X += static_cast<int>(pBullet->Velocity.X);
			this->CenterLocation.Y += static_cast<int>(pBullet->Velocity.Y);
			this->CenterLocation.Z += static_cast<int>(pBullet->Velocity.Z);
		}

		if (this->CenterLocation.DistanceFrom(pBullet->TargetCoords) < 100)
			pBullet->Location = this->CenterLocation;
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
