#pragma once
#include <BuildingTypeClass.h>
#include <SuperWeaponTypeClass.h>
#include <VoxClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/Template.h>
#include <Utilities/Constructs.h>
#include <Utilities/Enum.h>

class AresBuildingTypeExtData
{
public:
	BuildingTypeClass* AttachedObject;
	DWORD DWORD_4 = 0;
	DWORD DWORD_8 = 0;
	DWORD DWORD_C = 1;
	BYTE BYTE_10 = 0;
	DWORD DWORD_14 = 0;
	DWORD DWORD_18 = 0;
	DWORD DWORD_1C = 0;
	DWORD DWORD_20 = 0;
	DWORD DWORD_24 = 0;
	DWORD DWORD_28 = 0;
	DWORD DWORD_2C = 0;
	DWORD DWORD_30 = 0;
	DWORD vfptr;
	DWORD DWORD_38 = 0;
	DWORD DWORD_3C = 0;
	WORD WORD_40 = 1;
	DWORD DWORD_44 = 0;
	DWORD DWORD_48 = 10;
	NullableVector<TechnoTypeClass*> SecretLab_PossibleBoons;
	Valueable<bool> SecretLab_GenerateOnCapture;
	Valueable<bool> IsPassable;
	Valueable<double> LightningRod_Modifier;
	Valueable<double> UC_PassThrough;
	Valueable<double> UC_FatalRate;
	Valueable<double> UC_DamageMultiplier;
	Valueable<bool> Bunker_Raidable;
	DWORD IsTrench;
	Valueable<BuildingTypeClass*> Rubble_Intact;
	Valueable<BuildingTypeClass*> Rubble_Destroyed;
	Valueable<AnimTypeClass*> Rubble_Destroyed_Anim;
	Valueable<AnimTypeClass*> Rubble_Intact_Anim;
	Valueable<AresOwnerEnum> Rubble_Destroyed_Owner;
	Valueable<AresOwnerEnum> Rubble_Intact_Owner;
	Valueable<int> Rubble_Destroyed_Strength;
	Valueable<int> Rubble_Intact_Strength;
	Valueable<bool> Rubble_Destroyed_Remove;
	Valueable<bool> Rubble_Intact_Remove;
	Valueable<bool> SpyEffect_Custom;
	Valueable<bool> SpyEffect_RevealProduction;
	Valueable<bool> SpyEffect_ResetSuperweapons;
	Valueable<bool> SpyEffect_ResetRadar;
	Valueable<bool> SpyEffect_RevealRadar;
	Valueable<bool> SpyEffect_KeepRadar;
	Valueable<bool> SpyEffect_UnitVeterancy;
	Valueable<bool> SpyEffect_InfantryVeterancy;
	Valueable<bool> SpyEffect_VehicleVeterancy;
	Valueable<bool> SpyEffect_NavalVeterancy;
	Valueable<bool> SpyEffect_AircraftVeterancy;
	Valueable<bool> SpyEffect_BuildingVeterancy;
	Valueable<bool> SpyEffect_UndoReverseEngineer;
	Valueable<bool> SpyEffect_SuperWeaponPermanent;
	Valueable<SuperWeaponTypeClass*> SpyEffect_SuperWeapon;
	DWORD SpyEffect_StolenTechIndex;
	Valueable<int> SpyEffect_StolenMoneyAmount;
	Valueable<float> SpyEffect_StolenMoneyPercentage;
	Valueable<int> SpyEffect_PowerOutageDuration;
	Valueable<int> SpyEffect_SabotageDelay;
	ValueableVector<InfantryTypeClass*> CanBeOccupiedBy;
	Nullable<bool> Returnable;
	DWORD DWORD_E0 = 0;
	DWORD DWORD_E4 = 0;
	DWORD DWORD_E8 = 0;
	DWORD DWORD_EC = 0;
	DWORD DWORD_F0 = 0;
	BYTE BYTE_F4 = 0;
	DWORD DWORD_F8 = 1;
	DWORD DWORD_FC = 0;
	BYTE BYTE_100 = 0;
	DWORD DWORD_104 = 0;
	BYTE BYTE_108 = 0;
	DWORD DWORD_10C = 0;
	int DWORD_110 = -1;
	int DWORD_114 = 1;
	WORD WORD_118 = 0;
	int DWORD_11C = -1;
	int DWORD_120 = -1;
	Valueable<bool> ReverseEngineersVictims;
	Valueable<bool> CloningFacility;
	Valueable<bool> Factory_ExplicitOnly;
	NullableIdx<VocClass> GateDownSound;
	NullableIdx<VocClass> GateUpSound;
	bool Academy;
	ValueableVector<TechnoTypeClass*> Academy_Types;
	ValueableVector<TechnoTypeClass*> Academy_Ignore;
	Valueable<double> Academy_InfantryVeterancy;
	Valueable<double> Academy_AircraftVeterancy;
	Valueable<double> Academy_VehicleVeterancy;
	Valueable<double> Academy_BuildingVeterancy;
	ValueableIdxVector<SuperWeaponTypeClass> SuperWeapons;
	ValueableIdx<VoxClass> LostEvaEvent;
	PhobosFixedString<0x20> Message_Capture;
	DWORD DWORD_1A8 = 0;
	PhobosFixedString<0x20> Message_Lost;
	BYTE BYTE_1CC[4] = { 0 };
	Nullable<int> Degrade_Amount;
	Nullable<double> Degrade_Percentage;
	BYTE BYTE_1E4[4] = { 0 };
	Nullable<bool> ImmuneToSaboteurs;
	ValueableVector<int> AIBuildCounts;
	ValueableVector<int> AIExtraCounts;
	Nullable<double> BuildupTime;
	Nullable<double> SellTime;
	BYTE BYTE_220[4] = { 0 };
	DWORD DWORD_228 = 0;
	Nullable<bool> MassSelectable;
	Valueable<bool> ProduceCashDisplay;
	Nullable<bool> UnitSell;
	Nullable<bool> EngineerRepairable;
	Nullable<bool> AIInnerBase;
	Nullable<bool> AIBaseNormal;
	NullableIdx<VocClass> SlamSound;
	DWORD Cursor_Spy = 25;
	int Tunnel = -1;
	DWORD DWORD_248 = 3;
	DWORD DWORD_24C = 1;
	DWORD DWORD_250 = 8;

	AresBuildingTypeExtData() = delete;
};

static_assert(sizeof(AresBuildingTypeExtData) == 0x258);
