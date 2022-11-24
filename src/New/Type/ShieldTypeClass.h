#pragma once

#include <Utilities/Enumerable.h>
#include <Utilities/Template.h>
#include <Utilities/GeneralUtils.h>
#include <Ext/Rules/Body.h>
#include <Utilities/TemplateDef.h>

class DigitalDisplayTypeClass;

class ShieldTypeClass final : public Enumerable<ShieldTypeClass>
{
public:
	Valueable<int> Strength;
	Nullable<int> InitialStrength;
	Valueable<ArmorType> Armor;
	Valueable<bool> Powered;
	Valueable<double> Respawn;
	Valueable<int> Respawn_Rate;
	Valueable<double> SelfHealing;
	Valueable<int> SelfHealing_Rate;

	Valueable<bool> Directional;
	Valueable<float> Directional_FrontMultiplier;
	Valueable<float> Directional_SideMultiplier;
	Valueable<float> Directional_BackMultiplier;
	Valueable<float> Directional_FrontField;
	Valueable<float> Directional_BackField;

	Valueable<bool> AbsorbOverDamage;
	Valueable<int> BracketDelta;
	Valueable<AttachedAnimFlag> IdleAnim_OfflineAction;
	Valueable<AttachedAnimFlag> IdleAnim_TemporalAction;
	Damageable<AnimTypeClass*> IdleAnim;
	Damageable<AnimTypeClass*> IdleAnimDamaged;
	Nullable<AnimTypeClass*> BreakAnim;
	Nullable<WeaponTypeClass*> BreakWeapon;
	ValueableVector<AnimTypeClass*> HitAnim;
	Valueable<bool> HitAnim_PickRandom;
	Valueable<bool> HitAnim_PickByDirection;
	Valueable<double> AbsorbPercent;
	Valueable<double> PassPercent;

	Valueable<bool>	CanBeStolen;
	Valueable<bool>	CanBeStolenType;

	Nullable<bool> AllowTransfer;

	Nullable<Vector3D<int>> Pips;

	Nullable<int> Pips_Length;
	Nullable<int> Pips_XOffset;
	Nullable<Vector2D<int>> Pips_DrawOffset;

	Nullable<SHPStruct*> Pips_SHP;
	CustomPalette Pips_PAL;

	Nullable<SHPStruct*> Pips_Background;
	CustomPalette Pips_Background_PAL;

	Nullable<Vector3D<int>> Pips_Building;
	Nullable<int> Pips_Building_Empty;

	Valueable<bool> ImmuneToBerserk;

	Nullable<int> PipBrd;
	Nullable<Vector2D<int>> PipBrd_Offset;

	ValueableVector<TechnoTypeClass*> PoweredTechnos;
	Valueable<bool> PoweredTechnos_Any;

private:
	Valueable<double> Respawn_Rate__InMinutes;
	Valueable<double> SelfHealing_Rate__InMinutes;

public:
	ShieldTypeClass(const char* const pTitle) : Enumerable<ShieldTypeClass>(pTitle)
		, Strength(0)
		, InitialStrength()
		, Armor(Armor::None)
		, Powered(false)
		, Respawn(0.0)
		, Respawn_Rate(0)
		, SelfHealing(0.0)
		, SelfHealing_Rate(0)
		, Directional(false)
		, Directional_FrontMultiplier(1.0)
		, Directional_SideMultiplier(1.0)
		, Directional_BackMultiplier(1.0)
		, Directional_FrontField(0.5)
		, Directional_BackField(0.5)
		, AbsorbOverDamage(false)
		, BracketDelta(0)
		, IdleAnim_OfflineAction(AttachedAnimFlag::Hides)
		, IdleAnim_TemporalAction(AttachedAnimFlag::Hides)
		, IdleAnim()
		, IdleAnimDamaged()
		, BreakAnim()
		, BreakWeapon()
		, HitAnim()
		, HitAnim_PickRandom(false)
		, HitAnim_PickByDirection(false)
		, AbsorbPercent(1.0)
		, PassPercent(0.0)
		, Respawn_Rate__InMinutes(0.0)
		, SelfHealing_Rate__InMinutes(0.0)
		, AllowTransfer()
		, Pips_Length {}
		, Pips_XOffset {}
		, Pips_DrawOffset {}
		, Pips_SHP {}
		, Pips_PAL {}
		, Pips_Background_PAL {}
		, Pips { { -1, -1, -1 } }
		, Pips_Background { }
		, Pips_Building { { -1, -1, -1 } }
		, Pips_Building_Empty { }
		, ImmuneToBerserk { false }
		, PipBrd { }
		, PipBrd_Offset { }
		, CanBeStolen(true)
		, CanBeStolenType(true)
		, PoweredTechnos { }
		, PoweredTechnos_Any(true)
	{};

	virtual ~ShieldTypeClass() override = default;

	virtual void LoadFromINI(CCINIClass* pINI) override;
	virtual void LoadFromStream(PhobosStreamReader& Stm);
	virtual void SaveToStream(PhobosStreamWriter& Stm);

	AnimTypeClass* GetIdleAnimType(bool isDamaged, double healthRatio);

private:
	template <typename T>
	void Serialize(T& Stm);
};
