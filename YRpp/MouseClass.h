#pragma once

#include <SidebarClass.h>

class MouseCursor
{
public:
	static constexpr reference<MouseCursor, 0x82D028u, 86u> const Cursors{};

	static MouseCursor& GetCursor(MouseCursorType cursor)
	{
		return Cursors[static_cast<int>(cursor)];
	}

	MouseCursor() = default;

	MouseCursor
	(
		int frame,
		int count,
		int interval,
		int miniFrame,
		int miniCount,
		MouseHotSpotX hotX,
		MouseHotSpotY hotY
	)
		: Frame(frame)
		, Count(count)
		, Interval(interval)
		, MiniFrame(miniFrame)
		, MiniCount(miniCount)
		, HotX(hotX)
		, HotY(hotY)
	{ }

	int Frame{ 0 };
	int Count{ 1 };
	int Interval{ 1 };
	int MiniFrame{ -1 };
	int MiniCount{ 0 };
	MouseHotSpotX HotX{ MouseHotSpotX::Center };
	MouseHotSpotY HotY{ MouseHotSpotY::Middle };
};

struct TabDataClass
{
	TabDataClass() JMP_THIS(0x4A2350);

	int TargetValue;
	int LastValue;
	bool NeedsRedraw;
	bool ValueIncreased;
	bool ValueChanged;
	PROTECTED_PROPERTY(BYTE, align_B);
	int ValueDelta;
};

class TabClass : public SidebarClass, public INoticeSink
{
public:
	//Static
	static constexpr constant_ptr<TabClass, 0x87F7E8u> const Instance{};

	//Destructor
	virtual ~TabClass() override JMP_THIS(0x5BE9E0);

	//GScreenClass
	virtual void One_Time() override JMP_THIS(0x6D0260);
	virtual void Init_IO() override JMP_THIS(0x6D03A0);
	virtual void Update(const int& keyCode, const Point2D& mouseCoords) override JMP_THIS(0x6D0680);
	virtual void Draw(DWORD dwUnk) override JMP_THIS(0x6D0A20);

	//DisplayClass
	virtual const wchar_t* GetToolTip(UINT nDlgID) override JMP_THIS(0x6D1800);

	//RadarClass
	virtual void DisposeOfArt() override JMP_THIS(0x6D0270);
	virtual void Init_For_House() override JMP_THIS(0x6D02B0);

	//SidebarClass
	virtual bool Activate(int control = 1) override JMP_THIS(0x6D04F0);

	TabClass() JMP_THIS(0x6CFE20);

	TabDataClass TabData;
	CDTimerClass  unknown_timer_552C;
	CDTimerClass  InsufficientFundsBlinkTimer;
	BYTE unknown_byte_5544;
	bool MissionTimerPinged;
	BYTE unknown_byte_5546;
	PROTECTED_PROPERTY(BYTE, padding_5547);
};

class ScrollClass : public TabClass
{
public:
	//Static
	static constexpr constant_ptr<ScrollClass, 0x87F7E8u> const Instance{};

	//Destructor
	virtual ~ScrollClass() override JMP_THIS(0x6938F0);

	//GScreenClass
	virtual void Init_IO() override JMP_THIS(0x40D270);
	virtual void Update(const int& keyCode, const Point2D& mouseCoords) override JMP_THIS(0x6922E0);

	//MapClass
	virtual bool DraggingInProgress() override JMP_THIS(0x693060);

	//DisplayClass
	virtual void vt_entry_8C() override JMP_THIS(0x6938C0);
	virtual void vt_entry_B0(DWORD dwUnk) override JMP_THIS(0x693880);
	virtual void RightMouseButtonUp(DWORD dwUnk) override JMP_THIS(0x693840);

	ScrollClass() JMP_THIS(0x692290);

	DWORD unknown_int_5548;
	BYTE unknown_byte_554C;
	PROTECTED_PROPERTY(BYTE, align_554D[3]);
	DWORD unknown_int_5550;
	DWORD unknown_int_5554;
	BYTE unknown_byte_5558;
	BYTE unknown_byte_5559;
	BYTE unknown_byte_555A;
	PROTECTED_PROPERTY(BYTE, padding_555B);
};

class NOVTABLE MouseClass : public ScrollClass
{
public:
	//Static
	static constexpr constant_ptr<MouseClass, 0x87F7E8u> const Instance{};

	//Destructor
	virtual ~MouseClass() JMP_THIS(0x40D290);

	//GScreenClass
	virtual void One_Time() override JMP_THIS(0x5BDF30);
	virtual void Init_Clear() override JMP_THIS(0x5BDF50);
	virtual void Update(const int& keyCode, const Point2D& mouseCoords) override JMP_THIS(0x5BDDC0);
	virtual bool SetCursor(MouseCursorType idxCursor, bool miniMap) override JMP_THIS(0x5BDA80);
	virtual bool UpdateCursor(MouseCursorType idxCursor, bool miniMap) override JMP_THIS(0x5BDC80);
	virtual bool RestoreCursor() override JMP_THIS(0x5BDAA0);
	virtual void UpdateCursorMinimapState(bool miniMap) override JMP_THIS(0x5BDAB0);

	//DisplayClass
	virtual HRESULT Load(IStream* pStm) override JMP_THIS(0x5BDF70);
	virtual HRESULT Save(IStream* pStm) override JMP_THIS(0x5BE6D0);
	virtual MouseCursorType GetLastMouseCursor() override JMP_THIS(0x40D280);

	MouseClass() JMP_THIS(0x5BDA40);

	bool MouseCursorIsMini;
	PROTECTED_PROPERTY(BYTE, unknown_byte_5559[3]);
	MouseCursorType MouseCursorIndex;
	MouseCursorType MouseCursorLastIndex;
	int MouseCursorCurrentFrame;
};
