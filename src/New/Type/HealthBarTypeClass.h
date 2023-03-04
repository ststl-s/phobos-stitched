#pragma once

#include <Ext/Rules/Body.h>
#include <Utilities/Enumerable.h>
#include <Utilities/Template.h>
#include <New/Type/DigitalDisplayTypeClass.h>

class HealthBarTypeClass final : public Enumerable<HealthBarTypeClass>
{
public:

	Nullable<Vector2D<int>> GroupID_Offset;

	Valueable<bool> UnitHealthBar;

	Nullable<Vector2D<int>> SelfHealPips_Offset;
	Nullable<Vector2D<int>> SelfHealPips_Frame;

	Nullable<Vector3D<int>> Pips;
	Nullable<int> Pips_Empty;
	Nullable<SHPStruct*> PipsSHP;
	CustomPalette PipsPAL;

	Nullable<int> PipBrd;
	Nullable<SHPStruct*> PipBrdSHP;
	CustomPalette PipBrdPAL;
	Nullable<Vector2D<int>> PipBrdOffset;

	Nullable<int> Length;
	Nullable<Vector2D<int>> DrawOffset;
	Nullable<int> XOffset;
	Nullable<int> YOffset;

	Valueable<bool> PictureHealthBar;
	Nullable<SHPStruct*> PictureSHP;
	CustomPalette PicturePAL;
	Valueable<int> PictureTransparency;

	NullableVector<DigitalDisplayTypeClass*> DigitalDisplayTypes;
	Nullable<bool> DigitalDisplay_Disable;
	Nullable<bool> DigitalDisplay_ShowEnemy;

	HealthBarTypeClass(const char* pTitle = NONE_STR) : Enumerable<HealthBarTypeClass>(pTitle)
		, GroupID_Offset {}

		, UnitHealthBar { false }

		, SelfHealPips_Offset {}
		, SelfHealPips_Frame {}

		, Pips {}
		, Pips_Empty {}
		, PipsSHP { nullptr }
		, PipsPAL {}

		, PipBrd {}
		, PipBrdSHP { nullptr }
		, PipBrdPAL {}
		, PipBrdOffset { { 0,0 } }

		, Length {}
		, DrawOffset {}
		, XOffset {}
		, YOffset {}

		, PictureHealthBar { false }
		, PictureSHP { nullptr }
		, PicturePAL {}
		, PictureTransparency { 0 }

		, DigitalDisplayTypes {}
		, DigitalDisplay_Disable {}
		, DigitalDisplay_ShowEnemy { true }
	{}

	virtual ~HealthBarTypeClass() override = default;

	virtual void LoadFromINI(CCINIClass* pINI) override;
	virtual void LoadFromStream(PhobosStreamReader& Stm);
	virtual void SaveToStream(PhobosStreamWriter& Stm);

private:
	template <typename T>
	void Serialize(T& Stm);
};
