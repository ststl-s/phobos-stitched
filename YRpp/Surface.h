#pragma once

#include <GeneralDefinitions.h>
#include <YRDDraw.h>
#include <YRAllocator.h>

#include <Helpers/CompileTime.h>

class ConvertClass;
struct SHPStruct;

class NOVTABLE Surface
{
public:
	Surface() = default;

	virtual ~Surface() JMP_THIS(0x4115D0);

	//Surface
	virtual bool CopyFromWhole(Surface* pSrc, bool bUnk1, bool bUnk2) = 0;

	virtual bool CopyFromPart(
		RectangleStruct* pClipRect, //ignored and retrieved again...
		Surface* pSrc,
		RectangleStruct* pSrcRect,	//desired source rect of pSrc ?
		DWORD dwUnk,
		bool bUnk) = 0;

	virtual bool CopyFrom(
		RectangleStruct* pClipRect,
		RectangleStruct* pClipRect2,	//again? hmm
		Surface* pSrc,
		RectangleStruct* pDestRect,	//desired dest rect of pSrc ? (stretched? clipped?)
		RectangleStruct* pSrcRect,	//desired source rect of pSrc ?
		DWORD dwUnk,
		bool bUnk) = 0;

	virtual bool FillRectEx(
		RectangleStruct* pClipRect,
		RectangleStruct* pFillRect,
		COLORREF color) = 0;

	virtual bool FillRect(RectangleStruct* pFillRect, COLORREF color) = 0;

	virtual bool Fill(COLORREF color) = 0;

	virtual bool FillRectTrans(
		RectangleStruct* pClipRect,
		ColorStruct& color,
		int opacity) = 0;

	virtual bool DrawEllipse(
		Point2D offset,
		Point2D center,
		RectangleStruct rect,
		COLORREF color) = 0;

	virtual bool SetPixel(Point2D* pPoint, COLORREF color) = 0;

	virtual COLORREF GetPixel(Point2D* pPoint) = 0;

	virtual bool DrawLineEx(
		RectangleStruct* pClipRect,
		Point2D* pStart,
		Point2D* pEnd,
		COLORREF color) = 0;

	virtual bool DrawLine(Point2D* pStart, Point2D* pEnd, COLORREF color) = 0;

	virtual bool DrawLineColor_AZ(
		RectangleStruct* pRect,
		Point2D* pStart,
		Point2D* pEnd,
		COLORREF color,
		DWORD dwUnk1,
		DWORD dwUnk2,
		bool bUnk) = 0;

	virtual bool DrawMultiplyingLine_AZ(
		RectangleStruct* pRect,
		Point2D* pStart,
		Point2D* pEnd,
		DWORD dwMultiplier,
		DWORD dwUnk1,
		DWORD dwUnk2,
		bool bUnk) = 0;

	virtual bool DrawSubtractiveLine_AZ(
		RectangleStruct* pRect,
		Point2D* pStart,
		Point2D* pEnd,
		ColorStruct* pColor,
		DWORD dwUnk1,
		DWORD dwUnk2,
		bool bUnk1,
		bool bUnk2,
		bool bUkn3,
		bool bUkn4,
		float fUkn) = 0;

	virtual bool DrawRGBMultiplyingLine_AZ(
		RectangleStruct* pRect,
		Point2D* pStart,
		Point2D* pEnd,
		ColorStruct* pColor,
		float intensity,
		DWORD dwUnk1,
		DWORD dwUnk2) = 0;

	virtual bool PlotLine(
		RectangleStruct* pRect,
		Point2D* pStart,
		Point2D* pEnd,
		bool(__fastcall* fpDrawCallback)(int*)) = 0;

	virtual bool DrawDashedLine(
		Point2D* pStart,
		Point2D* pEnd,
		COLORREF color,
		bool* pattern,
		int offset) = 0;

	virtual bool DrawDashedLine_(
		Point2D* pStart,
		Point2D* pEnd,
		COLORREF color,
		bool* pattern,
		int offset,
		bool bUkn) = 0;

	virtual bool DrawLine_(
		Point2D* pStart,
		Point2D* pEnd,
		COLORREF color,
		bool bUnk) = 0;

	virtual bool DrawRectEx(
		RectangleStruct* pClipRect,
		RectangleStruct* pDrawRect,
		COLORREF color) = 0;

	virtual bool DrawRect(RectangleStruct* pDrawRect, COLORREF color) = 0;

	virtual void* Lock(int x, int y) = 0;

	virtual bool Unlock() = 0;

	virtual bool CanLock(DWORD dwUkn1 = 0, DWORD dwUkn2 = 0) { return true; }

	virtual bool vt_entry_68(DWORD dwUnk1, DWORD dwUnk2) { return true; } // {JMP_THIS(0x411500);}

	virtual bool IsLocked() = 0;

	virtual int GetBytesPerPixel() = 0;

	virtual int GetPitch() = 0;	//Bytes per scanline

	virtual RectangleStruct* GetRect(RectangleStruct* pRect)
	{
		*pRect = { 0,0,this->Height,this->Width };
		return pRect;
	}

	virtual int GetWidth() { return this->Width; }

	virtual int GetHeight() { return this->Height; }

	virtual bool IsDSurface() { return false; } // guessed - secsome

	// Helper
	RectangleStruct GetRect()
	{
		RectangleStruct ret;
		this->GetRect(&ret);
		return ret;
	}


	// Properties

	int Width;
	int Height;
};

class NOVTABLE XSurface : public Surface
{
public:
	XSurface(int nWidth = 640, int nHeight = 400) { JMP_THIS(0x5FE020); }

	virtual ~XSurface() JMP_THIS(0x4115A0);

	//Surface
	virtual bool CopyFromWhole(Surface* pSrc, bool bUnk1, bool bUnk2) override JMP_THIS(0x7BBAF0);

	virtual bool CopyFromPart(
		RectangleStruct* pClipRect, //ignored and retrieved again...
		Surface* pSrc,
		RectangleStruct* pSrcRect,	//desired source rect of pSrc ?
		DWORD dwUnk,				//IDA: dwUnk=&BlitTrans<byte>::`vftable` dwUnk=BlitPlain<byte>::`vftable` dwUnk=BlitPlain<unsigned short>::`vftable`
		bool bUnk) override JMP_THIS(0x7BBB90);

	virtual bool CopyFrom(
		RectangleStruct* pClipRect,
		RectangleStruct* pClipRect2,	//again? hmm
		Surface* pSrc,
		RectangleStruct* pDestRect,	//desired dest rect of pSrc ? (stretched? clipped?)
		RectangleStruct* pSrcRect,	//desired source rect of pSrc ?
		DWORD dwUnk,				//IDA: dwUnk=&BlitTrans<byte>::`vftable` dwUnk=BlitPlain<byte>::`vftable` dwUnk=BlitPlain<unsigned short>::`vftable`
		bool bUnk) override JMP_THIS(0x7BBCF0);

	virtual bool FillRectEx(
		RectangleStruct* pClipRect,
		RectangleStruct* pFillRect,
		COLORREF color) override JMP_THIS(0x7BB050);

	virtual bool FillRect(RectangleStruct* pFillRect, COLORREF color) override JMP_THIS(0x7BB020);

	virtual bool Fill(COLORREF color) override JMP_THIS(0x7BBAB0);

	virtual bool FillRectTrans(
		RectangleStruct* pClipRect,
		ColorStruct& color,
		int opacity) override
	{ return false; }

	virtual bool DrawEllipse(
		Point2D offset,
		Point2D center,
		RectangleStruct rect,
		COLORREF color) override JMP_THIS(0x7BB350);

	void DrawEllipse(Point2D center, double cellSpread, RectangleStruct rect, COLORREF color)
	{
		double factor = (cellSpread * 2 + 1) / sqrt(8);

		int semiMajor = static_cast<int>(factor * Unsorted::CellWidthInPixels);
		int semiMinor = static_cast<int>(factor * Unsorted::CellHeightInPixels);

		DrawEllipse(center, Point2D(semiMajor, semiMinor), rect, color);
	}

	void DrawEllipse(Point2D center, double cellSpread, COLORREF color)
	{
		RectangleStruct rect = { 0, 0, 0, 0 };
		this->GetRect(&rect);
		rect.Height -= 32; // account for bottom bar

		DrawEllipse(center, cellSpread, rect, color);
	}

	virtual bool SetPixel(Point2D* pPoint, COLORREF color) override JMP_THIS(0x7BAEB0);

	virtual COLORREF GetPixel(Point2D* pPoint) override JMP_THIS(0x7BAE60);

	virtual bool DrawLineEx(
		RectangleStruct* pClipRect,
		Point2D* pStart,
		Point2D* pEnd,
		COLORREF color) override JMP_THIS(0x7BA610);

	virtual bool DrawLine(Point2D* pStart, Point2D* pEnd, COLORREF color) override JMP_THIS(0x7BA5E0);

	virtual bool DrawLineColor_AZ(
		RectangleStruct* pRect,
		Point2D* pStart,
		Point2D* pEnd,
		COLORREF color,
		DWORD dwUnk1,
		DWORD dwUnk2,
		bool bUnk) override
	{ return false; }

	virtual bool DrawMultiplyingLine_AZ(
		RectangleStruct* pRect,
		Point2D* pStart,
		Point2D* pEnd,
		DWORD dwMultiplier,
		DWORD dwUnk1,
		DWORD dwUnk2,
		bool bUnk) override
	{ return false; }

	virtual bool DrawSubtractiveLine_AZ(
		RectangleStruct* pRect,
		Point2D* pStart,
		Point2D* pEnd,
		ColorStruct* pColor,
		DWORD dwUnk1,
		DWORD dwUnk2,
		bool bUnk1,
		bool bUnk2,
		bool bUkn3,
		bool bUkn4,
		float fUkn) override
	{ return false; }

	virtual bool DrawRGBMultiplyingLine_AZ(
		RectangleStruct* pRect,
		Point2D* pStart,
		Point2D* pEnd,
		ColorStruct* pColor,
		float intensity,
		DWORD dwUnk1,
		DWORD dwUnk2) override
	{ return false; }

	virtual bool PlotLine(
		RectangleStruct* pRect,
		Point2D* pStart,
		Point2D* pEnd,
		bool(__fastcall* fpDrawCallback)(int*)) override JMP_THIS(0x7BAB90);

	virtual bool DrawDashedLine(
		Point2D* pStart,
		Point2D* pEnd,
		COLORREF color,
		bool* pattern,
		int offset) override JMP_THIS(0x7BA8C0);

	virtual bool DrawDashedLine_(
		Point2D* pStart,
		Point2D* pEnd,
		COLORREF color,
		bool* pattern,
		int offset,
		bool bUkn) override
	{ return false; }

	virtual bool DrawLine_(
		Point2D* pStart,
		Point2D* pEnd,
		COLORREF color,
		bool bUnk) override
	{ return false; }

	virtual bool DrawRectEx(
		RectangleStruct* pClipRect,
		RectangleStruct* pDrawRect,
		COLORREF color) override JMP_THIS(0x7BADC0);

	virtual bool DrawRect(RectangleStruct* pDrawRect, COLORREF color) override JMP_THIS(0x7BAD90);

	virtual void* Lock(int x, int y) override
	{
		++this->LockLevel;
		return nullptr;
	}

	virtual bool Unlock() override
	{
		--this->LockLevel;
		return true;
	}

	virtual bool IsLocked() override {  return this->LockLevel != 0; }

	virtual bool IsDSurface() override { return false; } // guessed - secsome

	//XSurface
	virtual bool PutPixelClip(Point2D* pPoint, short nUnk, RectangleStruct* pRect) JMP_THIS(0x7BAF90);

	virtual short GetPixelClip(Point2D* pPoint, RectangleStruct* pRect) JMP_THIS(0x7BAF10);

	int LockLevel;
	int BytesPerPixel;
};

class NOVTABLE BSurface : public XSurface
{
public:
	static constexpr constant_ptr<BSurface, 0xB2D928> VoxelSurface {};

	BSurface() : XSurface(), Buffer { this->Width * this->Height * 2 } { BytesPerPixel = 2; ((int*)this)[0] = 0x7E2070; }

	~BSurface() JMP_THIS(0x411650);

	virtual void* Lock(int X, int Y) override JMP_THIS(0x4115F0);

	virtual int GetBytesPerPixel() override { return this->BytesPerPixel; }

	virtual int GetPitch() override { return this->BytesPerPixel * this->GetWidth(); }	//Bytes per scanline

	MemoryBuffer Buffer;
};

#pragma warning(push)
#pragma warning( disable : 4505) // 'function' : unreferenced local function has been removed

// Comments from thomassneddon
static void __fastcall CC_Draw_Shape(
	Surface* pSurface,
	const ConvertClass* palette,
	const SHPStruct* shape,
	int frameIndex,
	const Point2D& position,
	const RectangleStruct& bounds,
	BlitterFlags flags,
	int remap,
	int zAdjust, // + 1 = sqrt(3.0) pixels away from screen
	ZGradient zGradientDescIndex,
	int brightness, // 0~2000. Final color = saturate(OriginalColor * Brightness / 1000.0f)
	int tintColor,
	const SHPStruct* zShape,
	int zShapeFrame,
	Point2D offset
)
{
	JMP_STD(0x4AED70);
}

static Point2D* Fancy_Text_Print_Wide(
	Point2D& retBuffer,
	const wchar_t* text,
	Surface* pSurface,
	const RectangleStruct& bounds,
	const Point2D& location,
	COLORREF foreColor,
	COLORREF backColor,
	TextPrintType flags,
	...)
{
	JMP_STD(0x4A60E0);
}

class ColorScheme;

static Point2D* Fancy_Text_Print_Wide
(
	Point2D& retBuffer,
	const wchar_t* text,
	Surface* pSurface,
	const RectangleStruct& bounds,
	const Point2D& location,
	ColorScheme* foreScheme,
	ColorScheme* backScheme,
	TextPrintType flags,
	...
)
{
	JMP_STD(0x4A61C0);
}

static bool __fastcall ClipLine(Point2D* pStart, Point2D* pEnd, RectangleStruct* pBound)
{
	JMP_STD(0x7BC2B0);
}

#pragma warning(pop)

/*
static Point2D* __fastcall Simple_Text_Print_Wide(
	Point2D* RetVal,
	const wchar_t* Text,
	Surface* Surface,
	RectangleStruct* Bounds,
	Point2D* Location,
	COLORREF ForeColor,
	COLORREF BackColor,
	TextPrintType Flag,
	bool bUkn)
{
	JMP_STD(0x4A5EB0);
}
*/

class NOVTABLE DSurface : public XSurface
{
public:
	static constexpr reference<DSurface*, 0x8872FCu> const Tile{};
	static constexpr reference<DSurface*, 0x887300u> const Sidebar{};
	static constexpr reference<DSurface*, 0x887308u> const Primary{};
	static constexpr reference<DSurface*, 0x88730Cu> const Hidden{};
	static constexpr reference<DSurface*, 0x887310u> const Alternate{};
	static constexpr reference<DSurface*, 0x887314u> const Temp{};
	static constexpr reference<DSurface*, 0x88731Cu> const Composite{};

	static constexpr reference<RectangleStruct, 0x886F90u> const SidebarBounds{};
	static constexpr reference<RectangleStruct, 0x886FA0u> const ViewBounds{};
	static constexpr reference<RectangleStruct, 0x886FB0u> const WindowBounds{};

	// DP-Kratos
	static constexpr reference<bool*, 0x84310C> const pPattern{};

	DSurface(int width, int height, bool bUnk1, bool bUnk2) JMP_THIS(0x4BA5A0);

	virtual ~DSurface() override JMP_THIS(0x4C1AC0);

	virtual bool CopyFromWhole(Surface* pSrc, bool bUnk1, bool bUnk2) override
	{ return this->XSurface::CopyFromWhole(pSrc, bUnk1, bUnk2); }

	virtual bool CopyFromPart(
		RectangleStruct* pClipRect, //ignored and retrieved again...
		Surface* pSrc,
		RectangleStruct* pSrcRect,	//desired source rect of pSrc ?
		DWORD dwUnk,
		bool bUnk) override JMP_THIS(0x4BB080);

	virtual bool CopyFrom(
		RectangleStruct* pClipRect,
		RectangleStruct* pClipRect2,	//again? hmm
		Surface* pSrc,
		RectangleStruct* pDestRect,	//desired dest rect of pSrc ? (stretched? clipped?)
		RectangleStruct* pSrcRect,	//desired source rect of pSrc ?
		DWORD dwUnk,
		bool bUnk) override JMP_THIS(0x4BB0D0);

	virtual bool FillRectEx(
		RectangleStruct* pClipRect,
		RectangleStruct* pFillRect,
		COLORREF color) override JMP_THIS(0x4BB620);

	virtual bool FillRect(RectangleStruct* pFillRect, COLORREF color) override
	{
		RectangleStruct Rect;
		RectangleStruct* pRect = this->GetRect(&Rect);
		return this->FillRectEx(pRect, pFillRect, color);
		//JMP_THIS(0x4BB5F0);
	}

	virtual bool FillRectTrans(
		RectangleStruct* pClipRect,
		ColorStruct& color,
		int opacity) override JMP_THIS(0x4BB830);

	virtual bool DrawLineColor_AZ(
		RectangleStruct* pRect,
		Point2D* pStart,
		Point2D* pEnd,
		COLORREF color,
		DWORD dwUnk1,
		DWORD dwUnk2,
		bool bUnk) override JMP_THIS(0x4BFD30);

	virtual bool DrawMultiplyingLine_AZ(
		RectangleStruct* pRect,
		Point2D* pStart,
		Point2D* pEnd,
		DWORD dwMultiplier,
		DWORD dwUnk1,
		DWORD dwUnk2,
		bool bUnk) override JMP_THIS(0x4BBCA0);

	virtual bool DrawSubtractiveLine_AZ(
		RectangleStruct* pRect,
		Point2D* pStart,
		Point2D* pEnd,
		ColorStruct* pColor,
		DWORD dwUnk1,
		DWORD dwUnk2,
		bool bUnk1,
		bool bUnk2,
		bool bUkn3,
		bool bUkn4,
		float fUkn) override JMP_THIS(0x4BC750);

	virtual bool DrawRGBMultiplyingLine_AZ(
		RectangleStruct* pRect,
		Point2D* pStart,
		Point2D* pEnd,
		ColorStruct* pColor,
		float intensity,
		DWORD dwUnk1,
		DWORD dwUnk2) override JMP_THIS(0x4BDF00);

	virtual bool DrawDashedLine_(
		Point2D* pStart,
		Point2D* pEnd,
		COLORREF color,
		bool* pattern,
		int offset,
		bool bUkn) override JMP_THIS(0x4C0750);

	virtual bool DrawLine_(
		Point2D* pStart,
		Point2D* pEnd,
		COLORREF color,
		bool bUnk) override JMP_THIS(0x4C0E30);

	virtual void* Lock(int x, int y) override JMP_THIS(0x4BAD80);

	virtual bool Unlock() override JMP_THIS(0x4BAF40);

	virtual bool CanLock(DWORD dwUkn1 = 0, DWORD dwUkn2 = 0) override JMP_THIS(0x4BAEC0);

	virtual int GetBytesPerPixel() override { return this->BytesPerPixel; }

	virtual int GetPitch() override { return this->VideoSurfaceDescription->lPitch; }	//Bytes per scanline

	virtual bool IsDSurface() override { return true; } // guessed - secsome

	//DSurface
	virtual bool DrawGradientLine(
		RectangleStruct* pRect,
		Point2D* pStart,
		Point2D* pEnd,
		ColorStruct* pStartColor,
		ColorStruct* pEndColor,
		float step,
		COLORREF color) JMP_THIS(0x4BF750);

	virtual bool CanBlit() JMP_THIS(0x4BAF20);
	//{ return this->VideoSurfacePtr->GetBltStatus(1) == 0; }

	// Comments from thomassneddon
	void DrawSHP
	(
		ConvertClass* palette,
		SHPStruct* shape,
		int frameIndex,
		const Point2D& position,
		const RectangleStruct& bounds,
		BlitterFlags flags = BlitterFlags::None,
		int remap = 0,
		int zAdjust = 0, // + 1 = sqrt(3.0) pixels away from screen
		ZGradient zGradientDescIndex = ZGradient::Ground,
		int brightness = 1000, // 0~2000. Final color = saturate(OriginalColor * Brightness / 1000.0f)
		int tintColor = 0,
		SHPStruct* zShape = nullptr,
		int zShapeFrame = 0,
		Point2D offset = Point2D::Empty
	)
	{
		CC_Draw_Shape
		(
			this,
			palette,
			shape,
			frameIndex,
			position,
			bounds,
			flags,
			remap,
			zAdjust,
			zGradientDescIndex,
			brightness,
			tintColor,
			zShape,
			zShapeFrame,
			offset
		);
	}

	void DrawText
	(
		const wchar_t* text,
		const RectangleStruct& bounds,
		const Point2D& location,
		COLORREF foreColor,
		COLORREF backColor,
		TextPrintType flags
	)
	{
		Point2D tmp = Point2D::Empty;
		Fancy_Text_Print_Wide(tmp, text, this, bounds, location, foreColor, backColor, flags);
	}

	void DrawText
	(
		const wchar_t* text,
		const RectangleStruct& bounds,
		const Point2D& location,
		ColorScheme* foreColor,
		ColorScheme* backColor,
		TextPrintType flags
	)
	{
		Point2D tmp = Point2D::Empty;
		Fancy_Text_Print_Wide(tmp, text, this, bounds, location, foreColor, backColor, flags);
	}

	void DrawText(const wchar_t* text, const Point2D& loction, COLORREF color)
	{
		RectangleStruct rect = { 0, 0, 0, 0 };
		this->GetRect(&rect);

		Point2D tmp = Point2D::Empty;
		Fancy_Text_Print_Wide(tmp, text, this, rect, loction, color, 0, TextPrintType::NoShadow);
	}

	void DrawText(const wchar_t* text, int x, int y, COLORREF color)
	{
		DrawText(text, Point2D(x, y), color);
	}

	void* Buffer;
	bool IsAllocated;
	bool IsInVideoRam;
	PROTECTED_PROPERTY(BYTE, align_1A[2]);
	IDirectDrawSurface* VideoSurfacePtr;
	DDSURFACEDESC2* VideoSurfaceDescription;
};
