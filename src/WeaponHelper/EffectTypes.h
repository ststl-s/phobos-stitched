#pragma once

#include <GeneralDefinitions.h>
#include <WeaponTypeClass.h>

struct WeaponLaserType
{
	ColorStruct InnerColor;
	ColorStruct OuterColor;
	ColorStruct OuterSpread;
	int Duration;
	int Thickness;
	bool IsHouseColor;
	bool IsSupported;
	bool Fade;

	WeaponLaserType() = default;

	WeaponLaserType
	(
		ColorStruct innerColor,
		ColorStruct outerColor,
		int duration,
		int thickness,
		ColorStruct outerSpread = ColorStruct::Black,
		bool isHouseColor = false,
		bool isSupported = false,
		bool fade = true
	);

	WeaponLaserType(const WeaponTypeClass* pWeapon);
};

class WeaponBeamType
{
public:
	RadBeamType BeamType;
	ColorStruct BeamColor;
	int Period;
	double Amplitude;

	WeaponBeamType(RadBeamType beamType)
		: Period(15)
		, Amplitude(40.0)
	{
		SetBeamType(beamType);
	}

	void SetBeamType(RadBeamType beamType)
	{
		this->BeamType = beamType;
		switch (beamType)
		{
		case RadBeamType::Temporal:
			this->BeamColor = ColorStruct(128, 200, 255);
			break;
		default:
			this->BeamColor = ColorStruct::Green;
			break;
		}
	}
};

class WeaponEBoltType
{
public:
	bool IsAlternateColor;
	ColorStruct Color1;
	ColorStruct Color2;
	ColorStruct Color3;
	bool Disable1;
	bool Disable2;
	bool Disable3;

	WeaponEBoltType(bool alternate)
	{
		IsAlternateColor = alternate;
		Color1 = ColorStruct::Black;
		Color2 = ColorStruct::Black;
		Color3 = ColorStruct::Black;
		Disable1 = false;
		Disable2 = false;
		Disable3 = false;
	}
};
