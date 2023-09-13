#pragma once

#include <Utilities/Enumerable.h>
#include <Utilities/Template.h>

class IonCannonTypeClass final : public Enumerable<IonCannonTypeClass>
{
public:
	Valueable<int> IonCannon_Duration;
	Valueable<int> IonCannon_Radius;
	Valueable<int> IonCannon_MaxRadius;
	Valueable<int> IonCannon_MinRadius;
	Valueable<int> IonCannon_RadiusReduce;
	Valueable<int> IonCannon_RadiusReduceAcceleration;
	Valueable<int> IonCannon_RadiusReduceMax;
	Valueable<int> IonCannon_RadiusReduceMin;
	Valueable<int> IonCannon_Angle;
	Valueable<int> IonCannon_AngleAcceleration;
	Valueable<int> IonCannon_AngleMax;
	Valueable<int> IonCannon_AngleMin;
	Valueable<int> IonCannon_Lines;
	Valueable<bool> IonCannon_DrawLaser;
	Valueable<bool> IonCannon_DrawLaserWithFire;
	Valueable<int> IonCannon_LaserHeight;
	Valueable<bool> IonCannon_DrawEBolt;
	Valueable<bool> IonCannon_DrawEBoltWithFire;
	Valueable<int> IonCannon_EleHeight;
	Nullable<ColorStruct> IonCannon_InnerColor;
	Nullable<ColorStruct> IonCannon_OuterColor;
	Nullable<ColorStruct> IonCannon_OuterSpread;
	Valueable<int> IonCannon_LaserDuration;
	Valueable<int> IonCannon_Thickness;
	Valueable<bool> IonCannon_FireOnce;
	Valueable<int> IonCannon_Rate;
	Nullable<WeaponTypeClass*> IonCannon_Weapon;
	Valueable<int> IonCannon_ROF;
	Valueable<int> IonCannon_Scatter_Max;
	Valueable<int> IonCannon_Scatter_Min;
	Valueable<int> IonCannon_Scatter_Max_IncreaseMax;
	Valueable<int> IonCannon_Scatter_Max_IncreaseMin;
	Valueable<int> IonCannon_Scatter_Max_Increase;
	Valueable<int> IonCannon_Scatter_Min_IncreaseMax;
	Valueable<int> IonCannon_Scatter_Min_IncreaseMin;
	Valueable<int> IonCannon_Scatter_Min_Increase;

	IonCannonTypeClass(const char* pTitle = NONE_STR) : Enumerable<IonCannonTypeClass>(pTitle)
		, IonCannon_Duration { -1 }
		, IonCannon_Radius { 4096 }
		, IonCannon_MaxRadius { -1 }
		, IonCannon_MinRadius { -1 }
		, IonCannon_RadiusReduce { 20 }
		, IonCannon_RadiusReduceAcceleration { 0 }
		, IonCannon_RadiusReduceMax { 0 }
		, IonCannon_RadiusReduceMin { 0 }
		, IonCannon_Angle { 2 }
		, IonCannon_AngleAcceleration { 0 }
		, IonCannon_AngleMax { 0 }
		, IonCannon_AngleMin { 0 }
		, IonCannon_Lines { 8 }
		, IonCannon_DrawLaser { true }
		, IonCannon_DrawLaserWithFire { true }
		, IonCannon_LaserHeight { 20000 }
		, IonCannon_DrawEBolt { false }
		, IonCannon_DrawEBoltWithFire { true }
		, IonCannon_EleHeight { 4096 }
		, IonCannon_InnerColor { {255,0,0} }
		, IonCannon_OuterColor { {255,0,0} }
		, IonCannon_OuterSpread { {255,0,0} }
		, IonCannon_LaserDuration { 3 }
		, IonCannon_Thickness { 10 }
		, IonCannon_FireOnce { false }
		, IonCannon_Rate { 0 }
		, IonCannon_Weapon {}
		, IonCannon_ROF { 0 }
		, IonCannon_Scatter_Max { 0 }
		, IonCannon_Scatter_Min { 0 }
		, IonCannon_Scatter_Max_IncreaseMax { 0 }
		, IonCannon_Scatter_Max_IncreaseMin { 0 }
		, IonCannon_Scatter_Max_Increase { 0 }
		, IonCannon_Scatter_Min_IncreaseMax { 0 }
		, IonCannon_Scatter_Min_IncreaseMin { 0 }
		, IonCannon_Scatter_Min_Increase { 0 }
	{
	}

	virtual ~IonCannonTypeClass() override = default;

	virtual void LoadFromINI(CCINIClass* pINI) override;
	virtual void LoadFromStream(PhobosStreamReader& Stm);
	virtual void SaveToStream(PhobosStreamWriter& Stm);

private:
	template <typename T>
	void Serialize(T& Stm);
};
