/*
	Ground & Naval Vehicles
*/

#pragma once

#include <FootClass.h>
#include <UnitTypeClass.h>

//forward declarations
class EBolt;

class NOVTABLE UnitClass : public FootClass
{
public:
	static const AbstractType AbsID = AbstractType::Unit;
	using type_class = UnitTypeClass;

	//Static
	static constexpr constant_ptr<DynamicVectorClass<UnitClass*>, 0x8B4108u> const Array{};

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override JMP_STD(0x746DE0);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_THIS(0x744470);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override JMP_THIS(0x744600);

	//Destructor
	virtual ~UnitClass() override RX;// JMP_THIS(0x746E80);

	//AbstractClass
	virtual void Init() override JMP_THIS(0x746810);
	virtual void PointerExpired(AbstractClass* pAbstract, bool removed) override JMP_THIS(0x7446E0);
	virtual AbstractType WhatAmI() const override { return AbstractType::Unit; }
	virtual int Size() const override { return 0x8E8; }
	virtual void ComputeCRC(CRCEngine& crc) const override JMP_THIS(0x744640);
	virtual void Update() override JMP_THIS(0x7360C0);

	//ObjectClass
	virtual Action MouseOverCell(CellStruct const* pCell, bool checkFog = false, bool ignoreForce = false) const override JMP_THIS(0x7404B0);
	virtual Action MouseOverObject(ObjectClass const* pObject, bool ignoreForce = false) const override JMP_THIS(0x73FD50);
	virtual bool IsStrange() const override JMP_THIS(0x746400);
	virtual ObjectTypeClass* GetType() const override JMP_THIS(0x741490);
	virtual const wchar_t* GetUIName() const override JMP_THIS(0x746B20);
	virtual bool IsDisguisedAs(HouseClass* target) const override JMP_THIS(0x746750); // only works correctly on infantry!
	virtual ObjectTypeClass* GetDisguise(bool disguisedAgainstAllies) const override JMP_THIS(0x7465B0);
	virtual HouseClass* GetDisguiseHouse(bool disguisedAgainstAllies) const override JMP_THIS(0x7465F0);
	virtual bool Limbo() override JMP_THIS(0x7440B0);
	virtual bool Unlimbo(const CoordStruct& coords, DirType dFaceDir) override JMP_THIS(0x737BA0);
	virtual void RegisterDestruction(TechnoClass* pDestroyer) override JMP_THIS(0x744720);
	virtual void MarkAllOccupationBits(const CoordStruct& coords) override JMP_THIS(0x7441B0);
	virtual void UnmarkAllOccupationBits(const CoordStruct& coords) override JMP_THIS(0x744210);
	virtual bool DrawIfVisible(RectangleStruct* pBounds, bool evenIfCloaked, DWORD dwUnk3) const override JMP_THIS(0x73B0B0);
	virtual void DrawIt(Point2D* pLocation, RectangleStruct* pBounds) const override JMP_THIS(0x73CEC0);
	virtual bool CellClickedAction(Action action, CellStruct* pCell, CellStruct* pCell1, bool bUnk) override JMP_THIS(0x738910);
	virtual bool ObjectClickedAction(Action action, ObjectClass* pTarget, bool bUnk) override JMP_THIS(0x738890);
	virtual DamageState ReceiveDamage(
		int* pDamage,
		int distanceFromEpicenter,
		WarheadTypeClass* pWH,
		ObjectClass* pAttacker,
		bool ignoreDefenses,
		bool preventPassengerEscape,
		HouseClass* pAttackingHouse) override JMP_THIS(0x737C90);
	virtual void Destroy() override JMP_THIS(0x746D60);
	virtual void Scatter(const CoordStruct& crd, bool ignoreMission, bool ignoreDestination) override JMP_THIS(0x743A50);
	virtual void UpdatePosition(int dwUnk) override JMP_THIS(0x739EC0);
	virtual RadioCommand ReceiveCommand(TechnoClass* pSender, RadioCommand command, AbstractClass*& pInOut) override JMP_THIS(0x737430);
	virtual Move IsCellOccupied(CellClass* pDestCell, int facing, int level, CellClass* pSourceCell, bool alt) const override JMP_THIS(0x73F0A0);

	//MissionClass
	virtual bool ReadyToNextMission() const override JMP_THIS(0x744270);
	virtual int Mission_Attack() override JMP_THIS(0x7447A0);
	virtual int Mission_Guard() override JMP_THIS(0x740810);
	virtual int Mission_AreaGuard() override JMP_THIS(0x744100);
	virtual int Mission_Harvest() override JMP_THIS(0x73E5E0);
	virtual int Mission_Hunt() override JMP_THIS(0x73EFC0);
	virtual int Mission_Move() override JMP_THIS(0x740A90);
	virtual int Mission_Unload() override JMP_THIS(0x73D630);
	virtual int Mission_Repair() override JMP_THIS(0x740EF0);
	virtual int Mission_Patrol() override JMP_THIS(0x740B10);

	//TechnoClass
	virtual bool vt_entry_29C() override JMP_THIS(0x744180);
	virtual DirStruct* TurretFacing(DirStruct* pBuffer) const override JMP_THIS(0x746E30);
	virtual double GetStoragePercentage() const override JMP_THIS(0x7414A0);
	virtual int GetPipFillLevel() const override JMP_THIS(0x740E50);
	virtual int SelectWeapon(AbstractClass* pTarget) const override JMP_THIS(0x746CD0);
	virtual DWORD vt_entry_304(DWORD dwUnk1, DWORD dwUnk2) const override JMP_THIS(0x740B60);
	virtual DirStruct* GetRealFacing(DirStruct* pBuffer) const override JMP_THIS(0x740F80);
	virtual InfantryTypeClass* GetCrew() const override JMP_THIS(0x740EE0);
	virtual bool IsUnderEMP() const override JMP_THIS(0x746C90);
	virtual FireError GetFireError(AbstractClass* pTarget, int nWeaponIndex, bool ignoreRange) const override JMP_THIS(0x740FD0);
	virtual BulletClass* Fire(AbstractClass* pTarget, int nWeaponIndex) override JMP_THIS(0x741340);
	virtual bool SetOwningHouse(HouseClass* pHouse, bool announce = true) override JMP_THIS(0x7463A0);
	virtual void Reload() override JMP_THIS(0x736CA0);
	virtual void DisguiseAs(AbstractClass* pTarget) override JMP_THIS(0x746670);
	virtual void ClearDisguise() override JMP_THIS(0x746720);
	virtual void SetDestination(AbstractClass* pDest, bool bUnk) override JMP_THIS(0x741970)
	virtual bool EnterIdleMode(DWORD dwUnk1, DWORD dwUnk2) override JMP_THIS(0x738970);

	//FootClass
	virtual void ReceiveGunner(FootClass* pGunner) override JMP_THIS(0x746420);
	virtual void RemoveGunner(FootClass* pGunner) override JMP_THIS(0x7464E0);
	virtual bool CanDeployNow() const override JMP_THIS(0x736D50);
	virtual void vt_entry_534(DWORD dwUnk1, DWORD dwUnk2) override JMP_THIS(0x7416A0);
	virtual DWORD vt_entry_53C(DWORD dwUnk) override JMP_THIS(0x7414E0);

	//UnitClass
	//main drawing functions - DrawIt() calles one of these, they call parent's Draw_A_smth
	virtual void DrawAsVXL(Point2D coords, RectangleStruct boundingRect, int brightness, int tint) JMP_THIS(0x73B470);
	virtual void DrawAsSHP(Point2D coords, RectangleStruct boundingRect, int brightness, int tint) JMP_THIS(0x73C5F0);
	virtual void DrawObject(Surface* pSurface, Point2D coords, RectangleStruct cacheRect, int brightness, int tint) JMP_THIS(0x73B140);

	// non-virtual

	bool IsDeactivated() const
	{ JMP_THIS(0x70FBD0); }

	void UpdateTube() JMP_THIS(0x7359F0);
	void UpdateRotation() JMP_THIS(0x736990);
	void UpdateEdgeOfWorld() JMP_THIS(0x736C10); // inlined in game
	void UpdateFiring() JMP_THIS(0x736DF0);
	void UpdateVisceroid() JMP_THIS(0x737180);
	void UpdateDisguise() JMP_THIS(0x7468C0);

	void Explode() JMP_THIS(0x738680);

	bool GotoClearSpot() JMP_THIS(0x738D30);
	bool TryToDeploy() JMP_THIS(0x7393C0);
	void Deploy() JMP_THIS(0x739AC0);
	void Undeploy() JMP_THIS(0x739CD0);

	bool Harvesting() JMP_THIS(0x73D450);

	bool FlagAttach(int nHouseIdx) JMP_THIS(0x740DF0);
	bool FlagRemove() JMP_THIS(0x740E20);

	void APCCloseDoor() JMP_THIS(0x740E60); // inlined in game
	void APCOpenDoor() JMP_THIS(0x740E80); // inlined in game

	static void __fastcall ReadINI(CCINIClass* pINI) JMP_STD(0x743270);
	static void __fastcall WriteINI(CCINIClass* pINI) JMP_STD(0x7436E0);

	bool ShouldCrashIt(TechnoClass* pTarget) JMP_THIS(0x7438F0);

	AbstractClass* AssignDestination_7447B0(AbstractClass* pTarget) JMP_THIS(0x7447B0);
	bool AStarAttempt(const CellStruct& cell1, const CellStruct& cell2) JMP_THIS(0x746000);

	//helpers
	DirStruct GetRealFacing() const
	{
		DirStruct buffer;
		return *this->GetRealFacing(&buffer);
	}

	//Constructor
	UnitClass(UnitTypeClass* pType, HouseClass* pOwner) noexcept : UnitClass(noinit_t())
		{ JMP_THIS(0x7353C0); }

protected:
	explicit __forceinline UnitClass(noinit_t) noexcept
		: FootClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	int unknown_int_6C0;
	UnitTypeClass* Type;
	UnitClass* FollowerCar; // groovy - link defined in the map's [Units] section, looked up on startup
	int FlagHouseIndex; //Carrying the flag of this House
	bool HasFollowerCar; // yes, this vehicle has a another vehicle (a train's locomotive towing the cars)
	bool Unloading;
	bool IsHarvesting;
	bool TerrainPalette;
	int unknown_int_6D4;
	int DeathFrameCounter;
	EBolt* ElectricBolt; //Unit is the owner of this
	bool Deployed;
	bool Deploying;
	bool Undeploying;
	int NonPassengerCount; // Set when unloading passengers. Units with TurretCount>0 will not unload the gunner.

	wchar_t ToolTipText[0x100];
};
