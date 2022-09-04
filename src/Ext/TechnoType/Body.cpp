#include "Body.h"

#include <TechnoTypeClass.h>
#include <StringTable.h>
#include <DriveLocomotionClass.h>

#include <Utilities/GeneralUtils.h>

#include <Ext/WeaponType/Body.h>
#include <Ext/BuildingType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Techno/Body.h>
#include <New/Type/TemperatureTypeClass.h>

template<> const DWORD Extension<TechnoTypeClass>::Canary = 0x11111111;
TechnoTypeExt::ExtContainer TechnoTypeExt::ExtMap;

void TechnoTypeExt::ExtData::Initialize()
{
	this->ShieldType = ShieldTypeClass::FindOrAllocate(NONE_STR);
}

void TechnoTypeExt::ExtData::ApplyTurretOffset(Matrix3D* mtx, double factor)
{
	float x = static_cast<float>(this->TurretOffset.GetEx()->X * factor);
	float y = static_cast<float>(this->TurretOffset.GetEx()->Y * factor);
	float z = static_cast<float>(this->TurretOffset.GetEx()->Z * factor);

	mtx->Translate(x, y, z);
}

void TechnoTypeExt::ApplyTurretOffset(TechnoTypeClass* pType, Matrix3D* mtx, double factor)
{
	if (auto ext = TechnoTypeExt::ExtMap.Find(pType))
		ext->ApplyTurretOffset(mtx, factor);
}

// Ares 0.A source
const char* TechnoTypeExt::ExtData::GetSelectionGroupID() const
{
	return GeneralUtils::IsValidString(this->GroupAs) ? this->GroupAs : this->OwnerObject()->ID;
}

const char* TechnoTypeExt::GetSelectionGroupID(ObjectTypeClass* pType)
{
	if (auto pExt = TechnoTypeExt::ExtMap.Find(static_cast<TechnoTypeClass*>(pType)))
		return pExt->GetSelectionGroupID();

	return pType->ID;
}

bool TechnoTypeExt::HasSelectionGroupID(ObjectTypeClass* pType, const char* pID)
{
	auto id = TechnoTypeExt::GetSelectionGroupID(pType);

	return (_strcmpi(id, pID) == 0);
}

bool TechnoTypeExt::ExtData::IsCountedAsHarvester()
{
	auto pThis = this->OwnerObject();
	UnitTypeClass* pUnit = nullptr;

	if (pThis->WhatAmI() == AbstractType::UnitType)
		pUnit = abstract_cast<UnitTypeClass*>(pThis);

	if (this->Harvester_Counted.Get(pThis->Enslaves || pUnit && (pUnit->Harvester || pUnit->Enslaves)))
		return true;

	return false;
}

void TechnoTypeExt::ExtData::ReadWeapons(CCINIClass* const pINI)
{
	TechnoTypeClass* pType = OwnerObject();
	const char* pSection = pType->ID;
	const char* pArtSection = pType->ImageFile;
	char key[0x40] = { '\0' };
	INI_EX exINI(pINI);
	INI_EX exArtINI(CCINIClass::INI_Art);

	if (pType->IsGattling || pType->Gunner || pType->IsChargeTurret)
	{
		for (int i = 0; i < pType->WeaponCount; i++)
		{
			Nullable<WeaponTypeClass*> weapon;
			Nullable<WeaponTypeClass*> veteran;
			Nullable<WeaponTypeClass*> elite;
			Valueable<CoordStruct> baseFLH;
			Nullable<CoordStruct> veteranFLH;
			Nullable<CoordStruct> eliteFLH;
			Valueable<int> barrelLength;
			Nullable<int> veteranBarrelLength;
			Nullable<int> eliteBarrelLength;
			Valueable<int> barrelThickness;
			Nullable<int> veteranBarrelThickness;
			Nullable<int> eliteBarrelThickness;
			Valueable<bool> turretLocked;
			Nullable<bool> veteranTurretLocked;
			Nullable<bool> eliteTurretLocked;
			int idx = i + 1;

			sprintf_s(key, "Weapon%d", idx);
			weapon.Read(exINI, pSection, key, true);
			sprintf_s(key, "VeteranWeapon%d", idx);
			veteran.Read(exINI, pSection, key, true);
			sprintf_s(key, "EliteWeapon%d", idx);
			elite.Read(exINI, pSection, key, true);
			sprintf_s(key, "Weapon%dFLH", idx);
			baseFLH.Read(exArtINI, pArtSection, key);
			sprintf_s(key, "VeteranWeapon%dFLH", idx);
			veteranFLH.Read(exArtINI, pArtSection, key);
			sprintf_s(key, "EliteWeapon%dFLH", idx);
			eliteFLH.Read(exArtINI, pArtSection, key);
			sprintf_s(key, "Weapon%dBarrelLength", idx);
			barrelLength.Read(exArtINI, pArtSection, key);
			sprintf_s(key, "VeteranWeapon%dBarrelLength", idx);
			veteranBarrelLength.Read(exArtINI, pArtSection, key);
			sprintf_s(key, "EliteWeapon%dBarrelLength", idx);
			eliteBarrelLength.Read(exArtINI, pArtSection, key);
			sprintf_s(key, "Weapon%dBarrelThickness", idx);
			barrelThickness.Read(exArtINI, pArtSection, key);
			sprintf_s(key, "VeteranWeapon%dBarrelThickness", idx);
			veteranBarrelThickness.Read(exArtINI, pArtSection, key);
			sprintf_s(key, "EliteWeapon%dBarrelThickness", idx);
			eliteBarrelThickness.Read(exArtINI, pArtSection, key);
			sprintf_s(key, "Weapon%dTurretLocked", idx);
			turretLocked.Read(exArtINI, pArtSection, key);
			sprintf_s(key, "VeteranWeapon%dTurretLocked", idx);
			veteranTurretLocked.Read(exArtINI, pArtSection, key);
			sprintf_s(key, "EliteWeapon%dTurretLocked", idx);
			eliteTurretLocked.Read(exArtINI, pArtSection, key);

			if (!weapon.isset())
				Debug::Log("INIParseFailed: [%s]: Weapon%d not found\n", pSection, idx);

			if (i < static_cast<int>(this->Weapons.Base.size()))
				this->Weapons.Base[i] = std::move(WeaponStruct(weapon, baseFLH, barrelLength, barrelThickness, turretLocked));
			else
				this->Weapons.Base.emplace_back(weapon, baseFLH, barrelLength, barrelThickness, turretLocked);

			if (veteran.isset()
				|| veteranFLH.isset()
				|| veteranBarrelLength.isset()
				|| veteranBarrelThickness.isset()
				|| veteranTurretLocked.isset())
			{
				this->Weapons.Veteran[i] = std::move
				(
					WeaponStruct
					(
						veteran.Get(weapon),
						veteranFLH.Get(baseFLH),
						veteranBarrelLength.Get(barrelLength),
						veteranBarrelThickness.Get(barrelThickness),
						veteranTurretLocked.Get(turretLocked)
					)
				);
			}

			if (elite.isset()
				|| eliteFLH.isset()
				|| eliteBarrelLength.isset()
				|| eliteBarrelThickness.isset()
				|| eliteTurretLocked.isset())
			{
				this->Weapons.Elite[i] = std::move
				(
					WeaponStruct
					(
						elite.Get(veteran.Get(weapon)),
						eliteFLH.Get(veteranFLH.Get(baseFLH)),
						eliteBarrelLength.Get(veteranBarrelLength.Get(barrelLength)),
						eliteBarrelThickness.Get(veteranBarrelThickness.Get(barrelThickness)),
						eliteTurretLocked.Get(veteranTurretLocked.Get(turretLocked))
					)
				);
			}
		}
	}
	else
	{
		Valueable<WeaponTypeClass*> primary;
		Nullable<WeaponTypeClass*> veteranPrimary;
		Nullable<WeaponTypeClass*> elitePrimary;
		Valueable<WeaponTypeClass*> secondary;
		Nullable<WeaponTypeClass*> veteranSecondary;
		Nullable<WeaponTypeClass*> eliteSecondary;
		Valueable<CoordStruct> primaryFLH;
		Nullable<CoordStruct> veteranPrimaryFLH;
		Nullable<CoordStruct> elitePrimaryFLH;
		Valueable<CoordStruct> secondaryFLH;
		Nullable<CoordStruct> veteranSecondaryFLH;
		Nullable<CoordStruct> eliteSecondaryFLH;
		Valueable<int> primaryBarrelLength;
		Nullable<int> veteranPrimaryBarrelLength;
		Nullable<int> elitePrimaryBarrelLength;
		Valueable<int> secondaryBarrelLength;
		Nullable<int> veteranSecondaryBarrelLength;
		Nullable<int> eliteSecondaryBarrelLength;
		Valueable<int> primaryBarrelThickness;
		Nullable<int> veteranPrimaryBarrelThickness;
		Nullable<int> elitePrimaryBarrelThickness;
		Valueable<int> secondaryBarrelThickness;
		Nullable<int> veteranSecondaryBarrelThickness;
		Nullable<int> eliteSecondaryBarrelThickness;

		primary.Read(exINI, pSection, "Primary", true);

		if (primary != nullptr)
		{
			veteranPrimary.Read(exINI, pSection, "VeteranPrimary", true);
			elitePrimary.Read(exINI, pSection, "ElitePrimary", true);
			primaryFLH.Read(exArtINI, pArtSection, "PrimaryFireFLH");
			veteranPrimaryFLH.Read(exArtINI, pArtSection, "VeteranPrimaryFireFLH");
			elitePrimaryFLH.Read(exArtINI, pArtSection, "ElitePrimaryFireFLH");
			primaryBarrelLength.Read(exArtINI, pArtSection, "PBarrelLength");
			veteranPrimaryBarrelLength.Read(exArtINI, pArtSection, "VeteranPBarrelLength");
			elitePrimaryBarrelLength.Read(exArtINI, pArtSection, "ElitePBarrelLength");
			primaryBarrelThickness.Read(exArtINI, pArtSection, "PBarrelThickness");
			veteranPrimaryBarrelThickness.Read(exArtINI, pArtSection, "VeteranPBarrelThickness");
			elitePrimaryBarrelThickness.Read(exArtINI, pArtSection, "ElitePBarrelThickness");
		}

		if (this->Weapons.Base.empty())
			this->Weapons.Base.emplace_back(primary, primaryFLH, primaryBarrelLength, primaryBarrelThickness, false);
		else
			this->Weapons.Base[0] = std::move(WeaponStruct(primary, primaryFLH, primaryBarrelLength, primaryBarrelThickness, false));

		if (veteranPrimary.isset()
			|| veteranPrimaryFLH.isset()
			|| veteranPrimaryBarrelLength.isset()
			|| veteranPrimaryBarrelThickness.isset())
		{
			this->Weapons.Veteran[0] = std::move
			(
				WeaponStruct
				(
					veteranPrimary.Get(primary),
					veteranPrimaryFLH.Get(primaryFLH),
					veteranPrimaryBarrelLength.Get(primaryBarrelLength),
					veteranPrimaryBarrelThickness.Get(primaryBarrelThickness),
					false
				)
			);
		}

		if (elitePrimary.isset()
			|| elitePrimaryFLH.isset()
			|| elitePrimaryBarrelLength.isset()
			|| elitePrimaryBarrelThickness.isset())
		{
			this->Weapons.Elite[0] = std::move
			(
				WeaponStruct
				(
					elitePrimary.Get(veteranPrimary.Get(primary)),
					elitePrimaryFLH.Get(veteranPrimaryFLH.Get(primaryFLH)),
					elitePrimaryBarrelLength.Get(veteranPrimaryBarrelLength.Get(primaryBarrelLength)),
					elitePrimaryBarrelThickness.Get(veteranPrimaryBarrelThickness.Get(primaryBarrelThickness)),
					false
				)
			);
		}

		secondary.Read(exINI, pSection, "Secondary", true);

		if (secondary != nullptr)
		{
			veteranSecondary.Read(exINI, pSection, "VeteranSecondary", true);
			eliteSecondary.Read(exINI, pSection, "EliteSecondary", true);
			secondaryFLH.Read(exArtINI, pArtSection, "SecondaryFireFLH");
			veteranSecondaryFLH.Read(exArtINI, pArtSection, "VeteranSecondaryFireFLH");
			eliteSecondaryFLH.Read(exArtINI, pArtSection, "EliteSecondaryFireFLH");
			secondaryBarrelLength.Read(exArtINI, pArtSection, "SBarrelLength");
			veteranSecondaryBarrelLength.Read(exArtINI, pArtSection, "VeteranSBarrelLength");
			eliteSecondaryBarrelLength.Read(exArtINI, pArtSection, "EliteSBarrelLength");
			secondaryBarrelThickness.Read(exArtINI, pArtSection, "SBarrelThickness");
			veteranSecondaryBarrelThickness.Read(exArtINI, pArtSection, "VeteranSBarrelThickness");
			eliteSecondaryBarrelThickness.Read(exArtINI, pArtSection, "EliteSBarrelThickness");
		}

		if (this->Weapons.Base.size() < 2U)
			this->Weapons.Base.emplace_back(secondary, secondaryFLH, secondaryBarrelLength, secondaryBarrelThickness, false);
		else
			this->Weapons.Base[1] = std::move(WeaponStruct(secondary, secondaryFLH, secondaryBarrelLength, secondaryBarrelThickness, false));

		if (veteranSecondary.isset()
			|| veteranSecondaryFLH.isset()
			|| veteranSecondaryBarrelLength.isset()
			|| veteranSecondaryBarrelThickness.isset())
		{
			this->Weapons.Veteran[1] = std::move
			(
				WeaponStruct
				(
					veteranSecondary.Get(secondary),
					veteranSecondaryFLH.Get(secondaryFLH),
					veteranSecondaryBarrelLength.Get(secondaryBarrelLength),
					veteranSecondaryBarrelThickness.Get(secondaryBarrelThickness),
					false
				)
			);
		}

		if (eliteSecondary.isset()
			|| eliteSecondaryFLH.isset()
			|| eliteSecondaryBarrelLength.isset()
			|| eliteSecondaryBarrelThickness.isset())
		{
			this->Weapons.Elite[1] = std::move
			(
				WeaponStruct
				(
					eliteSecondary.Get(veteranSecondary.Get(secondary)),
					eliteSecondaryFLH.Get(veteranSecondaryFLH.Get(secondaryFLH)),
					eliteSecondaryBarrelLength.Get(veteranSecondaryBarrelLength.Get(secondaryBarrelLength)),
					eliteSecondaryBarrelThickness.Get(veteranSecondaryBarrelThickness.Get(secondaryBarrelThickness)),
					false
				)
			);
		}
	}
}

void TechnoTypeExt::GetBurstFLHs(TechnoTypeClass* pThis, INI_EX& exArtINI, const char* pArtSection,
	std::vector<DynamicVectorClass<CoordStruct>>& nFLH, std::vector<DynamicVectorClass<CoordStruct>>& nVFlh, std::vector<DynamicVectorClass<CoordStruct>>& nEFlh, const char* pPrefixTag)
{
	char tempBuffer[32];
	char tempBufferFLH[48];

	bool parseMultiWeapons = pThis->TurretCount > 0 && pThis->WeaponCount > 0;
	auto weaponCount = parseMultiWeapons ? pThis->WeaponCount : 2;

	nFLH.resize(weaponCount);
	nVFlh.resize(weaponCount);
	nEFlh.resize(weaponCount);

	for (int i = 0; i < weaponCount; i++)
	{
		for (int j = 0; j < INT_MAX; j++)
		{
			_snprintf_s(tempBuffer, sizeof(tempBuffer), "%sWeapon%d", pPrefixTag, i + 1);
			auto prefix = parseMultiWeapons ? tempBuffer : i > 0 ? "%sSecondaryFire" : "%sPrimaryFire";
			_snprintf_s(tempBuffer, sizeof(tempBuffer), prefix, pPrefixTag);

			_snprintf_s(tempBufferFLH, sizeof(tempBufferFLH), "%sFLH.Burst%d", tempBuffer, j);
			Nullable<CoordStruct> FLH;
			FLH.Read(exArtINI, pArtSection, tempBufferFLH);

			_snprintf_s(tempBufferFLH, sizeof(tempBufferFLH), "Veteran%sFLH.Burst%d", tempBuffer, j);
			Nullable<CoordStruct> veteranFLH;
			veteranFLH.Read(exArtINI, pArtSection, tempBufferFLH);

			_snprintf_s(tempBufferFLH, sizeof(tempBufferFLH), "Elite%sFLH.Burst%d", tempBuffer, j);
			Nullable<CoordStruct> eliteFLH;
			eliteFLH.Read(exArtINI, pArtSection, tempBufferFLH);

			if (FLH.isset() && !veteranFLH.isset())
				veteranFLH = FLH;
			if (veteranFLH.isset() && !eliteFLH.isset())
				eliteFLH = veteranFLH;
			if (!FLH.isset() && !eliteFLH.isset() && !veteranFLH.isset())
				break;

			nFLH[i].AddItem(FLH.Get());
			nVFlh[i].AddItem(veteranFLH.Get());
			nEFlh[i].AddItem(eliteFLH.Get());
		}
	}
}

void TechnoTypeExt::GetWeaponFLHs(TechnoTypeClass* pThis, INI_EX& exArtINI, const char* pArtSection,
	std::vector<DynamicVectorClass<CoordStruct>>& nFLH, std::vector<DynamicVectorClass<CoordStruct>>& nVFlh, std::vector<DynamicVectorClass<CoordStruct>>& nEFlh)
{
	char tempBuffer[32];

	auto weaponCount = pThis->WeaponCount;
	nFLH.resize(weaponCount);
	nVFlh.resize(weaponCount);
	nEFlh.resize(weaponCount);

	for (int i = 0; i < weaponCount; i++)
	{
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "Weapon%dFLH", i + 1);
		Nullable<CoordStruct> FLH;
		FLH.Read(exArtINI, pArtSection, tempBuffer);

		_snprintf_s(tempBuffer, sizeof(tempBuffer), "VeteranWeapon%dFLH", i + 1);
		Nullable<CoordStruct> VeteranFLH;
		VeteranFLH.Read(exArtINI, pArtSection, tempBuffer);

		_snprintf_s(tempBuffer, sizeof(tempBuffer), "EliteWeapon%dFLH", i + 1);
		Nullable<CoordStruct> EliteFLH;
		EliteFLH.Read(exArtINI, pArtSection, tempBuffer);

		if (!VeteranFLH.isset())
			VeteranFLH = FLH;

		if (!EliteFLH.isset())
			EliteFLH = VeteranFLH;

		nFLH[i].AddItem(FLH.Get());
		nVFlh[i].AddItem(VeteranFLH.Get());
		nEFlh[i].AddItem(EliteFLH.Get());
	}
}

void TechnoTypeExt::GetWeaponStages(TechnoTypeClass* pThis, INI_EX& exINI, const char* pSection,
	std::vector<DynamicVectorClass<int>>& nStage, std::vector<DynamicVectorClass<int>>& nStageV, std::vector<DynamicVectorClass<int>>& nStageE)
{
	char tempBuffer[32];

	auto weaponstage = pThis->WeaponStages;
	nStage.resize(weaponstage);
	nStageV.resize(weaponstage);
	nStageE.resize(weaponstage);

	for (int i = 0; i < weaponstage; i++)
	{
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "Stage%d", i + 1);
		Nullable<int> Stage;
		Stage.Read(exINI, pSection, tempBuffer);

		_snprintf_s(tempBuffer, sizeof(tempBuffer), "VeteranStage%d", i + 1);
		Nullable<int> VeteranStage;
		VeteranStage.Read(exINI, pSection, tempBuffer);

		_snprintf_s(tempBuffer, sizeof(tempBuffer), "EliteStage%d", i + 1);
		Nullable<int> EliteStage;
		EliteStage.Read(exINI, pSection, tempBuffer);

		if (!VeteranStage.isset())
			VeteranStage = Stage;

		if (!EliteStage.isset())
			EliteStage = VeteranStage;

		nStage[i].AddItem(Stage.Get());
		nStageV[i].AddItem(VeteranStage.Get());
		nStageE[i].AddItem(EliteStage.Get());
	}
}

void TechnoTypeExt::GetWeaponCounts(TechnoTypeClass* pThis, INI_EX& exINI, const char* pSection,
	std::vector<DynamicVectorClass<WeaponTypeClass*>>& n, std::vector<DynamicVectorClass<WeaponTypeClass*>>& nV, std::vector<DynamicVectorClass<WeaponTypeClass*>>& nE)
{
	char tempBuffer[32];

	auto weaponCount = pThis->WeaponCount;
	n.resize(weaponCount);
	nV.resize(weaponCount);
	nE.resize(weaponCount);

	for (int i = 0; i < weaponCount; i++)
	{
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "Weapon%d", i + 1);
		Nullable<WeaponTypeClass*> Weapon;
		Weapon.Read(exINI, pSection, tempBuffer);

		_snprintf_s(tempBuffer, sizeof(tempBuffer), "VeteranWeapon%d", i + 1);
		Nullable<WeaponTypeClass*> VeteranWeapon;
		VeteranWeapon.Read(exINI, pSection, tempBuffer);

		_snprintf_s(tempBuffer, sizeof(tempBuffer), "EliteWeapon%d", i + 1);
		Nullable<WeaponTypeClass*> EliteWeapon;
		EliteWeapon.Read(exINI, pSection, tempBuffer);

		if (!VeteranWeapon.isset())
			VeteranWeapon = Weapon;

		if (!EliteWeapon.isset())
			EliteWeapon = VeteranWeapon;

		n[i].AddItem(Weapon.Get());
		nV[i].AddItem(VeteranWeapon.Get());
		nE[i].AddItem(EliteWeapon.Get());
	}
}

void TechnoTypeExt::GetIFVTurrets(TechnoTypeClass* pThis, INI_EX& exINI, const char* pSection, std::vector<DynamicVectorClass<int>>& nturret)
{
	char tempBuffer[32];

	auto weaponCount = pThis->WeaponCount;
	nturret.resize(weaponCount);

	for (int i = 0; i < weaponCount; i++)
	{
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "WeaponTurretIndex%d", i + 1);
		Nullable<int> Turret;
		Turret.Read(exINI, pSection, tempBuffer);

		if (!Turret.isset())
			Turret = 0;

		nturret[i].AddItem(Turret.Get());
	}
}

TechnoTypeClass* TechnoTypeExt::GetTechnoType(ObjectTypeClass* pType)
{
	if (pType->WhatAmI() == AbstractType::AircraftType || pType->WhatAmI() == AbstractType::BuildingType ||
		pType->WhatAmI() == AbstractType::InfantryType || pType->WhatAmI() == AbstractType::UnitType)
	{
		return static_cast<TechnoTypeClass*>(pType);
	}

	return nullptr;
}

std::vector<WeaponTypeClass*> TechnoTypeExt::GetAllWeapons(TechnoTypeClass* pThis)
{
	ExtData* pExt = ExtMap.Find(pThis);
	std::vector<WeaponTypeClass*> vWeapons;

	for (const auto& weapon : pExt->Weapons.Base)
	{
		if (weapon.WeaponType != nullptr)
			vWeapons.emplace_back(weapon.WeaponType);
	}

	for (const auto& item : pExt->Weapons.Veteran)
	{
		if (item.second.WeaponType != nullptr)
			vWeapons.emplace_back(item.second.WeaponType);
	}

	for (const auto& item : pExt->Weapons.Elite)
	{
		if (item.second.WeaponType != nullptr)
			vWeapons.emplace_back(item.second.WeaponType);
	}
	
	if (pThis->WhatAmI() == AbstractType::InfantryType && static_cast<InfantryTypeClass*>(pThis)->OccupyWeapon.WeaponType != nullptr)
		vWeapons.emplace_back(static_cast<InfantryTypeClass*>(pThis)->OccupyWeapon.WeaponType);

	if (pExt->VeteranOccupyWeapon.Get() != nullptr)
		vWeapons.emplace_back(pExt->VeteranOccupyWeapon);

	return vWeapons;
}

// =============================
// load / save

void TechnoTypeExt::ExtData::LoadFromINIFile(CCINIClass* const pINI)
{
	auto pThis = this->OwnerObject();
	const char* pSection = pThis->ID;

	if (!pINI->GetSection(pSection))
		return;

	char tempBuffer[32];
	INI_EX exINI(pINI);

	this->ReadWeapons(pINI);

	this->HealthBar_Hide.Read(exINI, pSection, "HealthBar.Hide");
	this->UIDescription.Read(exINI, pSection, "UIDescription");
	this->LowSelectionPriority.Read(exINI, pSection, "LowSelectionPriority");
	this->MindControlRangeLimit.Read(exINI, pSection, "MindControlRangeLimit");

	this->Interceptor.Read(exINI, pSection, "Interceptor");
	this->Interceptor_Rookie.Read(exINI, pSection, "Interceptor.Rookie");
	this->Interceptor_Veteran.Read(exINI, pSection, "Interceptor.Veteran");
	this->Interceptor_Elite.Read(exINI, pSection, "Interceptor.Elite");
	this->Interceptor_CanTargetHouses.Read(exINI, pSection, "Interceptor.CanTargetHouses");
	this->Interceptor_GuardRange.Read(exINI, pSection, "Interceptor.%sGuardRange");
	this->Interceptor_MinimumGuardRange.Read(exINI, pSection, "Interceptor.%sMinimumGuardRange");
	this->Interceptor_Weapon.Read(exINI, pSection, "Interceptor.Weapon");
	this->Interceptor_DeleteOnIntercept.Read(exINI, pSection, "Interceptor.DeleteOnIntercept");
	this->Interceptor_WeaponOverride.Read(exINI, pSection, "Interceptor.WeaponOverride");
	this->Interceptor_WeaponReplaceProjectile.Read(exINI, pSection, "Interceptor.WeaponReplaceProjectile");
	this->Interceptor_WeaponCumulativeDamage.Read(exINI, pSection, "Interceptor.WeaponCumulativeDamage");
	this->Interceptor_KeepIntact.Read(exINI, pSection, "Interceptor.KeepIntact");
	this->Interceptor_Success.Read(exINI, pSection, "Interceptor.Success");
	this->Interceptor_RookieSuccess.Read(exINI, pSection, "Interceptor.RookieSuccess");
	this->Interceptor_VeteranSuccess.Read(exINI, pSection, "Interceptor.VeteranSuccess");
	this->Interceptor_EliteSuccess.Read(exINI, pSection, "Interceptor.EliteSuccess");

	this->Powered_KillSpawns.Read(exINI, pSection, "Powered.KillSpawns");
	this->Spawner_SameLoseTarget.Read(exINI, pSection, "Spawner.SameLoseTarget");
	this->Spawner_LimitRange.Read(exINI, pSection, "Spawner.LimitRange");
	this->Spawner_ExtraLimitRange.Read(exINI, pSection, "Spawner.ExtraLimitRange");
	this->Spawner_DelayFrames.Read(exINI, pSection, "Spawner.DelayFrames");
	this->Spawner_DelayFrams_PerSpawn.Read(exINI, pSection, "Spawner.DelayFrams.PerSpawn");
	this->Spawn_Types.Read(exINI, pSection, "Spawn.Types");
	this->Spawn_Nums.Read(exINI, pSection, "Spawn.Nums");
	this->Spawn_RegenRate.Read(exINI, pSection, "Spawn.RegenRate");
	this->Spawn_ReloadRate.Read(exINI, pSection, "Spawn.ReloadRate");
	
	if (!Spawn_Types.empty())
	{
		while (Spawn_Nums.size() < Spawn_Types.size())
		{
			Spawn_Nums.emplace_back(1);
		}

		int sum = 0;

		for (int num : Spawn_Nums)
		{
			sum += num;
		}

		OwnerObject()->SpawnsNumber = sum;
	}

	this->Harvester_Counted.Read(exINI, pSection, "Harvester.Counted");
	if (!this->Harvester_Counted.isset() && pThis->Enslaves)
		this->Harvester_Counted = true;
	if (this->Harvester_Counted.Get())
	{
		auto& list = RulesExt::Global()->HarvesterTypes;
		if (!list.Contains(pThis))
			list.emplace_back(pThis);
	}
	this->Promote_IncludeSpawns.Read(exINI, pSection, "Promote.IncludeSpawns");
	this->ImmuneToCrit.Read(exINI, pSection, "ImmuneToCrit");
	this->MultiMindControl_ReleaseVictim.Read(exINI, pSection, "MultiMindControl.ReleaseVictim");
	this->NoManualMove.Read(exINI, pSection, "NoManualMove");
	this->InitialStrength.Read(exINI, pSection, "InitialStrength");

	this->AutoDeath_Behavior.Read(exINI, pSection, "AutoDeath.Behavior");
	this->AutoDeath_OnAmmoDepletion.Read(exINI, pSection, "AutoDeath.OnAmmoDepletion");
	this->AutoDeath_AfterDelay.Read(exINI, pSection, "AutoDeath.AfterDelay");
	this->AutoDeath_Nonexist.Read(exINI, pSection, "AutoDeath.Nonexist");
	this->AutoDeath_Nonexist_House.Read(exINI, pSection, "AutoDeath.Nonexist.House");
	this->AutoDeath_Exist.Read(exINI, pSection, "AutoDeath.Exist");
	this->AutoDeath_Exist_House.Read(exINI, pSection, "AutoDeath.Exist.House");

	this->Slaved_OwnerWhenMasterKilled.Read(exINI, pSection, "Slaved.OwnerWhenMasterKilled");
	this->SellSound.Read(exINI, pSection, "SellSound");

	this->ShieldType.Read(exINI, pSection, "ShieldType", true);
	this->CameoPriority.Read(exINI, pSection, "CameoPriority");

	this->WarpOut.Read(exINI, pSection, "WarpOut");
	this->WarpIn.Read(exINI, pSection, "WarpIn");
	this->WarpAway.Read(exINI, pSection, "WarpAway");
	this->ChronoTrigger.Read(exINI, pSection, "ChronoTrigger");
	this->ChronoDistanceFactor.Read(exINI, pSection, "ChronoDistanceFactor");
	this->ChronoMinimumDelay.Read(exINI, pSection, "ChronoMinimumDelay");
	this->ChronoRangeMinimum.Read(exINI, pSection, "ChronoRangeMinimum");
	this->ChronoDelay.Read(exINI, pSection, "ChronoDelay");

	this->WarpInWeapon.Read(exINI, pSection, "WarpInWeapon", true);
	this->WarpInMinRangeWeapon.Read(exINI, pSection, "WarpInMinRangeWeapon", true);
	this->WarpOutWeapon.Read(exINI, pSection, "WarpOutWeapon", true);
	this->WarpInWeapon_UseDistanceAsDamage.Read(exINI, pSection, "WarpInWeapon.UseDistanceAsDamage");

	this->OreGathering_Anims.Read(exINI, pSection, "OreGathering.Anims");
	this->OreGathering_Tiberiums.Read(exINI, pSection, "OreGathering.Tiberiums");
	this->OreGathering_FramesPerDir.Read(exINI, pSection, "OreGathering.FramesPerDir");

	this->DestroyAnim_Random.Read(exINI, pSection, "DestroyAnim.Random");
	this->NotHuman_RandomDeathSequence.Read(exINI, pSection, "NotHuman.RandomDeathSequence");

	this->PassengerDeletion_Rate.Read(exINI, pSection, "PassengerDeletion.Rate");
	this->PassengerDeletion_Rate_SizeMultiply.Read(exINI, pSection, "PassengerDeletion.Rate.SizeMultiply");
	this->PassengerDeletion_UseCostAsRate.Read(exINI, pSection, "PassengerDeletion.UseCostAsRate");
	this->PassengerDeletion_CostMultiplier.Read(exINI, pSection, "PassengerDeletion.CostMultiplier");
	this->PassengerDeletion_Soylent.Read(exINI, pSection, "PassengerDeletion.Soylent");
	this->PassengerDeletion_SoylentMultiplier.Read(exINI, pSection, "PassengerDeletion.SoylentMultiplier");
	this->PassengerDeletion_SoylentFriendlies.Read(exINI, pSection, "PassengerDeletion.SoylentFriendlies");
	this->PassengerDeletion_DisplaySoylent.Read(exINI, pSection, "PassengerDeletion.DisplaySoylent");
	this->PassengerDeletion_DisplaySoylentToHouses.Read(exINI, pSection, "PassengerDeletion.DisplaySoylentToHouses");
	this->PassengerDeletion_DisplaySoylentOffset.Read(exINI, pSection, "PassengerDeletion.DisplaySoylentOffset");
	this->PassengerDeletion_ReportSound.Read(exINI, pSection, "PassengerDeletion.ReportSound");
	this->PassengerDeletion_Anim.Read(exINI, pSection, "PassengerDeletion.Anim");

	this->DefaultDisguise.Read(exINI, pSection, "DefaultDisguise");
	this->UseDisguiseMovementSpeed.Read(exINI, pSection, "UseDisguiseMovementSpeed");

	this->OpenTopped_RangeBonus.Read(exINI, pSection, "OpenTopped.RangeBonus");
	this->OpenTopped_DamageMultiplier.Read(exINI, pSection, "OpenTopped.DamageMultiplier");
	this->OpenTopped_WarpDistance.Read(exINI, pSection, "OpenTopped.WarpDistance");
	this->OpenTopped_IgnoreRangefinding.Read(exINI, pSection, "OpenTopped.IgnoreRangefinding");
	this->OpenTopped_AllowFiringIfDeactivated.Read(exINI, pSection, "OpenTopped.AllowFiringIfDeactivated");

	this->AutoFire.Read(exINI, pSection, "AutoFire");
	this->AutoFire_TargetSelf.Read(exINI, pSection, "AutoFire.TargetSelf");

	this->NoSecondaryWeaponFallback.Read(exINI, pSection, "NoSecondaryWeaponFallback");

	this->JumpjetAllowLayerDeviation.Read(exINI, pSection, "JumpjetAllowLayerDeviation");
	this->JumpjetTurnToTarget.Read(exINI, pSection, "JumpjetTurnToTarget");

	this->DeployingAnim_AllowAnyDirection.Read(exINI, pSection, "DeployingAnim.AllowAnyDirection");
	this->DeployingAnim_KeepUnitVisible.Read(exINI, pSection, "DeployingAnim.KeepUnitVisible");
	this->DeployingAnim_ReverseForUndeploy.Read(exINI, pSection, "DeployingAnim.ReverseForUndeploy");
	this->DeployingAnim_UseUnitDrawer.Read(exINI, pSection, "DeployingAnim.UseUnitDrawer");

	this->CanRepairCyborgLegs.Read(exINI, pSection, "CanRepairCyborgLegs");

	// The following loop iterates over size + 1 INI entries so that the
	// vector contents can be properly overriden via scenario rules - Kerbiter
	for (size_t i = 0; i <= this->AttachmentData.size(); ++i)
	{
		Nullable<AttachmentTypeClass*> type;
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "Attachment%d.Type", i);
		type.Read(exINI, pSection, tempBuffer);

		if (!type.isset())
			continue;

		NullableIdx<TechnoTypeClass> technoType;
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "Attachment%d.TechnoType", i);
		technoType.Read(exINI, pSection, tempBuffer);

		if (!technoType.isset())
			continue;

		Valueable<CoordStruct> flh;
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "Attachment%d.FLH", i);
		flh.Read(exINI, pSection, tempBuffer);

		Valueable<bool> isOnTurret;
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "Attachment%d.IsOnTurret", i);
		isOnTurret.Read(exINI, pSection, tempBuffer);

		if (i == AttachmentData.size())
		{
			std::unique_ptr<AttachmentDataEntry> tmp = nullptr;
			tmp.reset(new AttachmentDataEntry(type, TechnoTypeClass::Array->GetItem(technoType), flh, isOnTurret));
			this->AttachmentData.emplace_back(std::move(tmp));
		}
		else
		{
			this->AttachmentData[i].reset(new AttachmentDataEntry(type, TechnoTypeClass::Array->GetItem(technoType), flh, isOnTurret));
		}
	}

	//GiftBox
	this->GiftBoxData.GiftBox_Types.Read(exINI, pSection, "GiftBox.Types");
	this->GiftBoxData.GiftBox_Nums.Read(exINI, pSection, "GiftBox.Nums");
	this->GiftBoxData.GiftBox_Remove.Read(exINI, pSection, "GiftBox.Remove");
	this->GiftBoxData.GiftBox_Destroy.Read(exINI, pSection, "GiftBox.Destroy");
	this->GiftBoxData.GiftBox_Delay.Read(exINI, pSection, "GiftBox.Delay");
	this->GiftBoxData.GiftBox_DelayMinMax.Read(exINI, pSection, "GiftBox.DealyMinMax");
	this->GiftBoxData.GiftBox_EmptyCell.Read(exINI, pSection, "GiftBox.EmptyCell");
	this->GiftBoxData.GiftBox_CellRandomRange.Read(exINI, pSection, "GiftBox.CellRandomRange");
	this->GiftBoxData.GiftBox_RandomType.Read(exINI, pSection, "GiftBox.RandomType");

	this->InitialStrength_Cloning.Read(exINI, pSection, "InitialStrength.Cloning");

	for (size_t i = 0; i < TemperatureTypeClass::Array.size(); i++)
	{
		TemperatureTypeClass* pTempType = TemperatureTypeClass::Array[i].get();
		const char* pName = pTempType->Name;
		Nullable<int> maxTemperature;
		Nullable<int> heatUp_Frame;
		Nullable<int> heatUp_Amount;
		Valueable<bool> disable;
		Nullable<int> heatUp_Delay;

		const char* baseFlag = "Temperature.%s.%s";
		char key[0x50];
		_snprintf_s(key, _TRUNCATE, baseFlag, pName, "Max");
		maxTemperature.Read(exINI, pSection, key);
		_snprintf_s(key, _TRUNCATE, baseFlag, pName, "HeatUp.Frame");
		heatUp_Frame.Read(exINI, pSection, key);
		_snprintf_s(key, _TRUNCATE, baseFlag, pName, "HeatUp.Amount");
		heatUp_Amount.Read(exINI, pSection, key);
		_snprintf_s(key, _TRUNCATE, baseFlag, pName, "Disable");
		disable.Read(exINI, pSection, key);
		_snprintf_s(key, _TRUNCATE, baseFlag, pName, "HeatUp.Delay");
		heatUp_Delay.Read(exINI, pSection, key);

		Temperature.emplace(i, maxTemperature.Get(OwnerObject()->Strength));

		if (heatUp_Frame.isset())
			Temperature_HeatUpFrame.emplace(i, heatUp_Frame);

		if (heatUp_Amount.isset())
			Temperature_HeatUpAmount.emplace(i, heatUp_Amount);

		if (heatUp_Delay.isset())
			Temperature_HeatUpDelay.emplace(i, heatUp_Delay);

		Temperatrue_Disable.emplace(i, disable);
	}
	
	this->Overload_Count.Read(exINI, pSection, "Overload.Count");
	this->Overload_Damage.Read(exINI, pSection, "Overload.Damage");
	this->Overload_Frames.Read(exINI, pSection, "Overload.Frames");
	this->Overload_DeathSound.Read(exINI, pSection, "Overload.DeathSound");
	this->Overload_ParticleSys.Read(exINI, pSection, "Overload.ParticleSys");
	this->Overload_ParticleSysCount.Read(exINI, pSection, "Overload.ParticleSysCount");
	this->Draw_MindControlLink.Read(exINI, pSection, "MindControl.DrawLink");
	this->EnemyUIName.Read(exINI, pSection, "EnemyUIName");
	this->ForceWeapon_Naval_Decloaked.Read(exINI, pSection, "ForceWeapon.Naval.Decloaked");
	this->Ammo_Shared.Read(exINI, pSection, "Ammo.Shared");
	this->Ammo_Shared_Group.Read(exINI, pSection, "Ammo.Shared.Group");
	this->SelfHealGainType.Read(exINI, pSection, "SelfHealGainType");
	this->Passengers_SyncOwner.Read(exINI, pSection, "Passengers.SyncOwner");
	this->Passengers_SyncOwner_RevertOnExit.Read(exINI, pSection, "Passengers.SyncOwner.RevertOnExit");

	this->IronCurtain_KeptOnDeploy.Read(exINI, pSection, "IronCurtain.KeptOnDeploy");
	this->IronCurtain_Affect.Read(exINI, pSection, "IronCurtain.Affect");
	this->IronCuratin_KillWarhead.Read(exINI, pSection, "IronCurtain.KillWarhead");

	this->Insignia.Read(exINI, pSection, "Insignia.%s");
	this->InsigniaFrames.Read(exINI, pSection, "InsigniaFrames");
	this->InsigniaFrame.Read(exINI, pSection, "InsigniaFrame.%s");
	this->Insignia_ShowEnemy.Read(exINI, pSection, "Insignia.ShowEnemy");

	this->DigitalDisplayTypes.Read(exINI, pSection, "DigitalDisplayTypes");
	this->DigitalDisplay_Disable.Read(exINI, pSection, "DigitalDisplay.Disable");

	this->HugeBar.Read(exINI, pSection, "HugeBar");
	this->HugeBar_Priority.Read(exINI, pSection, "HugeBar.Priority");

	this->IonCannonType.Read(exINI, pSection, "IonCannonType", true);

	this->FireSelf_Weapon.Read(exINI, pSection, "FireSelf.Weapon.%s");
	this->FireSelf_ROF.Read(exINI, pSection, "FireSelf.ROF.%s");
	this->FireSelf_Immediately.Read(exINI, pSection, "FireSelf.Immediately.%s");

	this->HealthBar_Pips.Read(exINI, pSection, "HealthBar.Pips");
	this->HealthBar_Pips_DrawOffset.Read(exINI, pSection, "HealthBar.Pips.DrawOffset");
	this->HealthBar_PipsLength.Read(exINI, pSection, "HealthBar.PipsLength");
	this->HealthBar_PipsSHP.Read(pINI, pSection, "HealthBar.PipsSHP");
	this->HealthBar_PipsPAL.Read(pINI, pSection, "HealthBar.PipsPAL");
	this->HealthBar_PipBrd.Read(exINI, pSection, "HealthBar.PipBrd");
	this->HealthBar_PipBrdSHP.Read(pINI, pSection, "HealthBar.PipBrdSHP");
	this->HealthBar_PipBrdPAL.Read(pINI, pSection, "HealthBar.PipBrdPAL");
	this->HealthBar_PipBrdOffset.Read(exINI, pSection, "HealthBar.PipBrdOffset");
	this->HealthBar_XOffset.Read(exINI, pSection, "HealthBar.XOffset");
	this->UseNewHealthBar.Read(exINI, pSection, "UseNewHealthBar");
	this->HealthBar_PictureSHP.Read(pINI, pSection, "HealthBar.PictureSHP");
	this->HealthBar_PicturePAL.Read(pINI, pSection, "HealthBar.PicturePAL");
	this->HealthBar_PictureTransparency.Read(exINI, pSection, "HealthBar.PictureTransparency");

	this->GroupID_Offset.Read(exINI, pSection, "GroupID.ShowOffset");
	this->SelfHealPips_Offset.Read(exINI, pSection, "SelfHealPips.ShowOffset");
	this->UseCustomHealthBar.Read(exINI, pSection, "UseCustomHealthBar");
	this->UseUnitHealthBar.Read(exINI, pSection, "UseUnitHealthBar");

	this->GScreenAnimType.Read(exINI, pSection, "GScreenAnimType", true);

	this->RandomProduct.Read(exINI, pSection, "RandomProduct");

	this->MovePassengerToSpawn.Read(exINI, pSection, "MovePassengerToSpawn");
	this->SilentPassenger.Read(exINI, pSection, "SilentPassenger");
	
	this->DeterminedByRange.Read(exINI, pSection, "DeterminedByRange");
	this->DeterminedByRange_ExtraRange.Read(exINI, pSection, "DeterminedByRange.ExtraRange");
	this->DeterminedByRange_MainWeapon.Read(exINI, pSection, "DeterminedByRange.MainWeapon");

	this->BuildLimit_Group_Types.Read(exINI, pSection, "BuildLimit.Group.Types");
	this->BuildLimit_Group_Any.Read(exINI, pSection, "BuildLimit.Group.Any");
	this->BuildLimit_Group_Limits.Read(exINI, pSection, "BuildLimit.Group.Limits");

	this->BuildLimit_As.Read(exINI, pSection, "BuildLimitAs");

	this->VehicleImmuneToMindControl.Read(exINI, pSection, "VehicleImmuneToMindControl");

	NullableIdx<TechnoTypeClass> convert_deploy;
	convert_deploy.Read(exINI, pSection, "Convert.Deploy");

	if (convert_deploy.isset())
		this->Convert_Deploy = TechnoTypeClass::Array->GetItem(convert_deploy);

	this->Convert_DeployAnim.Read(exINI, pSection, "Convert.DeployAnim");

	this->Gattling_SelectWeaponByVersus.Read(exINI, pSection, "Gattling.SelectWeaponByVersus");
	this->IsExtendGattling.Read(exINI, pSection, "IsExtendGattling");
	this->Gattling_Cycle.Read(exINI, pSection, "Gattling.Cycle");
	this->Gattling_Charge.Read(exINI, pSection, "Gattling.Charge");

	this->OccupyWeapon.Read(exINI, pSection, "OccupyWeapon");
	this->VeteranOccupyWeapon.Read(exINI, pSection, "VeteranOccupyWeapon");
	this->EliteOccupyWeapon.Read(exINI, pSection, "EliteOccupyWeapon");

	this->JJConvert_Unload.Read(exINI, pSection, "JumpJetConvert.Unload");

	this->AttackedWeapon.Read(exINI, pSection, "AttackedWeapon");
	this->AttackedWeapon_Veteran.Read(exINI, pSection, "AttackedWeapon.Veteran");
	this->AttackedWeapon_Elite.Read(exINI, pSection, "AttackedWeapon.Elite");
	this->AttackedWeapon_FireToAttacker.Read(exINI, pSection, "AttackedWeapon.FireToAttacker");
	this->AttackedWeapon_ROF.Read(exINI, pSection, "AttackedWeapon.ROF");
	this->AttackedWeapon_IgnoreROF.Read(exINI, pSection, "AttackedWeapon.IgnoreROF");
	this->AttackedWeapon_IgnoreRange.Read(exINI, pSection, "AttackedWeapon.IgnoreRange");
	this->AttackedWeapon_Range.Read(exINI, pSection, "AttackedWeapon.Range");
	this->AttackedWeapon_ResponseWarhead.Read(exINI, pSection, "AttackedWeapon.ResponseWarhead");
	this->AttackedWeapon_NoResponseWarhead.Read(exINI, pSection, "AttackedWeapon.NoResponseWarhead");
	this->AttackedWeapon_ResponseZeroDamage.Read(exINI, pSection, "AttackedWeapon.ResponseZeroDamage");
	this->AttackedWeapon_ActiveMaxHealth.Read(exINI, pSection, "AttackedWeapon.ActiveMaxHealth");
	this->AttackedWeapon_ActiveMinHealth.Read(exINI, pSection, "AttackedWeapon.ActiveMinHealth");

	for (size_t i = 0; i < AttackedWeapon.size(); i++)
	{
		Valueable<CoordStruct> flh;
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "AttackedWeapon%u.FLH", i);
		flh.Read(exINI, pSection, tempBuffer);
		AttackedWeapon_FLHs.emplace_back(flh.Get());

		Nullable<AffectedHouse> responseHouse;
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "AttackedWeapon%u.AffectedHouse", i);
		responseHouse.Read(exINI, pSection, tempBuffer);

		if (responseHouse.isset())
			AttackedWeapon_ResponseHouse.emplace_back(responseHouse.Get());
		else
			AttackedWeapon_ResponseHouse.emplace_back(AffectedHouse::All);
	}

	this->WeaponInTransport.Read(exINI, pSection, "WeaponInTransport.%s");
	
	this->ProtectPassengers.Read(exINI, pSection, "ProtectPassengers");
	this->ProtectPassengers_Clear.Read(exINI, pSection, "ProtectPassengers.Clear");
	this->ProtectPassengers_Release.Read(exINI, pSection, "ProtectPassengers.Release");
	this->ProtectPassengers_Damage.Read(exINI, pSection, "ProtectPassengers.Damage");

	this->Dodge_Houses.Read(exINI, pSection, "Dodge.Houses");
	this->Dodge_MaxHealthPercent.Read(exINI, pSection, "Dodge.MaxHealthPercent");
	this->Dodge_MinHealthPercent.Read(exINI, pSection, "Dodge.MinHealthPercent");
	this->Dodge_Chance.Read(exINI, pSection, "Dodge.Chance");
	this->Dodge_Anim.Read(exINI, pSection, "Dodge.Anim");
	this->Dodge_OnlyDodgePositiveDamage.Read(exINI, pSection, "Dodge.OnlyDodgePositiveDamage");

	this->MoveDamage.Read(exINI, pSection, "MoveDamage");
	this->MoveDamage_Delay.Read(exINI, pSection, "MoveDamage.Delay");
	this->MoveDamage_Warhead.Read(exINI, pSection, "MoveDamage.Warhead");
	this->MoveDamage_Anim.Read(exINI, pSection, "MoveDamage.Anim");
	this->StopDamage.Read(exINI, pSection, "StopDamage");
	this->StopDamage_Delay.Read(exINI, pSection, "StopDamage.Delay");
	this->StopDamage_Warhead.Read(exINI, pSection, "StopDamage.Warhead");
	this->StopDamage_Anim.Read(exINI, pSection, "StopDamage.Anim");

	this->WeaponRangeShare_Technos.Read(exINI, pSection, "WeaponRangeShare.Technos");
	this->WeaponRangeShare_Range.Read(exINI, pSection, "WeaponRangeShare.Range");
	this->WeaponRangeShare_ForceAttack.Read(exINI, pSection, "WeaponRangeShare.ForceAttack");
	this->WeaponRangeShare_UseWeapon.Read(exINI, pSection, "WeaponRangeShare.UseWeapon");

	this->AllowMinHealth.Read(exINI, pSection, "AllowMinHealth");

	this->BuiltAt.Read(exINI, pSection, "BuiltAt");
	this->TurretROT.Read(exINI, pSection, "TurretROT");

	this->InitialPayload_Types.Read(exINI, pSection, "InitialPayload.Types");
	this->InitialPayload_Nums.Read(exINI, pSection, "InitialPayload.Nums");

	this->AllowMaxDamage.Read(exINI, pSection, "AllowMaxDamage");
	this->AllowMinDamage.Read(exINI, pSection, "AllowMinDamage");

	this->ImmuneToAbsorb.Read(exINI, pSection, "ImmuneToAbsorb");

	this->TeamAffect.Read(exINI, pSection, "TeamAffect");
	this->TeamAffect_Range.Read(exINI, pSection, "TeamAffect.Range");
	this->TeamAffect_Technos.Read(exINI, pSection, "TeamAffect.Technos");
	this->TeamAffect_Houses.Read(exINI, pSection, "TeamAffect.Houses");
	this->TeamAffect_Number.Read(exINI, pSection, "TeamAffect.Number");
	this->TeamAffect_Weapon.Read(exINI, pSection, "TeamAffect.Weapon");
	this->TeamAffect_ROF.Read(exINI, pSection, "TeamAffect.ROF");
	this->TeamAffect_LoseEfficacyWeapon.Read(exINI, pSection, "TeamAffect.LoseEfficacyWeapon");
	this->TeamAffect_LoseEfficacyROF.Read(exINI, pSection, "TeamAffect.LoseEfficacyROF");
	this->TeamAffect_Anim.Read(exINI, pSection, "TeamAffect.Anim");
	this->TeamAffect_ShareDamage.Read(exINI, pSection, "TeamAffect.ShareDamage");
	this->TeamAffect_MaxNumber.Read(exINI, pSection, "TeamAffect.MaxNumber");

	this->PoweredUnitBy.Read(exINI, pSection, "PoweredUnitBy");
	this->PoweredUnitBy_Any.Read(exINI, pSection, "PoweredUnitBy.Any");
	this->PoweredUnitBy_ParticleSystem.Read(exINI, pSection, "PoweredUnitBy.ParticleSystem");
	this->PoweredUnitBy_Sparkles.Read(exINI, pSection, "PoweredUnitBy.Sparkles");
	this->PoweredUnitBy_ParticleSystemXOffset.Read(exINI, pSection, "PoweredUnitBy.ParticleSystemXOffset");
	this->PoweredUnitBy_ParticleSystemYOffset.Read(exINI, pSection, "PoweredUnitBy.ParticleSystemYOffset");
	this->PoweredUnitBy_ParticleSystemSpawnDelay.Read(exINI, pSection, "PoweredUnitBy.ParticleSystemSpawnDelay");

	this->VeteranAnim.Read(exINI, pSection, "VeteranAnim");
	this->EliteAnim.Read(exINI, pSection, "EliteAnim");

	this->PassengerHeal_Rate.Read(exINI, pSection, "PassengerHeal.Rate");
	this->PassengerHeal_HealAll.Read(exINI, pSection, "PassengerHeal.HealAll");
	this->PassengerHeal_Amount.Read(exINI, pSection, "PassengerHeal.Amount");
	this->PassengerHeal_Sound.Read(exINI, pSection, "PassengerHeal.Sound");
	this->PassengerHeal_Anim.Read(exINI, pSection, "PassengerHeal.Anim");
	this->PassengerHeal_Houses.Read(exINI, pSection, "PassengerHeal.Houses");

	this->EVA_Sold.Read(exINI, pSection, "EVA.Sold");

	this->SelectBox_Shape.Read(exINI, pSection, "SelectBox.Shape");
	this->SelectBox_Palette.LoadFromINI(pINI, pSection, "SelectBox.Palette");
	this->SelectBox_Frame.Read(exINI, pSection, "SelectBox.Frame");
	this->SelectBox_DrawOffset.Read(exINI, pSection, "SelectBox.DrawOffset");
	this->SelectBox_TranslucentLevel.Read(exINI, pSection, "SelectBox.TranslucentLevel");
	this->SelectBox_CanSee.Read(exINI, pSection, "SelectBox.CanSee");
	this->SelectBox_CanObserverSee.Read(exINI, pSection, "SelectBox.CanObserverSee");

	TechnoTypeExt::GetWeaponStages(pThis, exINI, pSection, Stages, VeteranStages, EliteStages);
	TechnoTypeExt::GetIFVTurrets(pThis, exINI, pSection, Turrets);

	this->AttachEffects.Read(exINI, pSection, "AttachEffects");
	this->AttachEffects_Duration.Read(exINI, pSection, "AttachEffects.Duration");
	this->AttachEffects_Delay.Read(exINI, pSection, "AttachEffects.Delay");
	this->AttachEffects_Immune.Read(exINI, pSection, "AttachEffects.Immune");
	this->AttachEffects_OnlyAccept.Read(exINI, pSection, "AttachEffects.OnlyAccept");

	this->MobileRefinery.Read(exINI, pSection, "MobileRefinery");
	this->MobileRefinery_TransRate.Read(exINI, pSection, "MobileRefinery.TransRate");
	this->MobileRefinery_CashMultiplier.Read(exINI, pSection, "MobileRefinery.CashMultiplier");
	this->MobileRefinery_AmountPerCell.Read(exINI, pSection, "MobileRefinery.AmountPerCell");
	this->MobileRefinery_FrontOffset.Read(exINI, pSection, "MobileRefinery.FrontOffset");
	this->MobileRefinery_LeftOffset.Read(exINI, pSection, "MobileRefinery.LeftOffset");
	this->MobileRefinery_Display.Read(exINI, pSection, "MobileRefinery.Display");
	this->MobileRefinery_DisplayColor.Read(exINI, pSection, "MobileRefinery.DisplayColor");
	this->MobileRefinery_Anims.Read(exINI, pSection, "MobileRefinery.Anims");
	this->MobileRefinery_AnimMove.Read(exINI, pSection, "MobileRefinery.AnimMove");

	this->AllowPlanningMode.Read(exINI, pSection, "AllowPlanningMode");

	this->PassengerProduct.Read(exINI, pSection, "PassengerProduct");
	this->PassengerProduct_Type.Read(exINI, pSection, "PassengerProduct.Type");
	this->PassengerProduct_Rate.Read(exINI, pSection, "PassengerProduct.Rate");
	this->PassengerProduct_Amount.Read(exINI, pSection, "PassengerProduct.Amount");
	this->PassengerProduct_RandomPick.Read(exINI, pSection, "PassengerProduct.RandomPick");

	this->PassengerProduct_Type.erase
	(
		std::remove_if
		(
			this->PassengerProduct_Type.begin(),
			this->PassengerProduct_Type.end(),
			[](TechnoTypeClass* pType)
			{
				return pType->WhatAmI() == AbstractType::BuildingType;
			}
		),
		this->PassengerProduct_Type.end()
	);

	this->UseConvert.Read(exINI, pSection, "UseConvert");

	for (size_t i = 0; ; ++i)
	{
		char convert[32];
		NullableIdx<TechnoTypeClass> passenger;
		sprintf_s(convert, sizeof(convert), "Convert%d.Passenger", i);
		passenger.Read(exINI, pSection, convert);

		if (!passenger.isset())
			break;

		NullableIdx<TechnoTypeClass> type;
		sprintf_s(convert, sizeof(convert), "Convert%d.Type", i);
		type.Read(exINI, pSection, convert);

		if (!type.isset())
			break;

		this->Convert_Passengers.push_back(passenger);
		this->Convert_Types.push_back(type);
	}

	// Ares 0.2
	this->RadarJamRadius.Read(exINI, pSection, "RadarJamRadius");

	// Ares 0.9
	this->InhibitorRange.Read(exINI, pSection, "InhibitorRange");

	// Ares 0.A
	this->GroupAs.Read(pINI, pSection, "GroupAs");

	// Ares 0.C
	this->NoAmmoWeapon.Read(exINI, pSection, "NoAmmoWeapon");
	this->NoAmmoAmount.Read(exINI, pSection, "NoAmmoAmount");

	this->Passengers_BySize.Read(exINI, pSection, "Passengers.BySize");

	// Art tags
	INI_EX exArtINI(CCINIClass::INI_Art);
	auto pArtSection = pThis->ImageFile;

	this->TurretOffset.Read(exArtINI, pArtSection, "TurretOffset");

	//char tempBuffer[32];
	for (size_t i = 0; ; ++i)
	{
		NullableIdx<LaserTrailTypeClass> trail;
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "LaserTrail%d.Type", i);
		trail.Read(exArtINI, pArtSection, tempBuffer);

		if (!trail.isset())
			break;

		Valueable<CoordStruct> flh;
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "LaserTrail%d.FLH", i);
		flh.Read(exArtINI, pArtSection, tempBuffer);

		Valueable<bool> isOnTurret;
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "LaserTrail%d.IsOnTurret", i);
		isOnTurret.Read(exArtINI, pArtSection, tempBuffer);

		this->LaserTrailData.push_back({ ValueableIdx<LaserTrailTypeClass>(trail), flh, isOnTurret });
	}

	TechnoTypeExt::GetWeaponFLHs(pThis, exArtINI, pArtSection, WeaponFLHs, VeteranWeaponFLHs, EliteWeaponFLHs);
	
	TechnoTypeExt::GetBurstFLHs(pThis, exArtINI, pArtSection, WeaponBurstFLHs, VeteranWeaponBurstFLHs, EliteWeaponBurstFLHs, "");
	TechnoTypeExt::GetBurstFLHs(pThis, exArtINI, pArtSection, DeployedWeaponBurstFLHs, VeteranDeployedWeaponBurstFLHs, EliteDeployedWeaponBurstFLHs, "Deployed");
	TechnoTypeExt::GetBurstFLHs(pThis, exArtINI, pArtSection, CrouchedWeaponBurstFLHs, VeteranCrouchedWeaponBurstFLHs, EliteCrouchedWeaponBurstFLHs, "Prone");

	this->PronePrimaryFireFLH.Read(exArtINI, pArtSection, "PronePrimaryFireFLH");
	this->ProneSecondaryFireFLH.Read(exArtINI, pArtSection, "ProneSecondaryFireFLH");
	this->DeployedPrimaryFireFLH.Read(exArtINI, pArtSection, "DeployedPrimaryFireFLH");
	this->DeployedSecondaryFireFLH.Read(exArtINI, pArtSection, "DeployedSecondaryFireFLH");

	Subset_1 = Subset_1_Used();
	Subset_2 = Subset_2_Used();
	Subset_3 = Subset_3_Used();
}

bool TechnoTypeExt::ExtData::CanBeBuiltAt_Ares(BuildingTypeClass* pFactoryType)
{
	auto const pBExt = BuildingTypeExt::ExtMap.Find(pFactoryType);
	return (this->BuiltAt.empty() && !pBExt->Factory_ExplicitOnly)
		|| this->BuiltAt.Contains(pFactoryType);
}

/*
		EatPassengers
		MovePassengerToSpawn
		IonCannon
		AutoDeath
		AttackedWeapon
		PoweredShield
		PassengerHeal
*/
bool TechnoTypeExt::ExtData::Subset_1_Used() const
{
	return
		PassengerDeletion_Rate > 0
		|| MovePassengerToSpawn
		|| IonCannonType.isset()
		|| AutoDeath_Behavior.isset()
		|| !AttackedWeapon.empty()
		|| !AttackedWeapon_Veteran.empty()
		|| !AttackedWeapon_Elite.empty()
		|| ShieldType != nullptr && !ShieldType->PoweredTechnos.empty()
		|| PassengerHeal_Rate > 0
		;
}

/*
		SilentPassenger
		Spawner_SameLoseTarget
		Powered_KillSpawns
		Spawn_LimitRange
		MindControlRange
		Veteran/Elite Anim
		MobileRefinery
*/
bool TechnoTypeExt::ExtData::Subset_2_Used() const
{
	return
		SilentPassenger
		|| Spawner_SameLoseTarget
		|| Powered_KillSpawns
		|| Spawner_LimitRange
		|| MindControlRangeLimit.Get().value > 0
		|| VeteranAnim != nullptr
		|| EliteAnim != nullptr
		|| MobileRefinery
		;
}

/*
		LaserTrails
		ExtendGattling
		FireSelf
		VeteranWeapon
		TeamAffect
		PoweredUnit
		PassengerProduct
*/
bool TechnoTypeExt::ExtData::Subset_3_Used() const
{
	return
		!LaserTrailData.empty()
		|| IsExtendGattling && !OwnerObject()->IsGattling
		|| !FireSelf_Weapon.BaseValue.empty()
		|| FireSelf_Weapon.ConditionYellow.HasValue()
		|| FireSelf_Weapon.ConditionRed.HasValue()
		|| FireSelf_Weapon.MaxValue.HasValue()
		|| TeamAffect && TeamAffect_Range > 0.0
		|| !PoweredUnitBy.empty()
		|| PassengerProduct
		;
}

template <typename T>
void TechnoTypeExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->Weapons)

		.Process(this->HealthBar_Hide)
		.Process(this->UIDescription)
		.Process(this->LowSelectionPriority)
		.Process(this->MindControlRangeLimit)

		.Process(this->Interceptor)
		.Process(this->Interceptor_CanTargetHouses)
		.Process(this->Interceptor_Rookie)
		.Process(this->Interceptor_Veteran)
		.Process(this->Interceptor_Elite)
		.Process(this->Interceptor_GuardRange)
		.Process(this->Interceptor_MinimumGuardRange)
		.Process(this->Interceptor_Weapon)
		.Process(this->Interceptor_DeleteOnIntercept)
		.Process(this->Interceptor_WeaponOverride)
		.Process(this->Interceptor_WeaponReplaceProjectile)
		.Process(this->Interceptor_WeaponCumulativeDamage)
		.Process(this->Interceptor_KeepIntact)
		.Process(this->Interceptor_Success)
		.Process(this->Interceptor_RookieSuccess)
		.Process(this->Interceptor_VeteranSuccess)
		.Process(this->Interceptor_EliteSuccess)

		.Process(this->GroupAs)
		.Process(this->RadarJamRadius)
		.Process(this->InhibitorRange)
		.Process(this->TurretOffset)
		.Process(this->Powered_KillSpawns)

		.Process(this->Spawner_LimitRange)
		.Process(this->Spawner_ExtraLimitRange)
		.Process(this->Spawner_SameLoseTarget)
		.Process(this->Spawner_DelayFrames)
		.Process(this->Spawner_DelayFrams_PerSpawn)
		.Process(this->Spawn_Types)
		.Process(this->Spawn_Nums)
		.Process(this->Spawn_ReloadRate)
		.Process(this->Spawn_RegenRate)

		.Process(this->Harvester_Counted)
		.Process(this->Promote_IncludeSpawns)
		.Process(this->ImmuneToCrit)
		.Process(this->MultiMindControl_ReleaseVictim)
		.Process(this->CameoPriority)
		.Process(this->NoManualMove)
		.Process(this->InitialStrength)

		.Process(this->AutoDeath_Behavior)
		.Process(this->AutoDeath_OnAmmoDepletion)
		.Process(this->AutoDeath_AfterDelay)
		.Process(this->AutoDeath_Nonexist)
		.Process(this->AutoDeath_Nonexist_House)
		.Process(this->AutoDeath_Exist)
		.Process(this->AutoDeath_Exist_House)

		.Process(this->Slaved_OwnerWhenMasterKilled)
		.Process(this->SellSound)
		.Process(this->ShieldType)

		.Process(this->WarpOut)
		.Process(this->WarpIn)
		.Process(this->WarpAway)
		.Process(this->ChronoTrigger)
		.Process(this->ChronoDistanceFactor)
		.Process(this->ChronoMinimumDelay)
		.Process(this->ChronoRangeMinimum)
		.Process(this->ChronoDelay)
		.Process(this->WarpInWeapon)
		.Process(this->WarpInMinRangeWeapon)
		.Process(this->WarpOutWeapon)
		.Process(this->WarpInWeapon_UseDistanceAsDamage)

		.Process(this->OreGathering_Anims)
		.Process(this->OreGathering_Tiberiums)
		.Process(this->OreGathering_FramesPerDir)

		.Process(this->LaserTrailData)
		.Process(this->DestroyAnim_Random)
		.Process(this->NotHuman_RandomDeathSequence)
		.Process(this->DefaultDisguise)
		.Process(this->UseDisguiseMovementSpeed)

		.Process(this->WeaponBurstFLHs)
		.Process(this->VeteranWeaponBurstFLHs)
		.Process(this->EliteWeaponBurstFLHs)

		.Process(this->GiftBoxData)
		.Process(this->AttachmentData)

		.Process(this->PassengerDeletion_Rate)
		.Process(this->PassengerDeletion_Rate_SizeMultiply)
		.Process(this->PassengerDeletion_UseCostAsRate)
		.Process(this->PassengerDeletion_CostMultiplier)
		.Process(this->PassengerDeletion_Soylent)
		.Process(this->PassengerDeletion_SoylentMultiplier)
		.Process(this->PassengerDeletion_SoylentFriendlies)
		.Process(this->PassengerDeletion_DisplaySoylent)
		.Process(this->PassengerDeletion_DisplaySoylentToHouses)
		.Process(this->PassengerDeletion_DisplaySoylentOffset)
		.Process(this->PassengerDeletion_ReportSound)
		.Process(this->PassengerDeletion_Anim)

		.Process(this->OpenTopped_RangeBonus)
		.Process(this->OpenTopped_DamageMultiplier)
		.Process(this->OpenTopped_WarpDistance)
		.Process(this->OpenTopped_IgnoreRangefinding)
		.Process(this->OpenTopped_AllowFiringIfDeactivated)

		.Process(this->AutoFire)
		.Process(this->AutoFire_TargetSelf)
		.Process(this->NoSecondaryWeaponFallback)
		.Process(this->NoAmmoWeapon)
		.Process(this->NoAmmoAmount)
		.Process(this->JumpjetAllowLayerDeviation)
		.Process(this->JumpjetTurnToTarget)

		.Process(this->DeployingAnim_AllowAnyDirection)
		.Process(this->DeployingAnim_KeepUnitVisible)
		.Process(this->DeployingAnim_ReverseForUndeploy)
		.Process(this->DeployingAnim_UseUnitDrawer)

		.Process(this->EnemyUIName)
		.Process(this->ForceWeapon_Naval_Decloaked)

		.Process(this->Ammo_Shared)
		.Process(this->Ammo_Shared_Group)

		.Process(this->Passengers_ChangeOwnerWithTransport)

		.Process(this->SelectBox_Shape)
		.Process(this->SelectBox_Palette)
		.Process(this->SelectBox_Frame)
		.Process(this->SelectBox_DrawOffset)
		.Process(this->SelectBox_TranslucentLevel)
		.Process(this->SelectBox_CanSee)
		.Process(this->SelectBox_CanObserverSee)

		.Process(this->PronePrimaryFireFLH)
		.Process(this->ProneSecondaryFireFLH)
		.Process(this->DeployedPrimaryFireFLH)
		.Process(this->DeployedSecondaryFireFLH)
		.Process(this->CrouchedWeaponBurstFLHs)
		.Process(this->VeteranCrouchedWeaponBurstFLHs)
		.Process(this->EliteCrouchedWeaponBurstFLHs)
		.Process(this->DeployedWeaponBurstFLHs)
		.Process(this->VeteranDeployedWeaponBurstFLHs)
		.Process(this->EliteDeployedWeaponBurstFLHs)

		.Process(this->CanRepairCyborgLegs)

		.Process(this->Overload_Count)
		.Process(this->Overload_Damage)
		.Process(this->Overload_Frames)
		.Process(this->Overload_DeathSound)
		.Process(this->Overload_ParticleSys)
		.Process(this->Overload_ParticleSysCount)

		.Process(this->Draw_MindControlLink)

		.Process(this->SelfHealGainType)

		.Process(this->Passengers_SyncOwner)
		.Process(this->Passengers_SyncOwner_RevertOnExit)

		.Process(this->Insignia)
		.Process(this->InsigniaFrames)
		.Process(this->InsigniaFrame)
		.Process(this->Insignia_ShowEnemy)
		.Process(this->InitialStrength_Cloning)

		.Process(this->MobileRefinery)
		.Process(this->MobileRefinery_TransRate)
		.Process(this->MobileRefinery_CashMultiplier)
		.Process(this->MobileRefinery_AmountPerCell)
		.Process(this->MobileRefinery_FrontOffset)
		.Process(this->MobileRefinery_LeftOffset)
		.Process(this->MobileRefinery_Display)
		.Process(this->MobileRefinery_DisplayColor)
		.Process(this->MobileRefinery_Anims)
		.Process(this->MobileRefinery_AnimMove)

		.Process(this->IronCurtain_KeptOnDeploy)
		.Process(this->IronCurtain_Affect)
		.Process(this->IronCuratin_KillWarhead)


		.Process(this->DigitalDisplayTypes)
		.Process(this->DigitalDisplay_Disable)
		.Process(this->HugeBar)
		.Process(this->HugeBar_Priority)

		.Process(this->IonCannonType)

		.Process(this->FireSelf_Weapon)
		.Process(this->FireSelf_ROF)
		.Process(this->FireSelf_Immediately)

		.Process(this->HealthBar_Pips)
		.Process(this->HealthBar_Pips_DrawOffset)
		.Process(this->HealthBar_PipsLength)
		.Process(this->HealthBar_PipsSHP)
		.Process(this->HealthBar_PipsPAL)
		.Process(this->HealthBar_PipBrd)
		.Process(this->HealthBar_PipBrdSHP)
		.Process(this->HealthBar_PipBrdPAL)
		.Process(this->HealthBar_PipBrdOffset)
		.Process(this->HealthBar_XOffset)

		.Process(this->UseNewHealthBar)
		.Process(this->HealthBar_PictureSHP)
		.Process(this->HealthBar_PicturePAL)
		.Process(this->HealthBar_PictureTransparency)
		.Process(this->GroupID_Offset)
		.Process(this->SelfHealPips_Offset)
		.Process(this->UseCustomHealthBar)
		.Process(this->UseUnitHealthBar)

		.Process(this->GScreenAnimType)

		.Process(this->MovePassengerToSpawn)
		.Process(this->SilentPassenger)

		.Process(this->DeterminedByRange)
		.Process(this->DeterminedByRange_ExtraRange)
		.Process(this->DeterminedByRange_MainWeapon)

		.Process(this->BuildLimit_Group_Types)
		.Process(this->BuildLimit_Group_Any)
		.Process(this->BuildLimit_Group_Limits)

		.Process(this->VehicleImmuneToMindControl)
		.Process(this->Convert_Deploy)
		.Process(this->Convert_DeployAnim)

		.Process(this->Gattling_SelectWeaponByVersus)
		.Process(this->IsExtendGattling)
		.Process(this->Gattling_Cycle)
		.Process(this->Gattling_Charge)
		.Process(this->Stages)
		.Process(this->VeteranStages)
		.Process(this->EliteStages)
		.Process(this->WeaponFLHs)
		.Process(this->VeteranWeaponFLHs)
		.Process(this->EliteWeaponFLHs)
		.Process(this->OccupyWeapon)
		.Process(this->VeteranOccupyWeapon)
		.Process(this->EliteOccupyWeapon)

		.Process(this->JJConvert_Unload)
		.Process(this->BuildLimit_As)
		.Process(this->BuiltAt)
		.Process(this->TurretROT)

		.Process(this->AttackedWeapon)
		.Process(this->AttackedWeapon_Veteran)
		.Process(this->AttackedWeapon_Elite)
		.Process(this->AttackedWeapon_ROF)
		.Process(this->AttackedWeapon_FireToAttacker)
		.Process(this->AttackedWeapon_IgnoreROF)
		.Process(this->AttackedWeapon_IgnoreRange)
		.Process(this->AttackedWeapon_Range)
		.Process(this->AttackedWeapon_ResponseWarhead)
		.Process(this->AttackedWeapon_NoResponseWarhead)
		.Process(this->AttackedWeapon_ResponseZeroDamage)
		.Process(this->AttackedWeapon_ResponseHouse)
		.Process(this->AttackedWeapon_ActiveMaxHealth)
		.Process(this->AttackedWeapon_ActiveMinHealth)
		.Process(this->AttackedWeapon_FLHs)

		.Process(this->WeaponInTransport)

		.Process(this->ProtectPassengers)
		.Process(this->ProtectPassengers_Clear)
		.Process(this->ProtectPassengers_Release)
		.Process(this->ProtectPassengers_Damage)

		.Process(this->Dodge_Houses)
		.Process(this->Dodge_MaxHealthPercent)
		.Process(this->Dodge_MinHealthPercent)
		.Process(this->Dodge_Chance)
		.Process(this->Dodge_Anim)
		.Process(this->Dodge_OnlyDodgePositiveDamage)

		.Process(this->MoveDamage)
		.Process(this->MoveDamage_Delay)
		.Process(this->MoveDamage_Warhead)
		.Process(this->MoveDamage_Anim)
		.Process(this->StopDamage)
		.Process(this->StopDamage_Delay)
		.Process(this->StopDamage_Warhead)
		.Process(this->StopDamage_Anim)

		.Process(this->InitialPayload_Types)
		.Process(this->InitialPayload_Nums)

		.Process(this->WeaponRangeShare_Technos)
		.Process(this->WeaponRangeShare_Range)
		.Process(this->WeaponRangeShare_ForceAttack)
		.Process(this->WeaponRangeShare_UseWeapon)

		.Process(this->AllowMinHealth)
		.Process(this->Turrets)
		.Process(this->AllowMaxDamage)
		.Process(this->AllowMinDamage)
		.Process(this->ImmuneToAbsorb)

		.Process(this->TeamAffect)
		.Process(this->TeamAffect_Range)
		.Process(this->TeamAffect_Technos)
		.Process(this->TeamAffect_Houses)
		.Process(this->TeamAffect_Number)
		.Process(this->TeamAffect_Weapon)
		.Process(this->TeamAffect_ROF)
		.Process(this->TeamAffect_LoseEfficacyWeapon)
		.Process(this->TeamAffect_LoseEfficacyROF)
		.Process(this->TeamAffect_Anim)
		.Process(this->TeamAffect_ShareDamage)
		.Process(this->TeamAffect_MaxNumber)

		.Process(this->PoweredUnitBy)
		.Process(this->PoweredUnitBy_Any)
		.Process(this->PoweredUnitBy_Sparkles)
		.Process(this->PoweredUnitBy_ParticleSystem)
		.Process(this->PoweredUnitBy_ParticleSystemXOffset)
		.Process(this->PoweredUnitBy_ParticleSystemYOffset)
		.Process(this->PoweredUnitBy_ParticleSystemSpawnDelay)

		.Process(this->PassengerHeal_Rate)
		.Process(this->PassengerHeal_Amount)
		.Process(this->PassengerHeal_HealAll)
		.Process(this->PassengerHeal_Sound)
		.Process(this->PassengerHeal_Anim)
		.Process(this->PassengerHeal_Houses)

		.Process(this->EVA_Sold)

		.Process(this->AttachEffects)
		.Process(this->AttachEffects_Duration)
		.Process(this->AttachEffects_Delay)
		.Process(this->AttachEffects_Immune)
		.Process(this->AttachEffects_OnlyAccept)

		.Process(this->UseConvert)
		.Process(this->Convert_Passengers)
		.Process(this->Convert_Types)

		.Process(this->Temperature)
		.Process(this->Temperature_HeatUpFrame)
		.Process(this->Temperature_HeatUpAmount)
		.Process(this->Temperature_HeatUpDelay)
		.Process(this->Temperatrue_Disable)

		.Process(this->VeteranAnim)
		.Process(this->EliteAnim)

		.Process(this->AllowPlanningMode)

		.Process(this->PassengerProduct)
		.Process(this->PassengerProduct_Type)
		.Process(this->PassengerProduct_Rate)
		.Process(this->PassengerProduct_Amount)
		.Process(this->PassengerProduct_RandomPick)

		.Process(this->Passengers_BySize)
		;

	Stm
		.Process(this->Subset_1)
		.Process(this->Subset_2)
		.Process(this->Subset_3)
		;
}

void TechnoTypeExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<TechnoTypeClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void TechnoTypeExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<TechnoTypeClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

#pragma region Data entry save/load

bool TechnoTypeExt::ExtData::LaserTrailDataEntry::Load(PhobosStreamReader& stm, bool registerForChange)
{
	return this->Serialize(stm);
}

bool TechnoTypeExt::ExtData::LaserTrailDataEntry::Save(PhobosStreamWriter& stm) const
{
	return const_cast<LaserTrailDataEntry*>(this)->Serialize(stm);
}

template <typename T>
bool TechnoTypeExt::ExtData::LaserTrailDataEntry::Serialize(T& stm)
{
	return stm
		.Process(this->Type)
		.Process(this->FLH)
		.Process(this->IsOnTurret)
		.Success();
}

bool TechnoTypeExt::ExtData::AttachmentDataEntry::Load(PhobosStreamReader& stm, bool registerForChange)
{
	return Serialize(stm);
}

bool TechnoTypeExt::ExtData::AttachmentDataEntry::Save(PhobosStreamWriter& stm) const
{
	return const_cast<AttachmentDataEntry*>(this)->Serialize(stm);
}

template <typename T>
bool TechnoTypeExt::ExtData::AttachmentDataEntry::Serialize(T& stm)
{
	return stm
		.Process(this->Type)
		.Process(this->TechnoType)
		.Process(this->FLH)
		.Process(this->IsOnTurret)
		.Success();
}

bool TechnoTypeExt::ExtData::GiftBoxDataEntry::Load(PhobosStreamReader& stm, bool RegisterForChange)
{
	return Serialize(stm);
}

bool TechnoTypeExt::ExtData::GiftBoxDataEntry::Save(PhobosStreamWriter& stm) const
{
	return const_cast<GiftBoxDataEntry*>(this)->Serialize(stm);
}

template <typename T>
bool TechnoTypeExt::ExtData::GiftBoxDataEntry::Serialize(T& stm)
{
	return stm
		.Process(this->GiftBox_Types)
		.Process(this->GiftBox_Nums)
		.Process(this->GiftBox_Remove)
		.Process(this->GiftBox_Destroy)
		.Process(this->GiftBox_Delay)
		.Process(this->GiftBox_DelayMinMax)
		.Process(this->GiftBox_CellRandomRange)
		.Process(this->GiftBox_EmptyCell)
		.Process(this->GiftBox_RandomType)
		.Success()
		;
}

#pragma endregion

// =============================
// container

TechnoTypeExt::ExtContainer::ExtContainer() : Container("TechnoTypeClass") { }
TechnoTypeExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x711835, TechnoTypeClass_CTOR, 0x5)
{
	GET(TechnoTypeClass*, pItem, ESI);

	TechnoTypeExt::ExtMap.FindOrAllocate(pItem);

	return 0;
}

DEFINE_HOOK(0x711AE0, TechnoTypeClass_DTOR, 0x5)
{
	GET(TechnoTypeClass*, pItem, ECX);

	TechnoTypeExt::ExtMap.Remove(pItem);

	return 0;
}

DEFINE_HOOK_AGAIN(0x716DC0, TechnoTypeClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x7162F0, TechnoTypeClass_SaveLoad_Prefix, 0x6)
{
	GET_STACK(TechnoTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	TechnoTypeExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x716DAC, TechnoTypeClass_Load_Suffix, 0xA)
{
	TechnoTypeExt::ExtMap.LoadStatic();

	return 0;
}

DEFINE_HOOK(0x717094, TechnoTypeClass_Save_Suffix, 0x5)
{
	TechnoTypeExt::ExtMap.SaveStatic();

	return 0;
}

DEFINE_HOOK_AGAIN(0x716132, TechnoTypeClass_LoadFromINI, 0x5)
DEFINE_HOOK(0x716123, TechnoTypeClass_LoadFromINI, 0x5)
{
	GET(TechnoTypeClass*, pItem, EBP);
	GET_STACK(CCINIClass*, pINI, 0x380);

	TechnoTypeExt::ExtMap.LoadFromINI(pItem, pINI);

	return 0;
}

DEFINE_HOOK(0x679CAF, RulesClass_LoadAfterTypeData_CompleteInitialization, 0x5)
{
	//GET(CCINIClass*, pINI, ESI);

	for (auto const& pType : *BuildingTypeClass::Array)
	{
		auto const pExt = BuildingTypeExt::ExtMap.Find(pType);
		pExt->CompleteInitialization();
	}

	return 0;
}

DEFINE_HOOK(0x747E90, UnitTypeClass_LoadFromINI, 0x5)
{
	GET(UnitTypeClass*, pItem, ESI);

	if (auto pTypeExt = TechnoTypeExt::ExtMap.Find(pItem))
	{
		if (!pTypeExt->Harvester_Counted.isset() && pItem->Harvester)
		{
			pTypeExt->Harvester_Counted = true;
			auto& list = RulesExt::Global()->HarvesterTypes;
			if (!list.Contains(pItem))
				list.emplace_back(pItem);
		}
	}

	return 0;
}
