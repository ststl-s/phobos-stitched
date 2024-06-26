#pragma once

#include <Utilities/Enum.h>
#include <Utilities/Enumerable.h>
#include <Utilities/Template.h>

#include <New/AnonymousType/AttackedWeapon.h>

class CrateTypeClass;

class AttachEffectTypeClass : public Enumerable<AttachEffectTypeClass>
{
public:

	Valueable<int> FirePower;
	Valueable<int> ROF;
	Valueable<int> Armor;
	Valueable<int> Speed;
	Valueable<int> ROT;
	Valueable<Leptons> Range;
	Valueable<double> Weight;
	Valueable<double> FirePower_Multiplier;
	Valueable<double> ROF_Multiplier;
	Valueable<double> Armor_Multiplier;
	Valueable<double> Speed_Multiplier;
	Valueable<double> ROT_Multiplier;
	Valueable<double> Range_Multiplier;
	Valueable<double> Weight_Multiplier;
	Valueable<float> Scale;

	Nullable<bool> DecloakToFire;
	Valueable<bool> Cloak;
	Valueable<bool> Decloak;
	Valueable<bool> Sensor;

	Valueable<int> AllowMinHealth;
	Valueable<double> AllowMinHealth_Percentage;

	Valueable<bool> DisableWeapon;
	Valueable<DisableWeaponCate> DisableWeapon_Category;

	ValueableVector<AnimTypeClass*> Anim;
	ValueableVector<AnimTypeClass*> EndedAnim;
	Nullable<CoordStruct> Anim_FLH;
	std::vector<CoordStruct> Anim_FLHs;
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

	std::unique_ptr<AttackedWeaponTypeClass> AttackedWeaponType;

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

	Valueable<int> DiscardAfterShoots;
	Valueable<int> DiscardAfterHits;
	Valueable<int> MaxReceive;

	ValueableVector<TechnoTypeClass*> AuxTechnos;
	ValueableVector<TechnoTypeClass*> NegTechnos;

	ValueableVector<WeaponTypeClass*> FireOnOwner;
	ValueableVector<WeaponTypeClass*> OwnerFireOn;

	Valueable<bool> DisableBeSelect;
	Valueable<AffectedHouse> DisableBeTarget;

	Nullable<WeaponTypeClass*> RevengeWeapon;
	Valueable<AffectedHouse> RevengeWeapon_AffectsHouses;
	Nullable<WeaponTypeClass*> RevengeWeaponAttach;
	Valueable<AffectedHouse> RevengeWeaponAttach_AffectsHouses;

	Valueable<bool> ForbiddenSelfHeal;
	Valueable<bool> EMP;
	Valueable<bool> Psychedelic;
	Valueable<int> SensorsSight;
	Valueable<int> RevealSight;
	Valueable<int> GapSight;

	Valueable<double> Dodge_Chance;
	Valueable<AffectedHouse> Dodge_Houses;
	Valueable<double> Dodge_MaxHealthPercent;
	Valueable<double> Dodge_MinHealthPercent;
	Nullable<AnimTypeClass*> Dodge_Anim;
	Valueable<bool> Dodge_OnlyDodgePositiveDamage;

	Valueable<double> MissHit_Chance;
	Valueable<AffectedHouse> MissHit_Houses;
	Valueable<double> MissHit_MaxHealthPercent;
	Valueable<double> MissHit_MinHealthPercent;
	Nullable<AnimTypeClass*> MissHit_Anim;
	Valueable<bool> MissHit_OnlyMissPositiveDamage;

	Valueable<int> MoveDamage;
	Valueable<int> MoveDamage_Delay;
	Valueable<WarheadTypeClass*> MoveDamage_Warhead;
	Valueable<AnimTypeClass*> MoveDamage_Anim;

	Valueable<int> StopDamage;
	Valueable<int> StopDamage_Delay;
	Valueable<WarheadTypeClass*> StopDamage_Warhead;
	Valueable<AnimTypeClass*> StopDamage_Anim;

	Valueable<Leptons> Blackhole_Range;
	Valueable<Leptons> Blackhole_MinRange;
	Valueable<bool> Blackhole_Destory;
	Valueable<bool> Blackhole_Destory_TakeDamage;
	Valueable<double> Blackhole_Destory_TakeDamageMultiplier;
	Valueable<AffectedHouse> Blackhole_AffectedHouse;

	Valueable<double> Crit_Multiplier;
	Valueable<double> Crit_ExtraChance;
	ValueableVector<WarheadTypeClass*> Crit_AllowWarheads;
	ValueableVector<WarheadTypeClass*> Crit_DisallowWarheads;

	ValueableVector<CrateTypeClass*> CreateCrateTypes;

	Valueable<bool> LimitDamage;
	Valueable<Vector2D<int>> LimitDamage_MaxDamage;
	Valueable<Vector2D<int>> LimitDamage_MinDamage;

	Valueable<bool> PsychicDetection;
	Valueable<AffectedHouse> PsychicDetection_ReceiveHouses;

	Nullable<bool> BaseNormal;
	Nullable<bool> EligibileForAllyBuilding;

	Valueable<int> Power;
	Valueable<int> ExtraPower;
	Valueable<double> Power_Multiplier;
	Valueable<double> ExtraPower_Multiplier;

	int ArrayIndex;

	AttachEffectTypeClass(const char* pTitle = NONE_STR) : Enumerable<AttachEffectTypeClass>(pTitle)
		, FirePower(0)
		, ROF(0)
		, Armor(0)
		, Speed(0)
		, ROT(0)
		, Range(Leptons(0))
		, Weight(0.0)
		, FirePower_Multiplier(1.0)
		, ROF_Multiplier(1.0)
		, Armor_Multiplier(1.0)
		, Speed_Multiplier(1.0)
		, ROT_Multiplier(1.0)
		, Range_Multiplier(1.0)
		, Weight_Multiplier(1.0)
		, Scale(1.0f)
		, DisableWeapon(false)
		, DisableWeapon_Category(DisableWeaponCate::None)
		, DecloakToFire()
		, Cloak(false)
		, Decloak(false)
		, Sensor(false)
		, AllowMinHealth(0)
		, AllowMinHealth_Percentage(0.0)
		, Anim()
		, EndedAnim()
		, Anim_FLH(CoordStruct::Empty)
		, Anim_FLHs()
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
		, ReplaceWeapon(false)
		, ReplacePrimary(nullptr)
		, ReplaceSecondary(nullptr)
		, ReplaceGattlingWeapon()
		, ReplaceDeathWeapon()
		, AttackedWeaponType(nullptr)
		, ForceExplode(false)
		, InfDeathAnim(nullptr)
		, PenetratesIronCurtain(false)
		, DiscardOnEntry(false)
		, DiscardAfterShoots(INT_MAX)
		, Cumulative(false)
		, Cumulative_Maximum(-1)
		, IfExist_IgnoreOwner(true)
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
		, MaxReceive(-1)
		, AuxTechnos()
		, NegTechnos()
		, FireOnOwner()
		, OwnerFireOn()
		, DisableBeSelect(false)
		, DisableBeTarget(AffectedHouse::None)
		, RevengeWeapon()
		, RevengeWeapon_AffectsHouses(AffectedHouse::All)
		, RevengeWeaponAttach()
		, RevengeWeaponAttach_AffectsHouses()
		, ForbiddenSelfHeal(false)
		, EMP(false)
		, Psychedelic(false)
		, SensorsSight(0)
		, RevealSight(0)
		, GapSight(0)

		, Dodge_Chance(0.0)
		, Dodge_Houses(AffectedHouse::All)
		, Dodge_MaxHealthPercent(1.0)
		, Dodge_MinHealthPercent(0.0)
		, Dodge_Anim {}
		, Dodge_OnlyDodgePositiveDamage(true)

		, MissHit_Chance(0.0)
		, MissHit_Houses(AffectedHouse::All)
		, MissHit_MaxHealthPercent(1.0)
		, MissHit_MinHealthPercent(0.0)
		, MissHit_Anim {}
		, MissHit_OnlyMissPositiveDamage(true)

		, MoveDamage(0)
		, MoveDamage_Delay(0)
		, MoveDamage_Warhead(nullptr)
		, MoveDamage_Anim(nullptr)

		, StopDamage(0)
		, StopDamage_Delay(0)
		, StopDamage_Warhead(nullptr)
		, StopDamage_Anim(nullptr)

		, Blackhole_Range()
		, Blackhole_MinRange()
		, Blackhole_Destory(false)
		, Blackhole_Destory_TakeDamage(true)
		, Blackhole_Destory_TakeDamageMultiplier(1.0)
		, Blackhole_AffectedHouse(AffectedHouse::Enemies)

		, Crit_Multiplier(1.0)
		, Crit_ExtraChance(0.0)
		, Crit_AllowWarheads()
		, Crit_DisallowWarheads()

		, CreateCrateTypes()

		, LimitDamage(false)
		, LimitDamage_MaxDamage({ INT_MAX, -INT_MAX })
		, LimitDamage_MinDamage({ -INT_MAX, INT_MAX })

		, PsychicDetection(false)
		, PsychicDetection_ReceiveHouses(AffectedHouse::Owner)

		, BaseNormal()
		, EligibileForAllyBuilding()

		, Power(0)
		, ExtraPower(0)
		, Power_Multiplier(1.0)
		, ExtraPower_Multiplier(1.0)
	{ }

	virtual ~AttachEffectTypeClass() = default;

	virtual void LoadFromINI(CCINIClass* pINI);
	virtual void LoadFromStream(PhobosStreamReader& stm);
	virtual void SaveToStream(PhobosStreamWriter& stm);

private:

	template <typename T>
	void Serialize(T& stm);
};
