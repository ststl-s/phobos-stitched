#include "Body.h"

#include <SuperClass.h>
#include <VocClass.h>

#include <Helpers/Macro.h>

#include <Ext/House/Body.h>
#include <Ext/SWType/Body.h>

#include <Utilities/GeneralUtils.h>
#include <Utilities/TemplateDef.h>

template<> const DWORD Extension<BuildingTypeClass>::Canary = 0x11111111;
BuildingTypeExt::ExtContainer BuildingTypeExt::ExtMap;

int BuildingTypeExt::ExtData::GetSuperWeaponCount() const
{
	// 2 = SuperWeapon & SuperWeapon2
	return 2 + this->SuperWeapons.size();
}

int BuildingTypeExt::ExtData::GetSuperWeaponIndex(const int index, HouseClass* pHouse) const
{
	auto idxSW = this->GetSuperWeaponIndex(index);

	if (auto pSuper = pHouse->Supers.GetItemOrDefault(idxSW))
	{
		auto pExt = SWTypeExt::ExtMap.Find(pSuper->Type);

		if (!pExt->IsAvailable(pHouse))
			return -1;
	}

	return idxSW;
}

int BuildingTypeExt::ExtData::GetSuperWeaponIndex(const int index) const
{
	const auto pThis = this->OwnerObject();

	// 2 = SuperWeapon & SuperWeapon2
	if (index < 2)
		return !index ? pThis->SuperWeapon : pThis->SuperWeapon2;
	else if (index - 2 < static_cast<int>(this->SuperWeapons.size()))
		return this->SuperWeapons[index - 2];

	return -1;
}

int BuildingTypeExt::GetEnhancedPower(BuildingClass* pBuilding, HouseClass* pHouse)
{
	int nAmount = 0;
	double fFactor = 1.0f;

	auto const pHouseExt = HouseExt::ExtMap.Find(pHouse);

	for (const auto pair : pHouseExt->BuildingCounter)
	{
		const auto& pExt = pair.first;
		const auto& nCount = pair.second;
		if (pExt->PowerPlantEnhancer_Buildings.Contains(pBuilding->Type))
		{
			fFactor *= std::pow(pExt->PowerPlantEnhancer_Factor.Get(1.0f), nCount);
			nAmount += pExt->PowerPlantEnhancer_Amount.Get(0) * nCount;
		}
	}

	return static_cast<int>(std::round(pBuilding->GetPowerOutput() * fFactor)) + nAmount;
}

void BuildingTypeExt::PlayBunkerSound(BuildingClass const* pThis, bool bUp)
{
	auto nSound = bUp ? RulesClass::Instance->BunkerWallsUpSound : RulesClass::Instance->BunkerWallsDownSound;

	if (auto const TypeExt = BuildingTypeExt::ExtMap.Find(pThis->Type))
		nSound = bUp ? TypeExt->BunkerWallsUpSound.Get(nSound) : TypeExt->BunkerWallsDownSound.Get(nSound);

	if (nSound != -1)
		VocClass::PlayAt(nSound, pThis->Location);
}

int BuildingTypeExt::GetUpgradesAmount(BuildingTypeClass* pBuilding, HouseClass* pHouse) // not including producing upgrades
{
	int result = 0;
	bool isUpgrade = false;
	auto pPowersUp = pBuilding->PowersUpBuilding;

	auto checkUpgrade = [pHouse, pBuilding, &result, &isUpgrade](BuildingTypeClass* pTPowersUp)
	{
		isUpgrade = true;
		for (auto const& pBld : pHouse->Buildings)
		{
			if (pBld->Type == pTPowersUp)
			{
				for (auto const& pUpgrade : pBld->Upgrades)
				{
					if (pUpgrade == pBuilding)
						++result;
				}
			}
		}
	};

	if (pPowersUp[0])
	{
		if (auto const pTPowersUp = BuildingTypeClass::Find(pPowersUp))
			checkUpgrade(pTPowersUp);
	}

	if (auto pBuildingExt = BuildingTypeExt::ExtMap.Find(pBuilding))
	{
		for (auto pTPowersUp : pBuildingExt->PowersUp_Buildings)
			checkUpgrade(pTPowersUp);
	}

	return isUpgrade ? result : -1;
}

void BuildingTypeExt::ExtData::Initialize()
{
	this->PrismForwarding.Initialize(this->OwnerObject());
}

// =============================
// load / save

void BuildingTypeExt::ExtData::LoadFromINIFile(CCINIClass* const pINI)
{
	auto pThis = this->OwnerObject();
	const char* pSection = pThis->ID;
	const char* pArtSection = pThis->ImageFile;
	auto pArtINI = &CCINIClass::INI_Art();

	if (!pINI->GetSection(pSection))
		return;

	INI_EX exINI(pINI);
	INI_EX exArtINI(pArtINI);

	this->InitialStrength_Cloning.Read(exINI, pSection, "InitialStrength.Cloning");

	this->PowersUp_Owner.Read(exINI, pSection, "PowersUp.Owner");
	this->PowersUp_Buildings.Read(exINI, pSection, "PowersUp.Buildings");
	this->PowerPlantEnhancer_Buildings.Read(exINI, pSection, "PowerPlantEnhancer.PowerPlants");
	this->PowerPlantEnhancer_Amount.Read(exINI, pSection, "PowerPlantEnhancer.Amount");
	this->PowerPlantEnhancer_Factor.Read(exINI, pSection, "PowerPlantEnhancer.Factor");
	this->Powered_KillSpawns.Read(exINI, pSection, "Powered.KillSpawns");

	if (pThis->PowersUpBuilding[0] == NULL && this->PowersUp_Buildings.size() > 0)
		strcpy_s(pThis->PowersUpBuilding, this->PowersUp_Buildings[0]->ID);

	this->AllowAirstrike.Read(exINI, pSection, "AllowAirstrike");

	this->Grinding_AllowAllies.Read(exINI, pSection, "Grinding.AllowAllies");
	this->Grinding_AllowOwner.Read(exINI, pSection, "Grinding.AllowOwner");
	this->Grinding_AllowTypes.Read(exINI, pSection, "Grinding.AllowTypes");
	this->Grinding_DisallowTypes.Read(exINI, pSection, "Grinding.DisallowTypes");
	this->Grinding_Sound.Read(exINI, pSection, "Grinding.Sound");
	this->Grinding_PlayDieSound.Read(exINI, pSection, "Grinding.PlayDieSound");
	this->Grinding_Weapon.Read(exINI, pSection, "Grinding.Weapon", true);

	if (SuperWeaponTypeClass::Array->Count > 0)
		this->SuperWeapons.Read(exINI, pSection, "SuperWeapons");

	if (pThis->MaxNumberOccupants > 10)
	{
		char tempBuffer[32];
		this->OccupierMuzzleFlashes.Clear();
		this->OccupierMuzzleFlashes.Reserve(pThis->MaxNumberOccupants);

		for (int i = 0; i < pThis->MaxNumberOccupants; ++i)
		{
			Nullable<Point2D> nMuzzleLocation;
			_snprintf_s(tempBuffer, sizeof(tempBuffer), "MuzzleFlash%d", i);
			nMuzzleLocation.Read(exArtINI, pArtSection, tempBuffer);
			this->OccupierMuzzleFlashes[i] = nMuzzleLocation.Get(Point2D::Empty);
		}
	}

	this->Refinery_UseStorage.Read(exINI, pSection, "Refinery.UseStorage");


	this->PackupSound_PlayGlobal.Read(exINI, pSection, "PackupSoundPlayGlobal");
	this->DisableDamageSound.Read(exINI, pSection, "DisableDamagedSound");

	this->BuildingOccupyDamageMult.Read(exINI, pSection, "OccupyDamageMultiplier");
	this->BuildingOccupyROFMult.Read(exINI, pSection, "OccupyROFMultiplier");

	this->BuildingBunkerDamageMult.Read(exINI, pSection, "BunkerDamageMultiplier");
	this->BuildingBunkerROFMult.Read(exINI, pSection, "BunkerROFMultMultiplier");

	this->BunkerWallsUpSound.Read(exINI, pSection, "BunkerWallsUpSound");
	this->BunkerWallsDownSound.Read(exINI, pSection, "BunkerWallsDownSound");

	this->Power_DegradeWithHealth.Read(exINI, pSection, "Power.DegradeWithHealth");

	this->Factory_ExplicitOnly.Read(exINI, pSection, "Factory.ExplicitOnly");

	// PlacementPreview
	{
		this->PlacementPreview.Read(exINI, pSection, "PlacementPreview");
		this->PlacementPreview_Shape.Read(exINI, pSection, "PlacementPreview.Shape");
		this->PlacementPreview_ShapeFrame.Read(exINI, pSection, "PlacementPreview.ShapeFrame");
		this->PlacementPreview_Offset.Read(exINI, pSection, "PlacementPreview.Offset");
		this->PlacementPreview_Remap.Read(exINI, pSection, "PlacementPreview.Remap");
		this->PlacementPreview_Palette.LoadFromINI(pINI, pSection, "PlacementPreview.Palette");
		this->PlacementPreview_Translucency.Read(exINI, pSection, "PlacementPreview.Translucency");
	}
	this->SpyEffect_Custom.Read(exINI, pSection, "SpyEffect.Custom");
	this->SpyEffect_VictimSuperWeapon.Read(exINI, pSection, "SpyEffect.VictimSuperWeapon");
	this->SpyEffect_InfiltratorSuperWeapon.Read(exINI, pSection, "SpyEffect.InfiltratorSuperWeapon");

	this->SpyEffect_StolenMoneyAmount.Read(exINI, pSection, "SpyEffect.StolenMoneyAmount");
	this->SpyEffect_StolenMoneyPercentage.Read(exINI, pSection, "SpyEffect.StolenMoneyPercentage");
	this->SpyEffect_StolenMoneyDisplay.Read(exINI, pSection, "SpyEffect.StolenMoneyDisplay");
	this->SpyEffect_StolenMoneyDisplay_Houses.Read(exINI, pSection, "SpyEffect.StolenMoneyDisplay.Houses");
	this->SpyEffect_StolenMoneyDisplay_Offset.Read(exINI, pSection, "SpyEffect.StolenMoneyDisplay.Offset");

	this->SpyEffect_BuildingOfflineDuration.Read(exINI, pSection, "SpyEffect.BuildingOfflineDuration");

	this->SpyEffect_InfantryDeactivate_Duration.Read(exINI, pSection, "SpyEffect.InfantryDeactivate.Duration");
	this->SpyEffect_InfantryDeactivate_Types.Read(exINI, pSection, "SpyEffect.InfantryDeactivate.Types");
	this->SpyEffect_InfantryDeactivate_Ignore.Read(exINI, pSection, "SpyEffect.InfantryDeactivate.Ignore");
	this->SpyEffect_VehicleDeactivate_Duration.Read(exINI, pSection, "SpyEffect.VehicleDeactivate.Duration");
	this->SpyEffect_VehicleDeactivate_Types.Read(exINI, pSection, "SpyEffect.VehicleDeactivate.Types");
	this->SpyEffect_VehicleDeactivate_Ignore.Read(exINI, pSection, "SpyEffect.VehicleDeactivate.Ignore");
	this->SpyEffect_NavalDeactivate_Duration.Read(exINI, pSection, "SpyEffect.NavalDeactivate.Duration");
	this->SpyEffect_NavalDeactivate_Types.Read(exINI, pSection, "SpyEffect.NavalDeactivate.Types");
	this->SpyEffect_NavalDeactivate_Ignore.Read(exINI, pSection, "SpyEffect.NavalDeactivate.Ignore");
	this->SpyEffect_AircraftDeactivate_Duration.Read(exINI, pSection, "SpyEffect.AircraftDeactivate.Duration");
	this->SpyEffect_AircraftDeactivate_Types.Read(exINI, pSection, "SpyEffect.AircraftDeactivate.Types");
	this->SpyEffect_AircraftDeactivate_Ignore.Read(exINI, pSection, "SpyEffect.AircraftDeactivate.Ignore");
	this->SpyEffect_BuildingDeactivate_Duration.Read(exINI, pSection, "SpyEffect.BuildingDeactivate.Duration");
	this->SpyEffect_BuildingDeactivate_Types.Read(exINI, pSection, "SpyEffect.BuildingDeactivate.Types");
	this->SpyEffect_BuildingDeactivate_Ignore.Read(exINI, pSection, "SpyEffect.BuildingDeactivate.Ignore");
	this->SpyEffect_DefenseDeactivate_Duration.Read(exINI, pSection, "SpyEffect.DefenseDeactivate.Duration");
	this->SpyEffect_DefenseDeactivate_Types.Read(exINI, pSection, "SpyEffect.DefenseDeactivate.Types");
	this->SpyEffect_DefenseDeactivate_Ignore.Read(exINI, pSection, "SpyEffect.DefenseDeactivate.Ignore");

	this->SpyEffect_Veterancy_AffectBuildingOwner.Read(exINI, pSection, "SpyEffect.Veterancy.AffectBuildingOwner");
	this->SpyEffect_InfantryVeterancy.Read(exINI, pSection, "SpyEffect.Veterancy.Infantry");
	this->SpyEffect_VehicleVeterancy.Read(exINI, pSection, "SpyEffect.Veterancy.Vehicle");
	this->SpyEffect_NavalVeterancy.Read(exINI, pSection, "SpyEffect.Veterancy.Naval");
	this->SpyEffect_AircraftVeterancy.Read(exINI, pSection, "SpyEffect.Veterancy.Aircraft");
	this->SpyEffect_BuildingVeterancy.Read(exINI, pSection, "SpyEffect.Veterancy.Building");
	this->SpyEffect_InfantryVeterancy_Types.Read(exINI, pSection, "SpyEffect.Veterancy.Infantry.Types");
	this->SpyEffect_VehicleVeterancy_Types.Read(exINI, pSection, "SpyEffect.Veterancy.Vehicle.Types");
	this->SpyEffect_NavalVeterancy_Types.Read(exINI, pSection, "SpyEffect.Veterancy.Naval.Types");
	this->SpyEffect_AircraftVeterancy_Types.Read(exINI, pSection, "SpyEffect.Veterancy.Aircraft.Types");
	this->SpyEffect_BuildingVeterancy_Types.Read(exINI, pSection, "SpyEffect.Veterancy.Building.Types");
	this->SpyEffect_InfantryVeterancy_Ignore.Read(exINI, pSection, "SpyEffect.Veterancy.Infantry.Ignore");
	this->SpyEffect_VehicleVeterancy_Ignore.Read(exINI, pSection, "SpyEffect.Veterancy.Vehicle.Ignore");
	this->SpyEffect_NavalVeterancy_Ignore.Read(exINI, pSection, "SpyEffect.Veterancy.Naval.Ignore");
	this->SpyEffect_AircraftVeterancy_Ignore.Read(exINI, pSection, "SpyEffect.Veterancy.Aircraft.Ignore");
	this->SpyEffect_BuildingVeterancy_Ignore.Read(exINI, pSection, "SpyEffect.Veterancy.Building.Ignore");
	this->SpyEffect_InfantryVeterancy_Cumulative.Read(exINI, pSection, "SpyEffect.Veterancy.Infantry.Cumulative");
	this->SpyEffect_VehicleVeterancy_Cumulative.Read(exINI, pSection, "SpyEffect.Veterancy.Vehicle.Cumulative");
	this->SpyEffect_NavalVeterancy_Cumulative.Read(exINI, pSection, "SpyEffect.Veterancy.Naval.Cumulative");
	this->SpyEffect_AircraftVeterancy_Cumulative.Read(exINI, pSection, "SpyEffect.Veterancy.Aircraft.Cumulative");
	this->SpyEffect_BuildingVeterancy_Cumulative.Read(exINI, pSection, "SpyEffect.Veterancy.Building.Cumulative");
	this->SpyEffect_InfantryVeterancy_Reset.Read(exINI, pSection, "SpyEffect.Veterancy.Infantry.Reset");
	this->SpyEffect_VehicleVeterancy_Reset.Read(exINI, pSection, "SpyEffect.Veterancy.Vehicle.Reset");
	this->SpyEffect_NavalVeterancy_Reset.Read(exINI, pSection, "SpyEffect.Veterancy.Naval.Reset");
	this->SpyEffect_AircraftVeterancy_Reset.Read(exINI, pSection, "SpyEffect.Veterancy.Aircraft.Reset");
	this->SpyEffect_BuildingVeterancy_Reset.Read(exINI, pSection, "SpyEffect.Veterancy.Building.Reset");

	this->SpyEffect_SabotageDelay.Read(exINI, pSection, "SpyEffect.SabotageDelay");
	this->SpyEffect_SellDelay.Read(exINI, pSection, "SpyEffect.SellDelay");
	this->SpyEffect_CaptureDelay.Read(exINI, pSection, "SpyEffect.CaptureDelay");
	this->SpyEffect_CaptureCount.Read(exINI, pSection, "SpyEffect.CaptureCount");

	this->SpyEffect_SuperWeaponTypes.Read(exINI, pSection, "SpyEffect.SuperWeaponTypes");
	this->SpyEffect_SuperWeaponTypes_Permanent.Read(exINI, pSection, "SpyEffect.SuperWeaponTypes.Permanent");
	this->SpyEffect_SuperWeaponTypes_AffectOwned.Read(exINI, pSection, "SpyEffect.SuperWeaponTypes.AffectOwned");
	this->SpyEffect_SuperWeaponTypes_Delay.Read(exINI, pSection, "SpyEffect.SuperWeaponTypes.Delay");

	this->SpyEffect_RechargeSuperWeaponTypes.Read(exINI, pSection, "SpyEffect.RechargeSuperWeaponTypes");
	this->SpyEffect_RechargeSuperWeaponTypes_Duration.Read(exINI, pSection, "SpyEffect.RechargeSuperWeaponTypes.Duration");
	this->SpyEffect_RechargeSuperWeaponTypes_SetPercentage.Read(exINI, pSection, "SpyEffect.RechargeSuperWeaponTypes.SetPercentage");
	this->SpyEffect_RechargeSuperWeaponTypes_CumulativeCount.Read(exINI, pSection, "SpyEffect.RechargeSuperWeaponTypes.CumulativeCount");

	this->SpyEffect_RevealSightDuration.Read(exINI, pSection, "SpyEffect.RevealSightDuration");
	this->SpyEffect_RevealSightRange.Read(exINI, pSection, "SpyEffect.RevealSightRange");
	this->SpyEffect_RevealSightPermanent.Read(exINI, pSection, "SpyEffect.RevealSightPermanent");

	this->SpyEffect_RadarJamDuration.Read(exINI, pSection, "SpyEffect.RadarJamDuration");

	this->SpyEffect_PowerOutageDuration.Read(exINI, pSection, "SpyEffect.PowerOutageDuration");

	this->SpyEffect_GapRadarDuration.Read(exINI, pSection, "SpyEffect.GapRadarDuration");
	this->SpyEffect_RevealRadarSightDuration.Read(exINI, pSection, "SpyEffect.RevealRadarSightDuration");
	this->SpyEffect_RevealRadarSightPermanent.Read(exINI, pSection, "SpyEffect.RevealRadarSightPermanent");
	this->SpyEffect_KeepRevealRadarSight.Read(exINI, pSection, "SpyEffect.KeepRevealRadarSight");
	this->SpyEffect_RevealRadarSight_Infantry.Read(exINI, pSection, "SpyEffect.RevealRadarSight.Infantry");
	this->SpyEffect_RevealRadarSight_Unit.Read(exINI, pSection, "SpyEffect.RevealRadarSight.Unit");
	this->SpyEffect_RevealRadarSight_Aircraft.Read(exINI, pSection, "SpyEffect.RevealRadarSight.Aircraft");
	this->SpyEffect_RevealRadarSight_Building.Read(exINI, pSection, "SpyEffect.RevealRadarSight.Building");

	this->SpyEffect_CostBonus_AffectBuildingOwner.Read(exINI, pSection, "SpyEffect.CostBonus.AffectBuildingOwner");
	this->SpyEffect_InfantryCostBonus.Read(exINI, pSection, "SpyEffect.CostBonus.Infantry");
	this->SpyEffect_UnitsCostBonus.Read(exINI, pSection, "SpyEffect.CostBonus.Units");
	this->SpyEffect_NavalCostBonus.Read(exINI, pSection, "SpyEffect.CostBonus.Naval");
	this->SpyEffect_AircraftCostBonus.Read(exINI, pSection, "SpyEffect.CostBonus.Aircraft");
	this->SpyEffect_BuildingsCostBonus.Read(exINI, pSection, "SpyEffect.CostBonus.Buildings");
	this->SpyEffect_DefensesCostBonus.Read(exINI, pSection, "SpyEffect.CostBonus.Defenses");
	this->SpyEffect_InfantryCostBonus_Types.Read(exINI, pSection, "SpyEffect.CostBonus.Infantry.Types");
	this->SpyEffect_UnitsCostBonus_Types.Read(exINI, pSection, "SpyEffect.CostBonus.Units.Types");
	this->SpyEffect_NavalCostBonus_Types.Read(exINI, pSection, "SpyEffect.CostBonus.Naval.Types");
	this->SpyEffect_AircraftCostBonus_Types.Read(exINI, pSection, "SpyEffect.CostBonus.Aircraft.Types");
	this->SpyEffect_BuildingsCostBonus_Types.Read(exINI, pSection, "SpyEffect.CostBonus.Buildings.Types");
	this->SpyEffect_DefensesCostBonus_Types.Read(exINI, pSection, "SpyEffect.CostBonus.Defenses.Types");
	this->SpyEffect_InfantryCostBonus_Ignore.Read(exINI, pSection, "SpyEffect.CostBonus.Infantry.Ignore");
	this->SpyEffect_UnitsCostBonus_Ignore.Read(exINI, pSection, "SpyEffect.CostBonus.Units.Ignore");
	this->SpyEffect_NavalCostBonus_Ignore.Read(exINI, pSection, "SpyEffect.CostBonus.Naval.Ignore");
	this->SpyEffect_AircraftCostBonus_Ignore.Read(exINI, pSection, "SpyEffect.CostBonus.Aircraft.Ignore");
	this->SpyEffect_BuildingsCostBonus_Ignore.Read(exINI, pSection, "SpyEffect.CostBonus.Buildings.Ignore");
	this->SpyEffect_DefensesCostBonus_Ignore.Read(exINI, pSection, "SpyEffect.CostBonus.Defenses.Ignore");
	this->SpyEffect_InfantryCostBonus_Reset.Read(exINI, pSection, "SpyEffect.CostBonus.Infantry.Reset");
	this->SpyEffect_UnitsCostBonus_Reset.Read(exINI, pSection, "SpyEffect.CostBonus.Units.Reset");
	this->SpyEffect_NavalCostBonus_Reset.Read(exINI, pSection, "SpyEffect.CostBonus.Naval.Reset");
	this->SpyEffect_AircraftCostBonus_Reset.Read(exINI, pSection, "SpyEffect.CostBonus.Aircraft.Reset");
	this->SpyEffect_BuildingsCostBonus_Reset.Read(exINI, pSection, "SpyEffect.CostBonus.Buildings.Reset");
	this->SpyEffect_DefensesCostBonus_Reset.Read(exINI, pSection, "SpyEffect.CostBonus.Defenses.Reset");
	this->SpyEffect_InfantryCostBonus_Max.Read(exINI, pSection, "SpyEffect.CostBonus.Infantry.Max");
	this->SpyEffect_UnitsCostBonus_Max.Read(exINI, pSection, "SpyEffect.CostBonus.Units.Max");
	this->SpyEffect_NavalCostBonus_Max.Read(exINI, pSection, "SpyEffect.CostBonus.Naval.Max");
	this->SpyEffect_AircraftCostBonus_Max.Read(exINI, pSection, "SpyEffect.CostBonus.Aircraft.Max");
	this->SpyEffect_BuildingsCostBonus_Max.Read(exINI, pSection, "SpyEffect.CostBonus.Buildings.Max");
	this->SpyEffect_DefensesCostBonus_Max.Read(exINI, pSection, "SpyEffect.CostBonus.Defenses.Max");
	this->SpyEffect_InfantryCostBonus_Min.Read(exINI, pSection, "SpyEffect.CostBonus.Infantry.Min");
	this->SpyEffect_UnitsCostBonus_Min.Read(exINI, pSection, "SpyEffect.CostBonus.Units.Min");
	this->SpyEffect_NavalCostBonus_Min.Read(exINI, pSection, "SpyEffect.CostBonus.Naval.Min");
	this->SpyEffect_AircraftCostBonus_Min.Read(exINI, pSection, "SpyEffect.CostBonus.Aircraft.Min");
	this->SpyEffect_BuildingsCostBonus_Min.Read(exINI, pSection, "SpyEffect.CostBonus.Buildings.Min");
	this->SpyEffect_DefensesCostBonus_Min.Read(exINI, pSection, "SpyEffect.CostBonus.Defenses.Min");

	this->SpyEffect_PurifierBonus_AffectBuildingOwner.Read(exINI, pSection, "SpyEffect.PurifierBonus.AffectBuildingOwner");
	this->SpyEffect_PurifierBonus.Read(exINI, pSection, "SpyEffect.PurifierBonus");
	this->SpyEffect_PurifierBonus_Reset.Read(exINI, pSection, "SpyEffect.PurifierBonus.Reset");

	this->SpyEffect_Anim.Read(exINI, pSection, "SpyEffect.Anim");
	this->SpyEffect_Anim_Duration.Read(exINI, pSection, "SpyEffect.Anim.Duration");
	this->SpyEffect_Anim_DisplayHouses.Read(exINI, pSection, "SpyEffect.Anim.DisplayHouses");

	char tempBuffer[32];
	for (size_t i = 0; ; ++i)
	{
		Nullable<CSFText> text;
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "SpyEffect.Message%d", i);
		text.Read(exINI, pSection, tempBuffer);

		if (!text.isset())
			break;

		Nullable<ShowMessageHouse> house;
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "SpyEffect.Message%d.ShowOwner", i);
		house.Read(exINI, pSection, tempBuffer);

		if (!house.isset())
			house = ShowMessageHouse::All;

		Nullable<ShowMessageHouse> color;
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "SpyEffect.Message%d.ColorType", i);
		color.Read(exINI, pSection, tempBuffer);

		if (!color.isset())
			color = ShowMessageHouse::All;

		this->SpyEffect_Messages.push_back(text);
		this->SpyEffect_Message_ShowOwners.push_back(house);
		this->SpyEffect_Message_ColorTypes.push_back(color);
	}

	this->RallyRange.Read(exINI, pSection, "RallyRange");

	this->SellWeapon.Read(exINI, pSection, "SellWeapon");

	this->Overpower_KeepOnline.Read(exINI, pSection, "Overpower.KeepOnline");
	this->Overpower_ChargeWeapon.Read(exINI, pSection, "Overpower.ChargeWeapon");
	for (size_t i = 0; ; ++i)
	{
		Nullable<int> level;
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "Overpower.Level%d", i);
		level.Read(exINI, pSection, tempBuffer);

		if (!level.isset())
			break;

		Promotable<WeaponStruct> weapon;

		Nullable<WeaponTypeClass*> weapontype;
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "Overpower.Level%d.Weapon", i);
		weapontype.Read(exINI, pSection, tempBuffer);

		Nullable<WeaponTypeClass*> veteranweapontype;
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "Overpower.Level%d.VeteranWeapon", i);
		veteranweapontype.Read(exINI, pSection, tempBuffer);

		Nullable<WeaponTypeClass*> eliteweapontype;
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "Overpower.Level%d.EliteWeapon", i);
		eliteweapontype.Read(exINI, pSection, tempBuffer);

		Nullable<CoordStruct> weaponflh;
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "Overpower.Level%d.FLH", i);
		weaponflh.Read(exINI, pSection, tempBuffer);

		Nullable<CoordStruct>  veteranweaponflh;
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "Overpower.Level%d.VeteranFLH", i);
		veteranweaponflh.Read(exINI, pSection, tempBuffer);

		Nullable<CoordStruct> eliteweaponflh;
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "Overpower.Level%d.EliteFLH", i);
		eliteweaponflh.Read(exINI, pSection, tempBuffer);

		if (!weapontype.isset())
			weapontype = nullptr;

		if (!veteranweapontype.isset())
			veteranweapontype = weapontype;

		if (!eliteweapontype.isset())
			eliteweapontype = veteranweapontype;

		if (weapontype == nullptr && veteranweapontype == nullptr && eliteweapontype == nullptr)
			break;

		if (!weaponflh.isset())
			weaponflh = this->OwnerObject()->GetWeapon(1).FLH;

		if (!veteranweaponflh.isset())
			veteranweaponflh = weaponflh;

		if (!eliteweaponflh.isset())
			eliteweaponflh = veteranweaponflh;

		weapon.Rookie.WeaponType = weapontype;
		weapon.Rookie.FLH = weaponflh;

		weapon.Veteran.WeaponType = veteranweapontype;
		weapon.Veteran.FLH = veteranweaponflh;

		weapon.Elite.WeaponType = eliteweapontype;
		weapon.Elite.FLH = eliteweaponflh;

		this->Overpower_ChargeLevel.push_back(level);
		this->Overpower_ChargeLevel_Weapon.push_back(weapon);
	}

	this->LaserFencePost_FenceType.Read(exINI, pSection, "LaserFencePost.FenceType");
	this->LaserFence_Warhead.Read(exINI, pSection, "LaserFence.Warhead");

	this->DisplayIncome.Read(exINI, pSection, "DisplayIncome");
	this->DisplayIncome_Houses.Read(exINI, pSection, "DisplayIncome.Houses");
	this->DisplayIncome_Offset.Read(exINI, pSection, "DisplayIncome.Offset");

	this->PrismForwarding.LoadFromINIFile(pThis, pINI);
}

void BuildingTypeExt::ExtData::CompleteInitialization()
{
	auto const pThis = this->OwnerObject();
	UNREFERENCED_PARAMETER(pThis);
}

template <typename T>
void BuildingTypeExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->InitialStrength_Cloning)
		.Process(this->PowersUp_Owner)
		.Process(this->PowersUp_Buildings)
		.Process(this->PowerPlantEnhancer_Buildings)
		.Process(this->PowerPlantEnhancer_Amount)
		.Process(this->PowerPlantEnhancer_Factor)
		.Process(this->SuperWeapons)
		.Process(this->OccupierMuzzleFlashes)
		.Process(this->Powered_KillSpawns)
		.Process(this->AllowAirstrike)
		.Process(this->Refinery_UseStorage)
		.Process(this->Grinding_AllowAllies)
		.Process(this->Grinding_AllowOwner)
		.Process(this->Grinding_AllowTypes)
		.Process(this->Grinding_DisallowTypes)
		.Process(this->Grinding_Sound)
		.Process(this->Grinding_PlayDieSound)
		.Process(this->Grinding_Weapon)

		.Process(this->PlacementPreview)
		.Process(this->PlacementPreview_Shape)
		.Process(this->PlacementPreview_ShapeFrame)
		.Process(this->PlacementPreview_Offset)
		.Process(this->PlacementPreview_Remap)
		.Process(this->PlacementPreview_Palette)
		.Process(this->PlacementPreview_Translucency)

		.Process(this->PackupSound_PlayGlobal)
		.Process(this->DisableDamageSound)
		.Process(this->BuildingOccupyDamageMult)
		.Process(this->BuildingOccupyROFMult)
		.Process(this->BuildingBunkerDamageMult)
		.Process(this->BuildingBunkerROFMult)
		.Process(this->Power_DegradeWithHealth)

		.Process(this->Factory_ExplicitOnly)

		.Process(this->SpyEffect_Custom)
		.Process(this->SpyEffect_VictimSuperWeapon)
		.Process(this->SpyEffect_InfiltratorSuperWeapon)

		.Process(this->SpyEffect_StolenMoneyAmount)
		.Process(this->SpyEffect_StolenMoneyPercentage)
		.Process(this->SpyEffect_StolenMoneyDisplay)
		.Process(this->SpyEffect_StolenMoneyDisplay_Houses)
		.Process(this->SpyEffect_StolenMoneyDisplay_Offset)

		.Process(this->SpyEffect_BuildingOfflineDuration)

		.Process(this->SpyEffect_InfantryDeactivate_Duration)
		.Process(this->SpyEffect_InfantryDeactivate_Types)
		.Process(this->SpyEffect_InfantryDeactivate_Ignore)
		.Process(this->SpyEffect_VehicleDeactivate_Duration)
		.Process(this->SpyEffect_VehicleDeactivate_Types)
		.Process(this->SpyEffect_VehicleDeactivate_Ignore)
		.Process(this->SpyEffect_NavalDeactivate_Duration)
		.Process(this->SpyEffect_NavalDeactivate_Types)
		.Process(this->SpyEffect_NavalDeactivate_Ignore)
		.Process(this->SpyEffect_AircraftDeactivate_Duration)
		.Process(this->SpyEffect_AircraftDeactivate_Types)
		.Process(this->SpyEffect_AircraftDeactivate_Ignore)
		.Process(this->SpyEffect_BuildingDeactivate_Duration)
		.Process(this->SpyEffect_BuildingDeactivate_Types)
		.Process(this->SpyEffect_BuildingDeactivate_Ignore)
		.Process(this->SpyEffect_DefenseDeactivate_Duration)
		.Process(this->SpyEffect_DefenseDeactivate_Types)
		.Process(this->SpyEffect_DefenseDeactivate_Ignore)

		.Process(this->SpyEffect_Veterancy_AffectBuildingOwner)
		.Process(this->SpyEffect_InfantryVeterancy)
		.Process(this->SpyEffect_VehicleVeterancy)
		.Process(this->SpyEffect_NavalVeterancy)
		.Process(this->SpyEffect_AircraftVeterancy)
		.Process(this->SpyEffect_BuildingVeterancy)
		.Process(this->SpyEffect_InfantryVeterancy_Types)
		.Process(this->SpyEffect_VehicleVeterancy_Types)
		.Process(this->SpyEffect_NavalVeterancy_Types)
		.Process(this->SpyEffect_AircraftVeterancy_Types)
		.Process(this->SpyEffect_BuildingVeterancy_Types)
		.Process(this->SpyEffect_InfantryVeterancy_Ignore)
		.Process(this->SpyEffect_VehicleVeterancy_Ignore)
		.Process(this->SpyEffect_NavalVeterancy_Ignore)
		.Process(this->SpyEffect_AircraftVeterancy_Ignore)
		.Process(this->SpyEffect_BuildingVeterancy_Ignore)
		.Process(this->SpyEffect_InfantryVeterancy_Cumulative)
		.Process(this->SpyEffect_VehicleVeterancy_Cumulative)
		.Process(this->SpyEffect_NavalVeterancy_Cumulative)
		.Process(this->SpyEffect_AircraftVeterancy_Cumulative)
		.Process(this->SpyEffect_BuildingVeterancy_Cumulative)
		.Process(this->SpyEffect_InfantryVeterancy_Reset)
		.Process(this->SpyEffect_VehicleVeterancy_Reset)
		.Process(this->SpyEffect_NavalVeterancy_Reset)
		.Process(this->SpyEffect_AircraftVeterancy_Reset)
		.Process(this->SpyEffect_BuildingVeterancy_Reset)

		.Process(this->SpyEffect_SabotageDelay)
		.Process(this->SpyEffect_SellDelay)
		.Process(this->SpyEffect_CaptureDelay)
		.Process(this->SpyEffect_CaptureCount)

		.Process(this->SpyEffect_SuperWeaponTypes)
		.Process(this->SpyEffect_SuperWeaponTypes_Permanent)
		.Process(this->SpyEffect_SuperWeaponTypes_AffectOwned)
		.Process(this->SpyEffect_SuperWeaponTypes_Delay)

		.Process(this->SpyEffect_RechargeSuperWeaponTypes)
		.Process(this->SpyEffect_RechargeSuperWeaponTypes_Duration)
		.Process(this->SpyEffect_RechargeSuperWeaponTypes_SetPercentage)
		.Process(this->SpyEffect_RechargeSuperWeaponTypes_CumulativeCount)

		.Process(this->SpyEffect_RevealSightDuration)
		.Process(this->SpyEffect_RevealSightRange)
		.Process(this->SpyEffect_RevealSightPermanent)

		.Process(this->SpyEffect_RadarJamDuration)

		.Process(this->SpyEffect_PowerOutageDuration)

		.Process(this->SpyEffect_GapRadarDuration)
		.Process(this->SpyEffect_RevealRadarSightDuration)
		.Process(this->SpyEffect_RevealRadarSightPermanent)
		.Process(this->SpyEffect_KeepRevealRadarSight)
		.Process(this->SpyEffect_RevealRadarSight_Infantry)
		.Process(this->SpyEffect_RevealRadarSight_Unit)
		.Process(this->SpyEffect_RevealRadarSight_Aircraft)
		.Process(this->SpyEffect_RevealRadarSight_Building)

		.Process(this->SpyEffect_CostBonus_AffectBuildingOwner)
		.Process(this->SpyEffect_InfantryCostBonus)
		.Process(this->SpyEffect_UnitsCostBonus)
		.Process(this->SpyEffect_NavalCostBonus)
		.Process(this->SpyEffect_AircraftCostBonus)
		.Process(this->SpyEffect_BuildingsCostBonus)
		.Process(this->SpyEffect_DefensesCostBonus)
		.Process(this->SpyEffect_InfantryCostBonus_Types)
		.Process(this->SpyEffect_UnitsCostBonus_Types)
		.Process(this->SpyEffect_NavalCostBonus_Types)
		.Process(this->SpyEffect_AircraftCostBonus_Types)
		.Process(this->SpyEffect_BuildingsCostBonus_Types)
		.Process(this->SpyEffect_DefensesCostBonus_Types)
		.Process(this->SpyEffect_InfantryCostBonus_Ignore)
		.Process(this->SpyEffect_UnitsCostBonus_Ignore)
		.Process(this->SpyEffect_NavalCostBonus_Ignore)
		.Process(this->SpyEffect_AircraftCostBonus_Ignore)
		.Process(this->SpyEffect_BuildingsCostBonus_Ignore)
		.Process(this->SpyEffect_DefensesCostBonus_Ignore)
		.Process(this->SpyEffect_InfantryCostBonus_Reset)
		.Process(this->SpyEffect_UnitsCostBonus_Reset)
		.Process(this->SpyEffect_NavalCostBonus_Reset)
		.Process(this->SpyEffect_AircraftCostBonus_Reset)
		.Process(this->SpyEffect_BuildingsCostBonus_Reset)
		.Process(this->SpyEffect_DefensesCostBonus_Reset)
		.Process(this->SpyEffect_InfantryCostBonus_Max)
		.Process(this->SpyEffect_UnitsCostBonus_Max)
		.Process(this->SpyEffect_NavalCostBonus_Max)
		.Process(this->SpyEffect_AircraftCostBonus_Max)
		.Process(this->SpyEffect_BuildingsCostBonus_Max)
		.Process(this->SpyEffect_DefensesCostBonus_Max)
		.Process(this->SpyEffect_InfantryCostBonus_Min)
		.Process(this->SpyEffect_UnitsCostBonus_Min)
		.Process(this->SpyEffect_NavalCostBonus_Min)
		.Process(this->SpyEffect_AircraftCostBonus_Min)
		.Process(this->SpyEffect_BuildingsCostBonus_Min)
		.Process(this->SpyEffect_DefensesCostBonus_Min)

		.Process(this->SpyEffect_PurifierBonus_AffectBuildingOwner)
		.Process(this->SpyEffect_PurifierBonus)
		.Process(this->SpyEffect_PurifierBonus_Reset)

		.Process(this->SpyEffect_Messages)
		.Process(this->SpyEffect_Message_ShowOwners)
		.Process(this->SpyEffect_Message_ColorTypes)

		.Process(this->SpyEffect_Anim)
		.Process(this->SpyEffect_Anim_Duration)
		.Process(this->SpyEffect_Anim_DisplayHouses)

		.Process(this->RallyRange)

		.Process(this->SellWeapon)

		.Process(this->Overpower_KeepOnline)
		.Process(this->Overpower_ChargeWeapon)
		.Process(this->Overpower_ChargeLevel)
		.Process(this->Overpower_ChargeLevel_Weapon)

		.Process(this->LaserFencePost_FenceType)
		.Process(this->LaserFence_Warhead)

		.Process(this->DisplayIncome)
		.Process(this->DisplayIncome_Houses)
		.Process(this->DisplayIncome_Offset)

		.Process(this->PrismForwarding)
		;
}

void BuildingTypeExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<BuildingTypeClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void BuildingTypeExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<BuildingTypeClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool BuildingTypeExt::ExtContainer::Load(BuildingTypeClass* pThis, IStream* pStm)
{
	BuildingTypeExt::ExtData* pData = this->LoadKey(pThis, pStm);

	return pData != nullptr;
};

bool BuildingTypeExt::LoadGlobals(PhobosStreamReader& Stm)
{

	return Stm.Success();
}

bool BuildingTypeExt::SaveGlobals(PhobosStreamWriter& Stm)
{


	return Stm.Success();
}
// =============================
// container

BuildingTypeExt::ExtContainer::ExtContainer() : Container("BuildingTypeClass") { }

BuildingTypeExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x45E50C, BuildingTypeClass_CTOR, 0x6)
{
	GET(BuildingTypeClass*, pItem, EAX);

	BuildingTypeExt::ExtMap.FindOrAllocate(pItem);
	return 0;
}

DEFINE_HOOK(0x45E707, BuildingTypeClass_DTOR, 0x6)
{
	GET(BuildingTypeClass*, pItem, ESI);

	BuildingTypeExt::ExtMap.Remove(pItem);
	return 0;
}

DEFINE_HOOK_AGAIN(0x465300, BuildingTypeClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x465010, BuildingTypeClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(BuildingTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	BuildingTypeExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x4652ED, BuildingTypeClass_Load_Suffix, 0x7)
{
	BuildingTypeExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x46536A, BuildingTypeClass_Save_Suffix, 0x7)
{
	BuildingTypeExt::ExtMap.SaveStatic();
	return 0;
}

DEFINE_HOOK_AGAIN(0x464A56, BuildingTypeClass_LoadFromINI, 0xA)
DEFINE_HOOK(0x464A49, BuildingTypeClass_LoadFromINI, 0xA)
{
	GET(BuildingTypeClass*, pItem, EBP);
	GET_STACK(CCINIClass*, pINI, 0x364);

	BuildingTypeExt::ExtMap.LoadFromINI(pItem, pINI);
	return 0;
}
