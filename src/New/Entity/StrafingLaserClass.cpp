#include "StrafingLaserClass.h"

StrafingLaserClass::StrafingLaserClass() :
	Type { nullptr }
	, Target { nullptr }
	, InGround { false }
	, CurrentFrame { -1 }
	, FLH { CoordStruct::Empty }
	, SourceFLH { CoordStruct::Empty }
	, TargetFLH { CoordStruct::Empty }
{ }

StrafingLaserClass::StrafingLaserClass(StrafingLaserTypeClass* type, AbstractClass* target, bool inground, int currentFrame, CoordStruct flh, CoordStruct sourceflh,
	CoordStruct targetflh) :
	Type { type }
	, InGround { inground }
	, Target { target }
	, CurrentFrame { currentFrame }
	, FLH { flh }
	, SourceFLH { sourceflh }
	, TargetFLH { targetflh }
{ }

StrafingLaserClass::~StrafingLaserClass()
{
}

template <typename T>
bool StrafingLaserClass::Serialize(T& Stm)
{
	return Stm
		.Process(this->Type)
		.Process(this->Target)
		.Process(this->InGround)
		.Process(this->CurrentFrame)
		.Process(this->FLH)
		.Process(this->SourceFLH)
		.Process(this->TargetFLH)
		.Success();
}

bool StrafingLaserClass::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return Serialize(Stm);
}

bool StrafingLaserClass::Save(PhobosStreamWriter& Stm) const
{
	return const_cast<StrafingLaserClass*>(this)->Serialize(Stm);
}
