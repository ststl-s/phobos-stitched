#include "Body.h"

#include <Ext/HouseType/Body.h>

#include <Utilities/Helpers.Alex.h>

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
					|| pAE->Type->DiscardAfterHits > 0 && pAE->AttachOwnerAttackedCounter >= pAE->Type->DiscardAfterHits
					|| pAE->IsInvalid
					;
			}
		)
		, this->AttachEffects.end()
				);

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

			if (pAE->Type->SensorsSight != 0)
			{
				int sight = pAE->Type->SensorsSight > 0 ? pAE->Type->SensorsSight : pThis->GetTechnoType()->Sight;

				auto const pFoot = abstract_cast<FootClass*>(pThis);
				CellStruct lastCell;
				CellStruct currentCell;
				
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

				if (lastCell != currentCell)
				{
					TechnoExt::RemoveSensorsAt(pAE->OwnerHouse->ArrayIndex, sight, this->SensorCell);
					this->SensorCell = currentCell;
					TechnoExt::AddSensorsAt(pAE->OwnerHouse->ArrayIndex, sight, this->SensorCell);
				}
			}

			if (pAE->Type->RevealSight != 0)
			{
				int sight = pAE->Type->RevealSight > 0 ? pAE->Type->RevealSight : pThis->GetTechnoType()->Sight;
				CoordStruct coords = pThis->GetCenterCoords();
				MapClass::Instance->RevealArea1(&coords, sight, pAE->OwnerHouse, CellStruct::Empty, 0, 0, 0, 1);
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
	const TechnoTypeExt::ExtData* pTypeExt = TypeExtData;

	if (pThis->WhatAmI() == AbstractType::Building
		&& pTypeExt->EMPulseCannon)
		return;

	bool disable = DisableTurnCount > 0;

	if (disable)
		--DisableTurnCount;

	TechnoTypeClass* pType = pThis->GetTechnoType();
	double dblROTMultiplier = 1.0;
	int iROTBuff = 0;

	for (const auto& pAE : this->GetActiveAE())
	{
		dblROTMultiplier *= pAE->Type->ROT_Multiplier;
		iROTBuff += pAE->Type->ROT;
	}

	int iROT_Primary = Game::F2I(pType->ROT * dblROTMultiplier) + iROTBuff;
	int iROT_Secondary = Game::F2I(TypeExtData->TurretROT.Get(pType->ROT) * dblROTMultiplier) + iROTBuff;
	iROT_Primary = std::max(iROT_Primary, 0);
	iROT_Secondary = std::max(iROT_Secondary, 0);
	pThis->PrimaryFacing.SetROT(iROT_Primary == 0 ? 1 : iROT_Primary);
	pThis->SecondaryFacing.SetROT(iROT_Secondary == 0 ? 1 : iROT_Secondary);

	if (FacingInitialized && (iROT_Primary == 0 || disable))
	{
		pThis->PrimaryFacing.SetCurrent(LastSelfFacing);
		pThis->PrimaryFacing.SetDesired(LastSelfFacing);
	}

	if (FacingInitialized && (iROT_Secondary == 0 || disable))
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

double TechnoExt::ExtData::GetAERangeMul(int* adden) const
{
	int iRangeBuff = 0;
	double dblMultiplier = 1.0;

	const std::vector<AttachEffectClass*> attachEffects = this->GetActiveAE();

	for (const auto& pAE : attachEffects)
	{
		dblMultiplier *= pAE->Type->Range_Multiplier;
		iRangeBuff += pAE->Type->Range;
	}

	if (adden != nullptr)
		*adden = iRangeBuff;

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
