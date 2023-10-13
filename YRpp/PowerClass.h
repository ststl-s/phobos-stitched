#pragma once

#include <RadarClass.h>

class NOVTABLE PowerClass : public RadarClass
{
public:
	//Static
	static constexpr constant_ptr<PowerClass, 0x87F7E8u> const Instance{};

	//Destructor
	virtual ~PowerClass() override RX;// JMP_THIS(0x6404B0);

	//GScreenClass
	virtual void One_Time() override JMP_THIS(0x63F7B0);
	virtual void Init_Clear() override JMP_THIS(0x63F730);
	virtual void Update(const int& keyCode, const Point2D& mouseCoords) override JMP_THIS(0x63FEA0);
	virtual void Draw(DWORD dwUnk) override JMP_THIS(0x63FB20);

	//DisplayClass
	virtual const wchar_t* GetToolTip(UINT nDlgID) override JMP_THIS(0x640450);
	virtual void CloseWindow() override JMP_THIS(0x6403A0); //prolly wrong naming

	//RadarClass
	virtual void DisposeOfArt() override JMP_THIS(0x63F7E0);
	virtual void Init_For_House() override JMP_THIS(0x63F7C0);

	//Non-virtual

protected:
	//Constructor
	PowerClass() noexcept
	{ JMP_THIS(0x63F6B0); }	//don't need this

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:
	bool unknown_bool_150C;
	PROTECTED_PROPERTY(BYTE, align_150D[3])
	CDTimerClass unknown_timer_1510;
	DWORD unknown_151C;
	CDTimerClass unknown_timer_1520;
	DWORD unknown_152C;
	DWORD unknown_1530;
	DWORD unknown_1534;
	bool unknown_bool_1538;
	PROTECTED_PROPERTY(BYTE, align_1539[3])
	int PowerOutput;
	int PowerDrain;
};
