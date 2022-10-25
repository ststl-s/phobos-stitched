#pragma once

#include <GeneralDefinitions.h>
#include <GeneralStructures.h>
#include <AbstractClass.h>
#include <ColorScheme.h>
#include <Helpers/CompileTime.h>

struct TacticalSelectableStruct
{
	TechnoClass* Techno;
	int X;
	int Y;
};

class DSurface;
class CellClass;

class NOVTABLE TacticalClass : public AbstractClass
{
public:
	static constexpr reference<TacticalClass*, 0x887324u> const Instance{};

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override JMP_STD(0x6DBCE0);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x6DBD20);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override JMP_STD(0x6DBE00);

	//Destructor
	virtual ~TacticalClass() override JMP_THIS(0x6DC470);

	//AbstractClass
	void PointerExpired(AbstractClass* pAbstract, bool removed) override JMP_THIS(0x6DA560);
	AbstractType WhatAmI() const override { return AbstractType::TacticalMap; }
	int Size() const override { return 0xE18; }
	virtual void Update() override JMP_THIS(0x6D2540);
	virtual bool vt_entry_60(DWORD a2, int* a3, int a4, int val) JMP_THIS(0x6DBB60);

	//non-virtual

	void SetTacticalPosition(CoordStruct* pCoord)
	{ JMP_THIS(0x6D6070); }

	CellStruct* CoordsToCell(CellStruct* buffer, CoordStruct* pSource)
	{ JMP_THIS(0x6D6590); }

	// returns whether coords are visible at the moment
	bool CoordsToClient(CoordStruct const& coords, Point2D* buffer) const
	{ JMP_THIS(0x6D2140); }

	Point2D* CoordsToScreen(Point2D* buffer, CoordStruct* pSource)
	{ JMP_THIS(0x6D1F10); }

	CoordStruct* ClientToCoords(CoordStruct* buffer, Point2D const& client) const
	{ JMP_THIS(0x6D2280); }

	CoordStruct ClientToCoords(Point2D const& client) const
	{
		CoordStruct buffer;
		this->ClientToCoords(&buffer, client);
		return buffer;
	}

	char GetOcclusion(const CellStruct& cell, bool fog) const
	{ JMP_THIS(0x6D8700); }

	Point2D * AdjustForZShapeMove(Point2D* buffer, Point2D* pClient)
	{ JMP_THIS(0x6D1FE0); }

	// convert xyz height to xy height?
	static int __fastcall AdjustForZ(int height)
	{ JMP_STD(0x6D20E0); }

	void FocusOn(CoordStruct* pDest, int velocity)
	{ JMP_THIS(0x6D2420); }

	void AddSelectable(TechnoClass* pTechno, int x, int y)
	{ JMP_THIS(0x6D9EF0) };

	// called when area needs to be marked for redrawing due to external factors
	// - alpha lights, terrain changes like cliff destruction, etc
	void RegisterDirtyArea(RectangleStruct rArea, bool bUnk)
	{ JMP_THIS(0x6D2790); }

	void RegisterCellAsVisible(CellClass* pCell)
	{ JMP_THIS(0x6DA7D0) };

	//???
	static int DrawTimer(int index, ColorScheme *pScheme, int time, wchar_t *pText, Point2D *someXY1, Point2D *someXY2)
	{ JMP_STD(0x64DB50); }

	/*
	*   TacticalRenderMode_0_ALL = 0x0,
	*	TacticalRenderMode_TERRAIN = 0x1,
	*	TacticalRenderMode_MOVING_ANIMATING = 0x2,
	*	TacticalRenderMode_3_ALL = 0x3,
	*	TacticalRenderMode_STOPDRAWING = 0x4,
	*	TacticalRenderMode_5 = 0x5,
	*/
	void Render(DSurface* pSurface, bool flag, int eMode)
	{ JMP_THIS(0x6D3D10); }

	//Constructor
	TacticalClass() noexcept
		: TacticalClass(noinit_t())
	{ JMP_THIS(0x6D1C20); }

protected:

	explicit __forceinline TacticalClass(noinit_t) noexcept
		: AbstractClass(noinit_t())
	{ }

public:

	wchar_t ScreenText[64];
	int EndGameGraphicsFrame;
	int LastAIFrame;
	bool field_AC;
	bool field_AD;
	PROTECTED_PROPERTY(char, gap_AE[2]);
	Point2D TacticalPos;
	Point2D LastTacticalPos;
	double ZoomInFactor;
	Point2D Point_C8;
	Point2D Point_D0;
	float field_D8;
	float field_DC;
	int VisibleCellCount;
	CellClass * VisibleCells [800];
	Point2D TacticalCoord1;
	DWORD field_D6C;
	DWORD field_D70;
	Point2D TacticalCoord2;
	bool field_D7C;
	bool Redrawing; // set while redrawing - cheap mutex // TacticalPosUpdated
	PROTECTED_PROPERTY(char, gap_D7E[2]);
	RectangleStruct ContainingMapCoords;
	LTRBStruct Band;
	DWORD MouseFrameIndex;
	CDTimerClass StartTime;
	int SelectableCount;
	Matrix3D Unused_Matrix3D;
	Matrix3D IsoTransformMatrix;
	DWORD field_E14;
};
