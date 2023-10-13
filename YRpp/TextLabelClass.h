#pragma once

#include <GadgetClass.h>

class NOVTABLE TextLabelClass : public GadgetClass
{
public:

	//Destructor
	virtual ~TextLabelClass() override RX;// JMP_THIS(0x72A670);

	//GadgetClass
	virtual bool Draw(bool forced) override JMP_THIS(0x72A4A0);

	//TextLabelClass
	virtual void SetText(wchar_t* pText) JMP_THIS(0x72A660);

	//Non virtual

	//Statics

	//Constructors
	TextLabelClass(wchar_t* pText, int x, int y, int colorSchemeIndex, TextPrintType style) noexcept
		: GadgetClass(noinit_t())
	{ JMP_THIS(0x72A440); }

protected:
	explicit __forceinline TextLabelClass(noinit_t)  noexcept
		: GadgetClass(noinit_t())
	{
	}

	//Properties
public:

	void* UserData1;
	void* UserData2;
	DWORD Style;
	wchar_t* Text;
	int ColorSchemeIndex;
	DWORD PixWidth;
	DWORD anim_dword3C;
	bool SkipDraw;
	bool Animate;
	DWORD AnimPos;
	DWORD AnimTiming;
};
