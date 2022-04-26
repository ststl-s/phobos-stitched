#include "Body.h"

#include <TechnoTypeClass.h>
#include <StringTable.h>

#include <Ext/BuildingType/Body.h>
#include <Ext/BulletType/Body.h>
#include <Ext/Techno/Body.h>

#include <Utilities/GeneralUtils.h>
#include <Utilities/PointerMapper.h>

template<> const DWORD Extension<TechnoTypeClass>::Canary = 0x11111111;
TechnoTypeExt::ExtContainer TechnoTypeExt::ExtMap;
int TechnoTypeExt::ExtData::counter = 0;

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

	this->HealthBar_Hide.Read(exINI, pSection, "HealthBar.Hide");
	this->UIDescription.Read(exINI, pSection, "UIDescription");
	this->LowSelectionPriority.Read(exINI, pSection, "LowSelectionPriority");
	this->MindControlRangeLimit.Read(exINI, pSection, "MindControlRangeLimit");
	this->Interceptor.Read(exINI, pSection, "Interceptor");
	this->Interceptor_Rookie.Read(exINI, pSection, "Interceptor.Rookie");
	this->Interceptor_Veteran.Read(exINI, pSection, "Interceptor.Veteran");
	this->Interceptor_Elite.Read(exINI, pSection, "Interceptor.Elite");
	this->Interceptor_GuardRange.Read(exINI, pSection, "Interceptor.GuardRange");
	this->Interceptor_MinimumGuardRange.Read(exINI, pSection, "Interceptor.MinimumGuardRange");
	this->Interceptor_EliteGuardRange.Read(exINI, pSection, "Interceptor.EliteGuardRange");
	this->Interceptor_EliteMinimumGuardRange.Read(exINI, pSection, "Interceptor.EliteMinimumGuardRange");
	this->Interceptor_Success.Read(exINI, pSection, "Interceptor.Success");
	this->Interceptor_RookieSuccess.Read(exINI, pSection, "Interceptor.RookieSuccess");
	this->Interceptor_VeteranSuccess.Read(exINI, pSection, "Interceptor.VeteranSuccess");
	this->Interceptor_EliteSuccess.Read(exINI, pSection, "Interceptor.EliteSuccess");
	this->Powered_KillSpawns.Read(exINI, pSection, "Powered.KillSpawns");
	this->Spawn_LimitedRange.Read(exINI, pSection, "Spawner.LimitRange");
	this->Spawn_LimitedExtraRange.Read(exINI, pSection, "Spawner.ExtraLimitRange");
	this->Harvester_Counted.Read(exINI, pSection, "Harvester.Counted");
	this->Promote_IncludeSpawns.Read(exINI, pSection, "Promote.IncludeSpawns");
	this->ImmuneToCrit.Read(exINI, pSection, "ImmuneToCrit");
	this->MultiMindControl_ReleaseVictim.Read(exINI, pSection, "MultiMindControl.ReleaseVictim");
	this->NoManualMove.Read(exINI, pSection, "NoManualMove");
	this->InitialStrength.Read(exINI, pSection, "InitialStrength");
	this->Death_NoAmmo.Read(exINI, pSection, "Death.NoAmmo");
	this->Death_Countdown.Read(exINI, pSection, "Death.Countdown");
	this->Death_Peaceful.Read(exINI, pSection, "Death.Peaceful");
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

	this->PassengerDeletion_Soylent.Read(exINI, pSection, "PassengerDeletion.Soylent");
	this->PassengerDeletion_SoylentFriendlies.Read(exINI, pSection, "PassengerDeletion.SoylentFriendlies");
	this->PassengerDeletion_ReportSound.Read(exINI, pSection, "PassengerDeletion.ReportSound");
	this->PassengerDeletion_Rate_SizeMultiply.Read(exINI, pSection, "PassengerDeletion.Rate.SizeMultiply");
	this->PassengerDeletion_Rate.Read(exINI, pSection, "PassengerDeletion.Rate");
	this->PassengerDeletion_Anim.Read(exINI, pSection, "PassengerDeletion.Anim");

	this->DefaultDisguise.Read(exINI, pSection, "DefaultDisguise");

	this->OpenTopped_RangeBonus.Read(exINI, pSection, "OpenTopped.RangeBonus");
	this->OpenTopped_DamageMultiplier.Read(exINI, pSection, "OpenTopped.DamageMultiplier");
	this->OpenTopped_WarpDistance.Read(exINI, pSection, "OpenTopped.WarpDistance");

	this->AutoFire.Read(exINI, pSection, "AutoFire");
	this->AutoFire_TargetSelf.Read(exINI, pSection, "AutoFire.TargetSelf");

	this->NoSecondaryWeaponFallback.Read(exINI, pSection, "NoSecondaryWeaponFallback");

	this->JumpjetAllowLayerDeviation.Read(exINI, pSection, "JumpjetAllowLayerDeviation");

	this->DeployingAnim_AllowAnyDirection.Read(exINI, pSection, "DeployingAnim.AllowAnyDirection");
	this->DeployingAnim_KeepUnitVisible.Read(exINI, pSection, "DeployingAnim.KeepUnitVisible");
	this->DeployingAnim_ReverseForUndeploy.Read(exINI, pSection, "DeployingAnim.ReverseForUndeploy");
	this->DeployingAnim_UseUnitDrawer.Read(exINI, pSection, "DeployingAnim.UseUnitDrawer");

	this->CanRepairCyborgLegs.Read(exINI, pSection, "CanRepairCyborgLegs");

	// The following loop iterates over size + 1 INI entries so that the
	// vector contents can be properly overriden via scenario rules - Kerbiter
	for (size_t i = 0; i <= this->AttachmentData.size(); ++i)
	{
		NullableIdx<AttachmentTypeClass> type;
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "Attachment%d.Type", i);
		type.Read(exINI, pSection, tempBuffer);

		if (!type.isset())
			continue;

		ValueableVector<TechnoTypeClass*> technoType;
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "Attachment%d.TechnoType", i);
		technoType.Read(exINI, pSection, tempBuffer);

		Valueable<CoordStruct> flh;
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "Attachment%d.FLH", i);
		flh.Read(exINI, pSection, tempBuffer);

		Valueable<bool> isOnTurret;
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "Attachment%d.IsOnTurret", i);
		isOnTurret.Read(exINI, pSection, tempBuffer);

		if (!type.isset() || technoType.size() != 1U) continue;
		if (i == AttachmentData.size())
			this->AttachmentData.emplace_back(type.Get(), technoType[0], flh, isOnTurret.Get());
		else
			this->AttachmentData[i] = AttachmentDataEntry(type.Get(), technoType[0], flh, isOnTurret.Get());
	}

	// Ares 0.2
	this->RadarJamRadius.Read(exINI, pSection, "RadarJamRadius");

	// Ares 0.9
	this->InhibitorRange.Read(exINI, pSection, "InhibitorRange");

	// Ares 0.A
	this->GroupAs.Read(pINI, pSection, "GroupAs");

	this->GiftBoxData.Read(exINI, pSection);

	// Ares 0.C
	this->NoAmmoWeapon.Read(exINI, pSection, "NoAmmoWeapon");
	this->NoAmmoAmount.Read(exINI, pSection, "NoAmmoAmount");

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

	bool parseMultiWeapons = pThis->TurretCount > 0 && pThis->WeaponCount > 0;
	auto weaponCount = parseMultiWeapons ? pThis->WeaponCount : 2;
	this->WeaponBurstFLHs.resize(weaponCount);
	this->EliteWeaponBurstFLHs.resize(weaponCount);
	char tempBufferFLH[48];

	for (int i = 0; i < weaponCount; i++)
	{
		for (int j = 0; j < INT_MAX; j++)
		{
			_snprintf_s(tempBuffer, sizeof(tempBuffer), "Weapon%d", i + 1);
			auto prefix = parseMultiWeapons ? tempBuffer : i > 0 ? "SecondaryFire" : "PrimaryFire";

			_snprintf_s(tempBufferFLH, sizeof(tempBufferFLH), "%sFLH.Burst%d", prefix, j);
			Nullable<CoordStruct> FLH;
			FLH.Read(exArtINI, pArtSection, tempBufferFLH);

			_snprintf_s(tempBufferFLH, sizeof(tempBufferFLH), "Elite%sFLH.Burst%d", prefix, j);
			Nullable<CoordStruct> eliteFLH;
			eliteFLH.Read(exArtINI, pArtSection, tempBufferFLH);

			if (FLH.isset() && !eliteFLH.isset())
				eliteFLH = FLH;
			else if (!FLH.isset() && !eliteFLH.isset())
				break;

			WeaponBurstFLHs[i].AddItem(FLH.Get());
			EliteWeaponBurstFLHs[i].AddItem(eliteFLH.Get());
		}
	}

	this->CrouchedWeaponBurstFLHs.resize(weaponCount);
	this->EliteCrouchedWeaponBurstFLHs.resize(weaponCount);
	char tempBufferCrouchedFLH[48];

	// Crouched Weapon Burst FLH for infantry
	for (int i = 0; i < weaponCount; i++)
	{
		for (int j = 0; j < INT_MAX; j++)
		{
			_snprintf_s(tempBuffer, sizeof(tempBuffer), "CrouchedWeapon%d", i + 1);
			auto prefix = parseMultiWeapons ? tempBuffer : i > 0 ? "CrouchedSecondaryFire" : "CrouchedPrimaryFire";

			_snprintf_s(tempBufferCrouchedFLH, sizeof(tempBufferCrouchedFLH), "%sFLH.Burst%d", prefix, j);
			Nullable<CoordStruct> FLH;
			FLH.Read(exArtINI, pArtSection, tempBufferCrouchedFLH);

			_snprintf_s(tempBufferCrouchedFLH, sizeof(tempBufferCrouchedFLH), "Elite%sFLH.Burst%d", prefix, j);
			Nullable<CoordStruct> eliteFLH;
			eliteFLH.Read(exArtINI, pArtSection, tempBufferCrouchedFLH);

			if (FLH.isset() && !eliteFLH.isset())
				eliteFLH = FLH;
			else if (!FLH.isset() && !eliteFLH.isset())
				break;

			CrouchedWeaponBurstFLHs[i].AddItem(FLH.Get());
			EliteCrouchedWeaponBurstFLHs[i].AddItem(eliteFLH.Get());
		}
	}

	this->DeployedWeaponBurstFLHs.resize(weaponCount);
	this->EliteDeployedWeaponBurstFLHs.resize(weaponCount);
	char tempBufferDeployedFLH[48];

	// Deployed Weapon Burst FLH for infantry
	for (int i = 0; i < weaponCount; i++)
	{
		for (int j = 0; j < INT_MAX; j++)
		{
			_snprintf_s(tempBuffer, sizeof(tempBuffer), "DeployedWeapon%d", i + 1);
			auto prefix = parseMultiWeapons ? tempBuffer : i > 0 ? "DeployedSecondaryFire" : "DeployedPrimaryFire";

			_snprintf_s(tempBufferDeployedFLH, sizeof(tempBufferDeployedFLH), "%sFLH.Burst%d", prefix, j);
			Nullable<CoordStruct> FLH;
			FLH.Read(exArtINI, pArtSection, tempBufferDeployedFLH);

			_snprintf_s(tempBufferDeployedFLH, sizeof(tempBufferDeployedFLH), "Elite%sFLH.Burst%d", prefix, j);
			Nullable<CoordStruct> eliteFLH;
			eliteFLH.Read(exArtINI, pArtSection, tempBufferDeployedFLH);

			if (FLH.isset() && !eliteFLH.isset())
				eliteFLH = FLH;
			else if (!FLH.isset() && !eliteFLH.isset())
				break;

			DeployedWeaponBurstFLHs[i].AddItem(FLH.Get());
			EliteDeployedWeaponBurstFLHs[i].AddItem(eliteFLH.Get());
		}
	}

	this->EnemyUIName.Read(exINI, pSection, "EnemyUIName");

	this->Ammo_Shared.Read(exINI, pSection, "Ammo.Shared");
	this->Ammo_Shared_Group.Read(exINI, pSection, "Ammo.Shared.Group");

	this->UseCustomSelectBrd.Read(exINI, pSection, "UseCustomSelectBrd");
	this->SelectBrd_SHP.Read(pINI, pSection, "SelectBrd.SHP");
	this->SelectBrd_PAL.Read(pINI, pSection, "SelectBrd.PAL");
	this->SelectBrd_Frame.Read(exINI, pSection, "SelectBrd.Frame");
	this->SelectBrd_DrawOffset.Read(exINI, pSection, "SelectBrd.DrawOffset");

	this->CrouchedPrimaryFireFLH.Read(exArtINI, pArtSection, "CrouchedPrimaryFireFLH");
	this->CrouchedSecondaryFireFLH.Read(exArtINI, pArtSection, "CrouchedSecondaryFireFLH");
	this->DeployedPrimaryFireFLH.Read(exArtINI, pArtSection, "DeployedPrimaryFireFLH");
	this->DeployedSecondaryFireFLH.Read(exArtINI, pArtSection, "DeployedSecondaryFireFLH");

	this->ForceWeapon_Naval_Decloaked.Read(exINI, pSection, "ForceWeapon.Naval.Decloaked");

	this->Overload_Count.Read(exINI, pSection, "Overload.Count");
	this->Overload_Damage.Read(exINI, pSection, "Overload.Damage");
	this->Overload_Frames.Read(exINI, pSection, "Overload.Frames");
	this->Overload_DeathSound.Read(exINI, pSection, "Overload.DeathSound");
	this->Overload_ParticleSys.Read(exINI, pSection, "Overload.ParticleSys");
	this->Overload_ParticleSysCount.Read(exINI, pSection, "Overload.ParticleSysCount");
	this->Draw_MindControlLink.Read(exINI, pSection, "MindControll.DrawLink");
	this->SelfHealGainType.Read(exINI, pSection, "SelfHealGainType");

	this->DigitalDisplayType.Read(exINI, pSection, "DigitalDisplayType");
	this->DigitalDisplayType_Shield.Read(exINI, pSection, "DigitalDisplayType.Shield");
	this->HugeHP_Show.Read(exINI, pSection, "HugeHP.Show");
	this->HugeHP_Priority.Read(exINI, pSection, "HugeHP.Priority");

	this->IonCannonType.Read(exINI, pSection, "IonCannonType", true);

	this->FireSelf_Weapon.Read(exINI, pSection, "FireSelf.Weapon");
	this->FireSelf_ROF.Read(exINI, pSection, "FireSelf.ROF");
	this->FireSelf_Weapon_GreenHeath.Read(exINI, pSection, "FireSelf.Weapon.GreenHealth");
	this->FireSelf_ROF_GreenHeath.Read(exINI, pSection, "FireSelf.ROF.GreenHealth");
	this->FireSelf_Weapon_YellowHeath.Read(exINI, pSection, "FireSelf.Weapon.YellowHealth");
	this->FireSelf_ROF_YellowHeath.Read(exINI, pSection, "FireSelf.ROF.YellowHealth");
	this->FireSelf_Weapon_RedHeath.Read(exINI, pSection, "FireSelf.Weapon.RedHealth");
	this->FireSelf_ROF_RedHeath.Read(exINI, pSection, "FireSelf.ROF.RedHealth");

	this->Script_Fire.Read(pINI, pSection, "Script.Fire");
	this->Script_Fire_SelfCenter.Read(exINI, pSection, "Script.Fire.SelfCenter");

	this->HealthBar_Pips.Read(exINI, pSection, "HealthBar.Pips");
	this->HealthBar_Pips_DrawOffset.Read(exINI, pSection, "HealthBar.Pips.DrawOffset");
	this->HealthBar_PipsLength.Read(exINI, pSection, "HealthBar.PipsLength");
	this->HealthBar_PipsSHP.Read(pINI, pSection, "HealthBar.PipsSHP");
	this->HealthBar_PipsPAL.Read(pINI, pSection, "HealthBar.PipsPAL");
	this->HealthBar_PipBrd.Read(exINI, pSection, "HealthBar.PipBrd");
	this->HealthBar_PipBrdSHP.Read(pINI, pSection, "HealthBar.PipBrdSHP");
	this->HealthBar_PipBrdPAL.Read(pINI, pSection, "HealthBar.PipBrdPAL");
	this->HealthBar_XOffset.Read(exINI, pSection, "HealthBar.XOffset");

	this->GroupID_Offset.Read(exINI, pSection, "GroupID.ShowOffset");
	this->SelfHealPips_Offset.Read(exINI, pSection, "SelfHealPips.ShowOffset");
	this->UseCustomHealthBar.Read(exINI, pSection, "UseCustomHealthBar");

	this->GScreenAnimType.Read(exINI, pSection, "GScreenAnimType", true);
}

template <typename T>
void TechnoTypeExt::ExtData::Serialize(T& Stm)
{
	this->FireScriptType = nullptr;
	Stm
		.Process(this->HealthBar_Hide)
		.Process(this->UIDescription)
		.Process(this->LowSelectionPriority)
		.Process(this->MindControlRangeLimit)
		.Process(this->Interceptor)
		.Process(this->Interceptor_Rookie)
		.Process(this->Interceptor_Veteran)
		.Process(this->Interceptor_Elite)
		.Process(this->Interceptor_GuardRange)
		.Process(this->Interceptor_MinimumGuardRange)
		.Process(this->Interceptor_EliteGuardRange)
		.Process(this->Interceptor_EliteMinimumGuardRange)
		.Process(this->Interceptor_Success)
		.Process(this->Interceptor_RookieSuccess)
		.Process(this->Interceptor_VeteranSuccess)
		.Process(this->Interceptor_EliteSuccess)
		.Process(this->GroupAs)
		.Process(this->RadarJamRadius)
		.Process(this->InhibitorRange)
		.Process(this->TurretOffset)
		.Process(this->Powered_KillSpawns)
		.Process(this->Spawn_LimitedRange)
		.Process(this->Spawn_LimitedExtraRange)
		.Process(this->Harvester_Counted)
		.Process(this->Promote_IncludeSpawns)
		.Process(this->ImmuneToCrit)
		.Process(this->MultiMindControl_ReleaseVictim)
		.Process(this->CameoPriority)
		.Process(this->NoManualMove)
		.Process(this->InitialStrength)
		.Process(this->Death_NoAmmo)
		.Process(this->Death_Countdown)
		.Process(this->Death_Peaceful)
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
		.Process(this->WeaponBurstFLHs)
		.Process(this->EliteWeaponBurstFLHs)
		.Process(this->GiftBoxData)
		.Process(this->PassengerDeletion_Soylent)
		.Process(this->PassengerDeletion_SoylentFriendlies)
		.Process(this->PassengerDeletion_Rate)
		.Process(this->PassengerDeletion_ReportSound)
		.Process(this->PassengerDeletion_Rate_SizeMultiply)
		.Process(this->PassengerDeletion_Anim)
		.Process(this->OpenTopped_RangeBonus)
		.Process(this->OpenTopped_DamageMultiplier)
		.Process(this->OpenTopped_WarpDistance)
		.Process(this->AutoFire)
		.Process(this->AutoFire_TargetSelf)
		.Process(this->NoSecondaryWeaponFallback)
		.Process(this->NoAmmoWeapon)
		.Process(this->NoAmmoAmount)
		.Process(this->JumpjetAllowLayerDeviation)
		.Process(this->DeployingAnim_AllowAnyDirection)
		.Process(this->DeployingAnim_KeepUnitVisible)
		.Process(this->DeployingAnim_ReverseForUndeploy)
		.Process(this->DeployingAnim_UseUnitDrawer)
		.Process(this->EnemyUIName)
		.Process(this->ForceWeapon_Naval_Decloaked)
		.Process(this->Ammo_Shared)
		.Process(this->Ammo_Shared_Group)
		.Process(this->UseCustomSelectBrd)
		.Process(this->SelectBrd_SHP)
		.Process(this->SelectBrd_PAL)
		.Process(this->SelectBrd_Frame)
		.Process(this->SelectBrd_DrawOffset)
		.Process(this->CrouchedPrimaryFireFLH)
		.Process(this->CrouchedSecondaryFireFLH)
		.Process(this->DeployedPrimaryFireFLH)
		.Process(this->DeployedSecondaryFireFLH)
		.Process(this->CrouchedWeaponBurstFLHs)
		.Process(this->EliteCrouchedWeaponBurstFLHs)
		.Process(this->DeployedWeaponBurstFLHs)
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
		.Process(this->DigitalDisplayType)
		.Process(this->DigitalDisplayType_Shield)
		.Process(this->HugeHP_Show)
		.Process(this->HugeHP_Priority)
		.Process(this->IonCannonType)
		.Process(this->FireSelf_Weapon)
		.Process(this->FireSelf_ROF)
		.Process(this->FireSelf_Weapon_GreenHeath)
		.Process(this->FireSelf_ROF_GreenHeath)
		.Process(this->FireSelf_Weapon_YellowHeath)
		.Process(this->FireSelf_ROF_YellowHeath)
		.Process(this->FireSelf_Weapon_RedHeath)
		.Process(this->FireSelf_ROF_RedHeath)
		.Process(this->Script_Fire)
		.Process(this->Script_Fire_SelfCenter)
		//.Process(this->FireScriptType)
		.Process(this->HealthBar_Pips)
		.Process(this->HealthBar_Pips_DrawOffset)
		.Process(this->HealthBar_PipsLength)
		.Process(this->HealthBar_PipsSHP)
		.Process(this->HealthBar_PipsPAL)
		.Process(this->HealthBar_PipBrd)
		.Process(this->HealthBar_PipBrdSHP)
		.Process(this->HealthBar_PipBrdPAL)
		.Process(this->HealthBar_XOffset)
		.Process(this->GroupID_Offset)
		.Process(this->SelfHealPips_Offset)
		.Process(this->UseCustomHealthBar)
		.Process(this->GScreenAnimType)
		;
}
void TechnoTypeExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	TechnoTypeClass* oldPtr = nullptr;
	Stm.Load(oldPtr);
	PointerMapper::AddMapping(reinterpret_cast<long>(oldPtr), reinterpret_cast<long>(this->OwnerObject()));
	Debug::Log("[TechnoTypeClass] {%s} oldPtr[0x%X],newPtr[0x%X]\n", this->OwnerObject()->get_ID(), oldPtr, this->OwnerObject());
	ExistTechnoTypeExt::Array.push_back(this);

	Extension<TechnoTypeClass>::LoadFromStream(Stm);
	this->Serialize(Stm);

	size_t AttachmentDataSize = 0;
	Stm.Load(AttachmentDataSize);

	std::vector<ExtData::AttachmentDataEntry*> oldPtrs;

	for (size_t i = 0; i < AttachmentDataSize; i++)
	{
		ExtData::AttachmentDataEntry* pOld = nullptr;
		Stm.Load(pOld);
		oldPtrs.emplace_back(pOld);
		int AttachmentIdx = -1;
		TechnoTypeClass* pTmp = nullptr;
		bool IsOnTurret = false;
		CoordStruct FLH = { 0, 0, 0 };
		Stm.Load(AttachmentIdx);
		Stm.Load(pTmp);
		Stm.Load(IsOnTurret);
		Stm.Load(FLH);
		Debug::Log("[AttachmentDataEntry] Read a Data[{%d,0x%X}]\n", AttachmentIdx, pTmp);
		AttachmentData.emplace_back(AttachmentIdx, pTmp, FLH, IsOnTurret);
	}
	for (size_t i = 0; i < AttachmentData.size(); i++)
	{
		PointerMapper::AddMapping(reinterpret_cast<long>(oldPtrs[i]), reinterpret_cast<long>(&AttachmentData[i]));
		Debug::Log("[AttachmentDataEntry] old[0x%X],new[0x%X]\n", oldPtrs[i], &AttachmentData.back());
	}
	if (TechnoTypeClass::Array->Count == ExtData::counter)
	{
		//AttachmentClass::LoadGlobals(Stm);
	}
}

void TechnoTypeExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Stm.Save(this->OwnerObject());

	Extension<TechnoTypeClass>::SaveToStream(Stm);
	this->Serialize(Stm);

	Stm.Save(this->AttachmentData.size());
	for (auto& it : AttachmentData)
	{
		Stm.Save(&it);
		Stm.Save(it.Type.Get());
		TechnoTypeClass* pTmp = it.TechnoType[0];
		Stm.Save(pTmp);
		Stm.Save(it.IsOnTurret.Get());
		Stm.Save(it.FLH.Get());
	}
	ExtData::counter++;
	if (ExtData::counter == TechnoTypeClass::Array->Count)
	{
		//AttachmentClass::SaveGlobals(Stm);
	}
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
	this->TechnoType.Load(stm, false);
	//int TypeIdx = -1;
	//int TechnoTypeIdx = -1;
	//CoordStruct FLH = { 0,0,0 };
	//bool IsOnTurret = false;
	//stm.Load(TypeIdx);
	//stm.Load(TechnoTypeIdx);
	//stm.Load(FLH);
	//stm.Load(IsOnTurret);
	this->Serialize(stm);
	Debug::Log("[AttachmentDataEntry] Finish Load Type[%d],TechnoType.size[%d]\n", this->Type.Get(), this->TechnoType.size());
	return stm.Success();
}

bool TechnoTypeExt::ExtData::AttachmentDataEntry::Save(PhobosStreamWriter& stm) const
{
	stm.Save(this);
	this->TechnoType.Save(stm);
	//stm.Save(this->Type.Get());
	//stm.Save(this->TechnoType.Get());
	//stm.Save(this->FLH.Get());
	//stm.Save(this->IsOnTurret.Get());
	return const_cast<AttachmentDataEntry*>(this)->Serialize(stm);
}

template <typename T>
bool TechnoTypeExt::ExtData::AttachmentDataEntry::Serialize(T& stm)
{
	return stm
		.Process(this->Type)
		.Process(this->FLH)
		.Process(this->IsOnTurret)
		.Success();
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