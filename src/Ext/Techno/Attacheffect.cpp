#include "Body.h"

#include <Ext/HouseType/Body.h>

#include <Utilities/Helpers.Alex.h>
#include <Utilities/EnumFunctions.h>

#include <JumpjetLocomotionClass.h>

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

			if (count >= pAEType->Cumulative_Maximum)
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

		if (count >= pAEType->Cumulative_Maximum)
			return;
	}

	if (pAEType->Decloak)
		pThis->Uncloak(false);

	vAE.emplace_back(std::make_unique<AttachEffectClass>(pAEType, pInvoker, pThis, duration, pAEType->Delay));
}

void TechnoExt::ExtData::CheckAttachEffects()
{
	TechnoClass* pThis = this->OwnerObject();

	if (!TechnoExt::IsReallyAlive(pThis))
		return;

	if (!AttachEffects_Initialized)
	{
		TechnoTypeExt::ExtData* pTypeExt = this->TypeExtData;
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
	//bool decloak = false;
	//bool cloakable = SessionClass::IsSingleplayer() ? (TechnoExt::CanICloakByDefault(pThis) || this->Crate_Cloakable) : false;

	for (const auto& pAE : this->AttachEffects)
	{
		if (!pAE)
			continue;

		pAE->Update();

		if (!TechnoExt::IsReallyAlive(pThis))
			return;

		if (pAE->IsActive())
		{
			if (pAE->Type->ReplaceArmor.isset())
			{
				this->ReplacedArmorIdx = pAE->Type->ReplaceArmor.Get();
				armorReplaced = true;
			}

			if (pAE->Type->ReplaceArmor_Shield.isset() && this->Shield != nullptr)
			{
				this->Shield->ReplaceArmor(pAE->Type->ReplaceArmor_Shield.Get());
				armorReplaced_Shield = true;
			}

			//cloakable |= pAE->Type->Cloak;
			//decloak |= pAE->Type->Decloak;

			if (pAE->Type->EMP && !(this->TypeExtData->ImmuneToEMP))
			{
				if (pThis->IsUnderEMP())
				{
					if (pThis->EMPLockRemaining <= 2)
						pThis->EMPLockRemaining++;
				}
				else
					pThis->EMPLockRemaining = 2;
			}

			if (pAE->Type->Psychedelic && !(this->TypeExtData->ImmuneToBerserk.isset() ? this->TypeExtData->ImmuneToBerserk.Get() : pThis->GetTechnoType()->ImmuneToPsionics))
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

			if (pAE->Type->RevealSight != 0)
			{
				int sight = pAE->Type->RevealSight > 0 ? pAE->Type->RevealSight : pThis->GetTechnoType()->Sight;
				CoordStruct coords = pThis->GetCenterCoords();
				MapClass::Instance->RevealArea1(&coords, sight, pAE->OwnerHouse, CellStruct::Empty, 0, 0, 0, 1);
			}

			if (pAE->Type->SensorsSight != 0)
			{
				int sight = pAE->Type->SensorsSight > 0 ? pAE->Type->SensorsSight : pThis->GetTechnoType()->Sight;

				CellStruct lastCell;
				CellStruct currentCell;

				if (pThis->WhatAmI() == AbstractType::Building)
				{
					lastCell = CellClass::Coord2Cell(pThis->GetCenterCoords());
					currentCell = lastCell;
				}
				else
				{
					auto const pFoot = abstract_cast<FootClass*>(pThis);
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
					TechnoExt::RemoveSensorsAt(pAE->OwnerHouse->ArrayIndex, sight, this->SensorCell);
					this->SensorCell = currentCell;
					TechnoExt::AddSensorsAt(pAE->OwnerHouse->ArrayIndex, sight, this->SensorCell);
				}
			}

			if (pAE->Type->MoveDamage != 0)
			{
				auto const pFoot = abstract_cast<FootClass*>(pThis);
				if (pFoot->LastMapCoords != pFoot->CurrentMapCoords)
				{
					if (pAE->MoveDamageCount > 0)
						pAE->MoveDamageCount--;
					else
					{
						pAE->MoveDamageCount = pAE->Type->MoveDamage_Delay;

						pThis->TakeDamage
						(
							pAE->Type->MoveDamage,
							pAE->OwnerHouse,
							nullptr,
							pAE->Type->MoveDamage_Warhead == nullptr ? RulesClass::Instance->C4Warhead : pAE->Type->MoveDamage_Warhead
						);

						if (pAE->Type->MoveDamage_Anim)
						{
							if (auto pAnim = GameCreate<AnimClass>(pAE->Type->MoveDamage_Anim, pThis->Location))
							{
								pAnim->SetOwnerObject(pThis);
								pAnim->Owner = pAE->OwnerHouse;
							}
						}

						if (!TechnoExt::IsReallyAlive(pThis))
							return;
					}
				}
				else
				{
					if (pAE->MoveDamageCount > 0)
						pAE->MoveDamageCount--;
				}
			}

			if (pAE->Type->StopDamage != 0)
			{
				auto const pFoot = abstract_cast<FootClass*>(pThis);
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
						pAE->StopDamageCount = pAE->Type->StopDamage_Delay;

						pThis->TakeDamage
						(
							pAE->Type->StopDamage,
							pAE->OwnerHouse,
							nullptr,
							pAE->Type->StopDamage_Warhead == nullptr ? RulesClass::Instance->C4Warhead : pAE->Type->StopDamage_Warhead
						);

						if (pAE->Type->StopDamage_Anim)
						{
							if (auto pAnim = GameCreate<AnimClass>(pAE->Type->StopDamage_Anim, pThis->Location))
							{
								pAnim->SetOwnerObject(pThis);
								pAnim->Owner = pAE->OwnerHouse;
							}
						}

						if (!TechnoExt::IsReallyAlive(pThis))
							return;
					}
				}
			}

			if (pAE->Type->Blackhole_Range > 0)
			{
				std::vector<BulletClass*> vBullets(std::move(GeneralUtils::GetCellSpreadBullets(pThis->Location, (pAE->Type->Blackhole_Range * 256))));

				for (auto const pBullet : vBullets)
				{
					auto pBulletTypeExt = BulletTypeExt::ExtMap.Find(pBullet->Type);
					auto pBulletExt = BulletExt::ExtMap.Find(pBullet);

					if (!pBulletTypeExt || pBulletExt->Interfered)
						continue;

					auto bulletOwner = pBullet->Owner ? pBullet->Owner->Owner : pBulletExt->FirerHouse;

					if (pAE->Type->Blackhole_MinRange > 0)
					{
						const auto& minguardRange = pAE->Type->Blackhole_MinRange * 256;

						auto distance = pBullet->Location.DistanceFrom(pThis->Location);

						if (distance < minguardRange)
							continue;
					}

					if (EnumFunctions::CanTargetHouse(pAE->Type->Blackhole_AffectedHouse, pAE->OwnerHouse, bulletOwner))
					{
						if (pAE->Type->Blackhole_Destory)
						{
							if (pBulletTypeExt->ImmuneToBlackhole && pBulletTypeExt->ImmuneToBlackhole_Destory)
								continue;

							if (pAE->Type->Blackhole_Destory_TakeDamage)
							{
								int damage = static_cast<int>(pBullet->Health * pAE->Type->Blackhole_Destory_TakeDamageMultiplier);
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

							if (!TechnoExt::IsReallyAlive(pThis))
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
		}
	}

	if (!TechnoExt::IsReallyAlive(pThis))
		return;

	this->ArmorReplaced = armorReplaced;

	if (Shield != nullptr)
		Shield->SetArmorReplaced(armorReplaced_Shield);

	/*if (SessionClass::IsSingleplayer())
		pThis->Cloakable = cloakable && !decloak;*/
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
	TechnoClass* pThis = OwnerObject();

	if (pThis->IsSelected)
	{
		for (const auto& pAE : this->GetActiveAE())
		{
			if (pAE->Type->DisableBeSelect)
			{
				pThis->Deselect();
				break;
			}
		}
	}
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

	if (pThis->WhatAmI() == AbstractType::Building
		&& pTypeExt->EMPulseCannon)
		return;

	bool disable = DisableTurnCount > 0;

	if (disable)
		--DisableTurnCount;

	TechnoTypeClass* pType = pThis->GetTechnoType();
	double dblROTMultiplier = 1.0;
	int iROTBuff = 0;

	for (const auto pAE : this->GetActiveAE())
	{
		dblROTMultiplier *= pAE->Type->ROT_Multiplier;
		iROTBuff += pAE->Type->ROT;

		if (fabs(pAE->Type->ROT_Multiplier) < DBL_EPSILON)
			disable = true;
	}

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

double TechnoExt::ExtData::GetAEFireMul(int* adden) const
{
	double dblMultiplier = 1.0;
	int iDamageBuff = 0;

	const std::vector<AttachEffectClass*> attachEffects = this->GetActiveAE();

	for (const auto& pAE : attachEffects)
	{
		dblMultiplier *= pAE->Type->FirePower_Multiplier;
		iDamageBuff += pAE->Type->FirePower;
	}

	if (adden != nullptr)
		*adden = iDamageBuff;

	return dblMultiplier;
}

double TechnoExt::ExtData::GetAERangeMul(double* adden) const
{
	double rangeBuff = 0;
	double dblMultiplier = 1.0;

	const std::vector<AttachEffectClass*> attachEffects = this->GetActiveAE();

	for (const auto& pAE : attachEffects)
	{
		dblMultiplier *= pAE->Type->Range_Multiplier;
		rangeBuff += pAE->Type->Range;
	}

	if (adden != nullptr)
		*adden = rangeBuff;

	return dblMultiplier;
}

double TechnoExt::ExtData::GetAEROFMul(int* adden) const
{
	int iROFBuff = 0;
	double dblMultiplier = 1.0;

	const std::vector<AttachEffectClass*> attachEffects = this->GetActiveAE();

	for (const auto& pAE : attachEffects)
	{
		dblMultiplier *= pAE->Type->ROF_Multiplier;
		iROFBuff += pAE->Type->ROF;
	}
	
	if (adden != nullptr)
		*adden = iROFBuff;

	return dblMultiplier;
}

double TechnoExt::ExtData::GetAESpeedMul(int* adden) const
{
	int iSpeedBuff = 0;
	double dblMultiplier = 1.0;
	const std::vector<AttachEffectClass*> attachEffects = this->GetActiveAE();

	for (const auto& pAE : attachEffects)
	{
		dblMultiplier *= pAE->Type->Speed_Multiplier;
		iSpeedBuff += pAE->Type->Speed;
	}

	if (adden != nullptr)
		*adden = iSpeedBuff;

	return dblMultiplier;
}

double TechnoExt::ExtData::GetAEArmorMul(int* adden) const
{
	int iArmorBuff = 0;
	double dblMultiplier = 1.0;
	const std::vector<AttachEffectClass*> attachEffects = this->GetActiveAE();

	for (const auto& pAE : attachEffects)
	{
		iArmorBuff += pAE->Type->Armor;
		dblMultiplier *= pAE->Type->Armor_Multiplier;
	}

	if (adden != nullptr)
		*adden = iArmorBuff;

	return dblMultiplier;
}

double TechnoExt::ExtData::GetAEWeightMul(double* adden) const
{
	double weightBuff = 0.0;
	double multiplier = 1.0;

	for (const auto& pAE : this->GetActiveAE())
	{
		weightBuff += pAE->Type->Weight;
		multiplier *= pAE->Type->Weight_Multiplier;
	}

	if (adden != nullptr)
		*adden = weightBuff;

	return multiplier;
}
