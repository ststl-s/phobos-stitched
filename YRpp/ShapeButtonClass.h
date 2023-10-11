#pragma once

#include <ToggleClass.h>

class ConvertClass;
class SHPStruct;

enum class ShapeButtonFlag : int
{
	UpShape = 0x0,
	DownShape = 0x1,
	DisabledShape = 0x2
};

class NOVTABLE ShapeButtonClass : public ToggleClass
{
public:
	//Destructor
	virtual ~ShapeButtonClass() override JMP_THIS(0x4B57F0);

	//GadgetClass
	virtual bool Draw(bool forced) override JMP_THIS(0x69DEB0);

	//ShapeButtonClass
	virtual void SetShape(SHPStruct* shape, int width, int height) JMP_THIS(0x69DE00);

	//Non virtual
	ShapeButtonClass& operator=(ShapeButtonClass& another)
	{ JMP_THIS(0x4B5630); }

	void ClearShape()
	{ JMP_THIS(0x69DE70); }

	void SetLoadedStatus(bool status)
	{ JMP_THIS(0x69DEA0); }

	bool StartFlashing(int delay, int initDelay, bool start)
	{ JMP_THIS(0x69DFC0); }

	bool StopFlashing()
	{ JMP_THIS(0x69DFF0); }

	bool AI()
	{ JMP_THIS(0x69E010); } // Flash_AI

	bool IsFlashing()
	{ JMP_THIS(0x69E050); }

	bool ToFlash()
	{ JMP_THIS(0x69E060); } // no need for this function

	//Statics

	//Constructors
	ShapeButtonClass() noexcept
		:ShapeButtonClass(noinit_t())
	{ JMP_THIS(0x69DCF0); }

	ShapeButtonClass(unsigned int id, SHPStruct* shape, int x, int y, int width, int height, bool alpha) noexcept
		: ShapeButtonClass(noinit_t())
	{ JMP_THIS(0x69DD30); }

protected:
	explicit __forceinline ShapeButtonClass(noinit_t)  noexcept
		: ToggleClass(noinit_t())
	{
	}

	//Properties
public:

	bool IsToFlash;
	int FlashDelay;
	int FlashCounter;
	bool UseFlash; // ReflectButtonState
	Point2D DrawPosition;
	bool UseSidebarSurface;
	ConvertClass* Drawer;
	bool IsDrawn;
	bool IsAlpha;
	SHPStruct* ShapeData;
	bool IsShapeLoaded;
};
