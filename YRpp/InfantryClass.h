/*
	Infantry
*/

#pragma once

#include <FootClass.h>
#include <InfantryTypeClass.h>

class NOVTABLE InfantryClass : public FootClass
{
public:
	static constexpr uintptr_t VirtualTableAddress = 0x7EB058u;
	static const AbstractType AbsID = AbstractType::Infantry;
	using type_class = InfantryTypeClass;

	//Static
	static constexpr constant_ptr<DynamicVectorClass<InfantryClass*>, 0xA83DE8u> const Array{};

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override JMP_STD(0x523300);
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x521960);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override JMP_STD(0x521B00);

	//Destructor
	virtual ~InfantryClass() override RX;// JMP_THIS(0x523350);

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
	virtual void DrawIt(Point2D* pLocation, RectangleStruct* pBounds) const override JMP_THIS(0x518F90);
	virtual bool CellClickedAction(Action action, CellStruct* pCell1, CellStruct* pCell2, bool bUnk) override JMP_THIS(0x51F250);
	virtual bool ObjectClickedAction(Action action, ObjectClass* pTarget, bool bUnk) override JMP_THIS(0x51F190);
	virtual DamageState ReceiveDamage(
		int* pDamage,
		int distanceFromEpicenter,
		WarheadTypeClass* pWH,
		ObjectClass* pAttacker,
		bool ignoreDefenses,
		bool preventPassengerEscape,
		HouseClass* pAttackingHouse) override JMP_THIS(0x517FA0);
	virtual void Scatter(const CoordStruct& coords, bool ignoreMission, bool ignoreDestination) override JMP_THIS(0x51D0D0);
	virtual void UpdatePosition(int dwUnk) override JMP_THIS(0x519630);
	virtual Move IsCellOccupied(CellClass* pDestCell, int facing, int level, CellClass* pSourceCell, bool alt) const override JMP_THIS(0x51BF90);

	//MissionClass
	virtual bool ReadyToNextMission() const override JMP_THIS(0x521B60);
	virtual int Mission_Attack() override JMP_THIS(0x51F3E0);
	virtual int Mission_Guard() override JMP_THIS(0x51F620);
	virtual int Mission_AreaGuard() override JMP_THIS(0x51F640);
	virtual int Mission_Harvest() override JMP_THIS(0x512E70);
	virtual int Mission_Hunt() override JMP_THIS(0x51F540);
	virtual int Mission_Move() override JMP_THIS(0x51F660);

	//TechnoClass
	virtual int SelectWeapon(AbstractClass* pTarget) const override JMP_THIS(0x5218E0);
	virtual CoordStruct* vt_entry_300(CoordStruct* pBuffer, DWORD dwUnk) const override JMP_THIS(0x521D30);	//return this->GetFLH(pBuffer,dwUnk,CoordStruct::Empty)
	virtual bool IsEngineer() const override JMP_THIS(0x5224D0);
	virtual bool CanCheer() const override JMP_THIS(0x522BC0);
	virtual void Cheer(bool force) override JMP_THIS(0x522C00);
	virtual FireError GetFireError(AbstractClass* pTarget, int weaponIdx, bool considerRange) const override JMP_THIS(0x51C8B0);
	virtual AbstractClass* SelectAutoTarget(TargetFlags targetFlags, int currentThreat, bool onlyTargetHouseEnemy) override JMP_THIS(0x51E140);
	virtual void SetTarget(AbstractClass* pTarget) override JMP_THIS(0x51B1F0);
	virtual BulletClass* Fire(AbstractClass* pTarget, int weaponIdx) override JMP_THIS(0x51DF60);
	virtual void vt_entry_428() override JMP_THIS(0x51F330);
	//Only infantry....
	virtual void DisguiseAs(AbstractClass* pTarget) override JMP_THIS(0x522700);
	virtual void ClearDisguise() override JMP_THIS(0x522780);
	virtual bool IsItTimeForIdleActionYet() const override JMP_THIS(0x5216D0);
	virtual bool UpdateIdleAction() override JMP_THIS(0x51CDB0);
	virtual void SetDestination(AbstractClass* pDest, bool bUnk) override JMP_THIS(0x51AA40);
	virtual bool EnterIdleMode(DWORD dwUnk1, DWORD dwUnk2) JMP_THIS(0x709A40);
	virtual bool CanUseWaypoint() const JMP_THIS(0x5228B0);
	virtual bool CanAttackOnTheMove() const JMP_THIS(0x5228C0);

	//FootClass
	virtual void vt_entry_4F4() override JMP_THIS(0x521DD0);
	virtual bool vt_entry_4F8() { return false; }
	virtual bool MoveTo(CoordStruct* pCrd) JMP_THIS(0x4D55F0);
	virtual bool StopMoving() JMP_THIS(0x4D55C0);
	virtual bool ChronoWarpTo(CoordStruct coords) override JMP_THIS(0x522FE0);
	virtual void GoBerzerk() override JMP_THIS(0x5220F0);
	virtual void Panic() override JMP_THIS(0x521C10);
	virtual void UnPanic() override JMP_THIS(0x521C40);
	virtual void PlayIdleAnim(int idleAnimNumber) override JMP_THIS(0x521C60);
	virtual int GetCurrentSpeed() const override JMP_THIS(0x521D80);
	virtual DWORD vt_entry_53C(DWORD dwUnk) JMP_THIS(0x522340);
	virtual void vt_entry_548() override JMP_THIS(0x521B20);
	virtual void vt_entry_54C() override JMP_THIS(0x521B40);

	//InfantryClass
	virtual bool IsDeployed() const JMP_THIS(0x5228D0);
	virtual bool PlayAnim(Sequence index, bool force = false, bool randomStartFrame = false) JMP_THIS(0x51D6F0);

	//INoticeSink
	virtual bool INoticeSink_Unknown(DWORD dwUnknown) override JMP_THIS(0x522A60);

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

static_assert(sizeof(InfantryClass) == 0x6F0);
