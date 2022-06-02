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
	ArmorType Armor;
	Valueable<bool> Powered;
	Valueable<double> Respawn;
	Valueable<int> Respawn_Rate;
	Valueable<double> SelfHealing;
	Valueable<int> SelfHealing_Rate;

	Valueable<bool> AbsorbOverDamage;
	Valueable<int> BracketDelta;
	Valueable<AttachedAnimFlag> IdleAnim_OfflineAction;
	Valueable<AttachedAnimFlag> IdleAnim_TemporalAction;
	Damageable<AnimTypeClass*> IdleAnim;
	Damageable<AnimTypeClass*> IdleAnimDamaged;
	Nullable<AnimTypeClass*> BreakAnim;
	Nullable<AnimTypeClass*> HitAnim;
	Nullable<WeaponTypeClass*> BreakWeapon;
	Valueable<double> AbsorbPercent;
	Valueable<double> PassPercent;

	Valueable<bool>	CanBeStolen;
	Valueable<bool>	CanBeStolenType;

	Nullable<bool> AllowTransfer;

	Nullable<Vector3D<int>> Pips;

	Nullable<int> Pips_Length;
	Nullable<int> Pips_XOffset;
	Nullable<Vector2D<int>> Pips_DrawOffset;

	PhobosFixedString<32u> Pips_Filename;
	PhobosFixedString<32u> Pips_PALFilename;
	PhobosFixedString<32u> Pips_Background_PALFilename;

	SHPStruct* Pips_SHP;
	ConvertClass* Pips_PAL;

	Nullable<SHPStruct*> Pips_Background;
	ConvertClass* Pips_Background_PAL;

	Nullable<Vector3D<int>> Pips_Building;
	Nullable<int> Pips_Building_Empty;

	Nullable<DigitalDisplayTypeClass*> DigitalDisplayType;

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
		, AbsorbOverDamage(false)
		, BracketDelta(0)
		, IdleAnim_OfflineAction(AttachedAnimFlag::Hides)
		, IdleAnim_TemporalAction(AttachedAnimFlag::Hides)
		, IdleAnim()
		, IdleAnimDamaged()
		, BreakAnim()
		, HitAnim()
		, BreakWeapon()
		, AbsorbPercent(1.0)
		, PassPercent(0.0)
		, Respawn_Rate__InMinutes(0.0)
		, SelfHealing_Rate__InMinutes(0.0)
		, AllowTransfer()
		, Pips_Length {}
		, Pips_XOffset {}
		, Pips_DrawOffset {}
		, Pips_Filename { "" }
		, Pips_PALFilename { "" }
		, Pips_Background_PALFilename { "" }
		, Pips_SHP { nullptr }
		, Pips_PAL { nullptr }
		, Pips_Background_PAL { nullptr }
		, Pips { { -1,-1,-1 } }
		, Pips_Background { }
		, Pips_Building { { -1,-1,-1 } }
		, Pips_Building_Empty { }
		, CanBeStolen(true)
		, CanBeStolenType(true)
		, DigitalDisplayType()
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
