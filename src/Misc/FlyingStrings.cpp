#include "FlyingStrings.h"

#include <MapClass.h>
#include <Phobos.CRT.h>
#include <TacticalClass.h>
#include <ColorScheme.h>
#include <Drawing.h>
#include <ScenarioClass.h>
#include <BitFont.h>
#include <Utilities/EnumFunctions.h>
#include <Ext/Bullet/Body.h>

std::vector<FlyingStrings::Item> FlyingStrings::Data;
std::vector<FlyingStrings::pElectric> FlyingStrings::ElectricData;

bool FlyingStrings::DrawAllowed(CoordStruct& nCoords)
{
	if (auto const pCell = MapClass::Instance->TryGetCellAt(nCoords))
	{
		if (pCell->IsFogged() || pCell->IsShrouded())
			return false;

		return true;
	}

	return false;
}

void FlyingStrings::Add(const wchar_t* text, CoordStruct coords, ColorStruct color, Point2D pixelOffset)
{
	Item item {};
	item.Location = coords;
	item.PixelOffset = pixelOffset;
	item.CreationFrame = Unsorted::CurrentFrame;
	item.Color = Drawing::RGB_To_Int(color);
	PhobosCRT::wstrCopy(item.Text, text, 0x20);
	Data.push_back(item);
}

void FlyingStrings::GetElectric(CoordStruct PosFire, CoordStruct PosEnd, int Length, ColorStruct Color, float Amplitude, int Duration, int Thickness, bool IsSupported)
{
	pElectric item {};
	item.PosFire = PosFire;
	item.PosEnd = PosEnd;
	item.Length = Length;
	item.Color = Color;
	item.Amplitude = Amplitude;
	item.Duration = Duration;
	item.Thickness = Thickness;
	item.IsSupported = IsSupported;
	item.Frame = 15;
	ElectricData.push_back(item);
}

void FlyingStrings::AddMoneyString(int amount, HouseClass* owner, AffectedHouse displayToHouses, CoordStruct coords, Point2D pixelOffset)
{
	if (displayToHouses == AffectedHouse::All ||
		owner && EnumFunctions::CanTargetHouse(displayToHouses, owner, HouseClass::CurrentPlayer))
	{
		bool isPositive = amount > 0;
		ColorStruct color = isPositive ? ColorStruct { 0, 255, 0 } : ColorStruct { 255, 0, 0 };
		wchar_t moneyStr[0x20];
		swprintf_s(moneyStr, L"%ls%ls%d", isPositive ? L"+" : L"-", Phobos::UI::CostLabel, std::abs(amount));

		int width = 0, height = 0;
		BitFont::Instance->GetTextDimension(moneyStr, &width, &height, 120);
		pixelOffset.X -= (width / 2);

		FlyingStrings::Add(moneyStr, coords, color, pixelOffset);
	}
}

void FlyingStrings::UpdateAll()
{
	if (Data.empty())
		return;

	for (int i = Data.size() - 1; i >= 0; --i)
	{
		auto& dataItem = Data[i];

		Point2D point;

		TacticalClass::Instance->CoordsToClient(dataItem.Location, &point);

		point += dataItem.PixelOffset;

		RectangleStruct bound = DSurface::Temp->GetRect();
		bound.Height -= 32;

		if (Unsorted::CurrentFrame > dataItem.CreationFrame + Duration - 70)
		{
			point.Y -= (Unsorted::CurrentFrame - dataItem.CreationFrame);
			DSurface::Temp->DrawText(dataItem.Text, &bound, &point, dataItem.Color, 0, TextPrintType::NoShadow);
		}
		else
		{
			DSurface::Temp->DrawText(dataItem.Text, &bound, &point, dataItem.Color, 0, TextPrintType::NoShadow);
		}

		if (Unsorted::CurrentFrame > dataItem.CreationFrame + Duration || Unsorted::CurrentFrame < dataItem.CreationFrame)
			Data.erase(Data.begin() + i);
	}

	for (int i = ElectricData.size() - 1; i >= 0; --i)
	{
		auto& dataItem = ElectricData[i];

		BulletExt::DrawElectricLaser(dataItem.PosFire, dataItem.PosEnd, dataItem.Length,
			dataItem.Color,
			dataItem.Amplitude,
			dataItem.Duration,
			dataItem.Thickness,
			dataItem.IsSupported);

		dataItem.Frame--;

		if (dataItem.Frame <= 0)
			ElectricData.erase(ElectricData.begin() + i);
	}
}
