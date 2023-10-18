/*
	Aircraft
*/

#pragma once

#include <FootClass.h>
#include <AircraftTypeClass.h>

//AircraftClass
class NOVTABLE AircraftClass : public FootClass, public IFlyControl
{
public:
	static const AbstractType AbsID = AbstractType::Aircraft;
	using type_class = AircraftTypeClass;

	//Static
	static constexpr constant_ptr<DynamicVectorClass<AircraftClass*>, 0xA8E390u> const Array{};

	//IUnknown
	virtual HRESULT __stdcall QueryInterface(REFIID iid, void** ppvObject) override JMP_STD(0x414290);
	virtual ULONG __stdcall AddRef() override JMP_STD(0x4142F0);
	virtual ULONG __stdcall Release() override JMP_STD(0x414300);

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override JMP_STD(0x41C190);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x41B430);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override JMP_STD(0x41B5C0);

	//Destructor
	virtual ~AircraftClass() override RX;//JMP_THIS(0x41C210)

	//AbstractClass
	virtual void Init() override JMP_THIS(0x413F80);
	virtual void PointerExpired(AbstractClass* pAbstract, bool removed) override JMP_THIS(0x41B660);
	virtual AbstractType WhatAmI() const override JMP_THIS(0x41C180);
	virtual int	Size() const override JMP_THIS(0x41C170);
	virtual void ComputeCRC(CRCEngine& crc) const override JMP_THIS(0x41B610);
	virtual bool IsOnFloor() const override JMP_THIS(0x41B980);
	virtual bool IsInAir() const override JMP_THIS(0x41B920);
	virtual void Update() override JMP_THIS(0x414BB0);

	//ObjectClass
	virtual Action MouseOverCell(CellStruct const* pCell, bool checkFog = false, bool ignoreForce = false) const override JMP_THIS(0x417F80);
	virtual Action MouseOverObject(ObjectClass const* pObject, bool ignoreForce = false) const override JMP_THIS(0x417CC0);
	virtual Layer InWhichLayer() const override JMP_THIS(0x41ADC0);
	virtual bool IsStrange() const override JMP_THIS(0x41B910);
	virtual AircraftTypeClass* GetType() const override JMP_THIS(0x41C200);
	virtual const wchar_t* GetUIName() const override JMP_THIS(0x41C1D0);
	virtual bool Unlimbo(const CoordStruct& coords, DirType faceDir) override JMP_THIS(0x414310);
	virtual KickOutResult KickOutUnit(TechnoClass* pTechno, CellStruct cell) override JMP_THIS(0x415B10);
	virtual void DrawIt(Point2D* pLocation, RectangleStruct* pBounds) const override JMP_THIS(0x4144B0);
	virtual void See(DWORD dwUnk1, DWORD dwUnk2) override JMP_THIS(0x41ADF0);
	virtual bool CellClickedAction(Action action, CellStruct* pCell1, CellStruct* pCell2, bool bUnk) override JMP_THIS(0x417C80);
	virtual bool ObjectClickedAction(Action action, ObjectClass* pTarget, bool bUnk) override JMP_THIS(0x417BD0);
	virtual DamageState ReceiveDamage(
		int* pDamage,
		int distanceFromEpicenter,
		WarheadTypeClass* pWH,
		ObjectClass* pAttacker,
		bool ignoreDefenses,
		bool preventPassengerEscape,
		HouseClass* pAttackingHouse) override JMP_THIS(0x4165C0);
	virtual void Scatter(const CoordStruct& coords, bool ignoreMission, bool ignoreDestination) override JMP_THIS(0x41A590);
	virtual RadioCommand ReceiveCommand(TechnoClass* pSender, RadioCommand command, AbstractClass*& pInOut) override JMP_THIS(0x4190B0);
	virtual Move IsCellOccupied(CellClass* pDestCell, int facing, int level, CellClass* pSourceCell, bool alt) const override JMP_THIS(0x4196B0);

	//MissionClass
	virtual bool QueueMission(Mission mission, bool start_mission) override JMP_THIS(0x41BA90);
	virtual bool NextMission() override JMP_THIS(0x41B870);
	virtual void ForceMission(Mission mission) override JMP_THIS(0x41B9F0);
	virtual void Override_Mission(Mission mission, AbstractClass* pTarget, AbstractClass* pDestination) override JMP_THIS(0x41BB30);
	virtual bool ReadyToNextMission() const override JMP_THIS(0x41B5E0);
	virtual int Mission_Attack() override JMP_THIS(0x417FE0);
	virtual int Mission_Guard() override JMP_THIS(0x41A5C0);
	virtual int Mission_AreaGuard() override JMP_THIS(0x41A940);

	//IFlyControl
	virtual int __stdcall Landing_Altitude() R0;
	virtual int __stdcall Landing_Direction() R0;
	virtual long __stdcall Is_Loaded() R0;
	virtual long __stdcall Is_Strafe() R0;
	virtual long __stdcall Is_Fighter() R0;
	virtual long __stdcall Is_Locked() R0;

	//Constructor
	AircraftClass(AircraftTypeClass* pType, HouseClass* pOwner) noexcept
		: AircraftClass(noinit_t())
	{ JMP_THIS(0x413D20); }

protected:
	explicit __forceinline AircraftClass(noinit_t) noexcept
		: FootClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	AircraftTypeClass* Type;
	bool unknown_bool_6C8;
	bool HasPassengers;	//parachutes
	bool IsKamikaze; // when crashing down, duh
	BuildingClass* DockNowHeadingTo;
	bool unknown_bool_6D0;
	bool unknown_bool_6D1;
	bool unknown_bool_6D2;
	char unknown_char_6D3;
	bool unknown_bool_6D4;
	bool unknown_bool_6D5;
};
