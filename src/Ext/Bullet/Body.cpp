#include "Body.h"

#include <random>

#include <Helpers/Macro.h>

#include <Ext/RadSite/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Ext/WeaponType/Body.h>

#include <New/Entity/LaserTrailClass.h>
#include <New/Type/RadTypeClass.h>
#include <New/Armor/Armor.h>

#include <Misc/DrawLaser.h>

#include <Utilities/EnumFunctions.h>
#include <Utilities/Helpers.Alex.h>
#include <Utilities/GeneralUtils.h>
#include <Utilities/TemplateDef.h>

BulletExt::ExtContainer BulletExt::ExtMap;

void BulletExt::ExtData::InterceptBullet(TechnoClass* pSource, WeaponTypeClass* pWeapon)
{
	if (!pSource || !pWeapon)
		return;

	auto pThis = this->OwnerObject();
	auto pTypeExt = this->TypeExtData;
	bool canAffect = false;
	bool isIntercepted = false;

	if (pTypeExt->Armor.isset())
	{
		// double versus = GeneralUtils::GetWarheadVersusArmor(pWeapon->Warhead, pTypeExt->Armor.Get());
		double versus = CustomArmor::GetVersus(pWeapon->Warhead, this->Armor);

		if (versus != 0.0)
		{
			canAffect = true;
			this->CurrentStrength -= static_cast<int>(pWeapon->Damage * versus * pSource->FirepowerMultiplier);

			if (this->CurrentStrength <= 0)
				isIntercepted = true;
		}
	}
	else
	{
		canAffect = true;
		isIntercepted = true;
	}

	if (canAffect)
	{
		auto const pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pSource->GetTechnoType());
		const InterceptorTypeClass* pInterceptorType = pTechnoTypeExt->InterceptorType.get();
		auto const pWeaponOverride = pInterceptorType->WeaponOverride.Get(pTypeExt->Interceptable_WeaponOverride.Get(nullptr));
		bool detonate = !pInterceptorType->DeleteOnIntercept.Get(pTypeExt->Interceptable_DeleteOnIntercept);

		this->DetonateOnInterception = detonate;

		this->Interfere = pInterceptorType->InterfereOnIntercept.Get(pTypeExt->Interceptable_InterfereOnIntercept);
		this->InterfereToSource = pInterceptorType->InterfereToSource;
		if (pInterceptorType->InterfereToSelf)
		{
			this->InterfereToSelf = pSource;
		}

		if (pWeaponOverride)
		{
			bool replaceType = pInterceptorType->WeaponReplaceProjectile;
			bool cumulative = pInterceptorType->WeaponCumulativeDamage;

			pThis->WeaponType = pWeaponOverride;
			pThis->Health = cumulative ? pThis->Health + pWeaponOverride->Damage : pWeaponOverride->Damage;
			pThis->WH = pWeaponOverride->Warhead;
			pThis->Bright = pWeaponOverride->Bright;

			if (replaceType && pWeaponOverride->Projectile != pThis->Type && pWeaponOverride->Projectile)
			{
				pThis->Speed = pWeaponOverride->Speed;
				pThis->Type = pWeaponOverride->Projectile;
				this->TypeExtData = BulletTypeExt::ExtMap.Find(pThis->Type);
				this->Armor = this->TypeExtData->Armor.Get();

				if (this->LaserTrails.size())
				{
					this->LaserTrails.clear();

					if (!pThis->Type->Inviso)
						this->InitializeLaserTrails();
				}
			}
		}

		if (isIntercepted && !pInterceptorType->KeepIntact)
			this->InterceptedStatus = InterceptedStatus::Intercepted;
	}
}


void BulletExt::ExtData::ApplyRadiationToCell(CellStruct Cell, int Spread, int RadLevel)
{
	auto const pThis = this->OwnerObject();

	auto const pWeapon = pThis->GetWeaponType();
	auto const pWeaponExt = WeaponTypeExt::ExtMap.Find(pWeapon);
	auto const pRadType = pWeaponExt->RadType;
	auto const pThisHouse = pThis->Owner ? pThis->Owner->Owner : this->FirerHouse;

	auto const it = std::find_if(RadSiteClass::Array->begin(), RadSiteClass::Array->end(),
			[=](auto const pSite)
		{
			auto const pRadExt = RadSiteExt::ExtMap.Find(pSite);

			if (pRadExt->Type != pRadType)
				return false;

			if (MapClass::Instance->TryGetCellAt(pSite->BaseCell) != MapClass::Instance->TryGetCellAt(Cell))
				return false;

			if (Spread != pSite->Spread)
				return false;

			if (pWeapon != pRadExt->Weapon)
				return false;

			if (pRadExt->RadInvoker && pThis->Owner)
				return pRadExt->RadInvoker == pThis->Owner;

			return true;
			});

	if (it != RadSiteClass::Array->end())
	{
		if ((*it)->GetRadLevel() + RadLevel >= pRadType->GetLevelMax())
		{
			RadLevel = pRadType->GetLevelMax() - (*it)->GetRadLevel();
		}

		auto const pRadExt = RadSiteExt::ExtMap.Find((*it));
		// Handle It
		pRadExt->Add(RadLevel);
		return;
	}

	RadSiteExt::CreateInstance(Cell, Spread, RadLevel, pWeaponExt, pThisHouse, pThis->Owner);
}

void BulletExt::ExtData::InitializeLaserTrails()
{
	if (this->LaserTrails.size())
		return;

	if (auto pTypeExt = this->TypeExtData)
	{
		auto pThis = this->OwnerObject();
		auto pOwner = pThis->Owner ? pThis->Owner->Owner : nullptr;

		for (auto const& idxTrail : pTypeExt->LaserTrail_Types)
		{
			if (auto const pLaserType = LaserTrailTypeClass::Array[idxTrail].get())
			{
				this->LaserTrails.push_back(
					std::make_unique<LaserTrailClass>(pLaserType, pOwner));
			}
		}
	}
}

//绘制电流激光
void BulletExt::DrawElectricLaser
(
	CoordStruct posFire,
	CoordStruct posEnd,
	int length,
	ColorStruct color,
	double amplitude,
	int duration,
	int thickness,
	bool isSupported
)
{
	int xValue = (posEnd.X - posFire.X) / length;
	int yValue = (posEnd.Y - posFire.Y) / length;
	int zValue = (posEnd.Z - posFire.Z) / length;

	CoordStruct coords = posFire;
	CoordStruct lastcoords;

	int displace = static_cast<int>(posFire.DistanceFrom(posEnd) / amplitude);
	int thin = static_cast<int>(displace / 2);

	for (int i = 1; i <= length; i++)
	{
		lastcoords = coords;
		coords.X += xValue;
		coords.Y += yValue;
		coords.Z += zValue;
		coords.X += ScenarioClass::Instance->Random(-thin, thin);
		coords.Y += ScenarioClass::Instance->Random(-thin, thin);

		if (i == length)
			coords = posEnd;

		CoordStruct centerpos
		{
			lastcoords.X + (xValue / 2) + ScenarioClass::Instance->Random(-thin,thin),
			lastcoords.Y + (yValue / 2) + ScenarioClass::Instance->Random(-thin,thin),
			lastcoords.Z + (zValue / 2),
		};

		LaserDrawClass* pLaser1 =
			GameCreate<LaserDrawClass>
			(
				lastcoords,
				centerpos,
				color,
				ColorStruct(),
				ColorStruct(),
				duration
			);

		pLaser1->IsHouseColor = true;
		pLaser1->Thickness = thickness;
		pLaser1->IsSupported = isSupported;

		LaserDrawClass* pLaser2 =
			GameCreate<LaserDrawClass>
			(
				centerpos,
				coords,
				color,
				ColorStruct(),
				ColorStruct(),
				duration
			);

		pLaser2->IsHouseColor = true;
		pLaser2->Thickness = thickness;
		pLaser2->IsSupported = isSupported;
	}
}

//电流激光判定
void BulletExt::DrawElectricLaserWeapon(BulletClass* pThis, WeaponTypeClass* pWeaponType)
{
	const auto pWeaponTypeExt = WeaponTypeExt::ExtMap.Find(pWeaponType);

	if (!pWeaponTypeExt->ElectricLaser.Get())
		return;

	CoordStruct coords = pThis->SourceCoords;
	CoordStruct targetcoords = pThis->TargetCoords;

	double percentage = double(pThis->SourceCoords.DistanceFrom(pThis->TargetCoords) / 256);
	int length = int(percentage + 1);
	auto dir = DirStruct((-1) * Math::atan2(double(pThis->SourceCoords.Y - pThis->TargetCoords.Y),
		double(pThis->SourceCoords.X - pThis->TargetCoords.X)));

	for (int i = 0; i < pWeaponTypeExt->ElectricLaser_Count; i++)
	{
		DrawLaser::AddElectric(coords, targetcoords, dir.GetFacing<32>(),
			pWeaponTypeExt->ElectricLaser_Length.Get(length),
			pWeaponTypeExt->ElectricLaser_Timer.Get(),
			pWeaponTypeExt->ElectricLaser_Color[i],
			pWeaponTypeExt->ElectricLaser_Amplitude[i],
			pWeaponTypeExt->ElectricLaser_Duration[i],
			pWeaponTypeExt->ElectricLaser_Thickness[i],
			pWeaponTypeExt->ElectricLaser_IsSupported[i]);
	}
}

CoordStruct BulletExt::CalculateInaccurate(const BulletTypeClass* pBulletType)
{
	if (pBulletType->Inaccurate)
	{
		auto const pTypeExt = BulletTypeExt::ExtMap.Find(pBulletType);

		int ballisticScatter = RulesClass::Instance()->BallisticScatter;
		int scatterMax = pTypeExt->BallisticScatter_Max.isset() ? (int)(pTypeExt->BallisticScatter_Max.Get()) : ballisticScatter;
		int scatterMin = pTypeExt->BallisticScatter_Min.isset() ? (int)(pTypeExt->BallisticScatter_Min.Get()) : (scatterMax / 2);

		double random = ScenarioClass::Instance()->Random.RandomRanged(scatterMin, scatterMax);
		double theta = ScenarioClass::Instance()->Random.RandomDouble() * Math::TwoPi;

		CoordStruct offset
		{
			static_cast<int>(random * Math::cos(theta)),
			static_cast<int>(random * Math::sin(theta)),
			0
		};
		return offset;
	}

	return CoordStruct::Empty;
}

void BulletExt::ExtData::Shrapnel()
{
	BulletClass* pBullet = this->OwnerObject();

	if (!TechnoExt::IsReallyAlive(pBullet->Owner))
		return;

	BulletTypeClass* pType = pBullet->Type;
	const auto pTypeExt = BulletTypeExt::ExtMap.Find(pType);

	if (pTypeExt->Shrapnel_Chance < 1.0)
	{
		double dice = ScenarioClass::Instance->Random.RandomDouble();

		if (pTypeExt->Shrapnel_Chance < dice)
			return;
	}

	if (pBullet->Target && pBullet->Target->WhatAmI() == AbstractType::Building)
	{
		if (!pTypeExt->Shrapnel_AffectsBuildings)
			return;
	}

	if (pBullet->Target && pBullet->Target->WhatAmI() == AbstractType::Cell)
	{
		if (!pTypeExt->Shrapnel_AffectsGround)
			return;
	}

	WeaponTypeClass* pWeapon = pType->ShrapnelWeapon;

	if (pWeapon == nullptr)
		return;

	int shrapnelCount = pType->ShrapnelCount;
	WarheadTypeClass* pWH = pBullet->WH;
	const auto pWHExt = WarheadTypeExt::ExtMap.Find(pWH);

	CoordStruct sourceCoords = pType->Inviso
		? pBullet->Target ? pBullet->Target->GetCoords() : pBullet->TargetCoords
		: pBullet->GetCoords();

	std::vector<TechnoClass*> technos;
	int nonzeroNumber = 0;

	if (pTypeExt->Shrapnel_PriorityVersus)
	{
		technos = Helpers::Alex::getCellSpreadItems
		(
			sourceCoords,
			pWeapon->Range / 256,
			pTypeExt->Shrapnel_IncludeAir,
			[pWH, pWHExt, pBullet](const TechnoClass* pTechno)
			{
				if (!TechnoExt::IsReallyAlive(pTechno))
					return true;

				if (pTechno->Owner->IsAlliedWith(pBullet->Owner) || pTechno == pBullet->Target)
					return true;

				const auto pTechnoExt = TechnoExt::ExtMap.Find(pTechno);

				return fabs(CustomArmor::GetVersus(pWHExt, pTechnoExt->GetArmorIdx(pWH))) < DBL_EPSILON;
			},
			[pWH, pWHExt](const TechnoClass* pTechno1, const TechnoClass* pTechno2)
			{
				const auto pTechnoExt1 = TechnoExt::ExtMap.Find(pTechno1);
				const auto pTechnoExt2 = TechnoExt::ExtMap.Find(pTechno2);
				double versus1 = CustomArmor::GetVersus(pWHExt, pTechnoExt1->GetArmorIdx(pWH));
				double versus2 = CustomArmor::GetVersus(pWHExt, pTechnoExt2->GetArmorIdx(pWH));

				return fabs(versus1) > fabs(versus2);
			}
		);

		nonzeroNumber = static_cast<int>(technos.size());
	}
	else
	{
		technos = Helpers::Alex::getCellSpreadItems
		(
			sourceCoords,
			pWeapon->Range / 256,
			pTypeExt->Shrapnel_IncludeAir,
			[&](const TechnoClass* pTechno)
			{
				if (!TechnoExt::IsReallyAlive(pTechno))
					return true;

				if (pTypeExt->Shrapnel_IgnoreZeroVersus)
				{
					if (pTechno->GetTechnoType()->Immune)
						return true;

					TechnoExt::ExtData* pTechnoExt = TechnoExt::ExtMap.Find(pTechno);
					double versus = CustomArmor::GetVersus(pWHExt, pTechnoExt->GetArmorIdx(pWH));

					if (fabs(versus) < DBL_EPSILON)
						return true;
				}

				return pTechno->Owner->IsAlliedWith(pBullet->Owner) || pTechno == pBullet->Target;
			}
		);

		const auto end_of_nonzero = std::remove_if(technos.begin(), technos.end(),
			[pWH, pWHExt](const TechnoClass* pTechno)
			{
				const auto pTechnoExt = TechnoExt::ExtMap.Find(pTechno);
				double versus = CustomArmor::GetVersus(pWHExt, pTechnoExt->GetArmorIdx(pWH));
				return fabs(versus) < DBL_EPSILON;
			}
		);

		nonzeroNumber = static_cast<int>(end_of_nonzero - technos.cbegin());
	}

	if (nonzeroNumber >= shrapnelCount)
	{
		for (int i = 0; i < shrapnelCount; i++)
		{
			TechnoExt::SimulatedFire(pBullet->Owner, pWeapon, sourceCoords, technos[i]);
		}
	}
	else
	{
		int size = pTypeExt->Shrapnel_IgnoreZeroVersus
			? nonzeroNumber
			: static_cast<int>(technos.size());

		for (int i = 0; i < size; i++)
		{
			TechnoExt::SimulatedFire(pBullet->Owner, pWeapon, sourceCoords, technos[i]);
		}

		if (!pTypeExt->Shrapnel_IgnoreCell)
		{
			std::vector<CellClass*> cells;

			for (int i = -pWeapon->Range / 256; i < pWeapon->Range / 256; i++)
			{
				CellStruct cell = CellClass::Coord2Cell(sourceCoords);
				cell.X += static_cast<short>(i);

				for (int j = -pWeapon->Range / 256; j < pWeapon->Range / 256; j++)
				{
					CellStruct targetCell = cell;
					targetCell.Y += static_cast<short>(j);
					CoordStruct coords = CellClass::Cell2Coord(cell);
					coords.Z = MapClass::Instance->GetCellFloorHeight(coords);

					if (coords.DistanceFrom(sourceCoords) > pWeapon->Range)
						continue;

					CellClass* pCell = MapClass::Instance->GetCellAt(targetCell);

					if (pCell != &MapClass::InvalidCell)
					{
						if (pCell->GetBuilding() != nullptr
							|| pCell->GetAircraft(false) != nullptr
							|| pCell->GetInfantry(false) != nullptr
							|| pCell->GetUnit(false) != nullptr)
							continue;

						cells.emplace_back(pCell);
					}
				}
			}

			GeneralUtils::Shuffle(cells);

			for (int i = 0; i < shrapnelCount - size && i < static_cast<int>(cells.size()); i++)
			{
				TechnoExt::SimulatedFire(pBullet->Owner, pWeapon, sourceCoords, cells[i]);
			}
		}
	}
}

// =============================
// load / save

template <typename T>
void BulletExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->TypeExtData)
		.Process(this->FirerHouse)
		.Process(this->CurrentStrength)
		.Process(this->IsInterceptor)
		.Process(this->InterceptedStatus)
		.Process(this->DetonateOnInterception)
		.Process(this->Interfere)
		.Process(this->Interfered)
		.Process(this->InterfereToSource)
		.Process(this->InterfereToSelf)
		.Process(this->SnappedToTarget)
		.Process(this->LaserTrails)
		.Process(this->ShouldDirectional)
		.Process(this->BulletDir)
		.Process(this->DetonateOnWay_Timer)
		.Process(this->Passenger)
		.Process(this->SendPassengerMove)
		.Process(this->SendPassengerMoveHouse)
		.Process(this->SendPassengerMoveHouse_IgnoreNeturalHouse)
		.Process(this->SendPassenger_Overlap)
		.Process(this->SendPassenger_Select)
		.Process(this->SendPassenger_UseParachute)
		.Process(this->TrackTarget)
		;

	this->Trajectory = PhobosTrajectory::ProcessFromStream(Stm, this->Trajectory);
}

void BulletExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<BulletClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void BulletExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<BulletClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

// =============================
// container

BulletExt::ExtContainer::ExtContainer() : Container("BulletClass") { }

BulletExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x4664BA, BulletClass_CTOR, 0x5)
{
	GET(BulletClass*, pItem, ESI);

	BulletExt::ExtMap.TryAllocate(pItem);

	return 0;
}

DEFINE_HOOK(0x4665E9, BulletClass_DTOR, 0xA)
{
	GET(BulletClass*, pItem, ESI);

	if (auto pTraj = BulletExt::ExtMap.Find(pItem)->Trajectory)
		DLLDelete(pTraj);

	BulletExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x46AFB0, BulletClass_SaveLoad_Prefix, 0x8)
DEFINE_HOOK(0x46AE70, BulletClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(BulletClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	BulletExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK_AGAIN(0x46AF97, BulletClass_Load_Suffix, 0x7)
DEFINE_HOOK(0x46AF9E, BulletClass_Load_Suffix, 0x7)
{
	BulletExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x46AFC4, BulletClass_Save_Suffix, 0x3)
{
	BulletExt::ExtMap.SaveStatic();
	return 0;
}
