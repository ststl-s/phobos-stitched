#pragma once

#include <TechnoClass.h>

#include <Utilities/Container.h>

class AresTechnoExtData
{
public:
	TechnoClass* OwnerObject;
	InitState Initialized;
	byte unknown[0x70];
	double ROFMultiplier;
};
