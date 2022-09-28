/*
	Base class for all game objects.
*/

#pragma once

#include <AbstractClass.h>
#include <Audio.h>
#include <ObjectTypeClass.h>
#include <TagClass.h>
#include <RulesClass.h>

#include <Helpers/Template.h>

struct SHPStruct;
class LightConvertClass;

//forward declarations
class AnimClass;
class BombClass;
class BuildingTypeClass;
class CellClass;
class InfantryTypeClass;
class TechnoClass;
class TechnoTypeClass;
class WarheadTypeClass;

class HouseTypeClass;

class LineTrail;
struct WeaponStruct;

class NOVTABLE ObjectClass : public AbstractClass
{
public:
	static const auto AbsDerivateID = AbstractFlags::Object;

	//global arrays
	static constexpr reference<DynamicVectorClass<ObjectClass*>, 0xA8E360u> const Array{};
	//that's maybe selected object of this player
	static constexpr reference<DynamicVectorClass<ObjectClass*>, 0xA8ECB8u> const CurrentObjects{};

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) JMP_STD(0x5F5E80);

	//Destructor
	virtual ~ObjectClass() JMP_THIS(0x5F6DC0);

	//AbstractClass
	virtual void PointerExpired(AbstractClass* pAbstract, bool removed) JMP_THIS(0x5F5230);
	virtual void ComputeCRC(CRCEngine& crc) const JMP_THIS(0x5F6250);
	virtual bool IsDead() const JMP_THIS(0x5F6690);
	virtual CoordStruct* GetCoords(CoordStruct* pCrd) const JMP_THIS(0x5F65A0);
	virtual bool IsOnFloor() const JMP_THIS(0x5F6B60);
	virtual bool IsInAir() const JMP_THIS(0x5F6B90);
	virtual void Update() JMP_THIS(0x5F3E70);

	//ObjectClass
	virtual void AnimPointerExpired(AnimClass* pAnim) JMP_THIS(0x5F6DA0);
	virtual bool IsSelectable() const { return false; }
	virtual VisualType VisualCharacter(VARIANT_BOOL SpecificOwner, HouseClass* WhoIsAsking) const { return VisualType::Normal; }
	virtual SHPStruct* GetImage() const JMP_THIS(0x5F3E30);
	virtual Action MouseOverCell(CellStruct const* pCell, bool checkFog = false, bool ignoreForce = false) const { return Action::None; }
	virtual Action MouseOverObject(ObjectClass const* pObject, bool ignoreForce = false) const { return Action::None; }
	virtual Layer InWhichLayer() const JMP_THIS(0x5F4260);
	virtual bool IsSurfaced() JMP_THIS(0x5F6C10); // opposed to being submerged

 /*
  	Building returns if it is 1x1 and has UndeploysInto
  	inf returns 0
  	unit returns !NonVehicle
  	Aircraft returns IsOnFloor()

  users include:
  452656 - is this building click-repairable
  440C26 - should this building get considered in BaseSpacing
  445A8E - -""-
  51E7D1 - can a VehicleThief be clicked to steal this unit
  51E4D9 - can an engi be clicked to enter this to fix/takeover
  51F0D3 - -""-
  51EA06 - can this building be C4'd?
  51E243 - can a VehicleThief steal this on his own decision
  4F93F3 - should this building's damage raise a BaseUnderAttack?
  442286 - -""-
  44296A - -""-
  741117 - can this be healed by a vehicle?
  6F8242 - can this aircraft be auto-target
  6F85BE - can this aircraft be auto-attacked
  */
	virtual bool IsStrange() const { return false; }

	virtual TechnoTypeClass* GetTechnoType() const { return nullptr; }
	virtual ObjectTypeClass* GetType() const { return nullptr; }
	virtual DWORD GetTypeOwners() const { return INT_MAX; } // returns the data for IndexBitfield<HouseTypeClass*>
	virtual const wchar_t* GetUIName() const { return L"No name"; }
	virtual bool CanBeRepaired() const { return false; }
	virtual bool CanBeSold() const { return false; }
	virtual bool IsActive() const { return false; }

	// can the current player control this unit? (owned by him, not paralyzed, not spawned, not warping, not slaved...)
	virtual bool IsControllable() const { return false; }

	virtual CoordStruct* GetTargetCoords_FIX(CoordStruct* pCrd) const { return this->GetCoords(pCrd); }

	// gets a building's free dock coordinates for a unit. falls back to this->GetCoords(pCrd);
	virtual CoordStruct* GetDockCoords(CoordStruct* pCrd, TechnoClass* docker) const { return this->GetCoords(pCrd); }

	virtual CoordStruct* GetRenderCoords(CoordStruct* pCrd) const { return this->GetCoords(pCrd); }
	virtual CoordStruct* GetFLH(CoordStruct *pDest, int idxWeapon, CoordStruct BaseCoords) const JMP_THIS(0x4263D0);
	virtual CoordStruct* GetExitCoords(CoordStruct* pCrd, DWORD dwUnk) const { return this->GetCoords(pCrd); }
	virtual int GetYSort() const JMP_THIS(0x5F6BD0);
	virtual bool IsOnBridge(TechnoClass* pDocker = nullptr) const JMP_THIS(0x5F6A70); // pDocker is passed to GetDestination
	virtual bool IsStandingStill() const { return true; }
	virtual bool IsDisguised() const { return false; }
	virtual bool IsDisguisedAs(HouseClass* target) const { return false; } // only works correctly on infantry!
	virtual ObjectTypeClass* GetDisguise(bool DisguisedAgainstAllies) const { return nullptr; }
	virtual HouseClass* GetDisguiseHouse(bool DisguisedAgainstAllies) const { return nullptr; }

	// remove object from the map
	virtual bool Limbo() JMP_THIS(0x5F4D30);

	// place the object on the map
	virtual bool Unlimbo(const CoordStruct& coords, Direction::Value dFaceDir) JMP_THIS(0x5F4EC0);

	// cleanup things (lose line trail, deselect, etc). Permanently: destroyed/removed/gone opposed to just going out of sight.
	virtual void Disappear(bool permanently) JMP_THIS(0x5F5280);

	virtual void RegisterDestruction(TechnoClass* Destroyer) { }

	 // maybe Object instead of Techno? Raises Map Events, grants veterancy, increments house kill counters
	virtual void RegisterKill(HouseClass* Destroyer) { } // ++destroyer's kill counters , etc

	virtual bool SpawnParachuted(const CoordStruct& coords) JMP_THIS(0x5F5940);
	virtual void DropAsBomb() JMP_THIS(0x5F4160);
	virtual void MarkAllOccupationBits(const CoordStruct& coords) JMP_THIS(0x5F60A0);
	virtual void UnmarkAllOccupationBits(const CoordStruct& coords) JMP_THIS(0x5F6120);
	virtual void UnInit() JMP_THIS(0x5F65F0);
	virtual void Reveal() { } // uncloak when object is bumped, damaged, detected, ...
	virtual KickOutResult KickOutUnit(TechnoClass* pTechno, CellStruct Cell) { return KickOutResult::Failed; }
	virtual bool DrawIfVisible(RectangleStruct *pBounds, bool EvenIfCloaked, DWORD dwUnk3) const JMP_THIS(0x5F4B10);
	virtual CellStruct const* GetFoundationData(bool includeBib = false) const JMP_THIS(0x5F5B90);
	virtual void DrawBehind(Point2D* pLocation, RectangleStruct* pBounds) const { }
	virtual void DrawExtras(Point2D* pLocation, RectangleStruct* pBounds) const { } // draws ivan bomb, health bar, talk bubble, etc
	virtual void Draw(Point2D* pLocation, RectangleStruct* pBounds) const { }
	virtual void DrawAgain(const Point2D& location, const RectangleStruct& bounds) const { this->Draw(const_cast<Point2D*>(&location), const_cast<RectangleStruct*>(&bounds)); } // just forwards the call to Draw
	virtual void Undiscover() { };
	virtual void See(DWORD dwUnk1, DWORD dwUnk2) { };
	virtual bool UpdatePlacement(PlacementType value) JMP_THIS(0x5F5850);
	virtual RectangleStruct* GetDimensions(RectangleStruct* pRect) const JMP_THIS(0x5F4730);
	virtual RectangleStruct* GetRenderDimensions(RectangleStruct* pRect) JMP_THIS(0x5F4870);
	virtual void DrawRadialIndicator(DWORD dwUnk) { };
	virtual void MarkForRedraw() JMP_THIS(0x5F4D10);
	virtual bool CanBeSelected() const JMP_THIS(0x5F6C30);
	virtual bool CanBeSelectedNow() const { return this->CanBeSelected(); }
	virtual bool CellClickedAction(Action action, CellStruct* pCell, CellStruct* pCell1, bool bUnk) { return false; }
	virtual bool ObjectClickedAction(Action action, ObjectClass* pTarget, bool bUnk) { return 0; }
	virtual void Flash(int Duration) { }
	virtual bool Select() JMP_THIS(0x5F4520);
	virtual void Deselect() JMP_THIS(0x5F44A0);
	virtual DamageState IronCurtain(int nDuration, HouseClass* pSource, bool ForceShield) { return DamageState::Unaffected; }
	virtual void StopAirstrikeTimer() { }
	virtual void StartAirstrikeTimer(int Duration) { }
	virtual bool IsIronCurtained() const { return false; }
	virtual bool IsCloseEnough3D(DWORD dwUnk1, DWORD dwUnk2) const { return false; }
	virtual int GetWeaponRange(int idxWeapon) const { return 0; }
	virtual DamageState ReceiveDamage(
		int* pDamage,
		int distanceFromEpicenter,
		WarheadTypeClass* pWH,
		ObjectClass* pAttacker,
		bool ignoreDefenses,
		bool preventPassengerEscape,
		HouseClass* pAttackingHouse) JMP_THIS(0x5F5390);
	virtual void Destroy() { }
	virtual void Scatter(const CoordStruct& crd, bool ignoreMission, bool ignoreDestination) { }
	virtual bool Ignite() { return 0; }
	virtual void Extinguish() { }
	virtual DWORD GetPointsValue() const { return 0; }
	virtual Mission GetCurrentMission() const { return Mission::None; }
	virtual void RestoreMission(Mission mission) { }
	virtual void UpdatePosition(int dwUnk) { }
	virtual BuildingClass* FindFactory(bool allowOccupied, bool requirePower) const JMP_THIS(0x5F5C20);
	virtual RadioCommand ReceiveCommand(TechnoClass* pSender, RadioCommand command, AbstractClass* &pInOut) JMP_THIS(0x5F5320);
	virtual bool DiscoveredBy(HouseClass* pHouse) { return pHouse != nullptr; }
	virtual void SetRepairState(int state) { } // 0 - off, 1 - on, -1 - toggle
	virtual void Sell(DWORD dwUnk) { }
	virtual void AssignPlanningPath(signed int idxPath, signed char idxWP) { }
	virtual void vt_entry_1A8(DWORD dwUnk) JMP_THIS(0x5F4410);
	virtual Move IsCellOccupied(CellClass* pDestCell, int facing, int level, CellClass* pSourceCell, bool alt) const { return Move::OK; }
	virtual DWORD vt_entry_1B0(DWORD dwUnk1, DWORD dwUnk2, DWORD dwUnk3, DWORD dwUnk4, DWORD dwUnk5) { return false; }
	virtual void SetLocation(const CoordStruct& crd) { this->Location = crd; }

// these two work through the object's Location
	virtual CellStruct* GetMapCoords(CellStruct* pUCell) const { *pUCell = { short(this->Location.X / 256),short(this->Location.Y / 256) }; return pUCell; }
	virtual CellClass* GetCell() const JMP_THIS(0x5F6960);

// these two call ::GetCoords_() instead
	virtual CellStruct* GetMapCoordsAgain(CellStruct* pUCell) const JMP_THIS(0x5F69C0);
	virtual CellClass* GetCellAgain() const JMP_THIS(0x5F6A10);

	virtual int GetHeight() const JMP_THIS(0x5F5F40);
	virtual void SetHeight(DWORD dwUnk) JMP_THIS(0x5F5FA0);
	virtual int GetZ() const { return this->Location.Z; }
	virtual bool IsBeingWarpedOut() const { return false; }
	virtual bool IsWarpingIn() const { return false; }
	virtual bool IsWarpingSomethingOut() const { return false; }
	virtual bool IsNotWarping() const { return true; }
	virtual LightConvertClass* GetRemapColour() const { return nullptr; }

	// technically it takes an ecx<this> , but it's not used and ecx is immediately overwritten on entry
	// draws the mind control line when unit is selected
	static void DrawALinkTo(int src_X, int src_Y, int src_Z, int dst_X, int dst_Y, int dst_Z, ColorStruct color)
		{ PUSH_VAR32(color); PUSH_VAR32(dst_Z); PUSH_VAR32(dst_Y); PUSH_VAR32(dst_X);
		  PUSH_VAR32(src_Z); PUSH_VAR32(src_Y); PUSH_VAR32(src_X); CALL(0x704E40); }

	int DistanceFrom(AbstractClass *that) const
		{ JMP_THIS(0x5F6440); }

	double GetHealthPercentage() const
		{ return static_cast<double>(this->Health) / this->GetType()->Strength; }

	bool IsRedHP() const
		{ JMP_THIS(0x5F5CD0); }

	bool IsYellowHP() const
		{ JMP_THIS(0x5F5D20); }

	bool IsGreenHP() const
		{ JMP_THIS(0x5F5D90); }

	HealthState GetHealthStatus() const
		{ JMP_THIS(0x5F5DD0); }

	bool AttachTrigger(TagClass* pTag)
		{ JMP_THIS(0x5F5B50); }

	void BecomeUntargetable()
		{ JMP_THIS(0x70D4A0); }

	void ReplaceTag(TagClass* pTag)
		{ JMP_THIS(0x5F5B4C); }

	int GetCellLevel() const
		{ JMP_THIS(0x5F5F00); }

	TechnoClass* ConvertTechno()
		{ JMP_THIS(0x40DD70); }

	bool IsCrushable(TechnoClass* pCrusher) const
	{ JMP_THIS(0x5F6CD0); }

	CellStruct GetMapCoords() const
	{
		CellStruct ret;
		this->GetMapCoords(&ret);
		return ret;
	}

	CellStruct GetMapCoordsAgain() const
	{
		CellStruct ret;
		this->GetMapCoordsAgain(&ret);
		return ret;
	}

	CoordStruct GetFLH(int idxWeapon, const CoordStruct& base) const
	{
		CoordStruct ret;
		this->GetFLH(&ret, idxWeapon, base);
		return ret;
	}

	CoordStruct GetCoords() const
	{
		CoordStruct ret;
		this->GetCoords(&ret);
		return ret;
	}

	DamageState TakeDamage
	(
		int damage,
		HouseClass* pSourceHouse = nullptr,
		ObjectClass* pAttacker = nullptr,
		WarheadTypeClass* pWH = RulesClass::Instance->C4Warhead,
		bool preventPassengerEscape = false,
		bool ignoreDefense = true,
		int distanceFromEpicenter = 0
	)
	{
		return ReceiveDamage(&damage, distanceFromEpicenter, pWH, pAttacker, ignoreDefense, preventPassengerEscape, pSourceHouse);
	}

	//Constructor NEVER CALL IT DIRECTLY
	/*ObjectClass()  noexcept
		{ JMP_THIS(0x5F3900); }*/

protected:
	explicit __forceinline ObjectClass(noinit_t)  noexcept
		: AbstractClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	DWORD              unknown_24;
	DWORD              unknown_28;
	int                FallRate; //how fast is it falling down? only works if FallingDown is set below, and actually positive numbers will move the thing UPWARDS
	ObjectClass*       NextObject;	//Next Object in the same cell or transport. This is a linked list of Objects.
	TagClass*          AttachedTag; //Should be TagClass , TODO: change when implemented
	BombClass*         AttachedBomb; //Ivan's little friends.
	DECLARE_PROPERTY(AudioController, AmbientSoundController); // the "mofo" struct, evil evil stuff
	DECLARE_PROPERTY(AudioController, CustomSoundController); // the "mofo" struct, evil evil stuff
	int                CustomSound;
	bool               BombVisible; // In range of player's bomb seeing units, so should draw it
	PROTECTED_PROPERTY(BYTE, align_69[0x3]);
	int                Health;		//The current Health.
	int                EstimatedHealth; // used for auto-targeting threat estimation
	bool               IsOnMap; // has this object been placed on the map?
	PROTECTED_PROPERTY(BYTE, align_75[0x3]);
	DWORD              unknown_78;
	DWORD              unknown_7C;
	bool               NeedsRedraw;
	bool               InLimbo; // act as if it doesn't exist - e.g., post mortem state before being deleted
	bool               InOpenToppedTransport;
	bool               IsSelected;	//Has the player selected this Object?
	bool               HasParachute;	//Is this Object parachuting?
	PROTECTED_PROPERTY(BYTE, align_85[0x3]);
	AnimClass*         Parachute;		//Current parachute Anim.
	bool               OnBridge;
	bool               IsFallingDown;
	bool               WasFallingDown; // last falling state when FootClass::Update executed. used to find out whether it changed.
	bool               IsABomb; // if set, will explode after FallingDown brings it to contact with the ground
	bool               IsAlive;		//Self-explanatory.
	PROTECTED_PROPERTY(BYTE, align_91[0x3]);
	Layer              LastLayer;
	bool               IsInLogic; // has this object been added to the logic collection?
	bool               IsVisible; // was this object in viewport when drawn?
	PROTECTED_PROPERTY(BYTE, align_99[0x2]);
	CoordStruct        Location; //Absolute current 3D location (in leptons)
	LineTrail*         LineTrailer;
 };
