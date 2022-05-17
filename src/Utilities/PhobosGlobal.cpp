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
bool PhobosGlobal::Serialize(T& stm)
{
	return stm
		//.Process(this->Techno_HugeBar);
		.Success();
}

bool PhobosGlobal::Save(PhobosStreamWriter& stm)
{
	stm.Process(this->Techno_HugeBar);
	return Serialize(stm);
}

bool PhobosGlobal::Load(PhobosStreamReader& stm)
{
	stm.Process(this->Techno_HugeBar);
	return Serialize(stm);
}

bool PhobosGlobal::SaveGlobals(PhobosStreamWriter& stm)
{
	return GlobalObject.Save(stm);
}

bool PhobosGlobal::LoadGlobals(PhobosStreamReader& stm)
{
	return GlobalObject.Load(stm);
}

#pragma endregion save/load