#pragma once

#include <BulletClass.h>
#include <TechnoClass.h>

/*
https://github.com/ChrisLv-CN/YRDynamicPatcher-Kratos/blob/main/DynamicPatcher/Projects/Extension/Kraotos/Utilities/RadialFireHelper.cs
author: https://github.com/ChrisLv-CN
*/

class RadialFireHelper
{
private:
	int burst;
	double degrees = 0;
	int delta = 0;
	float deltaZ = 0;

public:
	
	 RadialFireHelper(TechnoClass* pTechno, int burst, int splitAngle)
	{
		this->burst = burst;
		DirStruct dir = pTechno->HasTurret()
			? pTechno->SecondaryFacing.target()
			: pTechno->PrimaryFacing.target();

		InitData(dir, splitAngle);
	}

	 RadialFireHelper(const DirStruct& dir, int burst, int splitAngle)
	 {
		 this->burst = burst;

		 InitData(dir, splitAngle);
	 }

	 BulletVelocity GetBulletVelocity(int index)
	 {
		 int z = 0;
		 double temp = burst / 2.0;

		 if (index - temp < 0.0)
			 z = index;
		 else
			 z = abs(index - burst + 1);

		 double angle = degrees + delta * (index + 1);
		 double radians = angle * (Math::Pi / 180);
		 DirStruct targetDir = DirStruct(radians);
		 Matrix3D matrix = Matrix3D();
		 matrix.MakeIdentity();
		 matrix.RotateZ(targetDir.radians());
		 matrix.Translate(1, 0, 0);
		 Vector3D<float> offset = Vector3D<float>::Empty;
		 Matrix3D::MatrixMultiply(matrix, offset);
		 return BulletVelocity(offset.X, -offset.Y, deltaZ * z);
	 }

private:

	void InitData(DirStruct dir, int splitAngle)
	{
		degrees = dir.radians() * (180.0 / Math::Pi) + splitAngle;
		delta = splitAngle / (burst + 1);
		deltaZ = 1.0 / (burst / 2.0 + 1);
	}
};
