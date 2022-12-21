#include "Body.h"

#include <Ext/House/Body.h>
#include <Utilities/GeneralUtils.h>
#include <Ext/SWType/Body.h>

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
	float fFactor = 1.0f;

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
	this->Grinding_Weapon.Read(exINI, pSection, "Grinding.Weapon", true);
	this->Grinding_DisplayRefund.Read(exINI, pSection, "Grinding.DisplayRefund");
	this->Grinding_DisplayRefund_Houses.Read(exINI, pSection, "Grinding.DisplayRefund.Houses");
	this->Grinding_DisplayRefund_Offset.Read(exINI, pSection, "Grinding.DisplayRefund.Offset");

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

	this->EnterBioReactorSound.Read(exINI, pSection, "EnterBioReactorSound");
	this->LeaveBioReactorSound.Read(exINI, pSection, "LeaveBioReactorSound");

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

	this->RallyRange.Read(exINI, pSection, "RallyRange");
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
		.Process(this->Grinding_Weapon)
		.Process(this->Grinding_DisplayRefund)
		.Process(this->Grinding_DisplayRefund_Houses)
		.Process(this->Grinding_DisplayRefund_Offset)

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

		.Process(this->EnterBioReactorSound)
		.Process(this->LeaveBioReactorSound)

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

		.Process(this->RallyRange)
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
