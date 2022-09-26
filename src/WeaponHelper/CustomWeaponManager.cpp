#include "CustomWeaponManager.h"

#include "RadialFireHelper.h"
#include "SimulatedFireHelper.h"

void CustomWeaponManager::Update(TechnoClass* pOwner)
{
	if (!TechnoExt::IsReallyAlive(pOwner) || pOwner->Target == nullptr)
	{
		while (!SimulateBurstQueue.empty())
			SimulateBurstQueue.pop();

		return;
	}

	for (size_t i = 0; i < SimulateBurstQueue.size(); i++)
	{
		SimulateBurst burst = std::move(SimulateBurstQueue.front());
		SimulateBurstQueue.pop();
		// 检查是否还需要发射
		if (burst.Index < burst.Burst)
		{
			// 检查延迟
			if (burst.CanFire())
			{
				TechnoClass* pAttacker = burst.Owner;
				AbstractClass* pTarget = burst.Target;
				ObjectClass* pTargetObject = abstract_cast<ObjectClass*>(pTarget);
				TechnoClass* pTargetTechno = abstract_cast<TechnoClass*>(pTarget);
				WeaponTypeClass* pWeapon = burst.Weapon;

				if (pWeapon != nullptr
					&& pAttacker != nullptr && TechnoExt::IsReallyAlive(pAttacker)
					&& pTarget != nullptr && (pTargetTechno == nullptr || TechnoExt::IsReallyAlive(pTargetTechno)) && (pTargetObject == nullptr || !pTargetObject->InLimbo)
					&& (!burst.FireData.CheckRange || pAttacker->InRange(pTarget, pWeapon))
					&& (pAttacker->Transporter == nullptr || (pWeapon->FireInTransport || burst.FireData.OnlyFireInTransport)))
				{
					// 发射
					SimulateBurstFire(pAttacker, pOwner, pTarget, pWeapon, burst);
				}
				else
				{
					// 武器失效
					continue;
				}
			}
			// 归队
			SimulateBurstQueue.emplace(std::move(burst));
		}
	}
}

bool CustomWeaponManager::FireCustomWeapon
(
	TechnoClass* pAttacker,
	TechnoClass* pOwner,
	AbstractClass* pTarget,
	WeaponTypeClass* pWeapon,
	const CoordStruct& flh,
	const CoordStruct& bulletSourcePos,
	double rofMult
)
{
	bool isFire = false;
	pAttacker = WhoIsAttacker(pAttacker);

	if (!TechnoExt::IsReallyAlive(pAttacker) || pWeapon == nullptr)
		return;

	auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);
	TechnoClass* pTransporter = pAttacker->Transporter;
	TechnoTypeClass* pAttackerType = pAttacker->GetTechnoType();
	int burst = pWeapon->Burst;
	int minRange = pWeapon->MinimumRange;
	int range = pWeapon->Range;

	if (pTarget->IsInAir())
		range += pAttackerType->AirRangeBonus;

	if (burst > 1)
	{
		int flipY = 1;
		AttachFireData fireData;
		BulletTypeClass* pBulletType = pWeapon->Projectile;
		SimulateBurst newBurst(pWeapon, pAttacker, pTarget, flh, burst, minRange, range, fireData, flipY);
		SimulateBurstFire(pAttacker, pOwner, pTarget, pWeapon, newBurst);
		SimulateBurstQueue.emplace(std::move(newBurst));
		isFire = true;
	}
	else
	{
		SimulatedFireHelper::FireWeaponTo(pAttacker, pOwner, pTarget, pWeapon, flh, bulletSourcePos);
		isFire = true;
	}

	return isFire;
}

void CustomWeaponManager::SimulateBurstFire
(
		TechnoClass* pAttacker,
		TechnoClass* pOwner,
		AbstractClass* pTarget,
		WeaponTypeClass* pWeapon,
		SimulateBurst& burst
)
{
	// 模拟Burst发射武器
	if (burst.FireData.SimulateBurstMode == 3)
	{
		// 模式3，双发
		SimulateBurst b2 = burst;
		b2.FlipY *= -1;
		SimulateBurstFireOnce(pAttacker, pOwner, pTarget, pWeapon, b2);
	}
	// 单发
	SimulateBurstFireOnce(pAttacker, pOwner, pTarget, pWeapon, burst);
}

void CustomWeaponManager::SimulateBurstFireOnce
(
	TechnoClass* pAttacker,
	TechnoClass* pOwner,
	AbstractClass* pTarget,
	WeaponTypeClass* pWeapon,
	SimulateBurst& burst)
{
	if (!TechnoExt::IsReallyAlive(pAttacker) || pWeapon == nullptr)
		return;

	// Pointer<TechnoClass> pShooter = WhoIsShooter(pShooter);
	CoordStruct crdSrc = TechnoExt::GetFLHAbsoluteCoords(pAttacker, burst.FLH, true);
	CoordStruct crdDest = pTarget->GetCoords();
	BulletVelocity bulletVelocity = BulletVelocity::Empty;

	// 扇形攻击
	if (burst.FireData.RadialFire)
	{
		RadialFireHelper radialFireHelper(pAttacker, burst.Burst, burst.FireData.RadialAngle);
		bulletVelocity = radialFireHelper.GetBulletVelocity(burst.Index);
	}
	else
	{
		bulletVelocity = SimulatedFireHelper::GetBulletVelocity(crdSrc, crdDest);
	}
	// 发射武器
	BulletClass* pBullet = SimulatedFireHelper::FireBulletTo(pAttacker, pOwner, pTarget, pWeapon, crdSrc, crdDest, bulletVelocity);
	burst.CountOne();
}
