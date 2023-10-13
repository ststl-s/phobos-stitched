/*
	Trees
*/

#pragma once

#include <ObjectClass.h>
#include <TerrainTypeClass.h>
#include <StageClass.h>

class NOVTABLE TerrainClass : public ObjectClass
{
public:
	static const AbstractType AbsID = AbstractType::Terrain;

	//global array
	static constexpr constant_ptr<DynamicVectorClass<TerrainClass*>, 0xA8E988u> const Array{};

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override JMP_STD(0x71D310);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x71CDA0)
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override JMP_STD(0x71CD30);

	//Destructor
	virtual ~TerrainClass() override RX;// JMP_THIS(0x71D350);

	//AbstractClass
	virtual void PointerExpired(AbstractClass* pAbstract, bool removed) override JMP_THIS(0x71CFD0);
	virtual AbstractType WhatAmI() const override JMP_THIS(0x71D300);
	virtual int Size() const override JMP_THIS(0x71D2F0);
	virtual void ComputeCRC(CRCEngine& crc) const override JMP_THIS(0x71CF50);
	virtual void Update() override JMP_THIS(0x71C730);

	//ObjectClass
	virtual TerrainTypeClass* GetType() const override JMP_THIS(0x71D150);
	virtual bool Limbo() override JMP_THIS(0x71C930);
	virtual bool Unlimbo(const CoordStruct& coords, DirType faceDir) override JMP_THIS(0x71D000);
	virtual void MarkAllOccupationBits(const CoordStruct& coords) override JMP_THIS(0x71C110);
	virtual void UnmarkAllOccupationBits(const CoordStruct& coords) JMP_THIS(0x71C070);
	virtual bool DrawIfVisible(RectangleStruct* pBounds, bool evenIfCloaked, DWORD dwUnk3) const override JMP_THIS(0x71CC50);
	virtual void DrawIt(Point2D* pLocation, RectangleStruct* pBounds) const override JMP_THIS(0x71C1B0);
	virtual void DrawAgain(const Point2D& location, const RectangleStruct& bounds) const override JMP_THIS(0x71C360); // just forwards the call to Draw
	virtual bool UpdatePlacement(PlacementType value) override JMP_THIS(0x71BFB0);
	virtual RectangleStruct* GetRenderDimensions(RectangleStruct* pRect) override JMP_THIS(0x71D160);
	virtual DamageState ReceiveDamage
	(
		int* pDamage,
		int distanceFromEpicenter,
		WarheadTypeClass* pWH,
		ObjectClass* pAttacker,
		bool ignoreDefenses,
		bool preventPassengerEscape,
		HouseClass* pAttackingHouse
	) override JMP_THIS(0x71B920);
	virtual bool Ignite() override JMP_THIS(0x71C5B0);
	virtual void Extinguish() override JMP_THIS(0x71C6B0);
	virtual Move IsCellOccupied(CellClass* pDestCell, int facing, int level, CellClass* pSourceCell, bool alt) const override JMP_THIS(0x71C4D0);

	//Constructor
	TerrainClass(TerrainTypeClass* pType, CellStruct cell) noexcept
		: TerrainClass(noinit_t())
	{ JMP_THIS(0x71BB90); }

protected:
	explicit __forceinline TerrainClass(noinit_t) noexcept
		: ObjectClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	StageClass Animation;
	TerrainTypeClass* Type;
	bool IsBurning; // this terrain object has been ignited
	bool TimeToDie; // finish the animation and uninit
	RectangleStruct unknown_rect_D0;

};
