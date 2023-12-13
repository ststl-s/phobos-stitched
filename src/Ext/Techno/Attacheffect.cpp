#include "Body.h"

#include <Ext/HouseType/Body.h>

#include <Utilities/Helpers.Alex.h>
#include <Utilities/EnumFunctions.h>

#include <JumpjetLocomotionClass.h>

#define Max(a, b) (a > b ? a : b)
#define Min(a, b) (a < b ? a : b)

void TechnoExt::AttachEffect(TechnoClass* pThis, TechnoClass* pInvoker, WarheadTypeExt::ExtData* pWHExt)
{
	if (!TechnoExt::IsReallyAlive(pThis))
		return;

	auto pExt = ExtMap.Find(pThis);
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	auto& vAE = pExt->AttachEffects;
	const auto& vAEType = pWHExt->AttachEffects;
	const auto& vResetTimer = pWHExt->AttachEffects_IfExist_ResetTimer;
	const auto& vResetAnim = pWHExt->AttachEffects_IfExist_ResetTimer;
	const auto& vAddTimer = pWHExt->AttachEffects_IfExist_AddTimer;
	const auto& vAddTimerCap = pWHExt->AttachEffects_IfExist_AddTimer_Cap;
	const auto& vDuration = pWHExt->AttachEffects_Duration;
	const auto& vRandomDuration = pWHExt->AttachEffects_RandomDuration;
	const auto& vRandomDurationInterval = pWHExt->AttachEffects_RandomDuration_Interval;
	const auto& vDelay = pWHExt->AttachEffects_Delay;

	for (size_t i = 0; i < pWHExt->DestroyAttachEffects.size(); i++)
	{
		AttachEffectTypeClass* pAEType = pWHExt->DestroyAttachEffects[i];

		std::for_each(vAE.begin(), vAE.end(),
			[pAEType](std::unique_ptr<AttachEffectClass>& pAE)
			{
				if (pAE->Type == pAEType)
				{
					pAE->IsInvalid = true;
				}
			});
	}

	for (size_t i = 0; i < pWHExt->DelayAttachEffects.size(); i++)
	{
		AttachEffectTypeClass* pAEType = pWHExt->DelayAttachEffects[i];

		if (i >= pWHExt->DelayAttachEffects_Time.size())
			break;

		int time = pWHExt->DelayAttachEffects_Time[i];

		std::for_each(vAE.begin(), vAE.end(),
			[pAEType, time](const std::unique_ptr<AttachEffectClass>& pAE)
			{
				if (pAE->Type == pAEType)
				{
					pAE->Delay_Timer.Start(pAE->Delay_Timer.GetTimeLeft() + time);
				}
			});
	}

	for (size_t i = 0; i < vAEType.size(); i++)
	{
		AttachEffectTypeClass* pAEType = vAEType[i];

		if (!AttachEffectClass::CanExist(pAEType))
			continue;

		if (!pAEType->PenetratesIronCurtain && pThis->IsIronCurtained())
			continue;

		if (pTypeExt->AttachEffects_Immune.Contains(pAEType)
			|| pTypeExt->AttachEffects_OnlyAccept.HasValue() && !pTypeExt->AttachEffects_OnlyAccept.Contains(pAEType)
			|| pAEType->MaxReceive > 0 && pExt->AttachEffects_ReceivedCounter[pAEType->ArrayIndex] >= pAEType->MaxReceive)
			continue;

		if (pAEType->MaxReceive > 0)
			++pExt->AttachEffects_ReceivedCounter[pAEType->ArrayIndex];

		int duration;
		bool randomDuration = i < vRandomDuration.size() ? vRandomDuration[i] : pAEType->RandomDuration;

		if (randomDuration)
		{
			Vector2D<int> Interval = vRandomDurationInterval.count(i) ? vRandomDurationInterval.at(i) : pAEType->RandomDuration_Interval;
			duration = ScenarioClass::Instance->Random.RandomRanged(Interval.X, Interval.Y);
		}
		else
		{
			duration = i < vDuration.size() ? vDuration[i] : pAEType->Duration;
		}

		if (!pAEType->Cumulative)
		{
			auto it = std::find_if(vAE.begin(), vAE.end(),
				[pAEType, pInvoker](const std::unique_ptr<AttachEffectClass>& pAE)
				{
					return pAE->Type == pAEType && (pAEType->IfExist_IgnoreOwner ? true : pAE->Owner == pInvoker);
				});

			if (it != vAE.end())
			{
				const auto& pAE = *it;
				bool resetTimer = i < vResetTimer.size() ? vResetTimer[i] : pAEType->IfExist_ResetTimer;
				bool resetAnim = i < vResetAnim.size() ? vResetAnim[i] : pAEType->IfExist_ResetAnim;

				if (resetTimer)
				{
					if (pAE->Timer.GetTimeLeft() < duration)
						pAE->Timer.Start(duration);
				}
				else
				{
					int addend = i < vAddTimer.size() ? vAddTimer[i] : pAEType->IfExist_AddTimer;
					int cap = i < vAddTimerCap.size() ? vAddTimerCap[i] : pAEType->IfExist_AddTimer_Cap;
					int timeLeft = pAE->Timer.GetTimeLeft();

					if (cap >= 0)
						addend = std::min(addend + timeLeft, cap);

					pAE->Timer.StartTime += addend;
				}

				if (resetAnim)
					pAE->ResetAnim();

				continue;
			}
		}
		else
		{
			int count = std::count_if(vAE.begin(), vAE.end(),
			[pAEType](const std::unique_ptr<AttachEffectClass>& pAE)
			{
				return pAE->Type == pAEType;
			}
			);

			if (pAEType->Cumulative_Maximum >= 0
				&& count >= pAEType->Cumulative_Maximum)
				continue;
		}

		int delay = i < vDelay.size() ? vDelay[i] : pAEType->Delay;
		std::unique_ptr<AttachEffectClass> pAE = std::make_unique<AttachEffectClass>(pAEType, pInvoker, pThis, duration, delay);
		pAE->Source = pWHExt->OwnerObject();
		vAE.emplace_back(std::move(pAE));
	}
}

void TechnoExt::AttachEffect(TechnoClass* pThis, TechnoClass* pInvoker, AttachEffectTypeClass* pAEType)
{
	if (!IsReallyAlive(pThis))
		return;

	if (!AttachEffectClass::CanExist(pAEType))
		return;

	if (!pAEType->PenetratesIronCurtain && pThis->IsIronCurtained())
		return;

	TechnoTypeExt::ExtData* pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (pTypeExt->AttachEffects_Immune.Contains(pAEType))
		return;

	if (pTypeExt->AttachEffects_OnlyAccept.HasValue()
		&& !pTypeExt->AttachEffects_OnlyAccept.Contains(pAEType))
		return;

	ExtData* pExt = ExtMap.Find(pThis);

	if (pAEType->MaxReceive > 0
		&& pExt->AttachEffects_ReceivedCounter[pAEType->ArrayIndex] >= pAEType->MaxReceive)
		return;

	if (pAEType->MaxReceive > 0)
		++pExt->AttachEffects_ReceivedCounter[pAEType->ArrayIndex];

	auto& vAE = pExt->AttachEffects;
	int duration = pAEType->RandomDuration
		? ScenarioClass::Instance->Random.RandomRanged(pAEType->RandomDuration_Interval.Get().X, pAEType->RandomDuration_Interval.Get().Y)
		: pAEType->Duration;

	if (!pAEType->Cumulative)
	{
		auto it = std::find_if(vAE.begin(), vAE.end(),
			[pAEType, pInvoker](const std::unique_ptr<AttachEffectClass>& pAE)
			{
				return pAE->Type == pAEType && (pAEType->IfExist_IgnoreOwner ? true : pAE->Owner == pInvoker);
			});

		if (it != vAE.end())
		{
			const auto& pAE = *it;

			if (pAEType->IfExist_ResetTimer)
			{
				if (pAE->Timer.GetTimeLeft() < duration)
					pAE->Timer.Start(duration);
			}
			else
			{
				int addend = pAEType->IfExist_AddTimer;

				if (pAEType->IfExist_AddTimer_Cap >= 0)
					addend = std::min(pAE->Timer.GetTimeLeft() + addend, pAEType->IfExist_AddTimer_Cap.Get());

				pAE->Timer.StartTime += addend;
			}

			if (pAE->Type->IfExist_ResetAnim)
				pAE->ResetAnim();

			return;
		}
	}
	else
	{
		int count = std::count_if(vAE.begin(), vAE.end(),
			[pAEType](const std::unique_ptr<AttachEffectClass>& pAE)
			{
				return pAE->Type == pAEType;
			}
		);

		if (pAEType->Cumulative_Maximum >= 0
			&& count >= pAEType->Cumulative_Maximum)
			return;
	}

	if (pAEType->Decloak)
		pThis->Uncloak(false);

	vAE.emplace_back(std::make_unique<AttachEffectClass>(pAEType, pInvoker, pThis, duration, pAEType->Delay));
}

void __fastcall CheckCloakable(TechnoExt::ExtData* pTechnoExt)
{
	TechnoClass* pTechno = pTechnoExt->OwnerObject();
	const auto pTypeExt = pTechnoExt->TypeExtData;
	TechnoExt::ExtData::AEBuff& buffs = pTechnoExt->AEBuffs;

	if (!pTypeExt->Cloakable_Allowed)
	{
		buffs.Cloakable = false;
		buffs.Decloak = true;
	}
	else
	{
		bool moving = false;
		bool deployed = false;
		bool powered = false;
		bool cloakable = buffs.Cloakable;
		bool decloak = buffs.Decloak;

		if (pTypeExt->CloakStop && pTechno->WhatAmI() != AbstractType::Building)
		{
			if (static_cast<FootClass*>(pTechno)->Locomotor->Is_Moving())
				moving = true;
		}

		if (pTypeExt->Cloakable_Deployed && pTechno->WhatAmI() == AbstractType::Infantry)
		{
			if (!static_cast<InfantryClass*>(pTechno)->IsDeployed())
				deployed = true;
		}

		if (pTypeExt->Cloakable_Powered && pTechno->WhatAmI() == AbstractType::Building)
		{
			if (pTechno->Owner->HasLowPower())
				powered = true;
		}

		if (decloak || !cloakable && (powered || deployed || moving))
		{
			buffs.Cloakable = false;
			buffs.Decloak = true;
		}
	}
}

void __fastcall ProcessBlackHole(const std::unique_ptr<AttachEffectClass>& pAE, TechnoExt::ExtData* pExt)
{
	TechnoClass* pThis = pExt->OwnerObject();
	const AttachEffectTypeClass* pAEType = pAE->Type;
	std::vector<BulletClass*> bullets(GeneralUtils::GetCellSpreadBullets(pThis->Location, pAEType->Blackhole_Range.Get()));

	for (auto const pBullet : bullets)
	{
		auto pBulletTypeExt = BulletTypeExt::ExtMap.Find(pBullet->Type);
		auto pBulletExt = BulletExt::ExtMap.Find(pBullet);

		if (!pBulletTypeExt || pBulletExt->Interfered)
			continue;

		auto bulletOwner = pBullet->Owner ? pBullet->Owner->Owner : pBulletExt->FirerHouse;

		if (pAEType->Blackhole_MinRange.Get() > 0)
		{
			auto distance = pBullet->Location.DistanceFrom(pThis->Location);

			if (distance < pAEType->Blackhole_MinRange.Get())
				continue;
		}

		if (EnumFunctions::CanTargetHouse(pAEType->Blackhole_AffectedHouse, pAE->OwnerHouse, bulletOwner))
		{
			if (pAEType->Blackhole_Destory)
			{
				if (pBulletTypeExt->ImmuneToBlackhole && pBulletTypeExt->ImmuneToBlackhole_Destory)
					continue;

				if (pAEType->Blackhole_Destory_TakeDamage)
				{
					int damage = Game::F2I(pBullet->Health * pAEType->Blackhole_Destory_TakeDamageMultiplier);
					pThis->TakeDamage
					(
						damage,
						pBullet->GetOwningHouse(),
						pBullet->Owner,
						pBullet->WeaponType->Warhead,
						false,
						false
					);
				}

				pBullet->Detonate(pBullet->GetCoords());
				pBullet->Limbo();
				pBullet->UnInit();

				const auto pTechno = pBullet->Owner;
				const bool isLimbo =
					pTechno &&
					pTechno->InLimbo &&
					pBullet->WeaponType &&
					pBullet->WeaponType->LimboLaunch;

				if (isLimbo)
				{
					pBullet->SetTarget(nullptr);
					auto damage = pTechno->Health * 2;
					pTechno->SetLocation(pBullet->GetCoords());
					pTechno->TakeDamage(damage);
				}

				if (pBulletExt->Passenger)
				{
					auto facing = static_cast<DirType>(ScenarioClass::Instance->Random.RandomRanged(0, 255));
					auto damage = pBulletExt->Passenger->Health * 2;
					pBulletExt->Passenger->Transporter = nullptr;
					++Unsorted::IKnowWhatImDoing;
					pBulletExt->Passenger->Unlimbo(pBullet->GetCoords(), facing);
					--Unsorted::IKnowWhatImDoing;
					pBulletExt->Passenger->TakeDamage(damage);
				}

				if (!pThis->IsAlive)
					return;
			}
			else
			{
				if (pBulletTypeExt->ImmuneToBlackhole)
					continue;

				pBullet->Target = pThis;
				pBullet->TargetCoords = pThis->Location;

				pBullet->Velocity.X = static_cast<double>(pBullet->TargetCoords.X - pBullet->Location.X);
				pBullet->Velocity.Y = static_cast<double>(pBullet->TargetCoords.Y - pBullet->Location.Y);
				pBullet->Velocity.Z = static_cast<double>(pBullet->TargetCoords.Z - pBullet->Location.Z);
				pBullet->Velocity *= 100 / pBullet->Velocity.Magnitude();

				pBulletExt->Interfered = true;
			}
		}
	}
}


void __fastcall ProcessSensor(const std::unique_ptr<AttachEffectClass>& pAE, TechnoExt::ExtData* pExt)
{
	TechnoClass* pThis = pExt->OwnerObject();
	const auto pTypeExt = pExt->TypeExtData;
	const TechnoTypeClass* pType = pTypeExt->OwnerObject();
	const AttachEffectTypeClass* pAEType = pAE->Type;
	int sight = pAEType->SensorsSight > 0 ? pAEType->SensorsSight : pType->Sight;

	CellStruct lastCell;
	CellStruct currentCell;

	if (pThis->WhatAmI() == AbstractType::Building)
	{
		lastCell = CellClass::Coord2Cell(pThis->GetCenterCoords());
		currentCell = lastCell;
	}
	else
	{
		FootClass* pFoot = static_cast<FootClass*>(pThis);

		if (locomotion_cast<JumpjetLocomotionClass*>(pFoot->Locomotor))
		{
			lastCell = pFoot->LastJumpjetMapCoords;
			currentCell = pFoot->CurrentJumpjetMapCoords;
		}
		else
		{
			lastCell = pFoot->LastMapCoords;
			currentCell = pFoot->CurrentMapCoords;
		}
	}

	if (lastCell != currentCell)
	{
		TechnoExt::RemoveSensorsAt(pAE->OwnerHouse->ArrayIndex, sight, pExt->SensorCell);
		pExt->SensorCell = currentCell;
		TechnoExt::AddSensorsAt(pAE->OwnerHouse->ArrayIndex, sight, pExt->SensorCell);
	}
}

void __fastcall ProcessMoveDamage(const std::unique_ptr<AttachEffectClass>& pAE, TechnoExt::ExtData* pExt)
{
	const AttachEffectTypeClass* pAEType = pAE->Type;
	FootClass* pFoot = static_cast<FootClass*>(pExt->OwnerObject());

	if (pAEType->MoveDamage != 0)
	{
		if (pFoot->LastMapCoords != pFoot->CurrentMapCoords)
		{
			if (pAE->MoveDamageCount > 0)
				pAE->MoveDamageCount--;
			else
			{
				pAE->MoveDamageCount = pAEType->MoveDamage_Delay;

				pFoot->TakeDamage
				(
					pAEType->MoveDamage,
					pAE->OwnerHouse,
					nullptr,
					pAEType->MoveDamage_Warhead == nullptr ? RulesClass::Instance->C4Warhead : pAEType->MoveDamage_Warhead
				);

				if (pAEType->MoveDamage_Anim)
				{
					if (auto pAnim = GameCreate<AnimClass>(pAEType->MoveDamage_Anim, pFoot->GetCenterCoords()))
					{
						pAnim->SetOwnerObject(pFoot);
						pAnim->Owner = pAE->OwnerHouse;
					}
				}

				if (!pFoot->IsAlive)
					return;
			}
		}
		else
		{
			if (pAE->MoveDamageCount > 0)
				pAE->MoveDamageCount--;
		}
	}
}

void __fastcall ProcessStopDamage(const std::unique_ptr<AttachEffectClass>& pAE, TechnoExt::ExtData* pExt)
{
	const AttachEffectTypeClass* pAEType = pAE->Type;
	FootClass* pFoot = static_cast<FootClass*>(pExt->OwnerObject());

	if (pAEType->StopDamage != 0)
	{
		if (pFoot->LastMapCoords != pFoot->CurrentMapCoords)
		{
			if (pAE->StopDamageCount > 0)
				pAE->StopDamageCount--;
		}
		else
		{
			if (pAE->StopDamageCount > 0)
				pAE->StopDamageCount--;
			else
			{
				pAE->StopDamageCount = pAEType->StopDamage_Delay;

				pFoot->TakeDamage
				(
					pAEType->StopDamage,
					pAE->OwnerHouse,
					nullptr,
					pAEType->StopDamage_Warhead == nullptr ? RulesClass::Instance->C4Warhead : pAEType->StopDamage_Warhead
				);

				if (pAEType->StopDamage_Anim)
				{
					if (auto pAnim = GameCreate<AnimClass>(pAEType->StopDamage_Anim, pFoot->GetCenterCoords()))
					{
						pAnim->SetOwnerObject(pFoot);
						pAnim->Owner = pAE->OwnerHouse;
					}
				}

				if (!pFoot->IsAlive)
					return;
			}
		}
	}
}

void TechnoExt::ExtData::CheckAttachEffects()
{
	TechnoClass* pThis = this->OwnerObject();
	const TechnoTypeExt::ExtData* pTypeExt = this->TypeExtData;
	const TechnoTypeClass* pType = pTypeExt->OwnerObject();

	if (!AttachEffects_Initialized)
	{
		HouseTypeClass* pHouseType = pThis->Owner->Type;

		for (const auto pAEType : pTypeExt->AttachEffects)
		{
			AttachEffect(pThis, pThis, pAEType);
		}

		if (const auto pAEType = HouseTypeExt::GetAttachEffectOnInit(pHouseType, pThis))
			AttachEffect(pThis, pThis, pAEType);

		this->AttachEffects_Initialized = true;
	}

	this->AttachEffects.erase
	(
		std::remove_if
		(
			this->AttachEffects.begin(),
			this->AttachEffects.end(),
			[](const std::unique_ptr<AttachEffectClass>& pAE)
			{
				return pAE == nullptr
					|| pAE->Timer.Completed()
					|| pAE->Type->DiscardAfterShoots <= pAE->AttachOwnerShoots
					|| pAE->Type->DiscardAfterHits > 0 && pAE->AttachOwnerAttackedCounter >= pAE->Type->DiscardAfterHits
					|| pAE->IsInvalid
					;
			}
		)
		, this->AttachEffects.end());

	bool armorReplaced = false;
	bool armorReplaced_Shield = false;

	for (const auto& pAE : this->AttachEffects)
	{
		if (!pAE)
			continue;

		pAE->Update();

		if (!pThis->IsAlive)
			return;

		const AttachEffectTypeClass* pAEType = pAE->Type;

		if (pAE->IsActive())
		{
			if (pAEType->ReplaceArmor.isset())
			{
				this->ReplacedArmorIdx = pAEType->ReplaceArmor.Get();
				armorReplaced = true;
			}

			if (pAEType->ReplaceArmor_Shield.isset() && this->Shield != nullptr)
			{
				this->Shield->ReplaceArmor(pAEType->ReplaceArmor_Shield.Get());
				armorReplaced_Shield = true;
			}

			if (pAEType->EMP && !(this->TypeExtData->ImmuneToEMP))
			{
				if (pThis->IsUnderEMP())
				{
					if (pThis->EMPLockRemaining <= 2)
						pThis->EMPLockRemaining++;
				}
				else
					pThis->EMPLockRemaining = 2;
			}

			if (pAEType->Psychedelic && !(pTypeExt->ImmuneToBerserk.isset() ? pTypeExt->ImmuneToBerserk : pType->ImmuneToPsionics))
			{
				if (pThis->Berzerk)
				{
					if (pThis->BerzerkDurationLeft <= 2)
						pThis->BerzerkDurationLeft++;
				}
				else
				{
					pThis->Berzerk = true;
					pThis->BerzerkDurationLeft = 2;
				}
			}

			if (pAEType->RevealSight != 0)
			{
				int sight = pAEType->RevealSight > 0 ? pAE->Type->RevealSight : pType->Sight;
				CoordStruct coords = pThis->GetCenterCoords();
				MapClass::Instance->RevealArea1(&coords, sight, pAE->OwnerHouse, CellStruct::Empty, 0, 0, 0, 1);
			}

			if (pAEType->SensorsSight != 0)
				ProcessSensor(pAE, this);

			if (pThis->AbstractFlags & AbstractFlags::Foot)
			{
				ProcessMoveDamage(pAE, this);
				ProcessStopDamage(pAE, this);
			}

			if (pAEType->Blackhole_Range.Get() > 0)
				ProcessBlackHole(pAE, this);
		}
	}

	this->ArmorReplaced = armorReplaced;

	if (Shield != nullptr)
		Shield->SetArmorReplaced(armorReplaced_Shield);

	std::vector<AttachEffectClass*> activeAEs = this->GetActiveAE();
	this->ResetAEBuffs();
	AEBuff& buffs = this->AEBuffs;

	std::for_each(activeAEs.cbegin(), activeAEs.cend(),
		[&buffs](const AttachEffectClass* const pAE)
		{
			const AttachEffectTypeClass* const pAEType = pAE->Type;
			buffs.FirepowerMul *= pAEType->FirePower_Multiplier;
			buffs.ROFMul *= pAEType->ROF_Multiplier;
			buffs.ArmorMul *= pAEType->Armor_Multiplier;
			buffs.SpeedMul *= pAEType->Speed_Multiplier;
			buffs.ROTMul *= pAEType->ROT_Multiplier;
			buffs.RangeMul *= pAEType->Range_Multiplier;
			buffs.WeightMul *= pAEType->Weight_Multiplier;
			buffs.Firepower += pAEType->FirePower;
			buffs.ROF += pAEType->ROF;
			buffs.Armor += pAEType->Armor;
			buffs.Speed += pAEType->Speed * 256 / 100;
			buffs.ROT += pAEType->ROT;
			buffs.Range += pAEType->Range.Get().value;
			buffs.Weight += pAEType->Weight;

			if (pAEType->DecloakToFire.isset())
				buffs.DecloakToFire = pAEType->DecloakToFire.Get();

			buffs.DisableWeapon |= pAEType->DisableWeapon_Category;
			buffs.LimitMaxPostiveDamage = Min(buffs.LimitMaxPostiveDamage, pAEType->LimitDamage_MaxDamage.Get().X);
			buffs.LimitMaxNegtiveDamage = Max(buffs.LimitMaxNegtiveDamage, pAEType->LimitDamage_MaxDamage.Get().Y);
			buffs.LimitMinPostiveDamage = Max(buffs.LimitMinPostiveDamage, pAEType->LimitDamage_MinDamage.Get().X);
			buffs.LimitMinNegtiveDamage = Min(buffs.LimitMinNegtiveDamage, pAEType->LimitDamage_MinDamage.Get().Y);

			buffs.Cloakable |= pAEType->Cloak;
			buffs.Decloak |= pAEType->Decloak;
			buffs.DisableTurn |= fabs(pAEType->ROT_Multiplier) < DBL_EPSILON;
			buffs.DisableSelect |= pAEType->DisableBeSelect;
			buffs.ImmuneMindControl |= pAEType->ImmuneMindControl;
			buffs.HideImage |= pAEType->HideImage;
		});

	CheckCloakable(this);
}

bool TechnoExt::ExtData::HasAttachedEffects(std::vector<AttachEffectTypeClass*> attachEffectTypes, bool requireAll, bool ignoreSameSource, TechnoClass* pInvoker, AbstractClass* pSource)
{
	unsigned int foundCount = 0;
	unsigned int typeCounter = 1;

	for (auto const& type : attachEffectTypes)
	{
		for (auto const& attachEffect : this->AttachEffects)
		{
			if (attachEffect->Type == type)
			{
				if (ignoreSameSource && pInvoker && pSource && attachEffect->IsFromSource(pInvoker, pSource))
					continue;

				// Only need to find one match, can stop here.
				if (!requireAll)
					return true;

				foundCount++;
				break;
			}
		}

		// One of the required types was not found, can stop here.
		if (requireAll && foundCount < typeCounter)
			return false;

		typeCounter++;
	}

	if (requireAll && foundCount == attachEffectTypes.size())
		return true;

	return false;
}

void TechnoExt::ExtData::DisableBeSelect()
{
	if (this->AEBuffs.DisableSelect)
		this->OwnerObject()->Deselect();
}

void TechnoExt::ExtData::DeployAttachEffect()
{
	if (!TechnoExt::IsReallyAlive(OwnerObject()))
		return;

	TechnoClass* pThis = OwnerObject();
	if (auto const pInfantry = abstract_cast<InfantryClass*>(OwnerObject()))
	{
		if (pInfantry->IsDeployed())
		{
			const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
			if (DeployAttachEffectsCount > 0)
			{
				DeployAttachEffectsCount--;
			}
			else
			{
				for (size_t i = 0; i < pTypeExt->DeployAttachEffects.size(); i++)
					AttachEffect(pThis, pThis, pTypeExt->DeployAttachEffects[i]);
				DeployAttachEffectsCount = pTypeExt->DeployAttachEffects_Delay;
			}
		}
		else if (DeployAttachEffectsCount > 0)
			DeployAttachEffectsCount--;
	}
}

void TechnoExt::ExtData::AttachEffectNext()
{
	TechnoClass* pThis = OwnerObject();

	if (!TechnoExt::IsReallyAlive(pThis))
		return;

	if (NextAttachEffects.size() > 0)
	{
		for (size_t i = 0; i < NextAttachEffects.size(); i++)
		{
			AttachEffect(pThis, NextAttachEffectsOwner, NextAttachEffects[i]);
		}
		NextAttachEffects.clear();
		NextAttachEffectsOwner = nullptr;
	}
}

std::vector<AttachEffectClass*> TechnoExt::ExtData::GetActiveAE() const
{
	std::vector<AttachEffectClass*> result;

	result.reserve(this->AttachEffects.size());

	for (const auto& pAE : this->AttachEffects)
	{
		if (pAE->IsActive())
			result.emplace_back(pAE.get());
	}

	if (this->ParentAttachment
		&& this->ParentAttachment->GetType()->InheritStateEffects)
	{
		auto pParentExt = TechnoExt::ExtMap.Find(this->ParentAttachment->Parent);
		for (const auto& pAE : pParentExt->AttachEffects)
		{
			if (pAE->IsActive())
				result.emplace_back(pAE.get());
		}
	}

	for (auto const& pAttachment : this->ChildAttachments)
	{
		if (pAttachment->GetType()->InheritStateEffects_Parent)
		{
			if (auto pChildExt = TechnoExt::ExtMap.Find(pAttachment->Child))
			{
				for (const auto& pAE : pChildExt->AttachEffects)
				{
					if (pAE->IsActive())
						result.emplace_back(pAE.get());
				}
			}
		}
	}

	return result;
}

void TechnoExt::ExtData::RecalculateROT()
{
	TechnoClass* pThis = OwnerObject();
	const TechnoTypeExt::ExtData* pTypeExt = this->TypeExtData;

	bool disable = DisableTurnCount > 0;

	if (disable)
		--DisableTurnCount;

	TechnoTypeClass* pType = pThis->GetTechnoType();
	double dblROTMultiplier = this->AEBuffs.ROTMul;
	int iROTBuff = this->AEBuffs.ROT;
	disable |= this->AEBuffs.DisableTurn;

	int iROT_Primary = Game::F2I(pType->ROT * dblROTMultiplier) + iROTBuff;
	int iROT_Secondary = Game::F2I(pTypeExt->TurretROT.Get(pType->ROT) * dblROTMultiplier) + iROTBuff;
	pThis->PrimaryFacing.SetROT(iROT_Primary < 0 ? 1 : iROT_Primary);
	pThis->SecondaryFacing.SetROT(iROT_Secondary < 0 ? 1 : iROT_Secondary);

	if (FacingInitialized && (iROT_Primary < 0 || disable))
	{
		pThis->PrimaryFacing.SetCurrent(LastSelfFacing);
		pThis->PrimaryFacing.SetDesired(LastSelfFacing);
	}

	if (FacingInitialized && (iROT_Secondary < 0 || disable))
	{
		pThis->SecondaryFacing.SetCurrent(LastTurretFacing);
		pThis->SecondaryFacing.SetDesired(LastTurretFacing);
	}

	LastSelfFacing = pThis->PrimaryFacing.Current();
	LastTurretFacing = pThis->SecondaryFacing.Current();
	FacingInitialized = true;
}
