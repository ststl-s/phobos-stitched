#pragma once
#include <Ext/SWType/Body.h>

class NewSWType
{
public:

	static void Init();
	static int GetNewSWTypeIdx(const char* TypeID);
	static NewSWType* GetNthItem(int idx);
	
	virtual int GetTypeIndex();

	virtual void Initialize(SWTypeExt::ExtData* pData, SuperWeaponTypeClass* pSW) { }
	virtual void LoadFromINI(SWTypeExt::ExtData* pData, SuperWeaponTypeClass* pSW, CCINIClass* pINI) { }

	virtual const char* GetTypeID() = 0;
	virtual bool Activate(SuperClass* pSW, const CellStruct& Coords, bool IsPlayer) = 0;

protected:

	virtual void SetTypeIndex(int idx);

private:

	static std::vector<std::unique_ptr<NewSWType>> Array;

	static void Register(std::unique_ptr<NewSWType> pType);

/*-----------------------------------------------------------------------------------------
================================Property==================================================
------------------------------------------------------------------------------------------*/
public:

protected:

private:

	int TypeIndex = -1;
};