#pragma once

#include <YRPP.h>

class NOVTABLE BitFont
{
public:
	static constexpr reference<BitFont*, 0x89C4D0> Instance {};

	virtual ~BitFont() RX;

	bool GetTextDimension(const wchar_t* text, int* pWidth, int* pHeight, int maxWidth)
	{ JMP_THIS(0x433CF0); }

	int Blit(wchar_t wch, int x, int y, COLORREF color)
	{ JMP_THIS(0x434120); }

	void Lock(Surface* pSurface)
	{ JMP_THIS(0x4348F0); }

	void Unlock(Surface* pSurface)
	{ JMP_THIS(0x434990); }

	unsigned char* GetCharacterBitmap(wchar_t wch)
	{ JMP_THIS(0x4346C0); }

	void SetBounds(LTRBStruct* pBound)
	{
		if (pBound)
			this->Bounds = *pBound;
		else
			this->Bounds = { 0,0,0,0 };
	}

	void SetColor(WORD nColor)
	{
		this->Color = nColor;
	}

	void SetField20(int x)
	{ JMP_THIS(0x434110); }

	void SetField41(char flag)
	{
		this->field_41 = flag;
	}

	/// Properties
	struct InternalData
	{
		int FontWidth;
		int Stride;
		int FontHeight;
		int Lines;
		int Count;
		int SymbolDataSize;
		short* SymbolTable;
		char* Bitmaps;
		int ValidSymbolCount;
	};

	static InternalData* __fastcall LoadInternalData(const char* filename)
	{ JMP_STD(0x433990); }

	BitFont(const char* filename)
	{ JMP_THIS(0x433880); }

	InternalData* InternalPTR;
	void* Pointer_8;
	short* pGraphBuffer;
	int PitchDiv2;
	int Unknown_14;
	wchar_t* field_18;
	int field_1C;
	int field_20;
	WORD Color;
	short DefaultColor2;
	int Unknown_28;
	int State_2C;
	LTRBStruct Bounds;
	bool Bool_40;
	bool field_41;
	bool field_42;
	bool field_43;
};
