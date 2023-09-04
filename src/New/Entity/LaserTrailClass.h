#pragma once

#include <New/Type/LaserTrailTypeClass.h>

class LaserTrailClass
{
public:
	LaserTrailTypeClass* Type = nullptr;
	bool Visible = false;
	CoordStruct FLH;
	bool IsOnTurret = false;
	ColorStruct CurrentColor;
	Nullable<CoordStruct> LastLocation;
	int FramesPassed = 0;

	LaserTrailClass() = default;

	LaserTrailClass
	(
		LaserTrailTypeClass* pTrailType,
		HouseClass* pHouse = nullptr,
		CoordStruct flh = CoordStruct::Empty,
		bool isOnTurret = false
	);

	bool Update(CoordStruct location);

	bool Load(PhobosStreamReader& stm, bool registerForChange);
	bool Save(PhobosStreamWriter& stm) const;

	ColorStruct GetCurrentColor();
	BYTE GetColorChannel(int cycle, int offset);

private:
	template <typename T>
	bool Serialize(T& stm);
};
