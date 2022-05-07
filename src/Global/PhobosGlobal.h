#pragma once
#include <Utilities/TemplateDef.h>
#include <BuildingClass.h>
#include <TechnoClass.h>
#include <SuperWeaponTypeClass.h>

class PhobosGlobal
{
public:
	static std::map<TechnoTypeClass*, SuperWeaponTypeClass*> AuxTechnoMapper;
};