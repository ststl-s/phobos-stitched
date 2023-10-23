#include "ArcingTrajectory.h"

#include <ScenarioClass.h>

#include <Ext/Bullet/Body.h>
#include <Ext/BulletType/Body.h>

#include <New/Entity/LaserTrailClass.h>

#include <Utilities/TemplateDef.h>

bool ArcingTrajectoryType::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->PhobosTrajectoryType::Load(Stm, false);

	Stm
		.Process(this->Elevation, false)
		.Process(this->Lobber, false)
		;

	return true;
}

bool ArcingTrajectoryType::Save(PhobosStreamWriter& Stm) const
{
	this->PhobosTrajectoryType::Save(Stm);

	Stm
		.Process(this->Elevation)
		.Process(this->Lobber)
		;

	return true;
}

void ArcingTrajectoryType::Read(CCINIClass* const pINI, const char* pSection)
{
	if (!pINI->GetSection(pSection))
		return;

	this->PhobosTrajectoryType::Read(pINI, pSection);

	INI_EX exINI(pINI);

	this->Elevation.Read(exINI, pSection, "Trajectory.Arcing.Elevation");
	this->Lobber.Read(exINI, pSection, "Trajectory.Arcing.Lobber");
}

bool ArcingTrajectory::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	this->PhobosTrajectory::Load(Stm, false);

	Stm
		.Process(this->OverRange, false)
		;

	return true;
}

bool ArcingTrajectory::Save(PhobosStreamWriter& Stm) const
{
	this->PhobosTrajectory::Save(Stm);

	Stm
		.Process(this->OverRange)
		;

	return true;
}

void ArcingTrajectory::CalculateVelocity(BulletClass* pBullet, double elevation, bool lobber, ArcingTrajectory* pTraj)
{
	CoordStruct InitialSourceLocation = pBullet->SourceCoords;
	InitialSourceLocation.Z = 0;
	CoordStruct InitialTargetLocation = pBullet->TargetCoords;
	InitialTargetLocation.Z = 0;
	double FullDistance = InitialTargetLocation.DistanceFrom(InitialSourceLocation); // 水平面投影距离

	double Z = pBullet->TargetCoords.Z - pBullet->SourceCoords.Z; // 竖直向上方向的位移
	double g = BulletTypeExt::GetAdjustedGravity(pBullet->Type);

	if (elevation > DBL_EPSILON) // Elevation设定值大于0时，无视Speed设定，自动计算出膛Speed
	{
		double LifeTime = Math::sqrt(2 / g * (elevation * FullDistance - Z));

		/*
		* 【飞行总帧数计算依据】
		* 斜抛运动竖直方向牛顿第二定律：Z = Vz * t - 0.5 * g * t * t
		* 斜抛运动水平方向匀速直线运动：FullDistance = Vxy * t
		* 发射斜度：Vz = Elevation * Vxy
		*/

		double Velocity_XY = FullDistance / LifeTime;
		double ratio = Velocity_XY / FullDistance; // 相似三角形计算Vxy的分量Vx、Vy
		pBullet->Velocity.X = static_cast<double>(pBullet->TargetCoords.X - pBullet->SourceCoords.X) * ratio;
		pBullet->Velocity.Y = static_cast<double>(pBullet->TargetCoords.Y - pBullet->SourceCoords.Y) * ratio;
		pBullet->Velocity.Z = elevation * Velocity_XY;
	}
	else // 不指定发射仰角，则读取Speed设定作为出膛速率，每次攻击时自动计算发射仰角
	{
		const auto pBulletTypeExt = BulletTypeExt::ExtMap.Find(pBullet->Type);
		double S = pBulletTypeExt->Trajectory_Speed;

		/*
		* 【飞行总帧数计算依据】
		* 斜抛运动竖直方向牛顿第二定律：Z = Vz * t - 0.5 * g * t * t
		* 斜抛运动水平方向匀速直线运动：FullDistance = Vxy * t
		* 出膛速率勾股定理：Vxy * Vxy + Vz * Vz = S * S
		* 联立三式得：A * t^4 + B * t^2 + C = 0，其中参数ABC如下
		*/

		double A = g * g / 4;
		double B = g * Z - S * S;
		double C = Z * Z + FullDistance * FullDistance;

		double delta = B * B - 4 * A * C;
		if (delta < 0) // 这代表着目标超出射程
		{
			double Velocity_XY = S / Math::Sqrt2; // 设置发射仰角为45°以获得最大射程，即XYPlain分量与Z分量的模相等
			double ratio = Velocity_XY / FullDistance; // 相似三角形计算Vxy的分量Vx、Vy
			pBullet->Velocity.X = static_cast<double>(pBullet->TargetCoords.X - pBullet->SourceCoords.X) * ratio;
			pBullet->Velocity.Y = static_cast<double>(pBullet->TargetCoords.Y - pBullet->SourceCoords.Y) * ratio;
			pBullet->Velocity.Z = Velocity_XY;

			if (pTraj != nullptr)
				pTraj->OverRange = true; // 目标超出射程时，抛射体经常穿到地面以下而太晚引爆，需做额外detonate判定
		}
		else
		{
			int isLobber = lobber ? 1 : -1; // 是否高抛
			double LifeTimeSquare = (-B + isLobber * Math::sqrt(delta)) / (2 * A);
			double LifeTime = Math::sqrt(LifeTimeSquare);

			double Velocity_XY = FullDistance / LifeTime;
			double ratio = Velocity_XY / FullDistance; // 相似三角形计算Vxy的分量Vx、Vy
			pBullet->Velocity.X = static_cast<double>(pBullet->TargetCoords.X - pBullet->SourceCoords.X) * ratio;
			pBullet->Velocity.Y = static_cast<double>(pBullet->TargetCoords.Y - pBullet->SourceCoords.Y) * ratio;

			pBullet->Velocity.Z = Math::sqrt(S * S - Velocity_XY * Velocity_XY);
		}
	}
}

void ArcingTrajectory::OnUnlimbo(BulletClass* pBullet, CoordStruct* pCoord, BulletVelocity* pVelocity)
{
	pBullet->TargetCoords += BulletExt::CalculateInaccurate(pBullet->Type);
	const auto pTrajType = this->GetTrajectoryType<ArcingTrajectoryType>(pBullet);
	CalculateVelocity(pBullet, pTrajType->Elevation, pTrajType->Lobber, this);
}

/*
* 【出膛速率恒定的斜抛运动的最大距离】
* 竖直方向牛顿第二定律：Z = Vz * t - 0.5 * g * t * t
* 水平方向匀速直线运动：D = Vxy * t
* 出膛速度分量：Vz = S * sin(theta), Vxy = S * cos(theta)
* 不妨令Z = 0
* 则 D = S * cos(theta) * 2 / g * S * sin(theta) = S^2 / g * sin(2*theta)
* 由 0 < theta < PI/2, 有 0 < 2*theta < PI
* 则当theta = PI/4时，D取得最大值，最大值为S^2 / g
* 默认g = 6，若S = 100.0，则Dmax = 100 * 100 / 6 = 1667 leptons，约 6.5 cells
*/

bool ArcingTrajectory::OnAI(BulletClass* pBullet)
{
	if (pBullet->TargetCoords.DistanceFrom(pBullet->Location) < this->DetonationDistance)
		return true;

	if (this->OverRange)
	{
		if (pBullet->GetHeight() + pBullet->Velocity.Z < 0) // 预测下一帧将运动到地面以下
			return true;
	}

	return false;
}

void ArcingTrajectory::OnAIPreDetonate(BulletClass* pBullet)
{
}

void ArcingTrajectory::OnAIVelocity(BulletClass* pBullet, BulletVelocity* pSpeed, BulletVelocity* pPosition)
{
}

TrajectoryCheckReturnType ArcingTrajectory::OnAITargetCoordCheck(BulletClass* pBullet)
{
	return TrajectoryCheckReturnType::ExecuteGameCheck; // Execute game checks.
}

TrajectoryCheckReturnType ArcingTrajectory::OnAITechnoCheck(BulletClass* pBullet, TechnoClass* pTechno)
{
	return TrajectoryCheckReturnType::ExecuteGameCheck; // Execute game checks.
}
