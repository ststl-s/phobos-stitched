/*
	Buildings
*/

#pragma once

#include <TechnoClass.h>
#include <BuildingTypeClass.h>
#include <BuildingLightClass.h>
#include <StageClass.h>

class FactoryClass;
class InfantryClass;
class LightSourceClass;
class FoggedObjectClass;

enum class BStateType : unsigned int
{
	Construction = 0x0,
	Idle = 0x1,
	Active = 0x2,
	Full = 0x3,
	Aux1 = 0x4,
	Aux2 = 0x5,
	Count = 0x6,
	None = 0xFFFFFFFF,
};

class NOVTABLE BuildingClass : public TechnoClass
{
public:
	static const AbstractType AbsID = AbstractType::Building;
	using type_class = BuildingTypeClass;

	//Static
	static constexpr constant_ptr<DynamicVectorClass<BuildingClass*>, 0xA8EB40u> const Array{};

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override JMP_STD(0x459E80);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x453E20);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override JMP_STD(0x454190);

	//Destructor
	virtual ~BuildingClass() override JMP_THIS(0x459F20);

	//AbstractClass
	virtual void Init() override JMP_THIS(0x442C40);
	virtual void PointerExpired(AbstractClass* pAbstract, bool removed) override JMP_THIS(0x44E8F0);
	virtual AbstractType WhatAmI() const override { return AbstractType::Building; }
	virtual int	Size() const override { return 0x720; }
	virtual void ComputeCRC(CRCEngine& crc) const override JMP_THIS(0x454260);
	virtual CoordStruct* GetCoords(CoordStruct* pCrd) const override JMP_THIS(0x447AC0);
	virtual CoordStruct* GetDestination(CoordStruct* pCrd, TechnoClass* pDocker = nullptr) const override JMP_THIS(0x447E90); // where this is moving, or a building's dock for a techno. iow, a rendez-vous point
	virtual void Update() override JMP_THIS(0x43FB20);

	//ObjectClass
	virtual VisualType VisualCharacter(VARIANT_BOOL specificOwner, HouseClass* whoIsAsking) const override JMP_THIS(0x4544A0);
	virtual SHPStruct* GetImage() const override JMP_THIS(0x4513D0);
	virtual Action MouseOverCell(CellStruct const* pCell, bool checkFog = false, bool ignoreForce = false) const override JMP_THIS(0x447540);
	virtual Action MouseOverObject(ObjectClass const* pObject, bool ignoreForce = false) const override JMP_THIS(0x447210);
	virtual bool IsStrange() const override JMP_THIS(0x457620);
	virtual ObjectTypeClass* GetType() const override JMP_THIS(0x459EE0);
	virtual const wchar_t* GetUIName() const override JMP_THIS(0x459ED0);
	virtual bool CanBeRepaired() const override JMP_THIS(0x452630);
	virtual bool CanBeSold() const override JMP_THIS(0x4494C0);
	// can the current player control this unit?
	virtual bool IsControllable() const override JMP_THIS(0x44F5C0);
	virtual CoordStruct* GetCenterCoordsAlt(CoordStruct* pCrd) const override JMP_THIS(0x4500A0);
	// gets a building's free dock coordinates for a unit. falls back to this->GetCoords(pCrd);
	virtual CoordStruct* GetDockCoords(CoordStruct* pCrd, TechnoClass* pDocker) const override JMP_THIS(0x447B20);
	virtual CoordStruct* GetRenderCoords(CoordStruct* pCrd) const override JMP_THIS(0x459EF0);
	virtual CoordStruct* GetFLH(CoordStruct* pDest, int weaponIdx, CoordStruct baseCoords) const override JMP_THIS(0x453840);
	virtual CoordStruct* GetExitCoords(CoordStruct* pCrd, DWORD dwUnk) const override JMP_THIS(0x44F640);
	virtual int GetYSort() const override JMP_THIS(0x449410);
	// remove object from the map
	virtual bool Limbo() override JMP_THIS(0x445880);
	// place the object on the map
	virtual bool Unlimbo(const CoordStruct& coords, DirType dFaceDir) override JMP_THIS(0x440580);
	// cleanup things (lose line trail, deselect, etc). Permanently: destroyed/removed/gone opposed to just going out of sight.
	virtual void Disappear(bool permanently) override JMP_THIS(0x44EBF0);
	virtual void MarkAllOccupationBits(const CoordStruct& coords) override JMP_THIS(0x453D60);
	virtual void UnmarkAllOccupationBits(const CoordStruct& coords) override JMP_THIS(0x453DC0);
	virtual KickOutResult KickOutUnit(TechnoClass* pTechno, CellStruct cell) override JMP_THIS(0x443C60);
	virtual bool DrawIfVisible(RectangleStruct* pBounds, bool evenIfCloaked, DWORD dwUnk3) const override JMP_THIS(0x43CEA0);
	virtual void DrawIt(Point2D* pLocation, RectangleStruct* pBounds) const override JMP_THIS(0x43D290);
	virtual void DrawAgain(const Point2D& location, const RectangleStruct& bounds) const override JMP_THIS(0x43D030);
	virtual bool UpdatePlacement(PlacementType value) override JMP_THIS(0x43F180);
	virtual RectangleStruct* GetRenderDimensions(RectangleStruct* pRect) override JMP_THIS(0x455C20);
	virtual void DrawRadialIndicator(DWORD dwUnk) override JMP_THIS(0x456750);
	virtual bool CanBeSelectedNow() const override JMP_THIS(0x459C00);
	virtual bool CellClickedAction(Action action, CellStruct* pCell, CellStruct* pCell1, bool bUnk) override JMP_THIS(0x4436F0);
	virtual bool ObjectClickedAction(Action action, ObjectClass* pTarget, bool bUnk) override JMP_THIS(0x443410);
	virtual void Flash(int Duration) override JMP_THIS(0x456E00);
	virtual DamageState IronCurtain(int duration, HouseClass* pSource, bool forceShield) override JMP_THIS(0x457C90);
	virtual DamageState ReceiveDamage(
		int* pDamage,
		int distanceFromEpicenter,
		WarheadTypeClass* pWH,
		ObjectClass* pAttacker,
		bool ignoreDefenses,
		bool preventPassengerEscape,
		HouseClass* pAttackingHouse) override JMP_THIS(0x442230);
	virtual RadioCommand ReceiveCommand(TechnoClass* pSender, RadioCommand command, AbstractClass*& pInOut) override JMP_THIS(0x43C2D0);
	virtual bool DiscoveredBy(HouseClass* pHouse) override JMP_THIS(0x44D5D0);
	virtual void SetRepairState(int state) override JMP_THIS(0x446FF0); // 0 - off, 1 - on, -1 - toggle
	virtual void Sell(DWORD dwUnk) override JMP_THIS(0x447110);
	virtual Move IsCellOccupied(CellClass* pDestCell, int facing, int level, CellClass* pSourceCell, bool alt) const override JMP_THIS(0x449440);

	//MissionClass
	virtual bool ReadyToNextMission() const override JMP_THIS(0x454250);
	virtual int Mission_Attack() override JMP_THIS(0x44ACF0);
	virtual int Mission_Capture() override JMP_THIS(0x44B760);	//jmp MissionClass::Mission_Capture { return 450; } 
	virtual int Mission_Guard() override JMP_THIS(0x4496B0);
	virtual int Mission_AreaGuard() override JMP_THIS(0x449A40);	// return this->Mission_Guard();
	virtual int Mission_Harvest() override JMP_THIS(0x44B770); //jmp MissionClass::Mission_Harvest { return 450; }
	virtual int Mission_Unload() override JMP_THIS(0x44D880);
	virtual int Mission_Construction() override JMP_THIS(0x449A50);
	virtual int Mission_Selling() override JMP_THIS(0x449C30);
	virtual int Mission_Repair() override JMP_THIS(0x44B780);
	virtual int Mission_Missile() override JMP_THIS(0x44C980);
	virtual int Mission_Open() override JMP_THIS(0x44E440);

	//TechnoClass
	virtual bool IsUnitFactory() const override JMP_THIS(0x455DA0);
	virtual bool ShouldBeCloaked() const override JMP_THIS(0x457770);
	virtual bool ShouldNotBeCloaked() const override JMP_THIS(0x4578C0);
	virtual DirStruct* TurretFacing(DirStruct* pBuffer) const override JMP_THIS(0x445E50);
	virtual bool IsArmed() const override JMP_THIS(0x458D80);
	virtual int GetPipFillLevel() const override JMP_THIS(0x44D700);
	virtual DWORD vt_entry_2C8(DWORD dwUnk1, DWORD dwUnk2) override JMP_THIS(0x43E940);
	virtual int GetCrewCount() const override JMP_THIS(0x451330);
	virtual int GetAntiAirValue() const override JMP_THIS(0x459870);
	virtual int GetAntiArmorValue() const override JMP_THIS(0x459880);
	virtual int GetAntiInfantryValue() const override JMP_THIS(0x459890);
	virtual void GotHijacked() override JMP_THIS(0x4576F0);
	virtual int GetZAdjustment() const override JMP_THIS(0x43E900);
	virtual CoordStruct* vt_entry_300(CoordStruct* pBuffer, DWORD dwUnk) const override JMP_THIS(0x453A70);
	virtual DirStruct* GetRealFacing(DirStruct* pBuffer) const override JMP_THIS(0x44D7D0);
	virtual InfantryTypeClass* GetCrew() const override JMP_THIS(0x44EB10);
	virtual bool IsRadarVisible(int* pOutDetection) const override JMP_THIS(0x457020); // out value will be set to 1 if unit is cloaked and 2 if it is subterranean, otherwise it's unchanged
	virtual bool IsPowerOnline() const override JMP_THIS(0x4555D0);
	virtual int VoiceDeploy() override JMP_THIS(0x459C20);
	virtual void Destroyed(ObjectClass* pKiller) override JMP_THIS(0x44D760);
	virtual FireError GetFireError(AbstractClass* pTarget, int nWeaponIndex, bool ignoreRange) const override JMP_THIS(0x447F10);
	virtual CellClass* SelectAutoTarget(TargetFlags targetFlags, int currentThreat, bool onlyTargetHouseEnemy) override JMP_THIS(0x445F00);
	virtual void SetTarget(AbstractClass* pTarget) override JMP_THIS(0x443B90);
	virtual bool SetOwningHouse(HouseClass* pHouse, bool announce = true) override JMP_THIS(0x448260);
	virtual WeaponStruct* GetWeapon(int nWeaponIndex) const override JMP_THIS(0x4526F0);
	virtual bool HasTurret() const override JMP_THIS(0x4527D0);
	virtual bool CanOccupyFire() const override JMP_THIS(0x458DD0);
	virtual int GetOccupyRangeBonus() const override JMP_THIS(0x458E00);
	virtual int GetOccupantCount() const override JMP_THIS(0x4581F0);
	virtual void UpdateCloak(bool bUnk = 1) override JMP_THIS(0x454DB0);
	virtual DWORD vt_entry_464(DWORD dwUnk) const override JMP_THIS(0x456F80);
	virtual void UpdateRefinerySmokeSystems() override JMP_THIS(0x459900);
	virtual void SetDestination(AbstractClass* pDest, bool bUnk) override JMP_THIS(0x455D50);
	virtual bool vt_entry_484(DWORD dwUnk1, DWORD dwUnk2) override JMP_THIS(0x44D6A0);
	virtual bool ForceCreate(CoordStruct& coord, DWORD dwUnk = 0) override JMP_THIS(0x458A80);
	virtual void RadarTrackingStart() override JMP_THIS(0x456580);
	virtual void RadarTrackingStop() override JMP_THIS(0x4565E0);
	virtual void RadarTrackingFlash() override JMP_THIS(0x456640);

	//BuildingClass
	virtual CellStruct FindExitCell(DWORD dwUnk, DWORD dwUnk2) const JMP_THIS(0x44EFB0);
	virtual int DistanceToDockingCoord(ObjectClass* pObj) const JMP_THIS(0x447E00);
	virtual void Place(bool captured) JMP_THIS(0x445F80);
	virtual void UpdateConstructionOptions() JMP_THIS(0x4456D0);
	virtual void Draw(const Point2D& point, const RectangleStruct& rect) JMP_THIS(0x43DA80);
	virtual DirStruct FireAngleTo(ObjectClass* pObject) const JMP_THIS(0x43ED40);
	virtual void Destory(DWORD dwUnused, TechnoClass* pTechno, bool NoSurvivor, CellStruct& cell) JMP_THIS(0x4415F0);
	virtual bool TogglePrimaryFactory() JMP_THIS(0x448160);
	virtual void SensorArrayActivate(CellStruct cell = CellStruct::Empty) JMP_THIS(0x455820);
	virtual void SensorArrayDeactivate(CellStruct cell = CellStruct::Empty) JMP_THIS(0x4556D0);
	virtual void DisguiseDetectorActivate(CellStruct cell = CellStruct::Empty) JMP_THIS(0x455A80);
	virtual void DisguiseDetectorDeactivate(CellStruct cell = CellStruct::Empty) JMP_THIS(0x455980);
	virtual int AlwaysZero() JMP_THIS(0x452250);

	// non-vt

	int GetCurrentFrame()
	{ JMP_THIS(0x43EF90); }

	void UpdateAnim_Powered()
	{ JMP_THIS(0x4549B0); }

	bool IsAllFogged() const
	{ JMP_THIS(0x457A10); }

	void SetRallypoint(CellStruct* pTarget, bool bPlayEVA)
	{ JMP_THIS(0x443860); }

	void FreezeInFog(DynamicVectorClass<FoggedObjectClass*>* pFoggedArray, CellClass* pCell, bool isVisible)
	{ JMP_THIS(0x457AA0); }

	// same as this->IsAllFogged
	bool CheckFog()
	{ JMP_THIS(0x457A10); }

	// power up
	void GoOnline()
	{ JMP_THIS(0x452260); }

	void GoOffline()
	{ JMP_THIS(0x452360); }

	int GetPowerOutput() const
	{ JMP_THIS(0x44E7B0); }

	int GetPowerDrain() const
	{ JMP_THIS(0x44E880); }

	// Firewall aka FirestormWall
	// depending on what facings of this building
	// are connected to another FWall,
	// returns the index of the image file
	// to draw.
	DWORD GetFWFlags() const
	{ JMP_THIS(0x455B90); }

	void CreateEndPost(bool arg)
	{ JMP_THIS(0x4533A0); }

	// kick out content
	void UnloadBunker()
	{ JMP_THIS(0x4593A0); }

	// content is dead - chronosphered away or died inside
	void ClearBunker()
	{ JMP_THIS(0x459470); }

	// kick out content, remove anims, etc... don't ask me what's different from kick out
	void EmptyBunker()
	{ JMP_THIS(0x4595C0); }

	bool UpdateBunker()
	{ JMP_THIS(0x458E50); }

	// called after destruction - CrateBeneath, resetting foundation'ed cells
	void AfterDestruction()
	{ JMP_THIS(0x441F60); }

	// destroys the specific animation (active, turret, special, etc)
	void DestroyNthAnim(BuildingAnimSlot slot)
	{ JMP_THIS(0x451E40); }

	void PlayNthAnim(BuildingAnimSlot slot, int effectDelay = 0)
	{
		bool Damaged = !this->IsGreenHP();
		bool Garrisoned = this->GetOccupantCount() > 0;

		auto& AnimData = this->Type->GetBuildingAnim(slot);
		const char* AnimName = nullptr;
		if (Damaged)
			AnimName = AnimData.Damaged;
		else if (Garrisoned)
			AnimName = AnimData.Garrisoned;
		else
			AnimName = AnimData.Anim;

		if (AnimName && *AnimName)
			this->PlayAnim(AnimName, slot, Damaged, Garrisoned, effectDelay);
	}

	void PlayAnim(const char* animName, BuildingAnimSlot slot, bool damaged, bool garrisoned, int effectDelay = 0)
	{ JMP_THIS(0x451890); }

	// changes between building's damaged and undamaged animations.
	void ToggleDamagedAnims(bool damaged)
	{ JMP_THIS(0x451EE0); }

	// when the building is switched off
	void DisableStuff()
	{ JMP_THIS(0x452480); }

	// when the building is switched on
	void EnableStuff()
	{ JMP_THIS(0x452410); }

	// when the building is warped
	void DisableTemporal()
	{ JMP_THIS(0x4521C0); }

	// when the building warped back in
	void EnableTemporal()
	{ JMP_THIS(0x452210); }

	// returns Type->SuperWeapon, if its AuxBuilding is satisfied
	int FirstActiveSWIdx() const
	{ JMP_THIS(0x457630); }

	// returns Type->SuperWeapon2, if its AuxBuilding is satisfied
	int SecondActiveSWIdx() const
	{ JMP_THIS(0x457690); }

	// same as this->GetCurrentFrame
	int GetShapeNumber() const
	{ JMP_THIS(0x43EF90); }

	void BeginMode(BStateType bType)
	{ JMP_THIS(0x447780); }

	void FireLaser(CoordStruct coords)
	{ JMP_THIS(0x44ABD0); }

	bool IsBeingDrained() const
	{ JMP_THIS(0x70FEC0); }

	void KillOccupants(TechnoClass* pAssaulter)
	{ JMP_THIS(0x4585C0); }

	// returns false if this is a gate that needs time to open, true otherwise
	bool MakeTraversable()
	{ JMP_THIS(0x452540); }

	// returns false if this is a gate that is closed, true otherwise
	bool IsTraversable() const
	{ JMP_THIS(0x4525F0); }

	Matrix3D* GetVoxelBarrelOffsetMatrix(Matrix3D& ret)
	{ JMP_THIS(0x458810); }

	// helpers
	bool HasSuperWeapon(int index) const
	{
		if (this->Type->HasSuperWeapon(index))
			return true;

		for (auto pType : this->Upgrades)
		{
			if (pType && pType->HasSuperWeapon(index))
				return true;
		}

		return false;
	}

	CoordStruct GetRenderCoords() const
	{
		CoordStruct buffer = CoordStruct::Empty;
		return *this->GetRenderCoords(&buffer);
	}

	CoordStruct GetFLH(int weaponIdx, const CoordStruct& baseCoords) const
	{
		CoordStruct buffer = CoordStruct::Empty;
		return *this->GetFLH(&buffer, weaponIdx, baseCoords);
	}

	TechnoTypeClass* GetSecretProduction() const;

	AnimClass*& GetAnim(BuildingAnimSlot slot)
	{
		return this->Anims[static_cast<int>(slot)];
	}

	AnimClass* const& GetAnim(BuildingAnimSlot slot) const
	{
		return this->Anims[static_cast<int>(slot)];
	}

	bool& GetAnimState(BuildingAnimSlot slot)
	{
		return this->AnimStates[static_cast<int>(slot)];
	}

	bool const& GetAnimState(BuildingAnimSlot slot) const
	{
		return this->AnimStates[static_cast<int>(slot)];
	}

	CoordStruct GetCoords() const
	{
		CoordStruct crd = CoordStruct::Empty;
		GetCoords(&crd);
		return crd;
	}

	//Constructor
	BuildingClass(BuildingTypeClass* pType, HouseClass* pOwner) noexcept
		: BuildingClass(noinit_t())
	{ JMP_THIS(0x43B740); }

protected:
	explicit __forceinline BuildingClass(noinit_t) noexcept
		: TechnoClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	BuildingTypeClass* Type;
	FactoryClass* Factory;
	CDTimerClass C4Timer;
	int BState;
	int QueueBState;
	DWORD OwnerCountryIndex;
	InfantryClass* C4AppliedBy;
	DWORD unknown_544;
	AnimClass* FirestormAnim; //pointer
	AnimClass* PsiWarnAnim; //pointer
	CDTimerClass unknown_timer_550;

// see eBuildingAnims above for slot index meanings
	AnimClass * Anims [0x15];
	bool AnimStates [0x15]; // one flag for each of the above anims (whether the anim was enabled when power went offline?)
	PROTECTED_PROPERTY(BYTE, align_5C5[3]);

	AnimClass * DamageFireAnims [0x8];

	bool RequiresDamageFires; // if set, ::Update spawns damage fire anims and zeroes it
	//5E8 - 5F8 ????????
	BuildingTypeClass * Upgrades [0x3];

	int FiringSWType; // type # of sw being launched
	DWORD unknown_5FC;
	BuildingLightClass* Spotlight;
	RateTimer GateTimer;
	LightSourceClass * LightSource; // tiled light , LightIntensity > 0
	DWORD LaserFenceFrame; // 0-7 for active directionals, 8/12 for offline ones, check ntfnce.shp or whatever
	DWORD FirestormWallFrame; // anim data for firestorm active animations
	StageClass RepairProgress; // for hospital, armory, unitrepair etc
	RectangleStruct unknown_rect_63C;
	CoordStruct unknown_coord_64C;
	int unknown_int_658;
	DWORD unknown_65C;
	bool HasPower;
	bool IsOverpowered;

	// each powered unit controller building gets this set on power activation and unset on power outage
	bool RegisteredAsPoweredUnitSource;

	DWORD SupportingPrisms;
	bool HasExtraPowerBonus;
	bool HasExtraPowerDrain;
	DynamicVectorClass<InfantryClass*> Overpowerers;
	DynamicVectorClass<InfantryClass*> Occupants;
	int FiringOccupantIndex; // which occupant should get XP, which weapon should be fired (see 6FF074)

	AudioController Audio7;
	AudioController Audio8;

	bool WasOnline; // the the last state when Update()ing. if this changed since the last Update(), UpdatePowered is called.
	bool ShowRealName;
	bool BeingProduced;
	bool ShouldRebuild;
	bool HasEngineer; // used to pass the NeedsEngineer check
	CDTimerClass CashProductionTimer;
	bool unknown_bool_6DC;
	bool IsReadyToCommence;
	bool NeedsRepairs; // AI handholder for repair logic,
	bool C4Applied;
	bool NoCrew;
	bool unknown_bool_6E1;
	bool unknown_bool_6E2;
	bool HasBeenCaptured; // has this building changed ownership at least once? affects crew and repair.
	bool ActuallyPlacedOnMap;
	bool unknown_bool_6E5;
	bool IsDamaged; // AI handholder for repair logic,
	bool IsFogged;
	bool IsBeingRepaired; // show animooted repair wrench
	bool HasBuildUp;
	bool StuffEnabled; // status set by EnableStuff() and DisableStuff()
	char HasCloakingData; // some fugly buffers
	byte CloakRadius; // from Type->CloakRadiusInCells
	char Translucency;
	DWORD StorageFilledSlots; // the old "silo needed" logic
	TechnoTypeClass * SecretProduction; // randomly assigned secret lab bonus, used if SecretInfantry, SecretUnit, and SecretBuilding are null
	ColorStruct ColorAdd;
	int unknown_int_6FC;
	short unknown_short_700;
	BYTE UpgradeLevel; // as defined by Type->UpgradesToLevel=
	char GateStage;
	PrismChargeState PrismStage;
	CoordStruct PrismTargetCoords;
	DWORD DelayBeforeFiring;

	int BunkerState; // used in UpdateBunker and friends
};
