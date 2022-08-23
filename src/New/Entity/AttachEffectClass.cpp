#include "AttachEffectClass.h"

#include <Utilities/TemplateDef.h>

#include <Ext/WeaponType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/Anim/Body.h>

#include <Misc/PhobosGlobal.h>

AttachEffectClass::AttachEffectClass(AttachEffectTypeClass* pType, TechnoClass* pOwner, TechnoClass* pTarget, int duration, int delay)
	: Type(pType)
	, Owner(pOwner)
	, AttachOwner(pTarget)
	, OwnerHouse(pOwner == nullptr ? HouseClass::FindNeutral() : pOwner->Owner)
	, Duration(duration)
	, Delay_Timer(delay)
	, Timer()
	, Loop_Timer()
	, Anim(nullptr)
	, WeaponTimers()
	, AttackedWeaponTimers()
	, Initialized(false)
	, InLoopDelay(false)
	, InCloak(false)
	, Inlimbo(false)
	, ReplaceWeapons_Rookie()
	, ReplaceWeapons_Veteran()
	, ReplaceWeapons_Elite()
{
	if (pType->ReplaceWeapon)
	{
		TechnoTypeClass* pTargetType = pTarget->GetTechnoType();
		auto pTargetTypeExt = TechnoTypeExt::ExtMap.Find(pTargetType);

		if (pTargetType->IsGattling)
		{
			for (int i = 0; i < pTargetType->WeaponCount; i++)
			{
				if (WeaponTypeClass* pWeapon = pType->ReplaceGattlingWeapon.Get(i, 0.0))
				{
					WeaponStruct weapon = pTargetTypeExt->Weapons.Get(i, 0.0);
					weapon.WeaponType = pWeapon;
					ReplaceWeapons_Rookie[i] = weapon;
				}

				if (WeaponTypeClass* pWeapon = pType->ReplaceGattlingWeapon.Get(i, 1.0))
				{
					WeaponStruct weapon = pTargetTypeExt->Weapons.Get(i, 1.0);
					weapon.WeaponType = pWeapon;
					ReplaceWeapons_Veteran[i] = weapon;
				}

				if (WeaponTypeClass* pWeapon = pType->ReplaceGattlingWeapon.Get(i, 2.0))
				{
					WeaponStruct weapon = pTargetTypeExt->Weapons.Get(i, 2.0);
					weapon.WeaponType = pWeapon;
					ReplaceWeapons_Elite[i] = weapon;
				}
			}
		}
		else
		{
			if (WeaponTypeClass* pWeapon = pType->ReplacePrimary.Get(0.0))
			{
				WeaponStruct weapon = pTargetTypeExt->Weapons.Get(0, 0.0);
				weapon.WeaponType = pWeapon;
				ReplaceWeapons_Rookie[0] = weapon;
			}

			if (WeaponTypeClass* pWeapon = pType->ReplacePrimary.Get(1.0))
			{
				WeaponStruct weapon = pTargetTypeExt->Weapons.Get(0, 1.0);
				weapon.WeaponType = pWeapon;
				ReplaceWeapons_Veteran[0] = weapon;
			}

			if (WeaponTypeClass* pWeapon = pType->ReplacePrimary.Get(2.0))
			{
				WeaponStruct weapon = pTargetTypeExt->Weapons.Get(0, 2.0);
				weapon.WeaponType = pWeapon;
				ReplaceWeapons_Elite[0] = weapon;
			}

			if (WeaponTypeClass* pWeapon = pType->ReplacePrimary.Get(0.0))
			{
				WeaponStruct weapon = pTargetTypeExt->Weapons.Get(1, 0.0);
				weapon.WeaponType = pWeapon;
				ReplaceWeapons_Rookie[1] = weapon;
			}
			if (WeaponTypeClass* pWeapon = pType->ReplacePrimary.Get(1.0))
			{
				WeaponStruct weapon = pTargetTypeExt->Weapons.Get(1, 1.0);
				weapon.WeaponType = pWeapon;
				ReplaceWeapons_Veteran[1] = weapon;
			}
			if (WeaponTypeClass* pWeapon = pType->ReplacePrimary.Get(2.0))
			{
				WeaponStruct weapon = pTargetTypeExt->Weapons.Get(1, 2.0);
				weapon.WeaponType = pWeapon;
				ReplaceWeapons_Elite[1] = weapon;
			}
		}
	}

	Init();
}

AttachEffectClass::~AttachEffectClass()
{
	Initialized = false;
	WeaponTimers.clear();
	AttackedWeaponTimers.clear();
	KillAnim();

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

	Anim.reset(GameCreate<AnimClass>(Type->Anim, AttachOwner->GetCoords()));
	Anim.get()->SetOwnerObject(AttachOwner);
	Anim.get()->RemainingIterations = 0xFFU;
	Anim.get()->Owner = OwnerHouse;
}

void AttachEffectClass::KillAnim()
{
	Anim.clear();
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
		Timer.InProgress() &&
		Delay_Timer.Expired() &&
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
	if (pAttacker == nullptr)
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

const WeaponStruct* AttachEffectClass::GetReplaceWeapon(int weaponIdx) const
{
	switch (AttachOwner->Veterancy.GetRemainingLevel())
	{
	case Rank::Rookie:
	{
		if (ReplaceWeapons_Rookie.count(weaponIdx))
		{
			return &ReplaceWeapons_Rookie.at(weaponIdx);
		}
	}break;
	case Rank::Veteran:
	{
		if (ReplaceWeapons_Veteran.count(weaponIdx))
		{
			return &ReplaceWeapons_Veteran.at(weaponIdx);
		}
	}break;
	case Rank::Elite:
	{
		if (ReplaceWeapons_Elite.count(weaponIdx))
		{
			return &ReplaceWeapons_Elite.at(weaponIdx);
		}
	}break;
	}

	return nullptr;
}

void AttachEffectClass::InvalidatePointer(void* ptr)
{
	if (Owner == ptr)
		Owner = nullptr;

	if (Anim == ptr)
		Anim.release();
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
		.Process(this->ReplaceWeapons_Rookie)
		.Process(this->ReplaceWeapons_Veteran)
		.Process(this->ReplaceWeapons_Elite)
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
