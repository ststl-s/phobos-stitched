/*
	TerrainTypes are initialized by INI files.
*/

#pragma once

#include <ObjectTypeClass.h>

class NOVTABLE TerrainTypeClass : public ObjectTypeClass
{
public:
	static const AbstractType AbsID = AbstractType::TerrainType;

	//Array
	ABSTRACTTYPE_ARRAY(TerrainTypeClass, 0xA8E318u);

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override JMP_STD(0x71E260);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x71E1D0);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override JMP_STD(0x71E240);

	//Destructor
	virtual ~TerrainTypeClass() override JMP_THIS(0x71E360);

	//AbstractClass
	virtual AbstractType WhatAmI() const override JMP_THIS(0x71E330);
	virtual int Size() const override JMP_THIS(0x71E340);
	virtual void ComputeCRC(CRCEngine& crc) const override JMP_THIS(0x71E140);
	virtual int GetArrayIndex() const override JMP_THIS(0x71E350);

	//AbstractTypeClass
	virtual bool LoadFromINI(CCINIClass* pINI) override JMP_THIS(0x71DEA0);

	//ObjectTypeClass
	virtual CoordStruct* vt_entry_6C(CoordStruct* pDest, CoordStruct* pSrc) const override JMP_THIS(0x71E0D0);
	virtual bool SpawnAtMapCoords(CellStruct* pMapCoords,HouseClass* pOwner) override JMP_THIS(0x71DDD0);
	virtual ObjectClass* CreateObject(HouseClass* owner) override JMP_THIS(0x71DE10);
	virtual CellStruct* GetFoundationData(bool includeBib) const override JMP_THIS(0x71DE40);

	//Constructor
	TerrainTypeClass(const char* pID) noexcept
		: TerrainTypeClass(noinit_t())
	{ JMP_THIS(0x71DA80); }

protected:
	explicit __forceinline TerrainTypeClass(noinit_t) noexcept
		: ObjectTypeClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	int ArrayIndex;
	int Foundation;
	ColorStruct RadarColor;
	int AnimationRate;
	float AnimationProbability;
	int TemperateOccupationBits;
	int SnowOccupationBits;
	bool WaterBound;
	bool SpawnsTiberium;
	bool IsFlammable;
	bool IsAnimated;
	bool IsVeinhole;
	CellStruct* FoundationData;
};
