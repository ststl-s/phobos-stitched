#pragma once

#include <Utilities/Enum.h>
#include <Utilities/Enumerable.h>
#include <Utilities/Template.h>

class AttachEffectTypeClass : public Enumerable<AttachEffectTypeClass>
{
public:

	Valueable<int> FirePower;
	Valueable<int> ROF;
	Valueable<int> Armor;
	Valueable<int> Speed;
	Valueable<int> ROT;
	Valueable<int> Range;
	Valueable<double> FirePower_Multiplier;
	Valueable<double> ROF_Multiplier;
	Valueable<double> Armor_Multiplier;
	Valueable<double> Speed_Multiplier;
	Valueable<double> ROT_Multiplier;
	Valueable<double> Range_Multiplier;
	Valueable<bool> Cloak;
	Valueable<bool> Decloak;
	Valueable<bool> Sensor;
	Valueable<bool> DisableWeapon;
	Valueable<int> AllowMinHealth;
	Valueable<DisableWeaponCate> DisableWeapon_Category;
	ValueableVector<AnimTypeClass*> Anim;
	ValueableVector<AnimTypeClass*> EndedAnim;
	Valueable<bool> Anim_RandomPick;
	Valueable<bool> EndedAnim_RandomPick;
	ValueableVector<WeaponTypeClass*> WeaponList;
	ValueableVector<WeaponTypeClass*> AttackedWeaponList;
	Valueable<bool> RandomDuration;
	Valueable<Vector2D<int>> RandomDuration_Interval;
	Valueable<int> Duration;
	Valueable<int> Delay;
	Valueable<int> Loop_Delay;
	Nullable<int> Loop_Duration;
	Valueable<bool> ImmuneMindControl;
	Valueable<bool> ReplaceWeapon;
	Promotable<WeaponTypeClass*> ReplacePrimary;
	Promotable<WeaponTypeClass*> ReplaceSecondary;
	PromotableVector<WeaponTypeClass*> ReplaceGattlingWeapon;
	Nullable<ArmorType> ReplaceArmor;
	Nullable<ArmorType> ReplaceArmor_Shield;
	Nullable<WeaponTypeClass*> ReplaceDeathWeapon;
	Valueable<bool> ForceExplode;
	Valueable<AnimTypeClass*> InfDeathAnim;
	Nullable<int> Coexist_Maximum;
	Valueable<bool> WeaponList_FireOnAttach;
	Valueable<bool> PenetratesIronCurtain;
	Valueable<bool> DiscardOnEntry;
	Valueable<bool> Cumulative;
	Valueable<int> Cumulative_Maximum;
	Valueable<bool> IfExist_IgnoreOwner;
	Valueable<int> IfExist_AddTimer;
	Valueable<int> IfExist_AddTimer_Cap;
	Valueable<bool> IfExist_ResetTimer;
	Valueable<bool> IfExist_ResetAnim;
	Valueable<bool> ShowAnim_Cloaked;
	Valueable<bool> HideImage;
	ValueableVector<ColorStruct> Tint_Colors;
	Valueable<int> Tint_TransitionDuration;
	ValueableVector<AttachEffectTypeClass*> NextAttachEffects;
	Valueable<int> DiscardAfterHits;
	Valueable<int> MaxReceive;
	ValueableVector<TechnoTypeClass*> AuxTechnos;
	ValueableVector<TechnoTypeClass*> NegTechnos;
	ValueableVector<WeaponTypeClass*> FireOnOwner;
	ValueableVector<WeaponTypeClass*> OwnerFireOn;
	Valueable<bool> DisableBeSelect;
	Valueable<bool> DisableBeTarget;
	Nullable<WeaponTypeClass*> RevengeWeapon;
	Valueable<AffectedHouse> RevengeWeapon_AffectsHouses;
	Nullable<WeaponTypeClass*> RevengeWeaponAttach;
	Valueable<AffectedHouse> RevengeWeaponAttach_AffectsHouses;
	Valueable<bool> ForbiddenSelfHeal;
	Valueable<bool> EMP;
	Valueable<bool> Psychedelic;
	Valueable<int> SensorsSight;
	Valueable<int> RevealSight;

	Valueable<double> Dodge_Chance;
	Valueable<AffectedHouse> Dodge_Houses;
	Valueable<double> Dodge_MaxHealthPercent;
	Valueable<double> Dodge_MinHealthPercent;
	Nullable<AnimTypeClass*> Dodge_Anim;
	Valueable<bool> Dodge_OnlyDodgePositiveDamage;

	Valueable<int> MoveDamage;
	Valueable<int> MoveDamage_Delay;
	Valueable<WarheadTypeClass*> MoveDamage_Warhead;
	Valueable<AnimTypeClass*> MoveDamage_Anim;

	Valueable<int> StopDamage;
	Valueable<int> StopDamage_Delay;
	Valueable<WarheadTypeClass*> StopDamage_Warhead;
	Valueable<AnimTypeClass*> StopDamage_Anim;

	Valueable<double> Blackhole_Range;
	Valueable<double> Blackhole_MinRange;
	Valueable<bool> Blackhole_Destory;
	Valueable<bool> Blackhole_Destory_TakeDamage;
	Valueable<double> Blackhole_Destory_TakeDamageMultiplier;
	Valueable<AffectedHouse> Blackhole_AffectedHouse;

	Nullable<bool> BaseNormal;
	Nullable<bool> EligibileForAllyBuilding;

	int ArrayIndex;

	AttachEffectTypeClass(const char* pTitle = NONE_STR) : Enumerable<AttachEffectTypeClass>(pTitle)
		, FirePower(0)
		, ROF(0)
		, Armor(0)
		, Speed(0)
		, ROT(0)
		, Range(0)
		, FirePower_Multiplier(1.0)
		, ROF_Multiplier(1.0)
		, Armor_Multiplier(1.0)
		, Speed_Multiplier(1.0)
		, ROT_Multiplier(1.0)
		, Range_Multiplier(1.0)
		, DisableWeapon(false)
		, DisableWeapon_Category(DisableWeaponCate::CantFire)
		, Cloak(false)
		, Decloak(false)
		, Sensor(false)
		, AllowMinHealth(0)
		, Anim()
		, EndedAnim()
		, Anim_RandomPick(false)
		, WeaponList()
		, WeaponList_FireOnAttach(false)
		, AttackedWeaponList()
		, Duration(0)
		, RandomDuration(false)
		, RandomDuration_Interval(Vector2D<int>::Empty)
		, Delay(0)
		, ImmuneMindControl(false)
		, ReplaceArmor()
		, ReplaceArmor_Shield()
		, ReplaceWeapon()
		, ReplacePrimary(nullptr)
		, ReplaceSecondary(nullptr)
		, ReplaceGattlingWeapon()
		, ReplaceDeathWeapon()
		, ForceExplode(false)
		, InfDeathAnim()
		, PenetratesIronCurtain(false)
		, DiscardOnEntry(false)
		, Cumulative(false)
		, Cumulative_Maximum(INT_MAX)
		, IfExist_IgnoreOwner(false)
		, IfExist_AddTimer(0)
		, IfExist_AddTimer_Cap(-1)
		, IfExist_ResetTimer(true)
		, IfExist_ResetAnim(false)
		, Loop_Delay(0)
		, Loop_Duration()
		, ShowAnim_Cloaked(false)
		, HideImage(false)
		, Coexist_Maximum()
		, ArrayIndex(Array.size())
		, Tint_Colors()
		, Tint_TransitionDuration(60)
		, NextAttachEffects()
		, DiscardAfterHits()
		, MaxReceive(0)
		, AuxTechnos {}
		, NegTechnos {}
		, FireOnOwner {}
		, OwnerFireOn {}
		, DisableBeSelect(false)
		, DisableBeTarget(false)
		, RevengeWeapon {}
		, RevengeWeapon_AffectsHouses { AffectedHouse::All }
		, RevengeWeaponAttach {}
		, RevengeWeaponAttach_AffectsHouses { AffectedHouse::All }
		, ForbiddenSelfHeal(false)
		, EMP(false)
		, Psychedelic(false)
		, SensorsSight(0)
		, RevealSight(0)

		, Dodge_Chance(0.0)
		, Dodge_Houses { AffectedHouse::All }
		, Dodge_MaxHealthPercent(1.0)
		, Dodge_MinHealthPercent(0.0)
		, Dodge_Anim {}
		, Dodge_OnlyDodgePositiveDamage(true)

		, MoveDamage(0)
		, MoveDamage_Delay(0)
		, MoveDamage_Warhead(nullptr)
		, MoveDamage_Anim(nullptr)

		, StopDamage(0)
		, StopDamage_Delay(0)
		, StopDamage_Warhead(nullptr)
		, StopDamage_Anim(nullptr)

		, Blackhole_Range { 0.0 }
		, Blackhole_MinRange { 0.0 }
		, Blackhole_Destory { false }
		, Blackhole_Destory_TakeDamage { true }
		, Blackhole_Destory_TakeDamageMultiplier { 1.0 }
		, Blackhole_AffectedHouse { AffectedHouse::Enemies }

		, BaseNormal {}
		, EligibileForAllyBuilding {}
	{ }

	virtual ~AttachEffectTypeClass() = default;

	virtual void LoadFromINI(CCINIClass* pINI);
	virtual void LoadFromStream(PhobosStreamReader& stm);
	virtual void SaveToStream(PhobosStreamWriter& stm);

private:

	template <typename T>
	void Serialize(T& stm);
};
