/*FlyingStrings.h
Useable to get out messages from units
Used to output Bounty messages
By AlexB and Joshy
*/

#pragma once
#include <vector>
#include <ColorScheme.h>
#include <HouseClass.h>
#include <Utilities/Enum.h>

class FlyingStrings
{
private:

	struct Item
	{
		CoordStruct Location;
		Point2D PixelOffset;
		int CreationFrame;
		wchar_t Text[0x20];
		COLORREF Color;

	};

	struct pElectric
	{
		CoordStruct PosFire;
		CoordStruct PosEnd;
		int Length;
		ColorStruct Color;
		double Amplitude;
		int Duration;
		int Thickness;
		bool IsSupported;
		int Frame;
	};

	static const int Duration = 75;
	static std::vector<Item> Data;

	static const int ElectricDuration;
	static std::vector<pElectric> ElectricData;

	static bool DrawAllowed(CoordStruct& nCoords);

public:
	static void Add(const wchar_t* text, CoordStruct coords, ColorStruct color, Point2D pixelOffset = Point2D::Empty);
	static void AddMoneyString(int amount, HouseClass* owner, AffectedHouse displayToHouses, CoordStruct coords, Point2D pixelOffset = Point2D::Empty);
	static void GetElectric(CoordStruct PosFire, CoordStruct PosEnd, int Length, int timer, ColorStruct Color, double Amplitude, int Duration, int Thickness, bool IsSupported);
	static void UpdateAll();
	static void UpdateAllElectric();
};
