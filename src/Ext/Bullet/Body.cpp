#include "Body.h"

#include <Ext/RadSite/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/TechnoType/Body.h>
#include <Ext/WarheadType/Body.h>
#include <Utilities/EnumFunctions.h>
#include <Misc/FlyingStrings.h>

template<> const DWORD Extension<BulletClass>::Canary = 0x2A2A2A2A;
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
		double versus = GeneralUtils::GetWarheadVersusArmor(pWeapon->Warhead, pTypeExt->Armor.Get());

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
	auto const& Instances = RadSiteExt::Array;
	auto const pWeapon = pThis->GetWeaponType();
	auto const pWeaponExt = WeaponTypeExt::ExtMap.FindOrAllocate(pWeapon);
	auto const pRadType = pWeaponExt->RadType;
	auto const pThisHouse = pThis->Owner ? pThis->Owner->Owner : this->FirerHouse;

	if (Instances.Count > 0)
	{
		auto const it = std::find_if(Instances.begin(), Instances.end(),
			[=](RadSiteExt::ExtData* const pSite) // Lambda
			{// find
				return pSite->Type == pRadType &&
					pSite->OwnerObject()->BaseCell == Cell &&
					Spread == pSite->OwnerObject()->Spread;
			});

		if (it == Instances.end())
		{
			RadSiteExt::CreateInstance(Cell, Spread, RadLevel, pWeaponExt, pThisHouse, pThis->Owner);
		}
		else
		{
			auto const pRadExt = *it;
			auto const pRadSite = pRadExt->OwnerObject();

			if (pRadSite->GetRadLevel() + RadLevel > pRadType->GetLevelMax())
			{
				RadLevel = pRadType->GetLevelMax() - pRadSite->GetRadLevel();
			}

			// Handle It
			RadSiteExt::Add(pRadSite, RadLevel);
		}
	}
	else
	{
		RadSiteExt::CreateInstance(Cell, Spread, RadLevel, pWeaponExt, pThisHouse, pThis->Owner);
	}
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
void BulletExt::DrawElectricLaser(CoordStruct PosFire, CoordStruct PosEnd, int Length, ColorStruct Color, float Amplitude, int Duration, int Thickness, bool IsSupported)
{
	auto Xvalue = (PosEnd.X - PosFire.X) / Length;
	auto Yvalue = (PosEnd.Y - PosFire.Y) / Length;
	auto Zvalue = (PosEnd.Z - PosFire.Z) / Length;

	CoordStruct coords = PosFire;
	CoordStruct lastcoords;

	auto displace = int(PosFire.DistanceFrom(PosEnd) / Amplitude);
	auto thin = int(displace / 2);

	for (int i = 1; i <= Length; i++)
	{
		lastcoords = coords;
		coords.X += Xvalue;
		coords.Y += Yvalue;
		coords.Z += Zvalue;
		coords.X += ScenarioClass::Instance->Random(-thin, thin);
		coords.Y += ScenarioClass::Instance->Random(-thin, thin);

		if (i == Length)
			coords = PosEnd;

		CoordStruct centerpos
		{
			lastcoords.X + (Xvalue / 2) + ScenarioClass::Instance->Random(-thin,thin),
			lastcoords.Y + (Yvalue / 2) + ScenarioClass::Instance->Random(-thin,thin),
			lastcoords.Z + (Zvalue / 2),
		};

		LaserDrawClass* pLaser = GameCreate<LaserDrawClass>(
				lastcoords, centerpos,
				Color, ColorStruct { 0,0,0 }, ColorStruct { 0,0,0 },
				Duration);

		pLaser->IsHouseColor = true;
		pLaser->Thickness = Thickness;
		pLaser->IsSupported = IsSupported;

		LaserDrawClass* pLaser2 = GameCreate<LaserDrawClass>(
				centerpos, coords,
				Color, ColorStruct { 0,0,0 }, ColorStruct { 0,0,0 },
				Duration);

		pLaser2->IsHouseColor = true;
		pLaser2->Thickness = Thickness;
		pLaser2->IsSupported = IsSupported;
	}
}

//电流激光判定
void BulletExt::DrawElectricLaserWeapon(BulletClass* pThis, WeaponTypeClass* pWeaponType)
{
	const auto pWeaponTypeExt = WeaponTypeExt::ExtMap.Find(pWeaponType);

	if (!pWeaponTypeExt->ElectricLaser.Get())
		return;

	int length = pWeaponTypeExt->ElectricLaser_Length.Get();
	int timer = pWeaponTypeExt->ElectricLaser_Timer.Get();

	CoordStruct coords = pThis->SourceCoords;
	CoordStruct targetcoords = pThis->TargetCoords;

	for (int i = 0; i < pWeaponTypeExt->ElectricLaser_Count; i++)
	{/*
		BulletExt::DrawElectricLaser(coords, targetcoords, length,
			pWeaponTypeExt->ElectricLaser_Color[i],
			pWeaponTypeExt->ElectricLaser_Amplitude[i],
			pWeaponTypeExt->ElectricLaser_Duration[i],
			pWeaponTypeExt->ElectricLaser_Thickness[i],
			pWeaponTypeExt->ElectricLaser_IsSupported[i]);
			*/
		FlyingStrings::GetElectric(coords, targetcoords, length, timer,
			pWeaponTypeExt->ElectricLaser_Color[i],
			pWeaponTypeExt->ElectricLaser_Amplitude[i],
			pWeaponTypeExt->ElectricLaser_Duration[i],
			pWeaponTypeExt->ElectricLaser_Thickness[i],
			pWeaponTypeExt->ElectricLaser_IsSupported[i]);
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
		.Process(this->SetDamageStrength)
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

	BulletExt::ExtMap.FindOrAllocate(pItem);

	return 0;
}

DEFINE_HOOK(0x4665E9, BulletClass_DTOR, 0xA)
{
	GET(BulletClass*, pItem, ESI);

	if (auto pTraj = BulletExt::ExtMap.Find(pItem)->Trajectory)
		GameDelete(pTraj);

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
