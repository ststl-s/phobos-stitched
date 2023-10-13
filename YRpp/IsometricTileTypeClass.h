#pragma once

#include <ObjectTypeClass.h>

class NOVTABLE IsometricTileTypeClass : public ObjectTypeClass
{
public:
	static const AbstractType AbsID = AbstractType::IsotileType;

	//Array
	static constexpr constant_ptr<DynamicVectorClass<IsometricTileTypeClass*>, 0xA8ED28u> const Array{};

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override JMP_STD(0x549D90);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x549C80);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override JMP_STD(0x549D70);

	//Destructor
	virtual ~IsometricTileTypeClass() override RX;// JMP_THIS(0x54A170);

	//AbstractClass
	virtual void PointerExpired(AbstractClass* pAbstract, bool removed) override JMP_THIS(0x549DD0);
	virtual AbstractType WhatAmI() const override { return AbstractType::IsotileType; }
	virtual int Size() const override { return 0x30C; }
	virtual void ComputeCRC(CRCEngine& crc) const override JMP_THIS(0x549B70);

	virtual int GetArrayIndex() const override { return this->ArrayIndex; }

	//ObjectTypeClass
	virtual CoordStruct* vt_entry_6C(CoordStruct* pDest, CoordStruct* pSrc) const override { *pDest = *pSrc; return pDest; }
	virtual bool SpawnAtMapCoords(CellStruct* pMapCoords, HouseClass* pOwner) override JMP_THIS(0x549AA0);
	virtual ObjectClass* CreateObject(HouseClass* pOwner) override JMP_THIS(0x549AE0);
	virtual CellStruct* GetFoundationData(bool IncludeBib) const override JMP_THIS(0x544D30); //what the hell? I don't konw why IDA say this is not a founction
	virtual SHPStruct* GetImage() const override JMP_THIS(0x544CB0);

	//non-virtual
	int sub_544BE0(int a2) JMP_THIS(0x544BE0);
	int sub_544C20(int a2, char a3) JMP_THIS(0x544C20);
	BOOL sub_544C80() JMP_THIS(0x544C80);
	void sub_544E00(int a2) JMP_THIS(0x544E00);

	//static
	static short __fastcall FindIndex(const char* pID) JMP_STD(0x544CE0);
	static void* sub_544D4D(DWORD a1, DWORD a2) JMP_STD(0x544D4D); //IDA can't analysis this

	//Constructor
	IsometricTileTypeClass
	(
		int ArrayIndex,
		int Minus65,
		int Zero1,
		const char* pName,
		int Zero2
	) noexcept
		: IsometricTileTypeClass(noinit_t())
	{ JMP_THIS(0x5447C0); }

	IsometricTileTypeClass(IStream* pStm) noexcept
		: IsometricTileTypeClass(noinit_t())
	{ JMP_THIS(0x544A00); }

protected:
	explicit __forceinline IsometricTileTypeClass(noinit_t) noexcept
		: ObjectTypeClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:
	int ArrayIndex;
	int MarbleMadnessTile;
	int NonMarbleMadnessTile;
	DWORD unk_2A0;
	DynamicVectorClass<Color16Struct*> unk_2A4;
	DWORD unk_2BC;
	int ToSnowTheater;
	int ToTemperateTheater;
	int TileAnimIndex; //Tile%02dAnim, actually an AnimTypeClass array index...
	int TileXOffset; //Tile%02dXOffset
	int TileYOffset; //Tile%02dYOffset
	int TileAttachesTo; //Tile%02dAttachesTo, iso tile index?
	int TileZAdjust; //Tile%02dZAdjust
	DWORD unk_2DC; //0xBF
	bool Morphable;
	bool ShadowCaster;
	bool AllowToPlace; //default true
	bool RequiredByRMG;
	DWORD unk_2E4;
	DWORD unk_2E8;
	DWORD unk_2EC;
	int unk_2F0; //default 1, no idea
	bool unk_2F4; //like always true
	char FileName[0xE]; // WARNING! Westwood strncpy's 0xE bytes into this buffer without NULL terminating it.
	bool AllowBurrowing;
	bool AllowTiberium;
	PROTECTED_PROPERTY(BYTE, align_305[3]);
	DWORD unk_308;
};
