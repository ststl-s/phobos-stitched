/*
	ObjectTypes are initialized by INI files.
*/

#pragma once

#include <AbstractTypeClass.h>
#include <Helpers\String.h>

class AircraftTypeClass;
class InfantryTypeClass;
class UnitTypeClass;

class NOVTABLE HouseTypeClass : public AbstractTypeClass
{
public:
	static const AbstractType AbsID = AbstractType::HouseType;

	//Array
	ABSTRACTTYPE_ARRAY(HouseTypeClass, 0xA83C98u);
	
	//static
	//static HouseTypeClass* __fastcall FindOrAllocate(const char* pID) JMP_STD(0x512680);
	//static int __fastcall FindIndex(const char* pID) JMP_STD(0x5117D0);

	//IUnknown
	virtual HRESULT __stdcall QueryInterface(REFIID iid, void** ppvObject) JMP_STD(0x5125A0);
	virtual ULONG __stdcall AddRef() { return true; }
	virtual ULONG __stdcall Release() { return true; }

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) JMP_STD(0x512640);

	//IPersistStream
	virtual HRESULT __stdcall IsDirty() { return 0; }
	virtual HRESULT __stdcall Load(IStream* pStm) JMP_STD(0x512290);
	virtual HRESULT __stdcall Save(IStream* pStm,BOOL fClearDirty) JMP_STD(0x512480);
	virtual HRESULT __stdcall GetSizeMax(ULARGE_INTEGER* pcbSize) JMP_STD(0x512570);

	//Destructor
	virtual ~HouseTypeClass() JMP_THIS(0x512760);

	//AbstractClass
	virtual AbstractType WhatAmI() const { return AbstractType::HouseType; }
	virtual int	Size() const { return 0x1B0; }
	virtual void CalculateChecksum(Checksummer& checksum) const JMP_THIS(0x512170);
	virtual int GetArrayIndex() const { return this->ArrayIndex; }

	//AbstractTypeClass
	virtual bool LoadFromINI(CCINIClass* pINI) JMP_THIS(0x511850);

	//helpers
	HouseTypeClass* FindParentCountry() const {
		return HouseTypeClass::Find(this->ParentCountry);
	}

	int FindParentCountryIndex() const {
		return HouseTypeClass::FindIndexOfName(this->ParentCountry);
	}

	static signed int __fastcall FindIndexOfName(const char *name)
		{ JMP_STD(0x5117D0); }

	//Constructor
	HouseTypeClass(const char* pID) noexcept
		: HouseTypeClass(noinit_t())
	{ JMP_THIS(0x5113F0); }

	HouseTypeClass(IStream* pStm) noexcept
		: HouseTypeClass(noinit_t())
	{ JMP_THIS(0x511650); }

protected:
	explicit __forceinline HouseTypeClass(noinit_t) noexcept
		: AbstractTypeClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	FixedString<25> ParentCountry;
	PROTECTED_PROPERTY(BYTE, align_B1[3]);
	int            ArrayIndex;
	int            ArrayIndex2; //dunno why
	int            SideIndex;
	int            ColorSchemeIndex;
	PROTECTED_PROPERTY(DWORD, align_C4);

	//are these unused TS leftovers?
	double         FirepowerMult;
	double         GroundspeedMult;
	double         AirspeedMult;
	double         ArmorMult;
	double         ROFMult;
	double         CostMult;
	double         BuildtimeMult;
	//---

	float          ArmorInfantryMult;
	float          ArmorUnitsMult;
	float          ArmorAircraftMult;
	float          ArmorBuildingsMult;
	float          ArmorDefensesMult;

	float          CostInfantryMult;
	float          CostUnitsMult;
	float          CostAircraftMult;
	float          CostBuildingsMult;
	float          CostDefensesMult;

	float          SpeedInfantryMult;
	float          SpeedUnitsMult;
	float          SpeedAircraftMult;

	float          BuildtimeInfantryMult;
	float          BuildtimeUnitsMult;
	float          BuildtimeAircraftMult;
	float          BuildtimeBuildingsMult;
	float          BuildtimeDefensesMult;

	float          IncomeMult;

	TypeList<InfantryTypeClass*> VeteranInfantry;
	TypeList<UnitTypeClass*> VeteranUnits;
	TypeList<AircraftTypeClass*> VeteranAircraft;

	char Suffix [4];

	char           Prefix;
	bool           Multiplay;
	bool           MultiplayPassive;
	bool           WallOwner;
	bool           SmartAI; //"smart"?
	PROTECTED_PROPERTY(BYTE, padding_1A9[7]);
};
