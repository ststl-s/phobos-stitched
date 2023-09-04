#pragma once

#include <BuildingTypeClass.h>

#include <Utilities/Constructs.h>
#include <Utilities/Container.h>
#include <Utilities/Enum.h>
#include <Utilities/Template.h>

class SuperWeaponTypeClass;
class VoxClass;

class BuildingTypeExt
{
public:
	using base_type = BuildingTypeClass;

	class ExtData final : public Extension<BuildingTypeClass>
	{
	public:
		Valueable<AffectedHouse> PowersUp_Owner;
		ValueableVector<BuildingTypeClass*> PowersUp_Buildings;
		ValueableIdxVector<SuperWeaponTypeClass> SuperWeapons;

		ValueableVector<BuildingTypeClass*> PowerPlantEnhancer_Buildings;
		Nullable<int> PowerPlantEnhancer_Amount;
		Nullable<float> PowerPlantEnhancer_Factor;

		DynamicVectorClass<Point2D> OccupierMuzzleFlashes;
		Valueable<bool> Powered_KillSpawns;
		Nullable<bool> AllowAirstrike;
		Valueable<bool> Refinery_UseStorage;

		Valueable<PartialVector2D<double>> InitialStrength_Cloning;

		Valueable<bool> Grinding_AllowAllies;
		Valueable<bool> Grinding_AllowOwner;
		ValueableVector<TechnoTypeClass*> Grinding_AllowTypes;
		ValueableVector<TechnoTypeClass*> Grinding_DisallowTypes;
		NullableIdx<VocClass> Grinding_Sound;
		Nullable<WeaponTypeClass*> Grinding_Weapon;
		Valueable<bool> Grinding_DisplayRefund;
		Valueable<AffectedHouse> Grinding_DisplayRefund_Houses;
		Valueable<Point2D> Grinding_DisplayRefund_Offset;

		Valueable<bool> PlacementPreview;
		TheaterSpecificSHP PlacementPreview_Shape;
		Nullable<int> PlacementPreview_ShapeFrame;
		Valueable<CoordStruct> PlacementPreview_Offset;
		Valueable<bool> PlacementPreview_Remap;
		CustomPalette PlacementPreview_Palette;
		Nullable<TranslucencyLevel> PlacementPreview_Translucency;

		Valueable<bool> PackupSound_PlayGlobal;
		Valueable<bool> DisableDamageSound;
		Nullable<float> BuildingOccupyDamageMult;
		Nullable<float> BuildingOccupyROFMult;
		Nullable<float> BuildingBunkerDamageMult;
		Nullable<float> BuildingBunkerROFMult;
		NullableIdx<VocClass> BunkerWallsUpSound;
		NullableIdx<VocClass> BunkerWallsDownSound;

		Valueable<bool> Power_DegradeWithHealth;

		NullableIdx<VoxClass> EnterBioReactorSound;
		NullableIdx<VoxClass> LeaveBioReactorSound;

		Valueable<bool> SpyEffect_Custom;
		NullableIdx<SuperWeaponTypeClass> SpyEffect_VictimSuperWeapon;
		NullableIdx<SuperWeaponTypeClass> SpyEffect_InfiltratorSuperWeapon;

		Valueable<int> SpyEffect_StolenMoneyAmount;
		Valueable<double> SpyEffect_StolenMoneyPercentage;
		Valueable<bool> SpyEffect_StolenMoneyDisplay;
		Valueable<AffectedHouse> SpyEffect_StolenMoneyDisplay_Houses;
		Valueable<Point2D> SpyEffect_StolenMoneyDisplay_Offset;

		Valueable<int> SpyEffect_BuildingOfflineDuration;

		Valueable<int> SpyEffect_InfantryDeactivate_Duration;
		ValueableVector<TechnoTypeClass*> SpyEffect_InfantryDeactivate_Types;
		ValueableVector<TechnoTypeClass*> SpyEffect_InfantryDeactivate_Ignore;
		Valueable<int> SpyEffect_VehicleDeactivate_Duration;
		ValueableVector<TechnoTypeClass*> SpyEffect_VehicleDeactivate_Types;
		ValueableVector<TechnoTypeClass*> SpyEffect_VehicleDeactivate_Ignore;
		Valueable<int> SpyEffect_NavalDeactivate_Duration;
		ValueableVector<TechnoTypeClass*> SpyEffect_NavalDeactivate_Types;
		ValueableVector<TechnoTypeClass*> SpyEffect_NavalDeactivate_Ignore;
		Valueable<int> SpyEffect_AircraftDeactivate_Duration;
		ValueableVector<TechnoTypeClass*> SpyEffect_AircraftDeactivate_Types;
		ValueableVector<TechnoTypeClass*> SpyEffect_AircraftDeactivate_Ignore;
		Valueable<int> SpyEffect_BuildingDeactivate_Duration;
		ValueableVector<TechnoTypeClass*> SpyEffect_BuildingDeactivate_Types;
		ValueableVector<TechnoTypeClass*> SpyEffect_BuildingDeactivate_Ignore;
		Valueable<int> SpyEffect_DefenseDeactivate_Duration;
		ValueableVector<TechnoTypeClass*> SpyEffect_DefenseDeactivate_Types;
		ValueableVector<TechnoTypeClass*> SpyEffect_DefenseDeactivate_Ignore;

		Valueable<bool> SpyEffect_Veterancy_AffectBuildingOwner;
		Valueable<double> SpyEffect_InfantryVeterancy;
		Valueable<double> SpyEffect_VehicleVeterancy;
		Valueable<double> SpyEffect_NavalVeterancy;
		Valueable<double> SpyEffect_AircraftVeterancy;
		Valueable<double> SpyEffect_BuildingVeterancy;
		ValueableVector<TechnoTypeClass*> SpyEffect_InfantryVeterancy_Types;
		ValueableVector<TechnoTypeClass*> SpyEffect_VehicleVeterancy_Types;
		ValueableVector<TechnoTypeClass*> SpyEffect_NavalVeterancy_Types;
		ValueableVector<TechnoTypeClass*> SpyEffect_AircraftVeterancy_Types;
		ValueableVector<TechnoTypeClass*> SpyEffect_BuildingVeterancy_Types;
		ValueableVector<TechnoTypeClass*> SpyEffect_InfantryVeterancy_Ignore;
		ValueableVector<TechnoTypeClass*> SpyEffect_VehicleVeterancy_Ignore;
		ValueableVector<TechnoTypeClass*> SpyEffect_NavalVeterancy_Ignore;
		ValueableVector<TechnoTypeClass*> SpyEffect_AircraftVeterancy_Ignore;
		ValueableVector<TechnoTypeClass*> SpyEffect_BuildingVeterancy_Ignore;
		Valueable<bool> SpyEffect_InfantryVeterancy_Cumulative;
		Valueable<bool> SpyEffect_VehicleVeterancy_Cumulative;
		Valueable<bool> SpyEffect_NavalVeterancy_Cumulative;
		Valueable<bool> SpyEffect_AircraftVeterancy_Cumulative;
		Valueable<bool> SpyEffect_BuildingVeterancy_Cumulative;
		Valueable<bool> SpyEffect_InfantryVeterancy_Reset;
		Valueable<bool> SpyEffect_VehicleVeterancy_Reset;
		Valueable<bool> SpyEffect_NavalVeterancy_Reset;
		Valueable<bool> SpyEffect_AircraftVeterancy_Reset;
		Valueable<bool> SpyEffect_BuildingVeterancy_Reset;

		Valueable<int> SpyEffect_SabotageDelay;
		Valueable<int> SpyEffect_SellDelay;
		Valueable<int> SpyEffect_CaptureDelay;
		Valueable<int> SpyEffect_CaptureCount;

		ValueableIdxVector<SuperWeaponTypeClass> SpyEffect_SuperWeaponTypes;
		ValueableVector<bool> SpyEffect_SuperWeaponTypes_AffectOwned;
		ValueableVector<int> SpyEffect_SuperWeaponTypes_Delay;

		ValueableIdxVector<SuperWeaponTypeClass> SpyEffect_RechargeSuperWeaponTypes;
		ValueableVector<double> SpyEffect_RechargeSuperWeaponTypes_Duration;
		ValueableVector<double> SpyEffect_RechargeSuperWeaponTypes_SetPercentage;
		ValueableVector<int> SpyEffect_RechargeSuperWeaponTypes_CumulativeCount;

		Valueable<int> SpyEffect_RevealSightDuration;
		Valueable<int> SpyEffect_RevealSightRange;
		Valueable<bool> SpyEffect_RevealSightPermanent;

		Valueable<int> SpyEffect_RadarJamDuration;

		Valueable<int> SpyEffect_PowerOutageDuration;

		Valueable<int> SpyEffect_GapRadarDuration;
		Valueable<int> SpyEffect_RevealRadarSightDuration;
		Valueable<bool> SpyEffect_RevealRadarSightPermanent;
		Valueable<bool> SpyEffect_KeepRevealRadarSight;
		Valueable<bool> SpyEffect_RevealRadarSight_Infantry;
		Valueable<bool> SpyEffect_RevealRadarSight_Unit;
		Valueable<bool> SpyEffect_RevealRadarSight_Aircraft;
		Valueable<bool> SpyEffect_RevealRadarSight_Building;

		Valueable<AnimTypeClass*> SpyEffect_Anim;
		Valueable<int> SpyEffect_Anim_Duration;
		Valueable<AffectedHouse> SpyEffect_Anim_DisplayHouses;

		ValueableVector<CSFText> SpyEffect_Messages;
		ValueableVector<ShowMessageHouse> SpyEffect_Message_ShowOwners;
		ValueableVector<ShowMessageHouse> SpyEffect_Message_ColorTypes;

		Valueable<int> RallyRange;

		Valueable<WeaponTypeClass*> SellWeapon;

		//Ares
		Valueable<bool> Factory_ExplicitOnly;

		ExtData(BuildingTypeClass* OwnerObject) : Extension<BuildingTypeClass>(OwnerObject)
			, PowersUp_Owner { AffectedHouse::Owner }
			, PowersUp_Buildings {}
			, PowerPlantEnhancer_Buildings {}
			, PowerPlantEnhancer_Amount {}
			, PowerPlantEnhancer_Factor {}
			, OccupierMuzzleFlashes()
			, Powered_KillSpawns { false }
			, AllowAirstrike {}
			, Refinery_UseStorage { false }
			, InitialStrength_Cloning { { 1.0, 0.0 } }
			, Grinding_AllowAllies { false }
			, Grinding_AllowOwner { true }
			, Grinding_AllowTypes {}
			, Grinding_DisallowTypes {}
			, Grinding_Sound {}
			, Grinding_Weapon {}
			, Grinding_DisplayRefund { false }
			, Grinding_DisplayRefund_Houses { AffectedHouse::All }
			, Grinding_DisplayRefund_Offset { { 0,0 } }
			, PackupSound_PlayGlobal { false }
			, DisableDamageSound { false }
			, BuildingOccupyDamageMult {}
			, BuildingOccupyROFMult {}
			, BuildingBunkerDamageMult {}
			, BuildingBunkerROFMult {}
			, BunkerWallsUpSound {}
			, BunkerWallsDownSound {}
			, Power_DegradeWithHealth { true }

			, PlacementPreview { true }
			, PlacementPreview_Shape {}
			, PlacementPreview_ShapeFrame {}
			, PlacementPreview_Remap { true }
			, PlacementPreview_Offset { { 0, -15, 1 } }
			, PlacementPreview_Palette {}
			, PlacementPreview_Translucency {}

			, SpyEffect_Custom { false }
			, SpyEffect_VictimSuperWeapon {}
			, SpyEffect_InfiltratorSuperWeapon {}

			, SpyEffect_StolenMoneyAmount { 0 }
			, SpyEffect_StolenMoneyPercentage { 0.0 }
			, SpyEffect_StolenMoneyDisplay { false }
			, SpyEffect_StolenMoneyDisplay_Houses { AffectedHouse::All }
			, SpyEffect_StolenMoneyDisplay_Offset { { 0, 0 } }

			, SpyEffect_BuildingOfflineDuration { 0 }

			, SpyEffect_InfantryDeactivate_Duration{ 0 }
			, SpyEffect_InfantryDeactivate_Types {}
			, SpyEffect_InfantryDeactivate_Ignore {}
			, SpyEffect_VehicleDeactivate_Duration { 0 }
			, SpyEffect_VehicleDeactivate_Types {}
			, SpyEffect_VehicleDeactivate_Ignore {}
			, SpyEffect_NavalDeactivate_Duration { 0 }
			, SpyEffect_NavalDeactivate_Types {}
			, SpyEffect_NavalDeactivate_Ignore {}
			, SpyEffect_AircraftDeactivate_Duration { 0 }
			, SpyEffect_AircraftDeactivate_Types {}
			, SpyEffect_AircraftDeactivate_Ignore {}
			, SpyEffect_BuildingDeactivate_Duration { 0 }
			, SpyEffect_BuildingDeactivate_Types {}
			, SpyEffect_BuildingDeactivate_Ignore {}
			, SpyEffect_DefenseDeactivate_Duration { 0 }
			, SpyEffect_DefenseDeactivate_Types {}
			, SpyEffect_DefenseDeactivate_Ignore {}

			, SpyEffect_Veterancy_AffectBuildingOwner { false }
			, SpyEffect_InfantryVeterancy { 0.0 }
			, SpyEffect_VehicleVeterancy { 0.0 }
			, SpyEffect_NavalVeterancy { 0.0 }
			, SpyEffect_AircraftVeterancy { 0.0 }
			, SpyEffect_BuildingVeterancy { 0.0 }
			, SpyEffect_InfantryVeterancy_Types {}
			, SpyEffect_VehicleVeterancy_Types {}
			, SpyEffect_NavalVeterancy_Types {}
			, SpyEffect_AircraftVeterancy_Types {}
			, SpyEffect_BuildingVeterancy_Types {}
			, SpyEffect_InfantryVeterancy_Ignore {}
			, SpyEffect_VehicleVeterancy_Ignore {}
			, SpyEffect_NavalVeterancy_Ignore {}
			, SpyEffect_AircraftVeterancy_Ignore {}
			, SpyEffect_BuildingVeterancy_Ignore {}
			, SpyEffect_InfantryVeterancy_Cumulative { false }
			, SpyEffect_VehicleVeterancy_Cumulative { false }
			, SpyEffect_NavalVeterancy_Cumulative { false }
			, SpyEffect_AircraftVeterancy_Cumulative { false }
			, SpyEffect_BuildingVeterancy_Cumulative { false }
			, SpyEffect_InfantryVeterancy_Reset { false }
			, SpyEffect_VehicleVeterancy_Reset { false }
			, SpyEffect_NavalVeterancy_Reset { false }
			, SpyEffect_AircraftVeterancy_Reset { false }
			, SpyEffect_BuildingVeterancy_Reset { false }

			, SpyEffect_SabotageDelay { 0 }
			, SpyEffect_SellDelay { 0 }
			, SpyEffect_CaptureDelay { 0 }
			, SpyEffect_CaptureCount { 0 }

			, SpyEffect_SuperWeaponTypes {}
			, SpyEffect_SuperWeaponTypes_AffectOwned {}
			, SpyEffect_SuperWeaponTypes_Delay {}

			, SpyEffect_RechargeSuperWeaponTypes {}
			, SpyEffect_RechargeSuperWeaponTypes_Duration {}
			, SpyEffect_RechargeSuperWeaponTypes_SetPercentage {}
			, SpyEffect_RechargeSuperWeaponTypes_CumulativeCount {}

			, SpyEffect_RevealSightDuration { 0 }
			, SpyEffect_RevealSightRange { 0 }
			, SpyEffect_RevealSightPermanent { false }

			, SpyEffect_RadarJamDuration { 0 }

			, SpyEffect_PowerOutageDuration { 0 }

			, SpyEffect_GapRadarDuration { 0 }
			, SpyEffect_RevealRadarSightDuration { 0 }
			, SpyEffect_RevealRadarSightPermanent { false }
			, SpyEffect_KeepRevealRadarSight { false }
			, SpyEffect_RevealRadarSight_Infantry { true }
			, SpyEffect_RevealRadarSight_Unit { true }
			, SpyEffect_RevealRadarSight_Aircraft { true }
			, SpyEffect_RevealRadarSight_Building { true }

			, SpyEffect_Anim { nullptr }
			, SpyEffect_Anim_Duration { 0 }
			, SpyEffect_Anim_DisplayHouses { AffectedHouse::All }

			, SpyEffect_Messages {}
			, SpyEffect_Message_ShowOwners {}
			, SpyEffect_Message_ColorTypes {}

			, EnterBioReactorSound {}
			, LeaveBioReactorSound {}

			, RallyRange { -1 }

			, SellWeapon { nullptr }
		{ }

		// Ares 0.A functions
		int GetSuperWeaponCount() const;
		int GetSuperWeaponIndex(int index, HouseClass* pHouse) const;
		int GetSuperWeaponIndex(int index) const;

		virtual ~ExtData() = default;

		virtual void LoadFromINIFile(CCINIClass* pINI) override;
		virtual void Initialize() override;
		virtual void CompleteInitialization();

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override
		{
		}

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<BuildingTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();

		virtual bool Load(BuildingTypeClass* pThis, IStream* pStm) override;
	};

	static ExtContainer ExtMap;
	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static void PlayBunkerSound(BuildingClass const* pThis, bool bUp = false);

	static int GetEnhancedPower(BuildingClass* pBuilding, HouseClass* pHouse);
	static bool CanUpgrade(BuildingClass* pBuilding, BuildingTypeClass* pUpgradeType, HouseClass* pUpgradeOwner);
	static int GetUpgradesAmount(BuildingTypeClass* pBuilding, HouseClass* pHouse);
};
