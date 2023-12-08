#include "CrateTypeClass.h"

#include <New/Type/AttachEffectTypeClass.h>

#include <Utilities/TemplateDef.h>

Enumerable<CrateTypeClass>::container_t Enumerable<CrateTypeClass>::Array;

const char* Enumerable<CrateTypeClass>::GetMainSection()
{
	return "CrateTypes";
}

void CrateTypeClass::LoadFromINI(CCINIClass* pINI)
{
	const char* pSection = Name.data();

	INI_EX exINI(pINI);

	this->Maximum.Read(exINI, pSection, "Maximum");
	this->Minimum.Read(exINI, pSection, "Minimum");

	this->Duration.Read(exINI, pSection, "Duration");
	this->SpwanDelay.Read(exINI, pSection, "SpwanDelay");

	this->Image.Read(exINI, pSection, "Image");
	this->AllowSpwanOnLand.Read(exINI, pSection, "AllowSpwanOnLand");
	this->WaterImage.Read(exINI, pSection, "WaterImage");
	this->AllowSpwanOnWater.Read(exINI, pSection, "AllowSpwanOnWater");

	this->Weapon_RandomPick.Read(exINI, pSection, "Weapon.RandomPick");
	this->Weapon_Weight.Read(exINI, pSection, "Weapon.Weight");
	for (int i = 0;; i++)
	{
		Nullable<WeaponTypeClass*> weapon;
		char keyName[0x30];
		sprintf_s(keyName, "Weapon%d", i);
		weapon.Read(exINI, pSection, keyName);

		if (!weapon.isset())
			break;

		Nullable<double> chance;
		sprintf_s(keyName, "Weapon%d.Chance", i);
		chance.Read(exINI, pSection, keyName);

		if (!chance.isset())
			chance = 1.0;

		Nullable<AnimTypeClass*> anim;
		sprintf_s(keyName, "Weapon%d.Anim", i);
		anim.Read(exINI, pSection, keyName);

		if (!anim.isset())
			anim = nullptr;

		NullableIdx<VocClass> sound;
		sprintf_s(keyName, "Weapon%d.Sound", i);
		sound.Read(exINI, pSection, keyName);

		Nullable<CSFText> text;
		sprintf_s(keyName, "Weapon%d.Message", i);
		text.Read(exINI, pSection, keyName);

		this->Weapons.emplace_back(weapon);
		this->Weapons_Chances.emplace_back(chance);
		this->Weapons_Anims.emplace_back(anim);
		this->Weapons_Sounds.emplace_back(sound);
		this->Weapons_Messages.emplace_back(text);
	}

	this->AttachEffect_RandomPick.Read(exINI, pSection, "AttachEffect.RandomPick");
	this->AttachEffect_Weight.Read(exINI, pSection, "AttachEffect.Weight");
	for (int i = 0;; i++)
	{
		Nullable<AttachEffectTypeClass*> ae;
		char keyName[0x30];
		sprintf_s(keyName, "AttachEffect%d", i);
		ae.Read(exINI, pSection, keyName);

		if (!ae.isset())
			break;

		Nullable<double> chance;
		sprintf_s(keyName, "AttachEffect%d.Chance", i);
		chance.Read(exINI, pSection, keyName);

		if (!chance.isset())
			chance = 1.0;

		Nullable<AnimTypeClass*> anim;
		sprintf_s(keyName, "AttachEffect%d.Anim", i);
		anim.Read(exINI, pSection, keyName);

		if (!anim.isset())
			anim = nullptr;

		NullableIdx<VocClass> sound;
		sprintf_s(keyName, "AttachEffect%d.Sound", i);
		sound.Read(exINI, pSection, keyName);

		Nullable<CSFText> text;
		sprintf_s(keyName, "AttachEffect%d.Message", i);
		text.Read(exINI, pSection, keyName);

		this->AttachEffects.emplace_back(ae);
		this->AttachEffects_Chances.emplace_back(chance);
		this->AttachEffects_Anims.emplace_back(anim);
		this->AttachEffects_Sounds.emplace_back(sound);
		this->AttachEffects_Messages.emplace_back(text);
	}

	this->Money.Read(exINI, pSection, "Money");
	this->Money_Weight.Read(exINI, pSection, "Money.Weight");
	this->Money_Anim.Read(exINI, pSection, "Money.Anim");
	this->Money_Sound.Read(exINI, pSection, "Money.Sound");
	this->Money_Message.Read(exINI, pSection, "Money.Message");
	this->Money_Display.Read(exINI, pSection, "Money.Display");
	this->Money_DisplayHouses.Read(exINI, pSection, "Money.DisplayHouses");
	this->Money_DisplayOffset.Read(exINI, pSection, "Money.DisplayOffset");

	this->SpySat_Range.Read(exINI, pSection, "SpySat.Range");
	this->SpySat_Weight.Read(exINI, pSection, "SpySat.Weight");
	this->SpySat_Anim.Read(exINI, pSection, "SpySat.Anim");
	this->SpySat_Sound.Read(exINI, pSection, "SpySat.Sound");
	this->SpySat_Message.Read(exINI, pSection, "SpySat.Message");

	this->Darkness_Weight.Read(exINI, pSection, "Darkness.Weight");
	this->Darkness_Anim.Read(exINI, pSection, "Darkness.Anim");
	this->Darkness_Sound.Read(exINI, pSection, "Darkness.Sound");
	this->Darkness_Message.Read(exINI, pSection, "Darkness.Message");

	this->SuperWeapon_RandomPick.Read(exINI, pSection, "SuperWeapon.RandomPick");
	this->SuperWeapon_Weight.Read(exINI, pSection, "SuperWeapon.Weight");
	for (int i = 0;; i++)
	{
		NullableIdx<SuperWeaponTypeClass> sw;
		char keyName[0x30];
		sprintf_s(keyName, "SuperWeapon%d", i);
		sw.Read(exINI, pSection, keyName);

		if (!sw.isset())
			break;

		Nullable<double> chance;
		sprintf_s(keyName, "SuperWeapon%d.Chance", i);
		chance.Read(exINI, pSection, keyName);

		if (!chance.isset())
			chance = 1.0;

		Nullable<AnimTypeClass*> anim;
		sprintf_s(keyName, "SuperWeapon%d.Anim", i);
		anim.Read(exINI, pSection, keyName);

		if (!anim.isset())
			anim = nullptr;

		NullableIdx<VocClass> sound;
		sprintf_s(keyName, "SuperWeapon%d.Sound", i);
		sound.Read(exINI, pSection, keyName);

		Nullable<CSFText> text;
		sprintf_s(keyName, "SuperWeapon%d.Message", i);
		text.Read(exINI, pSection, keyName);

		Nullable<bool> permanent;
		sprintf_s(keyName, "SuperWeapon%d.Permanent", i);
		permanent.Read(exINI, pSection, keyName);

		if (!permanent.isset())
			permanent = false;

		this->SuperWeapons.emplace_back(sw);
		this->SuperWeapons_Chances.emplace_back(chance);
		this->SuperWeapons_Anims.emplace_back(anim);
		this->SuperWeapons_Sounds.emplace_back(sound);
		this->SuperWeapons_Messages.emplace_back(text);
		this->SuperWeapons_Permanents.emplace_back(permanent);
	}

	this->RadarJam_Duration.Read(exINI, pSection, "RadarJam.Duration");
	this->RadarJam_Weight.Read(exINI, pSection, "RadarJam.Weight");
	this->RadarJam_Anim.Read(exINI, pSection, "RadarJam.Anim");
	this->RadarJam_Sound.Read(exINI, pSection, "RadarJam.Sound");
	this->RadarJam_Message.Read(exINI, pSection, "RadarJam.Message");

	this->PowerOutage_Duration.Read(exINI, pSection, "PowerOutage.Duration");
	this->PowerOutage_Weight.Read(exINI, pSection, "PowerOutage.Weight");
	this->PowerOutage_Anim.Read(exINI, pSection, "PowerOutage.Anim");
	this->PowerOutage_Sound.Read(exINI, pSection, "PowerOutage.Sound");
	this->PowerOutage_Message.Read(exINI, pSection, "PowerOutage.Message");

	this->Unit_RandomPick.Read(exINI, pSection, "Unit.RandomPick");
	this->Unit_Weight.Read(exINI, pSection, "Unit.Weight");
	for (int i = 0;; i++)
	{
		Nullable<TechnoTypeClass*> unit;
		char keyName[0x30];
		sprintf_s(keyName, "Unit%d", i);
		unit.Read(exINI, pSection, keyName);

		if (!unit.isset())
			break;

		Nullable<double> chance;
		sprintf_s(keyName, "Unit%d.Chance", i);
		chance.Read(exINI, pSection, keyName);

		if (!chance.isset())
			chance = 1.0;

		Nullable<AnimTypeClass*> anim;
		sprintf_s(keyName, "Unit%d.Anim", i);
		anim.Read(exINI, pSection, keyName);

		if (!anim.isset())
			anim = nullptr;

		NullableIdx<VocClass> sound;
		sprintf_s(keyName, "Unit%d.Sound", i);
		sound.Read(exINI, pSection, keyName);

		Nullable<CSFText> text;
		sprintf_s(keyName, "Unit%d.Message", i);
		text.Read(exINI, pSection, keyName);

		Nullable<Vector2D<int>> num;
		sprintf_s(keyName, "Unit%d.Num", i);
		num.Read(exINI, pSection, keyName);

		if (!num.isset())
			num = Vector2D<int> { 1, -1 };

		this->Units.emplace_back(unit);
		this->Units_Chances.emplace_back(chance);
		this->Units_Anims.emplace_back(anim);
		this->Units_Sounds.emplace_back(sound);
		this->Units_Messages.emplace_back(text);
		this->Units_Nums.emplace_back(num);
	}

	this->AllowPick_UnitTypes.Read(exINI, pSection, "AllowPick.UnitTypes");
	this->AllowPick_InfantryTypes.Read(exINI, pSection, "AllowPick.InfantryTypes");

	this->DisallowPick_UnitTypes.Read(exINI, pSection, "DisallowPick.UnitTypes");
	this->DisallowPick_InfantryTypes.Read(exINI, pSection, "DisallowPick.InfantryTypes");

	this->AllowPick_Houses.Read(exINI, pSection, "AllowPick.Houses");
}

template <typename T>
void CrateTypeClass::Serialize(T& stm)
{
	stm
		.Process(this->ArrayIndex)

		.Process(this->Maximum)
		.Process(this->Minimum)

		.Process(this->Duration)
		.Process(this->SpwanDelay)

		.Process(this->Image)
		.Process(this->AllowSpwanOnLand)
		.Process(this->WaterImage)
		.Process(this->AllowSpwanOnWater)

		.Process(this->Weapon_RandomPick)
		.Process(this->Weapon_Weight)
		.Process(this->Weapons)
		.Process(this->Weapons_Chances)
		.Process(this->Weapons_Anims)
		.Process(this->Weapons_Sounds)
		.Process(this->Weapons_Messages)

		.Process(this->AttachEffect_RandomPick)
		.Process(this->AttachEffect_Weight)
		.Process(this->AttachEffects)
		.Process(this->AttachEffects_Chances)
		.Process(this->AttachEffects_Anims)
		.Process(this->AttachEffects_Sounds)
		.Process(this->AttachEffects_Messages)

		.Process(this->Money)
		.Process(this->Money_Weight)
		.Process(this->Money_Anim)
		.Process(this->Money_Sound)
		.Process(this->Money_Message)
		.Process(this->Money_Display)
		.Process(this->Money_DisplayHouses)
		.Process(this->Money_DisplayOffset)

		.Process(this->SpySat_Range)
		.Process(this->SpySat_Weight)
		.Process(this->SpySat_Anim)
		.Process(this->SpySat_Sound)
		.Process(this->SpySat_Message)

		.Process(this->Darkness_Weight)
		.Process(this->Darkness_Anim)
		.Process(this->Darkness_Sound)
		.Process(this->Darkness_Message)

		.Process(this->SuperWeapon_RandomPick)
		.Process(this->SuperWeapon_Weight)
		.Process(this->SuperWeapons)
		.Process(this->SuperWeapons_Chances)
		.Process(this->SuperWeapons_Anims)
		.Process(this->SuperWeapons_Sounds)
		.Process(this->SuperWeapons_Messages)
		.Process(this->SuperWeapons_Permanents)

		.Process(this->RadarJam_Duration)
		.Process(this->RadarJam_Weight)
		.Process(this->RadarJam_Anim)
		.Process(this->RadarJam_Sound)
		.Process(this->RadarJam_Message)

		.Process(this->PowerOutage_Duration)
		.Process(this->PowerOutage_Weight)
		.Process(this->PowerOutage_Anim)
		.Process(this->PowerOutage_Sound)
		.Process(this->PowerOutage_Message)

		.Process(this->Unit_RandomPick)
		.Process(this->Unit_Weight)
		.Process(this->Units)
		.Process(this->Units_Chances)
		.Process(this->Units_Anims)
		.Process(this->Units_Sounds)
		.Process(this->Units_Messages)
		.Process(this->Units_Nums)

		.Process(this->AllowPick_UnitTypes)
		.Process(this->AllowPick_InfantryTypes)

		.Process(this->DisallowPick_UnitTypes)
		.Process(this->DisallowPick_InfantryTypes)

		.Process(this->AllowPick_Houses)
		;
}

void CrateTypeClass::LoadFromStream(PhobosStreamReader& stm)
{
	this->Serialize(stm);
}

void CrateTypeClass::SaveToStream(PhobosStreamWriter& stm)
{
	this->Serialize(stm);
}
