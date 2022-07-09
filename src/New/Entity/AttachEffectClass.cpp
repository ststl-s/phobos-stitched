#include "AttachEffectClass.h"

#include <AnimClass.h>
#include <Utilities/TemplateDef.h>

void AttachEffectClass::Init(const CoordStruct& crdLoc)
{
	if (!Delay_Timer.Completed())
		return;

	for (AnimTypeClass* pAnimType : Type->AnimList)
	{
		Anims.emplace_back(GameCreate<AnimClass>(pAnimType, crdLoc));
	}

	for (WeaponTypeClass* pWeapon : Type->WeaponList)
	{
		WeaponTimers.emplace_back(std::move(RepeatableTimerStruct(pWeapon->ROF)));
	}

	for (WeaponTypeClass* pWeapon : Type->AttackedWeaponList)
	{
		AttackedWeaponTimers.emplace_back(std::move(RepeatableTimerStruct(pWeapon->ROF)));
	}

	if (Type->Loop_Duration.isset())
		Loop_Timer.Start(Type->Loop_Duration);
	else if (!Anims.empty())
		Loop_Timer.Start(Anims[0]->Animation.Timer.Duration);
}

void AttachEffectClass::Update(const CoordStruct& crdLoc)
{

}

template <typename T>
bool AttachEffectClass::Serialize(T& stm)
{
	stm
		.Process(this->Type)
		.Process(this->Timer)
		.Process(this->Anims)
		.Process(this->Loop_Timer)
		.Process(this->Delay_Timer)
		.Process(this->WeaponTimers)
		.Process(this->AttackedWeaponTimers)
		;

	return stm.Success();
}

bool AttachEffectClass::Load(PhobosStreamReader& stm)
{
	return Serialize(stm);
}

bool AttachEffectClass::Save(PhobosStreamWriter& stm)
{
	return Serialize(stm);
}
