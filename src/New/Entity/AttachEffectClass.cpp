#include "AttachEffectClass.h"

#include <Utilities/TemplateDef.h>

#include <Ext/WeaponType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/Anim/Body.h>

#include <Misc/PhobosGlobal.h>

AttachEffectClass::AttachEffectClass(AttachEffectTypeClass* pType, TechnoClass* pOwner, TechnoClass* pTarget, int duration, int delay)
	: Type(pType), Owner(pOwner), AttachOwner(pTarget), Duration(duration), Delay_Timer(delay)
{
	OwnerHouse = pOwner == nullptr ? HouseClass::FindNeutral() : pOwner->GetOwningHouse();
	Init();
}

AttachEffectClass::~AttachEffectClass()
{
	Initialized = false;
	WeaponTimers.clear();
	AttackedWeaponTimers.clear();

	if (Type->EndedAnim.isset())
	{
		AnimClass* pAnim = GameCreate<AnimClass>(Type->EndedAnim, AttachOwner->GetCoords());
		pAnim->SetOwnerObject(AttachOwner);
		pAnim->Owner = OwnerHouse;
	}
}

void AttachEffectClass::Init()
{
	if (!Delay_Timer.Completed())
		return;

	KillAnim();
	WeaponTimers.clear();
	AttackedWeaponTimers.clear();

	if (!Initialized)
	{
		Timer.Start(Duration < 0 ? (1 << 30) : Duration);
		Initialized = true;
	}

	for (WeaponTypeClass* pWeapon : Type->WeaponList)
	{
		WeaponTimers.emplace_back(std::move(CDTimerClass(pWeapon->ROF)));

		if (Type->WeaponList_FireOnAttach)
			WeaponTimers.back().StartTime = Unsorted::CurrentFrame + pWeapon->ROF;
	}

	for (WeaponTypeClass* pWeapon : Type->AttackedWeaponList)
	{
		AttackedWeaponTimers.emplace_back(std::move(CDTimerClass(pWeapon->ROF)));
	}

	if (Type->Loop_Duration.isset())
	{
		Loop_Timer.Start(Type->Loop_Duration);
	}

	CreateAnim();
}

void AttachEffectClass::CreateAnim()
{
	if (!Type->Anim.isset() || Anim != nullptr)
		return;

	Anim = GameCreate<AnimClass>(Type->Anim, AttachOwner->GetCoords());
	Anim->SetOwnerObject(AttachOwner);
	Anim->RemainingIterations = 0xFFU;
	Anim->Owner = OwnerHouse;
}

void AttachEffectClass::KillAnim()
{
	if (Anim != nullptr)
	{
		Anim->DetachFromObject(AttachOwner, false);
		Anim = nullptr;
	}
}

void AttachEffectClass::AddAllTimers(int frames)
{
	if (frames <= 0)
		return;

	Timer.StartTime += frames;

	for (auto& timer : WeaponTimers)
	{
		timer.StartTime += frames;
	}

	for (auto& timer : AttackedWeaponTimers)
	{
		timer.StartTime += frames;
	}

	if (!Loop_Timer.Completed())
	{
		Loop_Timer.StartTime += frames;
	}

	if (!Delay_Timer.Completed())
	{
		Delay_Timer.StartTime += frames;
	}
}

bool AttachEffectClass::IsActive() const
{
	return
		Delay_Timer.Completed() &&
		!Inlimbo &&
		!InLoopDelay &&
		!Timer.Completed()
		;
}

void AttachEffectClass::Update()
{
	if (!Initialized && Delay_Timer.Completed())
		Init();

	if (AttachOwner->InLimbo || AttachOwner->IsImmobilized || AttachOwner->Transporter != nullptr)
	{
		if (Type->DiscardOnEntry && (AttachOwner->InLimbo || AttachOwner->Transporter != nullptr))
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
		CDTimerClass& timer = WeaponTimers[i];

		if (timer.Completed())
		{
			if (Owner != nullptr && !Owner->InLimbo && Owner->IsAlive)
			{
				WeaponTypeExt::DetonateAt(Type->WeaponList[i], AttachOwner, Owner);
			}
			else
			{
				TechnoClass* pStand = PhobosGlobal::Global()->GetGenericStand();
				HouseClass* pOriginStandOwner = pStand->Owner;
				pStand->Owner = OwnerHouse;
				WeaponTypeExt::DetonateAt(Type->WeaponList[i], AttachOwner, pStand);
				pStand->Owner = pOriginStandOwner;
			}

			timer.Restart();
		}
	}
}

void AttachEffectClass::AttachOwnerAttackedBy(TechnoClass* pAttacker)
{
	if (pAttacker == nullptr || !Delay_Timer.Completed())
		return;

	for (size_t i = 0; i < Type->AttackedWeaponList.size(); i++)
	{
		if (!AttackedWeaponTimers[i].Completed())
			continue;

		AttackedWeaponTimers[i].Restart();
		WeaponTypeClass* pWeapon = Type->AttackedWeaponList[i];
		WeaponStruct weaponStruct;
		weaponStruct.WeaponType = pWeapon;
		TechnoExt::SimulatedFire(AttachOwner, weaponStruct, pAttacker);
	}
}

void AttachEffectClass::InvalidatePointer(void* ptr)
{
	if (Owner == ptr)
		Owner = nullptr;

	if (Anim == ptr)
		KillAnim();
}

template <typename T>
bool AttachEffectClass::Serialize(T& stm)
{
	stm
		.Process(this->Type)
		.Process(this->Owner)
		.Process(this->AttachOwner)
		.Process(this->OwnerHouse)
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

bool AttachEffectClass::Load(PhobosStreamReader& stm, bool registerForChange)
{
	return Serialize(stm);
}

bool AttachEffectClass::Save(PhobosStreamWriter& stm) const
{
	return const_cast<AttachEffectClass*>(this)->Serialize(stm);
}
