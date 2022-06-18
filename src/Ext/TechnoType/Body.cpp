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
			else if (veteranFLH.isset() && !eliteFLH.isset())
				eliteFLH = veteranFLH;
			else if (!FLH.isset() && !eliteFLH.isset() && !veteranFLH.isset())
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

TechnoTypeClass* TechnoTypeExt::GetTechnoType(ObjectTypeClass* pType)
{
	if (pType->AbsID == AbstractType::AircraftType || pType->AbsID == AbstractType::BuildingType ||
		pType->AbsID == AbstractType::InfantryType || pType->AbsID == AbstractType::UnitType)
	{
		return static_cast<TechnoTypeClass*>(pType);
	}

	return nullptr;
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
	this->Spawner_LimitRange.Read(exINI, pSection, "Spawner.LimitRange");
	this->Spawner_ExtraLimitRange.Read(exINI, pSection, "Spawner.ExtraLimitRange");
	this->Spawner_DelayFrames.Read(exINI, pSection, "Spawner.DelayFrames");
	this->Harvester_Counted.Read(exINI, pSection, "Harvester.Counted");
	this->Promote_IncludeSpawns.Read(exINI, pSection, "Promote.IncludeSpawns");
	this->ImmuneToCrit.Read(exINI, pSection, "ImmuneToCrit");
	this->MultiMindControl_ReleaseVictim.Read(exINI, pSection, "MultiMindControl.ReleaseVictim");
	this->NoManualMove.Read(exINI, pSection, "NoManualMove");
	this->InitialStrength.Read(exINI, pSection, "InitialStrength");
	this->Death_NoAmmo.Read(exINI, pSection, "Death.NoAmmo");
	this->Death_Countdown.Read(exINI, pSection, "Death.Countdown");
	this->Death_Peaceful.Read(exINI, pSection, "Death.Peaceful");
	this->Death_WithMaster.Read(exINI, pSection, "Death.WithSlaveOwner");
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

	TechnoTypeExt::GetWeaponCounts(pThis, exINI, pSection, Weapons, VeteranWeapons, EliteWeapons);
	TechnoTypeExt::GetWeaponStages(pThis, exINI, pSection, Stages, VeteranStages, EliteStages);
	TechnoTypeExt::GetWeaponFLHs(pThis, exArtINI, pArtSection, WeaponFLHs, VeteranWeaponFLHs, EliteWeaponFLHs);

	TechnoTypeExt::GetBurstFLHs(pThis, exArtINI, pArtSection, WeaponBurstFLHs, VeteranWeaponBurstFLHs, EliteWeaponBurstFLHs, "");
	TechnoTypeExt::GetBurstFLHs(pThis, exArtINI, pArtSection, DeployedWeaponBurstFLHs, VeteranDeployedWeaponBurstFLHs, EliteDeployedWeaponBurstFLHs, "Deployed");
	TechnoTypeExt::GetBurstFLHs(pThis, exArtINI, pArtSection, CrouchedWeaponBurstFLHs, VeteranCrouchedWeaponBurstFLHs, EliteCrouchedWeaponBurstFLHs, "Prone");

	this->UseCustomSelectBox.Read(exINI, pSection, "UseCustomSelectBox");
	this->SelectBox_SHP.Read(pINI, pSection, "SelectBox.SHP");
	this->SelectBox_PAL.Read(pINI, pSection, "SelectBox.PAL");
	this->SelectBox_Frame.Read(exINI, pSection, "SelectBox.Frame");
	this->SelectBox_DrawOffset.Read(exINI, pSection, "SelectBox.DrawOffset");
	this->SelectBox_TranslucentLevel.Read(exINI, pSection, "SelectBox.TranslucentLevel");
	this->SelectBox_ShowEnemy.Read(exINI, pSection, "SelectBox.ShowEnemy");

	this->PronePrimaryFireFLH.Read(exArtINI, pArtSection, "PronePrimaryFireFLH");
	this->ProneSecondaryFireFLH.Read(exArtINI, pArtSection, "ProneSecondaryFireFLH");
	this->DeployedPrimaryFireFLH.Read(exArtINI, pArtSection, "DeployedPrimaryFireFLH");
	this->DeployedSecondaryFireFLH.Read(exArtINI, pArtSection, "DeployedSecondaryFireFLH");

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

	this->Insignia.Read(exINI, pSection, "Insignia.%s");
	this->InsigniaFrames.Read(exINI, pSection, "InsigniaFrames");
	this->InsigniaFrame.Read(exINI, pSection, "InsigniaFrame.%s");
	this->Insignia_ShowEnemy.Read(exINI, pSection, "Insignia.ShowEnemy");

	this->InitialStrength_Cloning.Read(exINI, pSection, "InitialStrength.Cloning");

	this->DigitalDisplayTypes.Read(exINI, pSection, "DigitalDisplayTypes");
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
	this->UseNewHealthBar.Read(exINI, pSection, "UseNewHealthBar");
	this->HealthBar_PictureSHP.Read(pINI, pSection, "HealthBar.PictureSHP");
	this->HealthBar_PicturePAL.Read(pINI, pSection, "HealthBar.PicturePAL");
	this->HealthBar_PictureTransparency.Read(exINI, pSection, "HealthBar.PictureTransparency");


	this->GroupID_Offset.Read(exINI, pSection, "GroupID.ShowOffset");
	this->SelfHealPips_Offset.Read(exINI, pSection, "SelfHealPips.ShowOffset");
	this->UseCustomHealthBar.Read(exINI, pSection, "UseCustomHealthBar");

	this->GScreenAnimType.Read(exINI, pSection, "GScreenAnimType", true);
	
	this->RandomProduct.Read(exINI, pSection, "RandomProduct");

	this->MovePassengerToSpawn.Read(exINI, pSection, "MovePassengerToSpawn");
	this->SilentPassenger.Read(exINI, pSection, "SilentPassenger");
	this->Spawner_SameLoseTarget.Read(exINI, pSection, "Spawner.SameLoseTarget");

	this->DeterminedByRange.Read(exINI, pSection, "DeterminedByRange");
	this->DeterminedByRange_ExtraRange.Read(exINI, pSection, "DeterminedByRange.ExtraRange");
	this->DeterminedByRange_MainWeapon.Read(exINI, pSection, "DeterminedByRange.MainWeapon");

	this->BuildLimit_Group_Types.Read(exINI, pSection, "BuildLimit.Group.Types");
	this->BuildLimit_Group_Any.Read(exINI, pSection, "BuildLimit.Group.Any");
	this->BuildLimit_Group_Limits.Read(exINI, pSection, "BuildLimit.Group.Limits");

	this->VehicleImmuneToMindControl.Read(exINI, pSection, "VehicleImmuneToMindControl");

	this->Convert_Deploy.Read(exINI, pSection, "Convert.Deploy");
	this->Convert_DeployAnim.Read(exINI, pSection, "Convert.DeployAnim");

	this->IsExtendGattling.Read(exINI, pSection, "IsExtendGattling");
	this->Gattling_Cycle.Read(exINI, pSection, "Gattling.Cycle");
	this->Gattling_Charge.Read(exINI, pSection, "Gattling.Charge");

	this->Primary.Read(exINI, pSection, "Primary");
	this->Secondary.Read(exINI, pSection, "Secondary");
	this->VeteranPrimary.Read(exINI, pSection, "VeteranPrimary");
	this->VeteranSecondary.Read(exINI, pSection, "VeteranSecondary");
	this->ElitePrimary.Read(exINI, pSection, "ElitePrimary");
	this->EliteSecondary.Read(exINI, pSection, "EliteSecondary");

	this->JJConvert_Unload.Read(exINI, pSection, "JJConvert.Unload");
	this->CanBeIronCurtain.Read(exINI, pSection, "CanBeIronCurtain");
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
		.Process(this->Spawner_DelayFrames)
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
		.Process(this->Death_WithMaster)
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
		.Process(this->UseCustomSelectBox)
		.Process(this->SelectBox_SHP)
		.Process(this->SelectBox_PAL)
		.Process(this->SelectBox_Frame)
		.Process(this->SelectBox_DrawOffset)
		.Process(this->SelectBox_TranslucentLevel)
		.Process(this->SelectBox_ShowEnemy)
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
		.Process(this->DigitalDisplayTypes)
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
		.Process(this->HealthBar_Pips)
		.Process(this->HealthBar_Pips_DrawOffset)
		.Process(this->HealthBar_PipsLength)
		.Process(this->HealthBar_PipsSHP)
		.Process(this->HealthBar_PipsPAL)
		.Process(this->HealthBar_PipBrd)
		.Process(this->HealthBar_PipBrdSHP)
		.Process(this->HealthBar_PipBrdPAL)
		.Process(this->HealthBar_XOffset)
		.Process(this->UseNewHealthBar)
		.Process(this->HealthBar_PictureSHP)
		.Process(this->HealthBar_PicturePAL)
		.Process(this->HealthBar_PictureTransparency)
		.Process(this->GroupID_Offset)
		.Process(this->SelfHealPips_Offset)
		.Process(this->UseCustomHealthBar)
		.Process(this->GScreenAnimType)
		.Process(this->MovePassengerToSpawn)
		.Process(this->SilentPassenger)
		.Process(this->Spawner_SameLoseTarget)
		.Process(this->DeterminedByRange)
		.Process(this->DeterminedByRange_ExtraRange)
		.Process(this->DeterminedByRange_MainWeapon)
		.Process(this->BuildLimit_Group_Types)
		.Process(this->BuildLimit_Group_Any)
		.Process(this->BuildLimit_Group_Limits)
		.Process(this->VehicleImmuneToMindControl)
		.Process(this->Convert_Deploy)
		.Process(this->Convert_DeployAnim)
		.Process(this->IsExtendGattling)
		.Process(this->Gattling_Cycle)
		.Process(this->Gattling_Charge)
		.Process(this->Weapons)
		.Process(this->VeteranWeapons)
		.Process(this->EliteWeapons)
		.Process(this->Stages)
		.Process(this->VeteranStages)
		.Process(this->EliteStages)
		.Process(this->WeaponFLHs)
		.Process(this->VeteranWeaponFLHs)
		.Process(this->EliteWeaponFLHs)
		.Process(this->Primary)
		.Process(this->Secondary)
		.Process(this->VeteranPrimary)
		.Process(this->VeteranSecondary)
		.Process(this->ElitePrimary)
		.Process(this->EliteSecondary)
		.Process(this->JJConvert_Unload)
		.Process(this->CanBeIronCurtain)
		;
}

void TechnoTypeExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	TechnoTypeClass* oldPtr = nullptr;
	Stm.Load(oldPtr);
	PointerMapper::AddMapping(oldPtr, this->OwnerObject());
	Extension<TechnoTypeClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void TechnoTypeExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Stm.Save(this->OwnerObject());
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
		.Process(this->TechnoType)
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