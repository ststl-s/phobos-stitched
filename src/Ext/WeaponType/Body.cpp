#include "Body.h"

#include <BulletClass.h>
#include <BulletTypeClass.h>
#include <Conversions.h>
#include <GameStrings.h>

#include <Ext/Techno/AresExtData.h>
#include <Ext/Techno/Body.h>

#include <New/Entity/StrafingLaserClass.h>
#include <New/Type/RadTypeClass.h>

#include <Misc/PhobosGlobal.h>

#include <Utilities/EnumFunctions.h>
#include <Utilities/GeneralUtils.h>
#include <Utilities/Helpers.Alex.h>
#include <Utilities/TemplateDef.h>
#include <Utilities/SavegameDef.h>

template<> const DWORD Extension<WeaponTypeClass>::Canary = 0x22222222;
WeaponTypeExt::ExtContainer WeaponTypeExt::ExtMap;

bool WeaponTypeExt::ExtData::HasRequiredAttachedEffects(TechnoClass* pTechno, TechnoClass* pFirer)
{
	bool hasRequiredTypes = this->AttachEffect_RequiredTypes.size() > 0;
	bool hasDisallowedTypes = this->AttachEffect_DisallowedTypes.size() > 0;

	if (hasRequiredTypes || hasDisallowedTypes)
	{
		auto const pTechnoExt = TechnoExt::ExtMap.Find(pTechno);

		if (hasDisallowedTypes && pTechnoExt->HasAttachedEffects(this->AttachEffect_DisallowedTypes, false, this->AttachEffect_IgnoreFromSameSource, pFirer, this->OwnerObject()->Warhead))
			return false;

		if (hasRequiredTypes && !pTechnoExt->HasAttachedEffects(this->AttachEffect_RequiredTypes, true, this->AttachEffect_IgnoreFromSameSource, pFirer, this->OwnerObject()->Warhead))
			return false;
	}

	return true;
}

void WeaponTypeExt::ExtData::AddStrafingLaser(TechnoClass* pThis, AbstractClass* pTarget, int IdxWeapon)
{
	if (!pTarget)
		return;

	if (this->StrafingLasers.empty())
		return;

	const auto pExt = TechnoExt::ExtMap.Find(pThis);
	if (!pExt)
		return;

	const auto pTechno = abstract_cast<TechnoClass*>(pTarget);
	CoordStruct targetCoord = pTechno ?
		pTechno->WhatAmI() == AbstractType::Building ? pTechno->GetCenterCoords() : pTechno->GetCoords() :
		pTarget->GetCoords();
	if (pTechno && pTechno->WhatAmI() == AbstractType::Building)
	{
		const auto targetCoordOffset = abstract_cast<BuildingClass*>(pTechno)->Type->TargetCoordOffset;
		targetCoord += targetCoordOffset;
	}

	auto addLaser = [pThis, pExt, pTarget, IdxWeapon, targetCoord](StrafingLaserTypeClass* pStrafingLaser)
	{
		if (!pStrafingLaser)
			return false;

		if ((!pStrafingLaser->InnerColor.isset() && !pStrafingLaser->IsHouseColor.Get()) ||
			   pStrafingLaser->Duration.Get() <= 0 ||
			   pStrafingLaser->Timer.Get() <= 0)
			return false;

		bool inground = pStrafingLaser->InGround.Get(!pTarget->IsInAir());
		CoordStruct flh = TechnoExt::GetFLHAbsoluteCoords(pThis, pStrafingLaser->FLH.Get(TechnoExt::GetFLH(pThis, IdxWeapon)),
			pThis->GetTechnoType()->Turret);

		CoordStruct source = pStrafingLaser->SourceFromTarget.Get() ?
			targetCoord : pThis->WhatAmI() == AbstractType::Building ?
			pThis->GetCenterCoords() : pThis->GetCoords();
		CoordStruct target = targetCoord;

		if (pStrafingLaser->SourceFLH.isset())
		{
			source = TechnoExt::GetFLHAbsoluteCoords(pThis, pStrafingLaser->SourceFLH.Get(), source, pThis->GetTechnoType()->Turret);
		}

		if (pStrafingLaser->TargetFLH.isset())
		{
			target = TechnoExt::GetFLHAbsoluteCoords(pThis, pStrafingLaser->TargetFLH.Get(), targetCoord, pThis->GetTechnoType()->Turret);
		}

		if (inground)
		{
			const auto nCell = CellClass::Coord2Cell(source);
			const auto pCell = MapClass::Instance->TryGetCellAt(nCell);
			if (pCell)
			{
				source.Z = pCell->GetCoordsWithBridge().Z;
			}

			const auto nCell2 = CellClass::Coord2Cell(target);
			const auto pCell2 = MapClass::Instance->TryGetCellAt(nCell2);
			if (pCell2)
			{
				target.Z = pCell2->GetCoordsWithBridge().Z;
			}
		}

		auto innerColor = pStrafingLaser->IsHouseColor.Get() ? pThis->Owner->Color : pStrafingLaser->InnerColor.Get();
		auto outercolor = pStrafingLaser->OuterColor.Get();
		auto outerspread = pStrafingLaser->OuterSpread.Get();

		LaserDrawClass* pLaser = GameCreate<LaserDrawClass>(
			flh,
			source,
			innerColor,
			outercolor,
			outerspread,
			1);

		pLaser->Fades = false;
		pLaser->IsHouseColor = (pStrafingLaser->IsHouseColor.Get() || pStrafingLaser->IsSingleColor.Get()) ? true : false;
		pLaser->IsSupported = pStrafingLaser->IsSupported.Get(pStrafingLaser->Thickness.Get() > 3) ? true : false;
		pLaser->Thickness = pStrafingLaser->Thickness.Get();

		if (const auto pWeapon = pStrafingLaser->Weapon.Get())
			WeaponTypeExt::DetonateAt(pWeapon, source, pThis);

		pExt->StrafingLasers.emplace_back(std::make_unique<StrafingLaserClass>(
			pStrafingLaser,
			pTarget,
			inground,
			Unsorted::CurrentFrame,
			pStrafingLaser->FLH.Get(TechnoExt::GetFLH(pThis, IdxWeapon)),
			source,
			target));

		return true;
	};

	if (this->StrafingLasers_Random.Get())
	{
		int count = this->StrafingLasers.size() - 1;
		int idx = ScenarioClass::Instance->Random.RandomRanged(0, count);

		addLaser(this->StrafingLasers.at(idx));
	}
	else
	{
		for (const auto pStrafingLaser : this->StrafingLasers)
		{
			if (!addLaser(pStrafingLaser))
				continue;
		}
	}
}

void WeaponTypeExt::ExtData::Initialize()
{
	this->RadType = RadTypeClass::FindOrAllocate(GameStrings::Radiation);
}

// =============================
// load / save

void WeaponTypeExt::ExtData::LoadFromINIFile(CCINIClass* const pINI)
{
	auto pThis = this->OwnerObject();
	const char* pSection = pThis->ID;

	if (!pINI->GetSection(pSection))
		return;

	INI_EX exINI(pINI);

	this->DiskLaser_Radius.Read(exINI, pSection, "DiskLaser.Radius");
	this->DiskLaser_Circumference = (int)(this->DiskLaser_Radius * Math::Pi * 2);

	this->LaserThickness.Read(exINI, pSection, "LaserThickness");

	this->Bolt_Disable1.Read(exINI, pSection, "Bolt.Disable1");
	this->Bolt_Disable2.Read(exINI, pSection, "Bolt.Disable2");
	this->Bolt_Disable3.Read(exINI, pSection, "Bolt.Disable3");

	this->RadType.Read(exINI, pSection, "RadType", true);

	this->Strafing_Shots.Read(exINI, pSection, "Strafing.Shots");
	this->Strafing_SimulateBurst.Read(exINI, pSection, "Strafing.SimulateBurst");
	this->CanTarget.Read(exINI, pSection, "CanTarget");
	this->CanTargetHouses.Read(exINI, pSection, "CanTargetHouses");
	this->Burst_Delays.Read(exINI, pSection, "Burst.Delays");
	this->AreaFire_Target.Read(exINI, pSection, "AreaFire.Target");
	this->FeedbackWeapon.Read(exINI, pSection, "FeedbackWeapon", true);
	this->Laser_IsSingleColor.Read(exINI, pSection, "IsSingleColor");
	this->ROF_RandomDelay.Read(exINI, pSection, "ROF.RandomDelay");

	this->AttachEffect_RequiredTypes.Read(exINI, pSection, "AttachEffect.RequiredTypes");
	this->AttachEffect_DisallowedTypes.Read(exINI, pSection, "AttachEffect.DisallowedTypes");
	this->AttachEffect_IgnoreFromSameSource.Read(exINI, pSection, "AttachEffect.IgnoreFromSameSource");

	this->BlinkWeapon.Read(exINI, pSection, "BlinkWeapon");
	this->InvBlinkWeapon.Read(exINI, pSection, "InvBlinkWeapon");
	this->BlinkWeapon_KillTarget.Read(exINI, pSection, "BlinkWeapon.KillTarget");
	this->BlinkWeapon_Overlap.Read(exINI, pSection, "BlinkWeapon.Overlap");
	this->BlinkWeapon_SelfAnim.Read(exINI, pSection, "BlinkWeapon.SelfAnim");
	this->BlinkWeapon_TargetAnim.Read(exINI, pSection, "BlinkWeapon.TargetAnim");

	this->IonCannonType.Read(exINI, pSection, "IonCannonType", true);

	this->IsBeamCannon.Read(exINI, pSection, "IsBeamCannon");
	this->BeamCannon_Length.Read(exINI, pSection, "BeamCannon.Length");
	this->BeamCannon_Length_Start.Read(exINI, pSection, "BeamCannon.Length.Start");
	this->BeamCannon_LengthIncrease.Read(exINI, pSection, "BeamCannon.LengthIncrease");
	this->BeamCannon_LengthIncreaseAcceleration.Read(exINI, pSection, "BeamCannon.LengthIncrease.Acceleration");
	this->BeamCannon_LengthIncreaseMax.Read(exINI, pSection, "BeamCannon.LengthIncrease.Max");
	this->BeamCannon_LengthIncreaseMin.Read(exINI, pSection, "BeamCannon.LengthIncrease.Min");
	this->BeamCannon_DrawEBolt.Read(exINI, pSection, "BeamCannon.DrawEBolt");
	this->BeamCannon_EleHeight.Read(exINI, pSection, "BeamCannon.EleHeight");
	this->BeamCannon_InnerColor.Read(exINI, pSection, "BeamCannon.InnerColor");
	this->BeamCannon_OuterColor.Read(exINI, pSection, "BeamCannon.OuterColor");
	this->BeamCannon_OuterSpread.Read(exINI, pSection, "BeamCannon.OuterSpread");
	this->BeamCannon_Duration.Read(exINI, pSection, "BeamCannon.Duration");
	this->BeamCannon_Thickness.Read(exINI, pSection, "BeamCannon.Thickness");
	this->BeamCannonWeapon.Read(exINI, pSection, "BeamCannon.Weapon", true);
	this->BeamCannon_DrawLaser.Read(exINI, pSection, "BeamCannon.DrawLaser");
	this->BeamCannon_LaserHeight.Read(exINI, pSection, "BeamCannon.LaserHeight");
	this->BeamCannon_DrawFromSelf.Read(exINI, pSection, "BeamCannon.DrawFromSelf");
	this->BeamCannon_DrawFromSelf_FLH.Read(exINI, pSection, "BeamCannon.DrawFromSelf.FLH");
	this->BeamCannon_ROF.Read(exINI, pSection, "BeamCannon.ROF");
	this->BeamCannon_Burst.Read(exINI, pSection, "BeamCannon.Burst");

	for (int i = 0; i < BeamCannon_Burst; i++)
	{
		char key[0x20];
		Nullable<CoordStruct> BeamCannonFLH;
		sprintf(key, "BeamCannon.DrawFromSelf.FLH.Burst%d", i);
		BeamCannonFLH.Read(exINI, pSection, key);

		Nullable<CoordStruct> BeamCannonStart;
		sprintf(key, "BeamCannon.Length.Start.Burst%d", i);
		BeamCannonStart.Read(exINI, pSection, key);

		if (!BeamCannonFLH.isset())
			BeamCannonFLH = BeamCannon_DrawFromSelf_FLH;

		if (!BeamCannonStart.isset())
			BeamCannonStart = BeamCannon_Length_Start;

		BeamCannon_FLH.emplace_back(BeamCannonFLH.Get());
		BeamCannon_Start.emplace_back(BeamCannonStart.Get());
	}

	this->PassengerDeletion.Read(exINI, pSection, "PassengerDeletion");
	this->PassengerTransport.Read(exINI, pSection, "PassengerTransport");
	this->PassengerTransport_UsePassengerData.Read(exINI, pSection, "PassengerTransport.UsePassengerData");
	this->PassengerTransport_Overlap.Read(exINI, pSection, "PassengerTransport.Overlap");
	this->PassengerTransport_MoveToTarget.Read(exINI, pSection, "PassengerTransport.MoveToTarget");
	this->PassengerTransport_MoveToTargetAllowHouses.Read(exINI, pSection, "PassengerTransport.MoveToTargetAllowHouses");
	this->PassengerTransport_MoveToTargetAllowHouses_IgnoreNeturalHouse.Read(exINI, pSection, "PassengerTransport.MoveToTargetAllowHouses.IgnoreNeturalHouse");
	this->PassengerTransport_UseParachute.Read(exINI, pSection, "PassengerTransport.UseParachute");

	this->SelfTransport.Read(exINI, pSection, "SelfTransport");
	this->SelfTransport_UseData.Read(exINI, pSection, "SelfTransport.UseData");
	this->SelfTransport_Overlap.Read(exINI, pSection, "SelfTransport.Overlap");
	this->SelfTransport_Anim.Read(exINI, pSection, "SelfTransport.Anim");
	this->SelfTransport_MoveToTarget.Read(exINI, pSection, "SelfTransport.MoveToTarget");
	this->SelfTransport_MoveToTargetAllowHouses.Read(exINI, pSection, "SelfTransport.MoveToTargetAllowHouses");
	this->SelfTransport_MoveToTargetAllowHouses_IgnoreNeturalHouse.Read(exINI, pSection, "SelfTransport.MoveToTargetAllowHouses.IgnoreNeturalHouse");
	this->SelfTransport_UseParachute.Read(exINI, pSection, "SelfTransport.UseParachute");

	this->FacingTarget.Read(exINI, pSection, "FacingTarget");
	this->KickOutPassenger.Read(exINI, pSection, "KickOutPassenger");

	this->AttachWeapons.Read(exINI, pSection, "AttachWeapons");
	this->AttachWeapons_Burst_InvertL.Read(exINI, pSection, "AttachWeapons.Burst.InvertL");
	this->AttachWeapons_DetachedROF.Read(exINI, pSection, "AttachWeapons.DetachedROF");
	this->AttachWeapons_UseAmmo.Read(exINI, pSection, "AttachWeapons.UseAmmo");
	this->AttachWeapons_DetachedFire.Read(exINI, pSection, "AttachWeapons.DetachedFire");

	for (size_t i = 0; i < AttachWeapons.size(); i++)
	{
		char key[0x20];
		Valueable<CoordStruct> crdFLH;
		sprintf(key, "AttachWeapon%d.FLH", i);
		crdFLH.Read(exINI, pSection, key);
		AttachWeapons_FLH.emplace_back(crdFLH.Get());
	}

	this->OnlyAllowOneFirer.Read(exINI, pSection, "OnlyAllowOneFirer");
	this->OnlyAllowOneFirer_Count.Read(exINI, pSection, "OnlyAllowOneFirer.Count");
	this->OnlyAllowOneFirer_OtherWeapons.Read(exINI, pSection, "OnlyAllowOneFirer.OtherWeapons");
	this->OnlyAllowOneFirer_IgnoreWeapons.Read(exINI, pSection, "OnlyAllowOneFirer.IgnoreWeapons");
	this->OnlyAllowOneFirer_ResetImmediately.Read(exINI, pSection, "OnlyAllowOneFirer.ResetImmediately");

	this->AttachAttachment_SelfToTarget.Read(exINI, pSection, "AttachAttachment.SelfToTarget");
	this->AttachAttachment_TargetToSelf.Read(exINI, pSection, "AttachAttachment.TargetToSelf");
	this->AttachAttachment_Type.Read(exINI, pSection, "AttachAttachment.Type");
	this->AttachAttachment_FLH.Read(exINI, pSection, "AttachAttachment.FLH");
	this->AttachAttachment_IsOnTurret.Read(exINI, pSection, "AttachAttachment.IsOnTurret");

	this->Ammo.Read(exINI, pSection, "Ammo");

	this->ExtraBurst.Read(exINI, pSection, "ExtraBurst");
	this->ExtraBurst_Weapon.Read(exINI, pSection, "ExtraBurst.Weapon");
	this->ExtraBurst_Houses.Read(exINI, pSection, "ExtraBurst.Houses");
	this->ExtraBurst_AlwaysFire.Read(exINI, pSection, "ExtraBurst.AlwaysFire");
	this->ExtraBurst_FacingRange.Read(exINI, pSection, "ExtraBurst.FacingRange");
	this->ExtraBurst_InvertL.Read(exINI, pSection, "ExtraBurst.InvertL");
	this->ExtraBurst_Spread.Read(exINI, pSection, "ExtraBurst.Spread");
	this->ExtraBurst_UseAmmo.Read(exINI, pSection, "ExtraBurst.UseAmmo");
	this->ExtraBurst_SkipNeutralTarget.Read(exINI, pSection, "ExtraBurst.SkipNeutralTarget");
	for (int i = 0; i < ExtraBurst; i++)
	{
		char key[0x20];
		Valueable<CoordStruct> crdFLH;
		sprintf(key, "ExtraBurst%d.FLH", i);
		crdFLH.Read(exINI, pSection, key);
		ExtraBurst_FLH.emplace_back(crdFLH.Get());
	}

	//电流激光（渣效果[哭]）
	this->ElectricLaser.Read(exINI, pSection, "IsElectricLaser");
	this->ElectricLaser_Count.Read(exINI, pSection, "ElectricLaser.Count");
	this->ElectricLaser_Length.Read(exINI, pSection, "ElectricLaser.Length");
	this->ElectricLaser_Timer.Read(exINI, pSection, "ElectricLaser.Timer");

	for (size_t i = 0; static_cast<int>(i) < this->ElectricLaser_Count; ++i)
	{
		char colortext[64];
		Nullable<ColorStruct> color;
		_snprintf_s(colortext, sizeof(colortext), "ElectricLaser%u.Color", i);
		color.Read(exINI, pSection, colortext);

		char amplitudetext[64];
		Nullable<float> amplitude;
		_snprintf_s(amplitudetext, sizeof(amplitudetext), "ElectricLaser%u.Amplitude", i);
		amplitude.Read(exINI, pSection, amplitudetext);

		char durationtext[64];
		Nullable<int> duration;
		_snprintf_s(durationtext, sizeof(durationtext), "ElectricLaser%u.Duration", i);
		duration.Read(exINI, pSection, durationtext);

		char thicknesstext[64];
		Nullable<int> thickness;
		_snprintf_s(thicknesstext, sizeof(thicknesstext), "ElectricLaser%u.Thickness", i);
		thickness.Read(exINI, pSection, thicknesstext);

		char issupportedtext[64];
		Nullable<bool> issupported;
		_snprintf_s(issupportedtext, sizeof(issupportedtext), "ElectricLaser%u.IsSupported", i);
		issupported.Read(exINI, pSection, issupportedtext);

		if (color.isset())
		{
			if (i < this->ElectricLaser_Color.size())
				this->ElectricLaser_Color[i] = color;
			else
				this->ElectricLaser_Color.emplace_back(color);
		}
		else if (i >= this->ElectricLaser_Color.size())
		{
			this->ElectricLaser_Color.emplace_back(ColorStruct(128, 128, 128));
		}

		if (amplitude.isset())
		{
			if (i < this->ElectricLaser_Amplitude.size())
				this->ElectricLaser_Amplitude[i] = amplitude;
			else
				this->ElectricLaser_Amplitude.emplace_back(amplitude);
		}
		else if (i >= this->ElectricLaser_Amplitude.size())
		{
			this->ElectricLaser_Amplitude.emplace_back(10.0);
		}

		if (duration.isset())
		{
			if (i < this->ElectricLaser_Duration.size())
				this->ElectricLaser_Duration[i] = duration;
			else
				this->ElectricLaser_Duration.emplace_back(duration);
		}
		else if (i >= this->ElectricLaser_Duration.size())
		{
			this->ElectricLaser_Duration.emplace_back(15);
		}

		if (thickness.isset())
		{
			if (i < this->ElectricLaser_Thickness.size())
				this->ElectricLaser_Thickness[i] = thickness;
			else
				this->ElectricLaser_Thickness.emplace_back(thickness);
		}
		else if (i >= this->ElectricLaser_Thickness.size())
		{
			this->ElectricLaser_Thickness.emplace_back(2);
		}

		if (issupported.isset())
		{
			if (i < this->ElectricLaser_IsSupported.size())
				this->ElectricLaser_IsSupported[i] = issupported;
			else
				this->ElectricLaser_IsSupported.emplace_back(issupported);
		}
		else if (i >= this->ElectricLaser_IsSupported.size())
		{
			this->ElectricLaser_IsSupported.emplace_back(false);
		}
	}

	this->DelayedFire_Anim.Read(exINI, pSection, "DelayedFire.Anim");
	this->DelayedFire_Anim_LoopCount.Read(exINI, pSection, "DelayedFire.Anim.LoopCount");
	this->DelayedFire_Anim_UseFLH.Read(exINI, pSection, "DelayedFire.Anim.UseFLH");
	this->DelayedFire_DurationTimer.Read(exINI, pSection, "DelayedFire.DurationTimer");

	this->ExtraWarheads.Read(exINI, pSection, "ExtraWarheads");
	this->ExtraWarheads_DamageOverrides.Read(exINI, pSection, "ExtraWarheads.DamageOverrides");

	this->Laser_Thickness.Read(exINI, pSection, "LaserThickness");
	this->IsTrackingLaser.Read(exINI, pSection, "IsTrackingLaser");

	StrafingLaserTypeClass::AddNewINIList(pINI, pSection, "StrafingLasers");
	this->StrafingLasers.Read(exINI, pSection, "StrafingLasers");
	this->StrafingLasers_Random.Read(exINI, pSection, "StrafingLasers.Random");

	this->DecloakToFire.Read(exINI, pSection, "DecloakToFire");

	this->EBolt_Color1.Read(exINI, pSection, "Bolt.Color1");
	this->EBolt_Color2.Read(exINI, pSection, "Bolt.Color2");
	this->EBolt_Color3.Read(exINI, pSection, "Bolt.Color3");

	this->Beam_Color.Read(exINI, pSection, "Beam.Color");
	this->Beam_IsHouseColor.Read(exINI, pSection, "Beam.IsHouseColor");
	this->Beam_Duration.Read(exINI, pSection, "Beam.Duration");
	this->Beam_Amplitude.Read(exINI, pSection, "Beam.Amplitude");
}

template <typename T>
void WeaponTypeExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->DiskLaser_Radius)
		.Process(this->DiskLaser_Circumference)
		.Process(this->LaserThickness)
		.Process(this->Bolt_Disable1)
		.Process(this->Bolt_Disable2)
		.Process(this->Bolt_Disable3)
		.Process(this->Strafing_Shots)
		.Process(this->Strafing_SimulateBurst)
		.Process(this->CanTarget)
		.Process(this->CanTargetHouses)
		.Process(this->RadType)
		.Process(this->Burst_Delays)
		.Process(this->AreaFire_Target)
		.Process(this->FeedbackWeapon)
		.Process(this->Laser_IsSingleColor)
		.Process(this->ROF_RandomDelay)
		.Process(this->AttachEffect_RequiredTypes)
		.Process(this->AttachEffect_DisallowedTypes)
		.Process(this->AttachEffect_IgnoreFromSameSource)
		.Process(this->BlinkWeapon)
		.Process(this->InvBlinkWeapon)
		.Process(this->BlinkWeapon_KillTarget)
		.Process(this->BlinkWeapon_Overlap)
		.Process(this->BlinkWeapon_SelfAnim)
		.Process(this->BlinkWeapon_TargetAnim)
		.Process(this->IonCannonType)

		.Process(this->IsBeamCannon)
		.Process(this->BeamCannonWeapon)
		.Process(this->BeamCannon_Length)
		.Process(this->BeamCannon_Length_Start)
		.Process(this->BeamCannon_LengthIncrease)
		.Process(this->BeamCannon_LengthIncreaseAcceleration)
		.Process(this->BeamCannon_LengthIncreaseMax)
		.Process(this->BeamCannon_LengthIncreaseMin)
		.Process(this->BeamCannon_DrawEBolt)
		.Process(this->BeamCannon_EleHeight)
		.Process(this->BeamCannon_InnerColor)
		.Process(this->BeamCannon_OuterColor)
		.Process(this->BeamCannon_OuterSpread)
		.Process(this->BeamCannon_Duration)
		.Process(this->BeamCannon_Thickness)
		.Process(this->BeamCannon_DrawLaser)
		.Process(this->BeamCannon_DrawFromSelf)
		.Process(this->BeamCannon_LaserHeight)
		.Process(this->BeamCannon_DrawFromSelf_FLH)
		.Process(this->BeamCannon_ROF)
		.Process(this->BeamCannon_Burst)
		.Process(this->BeamCannon_FLH)
		.Process(this->BeamCannon_Start)

		.Process(this->PassengerDeletion)
		.Process(this->PassengerTransport)
		.Process(this->PassengerTransport_UsePassengerData)
		.Process(this->PassengerTransport_Overlap)
		.Process(this->PassengerTransport_MoveToTarget)
		.Process(this->PassengerTransport_MoveToTargetAllowHouses)
		.Process(this->PassengerTransport_MoveToTargetAllowHouses_IgnoreNeturalHouse)
		.Process(this->PassengerTransport_UseParachute)
		.Process(this->KickOutPassenger)

		.Process(this->SelfTransport)
		.Process(this->SelfTransport_UseData)
		.Process(this->SelfTransport_Overlap)
		.Process(this->SelfTransport_Anim)
		.Process(this->SelfTransport_MoveToTarget)
		.Process(this->SelfTransport_MoveToTargetAllowHouses)
		.Process(this->SelfTransport_MoveToTargetAllowHouses_IgnoreNeturalHouse)
		.Process(this->SelfTransport_UseParachute)

		.Process(this->FacingTarget)

		.Process(this->AttachWeapons)
		.Process(this->AttachWeapons_Burst_InvertL)
		.Process(this->AttachWeapons_DetachedROF)
		.Process(this->AttachWeapons_FLH)
		.Process(this->AttachWeapons_UseAmmo)
		.Process(this->AttachWeapons_DetachedFire)

		.Process(this->OnlyAllowOneFirer)
		.Process(this->OnlyAllowOneFirer_Count)
		.Process(this->OnlyAllowOneFirer_OtherWeapons)
		.Process(this->OnlyAllowOneFirer_IgnoreWeapons)
		.Process(this->OnlyAllowOneFirer_ResetImmediately)

		.Process(this->ElectricLaser)
		.Process(this->ElectricLaser_Count)
		.Process(this->ElectricLaser_Length)
		.Process(this->ElectricLaser_Timer)
		.Process(this->ElectricLaser_Color)
		.Process(this->ElectricLaser_Amplitude)
		.Process(this->ElectricLaser_Duration)
		.Process(this->ElectricLaser_Thickness)
		.Process(this->ElectricLaser_IsSupported)

		.Process(this->DelayedFire_Anim)
		.Process(this->DelayedFire_Anim_LoopCount)
		.Process(this->DelayedFire_Anim_UseFLH)
		.Process(this->DelayedFire_DurationTimer)

		.Process(this->AttachAttachment_SelfToTarget)
		.Process(this->AttachAttachment_TargetToSelf)
		.Process(this->AttachAttachment_Type)
		.Process(this->AttachAttachment_FLH)
		.Process(this->AttachAttachment_IsOnTurret)

		.Process(this->Ammo)

		.Process(this->ExtraBurst)
		.Process(this->ExtraBurst_Weapon)
		.Process(this->ExtraBurst_Houses)
		.Process(this->ExtraBurst_AlwaysFire)
		.Process(this->ExtraBurst_FLH)
		.Process(this->ExtraBurst_FacingRange)
		.Process(this->ExtraBurst_InvertL)
		.Process(this->ExtraBurst_Spread)
		.Process(this->ExtraBurst_UseAmmo)
		.Process(this->ExtraBurst_SkipNeutralTarget)

		.Process(this->ExtraWarheads)
		.Process(this->ExtraWarheads_DamageOverrides)

		.Process(this->Laser_Thickness)
		.Process(this->IsTrackingLaser)

		.Process(this->StrafingLasers)
		.Process(this->StrafingLasers_Random)

		.Process(this->DecloakToFire)

		.Process(this->EBolt_Color1)
		.Process(this->EBolt_Color2)
		.Process(this->EBolt_Color3)

		.Process(this->Beam_Color)
		.Process(this->Beam_Duration)
		.Process(this->Beam_IsHouseColor)
		.Process(this->Beam_Amplitude)
		;
};

void WeaponTypeExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<WeaponTypeClass>::LoadFromStream(Stm);
	this->Serialize(Stm);

}

void WeaponTypeExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<WeaponTypeClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

bool WeaponTypeExt::LoadGlobals(PhobosStreamReader& Stm)
{
	return Stm
		.Process(nOldCircumference)
		.Success();
}

bool WeaponTypeExt::SaveGlobals(PhobosStreamWriter& Stm)
{
	return Stm
		.Process(nOldCircumference)
		.Success();
}

void WeaponTypeExt::DetonateAt(WeaponTypeClass* pThis, AbstractClass* pTarget, TechnoClass* pOwner, HouseClass* pFiringHouse)
{
	WeaponTypeExt::DetonateAt(pThis, pTarget, pOwner, pThis->Damage, pFiringHouse);
}

void WeaponTypeExt::DetonateAt(WeaponTypeClass* pThis, AbstractClass* pTarget, TechnoClass* pOwner, int damage, HouseClass* pFiringHouse)
{
	if (BulletClass* pBullet = pThis->Projectile->CreateBullet(pTarget, pOwner,
		damage, pThis->Warhead, 0, pThis->Bright))
	{
		const CoordStruct& coords = pTarget->GetCoords();

		if (pFiringHouse)
		{
			auto const pBulletExt = BulletExt::ExtMap.Find(pBullet);
			pBulletExt->FirerHouse = pFiringHouse;
		}

		pBullet->SetWeaponType(pThis);
		pBullet->Limbo();
		pBullet->SetLocation(coords);
		pBullet->Explode(true);
		pBullet->UnInit();
	}
}

void WeaponTypeExt::DetonateAt(WeaponTypeClass* pThis, const CoordStruct& coords, TechnoClass* pOwner, HouseClass* pFiringHouse)
{
	WeaponTypeExt::DetonateAt(pThis, coords, pOwner, pThis->Damage, pFiringHouse);
}

void WeaponTypeExt::DetonateAt(WeaponTypeClass* pThis, const CoordStruct& coords, TechnoClass* pOwner, int damage, HouseClass* pFiringHouse)
{
	if (BulletClass* pBullet = pThis->Projectile->CreateBullet(nullptr, pOwner,
		damage, pThis->Warhead, 0, pThis->Bright))
	{
		if (pFiringHouse)
		{
			auto const pBulletExt = BulletExt::ExtMap.Find(pBullet);
			pBulletExt->FirerHouse = pFiringHouse;
		}

		pBullet->SetWeaponType(pThis);
		pBullet->Limbo();
		pBullet->SetLocation(coords);
		pBullet->Explode(true);
		pBullet->UnInit();
	}
}

void WeaponTypeExt::ProcessAttachWeapons(WeaponTypeClass* pThis, TechnoClass* pOwner, AbstractClass* pTarget)
{
	WeaponTypeExt::ExtData* pExt = WeaponTypeExt::ExtMap.Find(pThis);
	TechnoExt::ExtData* pOwnerExt = TechnoExt::ExtMap.Find(pOwner);

	if (pExt->AttachWeapons.empty())
		return;

	const ValueableVector<WeaponTypeClass*>& vWeapons = pExt->AttachWeapons;
	std::vector<CDTimerClass>& vTimers = pOwnerExt->AttachWeapon_Timers[pThis->GetArrayIndex()];
	const std::vector<CoordStruct>& vFLH = pExt->AttachWeapons_FLH;

	if (pExt->AttachWeapons_DetachedROF)
	{
		while (vTimers.size() < vWeapons.size())
		{
			//size_t idx = vTimers.size();
			vTimers.emplace_back(CDTimerClass(-1));
		}
	}

	for (size_t i = 0; i < vWeapons.size(); i++)
	{
		WeaponTypeClass* pWeapon = vWeapons[i];

		if (pWeapon == pThis)
			return;

		if (pExt->AttachWeapons_DetachedROF)
		{
			if (!vTimers[i].Completed())
				continue;

			int rofBuff;
			double rofMulti = pOwnerExt->GetAEROFMul(&rofBuff) * pOwner->Owner->ROFMultiplier * pOwner->AresExtData->ROFMultiplier;

			vTimers[i].Start(Game::F2I(pWeapon->ROF * rofMulti) + rofBuff);

		}

		if (pExt->AttachWeapons_UseAmmo && pOwner->GetTechnoType()->Ammo > 0)
		{
			WeaponTypeExt::ExtData* pAttachExt = WeaponTypeExt::ExtMap.Find(pWeapon);
			if (pAttachExt->Ammo + pExt->Ammo > pOwner->Ammo)
				continue;
			else
				TechnoExt::ChangeAmmo(pOwner, pAttachExt->Ammo);
		}

		WeaponStruct weaponTmp;
		weaponTmp.WeaponType = pWeapon;
		weaponTmp.FLH = vFLH[i];

		if (pExt->AttachWeapons_Burst_InvertL && pThis->Burst > 1 && pOwner->CurrentBurstIndex & 1)
			weaponTmp.FLH.Y = -weaponTmp.FLH.Y;

		TechnoExt::SimulatedFire(pOwner, weaponTmp, pTarget);
	}
}

void WeaponTypeExt::ProcessExtraBrust(WeaponTypeClass* pThis, TechnoClass* pOwner, AbstractClass* pTarget)
{
	WeaponTypeExt::ExtData* pExt = WeaponTypeExt::ExtMap.Find(pThis);

	if (pExt->ExtraBurst <= 0 || !pExt->ExtraBurst_Weapon || pExt->ExtraBurst_Weapon == pThis)
		return;

	WeaponTypeExt::ExtData* pExtraExt = WeaponTypeExt::ExtMap.Find(pExt->ExtraBurst_Weapon);

	if (pExt->ExtraBurst_UseAmmo && pOwner->GetTechnoType()->Ammo > 0 && pExtraExt->Ammo + pExt->Ammo > pOwner->Ammo)
		return;

	const std::vector<CoordStruct>& vFLH = pExt->ExtraBurst_FLH;
	const std::vector<TechnoClass*> vTechnos(std::move(Helpers::Alex::getCellSpreadItems(pOwner->GetCoords(), (pThis->Range / 256), true)));

	size_t j = 0;
	for (int i = 0; i < pExt->ExtraBurst; i++)
	{
		WeaponStruct weaponTmp;
		weaponTmp.WeaponType = pExt->ExtraBurst_Weapon;
		weaponTmp.FLH = vFLH[i];

		if (pExt->ExtraBurst_InvertL && pThis->Burst > 1 && pOwner->CurrentBurstIndex & 1)
			weaponTmp.FLH.Y = -weaponTmp.FLH.Y;

		if (j >= vTechnos.size())
		{
			if (pExt->ExtraBurst_AlwaysFire)
			{
				if (pOwner->DistanceFrom(pTarget) < pExt->ExtraBurst_Weapon->MinimumRange)
					break;

				const auto pTechno = abstract_cast<TechnoClass*>(pTarget);
				CellClass* pTargetCell = nullptr;

				if (!pTechno || !pTechno->IsInAir())
				{
					if (const auto pCell = abstract_cast<CellClass*>(pTarget))
						pTargetCell = pCell;
					// else if (const auto pObject = abstract_cast<ObjectClass*>(pTarget))
						// pTargetCell = pObject->GetCell();
				}

				if (pTargetCell)
				{
					if (!EnumFunctions::IsCellEligible(pTargetCell, pExtraExt->CanTarget, true) ||
						!pExt->ExtraBurst_Weapon->Projectile->AG)
						break;
				}

				if (pTechno)
				{
					if (!EnumFunctions::IsTechnoEligible(pTechno, pExtraExt->CanTarget) ||
						!EnumFunctions::CanTargetHouse(pExtraExt->CanTargetHouses, pOwner->Owner, pTechno->Owner) ||
						(pTechno->IsInAir() && !pExt->ExtraBurst_Weapon->Projectile->AA) ||
						(!pTechno->IsInAir() && !pExt->ExtraBurst_Weapon->Projectile->AG) ||
						GeneralUtils::GetWarheadVersusArmor(pExt->ExtraBurst_Weapon->Warhead, pTechno->GetTechnoType()->Armor) == 0.0)
						break;
				}

				TechnoExt::SimulatedFire(pOwner, weaponTmp, pTarget);
				if (pExt->ExtraBurst_UseAmmo)
					TechnoExt::ChangeAmmo(pOwner, pExtraExt->Ammo);
				continue;
			}
			else
			{
				break;
			}
		}

		if (vTechnos[j] == pOwner ||
			!EnumFunctions::CanTargetHouse(pExtraExt->CanTargetHouses, pOwner->Owner, vTechnos[j]->Owner) ||
			!EnumFunctions::IsTechnoEligible(vTechnos[j], pExtraExt->CanTarget))
		{
			{
				i--;
				j++;
				continue;
			}
		}

		if (EnumFunctions::CanTargetHouse(pExt->ExtraBurst_Houses, pOwner->Owner, vTechnos[j]->Owner))
		{
			if (auto pTechno = abstract_cast<TechnoClass*>(pTarget))
			{
				if (pTechno == vTechnos[j])
				{
					i--;
					j++;
					continue;
				}
			}

			if (GeneralUtils::GetWarheadVersusArmor(pExt->ExtraBurst_Weapon->Warhead, vTechnos[j]->GetTechnoType()->Armor) == 0.0 ||
				(vTechnos[j]->IsInAir() && !pExt->ExtraBurst_Weapon->Projectile->AA) ||
				(!vTechnos[j]->IsInAir() && !pExt->ExtraBurst_Weapon->Projectile->AG) ||
				pOwner->DistanceFrom(pTarget) < pExt->ExtraBurst_Weapon->MinimumRange ||
				(pExt->ExtraBurst_SkipNeutralTarget && !vTechnos[j]->Owner->IsControlledByHuman() && (strcmp(vTechnos[j]->Owner->PlainName, "Computer") != 0)))
			{
				i--;
				j++;
				continue;
			}

			if (pExt->ExtraBurst_FacingRange < 128)
			{
				const CoordStruct source = pOwner->Location;
				const CoordStruct target = vTechnos[j]->Location;
				const DirStruct tgtDir = DirStruct(Math::atan2(source.Y - target.Y, target.X - source.X));

				auto targetfacing = static_cast<short>(tgtDir.GetDir());
				auto ownerfacing = pOwner->HasTurret() ? static_cast<short>(pOwner->SecondaryFacing.Current().GetDir()) : static_cast<short>(pOwner->PrimaryFacing.Current().GetDir());

				if (abs(targetfacing - ownerfacing) > pExt->ExtraBurst_FacingRange)
				{
					i--;
					j++;
					continue;
				}
			}

			TechnoExt::SimulatedFire(pOwner, weaponTmp, vTechnos[j]);
			if (pExt->ExtraBurst_UseAmmo)
				TechnoExt::ChangeAmmo(pOwner, pExtraExt->Ammo);
		}
		else
			i--;
		j++;
	}
}

void WeaponTypeExt::ProcessExtraBrustSpread(WeaponTypeClass* pThis, TechnoClass* pOwner, AbstractClass* pTarget)
{
	WeaponTypeExt::ExtData* pExt = WeaponTypeExt::ExtMap.Find(pThis);

	if (pExt->ExtraBurst <= 0 || !pExt->ExtraBurst_Weapon || pExt->ExtraBurst_Weapon == pThis)
		return;

	WeaponTypeExt::ExtData* pExtraExt = WeaponTypeExt::ExtMap.Find(pExt->ExtraBurst_Weapon);

	if (pExt->ExtraBurst_UseAmmo && pOwner->GetTechnoType()->Ammo > 0 && pExtraExt->Ammo + pExt->Ammo > pOwner->Ammo)
		return;

	TechnoExt::ExtData* pOwnerExt = TechnoExt::ExtMap.Find(pOwner);

	const std::vector<CoordStruct>& vFLH = pExt->ExtraBurst_FLH;
	if (pOwner->CurrentBurstIndex == 0)
	{
		const std::vector<TechnoClass*> vTechnos(std::move(Helpers::Alex::getCellSpreadItems(pOwner->GetCoords(), (pThis->Range / 256), true)));
		pOwnerExt->ExtraBurstTargets = vTechnos;
		pOwnerExt->ExtraBurstIndex = 0;
		pOwnerExt->ExtraBurstTargetIndex = 0;
	}

	for (int i = 0; i < (pExt->ExtraBurst / pThis->Burst); i++)
	{
		WeaponStruct weaponTmp;
		weaponTmp.WeaponType = pExt->ExtraBurst_Weapon;
		weaponTmp.FLH = vFLH[pOwnerExt->ExtraBurstIndex];

		if (pExt->ExtraBurst_InvertL && pThis->Burst > 1 && pOwner->CurrentBurstIndex & 1)
			weaponTmp.FLH.Y = -weaponTmp.FLH.Y;

		if (pOwnerExt->ExtraBurstTargetIndex >= pOwnerExt->ExtraBurstTargets.size())
		{
			if (pExt->ExtraBurst_AlwaysFire)
			{
				if (pOwner->DistanceFrom(pTarget) < pExt->ExtraBurst_Weapon->MinimumRange)
					break;

				const auto pTechno = abstract_cast<TechnoClass*>(pTarget);
				CellClass* pTargetCell = nullptr;

				if (!pTechno || !pTechno->IsInAir())
				{
					if (const auto pCell = abstract_cast<CellClass*>(pTarget))
						pTargetCell = pCell;
					// else if (const auto pObject = abstract_cast<ObjectClass*>(pTarget))
						// pTargetCell = pObject->GetCell();
				}

				if (pTargetCell)
				{
					if (!EnumFunctions::IsCellEligible(pTargetCell, pExtraExt->CanTarget, true) ||
						!pExt->ExtraBurst_Weapon->Projectile->AG)
						break;
				}

				if (pTechno)
				{
					if (!EnumFunctions::IsTechnoEligible(pTechno, pExtraExt->CanTarget) ||
						!EnumFunctions::CanTargetHouse(pExtraExt->CanTargetHouses, pOwner->Owner, pTechno->Owner) ||
						(pTechno->IsInAir() && !pExt->ExtraBurst_Weapon->Projectile->AA) ||
						(!pTechno->IsInAir() && !pExt->ExtraBurst_Weapon->Projectile->AG) ||
						GeneralUtils::GetWarheadVersusArmor(pExt->ExtraBurst_Weapon->Warhead, pTechno->GetTechnoType()->Armor) == 0.0)
						break;
				}

				TechnoExt::SimulatedFire(pOwner, weaponTmp, pTarget);
				pOwnerExt->ExtraBurstIndex++;
				if (pExt->ExtraBurst_UseAmmo)
					TechnoExt::ChangeAmmo(pOwner, pExtraExt->Ammo);
				continue;
			}
			else
			{
				break;
			}
		}

		if (pOwnerExt->ExtraBurstTargets[pOwnerExt->ExtraBurstTargetIndex] == pOwner ||
			!EnumFunctions::CanTargetHouse(pExtraExt->CanTargetHouses, pOwner->Owner, pOwnerExt->ExtraBurstTargets[pOwnerExt->ExtraBurstTargetIndex]->Owner) ||
			!EnumFunctions::IsTechnoEligible(pOwnerExt->ExtraBurstTargets[pOwnerExt->ExtraBurstTargetIndex], pExtraExt->CanTarget))
		{
			{
				i--;
				pOwnerExt->ExtraBurstTargetIndex++;
				continue;
			}
		}

		if (EnumFunctions::CanTargetHouse(pExt->ExtraBurst_Houses, pOwner->Owner, pOwnerExt->ExtraBurstTargets[pOwnerExt->ExtraBurstTargetIndex]->Owner))
		{
			if (auto pTechno = abstract_cast<TechnoClass*>(pTarget))
			{
				if (pTechno == pOwnerExt->ExtraBurstTargets[pOwnerExt->ExtraBurstTargetIndex])
				{
					i--;
					pOwnerExt->ExtraBurstTargetIndex++;
					continue;
				}
			}

			if (GeneralUtils::GetWarheadVersusArmor(pExt->ExtraBurst_Weapon->Warhead, pOwnerExt->ExtraBurstTargets[pOwnerExt->ExtraBurstTargetIndex]->GetTechnoType()->Armor) == 0.0 ||
				(pOwnerExt->ExtraBurstTargets[pOwnerExt->ExtraBurstTargetIndex]->IsInAir() && !pExt->ExtraBurst_Weapon->Projectile->AA) ||
				(!pOwnerExt->ExtraBurstTargets[pOwnerExt->ExtraBurstTargetIndex]->IsInAir() && !pExt->ExtraBurst_Weapon->Projectile->AG) ||
				pOwner->DistanceFrom(pTarget) < pExt->ExtraBurst_Weapon->MinimumRange ||
				(pExt->ExtraBurst_SkipNeutralTarget && !pOwnerExt->ExtraBurstTargets[pOwnerExt->ExtraBurstTargetIndex]->Owner->IsControlledByHuman() && (strcmp(pOwnerExt->ExtraBurstTargets[pOwnerExt->ExtraBurstTargetIndex]->Owner->PlainName, "Computer") != 0)))
			{
				i--;
				pOwnerExt->ExtraBurstTargetIndex++;
				continue;
			}

			if (pExt->ExtraBurst_FacingRange < 128)
			{
				const CoordStruct source = pOwner->Location;
				const CoordStruct target = pOwnerExt->ExtraBurstTargets[pOwnerExt->ExtraBurstTargetIndex]->Location;
				const DirStruct tgtDir = DirStruct(Math::atan2(source.Y - target.Y, target.X - source.X));

				auto targetfacing = static_cast<short>(tgtDir.GetDir());
				auto ownerfacing = pOwner->HasTurret() ? static_cast<short>(pOwner->SecondaryFacing.Current().GetDir()) : static_cast<short>(pOwner->PrimaryFacing.Current().GetDir());

				if (abs(targetfacing - ownerfacing) > pExt->ExtraBurst_FacingRange)
				{
					i--;
					pOwnerExt->ExtraBurstTargetIndex++;
					continue;
				}
			}

			if (pOwnerExt->ExtraBurstTargets[pOwnerExt->ExtraBurstTargetIndex]->DistanceFrom(pOwner) > pThis->Range)
			{
				i--;
				pOwnerExt->ExtraBurstTargetIndex++;
				continue;
			}

			TechnoExt::SimulatedFire(pOwner, weaponTmp, pOwnerExt->ExtraBurstTargets[pOwnerExt->ExtraBurstTargetIndex]);
			pOwnerExt->ExtraBurstIndex++;
			if (pExt->ExtraBurst_UseAmmo)
				TechnoExt::ChangeAmmo(pOwner, pExtraExt->Ammo);
		}
		else
			i--;
		pOwnerExt->ExtraBurstTargetIndex++;
	}
}

AnimTypeClass* WeaponTypeExt::GetFireAnim(WeaponTypeClass* pThis, TechnoClass* pFirer)
{
	if (pFirer == nullptr || pThis->Anim.Count <= 0)
		return nullptr;

	int highest = Conversions::Int2Highest(pThis->Anim.Count);

	if (highest >= 3)
	{
		unsigned int offset = 1U << (highest - 3);
		int index = Conversions::TranslateFixedPoint
		(
			16,
			highest,
			static_cast<WORD>(pFirer->HasTurret()
				? pFirer->TurretFacing().Raw
				: pFirer->GetRealFacing().Raw),
			offset
		);

		return pThis->Anim.GetItemOrDefault(index);
	}

	return pThis->Anim.GetItemOrDefault(0);
}

// =============================
// container

WeaponTypeExt::ExtContainer::ExtContainer() : Container("WeaponTypeClass") { }

WeaponTypeExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

DEFINE_HOOK(0x771EE9, WeaponTypeClass_CTOR, 0x5)
{
	GET(WeaponTypeClass*, pItem, ESI);

	WeaponTypeExt::ExtMap.FindOrAllocate(pItem);

	return 0;
}

DEFINE_HOOK(0x77311D, WeaponTypeClass_SDDTOR, 0x6)
{
	GET(WeaponTypeClass*, pItem, ESI);

	WeaponTypeExt::ExtMap.Remove(pItem);

	return 0;
}

DEFINE_HOOK_AGAIN(0x772EB0, WeaponTypeClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x772CD0, WeaponTypeClass_SaveLoad_Prefix, 0x7)
{
	GET_STACK(WeaponTypeClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	WeaponTypeExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK(0x772EA6, WeaponTypeClass_Load_Suffix, 0x6)
{
	WeaponTypeExt::ExtMap.LoadStatic();

	return 0;
}

DEFINE_HOOK(0x772F8C, WeaponTypeClass_Save, 0x5)
{
	WeaponTypeExt::ExtMap.SaveStatic();

	return 0;
}

DEFINE_HOOK_AGAIN(0x7729C7, WeaponTypeClass_LoadFromINI, 0x5)
DEFINE_HOOK_AGAIN(0x7729D6, WeaponTypeClass_LoadFromINI, 0x5)
DEFINE_HOOK(0x7729B0, WeaponTypeClass_LoadFromINI, 0x5)
{
	GET(WeaponTypeClass*, pItem, ESI);
	GET_STACK(CCINIClass*, pINI, 0xE4);

	WeaponTypeExt::ExtMap.LoadFromINI(pItem, pINI);

	return 0;
}
