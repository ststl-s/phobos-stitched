#pragma once

#include <Utilities/Enumerable.h>
#include <Utilities/Template.h>
#include <Utilities/Enum.h>

class BannerTypeClass final : public Enumerable<BannerTypeClass>
{
public:
	// read from INI
	PhobosFixedString<0x20> Content_PCX;
	Nullable<SHPStruct*> Content_SHP;
	CustomPalette Content_PAL;
	PhobosFixedString<0x32> Content_PAL_Name;
	Valueable<bool> Content_SHP_FadeIn;
	Valueable<bool> Content_SHP_FadeOut;
	Valueable<bool> Content_SHP_Play;
	Valueable<bool> Content_SHP_Loop;
	Valueable<int> Content_SHP_Rate;
	Valueable<int> Content_SHP_Transparency;
	Nullable<int> Content_SHP_Transparency_MouseHover;
	Valueable<bool> Content_SHP_OnlyOne;
	Valueable<bool> Content_SHP_Remove;
	Nullable<int> Content_SHP_Frame;
	Nullable<int> Content_SHP_End;
	NullableIdx<VocClass> Content_SHP_Sound;
	Nullable<Vector2D<int>> Content_SHP_FullScreen;
	Valueable<bool> Content_SHP_DrawInShroud;
	Valueable<CSFText> Content_CSF;
	Nullable<ColorStruct> Content_CSF_Color;
	Nullable<ColorStruct> Content_CSF_ColorShadow;
	Valueable<bool> Content_CSF_DrawBackground;
	Valueable<int> Content_CSF_FadeIn;
	NullableIdx<VocClass> Content_CSF_Sound;
	// internal
	BannerType Type;
	Valueable<int> Align;
	Valueable<int> Anchor;
	Nullable<int> ImageAlign;
	Valueable<bool> Percentage;
	Valueable<Vector2D<int>> Offset;
	Valueable<int> Duration;

	BannerTypeClass(const char* pTitle = NONE_STR) : Enumerable<BannerTypeClass>(pTitle)
		, Content_PCX {}
		, Content_SHP {}
		, Content_PAL {}
		, Content_PAL_Name { "palette.pal" }
		, Content_SHP_FadeIn { false }
		, Content_SHP_FadeOut { false }
		, Content_SHP_Play { false }
		, Content_SHP_Loop { false }
		, Content_SHP_Rate { 1 }
		, Content_SHP_Transparency { 0 }
		, Content_SHP_Transparency_MouseHover {}
		, Content_SHP_OnlyOne { false }
		, Content_SHP_Remove { false }
		, Content_SHP_Frame {}
		, Content_SHP_End {}
		, Content_SHP_Sound {}
		, Content_SHP_FullScreen {}
		, Content_SHP_DrawInShroud { true }
		, Content_CSF {}
		, Content_CSF_Color {}
		, Content_CSF_ColorShadow {}
		, Content_CSF_DrawBackground { false }
		, Content_CSF_FadeIn { 0 }
		, Content_CSF_Sound {}
		, Type { BannerType::CSF }
		, Align { 0 }
		, Anchor { 0 }
		, ImageAlign {}
		, Percentage { true }
		, Offset { {0,0} }
		, Duration { -1 }
	{}

	virtual ~BannerTypeClass() override = default;

	virtual void LoadFromINI(CCINIClass* pINI) override;
	virtual void LoadFromStream(PhobosStreamReader& Stm);
	virtual void SaveToStream(PhobosStreamWriter& Stm);

private:
	template <typename T>
	void Serialize(T& Stm);
};
