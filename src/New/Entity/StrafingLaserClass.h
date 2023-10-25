#pragma once
#include <vector>

#include <LaserDrawClass.h>
#include <WeaponTypeClass.h>

#include <Utilities/TemplateDef.h>
#include <Utilities/Stream.h>

#include <New/Type/StrafingLaserTypeClass.h>

class StrafingLaserClass
{
public:
	StrafingLaserTypeClass* Type;
	AbstractClass* Target;
	bool InGround;
	int CurrentFrame;
	CoordStruct FLH;
	CoordStruct SourceFLH;
	CoordStruct TargetFLH;

	StrafingLaserClass();
	StrafingLaserClass(StrafingLaserTypeClass* type, AbstractClass* target, bool inground, int currentFrame, CoordStruct flh, CoordStruct sourceflh,
		CoordStruct targetflh);
	~StrafingLaserClass();

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	bool Save(PhobosStreamWriter& Stm) const;

private:
	template <typename T>
	bool Serialize(T& Stm);
};
