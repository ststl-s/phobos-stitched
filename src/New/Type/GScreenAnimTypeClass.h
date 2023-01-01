#pragma once
#include <Utilities/Enumerable.h>
#include <Utilities/Template.h>

class GScreenAnimTypeClass final : public Enumerable<GScreenAnimTypeClass>
{
public:
	PhobosFixedString<0x20> ShowAnim_SHP;
	PhobosFixedString<0x20> ShowAnim_PAL;
	SHPStruct* SHP_ShowAnim;
	ConvertClass* PAL_ShowAnim;
	Valueable<Vector2D<int>> ShowAnim_Offset;
	Valueable<int> ShowAnim_TranslucentLevel;
	Valueable<int> ShowAnim_FrameKeep;
	Valueable<int> ShowAnim_LoopCount;
	Valueable<int> ShowAnim_CoolDown;
	Valueable<bool> ShowAnim_IsOnCursor;

	GScreenAnimTypeClass(const char* pTitle = NONE_STR) : Enumerable<GScreenAnimTypeClass>(pTitle)
		, ShowAnim_SHP { "pips.shp" }
		, ShowAnim_PAL { "" }
		, SHP_ShowAnim { nullptr }
		, PAL_ShowAnim { nullptr }
		, ShowAnim_Offset { { 0, 0 } }
		, ShowAnim_TranslucentLevel { 0 }
		, ShowAnim_FrameKeep { 5 }
		, ShowAnim_LoopCount { 1 }
		, ShowAnim_CoolDown { 100 }
		, ShowAnim_IsOnCursor { false }
	{ }

	virtual ~GScreenAnimTypeClass() override = default;

	virtual void LoadFromINI(CCINIClass* pINI) override;
	virtual void LoadFromStream(PhobosStreamReader& Stm);
	virtual void SaveToStream(PhobosStreamWriter& Stm);

private:
	template <typename T>
	void Serialize(T& Stm);
};
