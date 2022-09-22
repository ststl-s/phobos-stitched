#include "Body.h"

#include <BulletTypeClass.h>
#include <BulletClass.h>

#include <Ext/Techno/Body.h>

#include <Misc/PhobosGlobal.h>

template<> const DWORD Extension<WeaponTypeClass>::Canary = 0x22222222;
WeaponTypeExt::ExtContainer WeaponTypeExt::ExtMap;

void WeaponTypeExt::ExtData::Initialize()
{
	this->RadType = RadTypeClass::FindOrAllocate("Radiation");
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

	this->BlinkWeapon.Read(exINI, pSection, "BlinkWeapon");
	this->InvBlinkWeapon.Read(exINI, pSection, "InvBlinkWeapon");
	this->BlinkWeapon_KillTarget.Read(exINI, pSection, "BlinkWeapon.KillTarget");
	this->BlinkWeapon_Overlap.Read(exINI, pSection, "BlinkWeapon.Overlap");
	this->BlinkWeapon_SelfAnim.Read(exINI, pSection, "BlinkWeapon.SelfAnim");
	this->BlinkWeapon_TargetAnim.Read(exINI, pSection, "BlinkWeapon.TargetAnim");

	this->IonCannonType.Read(exINI, pSection, "IonCannonType", true);

	this->IsBeamCannon.Read(exINI, pSection, "IsBeamCannon");
	this->BeamCannon_Length.Read(exINI, pSection, "BeamCannon.Length");
	this->BeamCannon_Length_StartOffset.Read(exINI, pSection, "BeamCannon.Length.StartOffset");
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
	this->BeamCannon_DrawFromSelf_HeightOffset.Read(exINI, pSection, "BeamCannon.DrawFromSelf.HeightOffset");
	this->BeamCannon_ROF.Read(exINI, pSection, "BeamCannon.ROF");

	this->PassengerDeletion.Read(exINI, pSection, "PassengerDeletion");
	this->PassengerTransport.Read(exINI, pSection, "PassengerTransport");
	this->PassengerTransport_MoveToTarget.Read(exINI, pSection, "PassengerTransport.MoveToTarget");
	this->PassengerTransport_MoveToTargetAllowHouses.Read(exINI, pSection, "PassengerTransport.MoveToTargetAllowHouses");

	this->FacingTarget.Read(exINI, pSection, "FacingTarget");
	this->KickOutPassenger.Read(exINI, pSection, "KickOutPassenger");

	this->AttachWeapons.Read(exINI, pSection, "AttachWeapons");
	this->AttachWeapons_Burst_InvertL.Read(exINI, pSection, "AttachWeapons.Burst.InvertL");
	this->AttachWeapons_DetachedROF.Read(exINI, pSection, "AttachWeapons.DetachedROF");

	for (size_t i = 0; i < AttachWeapons.size(); i++)
	{
		char key[0x20];
		Valueable<CoordStruct> crdFLH;
		sprintf(key, "AttachWeapon%d.FLH", i);
		crdFLH.Read(exINI, pSection, key);
		AttachWeapons_FLH.emplace_back(crdFLH.Get());
	}

	this->OnlyAllowOneFirer.Read(exINI, pSection, "OnlyAllowOneFirer");

	//电流激光
	this->ElectricLaser.Read(exINI, pSection, "IsElectricLaser");
	this->ElectricLaser_Count.Read(exINI, pSection, "ElectricLaser.Count");
	this->ElectricLaser_Length.Read(exINI, pSection, "ElectricLaser.Length");

	for (int i = 0; i < this->ElectricLaser_Count; ++i)
	{
		char colortext[64];
		Nullable<ColorStruct> color;
		_snprintf_s(colortext, sizeof(colortext), "ElectricLaser%d.Color", i);
		color.Read(exINI, pSection, colortext);

		char amplitudetext[64];
		Nullable<float> amplitude;
		_snprintf_s(amplitudetext, sizeof(amplitudetext), "ElectricLaser%d.Amplitude", i);
		amplitude.Read(exINI, pSection, amplitudetext);

		char durationtext[64];
		Nullable<int> duration;
		_snprintf_s(durationtext, sizeof(durationtext), "ElectricLaser%d.Duration", i);
		duration.Read(exINI, pSection, durationtext);

		char thicknesstext[64];
		Nullable<int> thickness;
		_snprintf_s(thicknesstext, sizeof(thicknesstext), "ElectricLaser%d.Thickness", i);
		thickness.Read(exINI, pSection, thicknesstext);

		char issupportedtext[64];
		Nullable<bool> issupported;
		_snprintf_s(issupportedtext, sizeof(issupportedtext), "ElectricLaser%d.IsSupported", i);
		issupported.Read(exINI, pSection, issupportedtext);

		if (color.isset())
			this->ElectricLaser_Color.push_back(color);
		else
			this->ElectricLaser_Color.push_back({ 128,128,128 });

		if (amplitude.isset())
			this->ElectricLaser_Amplitude.push_back(amplitude);
		else
			this->ElectricLaser_Amplitude.push_back(10.0);

		if (duration.isset())
			this->ElectricLaser_Duration.push_back(duration);
		else
			this->ElectricLaser_Duration.push_back(15);

		if (thickness.isset())
			this->ElectricLaser_Thickness.push_back(thickness);
		else
			this->ElectricLaser_Thickness.push_back(2);

		if (issupported.isset())
			this->ElectricLaser_IsSupported.push_back(issupported);
		else
			this->ElectricLaser_IsSupported.push_back(false);
	}
}

template <typename T>
void WeaponTypeExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->DiskLaser_Radius)
		.Process(this->DiskLaser_Circumference)
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
		.Process(this->BeamCannon_Length_StartOffset)
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
		.Process(this->BeamCannon_DrawFromSelf_HeightOffset)
		.Process(this->BeamCannon_ROF)

		.Process(this->PassengerDeletion)
		.Process(this->PassengerTransport)
		.Process(this->PassengerTransport_MoveToTarget)
		.Process(this->PassengerTransport_MoveToTargetAllowHouses)
		.Process(this->KickOutPassenger)

		.Process(this->FacingTarget)

		.Process(this->AttachWeapons)
		.Process(this->AttachWeapons_Burst_InvertL)
		.Process(this->AttachWeapons_DetachedROF)
		.Process(this->AttachWeapons_FLH)

		.Process(this->OnlyAllowOneFirer)

		.Process(this->ElectricLaser)
		.Process(this->ElectricLaser_Count)
		.Process(this->ElectricLaser_Length)
		.Process(this->ElectricLaser_Color)
		.Process(this->ElectricLaser_Amplitude)
		.Process(this->ElectricLaser_Duration)
		.Process(this->ElectricLaser_Thickness)
		.Process(this->ElectricLaser_IsSupported)
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

void WeaponTypeExt::DetonateAt(WeaponTypeClass* pThis, ObjectClass* pTarget, TechnoClass* pOwner)
{
	WeaponTypeExt::DetonateAt(pThis, pTarget, pOwner, pThis->Damage);
}

void WeaponTypeExt::DetonateAt(WeaponTypeClass* pThis, ObjectClass* pTarget, TechnoClass* pOwner, int damage)
{
	if (BulletClass* pBullet = pThis->Projectile->CreateBullet(pTarget, pOwner,
		damage, pThis->Warhead, 0, pThis->Bright))
	{
		const CoordStruct& coords = pTarget->GetCoords();

		pBullet->SetWeaponType(pThis);
		pBullet->Limbo();
		pBullet->SetLocation(coords);
		pBullet->Explode(true);
		pBullet->UnInit();
	}
}

void WeaponTypeExt::DetonateAt(WeaponTypeClass* pThis, const CoordStruct& coords, TechnoClass* pOwner)
{
	WeaponTypeExt::DetonateAt(pThis, coords, pOwner, pThis->Damage);
}

void WeaponTypeExt::DetonateAt(WeaponTypeClass* pThis, const CoordStruct& coords, TechnoClass* pOwner, int damage)
{
	if (BulletClass* pBullet = pThis->Projectile->CreateBullet(nullptr, pOwner,
		damage, pThis->Warhead, 0, pThis->Bright))
	{
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
			size_t idx = vTimers.size();
			vTimers.emplace_back(CDTimerClass(vWeapons[idx]->ROF));
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

			vTimers[i].Start(pWeapon->ROF);
		}

		WeaponStruct weaponTmp;
		weaponTmp.WeaponType = pWeapon;
		weaponTmp.FLH = vFLH[i];

		if (pExt->AttachWeapons_Burst_InvertL && pThis->Burst > 1 && pOwner->CurrentBurstIndex & 1)
			weaponTmp.FLH.Y = -weaponTmp.FLH.Y;

		TechnoExt::SimulatedFire(pOwner, weaponTmp, pTarget);
	}
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
