/*
	LightSource - used for light posts and radiation
*/

#pragma once

#include <AbstractClass.h>

class NOVTABLE LightSourceClass : public AbstractClass
{
public:
	static const AbstractType AbsID = AbstractType::LightSource;
	
	//Array
	static constexpr reference<DynamicVectorClass<LightSourceClass*>, 0xABCA10u> const Array {};

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) JMP_STD(0x555080);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) JMP_STD(0x5550C0);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) JMP_STD(0x555110);

	//Destructor
	virtual ~LightSourceClass() JMP_THIS(0x555150);

	//AbstractClass
	virtual AbstractType WhatAmI() const { return AbstractType::LightSource; }
	virtual int Size() const { return 0x4C; }
	virtual void CalculateChecksum(Checksummer& checksum) const { this->AbstractClass::CalculateChecksum(checksum); }

	//non-virtual
	void Activate(DWORD dwZero = 0)	//Start lighting
		{ JMP_THIS(0x554A60); }

	void Deactivate(DWORD dwZero = 0)	//Stop lighting
		{ JMP_THIS(0x554A80); }

	void ChangeLevels(int nIntensity, TintStruct Tint, char mode)
		{ JMP_THIS(0x554AA0); }

	static void __fastcall UpdateLightConverts(int value)
		{ JMP_STD(0x554D50); }

	//Constructor
	LightSourceClass(
		int X, int Y, int Z, int nVisibility, int nIntensity, int Red, int Green, int Blue) noexcept
		: LightSourceClass(noinit_t())
	{ JMP_THIS(0x554760); }

	LightSourceClass(
		CoordStruct Crd, int nVisibility, int nIntensity, TintStruct Tint) noexcept
		: LightSourceClass(noinit_t())
	{ JMP_THIS(0x554760); }

protected:
	explicit __forceinline LightSourceClass(noinit_t) noexcept
		: AbstractClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	int LightIntensity;
	TintStruct LightTint;
	int DetailLevel;
	CoordStruct Location;
	int LightVisibility;
	bool Activated;
};
