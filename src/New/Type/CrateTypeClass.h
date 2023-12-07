#pragma once

#include <Utilities/Enum.h>
#include <Utilities/Enumerable.h>
#include <Utilities/Template.h>

class AttachEffectTypeClass;
class SuperWeaponTypeClass;

class CrateTypeClass : public Enumerable<CrateTypeClass>
{
public:

	int ArrayIndex;

	Valueable<int> Maximum;
	Valueable<int> Minimum;

	Valueable<int> Duration;
	Valueable<int> SpwanDelay;

	Valueable<AnimTypeClass*> Image;
	Valueable<bool> AllowSpwanOnLand;
	Valueable<AnimTypeClass*> WaterImage;
	Valueable<bool> AllowSpwanOnWater;

	Valueable<bool> Weapon_RandomPick;
	Valueable<int> Weapon_Weight;
	ValueableVector<WeaponTypeClass*> Weapons;
	ValueableVector<double> Weapons_Chances;
	ValueableVector<AnimTypeClass*> Weapons_Anims;
	NullableIdxVector<VocClass> Weapons_Sounds;
	ValueableVector<CSFText> Weapons_Messages;

	Valueable<bool> AttachEffect_RandomPick;
	Valueable<int> AttachEffect_Weight;
	ValueableVector<AttachEffectTypeClass*> AttachEffects;
	ValueableVector<double> AttachEffects_Chances;
	ValueableVector<AnimTypeClass*> AttachEffects_Anims;
	NullableIdxVector<VocClass> AttachEffects_Sounds;
	ValueableVector<CSFText> AttachEffects_Messages;

	Valueable<Vector2D<int>> Money;
	Valueable<int> Money_Weight;
	Valueable<AnimTypeClass*> Money_Anim;
	NullableIdx<VocClass> Money_Sound;
	Nullable<CSFText> Money_Message;
	Valueable<bool> Money_Display;
	Valueable<AffectedHouse> Money_DisplayHouses;
	Valueable<Point2D> Money_DisplayOffset;

	Valueable<Vector2D<int>> SpySat_Range;
	Valueable<int> SpySat_Weight;
	Valueable<AnimTypeClass*> SpySat_Anim;
	NullableIdx<VocClass> SpySat_Sound;
	Nullable<CSFText> SpySat_Message;

	Valueable<int> Darkness_Weight;
	Valueable<AnimTypeClass*> Darkness_Anim;
	NullableIdx<VocClass> Darkness_Sound;
	Nullable<CSFText> Darkness_Message;

	Valueable<bool> SuperWeapon_RandomPick;
	Valueable<int> SuperWeapon_Weight;
	ValueableIdxVector<SuperWeaponTypeClass> SuperWeapons;
	ValueableVector<double> SuperWeapons_Chances;
	ValueableVector<AnimTypeClass*> SuperWeapons_Anims;
	NullableIdxVector<VocClass> SuperWeapons_Sounds;
	ValueableVector<CSFText> SuperWeapons_Messages;
	ValueableVector<bool> SuperWeapons_Permanents;

	Valueable<Vector2D<int>> RadarJam_Duration;
	Valueable<int> RadarJam_Weight;
	Valueable<AnimTypeClass*> RadarJam_Anim;
	NullableIdx<VocClass> RadarJam_Sound;
	Nullable<CSFText> RadarJam_Message;

	Valueable<Vector2D<int>> PowerOutage_Duration;
	Valueable<int> PowerOutage_Weight;
	Valueable<AnimTypeClass*> PowerOutage_Anim;
	NullableIdx<VocClass> PowerOutage_Sound;
	Nullable<CSFText> PowerOutage_Message;

	Valueable<bool> Unit_RandomPick;
	Valueable<int> Unit_Weight;
	ValueableVector<TechnoTypeClass*> Units;
	ValueableVector<double> Units_Chances;
	ValueableVector<AnimTypeClass*> Units_Anims;
	NullableIdxVector<VocClass> Units_Sounds;
	ValueableVector<CSFText> Units_Messages;
	ValueableVector<Vector2D<int>> Units_Nums;

	ValueableVector<TechnoTypeClass*> AllowPick_UnitTypes;
	ValueableVector<TechnoTypeClass*> AllowPick_InfantryTypes;

	ValueableVector<TechnoTypeClass*> DisallowPick_UnitTypes;
	ValueableVector<TechnoTypeClass*> DisallowPick_InfantryTypes;

	CrateTypeClass(const char* pTitle = NONE_STR) : Enumerable<CrateTypeClass>(pTitle)
		, ArrayIndex(Array.size())

		, Maximum(-1)
		, Minimum(-1)

		, Duration(-1)
		, SpwanDelay(-1)

		, Image(nullptr)
		, AllowSpwanOnLand(true)
		, WaterImage(nullptr)
		, AllowSpwanOnWater(true)

		, Weapon_RandomPick(true)
		, Weapon_Weight(0)
		, Weapons()
		, Weapons_Chances()
		, Weapons_Anims()
		, Weapons_Sounds()
		, Weapons_Messages()

		, AttachEffect_RandomPick(true)
		, AttachEffect_Weight(0)
		, AttachEffects()
		, AttachEffects_Chances()
		, AttachEffects_Anims()
		, AttachEffects_Sounds()
		, AttachEffects_Messages()

		, Money({ 5000,-1 })
		, Money_Weight(100)
		, Money_Anim(nullptr)
		, Money_Sound()
		, Money_Message()
		, Money_Display(true)
		, Money_DisplayHouses(AffectedHouse::All)
		, Money_DisplayOffset({ 0, 0 })

		, SpySat_Range({ -1,-1 })
		, SpySat_Weight(0)
		, SpySat_Anim(nullptr)
		, SpySat_Sound()
		, SpySat_Message()

		, Darkness_Weight(0)
		, Darkness_Anim(nullptr)
		, Darkness_Sound()
		, Darkness_Message()

		, SuperWeapon_RandomPick(true)
		, SuperWeapon_Weight(0)
		, SuperWeapons()
		, SuperWeapons_Chances()
		, SuperWeapons_Anims()
		, SuperWeapons_Sounds()
		, SuperWeapons_Messages()
		, SuperWeapons_Permanents()

		, RadarJam_Duration({ 900,-1 })
		, RadarJam_Weight(0)
		, RadarJam_Anim(nullptr)
		, RadarJam_Sound()
		, RadarJam_Message()

		, PowerOutage_Duration({ 900,-1 })
		, PowerOutage_Weight(0)
		, PowerOutage_Anim(nullptr)
		, PowerOutage_Sound()
		, PowerOutage_Message()

		, Unit_RandomPick(true)
		, Unit_Weight(0)
		, Units()
		, Units_Chances()
		, Units_Anims()
		, Units_Sounds()
		, Units_Messages()
		, Units_Nums()

		, AllowPick_UnitTypes()
		, AllowPick_InfantryTypes()

		, DisallowPick_UnitTypes()
		, DisallowPick_InfantryTypes()
	{
	}

	virtual ~CrateTypeClass() = default;

	virtual void LoadFromINI(CCINIClass* pINI);
	virtual void LoadFromStream(PhobosStreamReader& stm);
	virtual void SaveToStream(PhobosStreamWriter& stm);

private:

	template <typename T>
	void Serialize(T& stm);
};
