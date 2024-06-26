#include "Body.h"

#include <Conversions.h>

#include <New/Entity/LaserTrailClass.h>

#pragma region DETONATION

bool DetonationInDamageArea = true;

DEFINE_HOOK(0x46920B, BulletClass_Detonate, 0x6)
{
	GET(BulletClass* const, pBullet, ESI);

	auto const pBulletExt = pBullet ? BulletExt::ExtMap.Find(pBullet) : nullptr;
	auto const pWH = pBullet ? pBullet->WH : nullptr;

	if (auto const pWHExt = WarheadTypeExt::ExtMap.Find(pWH))
	{
		GET_BASE(const CoordStruct*, pCoords, 0x8);
		auto const pOwner = pBullet->Owner;
		auto const pHouse = pOwner ? pOwner->Owner : nullptr;
		auto const pDecidedHouse = pHouse ? pHouse : pBulletExt->FirerHouse;

		pWHExt->Detonate(pOwner, pDecidedHouse, pBulletExt, *pCoords);
	}

	DetonationInDamageArea = false;

	return 0;
}

DEFINE_HOOK(0x46A290, BulletClass_Detonate_Return, 0x5)
{
	DetonationInDamageArea = true;
	return 0;
}

DEFINE_HOOK(0x489286, MapClass_DamageArea, 0x6)
{
	if (DetonationInDamageArea)
	{
		// GET(const int, Damage, EDX);
		// GET_BASE(const bool, AffectsTiberium, 0x10);

		GET_BASE(const WarheadTypeClass*, pWH, 0x0C);

		if (auto const pWHExt = WarheadTypeExt::ExtMap.Find(pWH))
		{
			GET(const CoordStruct*, pCoords, ECX);
			GET_BASE(TechnoClass*, pOwner, 0x08);
			GET_BASE(HouseClass*, pHouse, 0x14);

			auto const pDecidedHouse = !pHouse && pOwner ? pOwner->Owner : pHouse;

			pWHExt->Detonate(pOwner, pDecidedHouse, nullptr, *pCoords);
		}
	}

	return 0;
}
#pragma endregion

DEFINE_HOOK(0x48A551, WarheadTypeClass_AnimList_SplashList, 0x6)
{
	GET(WarheadTypeClass* const, pThis, ESI);
	auto pWHExt = WarheadTypeExt::ExtMap.Find(pThis);

	if (pWHExt && !pWHExt->SplashList.empty())
	{
		int idx = 0;

		if (pWHExt->SplashList.size() > 1)
		{
			if (pWHExt->AnimList_PickByDirection)
			{
				auto highest = Conversions::Int2Highest(static_cast<int>(pWHExt->SplashList.size()));

				if (highest >= 3)
				{
					auto offset = 1u << (highest - 3);
					idx = Conversions::TranslateFixedPoint(16, highest, static_cast<WORD>(pWHExt->HitDir), offset);
				}
			}
			else
			{
				GET(int, nDamage, ECX);
				idx = pWHExt->SplashList_PickRandom ?
					ScenarioClass::Instance->Random.RandomRanged(0, pWHExt->SplashList.size() - 1) :
					std::min(pWHExt->SplashList.size() * 35 - 1, (size_t)nDamage) / 35;
			}
		}

		R->EAX(pWHExt->SplashList[idx]);
		return 0x48A5AD;
	}

	return 0;
}

DEFINE_HOOK(0x48A5B3, WarheadTypeClass_AnimList_CritAnim, 0x6)
{
	GET(WarheadTypeClass* const, pThis, ESI);
	auto pWHExt = WarheadTypeExt::ExtMap.Find(pThis);

	if (pWHExt)
	{
		GET(int, nDamage, ECX);

		auto SetCritAnim = [pThis, pWHExt, nDamage](int i)->AnimTypeClass*
		{
			double chance = int(pWHExt->Crit_Chance.size()) > i ? pWHExt->Crit_Chance[i] : 0.0;
			auto animList = int(pWHExt->Crit_AnimList.size()) > i ? &pWHExt->Crit_AnimList[i] : nullptr;
			bool animListRadnow = int(pWHExt->Crit_AnimList_PickRandom.size()) > i ? pWHExt->Crit_AnimList_PickRandom[i] : pWHExt->AnimList_PickRandom.Get();
			bool affectedTarget = int(pWHExt->Crit_AnimOnAffectedTargets.size()) > i ? pWHExt->Crit_AnimOnAffectedTargets[i] : false;
			bool pickByDirection = int(pWHExt->Crit_AnimList_PickByDirection.size()) > i ? pWHExt->Crit_AnimList_PickByDirection[i] : false;

			if (!(chance < pWHExt->RandomBuffer) && animList && !animList->empty() && !affectedTarget)
			{
				int idx = 0;

				if (animList->size() > 1)
				{
					if (pickByDirection)
					{
						auto highest = Conversions::Int2Highest(static_cast<int>(animList->size()));

						if (highest >= 3)
						{
							auto offset = 1u << (highest - 3);
							idx = Conversions::TranslateFixedPoint(16, highest, static_cast<WORD>(pWHExt->HitDir), offset);
						}
					}
					else
					{
						idx = pThis->EMEffect || animListRadnow ?
							ScenarioClass::Instance->Random.RandomRanged(0, animList->size() - 1) :
							std::min(animList->size() * 25 - 1, (size_t)nDamage) / 25;
					}
				}

				return animList->at(idx);
			}

			return nullptr;
		};

		if (!pWHExt->Crit_Chance.empty())
		{
			if (pWHExt->Crit_RandomPick)
			{
				int i = ScenarioClass::Instance->Random.RandomRanged(0, pWHExt->Crit_Chance.size() - 1);

				if (auto const pAnimType = SetCritAnim(i))
				{
					R->EAX(pAnimType);
					return 0x48A5AD;
				}
				else if (pWHExt->AnimList_PickByDirection)
				{
					int idx = 0;

					if (pThis->AnimList.Count > 1)
					{
						auto highest = Conversions::Int2Highest(pThis->AnimList.Count);

						if (highest >= 3)
						{
							auto offset = 1u << (highest - 3);
							idx = Conversions::TranslateFixedPoint(16, highest, static_cast<WORD>(pWHExt->HitDir), offset);
						}
					}

					R->EAX(pThis->AnimList.GetItemOrDefault(idx));
					return 0x48A5AD;
				}
			}
			else
			{
				for (size_t i = 0; i < pWHExt->Crit_Chance.size(); i++)
				{
					if (auto const pAnimType = SetCritAnim(i))
					{
						R->EAX(pAnimType);
						return 0x48A5AD;
					}
					else if (pWHExt->AnimList_PickByDirection)
					{
						int idx = 0;

						if (pThis->AnimList.Count > 1)
						{
							auto highest = Conversions::Int2Highest(pThis->AnimList.Count);

							if (highest >= 3)
							{
								auto offset = 1u << (highest - 3);
								idx = Conversions::TranslateFixedPoint(16, highest, static_cast<WORD>(pWHExt->HitDir), offset);
							}
						}

						R->EAX(pThis->AnimList.GetItemOrDefault(idx));

						return 0x48A5AD;
					}
				}
			}
		}
		else if (pWHExt->AnimList_PickByDirection)
		{
			int idx = 0;

			if (pThis->AnimList.Count > 1)
			{
				auto highest = Conversions::Int2Highest(pThis->AnimList.Count);

				if (highest >= 3)
				{
					auto offset = 1u << (highest - 3);
					idx = Conversions::TranslateFixedPoint(16, highest, static_cast<WORD>(pWHExt->HitDir), offset);
				}
			}

			R->EAX(pThis->AnimList.GetItemOrDefault(idx));

			return 0x48A5AD;
		}
	}

	return 0;
}

DEFINE_HOOK(0x48A5BD, WarheadTypeClass_AnimList_PickRandom, 0x6)
{
	GET(WarheadTypeClass* const, pThis, ESI);
	auto pWHExt = WarheadTypeExt::ExtMap.Find(pThis);

	return pWHExt && pWHExt->AnimList_PickRandom ? 0x48A5C7 : 0;
}

DEFINE_HOOK(0x4896EC, Explosion_Damage_DamageSelf, 0x6)
{
	enum { SkipCheck = 0x489702 };

	GET_BASE(WarheadTypeClass*, pWarhead, 0xC);

	if (auto const pWHExt = WarheadTypeExt::ExtMap.Find(pWarhead))
	{
		if (pWHExt->AllowDamageOnSelf)
			return SkipCheck;
	}

	return 0;
}

#pragma region Fix_WW_Strength_ReceiveDamage_C4Warhead_Misuses

// Suicide=yes behavior on WeaponTypes
DEFINE_HOOK(0x6FDDCA, TechnoClass_Fire_Suicide, 0xA)
{
	GET(TechnoClass* const, pThis, ESI);

	pThis->ReceiveDamage(&pThis->Health, 0, RulesClass::Instance->C4Warhead,
		nullptr, true, false, pThis->Owner);

	return 0x6FDE03;
}

// Kill the vxl unit when flipped over
DEFINE_HOOK(0x70BC6F, TechnoClass_UpdateRigidBodyKinematics_KillFlipped, 0xA)
{
	GET(TechnoClass* const, pThis, ESI);

	auto const pFlipper = pThis->DirectRockerLinkedUnit;
	pThis->ReceiveDamage(&pThis->Health, 0, RulesClass::Instance->C4Warhead,
		nullptr, true, false, pFlipper ? pFlipper->Owner : nullptr);

	return 0x70BCA4;
}

// TODO:
// 0x4425C0, BuildingClass_ReceiveDamage_MaybeKillRadioLinks, 0x6
// 0x501477, HouseClass_IHouse_AllToHunt_KillMCInsignificant, 0xA
// 0x7187D2, TeleportLocomotionClass_7187A0_IronCurtainFuckMeUp, 0x8
// 0x718B1E

#pragma endregion Fix_WW_Strength_ReceiveDamage_C4Warhead_Misuse
