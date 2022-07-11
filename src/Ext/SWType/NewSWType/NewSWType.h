#pragma once
#include <Ext/SWType/Body.h>
#include <SuperClass.h>
#include <HouseClass.h>

class NewSWType
{
public:

	static void Init();
	static void Clear();
	static int GetNewSWTypeIdx(const char* TypeID);
	static NewSWType* GetNthItem(int idx);

	virtual int GetTypeIndex() final;

	// selectable override

	virtual void Initialize(SWTypeExt::ExtData* pData, SuperWeaponTypeClass* pSW) { }
	virtual void LoadFromINI(SWTypeExt::ExtData* pData, SuperWeaponTypeClass* pSW, CCINIClass* pINI) { }

	// must be override

	virtual const char* GetTypeID() = 0;
	virtual bool Activate(SuperClass* pSW, const CellStruct& Coords, bool IsPlayer) = 0;

	// load/save

	virtual bool Load(PhobosStreamReader& stm) final;
	virtual bool Save(PhobosStreamWriter& stm) final;

	static bool LoadGlobals(PhobosStreamReader& stm);
	static bool SaveGlobals(PhobosStreamWriter& stm);

protected:

	virtual void SetTypeIndex(int idx) final;

private:

	static std::vector<std::unique_ptr<NewSWType>> Array;

	static void Register(std::unique_ptr<NewSWType> pType);

	// template can't be virtual
	template <typename T>
	bool Serialize(T& stm);

	/*-----------------------------------------------------------------------------------------
	Property
	Property include global setting of this NewSWType, entrys of this type should in SWTypeExt::ExtData
	------------------------------------------------------------------------------------------*/

public:

protected:

private:

	int TypeIndex = -1;
};
