#pragma once

#include <WarheadTypeClass.h>

#include <Utilities/Template.h>

class AresWarheadTypeExt
{
public:
	WarheadTypeClass* AttachObject;
	DWORD dword_4;
	bool MindControl_Permanent;
	int EMP_Duration;
	//0x10
	int EMP_Cap;
	AnimTypeClass* EMP_Sparkles;
	int IronCurtain_Duration;
	int IronCurtain_Cap;
	//0x20
	int IronCurtain_Flash;
};
