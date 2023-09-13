#pragma once
#include <StringTable.h>
#include <CCINIClass.h>
#include <CellSpread.h>

#include <Helpers/Iterators.h>
#include <Helpers/Enumerators.h>

#include <string.h>
#include <string>
#include <iterator>
#include <vector>

#include <Utilities/Template.h>

class GeneralUtils
{
public:

	static bool IsValidString(const char* str);
	static bool IsValidString(const wchar_t* str);
	static void IntValidCheck(int* source, const char* section, const char* tag, int defaultValue, int min = MIN(int), int max = MAX(int));
	static void DoubleValidCheck(double* source, const char* section, const char* tag, double defaultValue, double min = MIN(double), double max = MAX(double));
	static const wchar_t* LoadStringOrDefault(const char* key, const wchar_t* defaultValue);
	static const wchar_t* LoadStringUnlessMissing(const char* key, const wchar_t* defaultValue);
	static std::vector<CellStruct> AdjacentCellsInRange(unsigned int range);
	static const int GetRangedRandomOrSingleValue(PartialVector2D<int> range);
	static const double GetRangedRandomOrSingleValue(PartialVector2D<double> range);
	static const double GetWarheadVersusArmor(WarheadTypeClass* pWH, Armor armor);
	static const bool ProduceBuilding(HouseClass* pOwner, int idxBuilding);
	static int ChooseOneWeighted(const double dice, const std::vector<int>* weights);
	static bool HasHealthRatioThresholdChanged(double oldRatio, double newRatio);
	static bool ApplyTheaterSuffixToString(char* str);
	static std::string IntToDigits(int num);
	static std::vector<BulletClass*> GetCellSpreadBullets(const CoordStruct& crd, double cellSpread);
	static DirType Point2Dir(CoordStruct src, CoordStruct dest);
	static ColorStruct HSV2RGB(int h, int s, int v);
	static void DrawSquare(Point2D center, double range, COLORREF nColor);
	static bool IsOperator(char c);
	static bool OperatorPriorityGreaterThan(char opa, char opb);
	static int CountDigitsInNumber(int number);

	template<typename T>
	static T FastPow(T x, size_t n)
	{
		// Real fast pow calc x^n in O(log(n))
		T result = 1;
		T base = x;
		while (n)
		{
			if (n & 1) result *= base;
			base *= base;
			n >>= 1;
		}
		return result;
	}
};
