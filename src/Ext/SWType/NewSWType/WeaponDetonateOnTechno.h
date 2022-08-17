#pragma once
#include "NewSWType.h"

class WeaponDetonateOnTechno : public NewSWType
{
public:

	virtual void LoadFromINI(SWTypeExt::ExtData* pData, SuperWeaponTypeClass* pSW, CCINIClass* pINI) override;

	virtual const char* GetTypeID() override;
	virtual bool Activate(SuperClass* pSW, const CellStruct& cell, bool isPlayer) override;

	template <class Entity>
	void ProcessSW(SWTypeExt::ExtData* pSWTypeExt, const std::map<int, std::vector<Entity*>>& ownedTechnos);

	template <class Entity>
	void ProcessSW(SWTypeExt::ExtData* pSWTypeExt, const std::vector<Entity*>& vTechnos);
};
