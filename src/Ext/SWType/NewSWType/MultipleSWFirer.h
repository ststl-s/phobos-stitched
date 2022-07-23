#pragma once
#include "NewSWType.h"

class MultipleSWFirer : public NewSWType
{
public:

	virtual void LoadFromINI(SWTypeExt::ExtData* pData, SuperWeaponTypeClass* pSW, CCINIClass* pINI) override;

	virtual const char* GetTypeID() override;
	virtual bool Activate(SuperClass* pSW, const CellStruct& cell, bool isPlayer) override;
};
