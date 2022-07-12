#include "AttachEffectClass.h"

#include <Utilities/TemplateDef.h>

#include <Ext/WeaponType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/Anim/Body.h>

AttachEffectClass::AttachEffectClass(AttachEffectTypeClass* pType, TechnoClass* pOwner, TechnoClass* pTarget, int duration, int delay)
	: Type(pType), Owner(pOwner), AttachOwner(pTarget), Duration(duration), Delay_Timer(delay)
{
	Init();
}

AttachEffectClass::~AttachEffectClass()
{
	Initialized = false;
	WeaponTimers.clear();
	AttackedWeaponTimers.clear();
}

void AttachEffectClass::Init()
{
	if (!Delay_Timer.Completed())
		return;

	for (WeaponTypeClass* pWeapon : Type->WeaponList)
	{
		WeaponTimers.emplace_back(std::move(RateTimer(pWeapon->ROF)));
	}

	for (WeaponTypeClass* pWeapon : Type->AttackedWeaponList)
	{
		AttackedWeaponTimers.emplace_back(std::move(RateTimer(pWeapon->ROF)));
	}

	CreateAnim();

	Initialized = true;
	Timer.Start(Duration);
}

void AttachEffectClass::CreateAnim()
{
	if (!Type->Anim.isset())
		return;

	Anim.reset(GameCreate<AnimClass>(Type->Anim, AttachOwner->GetCoords()));
	Anim->SetOwnerObject(AttachOwner);
	Anim->RemainingIterations = 0xFFU;

	if (Owner != nullptr)
		Anim->Owner = this->Owner->GetOwningHouse();
}

void AttachEffectClass::KillAnim()
{
	if (Anim == nullptr)
		return;

	Anim.get_deleter()(Anim.get());
	Anim.reset(nullptr);
}

void AttachEffectClass::Update()
{
	if (!Initialized && Delay_Timer.Completed())
		Init();

	for (size_t i = 0; i < Type->WeaponList.size(); i++)
	{
		RateTimer& timer = WeaponTimers[i];

		if (timer.Completed())
		{
			WeaponTypeExt::AssertValid(Type->WeaponList[i]);
			WeaponTypeExt::DetonateAt(Type->WeaponList[i], AttachOwner, Owner);
			timer.Resume();
		}
	}
}

void AttachEffectClass::AttachOwnerAttackedBy(TechnoClass* pAttacker)
{
	if (pAttacker == nullptr)
		return;

	for (size_t i = 0; i < Type->AttackedWeaponList.size(); i++)
	{
		WeaponTypeClass* pWeapon = Type->AttackedWeaponList[i];
		WeaponTypeExt::AssertValid(pWeapon);
		WeaponStruct weaponStruct;
		weaponStruct.WeaponType = pWeapon;
		TechnoExt::SimulatedFire(AttachOwner, weaponStruct, pAttacker);
	}
}

template <typename T>
bool AttachEffectClass::Serialize(T& stm)
{
	stm
		.Process(this->Type)
		.Process(this->Owner)
		.Process(this->AttachOwner)
		.Process(this->Timer)
		.Process(this->Anim)
		.Process(this->Loop_Timer)
		.Process(this->Delay_Timer)
		.Process(this->WeaponTimers)
		.Process(this->AttackedWeaponTimers)
		.Process(this->Initialized)
		.Process(this->Duration)
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
