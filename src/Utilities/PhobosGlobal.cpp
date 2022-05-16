#include "PhobosGlobal.h"

//GlobalObject initial
PhobosGlobal PhobosGlobal::GlobalObject;

PhobosGlobal* PhobosGlobal::Global()
{
	return &GlobalObject;
}

//Save/Load
#pragma region save/load

template <typename T>
void PhobosGlobal::Serialize(T& stm)
{
	stm
		.Process(this->Techno_HugeBar);
}

void PhobosGlobal::Save(PhobosStreamWriter& stm)
{
	Serialize(stm);
}

void PhobosGlobal::Load(PhobosStreamReader& stm)
{
	Serialize(stm);
}

void PhobosGlobal::SaveGlobal(PhobosStreamWriter& stm)
{
	GlobalObject.Save(stm);
}

void PhobosGlobal::LoadGlobal(PhobosStreamReader& stm)
{
	GlobalObject.Load(stm);
}

#pragma endregion save/load