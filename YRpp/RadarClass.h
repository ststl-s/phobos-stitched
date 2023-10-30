#pragma once

#include <DisplayClass.h>

class NOVTABLE RadarClass : public DisplayClass
{
public:
	//WIP: RadarClass::RTacticalClass goes HERE

	//Static
	static constexpr constant_ptr<RadarClass, 0x87F7E8u> const Instance{};

	//Destructor
	virtual ~RadarClass() override RX;// JMP_THIS(0x6587A0);

	//GScreenClass
	virtual void One_Time() override JMP_THIS(0x652CF0);
	virtual void Init_Clear() override JMP_THIS(0x652DE0);
	virtual void Init_IO() override JMP_THIS(0x653010);
	virtual void Update(const int& keyCode, const Point2D& mouseCoords) override JMP_THIS(0x653850);
	virtual void Draw(DWORD dwUnk) override JMP_THIS(0x653100);

	//MapClass
	virtual void CreateEmptyMap(const RectangleStruct& mapRect, bool reuse, char nLevel, bool bUnk2) override JMP_THIS(0x653F50);
	virtual void SetVisibleRect(const RectangleStruct& mapRect) override JMP_THIS(0x654490);

	//DisplayClass
	virtual HRESULT Load(IStream* pStm) override JMP_THIS(0x6568A0);
	virtual HRESULT Save(IStream* pStm) override JMP_THIS(0x656AC0);
	virtual const wchar_t* GetToolTip(UINT nDlgID) override JMP_THIS(0x658770);
	virtual void CloseWindow() override JMP_THIS(0x654320); //prolly wrong naming
	virtual bool MapCell(CellStruct& cell, HouseClass* pHouse) override JMP_THIS(0x653810);
	virtual bool RevealFogShroud(CellStruct& cell, HouseClass* pHouse, bool bIncreaseShroudCounter) override JMP_THIS(0x653830);

	//RadarClass
	virtual void DisposeOfArt() JMP_THIS(0x652D90);
	virtual void* vt_entry_CC(void* out_pUnk, Point2D& pPoint) JMP_THIS(0x653760);
	virtual void vt_entry_D0(DWORD dwUnk) JMP_THIS(0x653F70);
	virtual void Init_For_House() JMP_THIS(0x652E90);

	//Non-virtual

	static bool __fastcall RadarEvent(int kind, DWORD unknown, const CellStruct& cell)
	{ JMP_STD(0x65FA70); }

protected:
	//Constructor
	RadarClass() noexcept	//don't need this
	{ JMP_THIS(0x652960); }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:
	DWORD unknown_11E8;
	DWORD unknown_11EC;
	DWORD unknown_11F0;
	DWORD unknown_11F4;
	DWORD unknown_11F8;
	DWORD unknown_11FC;
	DWORD unknown_1200;
	DWORD unknown_1204;
	DWORD unknown_1208;
	RectangleStruct unknown_rect_120C;
	DWORD unknown_121C;
	DWORD unknown_1220;
	DynamicVectorClass<CellStruct> unknown_cells_1124;
	DWORD unknown_123C;
	DWORD unknown_1240;
	DWORD unknown_1244;
	DWORD unknown_1248;
	DWORD unknown_124C;
	DWORD unknown_1250;
	DWORD unknown_1254;
	DWORD unknown_1258;
	DynamicVectorClass<Point2D> unknown_points_125C;
	DWORD unknown_1274;
	DynamicVectorClass<Point2D> FoundationTypePixels[22];
	float RadarSizeFactor;
	int unknown_int_148C;
	DWORD unknown_1490;
	DWORD unknown_1494;
	DWORD unknown_1498;
	RectangleStruct unknown_rect_149C;
	DWORD unknown_14AC;
	DWORD unknown_14B0;
	DWORD unknown_14B4;
	DWORD unknown_14B8;
	bool unknown_bool_14BC;
	bool unknown_bool_14BD;
	DWORD unknown_14C0;
	DWORD unknown_14C4;
	DWORD unknown_14C8;
	DWORD unknown_14CC;
	DWORD unknown_14D0;
	int unknown_int_14D4;
	bool IsAvailableNow;
	bool unknown_bool_14D9;
	bool unknown_bool_14DA;
	RectangleStruct unknown_rect_14DC;
	DWORD unknown_14EC;
	DWORD unknown_14F0;
	DWORD unknown_14F4;
	DWORD unknown_14F8;
	DWORD unknown_14FC;
	CDTimerClass unknown_timer_1500;
};
