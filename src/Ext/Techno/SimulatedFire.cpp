#include "Body.h"

#include <TemporalClass.h>

#include <Helpers/Macro.h>

#include <Ext/EBolt/EBoltExt.h>

#include <Misc/CaptureManager.h>
#include <Misc/DrawLaser.h>

#include <Utilities/Helpers.Alex.h>

inline bool ManagersFire(TechnoClass* pThis, const WeaponStruct& weaponStruct, AbstractClass* pTarget)
{
	WeaponTypeClass* pWeapon = weaponStruct.WeaponType;
	WarheadTypeClass* pWH = pWeapon->Warhead;
	const auto pWHExt = WarheadTypeExt::ExtMap.Find(pWH);

	if (pWH->MindControl)
	{
		if (pThis->CaptureManager != nullptr)
		{
			if (pWH->CellSpread > 0)
			{
				std::vector<TechnoClass*> items(std::move(Helpers::Alex::getCellSpreadItems(pTarget->GetCoords(), pWH->CellSpread, true)));
				for (auto pTechno : items)
				{
					if (CaptureManager::CanCapture(pThis->CaptureManager, pTechno))
					{
						bool Remove = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType())->MultiMindControl_ReleaseVictim;
						auto const pAnimType = pWHExt->MindControl_Anim.isset() ? pWHExt->MindControl_Anim : RulesClass::Instance->ControlledAnimationType;
						CaptureManager::CaptureUnit(pThis->CaptureManager, pTechno, Remove, pAnimType);
					}
				}
			}
			else
			{
				if (TechnoClass* pTechno = abstract_cast<TechnoClass*>(pTarget))
				{
					if (CaptureManager::CanCapture(pThis->CaptureManager, pTechno))
					{
						bool Remove = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType())->MultiMindControl_ReleaseVictim;
						auto const pAnimType = pWHExt->MindControl_Anim.isset() ? pWHExt->MindControl_Anim : RulesClass::Instance->ControlledAnimationType;
						CaptureManager::CaptureUnit(pThis->CaptureManager, pTechno, Remove, pAnimType);
					}
				}
			}
		}

		return true;
	}

	if (pWH->Temporal)
	{
		if (pThis->TemporalImUsing == nullptr)
			pThis->TemporalImUsing = GameCreate<TemporalClass>(pThis);

		if (TechnoClass* pTechno = abstract_cast<TechnoClass*>(pTarget))
		{
			pThis->TemporalImUsing->Fire(pTechno);
			if (pWHExt->Temporal_CellSpread)
				TechnoExt::SetTemporalTeam(pThis, pTechno, pWHExt);
		}

		return true;
	}

	if (pWeapon->Spawner)
	{
		if (pThis->SpawnManager != nullptr)
			pThis->SpawnManager->SetTarget(pTarget);

		return true;
	}

	if (pWH->Parasite || pWeapon->DrainWeapon)
		return true;

	return false;
}

namespace SimulatedFireState
{
	bool Processing = false;
	const WeaponStruct* ProcessingWeapon = nullptr;
};

inline void ProcessEffects(TechnoClass* pThis, const WeaponStruct& weaponStruct, AbstractClass* pTarget, int damage)
{
	WeaponTypeClass* pWeapon = weaponStruct.WeaponType;
	WarheadTypeClass* pWH = pWeapon->Warhead;
	WeaponTypeExt::ExtData* pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);
	CoordStruct sourceCoords = TechnoExt::GetFLHAbsoluteCoords(pThis, weaponStruct.FLH, pThis->HasTurret());
	CoordStruct targetCoords = pTarget->GetCenterCoords();
	SimulatedFireState::Processing = true;
	SimulatedFireState::ProcessingWeapon = &weaponStruct;

	if (pWeapon->IsLaser)
	{
		LaserDrawClass* pLaser = pThis->CreateLaser(static_cast<ObjectClass*>(pTarget), 0, pWeapon, CoordStruct::Empty);
		pLaser->Thickness = pWeaponExt->LaserThickness;

		// 一样的道理2333。
		if (pWeaponExt->IsTrackingLaser.Get())
			DrawLaser::AddTrackingLaser(pLaser, pLaser->Duration, pThis, pTarget, CoordStruct::Empty, pThis->GetTechnoType()->Turret);
	}

	if (pWeapon->DiskLaser)
	{
		DiskLaserClass* pLaser = GameCreate<DiskLaserClass>();
		pThis->DiskLaserTimer.Start(pWeapon->ROF);
		pLaser->Fire(pThis, pTarget, pWeapon, damage);
	}

	if (pWeapon->IsElectricBolt)
	{
		EBolt* pBolt = GameCreate<EBolt>();
		EBoltExt::ExtData* pBoltExt = EBoltExt::ExtMap.Find(pBolt);
		pBoltExt->SetWeapon(weaponStruct);
		pBolt->Owner = pThis;
		pBolt->Fire(sourceCoords, targetCoords, 0);
	}

	if (pWeapon->IsRadBeam)
	{
		RadBeamType beamType= pWeapon->IsRadEruption
			? RadBeamType::Eruption :
			(pWH->Temporal
				? RadBeamType::Temporal
				: RadBeamType::RadBeam);
		pThis->CreateBeam(pTarget, beamType);
	}

	//Wave....
	if (pWeapon->IsMagBeam)
	{
	}

	//railgun, particle....
	if (pWeapon->AttachedParticleSystem != nullptr)
	{
		GameCreate<ParticleSystemClass>(pWeapon->AttachedParticleSystem, sourceCoords, pTarget, pThis, targetCoords, pThis->Owner);
	}

	SimulatedFireState::Processing = false;
	SimulatedFireState::ProcessingWeapon = nullptr;
}

BulletClass* TechnoExt::SimulatedFire(TechnoClass* pThis, const WeaponStruct& weaponStruct, AbstractClass* pTarget)
{
	if (!IsReallyAlive(pThis))
		return nullptr;

	// TechnoClass* pStand = PhobosGlobal::Global()->GetGenericStand();
	TechnoClass* pStand = abstract_cast<TechnoClass*>(TechnoTypeClass::Array->GetItem(0)->CreateObject(pThis->Owner));
	WeaponTypeClass* pWeapon = weaponStruct.WeaponType;

	if (pWeapon == nullptr)
		return nullptr;

	if (ManagersFire(pThis, weaponStruct, pTarget))
		return nullptr;

	ExtData* pExt = TechnoExt::ExtMap.Find(pThis);
	int iDamageBuff;
	double dblFirePowerMultiplier = pThis->FirepowerMultiplier * pExt->GetAEFireMul(&iDamageBuff);
	HouseClass* pStandOriginOwner = pStand->Owner;
	pStand->Owner = pThis->Owner;
	WeaponStruct& weaponCur = *pStand->GetWeapon(0);
	WeaponStruct weaponOrigin = *pStand->GetWeapon(0);
	bool bOmniFire = pWeapon->OmniFire;
	int iDamageOrigin = pWeapon->Damage;
	pWeapon->Damage = pWeapon->Damage >= 0 ?
		std::max(static_cast<int>(pWeapon->Damage * dblFirePowerMultiplier) + iDamageBuff, 0) :
		std::min(static_cast<int>(pWeapon->Damage * dblFirePowerMultiplier) + iDamageBuff, 0);
	pWeapon->OmniFire = true;
	weaponCur.WeaponType = weaponStruct.WeaponType;
	weaponCur.FLH = CoordStruct::Empty;
	CoordStruct absFLH = GetFLHAbsoluteCoords(pThis, weaponStruct.FLH, true);
	pStand->SetLocation(absFLH);
	BulletClass* pBullet = pStand->TechnoClass::Fire(pTarget, 0);
	if (pWeapon->Anim.Count > 0)
	{
		if (pWeapon->Anim.Count >= 8)
		{
			auto anim = GameCreate<AnimClass>(pWeapon->Anim.GetItem(pThis->GetRealFacing().GetFacing<8>()), absFLH);
			anim->SetOwnerObject(pThis);
		}
		else
		{
			auto anim = GameCreate<AnimClass>(pWeapon->Anim.GetItem(0), absFLH);
			anim->SetOwnerObject(pThis);
		}
	}

	if (pBullet != nullptr)
		pBullet->Owner = pThis;

	weaponCur = weaponOrigin;
	pWeapon->Damage = iDamageOrigin;
	pWeapon->OmniFire = bOmniFire;
	pStand->Owner = pStandOriginOwner;

	pStand->SetOwningHouse(HouseClass::FindCivilianSide(), false);
	KillSelf(pStand, AutoDeathBehavior::Vanish);

	return pBullet;
}

BulletClass* TechnoExt::SimulatedFireWithoutStand(TechnoClass* pThis, const WeaponStruct& weaponStruct, AbstractClass* pTarget)
{
	if (!TechnoExt::IsReallyAlive(pThis))
		return nullptr;

	WeaponTypeClass* pWeapon = weaponStruct.WeaponType;

	if (pWeapon == nullptr)
		return nullptr;

	TechnoExt::ExtData* pExt = TechnoExt::ExtMap.Find(pThis);
	HouseClass* pHouse = pThis->GetOwningHouse();
	int damageBuff;
	double damageMultiplier = pThis->FirepowerMultiplier * pHouse->FirepowerMultiplier * pExt->GetAEFireMul(&damageBuff);
	int damage = Game::F2I(pWeapon->Damage * damageMultiplier + damageBuff);

	ProcessEffects(pThis, weaponStruct, pTarget, damage);

	if (ManagersFire(pThis, weaponStruct, pTarget))
		return nullptr;

	BulletTypeClass* pBulletType = pWeapon->Projectile;
	CoordStruct sourceCoords = TechnoExt::GetFLHAbsoluteCoords(pThis, weaponStruct.FLH, pThis->HasTurret());
	CoordStruct targetCoords = pTarget->GetCenterCoords();
	BulletClass* pBullet = pBulletType->CreateBullet(pTarget, pThis, damage, pWeapon->Warhead, pWeapon->Speed, pWeapon->Bright);
	pBullet->SetWeaponType(pWeapon);
	Vector3D<int> velocityInt(targetCoords - sourceCoords);
	BulletVelocity velocity(velocityInt.X, velocityInt.Y, velocityInt.Z);

	pBullet->MoveTo(pTarget->GetCenterCoords(), velocity);

	return pBullet;
}

DEFINE_HOOK(0x6FD2DB, TechnoClass_CreateLaser_FLH, 0x8)
{
	GET(TechnoClass*, pThis, ESI);
	REF_STACK(CoordStruct, buffer, STACK_OFFSET(0x58, -0x24));

	if (!SimulatedFireState::Processing)
		return 0;

	const WeaponStruct* pWeapon = SimulatedFireState::ProcessingWeapon;

	buffer = TechnoExt::GetFLHAbsoluteCoords(pThis, pWeapon->FLH, pThis->HasTurret());

	R->EAX(&buffer);

	return 0x6FD2EF;
}

DEFINE_HOOK(0x6FD65D, TechnoClass_CreateRadBeam_FLH, 0x6)
{
	if (!SimulatedFireState::Processing)
		return 0;

	GET(TechnoClass*, pThis, EDI);
	REF_STACK(CoordStruct, buffer, STACK_OFFSET(0x4C, -0x30));

	const WeaponStruct* pWeapon = SimulatedFireState::ProcessingWeapon;

	buffer = TechnoExt::GetFLHAbsoluteCoords(pThis, pWeapon->FLH, pThis->HasTurret());

	R->EAX(&buffer);

	return 0x6FD2EF;
}

DEFINE_HOOK(0x6FD7E5, TechnoClass_CreateRadBeam, 0x6)
{
	if (!SimulatedFireState::Processing)
		return 0;

	GET(RadBeam*, pRad, ESI);
	GET(TechnoClass*, pThis, EDI);

	WeaponTypeExt::ExtData* pWeaponTypeExt = WeaponTypeExt::ExtMap.Find(SimulatedFireState::ProcessingWeapon->WeaponType);

	if (pWeaponTypeExt->Beam_IsHouseColor)
		pRad->SetColor(pThis->Owner->Color);
	else if (pWeaponTypeExt->Beam_Color.isset())
		pRad->SetColor(pWeaponTypeExt->Beam_Color);

	if (pWeaponTypeExt->Beam_Duration.isset())
		pRad->Period = pWeaponTypeExt->Beam_Duration;

	pRad->Amplitude = pWeaponTypeExt->Beam_Amplitude;

	return 0;
}
