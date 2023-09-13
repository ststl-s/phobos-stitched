#include "ShieldClass.h"

#include <AnimClass.h>
#include <Conversions.h>
#include <GameStrings.h>
#include <HouseClass.h>
#include <RadarEventClass.h>
#include <TacticalClass.h>

#include <Ext/Rules/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WarheadType/Body.h>

#include <Utilities/GeneralUtils.h>
#include <Utilities/EnumFunctions.h>
#include <Utilities/TemplateDef.h>

std::vector<ShieldClass*> ShieldClass::Array;

ShieldClass::ShieldClass() : Techno { nullptr }
	, HP { 0 }
	, Timers { }
	, AreAnimsHidden { false }
{
	ShieldClass::Array.emplace_back(this);
}

ShieldClass::ShieldClass(TechnoClass* pTechno, bool isAttached) : Techno { pTechno }
	, IdleAnim { nullptr }
	, Timers { }
	, Cloak { false }
	, Online { true }
	, Temporal { false }
	, Available { true }
	, AreAnimsHidden { false }
	, ArmorReplaced { false }
	, Attached { isAttached }
	, SelfHealing_Rate_Warhead { -1 }
	, Respawn_Rate_Warhead { -1 }
	, LastBreakFrame { 0 }
	, LastTechnoHealthRatio { 0.0 }
{
	this->UpdateType();
	SetHP(this->Type->InitialStrength.Get(this->Type->Strength));
	strcpy_s(this->TechnoID, this->Techno->get_ID());
	ShieldClass::Array.emplace_back(this);
}

ShieldClass::~ShieldClass()
{
	auto it = std::find(ShieldClass::Array.begin(), ShieldClass::Array.end(), this);

	if (it != ShieldClass::Array.end())
		ShieldClass::Array.erase(it);
}

void ShieldClass::UpdateType()
{
	this->Type = TechnoExt::ExtMap.Find(this->Techno)->CurrentShieldType;
}

void ShieldClass::PointerGotInvalid(void* ptr, bool removed)
{
	if (auto const pAnim = abstract_cast<AnimClass*>(static_cast<AbstractClass*>(ptr)))
	{
		for (auto pShield : ShieldClass::Array)
		{
			if (pAnim == pShield->IdleAnim)
				pShield->KillAnim();
		}
	}
}

// =============================
// load / save

template <typename T>
bool ShieldClass::Serialize(T& Stm)
{
	return Stm
		.Process(this->Techno)
		.Process(this->TechnoID)
		.Process(this->IdleAnim)
		.Process(this->Timers.SelfHealing)
		.Process(this->Timers.SelfHealing_Warhead)
		.Process(this->Timers.Respawn)
		.Process(this->HP)
		.Process(this->Cloak)
		.Process(this->Online)
		.Process(this->Temporal)
		.Process(this->Available)
		.Process(this->Attached)
		.Process(this->AreAnimsHidden)
		.Process(this->Type)
		.Process(this->SelfHealing_Warhead)
		.Process(this->SelfHealing_Rate_Warhead)
		.Process(this->Respawn_Warhead)
		.Process(this->Respawn_Rate_Warhead)
		.Process(this->LastBreakFrame)
		.Process(this->LastTechnoHealthRatio)
		.Process(this->ArmorReplaced)
		.Process(this->ReplacedArmorIdx)
		.Success();
}

bool ShieldClass::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return Serialize(Stm);
}

bool ShieldClass::Save(PhobosStreamWriter& Stm) const
{
	return const_cast<ShieldClass*>(this)->Serialize(Stm);
}

// =============================
//
// Is used for DeploysInto/UndeploysInto
void ShieldClass::SyncShieldToAnother(TechnoClass* pFrom, TechnoClass* pTo)
{
	const auto pFromExt = TechnoExt::ExtMap.Find(pFrom);
	const auto pToExt = TechnoExt::ExtMap.Find(pTo);

	if (pFromExt->Shield)
	{
		pToExt->CurrentShieldType = pFromExt->CurrentShieldType;
		pToExt->Shield = std::make_unique<ShieldClass>(pTo);
		strcpy_s(pToExt->Shield->TechnoID, pFromExt->Shield->TechnoID);
		pToExt->Shield->Available = pFromExt->Shield->Available;
		pToExt->Shield->HP = pFromExt->Shield->HP;
	}

	if (pFrom->WhatAmI() == AbstractType::Building && pFromExt->Shield)
		pFromExt->Shield = nullptr;
}

bool ShieldClass::ShieldIsBrokenTEvent(ObjectClass* pAttached)
{
	if (auto pTechno = abstract_cast<TechnoClass*>(pAttached))
	{
		if (auto pExt = TechnoExt::ExtMap.Find(pTechno))
		{
			ShieldClass* pShield = pExt->Shield.get();
			return !pShield || pShield->HP <= 0;
		}
	}

	return false;
}

int ShieldClass::ReceiveDamage(args_ReceiveDamage* args)
{
	const auto pWHExt = WarheadTypeExt::ExtMap.Find(args->WH);
	Armor armor = static_cast<Armor>(this->GetArmorIndex());

	if (!this->HP || this->Temporal || *args->Damage == 0 ||
		this->Techno->IsIronCurtained() || CanBePenetrated(pWHExt->OwnerObject()))
	{
		return *args->Damage;
	}

	int nDamage = 0;
	int shieldDamage = 0;
	int healthDamage = 0;

	if (pWHExt->CanTargetHouse(args->SourceHouse, this->Techno) && !args->WH->Temporal)
	{
		if (*args->Damage > 0)
		{
			nDamage = MapClass::GetTotalDamage
			(
				*args->Damage,
				args->WH,
				armor,
				args->DistanceToEpicenter
			);
		}
		else
		{
			nDamage = -MapClass::GetTotalDamage
			(
				-*args->Damage,
				args->WH,
				armor,
				args->DistanceToEpicenter
			);
		}

		bool affectsShield = pWHExt->Shield_AffectTypes.size() <= 0 || pWHExt->Shield_AffectTypes.Contains(this->Type);
		double absorbPercent = affectsShield ? pWHExt->Shield_AbsorbPercent.Get(this->Type->AbsorbPercent) : this->Type->AbsorbPercent;
		double passPercent = affectsShield ? pWHExt->Shield_PassPercent.Get(this->Type->PassPercent) : this->Type->PassPercent;
		double damageMultiplier = 1.0;

		if (this->Type->Directional && pWHExt->HitDir >= 0)
		{
			const int tarFacing = this->Techno->PrimaryFacing.Current().GetValue<16>();
			const int angle = abs(pWHExt->HitDir - tarFacing);
			const int frontField = static_cast<int>(16384 * this->Type->Directional_FrontField);
			const int backField = static_cast<int>(16384 * this->Type->Directional_BackField);

			if (angle >= 32768 - frontField && angle <= 32768 + frontField)//正面受击
			{
				if (this->Type->Directional_FrontMultiplier != 1.0)
					damageMultiplier *= this->Type->Directional_FrontMultiplier;
			}
			else if ((angle < backField && angle >= 0) || (angle > 49152 + backField && angle <= 65536))//背面受击
			{
				if (this->Type->Directional_BackMultiplier != 1.0)
					damageMultiplier *= this->Type->Directional_BackMultiplier;
			}
			else//侧面受击
			{
				if (this->Type->Directional_SideMultiplier != 1.0)
					damageMultiplier *= this->Type->Directional_SideMultiplier;
			}

			if (damageMultiplier <= 0)
				return *args->Damage;

			damageMultiplier *= WarheadTypeExt::ExtMap.Find(args->WH)->Directional_Multiplier;
		}

		shieldDamage = (int)((double)nDamage * absorbPercent * damageMultiplier);
		// passthrough damage shouldn't be affected by shield armor
		healthDamage = static_cast<int>(static_cast<double>(*args->Damage) * passPercent);
	}

	if (Phobos::Debug_DisplayDamageNumbers && shieldDamage != 0)
		TechnoExt::DisplayDamageNumberString(this->Techno, shieldDamage, true);

	if (shieldDamage > 0)
	{
		const int rate = this->Timers.SelfHealing_Warhead.InProgress() ? this->SelfHealing_Rate_Warhead : this->Type->SelfHealing_Rate;

		this->Timers.SelfHealing.Start(rate); // when attacked, restart the timer
		this->ResponseAttack();

		this->ShieldStolen(args, shieldDamage);

		if (pWHExt->DecloakDamagedTargets)
			this->Techno->Uncloak(false);

		this->ShieldStolen(args, shieldDamage);

		int residueDamage = shieldDamage - this->HP;
		if (residueDamage >= 0)
		{
			residueDamage = int((double)(residueDamage) /
				GeneralUtils::GetWarheadVersusArmor(args->WH, armor)); //only absord percentage damage

			this->BreakShield(pWHExt->Shield_BreakAnim.Get(nullptr), pWHExt->Shield_BreakWeapon.Get(nullptr));

			return this->Type->AbsorbOverDamage ? healthDamage : residueDamage + healthDamage;
		}
		else
		{
			AnimTypeClass* pHitAnimType = nullptr;

			int	hitDir = ((pWHExt->HitDir + 32768) % 65536);

			if (!pWHExt->Shield_HitAnim.empty())
			{
				if (pWHExt->Shield_HitAnim.size() > 1)
				{
					if (pWHExt->Shield_HitAnim_PickByDirection)
					{
						if (hitDir >= 0)
						{
							auto highest = Conversions::Int2Highest(static_cast<int>(pWHExt->Shield_HitAnim.size()));

							if (highest >= 3)
							{
								auto offset = 1u << (highest - 3);
								auto index = Conversions::TranslateFixedPoint(16, highest, static_cast<WORD>(hitDir), offset);
								pHitAnimType = pWHExt->Shield_HitAnim[index];
							}
							else
								pHitAnimType = pWHExt->Shield_HitAnim[0];
						}
					}
					else if (pWHExt->Shield_HitAnim_PickRandom)
						pHitAnimType = pWHExt->Shield_HitAnim[ScenarioClass::Instance->Random.RandomRanged(0, pWHExt->Shield_HitAnim.size() - 1)];
					else
						pHitAnimType = pWHExt->Shield_HitAnim[std::min(pWHExt->Shield_HitAnim.size() * 25 - 1, (size_t)shieldDamage) / 25];
				}
				else
					pHitAnimType = pWHExt->Shield_HitAnim[0];
			}

			this->WeaponNullifyAnim(pHitAnimType, hitDir, shieldDamage);
			this->HP = -residueDamage;

			UpdateIdleAnim();

			return healthDamage;
		}
	}
	else if (shieldDamage < 0)
	{
		const int nLostHP = this->Type->Strength - this->HP;

		if (!nLostHP)
		{
			int result = *args->Damage;
			TechnoExt::ExtData* pTechnoExt = TechnoExt::ExtMap.Find(this->Techno);

			if (result * GeneralUtils::GetWarheadVersusArmor(args->WH, static_cast<Armor>(pTechnoExt->GetArmorIdxWithoutShield())) > 0.0)
				result = 0;

			return result;
		}

		const int nRemainLostHP = nLostHP + shieldDamage;

		if (nRemainLostHP < 0)
			this->HP = this->Type->Strength;
		else
			this->HP -= shieldDamage;

		return 0;
	}

	// else if (nDamage == 0)
	UpdateIdleAnim();

	return healthDamage;
}

void ShieldClass::ResponseAttack()
{
	if (this->Techno->Owner != HouseClass::CurrentPlayer)
		return;

	if (const auto pBld = abstract_cast<BuildingClass*>(this->Techno))
	{
		this->Techno->Owner->BuildingUnderAttack(pBld);
	}
	else if (const auto pUnit = abstract_cast<UnitClass*>(this->Techno))
	{
		if (pUnit->Type->Harvester)
		{
			const auto pos = pUnit->GetDestination(pUnit);
			if (RadarEventClass::Create(RadarEventType::HarvesterAttacked, CellClass::Coord2Cell(pos)))
				VoxClass::Play(GameStrings::EVA_OreMinerUnderAttack);
		}
	}
}

void ShieldClass::ShieldStolen(args_ReceiveDamage* args, int shieldDamage)
{
	const auto pWHExt = WarheadTypeExt::ExtMap.Find(args->WH);
	if (args->Attacker)
	{
		auto const pAttackerType = TechnoTypeExt::ExtMap.Find(args->Attacker->GetTechnoType());
		const auto pAttacker = TechnoExt::ExtMap.Find(args->Attacker);

		if (this->GetType()->CanBeStolen &&
			pWHExt->Shield_Steal.Get() &&
			pWHExt->Shield_Assimilate_Rate.Get() >= 0 &&
			pAttacker->Shield.get() &&
			pAttackerType->ShieldType.Get()->Strength > 0)
		{
			double assimilateRate = pWHExt->Shield_Assimilate_Rate.Get() > 10 ? 1 : pWHExt->Shield_Assimilate_Rate.Get();
			int stolenHP = (int)(shieldDamage * assimilateRate);
			if (pAttackerType->ShieldType.Get()->Strength.Get() < pAttacker->Shield.get()->GetHP() + stolenHP)
			{
				pAttacker->Shield.get()->SetHP(pAttackerType->ShieldType.Get()->Strength);
			}
			else
			{
				pAttacker->Shield.get()->SetHP(pAttacker->Shield.get()->GetHP() + stolenHP);
			}
		}

		if (this->GetType()->CanBeStolenType && !pAttacker->Shield.get() && pWHExt->Shield_StealTargetType.Get())
		{
			pAttacker->CurrentShieldType = this->GetType();

			ShieldClass* newShield = new ShieldClass(pAttacker->OwnerObject());
			pAttacker->Shield.reset(newShield);

			auto initShieldRate = pWHExt->Shield_StealTargetType_InitShieldHealthRate;
			if (initShieldRate < 0 || initShieldRate > 1.0)
			{
				pAttacker->Shield.get()->SetHP(0);
			}
			else
			{
				pAttacker->Shield.get()->SetHP((int)(pAttacker->CurrentShieldType->Strength * initShieldRate));
			}
		}
	}
}

void ShieldClass::WeaponNullifyAnim(AnimTypeClass* pHitAnim, int hitDir, int shieldDamage)
{
	if (this->AreAnimsHidden)
		return;

	AnimTypeClass* pAnimType = pHitAnim;

	if (!pAnimType)
	{
		if (!this->Type->HitAnim.empty())
		{
			if (this->Type->HitAnim.size() > 1)
			{
				if (this->Type->HitAnim_PickByDirection)
				{
					if (hitDir < 0)
						return;

					auto highest = Conversions::Int2Highest(static_cast<int>(this->Type->HitAnim.size()));

					if (highest >= 3)
					{
						auto offset = 1u << (highest - 3);
						auto index = Conversions::TranslateFixedPoint(16, highest, static_cast<WORD>(hitDir), offset);
						pAnimType = this->Type->HitAnim[index];
					}
					else
						pAnimType = this->Type->HitAnim[0];
				}
				else if (this->Type->HitAnim_PickRandom)
					pAnimType = this->Type->HitAnim[ScenarioClass::Instance->Random.RandomRanged(0, this->Type->HitAnim.size() - 1)];
				else
					pAnimType = this->Type->HitAnim[std::min(this->Type->HitAnim.size() * 25 - 1, (size_t)shieldDamage) / 25];
			}
			else
				pAnimType = this->Type->HitAnim[0];
		}
	}

	if (pAnimType)
		GameCreate<AnimClass>(pAnimType, this->Techno->GetCoords());
}

bool ShieldClass::CanBeTargeted(WeaponTypeClass* pWeapon)
{
	if (!pWeapon)
		return false;

	if (CanBePenetrated(pWeapon->Warhead) || !this->HP)
		return true;

	return GeneralUtils::GetWarheadVersusArmor(pWeapon->Warhead, static_cast<Armor>(this->GetArmorIndex())) != 0.0;
}

bool ShieldClass::CanBePenetrated(const WarheadTypeClass* pWarhead)
{
	if (!pWarhead)
		return false;

	const auto pWHExt = WarheadTypeExt::ExtMap.Find(pWarhead);

	if (pWHExt->Shield_AffectTypes.size() > 0 && !pWHExt->Shield_AffectTypes.Contains(this->Type))
		return false;

	if (pWarhead->Psychedelic)
		return !this->Type->ImmuneToBerserk;

	return pWHExt->Shield_Penetrate;
}

void ShieldClass::AI_Temporal()
{
	if (!this->Temporal)
	{
		this->Temporal = true;

		const auto timer = (this->HP <= 0) ? &this->Timers.Respawn : &this->Timers.SelfHealing;
		timer->Pause();

		this->CloakCheck();

		if (this->IdleAnim)
		{
			switch (this->Type->IdleAnim_TemporalAction)
			{
			case AttachedAnimFlag::Hides:
				this->KillAnim();
				break;

			case AttachedAnimFlag::Temporal:
				this->IdleAnim->UnderTemporal = true;
				break;

			case AttachedAnimFlag::Paused:
				this->IdleAnim->Pause();
				break;

			case AttachedAnimFlag::PausedTemporal:
				this->IdleAnim->Pause();
				this->IdleAnim->UnderTemporal = true;
				break;
			}
		}
	}
}

void ShieldClass::AI()
{
	if (!this->Techno || this->Techno->InLimbo || this->Techno->IsImmobilized || this->Techno->Transporter)
		return;

	if (this->Techno->Health <= 0 || !this->Techno->IsAlive || this->Techno->IsSinking)
	{
		if (auto pTechnoExt = TechnoExt::ExtMap.Find(this->Techno))
		{
			pTechnoExt->Shield = nullptr;
			return;
		}
	}

	if (this->ConvertCheck())
		return;

	this->UpdateType();
	this->CloakCheck();

	if (!this->Available)
		return;

	this->TemporalCheck();
	if (this->Temporal)
		return;

	this->OnlineCheck();
	this->RespawnShield();
	this->SelfHealing();

	double ratio = this->Techno->GetHealthPercentage();

	if (!this->AreAnimsHidden)
	{
		if (GeneralUtils::HasHealthRatioThresholdChanged(LastTechnoHealthRatio, ratio))
			UpdateIdleAnim();

		if (!this->Cloak && !this->Temporal && this->Online && (this->HP > 0 && this->Techno->Health > 0))
			this->CreateAnim();
	}

	LastTechnoHealthRatio = ratio;
}

// The animation is automatically destroyed when the associated unit receives the isCloak statute.
// Therefore, we must zero out the invalid pointer
void ShieldClass::CloakCheck()
{
	const auto cloakState = this->Techno->CloakState;
	this->Cloak = cloakState == CloakState::Cloaked || cloakState == CloakState::Cloaking;

	if (this->Cloak)
		KillAnim();
}

void ShieldClass::OnlineCheck()
{
	if (!this->Type->Powered)
		return;

	const auto timer = (this->HP <= 0) ? &this->Timers.Respawn : &this->Timers.SelfHealing;

	auto pTechno = this->Techno;
	bool isActive = !(pTechno->Deactivated || pTechno->IsUnderEMP());

	if (isActive && this->Techno->WhatAmI() == AbstractType::Building)
	{
		auto const pBuilding = static_cast<BuildingClass const*>(this->Techno);
		isActive = pBuilding->IsPowerOnline();
	}

	if (!isActive)
	{
		this->Online = false;
		timer->Pause();

		if (this->IdleAnim)
		{
			switch (this->Type->IdleAnim_OfflineAction)
			{
			case AttachedAnimFlag::Hides:
				this->KillAnim();
				break;

			case AttachedAnimFlag::Temporal:
				this->IdleAnim->UnderTemporal = true;
				break;

			case AttachedAnimFlag::Paused:
				this->IdleAnim->Pause();
				break;

			case AttachedAnimFlag::PausedTemporal:
				this->IdleAnim->Pause();
				this->IdleAnim->UnderTemporal = true;
				break;
			}
		}
	}
	else
	{
		this->Online = true;
		timer->Resume();

		if (this->IdleAnim)
		{
			this->IdleAnim->UnderTemporal = false;
			this->IdleAnim->Unpause();
		}
	}
}

void ShieldClass::TemporalCheck()
{
	if (!this->Temporal)
		return;

	this->Temporal = false;

	const auto timer = (this->HP <= 0) ? &this->Timers.Respawn : &this->Timers.SelfHealing;
	timer->Resume();

	if (this->IdleAnim)
	{
		this->IdleAnim->UnderTemporal = false;
		this->IdleAnim->Unpause();
	}
}

// Is used for DeploysInto/UndeploysInto and DeploysInto/UndeploysInto
bool ShieldClass::ConvertCheck()
{
	const auto newID = this->Techno->get_ID();

	if (strcmp(this->TechnoID, newID) == 0)
		return false;

	const auto pTechnoExt = TechnoExt::ExtMap.Find(this->Techno);
	const auto pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(this->Techno->GetTechnoType());
	const auto pOldType = this->Type;
	bool allowTransfer = this->Type->AllowTransfer.Get(Attached);

	// Update shield type.
	if (!allowTransfer && !pTechnoTypeExt->ShieldType->Strength)
	{
		this->KillAnim();
		pTechnoExt->CurrentShieldType = ShieldTypeClass::FindOrAllocate(NONE_STR);
		pTechnoExt->Shield = nullptr;

		return true;
	}
	else if (pTechnoTypeExt->ShieldType->Strength)
	{
		pTechnoExt->CurrentShieldType = pTechnoTypeExt->ShieldType;
	}

	const auto pNewType = pTechnoExt->CurrentShieldType;

	// Update shield properties.
	if (pNewType->Strength && this->Available)
	{
		bool isDamaged = this->Techno->GetHealthPercentage() <= RulesClass::Instance->ConditionYellow;
		double healthRatio = this->GetHealthRatio();

		if (pOldType->GetIdleAnimType(isDamaged, healthRatio) != pNewType->GetIdleAnimType(isDamaged, healthRatio))
			this->KillAnim();

		this->HP = (int)round(
			(double)this->HP /
			(double)pOldType->Strength *
			(double)pNewType->Strength
		);
	}
	else
	{
		const auto timer = (this->HP <= 0) ? &this->Timers.Respawn : &this->Timers.SelfHealing;
		if (pNewType->Strength && !this->Available)
		{ // Resume this shield when became Available
			timer->Resume();
			this->Available = true;
		}
		else if (this->Available)
		{ // Pause this shield when became unAvailable
			timer->Pause();
			this->Available = false;
			this->KillAnim();
		}
	}

	strcpy_s(this->TechnoID, newID);

	return false;
}

void ShieldClass::SelfHealing()
{
	const auto pType = this->Type;
	const auto timer = &this->Timers.SelfHealing;
	const auto timerWH = &this->Timers.SelfHealing_Warhead;

	if (timerWH->Expired() && timer->InProgress())
	{
		int passedTime = Unsorted::CurrentFrame - timer->StartTime;
		int timeLeft = pType->SelfHealing_Rate - passedTime;
		timer->TimeLeft = timeLeft <= 0 ? 0 : timeLeft;
	}

	const double amount = timerWH->InProgress() ? this->SelfHealing_Warhead : pType->SelfHealing;
	const int rate = timerWH->InProgress() ? this->SelfHealing_Rate_Warhead : pType->SelfHealing_Rate;
	const auto percentageAmount = this->GetPercentageAmount(amount);

	if (percentageAmount != 0)
	{
		if ((this->HP < this->Type->Strength || percentageAmount < 0) && timer->StartTime == -1)
			timer->Start(rate);

		if (this->HP > 0 && timer->Completed())
		{
			timer->Start(rate);
			this->HP += percentageAmount;

			UpdateIdleAnim();

			if (this->HP > pType->Strength)
			{
				this->HP = pType->Strength;
				timer->Stop();
			}
			else if (this->HP <= 0)
			{
				BreakShield();
			}
		}
	}
}

int ShieldClass::GetPercentageAmount(double iStatus)
{
	if (iStatus == 0)
		return 0;

	if (iStatus >= -1.0 && iStatus <= 1.0)
		return (int)round(this->Type->Strength * iStatus);

	return (int)trunc(iStatus);
}

void ShieldClass::BreakShield(AnimTypeClass* pBreakAnim, WeaponTypeClass* pBreakWeapon)
{
	this->HP = 0;

	if (this->Type->Respawn)
		this->Timers.Respawn.Start(Timers.Respawn_Warhead.InProgress() ? Respawn_Rate_Warhead : this->Type->Respawn_Rate);

	this->Timers.SelfHealing.Stop();

	this->KillAnim();

	if (!this->AreAnimsHidden)
	{
		const auto pAnimType = pBreakAnim ? pBreakAnim : this->Type->BreakAnim.Get(nullptr);

		if (pAnimType)
		{
			if (auto const pAnim = GameCreate<AnimClass>(pAnimType, this->Techno->Location))
			{
				pAnim->SetOwnerObject(this->Techno);
				pAnim->Owner = this->Techno->Owner;
			}
		}
	}

	const auto pWeaponType = pBreakWeapon ? pBreakWeapon : this->Type->BreakWeapon.Get(nullptr);

	this->LastBreakFrame = Unsorted::CurrentFrame;

	if (pWeaponType)
		TechnoExt::FireWeaponAtSelf(this->Techno, pWeaponType);
}

void ShieldClass::RespawnShield()
{
	const auto timer = &this->Timers.Respawn;
	const auto timerWH = &this->Timers.Respawn_Warhead;

	if (this->HP <= 0 && timer->Completed())
	{
		timer->Stop();
		double amount = timerWH->InProgress() ? Respawn_Warhead : this->Type->Respawn;
		this->HP = this->GetPercentageAmount(amount);
	}
	else if (timerWH->Expired() && timer->InProgress())
	{
		int passedTime = Unsorted::CurrentFrame - timer->StartTime;
		int timeLeft = Type->Respawn_Rate - passedTime;
		timer->TimeLeft = timeLeft <= 0 ? 0 : timeLeft;
	}
}

void ShieldClass::SetRespawn(int duration, double amount, int rate, bool resetTimer)
{
	const auto timer = &this->Timers.Respawn;
	const auto timerWH = &this->Timers.Respawn_Warhead;

	this->Respawn_Warhead = amount > 0 ? amount : Type->Respawn;
	this->Respawn_Rate_Warhead = rate >= 0 ? rate : Type->Respawn_Rate;

	timerWH->Start(duration);

	if (this->HP <= 0 && Respawn_Rate_Warhead >= 0 && (resetTimer || timer->Expired()))
	{
		timer->Start(Respawn_Rate_Warhead);
	}
	else if (timer->InProgress())
	{
		int passedTime = Unsorted::CurrentFrame - timer->StartTime;
		int timeLeft = Respawn_Rate_Warhead - passedTime;
		timer->TimeLeft = timeLeft <= 0 ? 0 : timeLeft;
	}
}

void ShieldClass::SetSelfHealing(int duration, double amount, int rate, bool resetTimer)
{
	auto timer = &this->Timers.SelfHealing;
	auto timerWH = &this->Timers.SelfHealing_Warhead;

	this->SelfHealing_Warhead = amount;
	this->SelfHealing_Rate_Warhead = rate >= 0 ? rate : Type->SelfHealing_Rate;

	timerWH->Start(duration);

	if (this->HP < this->Type->Strength && (resetTimer || timer->Expired()))
	{
		timer->Start(this->SelfHealing_Rate_Warhead);
	}
	else if (timer->InProgress())
	{
		int passedTime = Unsorted::CurrentFrame - timer->StartTime;
		int timeLeft = SelfHealing_Rate_Warhead - passedTime;
		timer->TimeLeft = timeLeft <= 0 ? 0 : timeLeft;
	}
}

void ShieldClass::CreateAnim()
{
	auto idleAnimType = this->GetIdleAnimType();

	if (!this->IdleAnim && idleAnimType)
	{
		if (auto const pAnim = GameCreate<AnimClass>(idleAnimType, this->Techno->Location))
		{
			pAnim->SetOwnerObject(this->Techno);
			pAnim->Owner = this->Techno->Owner;
			pAnim->RemainingIterations = 0xFFu;
			this->IdleAnim = pAnim;
		}
	}
}

void ShieldClass::KillAnim()
{
	if (this->IdleAnim)
	{
		this->IdleAnim->DetachFromObject(this->Techno, false);
		this->IdleAnim = nullptr;
	}
}

void ShieldClass::UpdateIdleAnim()
{
	if (this->IdleAnim && this->IdleAnim->Type != this->GetIdleAnimType())
	{
		this->KillAnim();
		this->CreateAnim();
	}
}

AnimTypeClass* ShieldClass::GetIdleAnimType()
{
	if (!this->Type || !this->Techno)
		return nullptr;

	bool isDamaged = this->Techno->GetHealthPercentage() <= RulesClass::Instance->ConditionYellow;

	return this->Type->GetIdleAnimType(isDamaged, this->GetHealthRatio());
}

void ShieldClass::DrawShieldBar(HealthBarTypeClass* pShieldBar, int iLength, const Point2D& location, const RectangleStruct& bound)
{
	if (this->HP > 0 || this->Type->Respawn)
	{
		const auto ShieldBar = this->Type->ShieldBar.Get(pShieldBar);

		if (this->Techno->WhatAmI() == AbstractType::Building)
		{
			if (ShieldBar->PictureHealthBar.Get())
				this->DrawShieldBar_Picture(ShieldBar, iLength, location, bound);
			else if (ShieldBar->UnitHealthBar.Get())
				this->DrawShieldBar_Other(ShieldBar, iLength, location, bound);
			else
				this->DrawShieldBar_Building(ShieldBar, iLength, location, bound);
		}
		else
		{
			if (ShieldBar->PictureHealthBar.Get())
				this->DrawShieldBar_Picture(ShieldBar, iLength, location, bound);
			else
				this->DrawShieldBar_Other(ShieldBar, iLength, location, bound);
		}
	}
}

bool ShieldClass::IsGreenSP()
{
	return RulesClass::Instance->ConditionYellow * Type->Strength.Get() < HP;
}

bool ShieldClass::IsYellowSP()
{
	return RulesClass::Instance->ConditionRed * Type->Strength.Get() < HP && HP <= RulesClass::Instance->ConditionYellow * Type->Strength.Get();
}

bool ShieldClass::IsRedSP()
{
	return HP <= RulesClass::Instance->ConditionRed * Type->Strength.Get();
}

void ShieldClass::DrawShieldBar_Building(HealthBarTypeClass* pShieldBar, int iLength, const Point2D& location, const RectangleStruct& bound)
{
	SHPStruct* PipsSHP = pShieldBar->PipsSHP.Get() ? pShieldBar->PipsSHP.Get() : FileSystem::PIPS_SHP;
	ConvertClass* PipsPAL = pShieldBar->PipsPAL.GetOrDefaultConvert(FileSystem::PALETTE_PAL);
	CoordStruct vCoords = { 0, 0, 0 };
	this->Techno->GetTechnoType()->Dimension2(&vCoords);
	Point2D vPos2 = { 0, 0 };

	CoordStruct vCoords2 = { -vCoords.X / 2, vCoords.Y / 2,vCoords.Z };
	TacticalClass::Instance->CoordsToScreen(&vPos2, &vCoords2);

	Point2D vLoc = location;
	vLoc.X -= 5;
	vLoc.Y -= 3;

	Point2D vPos = { 0, 0 };

	const int iTotal = DrawShieldBar_PipAmount(iLength);
	int frame = DrawShieldBar_Pip(pShieldBar, true);


	vPos.X = vPos2.X + vLoc.X + 4 * iLength + 3;
	vPos.Y = vPos2.Y + vLoc.Y - 2 * iLength + 4;

	if (iTotal > 0)
	{
		int frameIdx, deltaX, deltaY;
		for (frameIdx = iTotal, deltaX = 0, deltaY = 0;
			frameIdx;
			frameIdx--, deltaX += 4, deltaY -= 2)
		{
			vPos.X = vPos2.X + vLoc.X + 4 * iLength + 3 - deltaX;
			vPos.Y = vPos2.Y + vLoc.Y - 2 * iLength + 4 - deltaY;

			DSurface::Temp->DrawSHP(PipsPAL, PipsSHP,
				frame, &vPos, &bound, BlitterFlags(0x600), 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);

		}
	}

	if (iTotal < iLength)
	{
		int frameIdx, deltaX, deltaY;
		for (frameIdx = iLength - iTotal, deltaX = 4 * iTotal, deltaY = -2 * iTotal;
			frameIdx;
			frameIdx--, deltaX += 4, deltaY -= 2)
		{
			int emptyFrame = this->Type->Pips_Building_Empty.Get(pShieldBar->Pips_Empty.Get(RulesExt::Global()->Pips_Shield_Building_Empty.Get(0)));

			vPos.X = vPos2.X + vLoc.X + 4 * iLength + 3 - deltaX;
			vPos.Y = vPos2.Y + vLoc.Y - 2 * iLength + 4 - deltaY;

			DSurface::Temp->DrawSHP(PipsPAL, PipsSHP,
				emptyFrame, &vPos, &bound, BlitterFlags(0x600), 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
		}
	}
}

void ShieldClass::DrawShieldBar_Other(HealthBarTypeClass* pShieldBar, int iLength, const Point2D& location, const RectangleStruct& bound)
{
	Point2D vPos = { 0,0 };
	Point2D vLoc = location;
	Point2D vDrawOffset = pShieldBar->DrawOffset.Get({ 2,0 });

	int frame, XOffset, YOffset;
	YOffset = pShieldBar->YOffset.Get((this->Techno->GetTechnoType()->PixelSelectionBracketDelta + this->Type->BracketDelta));
	vLoc.Y -= 5;

	SHPStruct* PipsSHP = pShieldBar->PipsSHP.Get() ? pShieldBar->PipsSHP.Get() : FileSystem::PIPS_SHP;
	ConvertClass* PipsPAL = pShieldBar->PipsPAL.GetOrDefaultConvert(FileSystem::PALETTE_PAL);
	SHPStruct* PipBrdSHP = this->Type->Pips_Background.Get(pShieldBar->PipBrdSHP.Get(RulesExt::Global()->Pips_Shield_Background.Get())) ?
		this->Type->Pips_Background.Get(pShieldBar->PipBrdSHP.Get(RulesExt::Global()->Pips_Shield_Background.Get())) : FileSystem::PIPBRD_SHP;
	ConvertClass* PipBrdPAL = pShieldBar->PipsPAL.GetOrDefaultConvert(FileSystem::PALETTE_PAL);

	if (iLength == 8)
	{
		vPos.X = vLoc.X + 11;
		vPos.Y = vLoc.Y - 25 + YOffset;
		frame = PipBrdSHP->Frames > 2 ? 3 : 1;
		XOffset = -5;
		YOffset -= 24;
	}
	else
	{
		vPos.X = vLoc.X + 1;
		vPos.Y = vLoc.Y - 26 + YOffset;
		frame = PipBrdSHP->Frames > 2 ? 2 : 0;
		XOffset = -15;
		YOffset -= 25;
	}

	if (this->Techno->IsSelected)
	{
		Point2D PipBrdOffset = pShieldBar->PipBrdOffset.Get();

		vPos.X += pShieldBar->XOffset.Get();
		vPos.X += PipBrdOffset.X;
		vPos.Y += PipBrdOffset.Y;

		DSurface::Temp->DrawSHP(PipBrdPAL, PipBrdSHP,
			pShieldBar->PipBrd.Get(frame), &vPos, &bound, BlitterFlags(0xE00), 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
	}

	const int iTotal = DrawShieldBar_PipAmount(pShieldBar->Length.Get(iLength));

	frame = this->DrawShieldBar_Pip(pShieldBar, false);

	for (int i = 0; i < iTotal; ++i)
	{
		vPos.X = vLoc.X + XOffset + vDrawOffset.X * i;
		vPos.Y = vLoc.Y + YOffset + vDrawOffset.Y * i;

		vPos.X += pShieldBar->XOffset.Get();

		DSurface::Temp->DrawSHP(PipsPAL, PipsSHP,
			frame, &vPos, &bound, BlitterFlags(0x600), 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
	}
}

void ShieldClass::DrawShieldBar_Picture(HealthBarTypeClass* pShieldBar, int iLength, const Point2D& location, const RectangleStruct& bound)
{
	Point2D vPos = { 0,0 };
	Point2D vLoc = location;

	int XOffset, YOffset;
	YOffset = pShieldBar->YOffset.Get((this->Techno->GetTechnoType()->PixelSelectionBracketDelta + this->Type->BracketDelta));
	vLoc.Y -= 5;

	SHPStruct* pShadpe = pShieldBar->PictureSHP.Get() ? pShieldBar->PictureSHP.Get() : FileSystem::PIPS_SHP;
	ConvertClass* pPalette = pShieldBar->PicturePAL.GetOrDefaultConvert(FileSystem::PALETTE_PAL);

	if (iLength == 8)
	{
		vPos.X = vLoc.X + 11;
		vPos.Y = vLoc.Y - 25 + YOffset;
		XOffset = -5;
		YOffset -= 24;
	}
	else
	{
		vPos.X = vLoc.X + 1;
		vPos.Y = vLoc.Y - 26 + YOffset;
		XOffset = -15;
		YOffset -= 25;
	}

	const auto length = pShieldBar->Length.Get(pShadpe->Frames - 1);
	const int iTotal = DrawShieldBar_PipAmount(length);
	vPos.X += pShieldBar->XOffset.Get();

	DSurface::Temp->DrawSHP(pPalette, pShadpe,
		iTotal, &vPos, &bound, EnumFunctions::GetTranslucentLevel(pShieldBar->PictureTransparency.Get()), 0, 0, ZGradient::Ground, 1000, 0, 0, 0, 0, 0);
}

int ShieldClass::DrawShieldBar_Pip(HealthBarTypeClass* pShieldBar, const bool isBuilding)
{
	const auto strength = this->Type->Strength;
	const auto pips_Shield = isBuilding ? this->Type->Pips_Building.Get(pShieldBar->Pips.Get()) : this->Type->Pips.Get(pShieldBar->Pips.Get());
	const auto pips_Global = isBuilding ? RulesExt::Global()->Pips_Shield_Building.Get() : RulesExt::Global()->Pips_Shield.Get();
	auto shieldPip = pips_Global;
	if (pips_Shield.X != -1)
		shieldPip = pips_Shield;

	if (this->HP > RulesClass::Instance->ConditionYellow * strength && shieldPip.X != -1)
		return shieldPip.X;
	else if (this->HP > RulesClass::Instance->ConditionRed * strength && (shieldPip.Y != -1 || shieldPip.X != -1))
		return shieldPip.Y == -1 ? shieldPip.X : shieldPip.Y;
	else if (shieldPip.Z != -1 || shieldPip.X != -1)
		return shieldPip.Z == -1 ? shieldPip.X : shieldPip.Z;

	return isBuilding ? 5 : 16;
}

int ShieldClass::DrawShieldBar_PipAmount(int iLength)
{
	return this->IsActive()
		? Math::clamp((int)round(this->GetHealthRatio() * iLength), 0, iLength)
		: 0;
}

double ShieldClass::GetHealthRatio()
{
	return static_cast<double>(this->HP) / this->Type->Strength;
}

int ShieldClass::GetHP()
{
	return this->HP;
}

void ShieldClass::SetHP(int amount)
{
	this->HP = amount;
	if (this->HP > this->Type->Strength)
		this->HP = this->Type->Strength;
}

ShieldTypeClass* ShieldClass::GetType()
{
	return this->Type;
}

int ShieldClass::GetFramesSinceLastBroken()
{
	return Unsorted::CurrentFrame - this->LastBreakFrame;
}

void ShieldClass::ReplaceArmor(int armorIdx)
{
	this->ArmorReplaced = true;
	this->ReplacedArmorIdx = armorIdx;
}

void ShieldClass::SetArmorReplaced(bool replaced)
{
	this->ArmorReplaced = replaced;
}

int ShieldClass::GetArmorIndex() const
{
	return this->ArmorReplaced ? this->ReplacedArmorIdx : static_cast<int>(this->Type->Armor.Get());
}

bool ShieldClass::IsActive()
{
	return
		this->Available &&
		this->HP > 0 &&
		this->Online;
}

bool ShieldClass::IsAvailable()
{
	return this->Available;
}

bool ShieldClass::IsBrokenAndNonRespawning()
{
	return this->HP <= 0 && !this->Type->Respawn;
}

void ShieldClass::SetAnimationVisibility(bool visible)
{
	if (!this->AreAnimsHidden && !visible)
		this->KillAnim();

	this->AreAnimsHidden = visible;
}
