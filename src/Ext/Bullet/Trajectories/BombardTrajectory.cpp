#include "BombardTrajectory.h"

#include <ScenarioClass.h>

#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>

#include <New/Entity/LaserTrailClass.h>

#include <Utilities/TemplateDef.h>

bool BombardTrajectoryType::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->PhobosTrajectoryType::Load(Stm, false);
	Stm
		.Process(this->Height, false)
		.Process(this->Anti, false)
		;
	return true;
}

bool BombardTrajectoryType::Save(PhobosStreamWriter& Stm) const
{
	this->PhobosTrajectoryType::Save(Stm);
	Stm
		.Process(this->Height)
		.Process(this->Anti)
		;
	return true;
}

void BombardTrajectoryType::Read(CCINIClass* const pINI, const char* pSection)
{
	if (!pINI->GetSection(pSection))
		return;

	this->PhobosTrajectoryType::Read(pINI, pSection);

	INI_EX exINI(pINI);

	this->Height.Read(exINI, pSection, "Trajectory.Bombard.Height");
	this->Anti.Read(exINI, pSection, "Trajectory.Bombard.Anti");
}

bool BombardTrajectory::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->PhobosTrajectory::Load(Stm, false);

	Stm
		.Process(this->IsFalling, false)
		.Process(this->Height, false)
		;

	return true;
}

bool BombardTrajectory::Save(PhobosStreamWriter& Stm) const
{
	this->PhobosTrajectory::Save(Stm);

	Stm
		.Process(this->IsFalling)
		.Process(this->Height)
		;

	return true;
}

void BombardTrajectory::OnUnlimbo(BulletClass* pBullet, CoordStruct* pCoord, BulletVelocity* pVelocity)
{
	this->Height = this->GetTrajectoryType<BombardTrajectoryType>(pBullet)->Height + pBullet->TargetCoords.Z;

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

	if (!this->GetTrajectoryType<BombardTrajectoryType>(pBullet)->Anti)
	{
		pBullet->Velocity.X = static_cast<double>(pBullet->TargetCoords.X - pBullet->SourceCoords.X);
		pBullet->Velocity.Y = static_cast<double>(pBullet->TargetCoords.Y - pBullet->SourceCoords.Y);
		pBullet->Velocity.Z = static_cast<double>(this->Height - pBullet->SourceCoords.Z);
		pBullet->Velocity *= this->GetTrajectorySpeed(pBullet) / pBullet->Velocity.Magnitude();
	}
	else
	{
		pBullet->Velocity.X = static_cast<double>(0);
		pBullet->Velocity.Y = static_cast<double>(0);
		pBullet->Velocity.Z = static_cast<double>(this->Height - pBullet->SourceCoords.Z);
		pBullet->Velocity *= this->GetTrajectorySpeed(pBullet) / pBullet->Velocity.Magnitude();
	}
}

bool BombardTrajectory::OnAI(BulletClass* pBullet)
{
	if (pBullet->TargetCoords.DistanceFrom(pBullet->Location) < this->DetonationDistance)
		return true;

	return false;
}

void BombardTrajectory::OnAIPreDetonate(BulletClass* pBullet)
{
}

void BombardTrajectory::OnAIVelocity(BulletClass* pBullet, BulletVelocity* pSpeed, BulletVelocity* pPosition)
{
	if (!BulletExt::ExtMap.Find(pBullet)->Interfered)
	{
		if (!this->GetTrajectoryType<BombardTrajectoryType>(pBullet)->Anti)
		{
			if (!this->IsFalling)
			{
				pSpeed->Z += BulletTypeExt::GetAdjustedGravity(pBullet->Type);
				if (pBullet->Location.Z + pBullet->Velocity.Z >= this->Height)
				{
					this->IsFalling = true;
					pSpeed->X = 0.0;
					pSpeed->Y = 0.0;
					pSpeed->Z = 0.0;
					pPosition->X = pBullet->TargetCoords.X;
					pPosition->Y = pBullet->TargetCoords.Y;
				}
			}
		}
		else
		{
			pSpeed->Z += BulletTypeExt::GetAdjustedGravity(pBullet->Type);
			if (!this->IsFalling)
			{
				if (pBullet->Location.Z + pBullet->Velocity.Z >= this->Height)
				{
					this->IsFalling = true;
					pBullet->Velocity.X = static_cast<double>(pBullet->TargetCoords.X - pBullet->Location.X);
					pBullet->Velocity.Y = static_cast<double>(pBullet->TargetCoords.Y - pBullet->Location.Y);
					pBullet->Velocity.Z = static_cast<double>(pBullet->TargetCoords.Z - pBullet->Location.Z);
					pBullet->Velocity *= this->GetTrajectorySpeed(pBullet) / pBullet->Velocity.Magnitude();

					pSpeed->X = pBullet->Velocity.X;
					pSpeed->Y = pBullet->Velocity.Y;
					pSpeed->Z = pBullet->Velocity.Z;
				}
			}
		}
	}
	else
	{
		pSpeed->Z += BulletTypeExt::GetAdjustedGravity(pBullet->Type);
	}
}

TrajectoryCheckReturnType BombardTrajectory::OnAITargetCoordCheck(BulletClass* pBullet)
{
	return TrajectoryCheckReturnType::ExecuteGameCheck; // Execute game checks.
}

TrajectoryCheckReturnType BombardTrajectory::OnAITechnoCheck(BulletClass* pBullet, TechnoClass* pTechno)
{
	return TrajectoryCheckReturnType::ExecuteGameCheck; // Execute game checks.
}
