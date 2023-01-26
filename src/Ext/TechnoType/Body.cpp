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
	// Does not verify if the offset actually has all values parsed as it makes no difference, it will be 0 for the unparsed ones either way.
	auto offset = this->TurretOffset.GetEx();
	float x = static_cast<float>(offset->X * factor);
	float y = static_cast<float>(offset->Y * factor);
	float z = static_cast<float>(offset->Z * factor);

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

void TechnoTypeExt::ExtData::ReadWeapons(CCINIClass* const pINI)
{
	TechnoTypeClass* pType = OwnerObject();
	const char* pSection = pType->ID;
	const char* pArtSection = pType->ImageFile;
	char key[0x40] = { '\0' };
	INI_EX exINI(pINI);
	INI_EX exArtINI(CCINIClass::INI_Art);

	Valueable<bool> ExtendGattling;
	ExtendGattling = false;
	ExtendGattling.Read(exINI, pSection, "IsExtendGattling");

	if (pType->IsGattling || pType->Gunner || pType->IsChargeTurret || ExtendGattling)
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

			if (weapon.isset() && i < static_cast<int>(this->Weapons.Base.size()))
				this->Weapons.Base[i] = std::move(WeaponStruct(weapon, baseFLH, barrelLength, barrelThickness, turretLocked));
			else if (i >= static_cast<int>(this->Weapons.Base.size()))
				this->Weapons.Base.emplace_back(weapon, baseFLH, barrelLength, barrelThickness, turretLocked);

			if (veteran.isset())
				this->Weapons.Veteran.emplace(i, this->Weapons.Base[i]);

			if (this->Weapons.Veteran.count(i))
			{
				WeaponStruct& veteranWeapon = this->Weapons.Veteran[i];
				veteranWeapon = std::move
				(
					WeaponStruct
					(
						veteran.Get(veteranWeapon.WeaponType),
						veteranFLH.Get(baseFLH),
						veteranBarrelLength.Get(barrelLength),
						veteranBarrelThickness.Get(barrelThickness),
						veteranTurretLocked.Get(turretLocked)
					)
				);
			}

			if (elite.isset())
				this->Weapons.Elite.emplace(i, this->Weapons.Veteran.count(i) ? this->Weapons.Veteran[i] : this->Weapons.Base[i]);

			if (this->Weapons.Elite.count(i))
			{
				WeaponStruct& eliteWeapon = this->Weapons.Elite[i];
				eliteWeapon = std::move
				(
					WeaponStruct
					(
						elite.Get(eliteWeapon.WeaponType),
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
		Nullable<WeaponTypeClass*> primary;
		Nullable<WeaponTypeClass*> veteranPrimary;
		Nullable<WeaponTypeClass*> elitePrimary;
		Nullable<WeaponTypeClass*> secondary;
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

		if (primary.isset())
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

		if (primary.isset())
			this->Weapons.Base[0] = std::move(WeaponStruct(primary, primaryFLH, primaryBarrelLength, primaryBarrelThickness, false));

		if (veteranPrimary.isset())
			this->Weapons.Veteran.emplace(0, this->Weapons.Base[0]);

		if (this->Weapons.Veteran.count(0))
		{
			WeaponStruct& veteranWeapon = this->Weapons.Veteran[0];
			veteranWeapon = std::move
			(
				WeaponStruct
				(
					veteranPrimary.Get(veteranWeapon.WeaponType),
					veteranPrimaryFLH.Get(primaryFLH),
					veteranPrimaryBarrelLength.Get(primaryBarrelLength),
					veteranPrimaryBarrelThickness.Get(primaryBarrelThickness),
					false
				)
			);
		}

		if (elitePrimary.isset())
			this->Weapons.Elite.emplace(0, this->Weapons.Veteran.count(0) ? this->Weapons.Veteran[0] : this->Weapons.Base[0]);

		if (elitePrimary.isset())
		{
			WeaponStruct& eliteWeapon = this->Weapons.Elite[0];
			eliteWeapon = std::move
			(
				WeaponStruct
				(
					elitePrimary.Get(eliteWeapon.WeaponType),
					elitePrimaryFLH.Get(veteranPrimaryFLH.Get(primaryFLH)),
					elitePrimaryBarrelLength.Get(veteranPrimaryBarrelLength.Get(primaryBarrelLength)),
					elitePrimaryBarrelThickness.Get(veteranPrimaryBarrelThickness.Get(primaryBarrelThickness)),
					false
				)
			);
		}

		secondary.Read(exINI, pSection, "Secondary", true);

		if (secondary.isset())
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

		if (secondary.isset())
			this->Weapons.Base[1] = std::move(WeaponStruct(secondary, secondaryFLH, secondaryBarrelLength, secondaryBarrelThickness, false));

		if (veteranSecondary.isset())
			this->Weapons.Veteran.emplace(1, this->Weapons.Base[1]);

		if(this->Weapons.Veteran.count(1))
		{
			WeaponStruct& veteranWeapon = this->Weapons.Veteran[1];
			veteranWeapon = std::move
			(
				WeaponStruct
				(
					veteranSecondary.Get(veteranWeapon.WeaponType),
					veteranSecondaryFLH.Get(secondaryFLH),
					veteranSecondaryBarrelLength.Get(secondaryBarrelLength),
					veteranSecondaryBarrelThickness.Get(secondaryBarrelThickness),
					false
				)
			);
		}

		if (eliteSecondary.isset())
			this->Weapons.Elite.emplace(1, this->Weapons.Veteran.count(1) ? this->Weapons.Veteran[1] : this->Weapons.Base[1]);

		if(this->Weapons.Elite.count(1))
		{
			WeaponStruct& eliteWeapon = this->Weapons.Elite[1];
			eliteWeapon = std::move
			(
				WeaponStruct
				(
					eliteSecondary.Get(eliteWeapon.WeaponType),
					eliteSecondaryFLH.Get(veteranSecondaryFLH.Get(secondaryFLH)),
					eliteSecondaryBarrelLength.Get(veteranSecondaryBarrelLength.Get(secondaryBarrelLength)),
					eliteSecondaryBarrelThickness.Get(veteranSecondaryBarrelThickness.Get(secondaryBarrelThickness)),
					false
				)
			);
		}
	}


	this->UseNewWeapon.Read(exINI, pSection, "UseNewWeapon");
	this->NewWeapon_FireIndex.Read(exINI, pSection, "Weapon.FireIndex");

	//New DeployWeapon
	{
		Nullable<WeaponTypeClass*> weapon;
		weapon.Read(exINI, pSection, "DeployWeapon", true);

		Nullable<WeaponTypeClass*> veteran;
		veteran.Read(exINI, pSection, "DeployWeapon.Veteran", true);

		Nullable<WeaponTypeClass*> elite;
		elite.Read(exINI, pSection, "DeployWeapon.Elite", true);

		if (weapon.isset())
			this->NewDeployWeapon.SetAll(WeaponStruct(weapon));

		if (veteran.isset())
			this->NewDeployWeapon.Veteran.WeaponType = veteran;

		if (elite.isset())
			this->NewDeployWeapon.Elite.WeaponType = elite;
	}
	//OccupyWeapon
	{
		Nullable<WeaponTypeClass*> weapon;
		weapon.Read(exINI, pSection, "OccupyWeapon", true);

		Nullable<WeaponTypeClass*> primary;
		primary.Read(exINI, pSection, "Primary", true);

		Nullable<WeaponTypeClass*> veteran;
		veteran.Read(exINI, pSection, "VeteranOccupyWeapon", true);

		Nullable<WeaponTypeClass*> veteranprimary;
		veteranprimary.Read(exINI, pSection, "VeteranPrimary", true);

		Nullable<WeaponTypeClass*> elite;
		elite.Read(exINI, pSection, "EliteOccupyWeapon", true);

		Nullable<WeaponTypeClass*> eliteprimary;
		eliteprimary.Read(exINI, pSection, "ElitePrimary", true);

		if (weapon.isset())
			this->OccupyWeapons.SetAll(WeaponStruct(weapon));
		else if (primary.isset())
			this->OccupyWeapons.SetAll(WeaponStruct(primary));

		if (veteran.isset())
			this->OccupyWeapons.Veteran.WeaponType = veteran;
		else if (!weapon.isset())
		{
			if (veteranprimary.isset())
				this->OccupyWeapons.Veteran.WeaponType = veteranprimary;
		}

		if (elite.isset())
			this->OccupyWeapons.Elite.WeaponType = elite;
		else if (!veteran.isset())
		{
			if (!weapon.isset())
			{
				if (eliteprimary.isset())
					this->OccupyWeapons.Elite.WeaponType = eliteprimary;
				else if (veteranprimary.isset())
					this->OccupyWeapons.Elite.WeaponType = veteranprimary;
			}
		}
		else
			this->OccupyWeapons.Elite.WeaponType = veteran;
	}
	//New Weapon - Infantrys
	{
		//对地武器
		Nullable<WeaponTypeClass*> weapon;
		weapon.Read(exINI, pSection, "Weapon.Infantry", true);

		Nullable<WeaponTypeClass*> veteran;
		veteran.Read(exINI, pSection, "Weapon.Infantry.Veteran", true);

		Nullable<WeaponTypeClass*> elite;
		elite.Read(exINI, pSection, "Weapon.Infantry.Elite", true);

		Nullable<CoordStruct> flh;
		flh.Read(exArtINI, pArtSection, "Weapon.Infantry.FLH", true);

		Nullable<CoordStruct> veteranflh;
		veteranflh.Read(exArtINI, pArtSection, "Weapon.Infantry.VeteranFLH", true);

		Nullable<CoordStruct> eliteflh;
		eliteflh.Read(exArtINI, pArtSection, "Weapon.Infantry.EliteFLH", true);

		if (weapon.isset())
			this->NewWeapon_Infantry.SetAll(WeaponStruct(weapon));

		if (veteran.isset())
			this->NewWeapon_Infantry.Veteran.WeaponType = veteran;

		if (elite.isset())
			this->NewWeapon_Infantry.Elite.WeaponType = elite;

		if (flh.isset())
			this->NewWeapon_Infantry.Rookie.FLH = flh;

		if (veteranflh.isset())
			this->NewWeapon_Infantry.Veteran.FLH = veteranflh;
		else
			this->NewWeapon_Infantry.Veteran.FLH = flh;

		if (eliteflh.isset())
			this->NewWeapon_Infantry.Elite.FLH = eliteflh;
		else
			this->NewWeapon_Infantry.Elite.FLH = flh;

		//对空武器
		Nullable<WeaponTypeClass*> weaponair;
		weaponair.Read(exINI, pSection, "WeaponAir.Infantry", true);

		Nullable<WeaponTypeClass*> veteranair;
		veteranair.Read(exINI, pSection, "WeaponAir.Infantry.Veteran", true);

		Nullable<WeaponTypeClass*> eliteair;
		eliteair.Read(exINI, pSection, "WeaponAir.Infantry.Elite", true);

		Nullable<CoordStruct> airflh;
		airflh.Read(exArtINI, pArtSection, "WeaponAir.Infantry.FLH", true);

		Nullable<CoordStruct> veteranairflh;
		veteranairflh.Read(exArtINI, pArtSection, "WeaponAir.Infantry.VeteranFLH", true);

		Nullable<CoordStruct> eliteairflh;
		eliteairflh.Read(exArtINI, pArtSection, "WeaponAir.Infantry.EliteFLH", true);

		if (weaponair.isset())
			this->NewWeapon_Infantry_AIR.SetAll(WeaponStruct(weaponair));

		if (veteranair.isset())
			this->NewWeapon_Infantry_AIR.Veteran.WeaponType = veteranair;

		if (eliteair.isset())
			this->NewWeapon_Infantry_AIR.Elite.WeaponType = eliteair;

		if (airflh.isset())
			this->NewWeapon_Infantry_AIR.Rookie.FLH = airflh;

		if (veteranairflh.isset())
			this->NewWeapon_Infantry_AIR.Veteran.FLH = veteranairflh;
		else
			this->NewWeapon_Infantry_AIR.Veteran.FLH = airflh;

		if (eliteairflh.isset())
			this->NewWeapon_Infantry_AIR.Elite.FLH = eliteairflh;
		else
			this->NewWeapon_Infantry_AIR.Elite.FLH = airflh;
	}
	//New Weapon - Units
	{
		//对地武器
		Nullable<WeaponTypeClass*> weapon;
		weapon.Read(exINI, pSection, "Weapon.Unit", true);

		Nullable<WeaponTypeClass*> veteran;
		veteran.Read(exINI, pSection, "Weapon.Unit.Veteran", true);

		Nullable<WeaponTypeClass*> elite;
		elite.Read(exINI, pSection, "Weapon.Unit.Elite", true);

		Nullable<CoordStruct> flh;
		flh.Read(exArtINI, pArtSection, "Weapon.Unit.FLH", true);

		Nullable<CoordStruct> veteranflh;
		veteranflh.Read(exArtINI, pArtSection, "Weapon.Unit.VeteranFLH", true);

		Nullable<CoordStruct> eliteflh;
		eliteflh.Read(exArtINI, pArtSection, "Weapon.Unit.EliteFLH", true);

		if (weapon.isset())
			this->NewWeapon_Unit.SetAll(WeaponStruct(weapon));

		if (veteran.isset())
			this->NewWeapon_Unit.Veteran.WeaponType = veteran;

		if (elite.isset())
			this->NewWeapon_Unit.Elite.WeaponType = elite;

		if (flh.isset())
			this->NewWeapon_Unit.Rookie.FLH = flh;

		if (veteranflh.isset())
			this->NewWeapon_Unit.Veteran.FLH = veteranflh;
		else
			this->NewWeapon_Unit.Veteran.FLH = flh;

		if (eliteflh.isset())
			this->NewWeapon_Unit.Elite.FLH = eliteflh;
		else
			this->NewWeapon_Unit.Elite.FLH = flh;

		//对空武器
		Nullable<WeaponTypeClass*> weaponair;
		weaponair.Read(exINI, pSection, "WeaponAir.Unit", true);

		Nullable<WeaponTypeClass*> veteranair;
		veteranair.Read(exINI, pSection, "WeaponAir.Unit.Veteran", true);

		Nullable<WeaponTypeClass*> eliteair;
		eliteair.Read(exINI, pSection, "WeaponAir.Unit.Elite", true);

		Nullable<CoordStruct> airflh;
		airflh.Read(exArtINI, pArtSection, "WeaponAir.Unit.FLH", true);

		Nullable<CoordStruct> veteranairflh;
		veteranairflh.Read(exArtINI, pArtSection, "WeaponAir.Unit.VeteranFLH", true);

		Nullable<CoordStruct> eliteairflh;
		eliteairflh.Read(exArtINI, pArtSection, "WeaponAir.Unit.EliteFLH", true);

		if (weaponair.isset())
			this->NewWeapon_Unit_AIR.SetAll(WeaponStruct(weaponair));

		if (veteranair.isset())
			this->NewWeapon_Unit_AIR.Veteran.WeaponType = veteranair;

		if (eliteair.isset())
			this->NewWeapon_Unit_AIR.Elite.WeaponType = eliteair;

		if (airflh.isset())
			this->NewWeapon_Unit_AIR.Rookie.FLH = airflh;

		if (veteranairflh.isset())
			this->NewWeapon_Unit_AIR.Veteran.FLH = veteranairflh;
		else
			this->NewWeapon_Unit_AIR.Veteran.FLH = airflh;

		if (eliteairflh.isset())
			this->NewWeapon_Unit_AIR.Elite.FLH = eliteairflh;
		else
			this->NewWeapon_Unit_AIR.Elite.FLH = airflh;
	}
	//New Weapon - Aircrafts
	{
		//对地武器
		Nullable<WeaponTypeClass*> weapon;
		weapon.Read(exINI, pSection, "Weapon.Aircraft", true);

		Nullable<WeaponTypeClass*> veteran;
		veteran.Read(exINI, pSection, "Weapon.Aircraft.Veteran", true);

		Nullable<WeaponTypeClass*> elite;
		elite.Read(exINI, pSection, "Weapon.Aircraft.Elite", true);

		Nullable<CoordStruct> flh;
		flh.Read(exArtINI, pArtSection, "Weapon.Aircraft.FLH", true);

		Nullable<CoordStruct> veteranflh;
		veteranflh.Read(exArtINI, pArtSection, "Weapon.Aircraft.VeteranFLH", true);

		Nullable<CoordStruct> eliteflh;
		eliteflh.Read(exArtINI, pArtSection, "Weapon.Aircraft.EliteFLH", true);

		if (weapon.isset())
			this->NewWeapon_Aircraft.SetAll(WeaponStruct(weapon));

		if (veteran.isset())
			this->NewWeapon_Aircraft.Veteran.WeaponType = veteran;

		if (elite.isset())
			this->NewWeapon_Aircraft.Elite.WeaponType = elite;

		if (flh.isset())
			this->NewWeapon_Aircraft.Rookie.FLH = flh;

		if (veteranflh.isset())
			this->NewWeapon_Aircraft.Veteran.FLH = veteranflh;
		else
			this->NewWeapon_Aircraft.Veteran.FLH = flh;

		if (eliteflh.isset())
			this->NewWeapon_Aircraft.Elite.FLH = eliteflh;
		else
			this->NewWeapon_Aircraft.Elite.FLH = flh;

		//对空武器
		Nullable<WeaponTypeClass*> weaponair;
		weaponair.Read(exINI, pSection, "WeaponAir.Aircraft", true);

		Nullable<WeaponTypeClass*> veteranair;
		veteranair.Read(exINI, pSection, "WeaponAir.Aircraft.Veteran", true);

		Nullable<WeaponTypeClass*> eliteair;
		eliteair.Read(exINI, pSection, "WeaponAir.Aircraft.Elite", true);

		Nullable<CoordStruct> airflh;
		airflh.Read(exArtINI, pArtSection, "WeaponAir.Aircraft.FLH", true);

		Nullable<CoordStruct> veteranairflh;
		veteranairflh.Read(exArtINI, pArtSection, "WeaponAir.Aircraft.VeteranFLH", true);

		Nullable<CoordStruct> eliteairflh;
		eliteairflh.Read(exArtINI, pArtSection, "WeaponAir.Aircraft.EliteFLH", true);

		if (weaponair.isset())
			this->NewWeapon_Aircraft_AIR.SetAll(WeaponStruct(weaponair));

		if (veteranair.isset())
			this->NewWeapon_Aircraft_AIR.Veteran.WeaponType = veteranair;

		if (eliteair.isset())
			this->NewWeapon_Aircraft_AIR.Elite.WeaponType = eliteair;

		if (airflh.isset())
			this->NewWeapon_Aircraft_AIR.Rookie.FLH = airflh;

		if (veteranairflh.isset())
			this->NewWeapon_Aircraft_AIR.Veteran.FLH = veteranairflh;
		else
			this->NewWeapon_Aircraft_AIR.Veteran.FLH = airflh;

		if (eliteairflh.isset())
			this->NewWeapon_Aircraft_AIR.Elite.FLH = eliteairflh;
		else
			this->NewWeapon_Aircraft_AIR.Elite.FLH = airflh;
	}
	//New Weapon - Buildings
	{
		//对地武器
		Nullable<WeaponTypeClass*> weapon;
		weapon.Read(exINI, pSection, "Weapon.Building", true);

		Nullable<WeaponTypeClass*> veteran;
		veteran.Read(exINI, pSection, "Weapon.Building.Veteran", true);

		Nullable<WeaponTypeClass*> elite;
		elite.Read(exINI, pSection, "Weapon.Building.Elite", true);

		Nullable<CoordStruct> flh;
		flh.Read(exArtINI, pArtSection, "Weapon.Building.FLH", true);

		Nullable<CoordStruct> veteranflh;
		veteranflh.Read(exArtINI, pArtSection, "Weapon.Building.VeteranFLH", true);

		Nullable<CoordStruct> eliteflh;
		eliteflh.Read(exArtINI, pArtSection, "Weapon.Building.EliteFLH", true);

		if (weapon.isset())
			this->NewWeapon_Building.SetAll(WeaponStruct(weapon));

		if (veteran.isset())
			this->NewWeapon_Building.Veteran.WeaponType = veteran;

		if (elite.isset())
			this->NewWeapon_Building.Elite.WeaponType = elite;

		if (flh.isset())
			this->NewWeapon_Building.Rookie.FLH = flh;

		if (veteranflh.isset())
			this->NewWeapon_Building.Veteran.FLH = veteranflh;
		else
			this->NewWeapon_Building.Veteran.FLH = flh;

		if (eliteflh.isset())
			this->NewWeapon_Building.Elite.FLH = eliteflh;
		else
			this->NewWeapon_Building.Elite.FLH = flh;

		//建筑不会飞，所以没有这个武器。
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

	//if (pThis->WhatAmI() == AbstractType::InfantryType && static_cast<InfantryTypeClass*>(pThis)->OccupyWeapon.WeaponType != nullptr)
		//vWeapons.emplace_back(static_cast<InfantryTypeClass*>(pThis)->OccupyWeapon.WeaponType);

	//if (pExt->VeteranOccupyWeapon.Get() != nullptr)
		//vWeapons.emplace_back(pExt->VeteranOccupyWeapon);

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

	if (this->Interceptor)
	{
		if (this->InterceptorType == nullptr)
			this->InterceptorType = std::make_unique<InterceptorTypeClass>(this->OwnerObject());

		this->InterceptorType->LoadFromINI(pINI, pSection);
	}

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
		RulesExt::Global()->HarvesterTypes.AddUnique(pThis);

	this->Promote_IncludeSpawns.Read(exINI, pSection, "Promote.IncludeSpawns");
	this->ImmuneToCrit.Read(exINI, pSection, "ImmuneToCrit");
	this->MultiMindControl_ReleaseVictim.Read(exINI, pSection, "MultiMindControl.ReleaseVictim");
	this->NoManualMove.Read(exINI, pSection, "NoManualMove");

	this->InitialStrength.Read(exINI, pSection, "InitialStrength");

	if (this->InitialStrength.isset())
		this->InitialStrength = Math::clamp(this->InitialStrength, 1, pThis->Strength);

	this->AutoDeath_Behavior.Read(exINI, pSection, "AutoDeath.Behavior");
	this->AutoDeath_OnAmmoDepletion.Read(exINI, pSection, "AutoDeath.OnAmmoDepletion");
	this->AutoDeath_AfterDelay.Read(exINI, pSection, "AutoDeath.AfterDelay");
	this->AutoDeath_TechnosDontExist.Read(exINI, pSection, "AutoDeath.TechnosDontExist");
	this->AutoDeath_TechnosDontExist_Any.Read(exINI, pSection, "AutoDeath.TechnosDontExist.Any");
	this->AutoDeath_TechnosDontExist_Houses.Read(exINI, pSection, "AutoDeath.TechnosDontExist.Houses");
	this->AutoDeath_TechnosExist.Read(exINI, pSection, "AutoDeath.TechnosExist");
	this->AutoDeath_TechnosExist_Any.Read(exINI, pSection, "AutoDeath.TechnosExist.Any");
	this->AutoDeath_TechnosExist_Houses.Read(exINI, pSection, "AutoDeath.TechnosExist.Houses");
	this->AutoDeath_OnPassengerDepletion.Read(exINI, pSection, "AutoDeath.OnPassengerDepletion");
	this->AutoDeath_OnPassengerDepletion_Delay.Read(exINI, pSection, "AutoDeath.OnPassengerDepletion.Delay");

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
	this->DefaultVehicleDisguise.Read(exINI, pSection, "DefaultVehicleDisguise");
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
	this->GiftBoxData.Types.Read(exINI, pSection, "GiftBox.Types");
	this->GiftBoxData.Nums.Read(exINI, pSection, "GiftBox.Nums");
	this->GiftBoxData.Remove.Read(exINI, pSection, "GiftBox.Remove");
	this->GiftBoxData.Destroy.Read(exINI, pSection, "GiftBox.Destroy");
	this->GiftBoxData.Delay.Read(exINI, pSection, "GiftBox.Delay");
	this->GiftBoxData.DelayMinMax.Read(exINI, pSection, "GiftBox.DealyMinMax");
	this->GiftBoxData.EmptyCell.Read(exINI, pSection, "GiftBox.EmptyCell");
	this->GiftBoxData.CellRandomRange.Read(exINI, pSection, "GiftBox.CellRandomRange");
	this->GiftBoxData.RandomType.Read(exINI, pSection, "GiftBox.RandomType");
	this->GiftBoxData.ApplyOnce.Read(exINI, pSection, "GiftBox.ApplyOnce");

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
	this->ForceWeapon_Cloaked.Read(exINI, pSection, "ForceWeapon.Cloaked");
	this->ForceWeapon_Disguised.Read(exINI, pSection, "ForceWeapon.Disguised");
	this->ForceWeapon_UnderEMP.Read(exINI, pSection, "ForceWeapon.UnderEMP");

	this->Ammo_Shared.Read(exINI, pSection, "Ammo.Shared");
	this->Ammo_Shared_Group.Read(exINI, pSection, "Ammo.Shared.Group");
	this->SelfHealGainType.Read(exINI, pSection, "SelfHealGainType");
	this->Passengers_SyncOwner.Read(exINI, pSection, "Passengers.SyncOwner");
	this->Passengers_SyncOwner_RevertOnExit.Read(exINI, pSection, "Passengers.SyncOwner.RevertOnExit");

	this->IronCurtain_KeptOnDeploy.Read(exINI, pSection, "IronCurtain.KeptOnDeploy");
	this->IronCurtain_Effect.Read(exINI, pSection, "IronCurtain.Effect");
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
	this->HealthBar_PipsSHP.Read(exINI, pSection, "HealthBar.PipsSHP");
	this->HealthBar_PipsPAL.LoadFromINI(pINI, pSection, "HealthBar.PipsPAL");
	this->HealthBar_PipBrd.Read(exINI, pSection, "HealthBar.PipBrd");
	this->HealthBar_PipBrdSHP.Read(exINI, pSection, "HealthBar.PipBrdSHP");
	this->HealthBar_PipBrdPAL.LoadFromINI(pINI, pSection, "HealthBar.PipBrdPAL");
	this->HealthBar_PipBrdOffset.Read(exINI, pSection, "HealthBar.PipBrdOffset");
	this->HealthBar_XOffset.Read(exINI, pSection, "HealthBar.XOffset");
	this->UseNewHealthBar.Read(exINI, pSection, "UseNewHealthBar");
	this->HealthBar_PictureSHP.Read(exINI, pSection, "HealthBar.PictureSHP");
	this->HealthBar_PicturePAL.LoadFromINI(pINI, pSection, "HealthBar.PicturePAL");
	this->HealthBar_PictureTransparency.Read(exINI, pSection, "HealthBar.PictureTransparency");

	this->GroupID_Offset.Read(exINI, pSection, "GroupID.ShowOffset");
	this->SelfHealPips_Offset.Read(exINI, pSection, "SelfHealPips.ShowOffset");
	this->UseCustomHealthBar.Read(exINI, pSection, "UseCustomHealthBar");
	this->UseUnitHealthBar.Read(exINI, pSection, "UseUnitHealthBar");

	this->GScreenAnimType.Read(exINI, pSection, "GScreenAnimType", true);

	this->RandomProduct.Read(exINI, pSection, "RandomProduct");

	this->KickOutSW_Types.Read(exINI, pSection, "KickOutSW.Types");
	this->KickOutSW_Random.Read(exINI, pSection, "KickOutSW.Random");
	this->KickOutSW_Offset.Read(exINI, pSection, "KickOutSW.Offset");

	this->SkipOpenDoor.Read(exINI, pSection, "SkipOpenDoor");
	this->SkipOpenDoor_Offset.Read(exINI, pSection, "SkipOpenDoor.Offset");
	this->SkipOpenDoor_Anim.Read(exINI, pSection, "SkipOpenDoor.Anim");

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

	this->EMPulseCannon.Read(exINI, pSection, "EMPulseCannon");

	NullableIdx<TechnoTypeClass> convert_deploy;
	convert_deploy.Read(exINI, pSection, "Convert.Deploy");

	if (convert_deploy.isset())
		this->Convert_Deploy = TechnoTypeClass::Array->GetItem(convert_deploy);

	this->Convert_DeployAnim.Read(exINI, pSection, "Convert.DeployAnim");

	this->Gattling_SelectWeaponByVersus.Read(exINI, pSection, "Gattling.SelectWeaponByVersus");
	this->IsExtendGattling.Read(exINI, pSection, "IsExtendGattling");
	this->Gattling_Cycle.Read(exINI, pSection, "Gattling.Cycle");
	this->Gattling_Charge.Read(exINI, pSection, "Gattling.Charge");

	this->JJConvert_Unload.Read(exINI, pSection, "JumpJetConvert.Unload");

	NullableIdx<TechnoTypeClass> convert_ai;
	convert_ai.Read(exINI, pSection, "Convert.AI");

	if (convert_ai.isset())
		this->Convert_AI = TechnoTypeClass::Array->GetItem(convert_ai);

	NullableIdx<TechnoTypeClass> convert_netural;
	convert_netural.Read(exINI, pSection, "Convert.Netural");

	if (convert_netural.isset())
		this->Convert_Netural = TechnoTypeClass::Array->GetItem(convert_netural);

	NullableIdx<TechnoTypeClass> convert_player;
	convert_player.Read(exINI, pSection, "Convert.Player");

	if (convert_player.isset())
		this->Convert_Player = TechnoTypeClass::Array->GetItem(convert_player);

	NullableIdx<TechnoTypeClass> convert_stand;
	convert_stand.Read(exINI, pSection, "Convert.Stand");

	if (convert_stand.isset())
		this->Convert_Stand = TechnoTypeClass::Array->GetItem(convert_stand);

	NullableIdx<TechnoTypeClass> convert_move;
	convert_move.Read(exINI, pSection, "Convert.Move");

	if (convert_move.isset())
		this->Convert_Move = TechnoTypeClass::Array->GetItem(convert_move);

	this->CrushLevel.Read(exINI, pSection, "%sCrushLevel");
	this->CrushableLevel.Read(exINI, pSection, "%sCrushableLevel");
	this->DeployCrushableLevel.Read(exINI, pSection, "%sDeployCrushableLevel");

	// Auto Adjust CrushLevel tags
	{
		if (this->CrushLevel.Rookie <= 0)
		{
			if (pThis->OmniCrusher)
				this->CrushLevel.Rookie = 10;
			else if (pThis->Crusher)
				this->CrushLevel.Rookie = 5;
			else
				this->CrushLevel.Rookie = 0;
		}
		if (this->CrushLevel.Veteran <= 0)
		{
			if (!pThis->OmniCrusher && pThis->VeteranAbilities.CRUSHER)
				this->CrushLevel.Veteran = 5;
			else
				this->CrushLevel.Veteran = this->CrushLevel.Rookie;
		}
		if (this->CrushLevel.Elite <= 0)
		{
			if (!pThis->OmniCrusher && pThis->EliteAbilities.CRUSHER)
				this->CrushLevel.Elite = 5;
			else
				this->CrushLevel.Elite = this->CrushLevel.Veteran;
		}
		if (!pThis->Crusher && (this->CrushLevel.Rookie > 0 || this->CrushLevel.Veteran > 0 || this->CrushLevel.Elite > 0) &&
			pThis->WhatAmI() == AbstractType::UnitType)
			pThis->Crusher = true;

		if (this->CrushableLevel.Rookie <= 0)
		{
			if (pThis->OmniCrushResistant)
				this->CrushableLevel.Rookie = 10;
			else if (!pThis->Crushable)
				this->CrushableLevel.Rookie = 5;
			else
				this->CrushableLevel.Rookie = 0;
		}
		if (this->CrushableLevel.Veteran <= 0)
			this->CrushableLevel.Veteran = this->CrushableLevel.Rookie;
		if (this->CrushableLevel.Elite <= 0)
			this->CrushableLevel.Elite = this->CrushableLevel.Veteran;

		if (const auto pInfType = abstract_cast<InfantryTypeClass*>(pThis))
		{
			if (this->DeployCrushableLevel.Rookie <= 0)
			{
				if (!pInfType->DeployedCrushable)
					this->DeployCrushableLevel.Rookie = 5;
				else
					this->DeployCrushableLevel.Rookie = this->CrushableLevel.Rookie;
			}
			if (this->DeployCrushableLevel.Veteran <= 0)
				this->DeployCrushableLevel.Veteran = this->DeployCrushableLevel.Rookie;
			if (this->DeployCrushableLevel.Elite <= 0)
				this->DeployCrushableLevel.Elite = this->DeployCrushableLevel.Veteran;
		}
	}

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

	//WeaponInTransport
	{
		Nullable<WeaponTypeClass*> weapon;
		weapon.Read(exINI, pSection, "WeaponInTransport", true);

		Nullable<WeaponTypeClass*> rookie;
		rookie.Read(exINI, pSection, "WeaponInTransport.Rookie", true);

		Nullable<WeaponTypeClass*> veteran;
		veteran.Read(exINI, pSection, "WeaponInTransport.Veteran", true);

		Nullable<WeaponTypeClass*> elite;
		elite.Read(exINI, pSection, "WeaponInTransport.Elite", true);

		if (weapon.isset())
			this->WeaponInTransport.SetAll(WeaponStruct(weapon));

		if (rookie.isset())
			this->WeaponInTransport.Rookie.WeaponType = rookie;

		if (veteran.isset())
			this->WeaponInTransport.Veteran.WeaponType = veteran;

		if (elite.isset())
			this->WeaponInTransport.Elite.WeaponType = elite;
	}

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
	this->TeamAffect_Targets.Read(exINI, pSection, "TeamAffect.Targets");
	this->TeamAffect_Number.Read(exINI, pSection, "TeamAffect.Number");
	this->TeamAffect_Weapon.Read(exINI, pSection, "TeamAffect.Weapon");
	this->TeamAffect_ROF.Read(exINI, pSection, "TeamAffect.ROF");
	this->TeamAffect_LoseEfficacyWeapon.Read(exINI, pSection, "TeamAffect.LoseEfficacyWeapon");
	this->TeamAffect_LoseEfficacyROF.Read(exINI, pSection, "TeamAffect.LoseEfficacyROF");
	this->TeamAffect_Anim.Read(exINI, pSection, "TeamAffect.Anim");
	this->TeamAffect_ShareDamage.Read(exINI, pSection, "TeamAffect.ShareDamage");
	this->TeamAffect_ShareDamagePercent.Read(exINI, pSection, "TeamAffect.ShareDamagePercent");
	this->TeamAffect_MaxNumber.Read(exINI, pSection, "TeamAffect.MaxNumber");

	this->PoweredUnitBy.Read(exINI, pSection, "PoweredUnitBy");
	this->PoweredUnitBy_Any.Read(exINI, pSection, "PoweredUnitBy.Any");
	this->PoweredUnitBy_ParticleSystem.Read(exINI, pSection, "PoweredUnitBy.ParticleSystem");
	this->PoweredUnitBy_Sparkles.Read(exINI, pSection, "PoweredUnitBy.Sparkles");
	this->PoweredUnitBy_ParticleSystemXOffset.Read(exINI, pSection, "PoweredUnitBy.ParticleSystemXOffset");
	this->PoweredUnitBy_ParticleSystemYOffset.Read(exINI, pSection, "PoweredUnitBy.ParticleSystemYOffset");
	this->PoweredUnitBy_ParticleSystemSpawnDelay.Read(exINI, pSection, "PoweredUnitBy.ParticleSystemSpawnDelay");

	this->Power.Read(exINI, pSection, "Power");
	this->Powered.Read(exINI, pSection, "Powered");

	this->Locomotor_Change.Read(exINI, pSection, "LocomotorChange");
	this->Locomotor_ChangeTo.Read(exINI, pSection, "LocomotorChange.To");
	this->Locomotor_ChangeMinRange.Read(exINI, pSection, "LocomotorChange.MinRange");
	this->Locomotor_ChangeMaxRange.Read(exINI, pSection, "LocomotorChange.MaxRange");

	this->VeteranAnim.Read(exINI, pSection, "VeteranAnim");
	this->EliteAnim.Read(exINI, pSection, "EliteAnim");

	this->IsExtendAcademy.Read(exINI, pSection, "IsExtendAcademy");
	this->Academy_Immediately.Read(exINI, pSection, "Academy.Immediately");
	this->Academy_Immediately_Addition.Read(exINI, pSection, "Academy.Immediately.Addition");
	this->Academy_Powered.Read(exINI, pSection, "Academy.Powered");
	this->Academy_InfantryVeterancy.Read(exINI, pSection, "Academy.Infantry.Veterancy");
	this->Academy_VehicleVeterancy.Read(exINI, pSection, "Academy.Vehicle.Veterancy");
	this->Academy_NavalVeterancy.Read(exINI, pSection, "Academy.Naval.Veterancy");
	this->Academy_AircraftVeterancy.Read(exINI, pSection, "Academy.Aircraft.Veterancy");
	this->Academy_BuildingVeterancy.Read(exINI, pSection, "Academy.Building.Veterancy");
	this->Academy_InfantryVeterancy_Types.Read(exINI, pSection, "Academy.Infantry.Veterancy.Types");
	this->Academy_VehicleVeterancy_Types.Read(exINI, pSection, "Academy.Vehicle.Veterancy.Types");
	this->Academy_NavalVeterancy_Types.Read(exINI, pSection, "Academy.Naval.Veterancy.Types");
	this->Academy_AircraftVeterancy_Types.Read(exINI, pSection, "Academy.Aircraft.Veterancy.Types");
	this->Academy_BuildingVeterancy_Types.Read(exINI, pSection, "Academy.Building.Veterancy.Types");
	this->Academy_InfantryVeterancy_Ignore.Read(exINI, pSection, "Academy.Infantry.Veterancy.Ignore");
	this->Academy_VehicleVeterancy_Ignore.Read(exINI, pSection, "Academy.Vehicle.Veterancy.Ignore");
	this->Academy_NavalVeterancy_Ignore.Read(exINI, pSection, "Academy.Naval.Veterancy.Ignore");
	this->Academy_AircraftVeterancy_Ignore.Read(exINI, pSection, "Academy.Aircraft.Veterancy.Ignore");
	this->Academy_BuildingVeterancy_Ignore.Read(exINI, pSection, "Academy.Building.Veterancy.Ignore");
	this->Academy_InfantryVeterancy_Cumulative.Read(exINI, pSection, "Academy.Infantry.Veterancy.Cumulative");
	this->Academy_VehicleVeterancy_Cumulative.Read(exINI, pSection, "Academy.Vehicle.Veterancy.Cumulative");
	this->Academy_NavalVeterancy_Cumulative.Read(exINI, pSection, "Academy.Naval.Veterancy.Cumulative");
	this->Academy_AircraftVeterancy_Cumulative.Read(exINI, pSection, "Academy.Aircraft.Veterancy.Cumulative");
	this->Academy_BuildingVeterancy_Cumulative.Read(exINI, pSection, "Academy.Building.Veterancy.Cumulative");

	this->ReturnMoney.Read(exINI, pSection, "ReturnMoney");
	this->ReturnMoney_Percentage.Read(exINI, pSection, "ReturnMoney.Percentage");
	this->ReturnMoney_Display.Read(exINI, pSection, "ReturnMoney.Display");
	this->ReturnMoney_Display_Houses.Read(exINI, pSection, "ReturnMoney.Display.Houses");
	this->ReturnMoney_Display_Offset.Read(exINI, pSection, "ReturnMoney.Display.Offset");

	this->LimitedAttackRange.Read(exINI, pSection, "LimitedAttackRange");

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
	this->AttachEffects_Immune.Read(exINI, pSection, "AttachEffects.Immune");
	this->AttachEffects_OnlyAccept.Read(exINI, pSection, "AttachEffects.OnlyAccept");

	this->DeployAttachEffects.Read(exINI, pSection, "DeployAttachEffects");
	this->DeployAttachEffects_Delay.Read(exINI, pSection, "DeployAttachEffects.Delay");

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

	this->DirectionalArmor.Read(exINI, pSection, "DirectionalArmor");
	this->DirectionalArmor_FrontMultiplier.Read(exINI, pSection, "DirectionalArmor.FrontMultiplier");
	this->DirectionalArmor_SideMultiplier.Read(exINI, pSection, "DirectionalArmor.SideMultiplier");
	this->DirectionalArmor_BackMultiplier.Read(exINI, pSection, "DirectionalArmor.BackMultiplier");
	this->DirectionalArmor_FrontField.Read(exINI, pSection, "DirectionalArmor.FrontField");
	this->DirectionalArmor_BackField.Read(exINI, pSection, "DirectionalArmor.BackField");

	if (this->DirectionalArmor_FrontField.isset())
	{
		this->DirectionalArmor_FrontField = Math::min(this->DirectionalArmor_FrontField, 1.0f);
		this->DirectionalArmor_FrontField = Math::max(this->DirectionalArmor_FrontField, 0.0f);
	}
	if (this->DirectionalArmor_BackField.isset())
	{
		this->DirectionalArmor_BackField = Math::min(this->DirectionalArmor_BackField, 1.0f);
		this->DirectionalArmor_BackField = Math::max(this->DirectionalArmor_BackField, 0.0f);
	}

	this->Experience_KillerMultiple.Read(exINI, pSection, "Experience.KillerMultiple", 1.0f);
	this->Experience_VictimMultiple.Read(exINI, pSection, "Experience.VictimMultiple", 1.0f);

	this->Parachute_Anim.Read(exINI, pSection, "Parachute.Anim");
	this->Parachute_OpenHeight.Read(exINI, pSection, "Parachute.OpenHeight");

	this->Parasite_ParticleSystem.Read(exINI, pSection, "Parasite.ParticleSystem");
	this->Parasite_NoParticleSystem.Read(exINI, pSection, "Parasite.NoParticleSystem");
	this->Parasite_NoRock.Read(exINI, pSection, "Parasite.NoRock");
	this->Parasite_AttachEffects.Read(exINI, pSection, "Parasite.AttachEffects");

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

	this->Explodes_KillPassengers.Read(exINI, pSection, "Explodes.KillPassengers");

	this->SuperWeapon_Quick.Read(exINI, pSection, "SuperWeapon.Quick");

	// 烈葱的可建造范围扩展
	this->BaseNormal.Read(exINI, pSection, "BaseNormal");
	this->EligibileForAllyBuilding.Read(exINI, pSection, "EligibileForAllyBuilding");

	// 微观定义DrainWeapon效果
	this->DrainMoneyFrameDelay.Read(exINI, pSection, "DrainMoneyFrameDelay");
	this->DrainMoneyAmount.Read(exINI, pSection, "DrainMoneyAmount");
	this->DrainAnimationType.Read(exINI, pSection, "DrainAnimationType");
	this->DrainMoney_Display.Read(exINI, pSection, "DrainMoney.Display");
	this->DrainMoney_Display_Houses.Read(exINI, pSection, "DrainMoney.Display.Houses");
	this->DrainMoney_Display_AtFirer.Read(exINI, pSection, "DrainMoney.Display.AtFirer");
	this->DrainMoney_Display_Offset.Read(exINI, pSection, "DrainMoney.Display.Offset");

	// OnFire 拓至所有单位类型
	this->OnFire.Read(exINI, pSection, "OnFire");

	this->Line_Attack_Weapon.Read(exINI, pSection, "Line.Attack.Weapon");
	this->Line_Move_Weapon.Read(exINI, pSection, "Line.Move.Weapon");

	// Ares 0.2
	this->RadarJamRadius.Read(exINI, pSection, "RadarJamRadius");
	this->ImmuneToEMP.Read(exINI, pSection, "ImmuneToEMP");

	// Ares 0.9
	this->InhibitorRange.Read(exINI, pSection, "InhibitorRange");
	this->DesignatorRange.Read(exINI, pSection, "DesignatorRange");

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

	for (size_t i = 0; ; i++)
	{
		Nullable<CoordStruct> alternateFLH;
		sprintf_s(tempBuffer, "AlternateFLH%u", i);
		alternateFLH.Read(exArtINI, pArtSection, tempBuffer);

		// ww always read all of AlternateFLH0-5
		if (i >= 5U && !alternateFLH.isset())
			break;

		this->AlternateFLHs.size() < i
			? this->AlternateFLHs[i] = alternateFLH
			: this->AlternateFLHs.emplace_back(alternateFLH);
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
		Spawn_LimitRange
		MindControlRange
		MobileRefinery
*/
bool TechnoTypeExt::ExtData::Subset_2_Used() const
{
	return
		SilentPassenger
		|| Spawner_SameLoseTarget
		|| Spawner_LimitRange
		|| MindControlRangeLimit.Get().value > 0
		|| MobileRefinery
		;
}

/*
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
		|| Power != 0
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
		.Process(this->InterceptorType)

		.Process(this->GroupAs)
		.Process(this->RadarJamRadius)
		.Process(this->InhibitorRange)
		.Process(this->DesignatorRange)
		.Process(this->TurretOffset)

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
		.Process(this->AutoDeath_TechnosDontExist)
		.Process(this->AutoDeath_TechnosDontExist_Any)
		.Process(this->AutoDeath_TechnosDontExist_Houses)
		.Process(this->AutoDeath_TechnosExist)
		.Process(this->AutoDeath_TechnosExist_Any)
		.Process(this->AutoDeath_TechnosExist_Houses)
		.Process(this->AutoDeath_OnPassengerDepletion)
		.Process(this->AutoDeath_OnPassengerDepletion_Delay)

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
		.Process(this->DefaultVehicleDisguise)
		.Process(this->UseDisguiseMovementSpeed)

		.Process(this->WeaponBurstFLHs)
		.Process(this->VeteranWeaponBurstFLHs)
		.Process(this->EliteWeaponBurstFLHs)
		.Process(this->AlternateFLHs)

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
		.Process(this->ForceWeapon_Cloaked)
		.Process(this->ForceWeapon_Disguised)
		.Process(this->ForceWeapon_UnderEMP)

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
		.Process(this->IronCurtain_Effect)
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

		.Process(this->KickOutSW_Types)
		.Process(this->KickOutSW_Random)
		.Process(this->KickOutSW_Offset)

		.Process(this->SkipOpenDoor)
		.Process(this->SkipOpenDoor_Offset)
		.Process(this->SkipOpenDoor_Anim)

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

		.Process(this->EMPulseCannon)

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
		.Process(this->OccupyWeapons)

		.Process(this->JJConvert_Unload)

		.Process(this->Convert_AI)
		.Process(this->Convert_Netural)
		.Process(this->Convert_Player)
		.Process(this->Convert_Stand)
		.Process(this->Convert_Move)

		.Process(this->CrushLevel)
		.Process(this->CrushableLevel)
		.Process(this->DeployCrushableLevel)

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
		.Process(this->TeamAffect_Targets)
		.Process(this->TeamAffect_Number)
		.Process(this->TeamAffect_Weapon)
		.Process(this->TeamAffect_ROF)
		.Process(this->TeamAffect_LoseEfficacyWeapon)
		.Process(this->TeamAffect_LoseEfficacyROF)
		.Process(this->TeamAffect_Anim)
		.Process(this->TeamAffect_ShareDamage)
		.Process(this->TeamAffect_ShareDamagePercent)
		.Process(this->TeamAffect_MaxNumber)

		.Process(this->PoweredUnitBy)
		.Process(this->PoweredUnitBy_Any)
		.Process(this->PoweredUnitBy_Sparkles)
		.Process(this->PoweredUnitBy_ParticleSystem)
		.Process(this->PoweredUnitBy_ParticleSystemXOffset)
		.Process(this->PoweredUnitBy_ParticleSystemYOffset)
		.Process(this->PoweredUnitBy_ParticleSystemSpawnDelay)

		.Process(this->Power)
		.Process(this->Powered)

		.Process(this->Locomotor_Change)
		.Process(this->Locomotor_ChangeTo)
		.Process(this->Locomotor_ChangeMinRange)
		.Process(this->Locomotor_ChangeMaxRange)

		.Process(this->PassengerHeal_Rate)
		.Process(this->PassengerHeal_Amount)
		.Process(this->PassengerHeal_HealAll)
		.Process(this->PassengerHeal_Sound)
		.Process(this->PassengerHeal_Anim)
		.Process(this->PassengerHeal_Houses)

		.Process(this->EVA_Sold)

		.Process(this->AttachEffects)
		.Process(this->AttachEffects_Immune)
		.Process(this->AttachEffects_OnlyAccept)

		.Process(this->DeployAttachEffects)
		.Process(this->DeployAttachEffects_Delay)

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

		.Process(this->IsExtendAcademy)
		.Process(this->Academy_Immediately)
		.Process(this->Academy_Immediately_Addition)
		.Process(this->Academy_Powered)
		.Process(this->Academy_InfantryVeterancy)
		.Process(this->Academy_VehicleVeterancy)
		.Process(this->Academy_NavalVeterancy)
		.Process(this->Academy_AircraftVeterancy)
		.Process(this->Academy_BuildingVeterancy)
		.Process(this->Academy_InfantryVeterancy_Types)
		.Process(this->Academy_VehicleVeterancy_Types)
		.Process(this->Academy_NavalVeterancy_Types)
		.Process(this->Academy_AircraftVeterancy_Types)
		.Process(this->Academy_BuildingVeterancy_Types)
		.Process(this->Academy_InfantryVeterancy_Ignore)
		.Process(this->Academy_VehicleVeterancy_Ignore)
		.Process(this->Academy_NavalVeterancy_Ignore)
		.Process(this->Academy_AircraftVeterancy_Ignore)
		.Process(this->Academy_BuildingVeterancy_Ignore)
		.Process(this->Academy_InfantryVeterancy_Cumulative)
		.Process(this->Academy_VehicleVeterancy_Cumulative)
		.Process(this->Academy_NavalVeterancy_Cumulative)
		.Process(this->Academy_AircraftVeterancy_Cumulative)
		.Process(this->Academy_BuildingVeterancy_Cumulative)

		.Process(this->ReturnMoney)
		.Process(this->ReturnMoney_Percentage)
		.Process(this->ReturnMoney_Display)
		.Process(this->ReturnMoney_Display_Houses)
		.Process(this->ReturnMoney_Display_Offset)

		.Process(this->LimitedAttackRange)

		.Process(this->AllowPlanningMode)

		.Process(this->PassengerProduct)
		.Process(this->PassengerProduct_Type)
		.Process(this->PassengerProduct_Rate)
		.Process(this->PassengerProduct_Amount)
		.Process(this->PassengerProduct_RandomPick)

		.Process(this->DirectionalArmor)
		.Process(this->DirectionalArmor_FrontMultiplier)
		.Process(this->DirectionalArmor_SideMultiplier)
		.Process(this->DirectionalArmor_BackMultiplier)
		.Process(this->DirectionalArmor_FrontField)
		.Process(this->DirectionalArmor_BackField)

		.Process(this->Experience_KillerMultiple)
		.Process(this->Experience_VictimMultiple)
		.Process(this->Explodes_KillPassengers)

		.Process(this->Parachute_Anim)
		.Process(this->Parachute_OpenHeight)

		.Process(this->Parasite_ParticleSystem)
		.Process(this->Parasite_NoParticleSystem)
		.Process(this->Parasite_NoRock)
		.Process(this->Parasite_AttachEffects)

		.Process(this->UseNewWeapon)
		.Process(this->NewWeapon_FireIndex)
		.Process(this->NewDeployWeapon)
		.Process(this->NewWeapon_Infantry)
		.Process(this->NewWeapon_Infantry_AIR)
		.Process(this->NewWeapon_Unit)
		.Process(this->NewWeapon_Unit_AIR)
		.Process(this->NewWeapon_Aircraft)
		.Process(this->NewWeapon_Aircraft_AIR)
		.Process(this->NewWeapon_Building)

		.Process(this->Passengers_BySize)
		.Process(this->ImmuneToEMP)

		.Process(this->SuperWeapon_Quick)

		.Process(this->BaseNormal)
		.Process(this->EligibileForAllyBuilding)

		.Process(this->DrainMoneyFrameDelay)
		.Process(this->DrainMoneyAmount)
		.Process(this->DrainAnimationType)
		.Process(this->DrainMoney_Display)
		.Process(this->DrainMoney_Display_Houses)
		.Process(this->DrainMoney_Display_AtFirer)
		.Process(this->DrainMoney_Display_Offset)

		.Process(this->OnFire)

		.Process(this->Line_Attack_Weapon)
		.Process(this->Line_Move_Weapon)
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
		.Process(this->Types)
		.Process(this->Nums)
		.Process(this->Remove)
		.Process(this->Destroy)
		.Process(this->Delay)
		.Process(this->DelayMinMax)
		.Process(this->CellRandomRange)
		.Process(this->EmptyCell)
		.Process(this->RandomType)
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
			RulesExt::Global()->HarvesterTypes.AddUnique(pItem);
		}
	}

	return 0;
}
