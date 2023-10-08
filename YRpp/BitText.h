#pragma once

#include <YRPP.h>

class BitFont;

class BitText
{
public:
	static constexpr reference<BitText*, 0x89C4B8> Instance {};

	virtual ~BitText() JMP_THIS(0x435560);

	// Seems like draw in a single line
	void Print(BitFont* pFont, Surface* pSurface, const wchar_t* pWideString, int x, int y, int width, int height)
	{ JMP_THIS(0x434B90); }

	void DrawText(BitFont* pFont, Surface* pSurface, const wchar_t* pWideString, int x, int y, int width, int height, char a8, int a9, int colorAdjust)
	{ JMP_THIS(0x434CD0); }

	BitText()
	{ JMP_THIS(0x434AD0); }
};
