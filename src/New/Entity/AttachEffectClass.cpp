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

	Initialized = false;
	WeaponTimers.clear();
	AttackedWeaponTimers.clear();
	KillAnim();

	for (WeaponTypeClass* pWeapon : Type->WeaponList)
	{
		WeaponTimers.emplace_back(std::move(RateTimer(pWeapon->ROF)));
	}

	for (WeaponTypeClass* pWeapon : Type->AttackedWeaponList)
	{
		AttackedWeaponTimers.emplace_back(std::move(RateTimer(pWeapon->ROF)));
	}

	CreateAnim();

	if (Type->Loop_Duration.isset())
	{
		Loop_Timer.Start(Type->Loop_Duration);
	}

	Initialized = true;
	Timer.Start(Duration);
}

void AttachEffectClass::CreateAnim()
{
	if (!Type->Anim.isset() || Anim != nullptr)
		return;

	Anim.reset(GameCreate<AnimClass>(Type->Anim, AttachOwner->GetCoords()));
	Anim->SetOwnerObject(AttachOwner);
	Anim->RemainingIterations = 0xFFU;

	if (Owner != nullptr)
		Anim->Owner = this->Owner->GetOwningHouse();
}

void AttachEffectClass::KillAnim()
{
	Anim.reset(nullptr);
}

void AttachEffectClass::AddAllTimers(int frames)
{
	Timer.Start(Timer.GetTimeLeft() + frames);

	for (auto& timer : WeaponTimers)
	{
		timer.Start(timer.GetTimeLeft() + frames);
	}

	for (auto& timer : AttackedWeaponTimers)
	{
		timer.Start(timer.GetTimeLeft() + frames);
	}

	if (!Loop_Timer.Completed())
		Loop_Timer.Start(Loop_Timer.GetTimeLeft() + frames);

	if (!Delay_Timer.Completed())
		Delay_Timer.Start(Delay_Timer.GetTimeLeft() + frames);
}

void AttachEffectClass::Update()
{
	if (!Initialized && Delay_Timer.Completed())
		Init();

	if (AttachOwner->InLimbo || AttachOwner->IsImmobilized || AttachOwner->Transporter != nullptr)
	{
		if (Type->DiscardOnEntry && AttachOwner->InLimbo || AttachOwner->Transporter != nullptr)
		{
			Timer.Start(-1);
			return;
		}
		Inlimbo = true;
		KillAnim();
		AddAllTimers();

		return;
	}

	if (Type->Loop_Duration.isset() && Loop_Timer.Completed())
	{
		KillAnim();
		Delay_Timer.Start(Type->Loop_Delay);
		InLoopDelay = true;
	}

	if (!Delay_Timer.Completed())
		return;

	if (InLoopDelay)
	{
		InLoopDelay = false;
		Init();
	}

	if (!Type->ShowAnim_Cloaked)
	{
		if (AttachOwner->CloakState == CloakState::Cloaking || AttachOwner->CloakState == CloakState::Cloaked)
		{
			InCloak = true;
			KillAnim();
		}
		else if(InCloak)
		{
			InCloak = false;
			CreateAnim();
		}
	}

	if (Inlimbo)
	{
		Inlimbo = false;
		CreateAnim();
	}

	AttachOwner->Cloakable |= Type->Cloak;
	AttachOwner->Cloakable &= !Type->Decloak;

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
	if (pAttacker == nullptr || !Delay_Timer.Completed())
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
		.Process(this->InLoopDelay)
		.Process(this->InCloak)
		.Process(this->Inlimbo)
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
