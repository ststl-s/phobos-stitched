#pragma once

#include <ControlClass.h>

class NOVTABLE ToggleClass : public ControlClass
{
public:
	//Destructor
	virtual ~ToggleClass() override RX;// JMP_THIS(0x4B5810);

	//GadgetClass
	virtual bool Action(GadgetFlag flags, DWORD* pKey, KeyModifier modifier) override JMP_THIS(0x723EC0);

	//Non virtual
	void TurnOn()
	{ JMP_THIS(0x723EA0); }

	void TurnOff()
	{ JMP_THIS(0x723EB0); }

	//Statics

	//Constructors
	ToggleClass(unsigned int id, int x, int y, int width, int height) noexcept
		: ToggleClass(noinit_t())
	{ JMP_THIS(0x723E60); }

protected:
	explicit __forceinline ToggleClass(noinit_t)  noexcept
		: ControlClass(noinit_t())
	{
	}

	//Properties
public:

	bool IsPressed;
	bool IsOn;
	DWORD ToggleType;
};
