/*
	Lasers
*/

#pragma once

#include <GeneralDefinitions.h>
#include <ProgressTimer.h>
#include <ArrayClasses.h>

class LaserDrawClass
{
public:

	//Array
	static constexpr constant_ptr<DynamicVectorClass<LaserDrawClass*>, 0xABC878u> const Array {};

	//Constructor, Destructor
	LaserDrawClass(const CoordStruct& source, const CoordStruct& target, const ColorStruct& innerColor,
		const ColorStruct& outerColor, const ColorStruct& outerSpread, int duration)
			: LaserDrawClass(source, target, 0, 1, innerColor, outerColor, outerSpread, duration)
	{ }

	LaserDrawClass(CoordStruct source, CoordStruct target, int zAdjust, BYTE unknown,
		ColorStruct innerColor, ColorStruct outerColor, ColorStruct outerSpread,
		int duration, bool blinks = false, bool fades = true,
		float startIntensity = 1.0f, float endIntensity = 0.0f)
		{ JMP_THIS(0x54FE60); }

	~LaserDrawClass()
		{ JMP_THIS(0x54FFB0); }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================
public:
	DECLARE_PROPERTY(ProgressTimer, Progress);
	int Thickness; // only respected if IsHouseColor
	bool IsHouseColor;
	bool IsSupported; // this changes the values for InnerColor (false: halve, true: double), HouseColor only
	PROTECTED_PROPERTY(BYTE, align_22[2]);
	CoordStruct Source;
	CoordStruct Target;
	int ZAdjust;
	char field_40;
	ColorStruct InnerColor;
	ColorStruct OuterColor;
	ColorStruct OuterSpread;
	PROTECTED_PROPERTY(BYTE, align_4A[2]);
	int Duration;
	bool Blinks;
	bool BlinkState;
	bool Fades;
	PROTECTED_PROPERTY(BYTE, align_53);
	float StartIntensity;
	float EndIntensity;
};
