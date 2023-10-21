#include "DrawLaser.h"

#include <MapClass.h>
#include <Phobos.CRT.h>
#include <TacticalClass.h>
#include <ColorScheme.h>
#include <Drawing.h>
#include <ScenarioClass.h>
#include <BitFont.h>

#include <Ext/Bullet/Body.h>
#include <Ext/Techno/Body.h>

std::vector<DrawLaser::ElectricItem> DrawLaser::ElectricData;
std::vector<DrawLaser::TrackingLaser> DrawLaser::TrackingLaserData;

void DrawLaser::Clear()
{
	DrawLaser::ElectricData.clear();
	DrawLaser::TrackingLaserData.clear();
}

void DrawLaser::AddElectric(CoordStruct PosFire, CoordStruct PosEnd, int Angle, int Length, int DrawTimer,
	ColorStruct Color, double Amplitude, int Duration, int Thickness, bool IsSupported)
{
	DrawLaser::ElectricItem electric {};
	electric.PosFire = PosFire;
	electric.PosEnd = PosEnd;
	electric.Angle = Angle;
	electric.Length = Length;
	electric.DrawTimer = DrawTimer;
	electric.Color = Color;
	electric.Amplitude = Amplitude;
	electric.Duration = Duration;
	electric.Thickness = Thickness;
	electric.IsSupported = IsSupported;

	DrawLaser::ElectricData.push_back(electric);
}

void DrawLaser::DrawElectricLaser(CoordStruct PosFire, CoordStruct PosEnd, int Angle, int Length,
	ColorStruct Color, double Amplitude, int Duration, int Thickness, bool IsSupported)
{
	int length = Length * 3;
	double Xvalue = (PosEnd.X - PosFire.X) / static_cast<double>(length);
	double Yvalue = (PosEnd.Y - PosFire.Y) / static_cast<double>(length);
	double Zvalue = (PosEnd.Z - PosFire.Z) / static_cast<double>(length);

	CoordStruct coords = PosFire;
	CoordStruct lastcoords;

	auto displace = int(PosFire.DistanceFrom(PosEnd) / Amplitude);

	auto thin = int(displace / 2);
	if (thin < 4)
		thin = 4;

	auto Xthin = thin;
	if (Angle == 12 || Angle == 28)
	{
		Xthin = int(Xthin * 0.2);
		if (Xthin < 4)
			Xthin = 4;
	}

	for (int i = 1; i <= length; i++)
	{
		lastcoords = coords;
		coords.X += static_cast<int>(Xvalue);
		coords.Y += static_cast<int>(Yvalue);
		coords.Z += static_cast<int>(Zvalue);

		if (i <= length - 6)
		{
			coords.X += ScenarioClass::Instance->Random(-Xthin, Xthin);
			coords.Y += ScenarioClass::Instance->Random(-thin, thin);
		}
		else if (i <= length - 3)
		{
			int thin2 = int(Xthin / 2);
			int thin3 = int(thin / 2);

			coords.X += ScenarioClass::Instance->Random(-thin2, thin2);
			coords.Y += ScenarioClass::Instance->Random(-thin3, thin3);
		}
		else
			coords = PosEnd;

		LaserDrawClass* pLaser = GameCreate<LaserDrawClass>(
				lastcoords, coords,
				Color, ColorStruct { 0,0,0 }, ColorStruct { 0,0,0 },
				Duration);

		pLaser->IsHouseColor = true;
		pLaser->Thickness = Thickness;
		pLaser->IsSupported = IsSupported;
	}
}

void DrawLaser::UpdateElectricLaser()
{
	if (DrawLaser::ElectricData.empty())
		return;

	for (int i = DrawLaser::ElectricData.size() - 1; i >= 0; --i)
	{
		auto& data = DrawLaser::ElectricData[i];

		DrawLaser::DrawElectricLaser(data.PosFire, data.PosEnd, data.Angle,
			data.Length,
			data.Color,
			data.Amplitude,
			data.Duration,
			data.Thickness,
			data.IsSupported);

		data.DrawTimer--;

		if (data.DrawTimer <= 0)
			DrawLaser::ElectricData.erase(DrawLaser::ElectricData.begin() + i);
	}
}

void DrawLaser::AddTrackingLaser(LaserDrawClass* Laser, int Duration, TechnoClass* Owner, AbstractClass* Target, CoordStruct fireCoord, bool OnTurret)
{
	if (!TechnoExt::IsActive(Owner) ||
		Duration <= 0 ||
		!Laser ||
		!Target ||
		Target->WhatAmI() == AbstractType::Building)
		return;

	DrawLaser::TrackingLaser laser {};
	laser.Laser = Laser;
	laser.Duration = Duration;
	laser.Owner = Owner;
	laser.Target = Target;
	laser.fireCoord = fireCoord;
	laser.OnTurret = OnTurret;

	DrawLaser::TrackingLaserData.push_back(laser);
}

void DrawLaser::UpdateTrackingLaser()
{
	if (DrawLaser::TrackingLaserData.empty())
		return;

	for (int i = DrawLaser::TrackingLaserData.size() - 1; i >= 0; --i)
	{
		auto& laser = DrawLaser::TrackingLaserData[i];

		if (!TechnoExt::IsActive(laser.Owner) ||
			!laser.Laser ||
			laser.Duration <= 0 ||
			!laser.Target ||
			laser.Owner->Target != laser.Target)
		{
			DrawLaser::TrackingLaserData.erase(DrawLaser::TrackingLaserData.begin() + i);
			continue;
		}

		if (laser.fireCoord != CoordStruct::Empty)
			laser.Laser->Source = TechnoExt::GetFLHAbsoluteCoords(laser.Owner, laser.fireCoord, laser.OnTurret);

		if (laser.Laser->Target != laser.Target->GetCoords())
			laser.Laser->Target = laser.Target->GetCoords();

		laser.Duration--;
	}
}

void DrawLaser::UpdateAll()
{
	UpdateElectricLaser();
	UpdateTrackingLaser();
}
