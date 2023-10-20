#include "Body.h"


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

		int delay = i < vDelay.size() ? vDelay[i] : pAEType->Delay;
		auto AEnew = new AttachEffectClass(pAEType, pInvoker, pThis, duration, delay);
		AEnew->Source = pWHExt->OwnerObject();
		vAE.emplace_back(AEnew);
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

	if (pAEType->Decloak)
		pThis->Uncloak(false);

	vAE.emplace_back(new AttachEffectClass(pAEType, pInvoker, pThis, duration, pAEType->Delay));
}

double TechnoExt::ExtData::GetAEFireMul(int* adden) const
{
	double dblMultiplier = 1.0;
	int iDamageBuff = 0;

	for (const auto& pAE : this->AttachEffects)
	{
		if (!pAE->IsActive())
			continue;

		dblMultiplier *= pAE->Type->FirePower_Multiplier;
		iDamageBuff += pAE->Type->FirePower;
	}

	if (this->ParentAttachment
		&& this->ParentAttachment->GetType()->InheritStateEffects)
	{
		auto pParentExt = TechnoExt::ExtMap.Find(this->ParentAttachment->Parent);
		for (const auto& pAE : pParentExt->AttachEffects)
		{
			if (!pAE->IsActive())
				continue;

			dblMultiplier *= pAE->Type->FirePower_Multiplier;
			iDamageBuff += pAE->Type->FirePower;
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
					if (!pAE->IsActive())
						continue;

					dblMultiplier *= pAE->Type->FirePower_Multiplier;
					iDamageBuff += pAE->Type->FirePower;
				}
			}
		}
	}

	if (adden != nullptr)
		*adden = iDamageBuff;

	return dblMultiplier;
}

double TechnoExt::ExtData::GetAERangeMul(int* adden) const
{
	int iRangeBuff = 0;
	double dblMultiplier = 1.0;

	for (const auto& pAE : this->AttachEffects)
	{
		if (!pAE->IsActive())
			continue;

		iRangeBuff += pAE->Type->Range;
		dblMultiplier *= pAE->Type->Range_Multiplier;
	}

	if (this->ParentAttachment && this->ParentAttachment->GetType()->InheritStateEffects)
	{
		auto pParentExt = TechnoExt::ExtMap.Find(this->ParentAttachment->Parent);
		for (const auto& pAE : pParentExt->AttachEffects)
		{
			if (!pAE->IsActive())
				continue;

			iRangeBuff += pAE->Type->Range;
			dblMultiplier *= pAE->Type->Range_Multiplier;
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
					if (!pAE->IsActive())
						continue;

					iRangeBuff += pAE->Type->Range;
					dblMultiplier *= pAE->Type->Range_Multiplier;
				}
			}
		}
	}

	if (adden != nullptr)
		*adden = iRangeBuff;

	return dblMultiplier;
}

double TechnoExt::ExtData::GetAEROFMul(int* adden) const
{
	int iROFBuff = 0;
	double dblMultiplier = 1.0;

	for (const auto& pAE :this->AttachEffects)
	{
		if (!pAE->IsActive())
			continue;

		dblMultiplier *= pAE->Type->ROF_Multiplier;
		iROFBuff += pAE->Type->ROF;
	}

	if (this->ParentAttachment
		&& this->ParentAttachment->GetType()->InheritStateEffects)
	{
		auto pParentExt = TechnoExt::ExtMap.Find(this->ParentAttachment->Parent);

		for (const auto& pAE : pParentExt->AttachEffects)
		{
			if (!pAE->IsActive())
				continue;

			dblMultiplier *= pAE->Type->ROF_Multiplier;
			iROFBuff += pAE->Type->ROF;
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
					if (!pAE->IsActive())
						continue;

					dblMultiplier *= pAE->Type->ROF_Multiplier;
					iROFBuff += pAE->Type->ROF;
				}
			}
		}
	}
	
	if (adden != nullptr)
		*adden = iROFBuff;

	return dblMultiplier;
}

double TechnoExt::ExtData::GetAESpeedMul(int* adden) const
{
	int iSpeedBuff = 0;
	double dblMultiplier = 1.0;

	for (const auto& pAE : this->AttachEffects)
	{
		if (!pAE->IsActive())
		{
			continue;
		}

		dblMultiplier *= pAE->Type->Speed_Multiplier;
		iSpeedBuff += pAE->Type->Speed;
	}

	if (this->ParentAttachment
		&& this->ParentAttachment->GetType()->InheritStateEffects)
	{
		auto pParentExt = TechnoExt::ExtMap.Find(this->ParentAttachment->Parent);
		for (const auto& pAE : pParentExt->AttachEffects)
		{
			if (!pAE->IsActive())
				continue;

			dblMultiplier *= pAE->Type->Speed_Multiplier;
			iSpeedBuff += pAE->Type->Speed;
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
					if (!pAE->IsActive())
						continue;

					dblMultiplier *= pAE->Type->Speed_Multiplier;
					iSpeedBuff += pAE->Type->Speed;
				}
			}
		}
	}

	if (adden != nullptr)
		*adden = iSpeedBuff;

	return dblMultiplier;
}

double TechnoExt::ExtData::GetAEArmorMul(int* adden) const
{
	int iArmorBuff = 0;
	double dblMultiplier = 1.0;

	for (auto& pAE : this->AttachEffects)
	{
		if (!pAE->IsActive())
			continue;

		iArmorBuff += pAE->Type->Armor;
		dblMultiplier *= pAE->Type->Armor_Multiplier;
	}

	if (this->ParentAttachment
		&& this->ParentAttachment->GetType()->InheritStateEffects)
	{
		auto pParentExt = TechnoExt::ExtMap.Find(this->ParentAttachment->Parent);
		for (const auto& pAE : pParentExt->AttachEffects)
		{
			if (!pAE->IsActive())
				continue;

			dblMultiplier *= pAE->Type->Armor_Multiplier;
			iArmorBuff += pAE->Type->Armor;
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
					if (!pAE->IsActive())
						continue;

					dblMultiplier *= pAE->Type->Armor_Multiplier;
					iArmorBuff += pAE->Type->Armor;
				}
			}
		}
	}

	if (adden != nullptr)
		*adden = iArmorBuff;

	return dblMultiplier;
}
