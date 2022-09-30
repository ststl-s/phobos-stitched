#include "AttachEffectClass.h"

#include <Utilities/TemplateDef.h>

#include <Ext/WeaponType/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/Anim/Body.h>

#include <Misc/PhobosGlobal.h>

std::unordered_map<int, int> AttachEffectClass::AttachEffect_Exist;

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
	, AnimIndex(-1)
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

			if (WeaponTypeClass* pWeapon = pType->ReplaceSecondary.Get(0.0))
			{
				WeaponStruct weapon = pTargetTypeExt->Weapons.Get(1, 0.0);
				weapon.WeaponType = pWeapon;
				ReplaceWeapons_Rookie[1] = weapon;
			}
			if (WeaponTypeClass* pWeapon = pType->ReplaceSecondary.Get(1.0))
			{
				WeaponStruct weapon = pTargetTypeExt->Weapons.Get(1, 1.0);
				weapon.WeaponType = pWeapon;
				ReplaceWeapons_Veteran[1] = weapon;
			}
			if (WeaponTypeClass* pWeapon = pType->ReplaceSecondary.Get(2.0))
			{
				WeaponStruct weapon = pTargetTypeExt->Weapons.Get(1, 2.0);
				weapon.WeaponType = pWeapon;
				ReplaceWeapons_Elite[1] = weapon;
			}
		}
	}

	if (Type->Coexist_Maximum.isset())
	{
		++AttachEffect_Exist[Type->ArrayIndex];
	}

	if (!Type->Anim.empty())
	{
		if (SessionClass::IsSingleplayer() && Type->Anim_RandomPick)
		{
			AnimIndex = ScenarioClass::Instance->Random.RandomRanged(0, static_cast<int>(Type->Anim.size()) - 1);
		}
		else
		{
			AnimIndex = 0;
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

	if (Timer.Completed() && !Type->EndedAnim.empty())
	{
		int idx = AnimIndex;

		if (Type->EndedAnim_RandomPick)
		{
			idx = ScenarioClass::Instance->Random.RandomRanged(0, static_cast<int>(Type->EndedAnim.size()) - 1);
		}

		if (idx < 0 || idx >= static_cast<int>(Type->EndedAnim.size()))
		{
			idx = 0;
		}

		if (idx >= 0)
		{
			AnimClass* pAnim = GameCreate<AnimClass>(Type->EndedAnim[idx], AttachOwner->Location);

			if (TechnoExt::IsReallyAlive(this->AttachOwner))
				pAnim->SetOwnerObject(this->AttachOwner);

			pAnim->Owner = OwnerHouse;
		}
	}

	if (Type->Coexist_Maximum.isset() && Type->Coexist_Maximum > 0)
	{
		--AttachEffect_Exist[Type->ArrayIndex];
	}
}

bool AttachEffectClass::CanExist(AttachEffectTypeClass* pType)
{
	if (pType != nullptr && (!pType->Coexist_Maximum.isset() || AttachEffect_Exist[pType->ArrayIndex] < abs(pType->Coexist_Maximum)))
		return true;

	return false;
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

void AttachEffectClass::ResetAnim()
{
	KillAnim();
	CreateAnim();
}

void AttachEffectClass::CreateAnim()
{
	if (this->AnimIndex < 0 || this->Anim != nullptr)
		return;

	this->Anim = GameCreate<AnimClass>(Type->Anim[AnimIndex], AttachOwner->GetCoords());
	this->Anim->SetOwnerObject(AttachOwner);
	this->Anim->RemainingIterations = 0xFFU;
	this->Anim->Owner = OwnerHouse;
}

void AttachEffectClass::KillAnim()
{
	if (this->AnimIndex >= 0)
	{
		if (this->Anim != nullptr)
		{
			this->Anim->UnInit();
		}
	}

	this->Anim = nullptr;
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
		ResetAnim();
	}

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

			timer.StartTime = Unsorted::CurrentFrame;
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

		if (pWeapon == nullptr || !AttackedWeaponTimers[i].Completed() || pAttacker->DistanceFrom(this->AttachOwner) > pWeapon->Range)
			continue;

		AttackedWeaponTimers[i].StartTime = Unsorted::CurrentFrame;
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

int AttachEffectClass::GetCurrentTintColor()
{
	auto& colors = this->Type->Tint_Colors;

	if (colors.empty())
	{
		return 0;
	}
	else if (colors.size() == 1)
	{
		return Drawing::RGB_To_Int(colors[0]);
	}
	else
	{
		int passedTime = Unsorted::CurrentFrame - Timer.StartTime;;
		int transitionTime = this->Type->Tint_TransitionDuration;
		int transitionCycle = (passedTime / transitionTime) % colors.size();
		int currentColorIndex = transitionCycle;
		int nextColorIndex = (transitionCycle + 1) % colors.size();
		double blendingCoef = (passedTime % transitionTime) / static_cast<double>(transitionTime);
		ColorStruct ColorToPaint =
		{
			(BYTE)(colors[currentColorIndex].R * (1 - blendingCoef) + colors[nextColorIndex].R * blendingCoef),
			(BYTE)(colors[currentColorIndex].G * (1 - blendingCoef) + colors[nextColorIndex].G * blendingCoef),
			(BYTE)(colors[currentColorIndex].B * (1 - blendingCoef) + colors[nextColorIndex].B * blendingCoef)
		};
		return Drawing::RGB_To_Int(ColorToPaint);
	}
}

void AttachEffectClass::InvalidatePointer(void* ptr, bool removed)
{
	if (this == nullptr)
		return;

	if (Owner == ptr && removed)
		this->Owner = nullptr;

	if (Anim == ptr && removed)
		this->Anim = nullptr;
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
		.Process(this->AnimIndex)
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

void AttachEffectClass::Clear()
{
	AttachEffect_Exist.clear();
}

bool AttachEffectClass::LoadGlobals(PhobosStreamReader& stm)
{
	return stm
		.Process(AttachEffect_Exist)
		.Success();
}

bool AttachEffectClass::SaveGlobals(PhobosStreamWriter& stm)
{
	return stm
		.Process(AttachEffect_Exist)
		.Success();
}
