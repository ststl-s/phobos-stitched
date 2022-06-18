/*
	Base class for units that can move (that have "feet")
*/

#pragma once

#include <TechnoClass.h>
#include <ParasiteClass.h>
#include <Helpers/ComPtr.h>

//forward declarations
class LocomotionClass;
class TeamClass;

class NOVTABLE FootClass : public TechnoClass
{
public:
	static const auto AbsDerivateID = AbstractFlags::Foot;

	static constexpr constant_ptr<DynamicVectorClass<FootClass*>, 0x8B3DC0u> const Array{};

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) JMP_STD(0x4DB3C0);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) JMP_STD(0x4DB690);

	//Destructor
	virtual ~FootClass() JMP_THIS(0x4E0170);

	//AbstractClass
	virtual void PointerExpired(AbstractClass* pAbstract, bool removed) JMP_THIS(0x4D9960);
	virtual void CalculateChecksum(Checksummer& checksum) const JMP_THIS(0x4DBAD0);
	virtual CoordStruct* GetDestination(CoordStruct* pCrd, TechnoClass* pDocker = nullptr) const R0; // where this is moving, or a building's dock for a techno. iow, a rendez-vous point
	virtual bool IsInAir() const { return this->ObjectClass::IsInAir(); }
	virtual void Update() JMP_THIS(0x4DA530);

	//ObjectClass
	virtual VisualType VisualCharacter(VARIANT_BOOL SpecificOwner, HouseClass* WhoIsAsking) const JMP_THIS(0x4DA4E0);
	virtual SHPStruct* GetImage() const JMP_THIS(0x4DED70);
	virtual Action MouseOverCell(CellStruct const* pCell, bool checkFog = false, bool ignoreForce = false) const JMP_THIS(0x4DDDE0);
	virtual Action MouseOverObject(ObjectClass const* pObject, bool ignoreForce = false) const JMP_THIS(0x4DDED0);
	virtual Layer InWhichLayer() const JMP_THIS(0x4DB7E0);
	virtual bool CanBeSold() const { return false; }
	virtual bool IsOnBridge(TechnoClass* pDocker = nullptr) const 
	{ 
		/*if (this->TubeIndex < 0) 
			return this->ObjectClass::IsOnBridge(pDocker);
		return false;*/
		JMP_THIS(0x4DDC40);
	} // pDocker is passed to GetDestination
	virtual bool IsStandingStill() const { return this->FrozenStill; }
	virtual bool Limbo() JMP_THIS(0x4DB260);
	// place the object on the map
	virtual bool Unlimbo(const CoordStruct& Crd, Direction::Value dFaceDir) JMP_THIS(0x4D7170);
	// cleanup things (lose line trail, deselect, etc). Permanently: destroyed/removed/gone opposed to just going out of sight.
	virtual void Disappear(bool permanently) JMP_THIS(0x4D9270);
	virtual void UnInit() JMP_THIS(0x4DE5D0);
	virtual void Draw(Point2D* pLocation, RectangleStruct* pBounds) const { }
	virtual bool UpdatePlacement(PlacementType value) JMP_THIS(0x4D3780);
	virtual bool CanBeSelected() const
	{
		/*if (this->IsAttackedByLocomotor)
			return false;
		return this->ObjectClass::CanBeSelected();
		*/
		JMP_THIS(0x4DFA50);
	}
	virtual bool CellClickedAction(Action action, CellStruct* pCell, CellStruct* pCell1, bool bUnk) JMP_THIS(0x4D7D50);
	virtual bool ObjectClickedAction(Action action, ObjectClass* pTarget, bool bUnk) JMP_THIS(0x4D74E0);
	virtual DamageState IronCurtain(int nDuration, HouseClass* pSource, bool ForceShield) JMP_THIS(0x4DEAE0);
	virtual DamageState ReceiveDamage(
		int* pDamage,
		int DistanceFromEpicenter,
		WarheadTypeClass* pWH,
		ObjectClass* Attacker,
		bool IgnoreDefenses,
		bool PreventPassengerEscape,
		HouseClass* pAttackingHouse) JMP_THIS(0x4D7330);
	virtual void UpdatePosition(int dwUnk) JMP_THIS(0x4D85D0);
	virtual RadioCommand ReceiveCommand(TechnoClass* pSender, RadioCommand command, AbstractClass*& pInOut) JMP_THIS(0x4D8FB0);
	virtual void Sell(DWORD dwUnk) JMP_THIS(0x4D9F70);
	virtual void AssignPlanningPath(signed int idxPath, signed char idxWP) JMP_THIS(0x4DC810);
	virtual Move IsCellOccupied(CellClass* pDestCell, int facing, int level, CellClass* pSourceCell, bool alt) const
	{
		/*if (!this->Locomotor.get() || !alt)
			return Move::OK;
		if (!this->Locomotor.get())
			Game::RaiseError(-2147467261);
		return this->Locomotor->Can_Enter_Cell(pDestCell->MapCoords);*/
		JMP_THIS(0x4D9C10)
	}
	virtual DWORD vt_entry_1B0(DWORD dwUnk1, DWORD dwUnk2, DWORD dwUnk3, DWORD dwUnk4, DWORD dwUnk5) JMP_THIS(0x4D9C60);
	virtual void SetLocation(const CoordStruct& crd) JMP_THIS(0x4DB810);

	//MissionClass
	virtual void vt_entry_1F4(Mission mission) JMP_THIS(0x4D8F40);
	virtual bool Mission_Revert()
	{
		/*if (!this->TechnoClass::Mission_Revert())
			return false;
		this->SetDestination(this->LastDestination, true);*/
		JMP_THIS(0x4D8F80);
	}
	virtual int Mission_Attack() JMP_THIS(0x4D4DC0);
	virtual int Mission_Capture() JMP_THIS(0x4D4B20);
	virtual int Mission_Eaten() JMP_THIS(0x4D4CB0);
	virtual int Mission_Guard() JMP_THIS(0x4D5070);
	virtual int Mission_AreaGuard() JMP_THIS(0x4D6AA0);
	virtual int Mission_Hunt() JMP_THIS(0x4D5350);
	virtual int Mission_Move() JMP_THIS(0x4D4200);
	virtual int Mission_Retreat() JMP_THIS(0x4DA2C0);
	//return 450 directly
	virtual int Mission_Unload() { return this->MissionClass::Mission_Unload(); }
	virtual int Mission_Enter() JMP_THIS(0x4D9290);
	virtual int Mission_Rescue() JMP_THIS(0x4DDF90);
	virtual int Mission_Patrol() JMP_THIS(0x4D4280);

	//TechnoClass
	virtual bool IsCloakable() const JMP_THIS(0x4DBDA0);
	virtual bool BelongsToATeam() const { return this->Team != nullptr; }
	virtual bool vt_entry_2C4(DWORD dwUnk) JMP_THIS(0x4DBA50);
	virtual bool vt_entry_2CC(DWORD dwUnk) JMP_THIS(0x4D3810);	//dwUnk is CoordStruct?
	virtual int GetZAdjustment() const JMP_THIS(0x4DAFC0);
	virtual ZGradient GetZGradient() const
	{
		/*if (this->Locomotor.get() != nullptr)
			return this->Locomotor->Z_Gradient();
		return ZGradient::Deg90;*/
		JMP_THIS(0x4DB0A0)
	}
	virtual CellStruct* GetSomeCellStruct(CellStruct* pCell) const { *pCell = this->LastJumpjetMapCoords; return pCell; }
	virtual void SetSomeCellStruct(CellStruct Cell) { this->LastJumpjetMapCoords = Cell; }
	virtual bool vt_entry_320() const JMP_THIS(0x4DA1D0);
	virtual void ProceedToNextPlanningWaypoint() JMP_THIS(0x4DE580);
	virtual DWORD ScanForTiberium(DWORD dwUnk1, DWORD dwUnk2, DWORD dwUnk3) const JMP_THIS(0x4DD0A0);
	virtual bool EnterGrinder() JMP_THIS(0x4DFA70);
	virtual bool EnterBioReactor() JMP_THIS(0x4DFB70);
	virtual bool EnterTankBunker() JMP_THIS(0x4DFF40);
	virtual bool EnterBattleBunker() JMP_THIS(0x4DFCB0);
	virtual bool GarrisonStructure() JMP_THIS(0x4DFE00);
	virtual bool IsParalyzed() const
	{
		/*int startTime = this->ParalysisTimer.StartTime;
		int leftTime = this->ParalysisTimer.TimeLeft;
		if (startTime != -1)
		{
			if (Unsorted::CurrentFrame - startTime < leftTime)
				return leftTime != Unsorted::CurrentFrame - startTime;
			leftTime = 0;
		}
		return leftTime != 0;*/
		JMP_THIS(0x4DE770);
	}
	virtual void AddPassenger(FootClass* pPassenger)
	{
		/*this->TechnoClass::AddPassenger(pPassenger);
		if (this->GetTechnoType()->Gunner && this->Passengers.NumPassengers == 1)
			this->ReceiveGunner(pPassenger);*/
		JMP_THIS(0x4DE630);
	}
	virtual void vt_entry_3A0()
	{
		/*this->SetDestination(nullptr, true);
		this->PathDirections[0] = -1;
		this->StopMoving();
		this->TechnoClass::vt_entry_3A0();
		*/
		JMP_THIS(0x4D5660);
	}
	virtual void Destroyed(ObjectClass *Killer) JMP_THIS(0x4D98C0);
	virtual CellClass* SelectAutoTarget(TargetFlags TargetFlags, int CurrentThreat, bool OnlyTargetHouseEnemy) JMP_THIS(0x4D9920);
	virtual bool SetOwningHouse(HouseClass* pHouse, bool announce = true) JMP_THIS(0x4DBED0);
	virtual bool Crash(ObjectClass* Killer) JMP_THIS(0x4DEBB0);
	virtual CoordStruct* GetTargetCoords(CoordStruct* pCrd) const JMP_THIS(0x4D8560);
	virtual void DrawActionLines(bool Force, DWORD dwUnk) JMP_THIS(0x4DC060);
	virtual void vt_entry_47C(DWORD dwUnk) { this->unknown_5A0 = dwUnk; }
	virtual void SetDestination(AbstractClass* pDest, bool bUnk) JMP_THIS(0x4D94B0);
	virtual bool vt_entry_484(DWORD dwUnk1, DWORD dwUnk2) JMP_THIS(0x4D82B0);
	virtual bool ForceCreate(CoordStruct& coord, DWORD dwUnk = 0) JMP_THIS(0x4DF510);
	virtual void vt_entry_4A4(DWORD dwUnk) JMP_THIS(0x4DF0E0);
	virtual void vt_entry_4A8()
	{
		/*this->unknown_int_5C4 = -1;
		this->unknown_5C8 = 0;
		this->unknown_5CC = 0;
		this->unknown_bool_5D1 = false;*/
		JMP_THIS(0x4DF1A0)
	}
	virtual bool vt_entry_4AC() const { return this->unknown_int_5C4 != -1; }
	virtual bool vt_entry_4B0() const { return this->unknown_bool_5D1; }
	virtual int vt_entry_4B4() const { return this->unknown_int_5C4; }
	virtual CoordStruct* vt_entry_4B8(CoordStruct* pCrd) JMP_THIS(0x4DF1F0);
	virtual bool vt_entry_4C4() const { return this->unknown_int_5C4 == 29; }
	virtual bool vt_entry_4C8() JMP_THIS(0x4DF320);
	virtual void vt_entry_4CC() JMP_THIS(0x4DF3A0);
	virtual bool vt_entry_4D0() JMP_THIS(0x4DF4B0);

	//FootClass
	virtual void ReceiveGunner(FootClass* Gunner) { }
	virtual void RemoveGunner(FootClass* Gunner) { }
	virtual bool IsLeavingMap() const JMP_THIS(0x4DC790);
	virtual bool vt_entry_4E0() const
	{
		/*if (!this->GetWeapon(0))
			return true;
		if (this->GetTechnoType()->TechLevel == -1)
			return true;
		if (this->GetTechnoType()->DeploysInto != nullptr)
			return true;
		return this->GetTechnoType()->Speed <= 13;*/
		JMP_THIS(0x4DBFD0)
	}
	virtual bool vt_entry_4E4() const { return false; }
	virtual void vt_entry_4E8(CellStruct* pCell) JMP_THIS(0x4DE7B0);
	virtual void vt_entry_4EC(CellStruct* pCell) JMP_THIS(0x4DE940);
	virtual CoordStruct* vt_entry_4F0(CoordStruct* pCrd)
	{
		/*CoordStruct tmp;
		CoordStruct* res;
		res = this->GetPosition_0(&tmp);
		*pCrd = *res;
		return pCrd;*/
		JMP_THIS(0x4D9FF0)
	}
	virtual void vt_entry_4F4()
	{
		/*if (this->GetCurrentMission() == Mission::Hunt)
		{
			this->SetTarget(nullptr);
			this->SetDestination(nullptr, true);
		}*/
		JMP_THIS(0x4DC030)
	}
	virtual bool vt_entry_4F8() { return false; }
	virtual bool MoveTo(CoordStruct* pCrd)
	{
		/*if (this->Locomotor.get() == nullptr)
			Game::RaiseError(-2147467261);
		this->Locomotor->Move_To(*pCrd);
		if (this->Locomotor.get() == nullptr)
			Game::RaiseError(-2147467261);
		return this->Locomotor->Is_Moving();
		*/
		JMP_THIS(0x4D55F0)
	}
	virtual bool StopMoving()
	{
		/*if (this->Locomotor.get() == nullptr)
			Game::RaiseError(-2147467261);
		this->Locomotor->Stop_Moving();
		return false;
		*/
		JMP_THIS(0x4D55C0)
	}
	virtual bool vt_entry_504() JMP_THIS(0x4DB9B0);
	virtual bool ChronoWarpTo(CoordStruct pDest) JMP_THIS(0x4DF7F0); // fsds... only implemented for one new YR map trigger, other chrono events repeat the code...
	virtual void Draw_A_SHP(
		SHPStruct *SHP,
		int idxFacing,
		Point2D * Coords,
		RectangleStruct *Rectangle,
		DWORD dwUnk5,
		DWORD dwUnk6,
		DWORD dwUnk7,
		ZGradient ZGradient,
		DWORD dwUnk9,
		int extraLight,
		DWORD dwUnk11,
		DWORD dwUnk12,
		DWORD dwUnk13,
		DWORD dwUnk14,
		DWORD dwUnk15,
		DWORD dwUnk16) JMP_THIS(0x41C090);

	virtual void Draw_A_VXL(
		VoxelStruct *VXL,
		int HVAFrameIndex,
		int Flags,
		IndexClass<int, int> *Cache,
		RectangleStruct *Rectangle,
		Point2D *CenterPoint,
		Matrix3D *Matrix,
		DWORD dwUnk8,
		DWORD DrawFlags,
		DWORD dwUnk10) JMP_THIS(0x4DAF10);

	virtual void vt_entry_514() { }
	virtual void Panic() { }
	virtual void UnPanic() { } //never
	virtual void PlayIdleAnim(int nIdleAnimNumber) { }
	virtual DWORD vt_entry_524() { return 0; }
	virtual DWORD vt_entry_528 (DWORD dwUnk1, DWORD dwUnk2, DWORD dwUnk3) const JMP_THIS(0x4DF040);
	virtual DWORD vt_entry_52C(DWORD dwUnk1, DWORD dwUnk2, DWORD dwUnk3, DWORD dwUnk4) const JMP_THIS(0x4DEE80);
	virtual DWORD vt_entry_530(DWORD dwUnk1, DWORD dwUnk2, DWORD dwUnk3) const JMP_THIS(0x4DEE50);
	virtual void vt_entry_534(DWORD dwUnk1, DWORD dwUnk2) { }
	virtual int GetCurrentSpeed() const JMP_THIS(0x4DB1A0);
	virtual DWORD vt_entry_53C(DWORD dwUnk) JMP_THIS(0x4D5690);
	virtual void vt_entry_540(DWORD dwUnk) { }
	virtual void SetSpeedPercentage(double percentage)
	{
		/*if (percentage < 1.0)
		{
			if (percentage > 0.0)
				this->SpeedPercentage = percentage;
			else
				this->SpeedPercentage = 0.0;
		}
		else
		{
			this->SpeedPercentage = 0x3FF0000000000000;
		}*/
		JMP_THIS(0x4D3710);
	}
	virtual void vt_entry_548() { }
	virtual void vt_entry_54C() { }
	virtual bool vt_entry_550(DWORD dwUnk) JMP_THIS(0x4DDC60);

	// non-virtual

	bool CanBeRecruited(HouseClass* ByWhom) const
		{ JMP_THIS(0x4DA230); }

	// only used by squid damage routines, normal wakes are created differently it seems
	// creates 3 wake animations behind the unit
	void CreateWakes(CoordStruct coords)
		{ JMP_THIS(0x629E90); }

	// can this jumpjet stay in this cell or not? (two jumpjets in one cell are not okay, locomotor kicks one of them out in the next frame)
	bool Jumpjet_LocationClear() const
		{ JMP_THIS(0x4135A0); }

	void Jumpjet_OccupyCell(CellStruct Cell)
		{ JMP_THIS(0x4E00B0); }

	// changes locomotor to the given one, Magnetron style
	// mind that this locks up the source too, Magnetron style
	void FootClass_ImbueLocomotor(FootClass *target, CLSID clsid)
		{ JMP_THIS(0x710000); }

	// var $this = this; $.each($this.Passengers, function(ix, p) { p.Location = $this.Location; });
	void UpdatePassengerCoords()
		{ JMP_THIS(0x7104F0); }

	void AbortMotion()
		{ JMP_THIS(0x4DF0D0); }

	bool UpdatePathfinding(CellStruct unkCell, CellStruct unkCell2, int unk3)
		{ JMP_THIS(0x4D3920); }

	// Removes the first passenger and updates the Gunner.
	FootClass* RemoveFirstPassenger()
		{ JMP_THIS(0x4DE710); }

	// Removes a specific passenger and updates the Gunner.
	FootClass* RemovePassenger(FootClass* pPassenger)
		{ JMP_THIS(0x4DE670); }

	// Adds a specific passenger and updates the Gunner.
	void EnterAsPassenger(FootClass* pPassenger)
		{ JMP_THIS(0x4DE630); }

	void ClearSomeVector() // clears 5AC
		{ JMP_THIS(0x4DA1C0); }

	// searches cell, sets destination, and returns whether unit is on that cell
	bool MoveToTiberium(int radius, bool scanClose = false)
		{ JMP_THIS(0x4DCFE0); }

	// searches cell, sets destination, and returns whether unit is on that cell
	bool MoveToWeed(int radius)
		{ JMP_THIS(0x4DDB90); }

	MissionControlClass** sub_5B3A00() JMP_THIS(0x5B3A00);

	//helpers
	CoordStruct GetDestination(TechnoClass* pDocker = nullptr) const
	{
		CoordStruct ret;
		this->GetDestination(&ret, pDocker);
		return ret;
	}

	//Constructor
	FootClass(HouseClass* pOwner) noexcept : FootClass(noinit_t())
		{ JMP_THIS(0x4D31E0); }

protected:
	explicit __forceinline FootClass(noinit_t) noexcept
		: TechnoClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	int             PlanningPathIdx; // which planning path am I following?
	short           unknown_short_524;
	short           unknown_short_526;
	short           unknown_short_528;
	short           unknown_short_52A;
	DWORD           unknown_52C;	//unused?
	double          unknown_530;
	int				WalkedFramesSoFar;
	bool            unknown_bool_53C;
	DWORD           unknown_540;

	DECLARE_PROPERTY(AudioController, Audio7);

	CellStruct      CurrentMapCoords;
	CellStruct      LastMapCoords; // ::UpdatePosition uses this to remove threat from last occupied cell, etc
	CellStruct      LastJumpjetMapCoords; // which cell was I occupying previously? only for jumpjets
	CellStruct      CurrentJumpjetMapCoords; // which cell am I occupying? only for jumpjets
	CoordStruct     unknown_coords_568;
	PROTECTED_PROPERTY(DWORD,   unused_574);
	double          SpeedPercentage;
	double          SpeedMultiplier;
	DECLARE_PROPERTY(DynamicVectorClass<AbstractClass*>, unknown_abstract_array_588);
	DWORD           unknown_5A0;
	AbstractClass*  Destination; // possibly other objects as well
	AbstractClass*  LastDestination;
	DECLARE_PROPERTY(DynamicVectorClass<AbstractClass*>, unknown_abstract_array_5AC);
	int             unknown_int_5C4;
	DWORD           unknown_5C8;
	DWORD           unknown_5CC;
	BYTE            unknown_5D0;	//unused?
	bool            unknown_bool_5D1;
	TeamClass*      Team;
	FootClass*      NextTeamMember;        //next unit in team
	DWORD           unknown_5DC;
	int             PathDirections[24]; // list of directions to move in next, like tube directions
	DECLARE_PROPERTY(TimerStruct, PathDelayTimer);
	int             unknown_int_64C;
	DECLARE_PROPERTY(TimerStruct, unknown_timer_650);
	DECLARE_PROPERTY(TimerStruct, SightTimer);
	DECLARE_PROPERTY(TimerStruct, BlockagePathTimer);
	DECLARE_PROPERTY(YRComPtr<ILocomotion>, Locomotor);
	CoordStruct       unknown_point3d_678;
	signed char       TubeIndex;	//I'm in this tunnel
	bool              unknown_bool_685;
	signed char       WaypointIndex; // which waypoint in my planning path am I following?
	bool              unknown_bool_687;
	bool              unknown_bool_688;
	bool              IsTeamLeader;
	bool              ShouldScanForTarget;
	bool              unknown_bool_68B;
	bool              unknown_bool_68C;
	bool              unknown_bool_68D;
	bool              unknown_bool_68E;
	bool              ShouldEnterAbsorber; // orders the unit to enter the closest bio reactor
	bool              ShouldEnterOccupiable; // orders the unit to enter the closest battle bunker
	bool              ShouldGarrisonStructure; // orders the unit to enter the closest neutral building
	FootClass*        ParasiteEatingMe; // the tdrone/squid that's eating me
	DWORD             unknown_698;
	ParasiteClass*    ParasiteImUsing;	// my parasitic half, nonzero for, eg, terror drone or squiddy
	DECLARE_PROPERTY(TimerStruct, ParalysisTimer); // for squid victims
	bool              unknown_bool_6AC;
	bool              IsAttackedByLocomotor; // the unit's locomotor is jammed by a magnetron
	bool              IsLetGoByLocomotor; // a magnetron attacked this unit and let it go. falling, landing, or sitting on the ground
	bool              unknown_bool_6AF;
	bool              unknown_bool_6B0;
	bool              unknown_bool_6B1;
	bool              unknown_bool_6B2;
	bool              unknown_bool_6B3;
	bool              unknown_bool_6B4;
	bool              unknown_bool_6B5;
	bool              FrozenStill; // frozen in first frame of the proper facing - when magnetron'd or warping
	bool              unknown_bool_6B7;
	bool              unknown_bool_6B8;
	PROTECTED_PROPERTY(DWORD,   unused_6BC);	//???
};