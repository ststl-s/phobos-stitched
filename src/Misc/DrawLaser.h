#pragma once
#include <vector>

#include <ColorScheme.h>
#include <LaserDrawClass.h>
#include <TechnoClass.h>

class DrawLaser
{
private:

	struct ElectricItem
	{
		CoordStruct PosFire;
		CoordStruct PosEnd;
		int Length;
		ColorStruct Color;
		double Amplitude;
		int Duration;
		int Thickness;
		bool IsSupported;
		int DrawTimer;
		int Angle;
	};
	static std::vector<ElectricItem> ElectricData;

	struct TrackingLaser
	{
		LaserDrawClass* Laser;
		int Duration;
		TechnoClass* Owner;
		AbstractClass* Target;
		CoordStruct fireCoord;
		bool OnTurret;
	};
	static std::vector<TrackingLaser> TrackingLaserData;

public:
	static void AddElectric(CoordStruct PosFire, CoordStruct PosEnd, int Angle, int Length, int DrawTimer, ColorStruct Color, double Amplitude, int Duration, int Thickness, bool IsSupported);
	static void DrawElectricLaser(CoordStruct PosFire, CoordStruct PosEnd, int Angle, int Length, ColorStruct Color, double Amplitude, int Duration, int Thickness, bool IsSupported);
	static void UpdateElectricLaser();
	static void AddTrackingLaser(LaserDrawClass* Laser, int Duration, TechnoClass* Owner, AbstractClass* Target, CoordStruct fireCoord, bool OnTurret);
	static void UpdateTrackingLaser();
	static void Clear();
	static void UpdateAll();
};
