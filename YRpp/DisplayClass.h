#pragma once

#include <MapClass.h>

class CCINIClass;
class ObjectTypeClass;

class NOVTABLE DisplayClass : public MapClass
{
public:
	//Static
	static constexpr constant_ptr<DisplayClass, 0x87F7E8u> const Instance{};

	//WIP: DisplayClass::TacticalClass goes HERE

	bool ProcessClickCoords(Point2D *src, CellStruct *XYdst, CoordStruct *XYZdst, ObjectClass **Target, BYTE *a5, BYTE *a6)
	{ JMP_THIS(0x692300); }

	// the foundation for placement with green/red
	void  SetActiveFoundation(const CellStruct& cell)
	{ JMP_THIS(0x4A8BF0); }

	//Destructor
	virtual ~DisplayClass() override JMP_THIS(0x4AEBF0);
	
	//GScreenClass
	virtual void One_Time() override JMP_THIS(0x4A8850);
	virtual void Init_Clear() override JMP_THIS(0x4A88C0);
	virtual void Init_IO() override JMP_THIS(0x4A8930);
	virtual void Update(const int& keyCode, const Point2D& mouseCoords) override JMP_THIS(0x4A9700);

	//MapClass

	//DisplayClass
	virtual HRESULT Load(IStream* pStm) JMP_THIS(0x4AE6F0);
	virtual HRESULT Save(IStream* pStm) JMP_THIS(0x4AE720);
	virtual void LoadFromINI(CCINIClass* pINI) JMP_THIS(0x4ACE70); //Loads the map from a map file.
	virtual const wchar_t* GetToolTip(UINT nDlgID) JMP_THIS(0x4AE4F0);
	virtual void CloseWindow() JMP_THIS(0x4AE6B0); //prolly wrong naming
	virtual void vt_entry_8C() JMP_THIS(0x4AEAD0);
	virtual bool MapCell(CellStruct& cell, HouseClass* pHouse) JMP_THIS(0x4A9890);
	virtual bool RevealFogShroud(CellStruct& cell, HouseClass* pHouse, bool bIncreaseShroudCounter) JMP_THIS(0x4A9CA0);
	virtual bool MapCellFoggedness(CellStruct& cell, HouseClass* pHouse) JMP_THIS(0x4A9DD0);
	virtual bool MapCellVisibility(CellStruct& cell, HouseClass* pHouse) JMP_THIS(0x4AA050);
	virtual MouseCursorType GetLastMouseCursor() = 0;
	virtual bool ScrollMap(DWORD dwUnk1, DWORD dwUnk2, DWORD dwUnk3) JMP_THIS(0x4A9840);
	virtual void Set_View_Dimensions(const RectangleStruct& rect) JMP_THIS(0x4A8960);
	virtual void vt_entry_AC(DWORD dwUnk) { }
	virtual void vt_entry_B0(DWORD dwUnk) { }
	virtual void vt_entry_B4(Point2D* pPoint) JMP_THIS(0x4AC310);

	//Decides which mouse pointer to set and then does it.
	//Mouse is over cell pMapCoords which is bShrouded and holds pObject.
	virtual bool ConvertAction(const CellStruct& cell, bool bShrouded, ObjectClass* pObject, Action action, bool dwUnk) JMP_THIS(0x4AAE90);
	virtual void LeftMouseButtonDown(const Point2D& point) JMP_THIS(0x4AC380);
	virtual void LeftMouseButtonUp(const CoordStruct& coords, const CellStruct& cell, ObjectClass* pObject, Action action, DWORD dwUnk2) JMP_THIS(0x4AB9B0);
	virtual void RightMouseButtonUp(DWORD dwUnk) JMP_THIS(0x4AAD30);

	//Non-virtual

	Action DecideAction(const CellStruct& cell, ObjectClass* pObject, DWORD dwUnk)
	{ JMP_THIS(0x692610); }

	/* pass in CurrentFoundationData and receive the width/height of a bounding rectangle in cells */
	CellStruct* FoundationBoundsSize(CellStruct& buffer, CellStruct const* const pFoundationData) const
	{ JMP_THIS(0x4A94F0); }

	CellStruct FoundationBoundsSize(CellStruct const* const pFoundationData) const
	{
		CellStruct buffer;
		FoundationBoundsSize(buffer, pFoundationData);
		return buffer;
	}

	/* marks or unmarks the cells pointed to by CurrentFoundationData as containing a building */
	void MarkFoundation(CellStruct& baseCell, bool Mark)
		{ JMP_THIS(0x4A95A0); }

protected:
	//Constructor
	DisplayClass() noexcept
	{ JMP_THIS(0x4A8730) }//don't need this

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:
	CellStruct CurrentFoundation_CenterCell;	//Currently placing the building here
	CellStruct CurrentFoundation_TopLeftOffset;		// offset from center cell of the current foundation (under the mouse) to the top left cell
	CellStruct* CurrentFoundation_Data;	//Foundation data of the building we're currently placing (note: limited to 120 cells)
	bool unknown_1180;
	bool unknown_1181;
	CellStruct CurrentFoundationCopy_CenterCell; // this is a copy of the CurrentFoundation data above..
	CellStruct CurrentFoundationCopy_TopLeftOffset;
	CellStruct * CurrentFoundationCopy_Data; // (note: limited to 50 [!] cells)
	DWORD unknown_1190;
	DWORD unknown_1194;
	DWORD unknown_1198;
	bool FollowObject;
	ObjectClass* ObjectToFollow;
	ObjectClass* CurrentBuilding;		//Building we're currently placing
	ObjectTypeClass* CurrentBuildingType;	//Type of that building
	DWORD unknown_11AC;
	bool RepairMode;
	bool SellMode;
	bool PowerToggleMode;
	bool PlanningMode;
	bool PlaceBeaconMode;
	int CurrentSWTypeIndex;	//Index of the SuperWeaponType we have currently selected
	DWORD unknown_11BC;
	DWORD unknown_11C0;
	DWORD unknown_11C4;
	DWORD unknown_11C8;
	bool unknown_bool_11CC;
	bool unknown_bool_11CD;
	bool unknown_bool_11CE;
	bool DraggingRectangle;
	bool unknown_bool_11D0;
	bool unknown_bool_11D1;
	DWORD unknown_11D4;
	DWORD unknown_11D8;
	DWORD unknown_11DC;
	DWORD unknown_11E0;
	PROTECTED_PROPERTY(DWORD, padding_11E4);
};
