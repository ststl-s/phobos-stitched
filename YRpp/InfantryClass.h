/*
	Infantry
*/

#pragma once

#include <FootClass.h>
#include <InfantryTypeClass.h>

class NOVTABLE InfantryClass : public FootClass
{
public:
	static const AbstractType AbsID = AbstractType::Infantry;
	using type_class = InfantryTypeClass;

	//Static
	static constexpr constant_ptr<DynamicVectorClass<InfantryClass*>, 0xA83DE8u> const Array{};

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override JMP_STD(0x523300);
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x521960);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override JMP_STD(0x521B00);

	//Destructor
	virtual ~InfantryClass() override JMP_THIS(0x523350);

	//AbstractClass
	virtual void PointerExpired(AbstractClass* pAbstract, bool removed) override JMP_THIS(0x51AA10);
	virtual AbstractType WhatAmI() const override { return AbstractType::Infantry; }
	virtual int	Size() const override { return 0x6F0; }
	virtual void ComputeCRC(CRCEngine& crc) const override JMP_THIS(0x521C90);
	virtual void Update() override JMP_THIS(0x51BAB0);

	//ObjectClass
	virtual SHPStruct* GetImage() const override JMP_THIS(0x5216C0);
	virtual Action MouseOverCell(CellStruct const* pCell, bool checkFog = false, bool ignoreForce = false) const override JMP_THIS(0x51F800);
	virtual Action MouseOverObject(ObjectClass const* pObject, bool ignoreForce = false) const override JMP_THIS(0x51E3B0);
	virtual ObjectTypeClass* GetType() const override JMP_THIS(0x51FAF0);
	virtual const wchar_t* GetUIName() const override JMP_THIS(0x51F2C0);
	virtual CoordStruct* GetFLH(CoordStruct* pDest, int idxWeapon, CoordStruct baseCoords) const override JMP_THIS(0x523250);
	virtual bool IsDisguisedAs(HouseClass* target) const override JMP_THIS(0x5227F0); // only works correctly on infantry!
	virtual ObjectTypeClass* GetDisguise(bool disguisedAgainstAllies) const override JMP_THIS(0x522640);
	virtual HouseClass* GetDisguiseHouse(bool disguisedAgainstAllies) const override JMP_THIS(0x5226C0);
	virtual bool Limbo() override JMP_THIS(0x51DF10);
	virtual bool Unlimbo(const CoordStruct& coords, DirType dFaceDir) override JMP_THIS(0x51DFF0);
	virtual bool SpawnParachuted(const CoordStruct& coords) override JMP_THIS(0x521760);
	virtual void MarkAllOccupationBits(const CoordStruct& coords) override JMP_THIS(0x5217C0);
	virtual void UnmarkAllOccupationBits(const CoordStruct& coords) override JMP_THIS(0x521850);
	virtual void Draw(Point2D* pLocation, RectangleStruct* pBounds) const override JMP_THIS(0x518F90);

	//InfantryClass
	virtual bool IsDeployed() const R0;
	virtual bool PlayAnim(Sequence index, bool force = false, bool randomStartFrame = false) R0;

	//Constructor
	InfantryClass(InfantryTypeClass* pType, HouseClass* pOwner) noexcept
		: InfantryClass(noinit_t())
	{ JMP_THIS(0x517A50); }

protected:
	explicit __forceinline InfantryClass(noinit_t) noexcept
		: FootClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	InfantryTypeClass* Type;
	Sequence SequenceAnim; //which is currently playing
	CDTimerClass unknown_Timer_6C8;
	DWORD          PanicDurationLeft; // set in ReceiveDamage on panicky units
	bool           PermanentBerzerk; // set by script action, not cleared anywhere
	bool           Technician;
	bool           unknown_bool_6DA;
	bool           Crawling;
	bool           unknown_bool_6DC;
	bool           unknown_bool_6DD;
	DWORD          unknown_6E0;
	bool           ShouldDeploy;
	int            unknown_int_6E8;
	PROTECTED_PROPERTY(DWORD, unused_6EC); //??
};
