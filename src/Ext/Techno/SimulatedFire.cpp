#include "Body.h"

#include <TemporalClass.h>

#include <Helpers/Macro.h>

#include <Ext/Bullet/Trajectories/ArcingTrajectory.h>
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
	const CoordStruct* TargetCoords = nullptr;
	const CoordStruct* SourceCoords = nullptr;
	CoordStruct Buffer;
};

inline void ProcessEffects
(
	TechnoClass* pThis,
	const WeaponStruct& weaponStruct,
	AbstractClass* pTarget,
	int damage,
	const CoordStruct& targetCoords,
	const CoordStruct* sourceCoordsOverride = nullptr
)
{
	WeaponTypeClass* pWeapon = weaponStruct.WeaponType;
	WarheadTypeClass* pWH = pWeapon->Warhead;
	WeaponTypeExt::ExtData* pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);
	CoordStruct sourceCoords;

	if (sourceCoordsOverride != nullptr)
		sourceCoords = *sourceCoordsOverride;
	else
		sourceCoords = TechnoExt::GetFLHAbsoluteCoords(pThis, weaponStruct.FLH, pThis->HasTurret());
	
	if (pWeapon->IsLaser)
	{
		LaserDrawClass* pLaser = pThis->CreateLaser(static_cast<ObjectClass*>(pTarget), 0, pWeapon, CoordStruct::Empty);
		pLaser->Thickness = pWeaponExt->LaserThickness;
		pLaser->Target = targetCoords;
		pLaser->Source = sourceCoords;

		// 一样的道理2333。
		if (pWeaponExt->IsTrackingLaser)
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

		if (sourceCoordsOverride == nullptr)
			pBolt->Owner = pThis;

		pBolt->Fire(sourceCoords, targetCoords, 0);
		pBolt->Point1 = sourceCoords;
		pBolt->Point2 = targetCoords;
	}

	if (pWeapon->IsRadBeam)
	{
		RadBeamType beamType= pWeapon->IsRadEruption
			? RadBeamType::Eruption :
			(pWH->Temporal
				? RadBeamType::Temporal
				: RadBeamType::RadBeam);
		RadBeam* pRad = pThis->CreateBeam(pTarget, beamType);
		pRad->TargetLocation = targetCoords;
		pRad->SourceLocation = sourceCoords;
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
}

BulletClass* TechnoExt::SimulatedFire(TechnoClass* pThis, const WeaponStruct& weaponStruct, AbstractClass* pTarget)
{
	return TechnoExt::SimulatedFireWithoutStand(pThis, weaponStruct, pTarget);

	/*
	if (!IsReallyAlive(pThis))
		return nullptr;

	// TechnoClass* pStand = PhobosGlobal::Global()->GetGenericStand();
	TechnoClass* pStand = abstract_cast<TechnoClass*>(TechnoTypeClass::Array->GetItem(0)->CreateObject(HouseClass::FindCivilianSide()));
	//TechnoClass * pStand = abstract_cast<TechnoClass*>(TechnoTypeClass::Array->GetItem(0)->CreateObject(pThis->Owner));
	WeaponTypeClass * pWeapon = weaponStruct.WeaponType;

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

	if (pBullet != nullptr)
		pBullet->Owner = pThis;

	weaponCur = weaponOrigin;
	pWeapon->Damage = iDamageOrigin;
	pWeapon->OmniFire = bOmniFire;
	pStand->Owner = pStandOriginOwner;

	pStand->Owner = HouseClass::FindCivilianSide();
	KillSelf(pStand, AutoDeathBehavior::Vanish);

	return pBullet;*/
}

BulletClass* TechnoExt::SimulatedFireWithoutStand(TechnoClass* pThis, const WeaponStruct& weaponStruct, AbstractClass* pTarget)
{
	SimulatedFireState::Processing = true;
	SimulatedFireState::ProcessingWeapon = &weaponStruct;

	if (!TechnoExt::IsReallyAlive(pThis))
	{
		SimulatedFireState::Processing = false;
		SimulatedFireState::ProcessingWeapon = nullptr;
		return nullptr;
	}

	WeaponTypeClass* pWeapon = weaponStruct.WeaponType;

	if (pWeapon == nullptr)
	{
		SimulatedFireState::Processing = false;
		SimulatedFireState::ProcessingWeapon = nullptr;
		return nullptr;
	}

	if (pWeapon->AreaFire)
	{
		CoordStruct targetCoords = pThis->GetCenterCoords();
		SimulatedFireState::TargetCoords = SimulatedFireState::SourceCoords = &targetCoords;
		WeaponTypeExt::DetonateAt(pWeapon, targetCoords, pThis, pThis->Owner);
		SimulatedFireState::Processing = false;
		SimulatedFireState::ProcessingWeapon = nullptr;
		SimulatedFireState::SourceCoords = nullptr;
		SimulatedFireState::TargetCoords = nullptr;
		return nullptr;
	}

	BulletTypeClass* pBulletType = pWeapon->Projectile;
	HouseClass* pHouse = pThis->GetOwningHouse();
	double damageMultiplier = pThis->FirepowerMultiplier * pHouse->FirepowerMultiplier;
	int damage = Game::F2I(pWeapon->Damage * damageMultiplier);

	CoordStruct sourceCoords = TechnoExt::GetFLHAbsoluteCoords(pThis, weaponStruct.FLH, pThis->HasTurret());
	CoordStruct targetCoords = pTarget->GetCenterCoords();
	CoordStruct inaccurateCoords = BulletExt::CalculateInaccurate(pBulletType);

	if (inaccurateCoords != CoordStruct::Empty)
		targetCoords += BulletExt::CalculateInaccurate(pBulletType);

	SimulatedFireState::TargetCoords = &targetCoords;

	ProcessEffects(pThis, weaponStruct, pTarget, damage, targetCoords);

	if (ManagersFire(pThis, weaponStruct, pTarget))
	{
		SimulatedFireState::Processing = false;
		SimulatedFireState::ProcessingWeapon = nullptr;
		SimulatedFireState::TargetCoords = nullptr;
		return nullptr;
	}

	AnimTypeClass* pFireAnimType = WeaponTypeExt::GetFireAnim(pWeapon, pThis);

	if (pFireAnimType != nullptr)
	{
		AnimClass* pFireAnim = GameCreate<AnimClass>(pFireAnimType, sourceCoords);
		pFireAnim->SetOwnerObject(pThis);
	}

	if (pWeapon->Report.Count > 0)
	{
		for (int idx : pWeapon->Report)
		{
			VocClass::PlayAt(idx, sourceCoords);
		}
	}

	BulletClass* pBullet = pBulletType->CreateBullet(pTarget, pThis, damage, pWeapon->Warhead, pWeapon->Speed, pWeapon->Bright);
	pBullet->SourceCoords = sourceCoords;
	pBullet->TargetCoords = targetCoords;
	pBullet->SetWeaponType(pWeapon);

	const auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);
	pBullet->Range = pWeaponExt->ProjectileRange.Get();

	if (pBulletType->Arcing)
	{
		ArcingTrajectory::CalculateVelocity(pBullet, 1.0, pBulletType->VeryHigh);
		pBullet->MoveTo(sourceCoords, pBullet->Velocity);
	}
	else
	{
		Vector3D<int> velocity(targetCoords - sourceCoords);
		pBullet->MoveTo(sourceCoords, velocity);
	}

	if (inaccurateCoords != CoordStruct::Empty)
	{
		BulletExt::ExtData* pBulletExt = BulletExt::ExtMap.Find(pBullet);
		pBulletExt->TrackTarget = false;
	}

	SimulatedFireState::Processing = false;
	SimulatedFireState::ProcessingWeapon = nullptr;
	SimulatedFireState::SourceCoords = nullptr;
	SimulatedFireState::TargetCoords = nullptr;

	return pBullet;
}

BulletClass* TechnoExt::SimulatedFire(TechnoClass* pThis, WeaponTypeClass* pWeapon, const CoordStruct& sourceCoords, AbstractClass* pTarget)
{
	WeaponStruct weapon(pWeapon);

	SimulatedFireState::Processing = true;
	SimulatedFireState::ProcessingWeapon = &weapon;
	SimulatedFireState::SourceCoords = &sourceCoords;

	if (pWeapon == nullptr)
	{
		SimulatedFireState::Processing = false;
		SimulatedFireState::ProcessingWeapon = nullptr;
		SimulatedFireState::SourceCoords = nullptr;
		return nullptr;
	}

	if (!TechnoExt::IsReallyAlive(pThis))
		return nullptr;

	BulletTypeClass* pBulletType = pWeapon->Projectile;
	HouseClass* pHouse = pThis->GetOwningHouse();
	double damageMultiplier = pThis->FirepowerMultiplier * pHouse->FirepowerMultiplier;
	int	damage = Game::F2I(pWeapon->Damage * damageMultiplier);
	CoordStruct targetCoords = pTarget->GetCenterCoords();
	CoordStruct inaccurateCoords = BulletExt::CalculateInaccurate(pBulletType);

	if (inaccurateCoords != CoordStruct::Empty)
		targetCoords += BulletExt::CalculateInaccurate(pBulletType);

	SimulatedFireState::TargetCoords = &targetCoords;

	ProcessEffects(pThis, weapon, pTarget, damage, targetCoords, &sourceCoords);

	if (ManagersFire(pThis, weapon, pTarget))
	{
		SimulatedFireState::Processing = false;
		SimulatedFireState::ProcessingWeapon = nullptr;
		SimulatedFireState::SourceCoords = nullptr;
		SimulatedFireState::TargetCoords = nullptr;
		return nullptr;
	}

	BulletClass* pBullet = pBulletType->CreateBullet(pTarget, pThis, damage, pWeapon->Warhead, pWeapon->Speed, pWeapon->Bright);
	pBullet->SourceCoords = sourceCoords;
	pBullet->TargetCoords = targetCoords;
	pBullet->SetWeaponType(pWeapon);

	if (pBulletType->Arcing)
	{
		ArcingTrajectory::CalculateVelocity(pBullet, 1.0, pBulletType->VeryHigh);
		pBullet->MoveTo(sourceCoords, pBullet->Velocity);
	}
	else
	{
		Vector3D<int> velocity(targetCoords - sourceCoords);
		pBullet->MoveTo(sourceCoords, velocity);
	}

	if (inaccurateCoords != CoordStruct::Empty)
	{
		BulletExt::ExtData* pBulletExt = BulletExt::ExtMap.Find(pBullet);
		pBulletExt->TrackTarget = false;
	}

	SimulatedFireState::Processing = false;
	SimulatedFireState::ProcessingWeapon = nullptr;
	SimulatedFireState::TargetCoords = nullptr;
	SimulatedFireState::SourceCoords = nullptr;

	return pBullet;
}

BulletClass* TechnoExt::SimulatedFire(TechnoClass* pThis, const WeaponStruct& weaponStruct, AbstractClass* pTarget, CoordStruct targetCoords)
{
	SimulatedFireState::Processing = true;
	SimulatedFireState::ProcessingWeapon = &weaponStruct;
	
	if (!TechnoExt::IsReallyAlive(pThis))
	{
		SimulatedFireState::Processing = false;
		SimulatedFireState::ProcessingWeapon = nullptr;
		return nullptr;
	}

	WeaponTypeClass* pWeapon = weaponStruct.WeaponType;

	if (pWeapon == nullptr)
	{
		SimulatedFireState::Processing = false;
		SimulatedFireState::ProcessingWeapon = nullptr;
		return nullptr;
	}

	if (pWeapon->AreaFire)
	{
		targetCoords = pThis->GetCenterCoords();
		SimulatedFireState::TargetCoords = SimulatedFireState::SourceCoords = &targetCoords;
		WeaponTypeExt::DetonateAt(pWeapon, targetCoords, pThis, pThis->Owner);
		SimulatedFireState::Processing = false;
		SimulatedFireState::ProcessingWeapon = nullptr;
		SimulatedFireState::SourceCoords = nullptr;
		SimulatedFireState::TargetCoords = nullptr;
		return nullptr;
	}

	BulletTypeClass* pBulletType = pWeapon->Projectile;
	HouseClass* pHouse = pThis->GetOwningHouse();
	double damageMultiplier = pThis->FirepowerMultiplier * pHouse->FirepowerMultiplier;
	int damage = Game::F2I(pWeapon->Damage * damageMultiplier);

	CoordStruct sourceCoords = TechnoExt::GetFLHAbsoluteCoords(pThis, weaponStruct.FLH, pThis->HasTurret());
	targetCoords += BulletExt::CalculateInaccurate(pBulletType);

	SimulatedFireState::TargetCoords = &targetCoords;

	ProcessEffects(pThis, weaponStruct, pTarget, damage, targetCoords);

	if (ManagersFire(pThis, weaponStruct, pTarget))
	{
		SimulatedFireState::Processing = false;
		SimulatedFireState::ProcessingWeapon = nullptr;
		SimulatedFireState::TargetCoords = nullptr;
		return nullptr;
	}

	AnimTypeClass* pFireAnimType = WeaponTypeExt::GetFireAnim(pWeapon, pThis);

	if (pFireAnimType != nullptr)
	{
		AnimClass* pFireAnim = GameCreate<AnimClass>(pFireAnimType, sourceCoords);
		pFireAnim->SetOwnerObject(pThis);
	}

	if (pWeapon->Report.Count > 0)
	{
		for (int idx : pWeapon->Report)
		{
			VocClass::PlayAt(idx, sourceCoords);
		}
	}

	BulletClass* pBullet = pBulletType->CreateBullet(pTarget, pThis, damage, pWeapon->Warhead, pWeapon->Speed, pWeapon->Bright);
	pBullet->SourceCoords = sourceCoords;
	pBullet->TargetCoords = targetCoords;
	pBullet->SetWeaponType(pWeapon);
	BulletExt::ExtData* pBulletExt = BulletExt::ExtMap.Find(pBullet);
	pBulletExt->TrackTarget = false;

	const auto pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);
	pBullet->Range = pWeaponExt->ProjectileRange.Get();

	if (pBulletType->Arcing)
	{
		ArcingTrajectory::CalculateVelocity(pBullet, 1.0, pBulletType->VeryHigh);
		pBullet->MoveTo(sourceCoords, pBullet->Velocity);
	}
	else
	{
		Vector3D<int> velocity(targetCoords - sourceCoords);
		pBullet->MoveTo(sourceCoords, velocity);
	}

	SimulatedFireState::Processing = false;
	SimulatedFireState::ProcessingWeapon = nullptr;
	SimulatedFireState::TargetCoords = nullptr;
	SimulatedFireState::SourceCoords = nullptr;

	return pBullet;
}

DEFINE_HOOK(0x46870A, BulletClass_MoveTo_TargetCoords, 0x8)
{
	if (!SimulatedFireState::Processing)
		return 0;

	SimulatedFireState::Buffer = *SimulatedFireState::TargetCoords;

	R->EAX(&SimulatedFireState::Buffer);

	return 0;
}

DEFINE_HOOK(0x6FD2EF, TechnoClass_CreateLaser_FLH, 0x6)
{	
	if (!SimulatedFireState::Processing)
		return 0;

	if (SimulatedFireState::SourceCoords != nullptr)
	{
		SimulatedFireState::Buffer = *SimulatedFireState::SourceCoords;
		R->EAX(&SimulatedFireState::Buffer);

		return 0;
	}

	GET(TechnoClass*, pThis, ESI);

	const WeaponStruct* pWeapon = SimulatedFireState::ProcessingWeapon;

	SimulatedFireState::Buffer = TechnoExt::GetFLHAbsoluteCoords(pThis, pWeapon->FLH, pThis->HasTurret());

	R->EAX(&SimulatedFireState::Buffer);

	return 0;
}

DEFINE_HOOK(0x6FD65D, TechnoClass_CreateRadBeam_FLH, 0x6)
{
	if (!SimulatedFireState::Processing)
		return 0;

	if (SimulatedFireState::SourceCoords != nullptr)
	{
		SimulatedFireState::Buffer = *SimulatedFireState::SourceCoords;
		R->EAX(&SimulatedFireState::Buffer);

		return 0;
	}

	GET(TechnoClass*, pThis, EDI);

	const WeaponStruct* pWeapon = SimulatedFireState::ProcessingWeapon;

	SimulatedFireState::Buffer = TechnoExt::GetFLHAbsoluteCoords(pThis, pWeapon->FLH, pThis->HasTurret());

	R->EAX(&SimulatedFireState::Buffer);

	return 0;
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
