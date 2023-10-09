#pragma once

#include <GaugeClass.h>

class NOVTABLE SliderClass : public GaugeClass
{
public:

	//Destructor
	virtual ~SliderClass() override JMP_THIS(0x558180);

	//GadgetClass
	virtual void PeerToPeer(unsigned int flags, DWORD* pKey, GadgetClass* pSendTo) override JMP_THIS(0x6B2160);
	virtual bool Draw(bool forced) override JMP_THIS(0x6B20F0);
	virtual bool Action(GadgetFlag flags, DWORD* pKey, KeyModifier modifier) override JMP_THIS(0x6B1F50);

	//GaugeClass
	virtual bool SetMaximum(int value) override JMP_THIS(0x6B1D40);
	virtual bool SetValue(int value) override JMP_THIS(0x6B1E50);
	virtual int GetThumbPixel() JMP_THIS(0x4E30C0); // return 4 if not overloaded
	virtual void DrawThumb() JMP_THIS(0x4E29A0);

	//SliderClass
	virtual void SetThumb(int value) JMP_THIS(0x6B1DF8);
	virtual int Bump(bool minus) JMP_THIS(0x6B2000); // CurrentValue +=/-= Thumb
	virtual int Step(bool minus) JMP_THIS(0x6B2040); // CurrentValue +=/-= 1

	//Non virtual
	void RecalculateThumb()
	{ JMP_THIS(0x6B1EE0); }

	//Constructors
	SliderClass(unsigned int id, int x, int y, int width, int height, bool belongToList) noexcept
		: GaugeClass(noinit_t())
	{ JMP_THIS(0x6B1B20); }

	SliderClass(SliderClass& another) noexcept
		: GaugeClass(noinit_t())
	{ JMP_THIS(0x5581A0); }

	explicit __forceinline SliderClass(noinit_t)  noexcept // not protected for ListClass Constructor
		: GaugeClass(noinit_t())
	{
	}

	//Properties
public:

	GadgetClass* PlusGadget;
	GadgetClass* MinusGadget;
	bool BelongToList;
	int Thumb;
	int ThumbSize;
	int ThumbStart;
};
