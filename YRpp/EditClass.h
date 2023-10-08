#pragma once

#include <ControlClass.h>
#include <GeneralStructures.h>

// Accept Alpha, Digit, and other characters, ToUpper will convert alpha into upper-case
enum class EditFlag : int
{
	Alpha = 0x1,
	Digit = 0x2,
	Other = 0x4,
	ToUpper = 0x8
};

class NOVTABLE EditClass : public ControlClass
{
public:
	//Destructor
	virtual ~EditClass() override JMP_THIS(0x4B57D0);

	//GadgetClass
	virtual void SetFocus() override JMP_THIS(0x4C3570);
	virtual bool Draw(bool forced) JMP_THIS(0x4C3110);
	virtual bool Action(GadgetFlag flags, DWORD* pKey, KeyModifier modifier) override JMP_THIS(0x4C3190);

	//EditClass
	virtual void SetText(wchar_t* text, int maxLength) JMP_THIS(0x4C30E0);
	virtual wchar_t* GetText() JMP_THIS(0x4B55A0);
	virtual void DrawBackground() JMP_THIS(0x4C32A0);
	virtual void DrawText(wchar_t* text) JMP_THIS(0x4C32E0);
	virtual bool HandleKeyInput(int ascii) JMP_THIS(0x4C3420);

	//Non virtual

	//Statics

	//Constructors
	EditClass(int id, wchar_t* text, int maxLength, TextPrintType textFlag, int x, int y, int width, int height, EditFlag editFlag) noexcept
		: ControlClass(noinit_t())
	{ JMP_THIS(0x4C2FC0); }

protected:
	explicit __forceinline EditClass(noinit_t)  noexcept
		: ControlClass(noinit_t())
	{
	}

	//Properties
public:

	TextPrintType TextFlags;
	EditFlag EditFlags; // 1 for alpha only; 2 for digit only; 4 for not num nor alpha accepted; 8 to convert alpha into upper case
	wchar_t* Text;
	int MaxLength;
	int Length;
	int Color;
	bool IsReadOnly;
};
