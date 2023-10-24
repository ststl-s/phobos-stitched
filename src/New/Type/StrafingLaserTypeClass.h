#pragma once

#include <Ext/Rules/Body.h>
#include <Utilities/Enumerable.h>
#include <Utilities/Template.h>

class StrafingLaserTypeClass final : public Enumerable<StrafingLaserTypeClass>
{
public:
	Nullable<ColorStruct> InnerColor;
	Valueable<ColorStruct> OuterColor;
	Valueable<ColorStruct> OuterSpread;
	Valueable<bool> IsHouseColor;
	Valueable<bool> IsSingleColor;
	Nullable<bool> IsSupported;
	Valueable<int> Duration;
	Valueable<int> Thickness;
	Nullable<int> EndThickness;

	Nullable<CoordStruct> FLH;
	Nullable<CoordStruct> SourceFLH;
	Nullable<CoordStruct> TargetFLH;
	Nullable<bool> InGround;
	Valueable<bool> SourceFromTarget;
	Valueable<int> Timer;
	Valueable<WeaponTypeClass*> Weapon;
	Valueable<int> Weapon_Delay;
	Valueable<WeaponTypeClass*> DetonateWeapon;

	StrafingLaserTypeClass(const char* pTitle = NONE_STR) : Enumerable<StrafingLaserTypeClass>(pTitle)
		, InnerColor {}
		, OuterColor { {0,0,0} }
		, OuterSpread { {0,0,0} }
		, IsHouseColor { false }
		, IsSingleColor { false }
		, IsSupported {}
		, Duration { 15 }
		, Thickness { 3 }
		, EndThickness {}

		, FLH {}
		, SourceFLH {}
		, TargetFLH {}
		, InGround {}
		, SourceFromTarget { false }
		, Timer { 30 }
		, Weapon { nullptr }
		, Weapon_Delay { 1 }
		, DetonateWeapon { nullptr }
	{}

	virtual ~StrafingLaserTypeClass() override = default;

	virtual void LoadFromINI(CCINIClass* pINI) override;
	virtual void LoadFromStream(PhobosStreamReader& Stm);
	virtual void SaveToStream(PhobosStreamWriter& Stm);

	static void AddNewINIList(CCINIClass* pINI, const char* pSection, const char* pKey);

private:
	template <typename T>
	void Serialize(T& Stm);
};
