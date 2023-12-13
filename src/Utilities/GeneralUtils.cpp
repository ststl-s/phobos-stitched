#include "GeneralUtils.h"
#include "Debug.h"

#include <stack>

#include <BuildingTypeClass.h>
#include <BulletClass.h>
#include <EventClass.h>
#include <HouseClass.h>
#include <ScenarioClass.h>
#include <Theater.h>
#include <VocClass.h>
#include <Unsorted.h>

bool GeneralUtils::IsValidString(const char* str)
{
	return str != nullptr && strlen(str) != 0 && !INIClass::IsBlank(str);
}

bool GeneralUtils::IsValidString(const wchar_t* str)
{
	return str != nullptr && wcslen(str) != 0 && !wcsstr(str, L"MISSING:");
}

void GeneralUtils::IntValidCheck(int* source, const char* section, const char* tag, int defaultValue, int min, int max)
{
	if (*source < min || *source > max)
	{
		Debug::Log("[Developer warning][%s]%s=%d is invalid! Reset to %d.\n", section, tag, *source, defaultValue);
		*source = defaultValue;
	}
}

void GeneralUtils::DoubleValidCheck(double* source, const char* section, const char* tag, double defaultValue, double min, double max)
{
	if (*source < min || *source > max)
	{
		Debug::Log("[Developer warning][%s]%s=%f is invalid! Reset to %f.\n", section, tag, *source, defaultValue);
		*source = defaultValue;
	}
}

const wchar_t* GeneralUtils::LoadStringOrDefault(const char* key, const wchar_t* defaultValue)
{
	if (GeneralUtils::IsValidString(key))
		return StringTable::LoadString(key);
	else
		return defaultValue;
}

const wchar_t* GeneralUtils::LoadStringUnlessMissing(const char* key, const wchar_t* defaultValue)
{
	return wcsstr(LoadStringOrDefault(key, defaultValue), L"MISSING:") ? defaultValue : LoadStringOrDefault(key, defaultValue);
}

std::vector<CellStruct> GeneralUtils::AdjacentCellsInRange(unsigned int range)
{
	std::vector<CellStruct> result;

	for (CellSpreadEnumerator it(range); it; ++it)
		result.push_back(*it);

	return result;
}

const int GeneralUtils::GetRangedRandomOrSingleValue(PartialVector2D<int> range)
{
	return range.X >= range.Y || range.ValueCount < 2 ? range.X : ScenarioClass::Instance->Random.RandomRanged(range.X, range.Y);
}

const double GeneralUtils::GetRangedRandomOrSingleValue(PartialVector2D<double> range)
{
	int min = static_cast<int>(range.X * 100);
	int max = static_cast<int>(range.Y * 100);

	return range.X >= range.Y || range.ValueCount < 2 ? range.X : (ScenarioClass::Instance->Random.RandomRanged(min, max) / 100.0);
}

const double GeneralUtils::GetWarheadVersusArmor(WarheadTypeClass* pWH, Armor armor)
{
	return double(MapClass::GetTotalDamage(100, pWH, armor, 0)) / 100.0;
}

const bool GeneralUtils::ProduceBuilding(HouseClass* pOwner, int idxBuilding)
{
	if (auto pItem = ObjectTypeClass::GetTechnoType(AbstractType::BuildingType, idxBuilding))
	{
		if (pOwner->CanBuild(pItem, true, true) == CanBuildResult::Buildable)
		{
			if (pItem->FindFactory(true, true, true, pOwner))
			{
				auto pBuilding = abstract_cast<BuildingTypeClass*>(pItem);
				if (pOwner->GetPrimaryFactory(AbstractType::Building, false, pBuilding->BuildCat))
					return false;

				VocClass::PlayGlobal(RulesClass::Instance->GUIBuildSound, 0x2000, 1.0);

				EventClass eEvent(pOwner->ArrayIndex, EventType::PRODUCE, static_cast<int>(pItem->WhatAmI()), pItem->GetArrayIndex(), pItem->Naval);
				EventClass::AddEvent(eEvent);

				return true;
			}
		}
	}

	return false;
}

// Weighted random element choice (weight) - roll for one.
// Takes a vector of integer type weights, which are then summed to calculate the chances.
// Returns chosen index or -1 if nothing is chosen.
int GeneralUtils::ChooseOneWeighted(const double dice, const std::vector<int>* weights)
{
	float sum = 0.0;
	float sum2 = 0.0;

	for (size_t i = 0; i < weights->size(); i++)
		sum += (*weights)[i];

	for (size_t i = 0; i < weights->size(); i++)
	{
		sum2 += (*weights)[i];
		if (dice < (sum2 / sum))
			return i;
	}

	return -1;
}

// Checks if health ratio has changed threshold (Healthy/ConditionYellow/Red).
bool GeneralUtils::HasHealthRatioThresholdChanged(double oldRatio, double newRatio)
{
	if (oldRatio == newRatio)
		return false;

	if (oldRatio > RulesClass::Instance->ConditionYellow && newRatio <= RulesClass::Instance->ConditionYellow)
	{
		return true;
	}
	else if (oldRatio <= RulesClass::Instance->ConditionYellow && oldRatio > RulesClass::Instance->ConditionRed &&
		(newRatio <= RulesClass::Instance->ConditionRed || newRatio > RulesClass::Instance->ConditionYellow))
	{
		return true;
	}
	else if (oldRatio <= RulesClass::Instance->ConditionRed && newRatio > RulesClass::Instance->ConditionRed)
	{
		return true;
	}

	return false;
}

bool GeneralUtils::ApplyTheaterSuffixToString(char* str)
{
	if (auto pSuffix = strstr(str, "~~~"))
	{
		auto theater = ScenarioClass::Instance->Theater;
		auto pExtension = Theater::GetTheater(theater).Extension;
		pSuffix[0] = pExtension[0];
		pSuffix[1] = pExtension[1];
		pSuffix[2] = pExtension[2];
		return true;
	}

	return false;
}

std::string GeneralUtils::IntToDigits(int num)
{
	std::string sDigits;

	if (num == 0)
	{
		sDigits.push_back('0');
		return sDigits;
	}

	while (num)
	{
		sDigits.push_back(static_cast<char>(num % 10) + '0');
		num /= 10;
	}

	std::reverse(sDigits.begin(), sDigits.end());

	return sDigits;
}

std::vector<BulletClass*> GeneralUtils::GetCellSpreadBullets(const CoordStruct& crd, double cellSpread)
{
	std::vector<BulletClass*> result;
	for (BulletClass* pBullet : *BulletClass::Array)
	{
		double distance = pBullet->GetCoords().DistanceFrom(crd);

		if (distance <= cellSpread)
			result.emplace_back(pBullet);
	}

	return result;
}

// DP-Kratos
DirType GeneralUtils::Point2Dir(CoordStruct src, CoordStruct dest)
{
	// get angle
	double radians = Math::atan2(src.Y - dest.Y, dest.X - src.X);
	// Magic form tomsons26
	radians -= Math::deg2rad(90);
	const double BINARY_ANGLE_MAGIC = -(360.0 / 256) * (Math::Pi / 180);
	short d = (short)(radians / BINARY_ANGLE_MAGIC);
	return static_cast<DirType>(d);
}

ColorStruct GeneralUtils::HSV2RGB(int h, int s, int v)
{
	float R = 0, G = 0, B = 0;
	float C = 0, X = 0, Y = 0, Z = 0;
	float H = (float)(h);
	float S = (float)(s) / 100.0f;
	float V = (float)(v) / 100.0f;
	if (S == 0)
		R = G = B = V;
	else
	{
		H = H / 60;
		int i = (int)H;
		C = H - i;

		X = V * (1 - S);
		Y = V * (1 - S * C);
		Z = V * (1 - S * (1 - C));

		switch (i)
		{
		case 0: R = V; G = Z; B = X; break;
		case 1: R = Y; G = V; B = X; break;
		case 2: R = X; G = V; B = Z; break;
		case 3: R = X; G = Y; B = V; break;
		case 4: R = Z; G = X; B = V; break;
		case 5: R = V; G = X; B = Y; break;
		}
	}

	return ColorStruct((BYTE)(R * 255), (BYTE)(G * 255), (BYTE)(B * 255));
}

void GeneralUtils::DrawSquare(Point2D center, double range, COLORREF nColor)
{
	int semiWidth = static_cast<int>(range * Unsorted::CellWidthInPixels);
	int semiHeight = static_cast<int>(range * Unsorted::CellHeightInPixels);

	Point2D points[4] = {
		center + Point2D{ semiWidth, 0 },
		center + Point2D{ 0, semiHeight },
		center + Point2D{ -semiWidth, 0 },
		center + Point2D{ 0, -semiHeight }
	};

	for (int i = 0; i < 4; i++)
	{
		DSurface::Composite->DrawLineEx(&DSurface::ViewBounds, &points[i], &points[(i + 1) % 4], nColor);
	}
}

bool GeneralUtils::IsOperator(char c)
{
	return c == '+' || c == '-' || c == '*' || c == '/' || c == '(' || c == ')';
}

bool GeneralUtils::OperatorPriorityGreaterThan(char opa, char opb)
{
	if (opb == '(' || opb == ')')
		return false;

	if (opa == '(' || opa == ')')
		return true;

	if (opb == '*' || opb == '/' || opb == '%')
		return false;

	if (opa == '*' || opa == '/' || opa == '%')
		return true;

	return false;
}

int GeneralUtils::CountDigitsInNumber(int number)
{
	int digits = 0;

	while (number)
	{
		number /= 10;
		digits++;
	}

	return digits;
}
