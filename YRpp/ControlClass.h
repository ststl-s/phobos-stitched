#pragma once

#include <GadgetClass.h>

class NOVTABLE ControlClass : public GadgetClass
{
public:
	//Destructor
	virtual ~ControlClass() override JMP_THIS(0x48E660);

	//GadgetClass
	virtual unsigned int const GetID() override JMP_THIS(0x48E610);
	virtual bool Draw(bool forced) override JMP_THIS(0x48E620);
	virtual bool Action(GadgetFlag flags, DWORD* pKey, KeyModifier modifier) override JMP_THIS(0x48E5A0);

	//ControlClass
	virtual void SetSendTo(GadgetClass* pSendTo) JMP_THIS(0x48E600);

	//Non virtual

	//Statics

	//Constructors
	ControlClass(unsigned int id, int x, int y, int width, int height, GadgetFlag flags, bool sticky) noexcept
		: ControlClass(noinit_t())
	{ JMP_THIS(0x48E520); }

	ControlClass(ControlClass& other) noexcept
		: ControlClass(noinit_t())
	{ JMP_THIS(0x48E570); }

protected:
	explicit __forceinline ControlClass(noinit_t)  noexcept
		: GadgetClass(noinit_t())
	{
	}

	//Properties
public:

	int ID;
	GadgetClass* SendTo; // Peer
};
