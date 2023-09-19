#pragma once

#include <ControlClass.h>

class NOVTABLE GaugeClass : public ControlClass
{
public:
	//GadgetClass
	virtual bool Draw(bool forced) override JMP_THIS(0x4E2690);
	virtual bool Action(GadgetFlag flags, DWORD* pKey, KeyModifier modifier) override JMP_THIS(0x4E2830);

	//GaugeClass
	virtual bool SetMaximum(int value) JMP_THIS(0x4E2580);
	virtual bool SetValue(int value) JMP_THIS(0x4E25A0);
	virtual int GetValue() JMP_THIS(0x4E30A0);
	virtual void SetThumb(bool value) JMP_THIS(0x4E30B0); // Set HasThumb
	virtual int GetThumbPixel() JMP_THIS(0x4E30C0); // return 4 if not overloaded
	virtual void DrawThumb() JMP_THIS(0x4E29A0);
	virtual int PixelToValue(int pixel) JMP_THIS(0x4E25D0);
	virtual int ValueToPixel(int value) JMP_THIS(0x4E2650);

	//Non virtual

	//Statics

	//Constructors
	GaugeClass(unsigned int id, int x, int y, int width, int height) noexcept
		: ControlClass(noinit_t())
	{ JMP_THIS(0x4E2500); }

protected:
	explicit __forceinline GaugeClass(noinit_t)  noexcept
		: ControlClass(noinit_t())
	{
	}

	//Properties
public:

	bool IsColorized;
	bool HasThumb;
	bool IsHorizontal;
	int MaxValue;
	int CurrentValue;
	int ClickDiff;
};
