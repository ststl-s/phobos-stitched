#include "IonCannonTypeClass.h"

#include <Utilities/TemplateDef.h>

Enumerable<IonCannonTypeClass>::container_t Enumerable<IonCannonTypeClass>::Array;

const char* Enumerable<IonCannonTypeClass>::GetMainSection()
{
	return "IonCannonTypes";
}

void IonCannonTypeClass::LoadFromINI(CCINIClass* pINI)
{
	const char* section = this->Name;

	INI_EX exINI(pINI);

	this->IonCannon_Duration.Read(exINI, section, "IonCannon.Duration");
	this->IonCannon_Radius.Read(exINI, section, "IonCannon.Radius");
	this->IonCannon_MaxRadius.Read(exINI, section, "IonCannon.MaxRadius");
	this->IonCannon_MinRadius.Read(exINI, section, "IonCannon.MinRadius");
	this->IonCannon_RadiusReduce.Read(exINI, section, "IonCannon.RadiusReduce");
	this->IonCannon_RadiusReduceAcceleration.Read(exINI, section, "IonCannon.RadiusReduce.Acceleration");
	this->IonCannon_RadiusReduceMax.Read(exINI, section, "IonCannon.RadiusReduce.Max");
	this->IonCannon_RadiusReduceMin.Read(exINI, section, "IonCannon.RadiusReduce.Min");
	this->IonCannon_Angle.Read(exINI, section, "IonCannon.Angle");
	this->IonCannon_AngleAcceleration.Read(exINI, section, "IonCannon.Angle.Acceleration");
	this->IonCannon_AngleMax.Read(exINI, section, "IonCannon.Angle.Max");
	this->IonCannon_AngleMin.Read(exINI, section, "IonCannon.Angle.Min");
	this->IonCannon_Lines.Read(exINI, section, "IonCannon.Lines");
	this->IonCannon_DrawLaser.Read(exINI, section, "IonCannon.DrawLaser");
	this->IonCannon_DrawLaserWithFire.Read(exINI, section, "IonCannon.DrawLaserWithFire");
	this->IonCannon_LaserHeight.Read(exINI, section, "IonCannon.LaserHeight");
	this->IonCannon_DrawEBolt.Read(exINI, section, "IonCannon.DrawEBolt");
	this->IonCannon_DrawEBoltWithFire.Read(exINI, section, "IonCannon.DrawEBoltWithFire");
	this->IonCannon_EleHeight.Read(exINI, section, "IonCannon.EleHeight");
	this->IonCannon_InnerColor.Read(exINI, section, "IonCannon.InnerColor");
	this->IonCannon_OuterColor.Read(exINI, section, "IonCannon.OuterColor");
	this->IonCannon_OuterSpread.Read(exINI, section, "IonCannon.OuterSpread");
	this->IonCannon_LaserDuration.Read(exINI, section, "IonCannon.LaserDuration");
	this->IonCannon_Thickness.Read(exINI, section, "IonCannon.Thickness");
	this->IonCannon_FireOnce.Read(exINI, section, "IonCannon.FireOnce");
	this->IonCannon_Rate.Read(exINI, section, "IonCannon.Rate");
	this->IonCannon_Weapon.Read(exINI, section, "IonCannon.Weapon", true);
	this->IonCannon_ROF.Read(exINI, section, "IonCannon.ROF");
	this->IonCannon_Scatter_Max.Read(exINI, section, "IonCannon.Scatter.Max");
	this->IonCannon_Scatter_Min.Read(exINI, section, "IonCannon.Scatter.Min");
	this->IonCannon_Scatter_Max_IncreaseMax.Read(exINI, section, "IonCannon.Scatter.Max.IncreaseMax");
	this->IonCannon_Scatter_Max_IncreaseMin.Read(exINI, section, "IonCannon.Scatter.Max.IncreaseMin");
	this->IonCannon_Scatter_Max_Increase.Read(exINI, section, "IonCannon.Scatter.Max.Increase");
	this->IonCannon_Scatter_Min_IncreaseMax.Read(exINI, section, "IonCannon.Scatter.Min.IncreaseMax");
	this->IonCannon_Scatter_Min_IncreaseMin.Read(exINI, section, "IonCannon.Scatter.Min.IncreaseMin");
	this->IonCannon_Scatter_Min_Increase.Read(exINI, section, "IonCannon.Scatter.Min.Increase");
}

template <typename T>
void IonCannonTypeClass::Serialize(T& Stm)
{
	Stm
		.Process(this->IonCannon_Duration)
		.Process(this->IonCannon_Radius)
		.Process(this->IonCannon_MaxRadius)
		.Process(this->IonCannon_MinRadius)
		.Process(this->IonCannon_RadiusReduce)
		.Process(this->IonCannon_RadiusReduceAcceleration)
		.Process(this->IonCannon_RadiusReduceMax)
		.Process(this->IonCannon_RadiusReduceMin)
		.Process(this->IonCannon_Angle)
		.Process(this->IonCannon_AngleAcceleration)
		.Process(this->IonCannon_AngleMax)
		.Process(this->IonCannon_AngleMin)
		.Process(this->IonCannon_Lines)
		.Process(this->IonCannon_DrawLaser)
		.Process(this->IonCannon_DrawLaserWithFire)
		.Process(this->IonCannon_LaserHeight)
		.Process(this->IonCannon_DrawEBolt)
		.Process(this->IonCannon_DrawEBoltWithFire)
		.Process(this->IonCannon_EleHeight)
		.Process(this->IonCannon_InnerColor)
		.Process(this->IonCannon_OuterColor)
		.Process(this->IonCannon_OuterSpread)
		.Process(this->IonCannon_LaserDuration)
		.Process(this->IonCannon_Thickness)
		.Process(this->IonCannon_FireOnce)
		.Process(this->IonCannon_Rate)
		.Process(this->IonCannon_Weapon)
		.Process(this->IonCannon_ROF)
		.Process(this->IonCannon_Scatter_Max)
		.Process(this->IonCannon_Scatter_Min)
		.Process(this->IonCannon_Scatter_Max_IncreaseMax)
		.Process(this->IonCannon_Scatter_Max_IncreaseMin)
		.Process(this->IonCannon_Scatter_Max_Increase)
		.Process(this->IonCannon_Scatter_Min_IncreaseMax)
		.Process(this->IonCannon_Scatter_Min_IncreaseMin)
		.Process(this->IonCannon_Scatter_Min_Increase)
		;
}

void IonCannonTypeClass::LoadFromStream(PhobosStreamReader& Stm)
{
	this->Serialize(Stm);
}

void IonCannonTypeClass::SaveToStream(PhobosStreamWriter& Stm)
{
	this->Serialize(Stm);
}
