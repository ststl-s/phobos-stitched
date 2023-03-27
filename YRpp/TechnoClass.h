/*
	Base class for buildable objects
*/

#pragma once

#include <Matrix3D.h>
#include <RadioClass.h>
#include <RadBeam.h>
#include <TechnoTypeClass.h>
#include <CaptureManagerClass.h>
#include <SlaveManagerClass.h>
#include <TeamClass.h>
#include <TemporalClass.h>
#include <LaserDrawClass.h>
#include <EBolt.h>
#include <Helpers/Template.h>
#include <StageClass.h>
#include <PlanningTokenClass.h>

//forward declarations
class AirstrikeClass;
class AnimClass;
class BulletClass;
class BuildingClass;
class CellClass;
class HouseClass;
class FootClass;
class HouseClass;
class InfantryTypeClass;
class ObjectTypeClass;
class ParticleSystemClass;
class SpawnManagerClass;
class WaveClass;

struct NetworkEvent;

#include <TransitionTimer.h>

struct VeterancyStruct
{
	VeterancyStruct() = default;

	explicit VeterancyStruct(double value) noexcept {
		this->Add(value);
	}

	void Add(int ownerCost, int victimCost) noexcept {
		this->Add(static_cast<double>(victimCost)
			/ (ownerCost * RulesClass::Instance->VeteranRatio));
	}

	void Add(double value) noexcept {
		auto val = this->Veterancy + value;

		if(val > RulesClass::Instance->VeteranCap) {
			val = RulesClass::Instance->VeteranCap;
		}

		this->Veterancy = static_cast<float>(val);
	}

	Rank GetRemainingLevel() const noexcept {
		if(this->Veterancy >= 2.0f) {
			return Rank::Elite;
		}

		if(this->Veterancy >= 1.0f) {
			return Rank::Veteran;
		}

		return Rank::Rookie;
	}

	bool IsNegative() const noexcept {
		return this->Veterancy < 0.0f;
	}

	bool IsRookie() const noexcept {
		return this->Veterancy >= 0.0f && this->Veterancy < 1.0f;
	}

	bool IsVeteran() const noexcept {
		return this->Veterancy >= 1.0f && this->Veterancy < 2.0f;
	}

	bool IsElite() const noexcept {
		return this->Veterancy >= 2.0f;
	}

	void Reset() noexcept {
		this->Veterancy = 0.0f;
	}

	void SetRookie(bool notReally = true) noexcept {
		this->Veterancy = notReally ? -0.25f : 0.0f;
	}

	void SetVeteran(bool yesReally = true) noexcept {
		this->Veterancy = yesReally ? 1.0f : 0.0f;
	}

	void SetElite(bool yesReally = true) noexcept {
		this->Veterancy = yesReally ? 2.0f : 0.0f;
	}

	float Veterancy{ 0.0f };
};

class PassengersClass
{
public:
	int NumPassengers;
	FootClass* FirstPassenger;

	void AddPassenger(FootClass* pPassenger)
		{ JMP_THIS(0x4733A0); }

	FootClass* GetFirstPassenger() const
		{ return this->FirstPassenger; }

	FootClass* RemoveFirstPassenger()
		{ JMP_THIS(0x473430); }

	int GetTotalSize() const
		{ JMP_THIS(0x473460); }

	int IndexOf(FootClass* candidate) const
		{ JMP_THIS(0x473500); }

	void RemovePassenger(FootClass* pPassenger)
		{ JMP_THIS(0x4734B0); }

	PassengersClass() : NumPassengers(0), FirstPassenger(nullptr) {};

	~PassengersClass() { };
};

struct FlashData
{
	int DurationRemaining;
	bool FlashingNow;

	bool Update()
		{ JMP_THIS(0x4CC770); }
};

struct RecoilData
{
	enum class RecoilState : unsigned int {
		Inactive = 0,
		Compressing = 1,
		Holding = 2,
		Recovering = 3,
	};

	TurretControl Turret;
	float TravelPerFrame;
	float TravelSoFar;
	RecoilState State;
	int TravelFramesLeft;

	void Update()
		{ JMP_THIS(0x70ED10); }

	void Fire()
		{ JMP_THIS(0x70ECE0); }
};

class NOVTABLE TechnoClass : public RadioClass
{
public:
	static const auto AbsDerivateID = AbstractFlags::Techno;

	static constexpr constant_ptr<DynamicVectorClass<TechnoClass*>, 0xA8EC78u> const Array{};

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x70BF50);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override JMP_STD(0x70C250);

	//Destructor
	virtual ~TechnoClass() override JMP_THIS(0x7106E0);

	//AbstractClass
	virtual void Init() override { JMP_THIS(0x6F3F40); }
	virtual void PointerExpired(AbstractClass* pAbstract, bool removed) override JMP_THIS(0x7077C0);
	virtual void ComputeCRC(CRCEngine& crc) const override JMP_THIS(0x70C270);
	virtual int GetOwningHouseIndex() const override JMP_THIS(0x6F9DB0);//{ return this->Owner->ArrayIndex; }
	virtual HouseClass* GetOwningHouse() const override { return this->Owner; }
	virtual void Update() override JMP_THIS(0x6F9E50);

	//ObjectClass
	virtual void AnimPointerExpired(AnimClass* pAnim) override JMP_THIS(0x710410);
	virtual bool IsSelectable() const override JMP_THIS(0x6F32D0);
	virtual VisualType VisualCharacter(VARIANT_BOOL specificOwner, HouseClass* whoIsAsking) const override JMP_THIS(0x703860);
	virtual Action MouseOverCell(CellStruct const* pCell, bool checkFog = false, bool ignoreForce = false) const override JMP_THIS(0x700600);
	virtual Action MouseOverObject(ObjectClass const* pObject, bool ignoreForce = false) const override JMP_THIS(0x6FFEC0);
	virtual TechnoTypeClass* GetTechnoType() const override { return static_cast<TechnoTypeClass*>(this->GetType()); }
	virtual DWORD GetTypeOwners() const override JMP_THIS(0x708B30);
	virtual bool CanBeRepaired() const override JMP_THIS(0x701140);
	virtual bool IsActive() const override JMP_THIS(0x7010D0);
	virtual bool IsControllable() const override JMP_THIS(0x700C40);
	virtual CoordStruct* GetFLH(CoordStruct* pDest, int idxWeapon, CoordStruct baseCoords) const override JMP_THIS(0x6F3AD0);
	virtual bool IsDisguised() const override JMP_THIS(0x41C010);
	virtual bool IsDisguisedAs(HouseClass* target) const override JMP_THIS(0x41C020)
	virtual bool Limbo() override JMP_THIS(0x6F6AC0);
	virtual bool Unlimbo(const CoordStruct& coords, DirType dFaceDir) override JMP_THIS(0x6F6CA0);
	virtual void RegisterDestruction(TechnoClass* pDestroyer) override JMP_THIS(0x702D40);
	virtual void RegisterKill(HouseClass* Destroyer) override JMP_THIS(0x703230);
	virtual void Reveal() override { this->Uncloak(false); }
	virtual KickOutResult KickOutUnit(TechnoClass* pTechno, CellStruct cell) override { return KickOutResult::Failed; }
	virtual void DrawBehind(Point2D* pLocation, RectangleStruct* pBounds) const override JMP_THIS(0x6F60D0);
	virtual void DrawExtras(Point2D* pLocation, RectangleStruct* pBounds) const override JMP_THIS(0x6F5190);
	virtual void Undiscover() override JMP_THIS(0x6F4A40);
	virtual void See(DWORD dwUnk1, DWORD dwUnk2) override JMP_THIS(0x70ADC0);
	virtual bool UpdatePlacement(PlacementType value) override JMP_THIS(0x6F4A70);
	virtual bool CanBeSelectedNow() const override JMP_THIS(0x6FC030);
	virtual void Flash(int duration) override JMP_THIS(0x6F9DD0);
	virtual bool Select() override JMP_THIS(0x6FBFA0);
	virtual DamageState IronCurtain(int duration, HouseClass* pSource, bool forceShield) override JMP_THIS(0x70E2B0);
	virtual void StopAirstrikeTimer() override JMP_THIS(0x70E340);
	virtual void StartAirstrikeTimer(int duration) override JMP_THIS(0x70E300);
	virtual bool IsIronCurtained() const override JMP_THIS(0x41BF40);
	virtual bool IsCloseEnough3D(DWORD dwUnk1, DWORD dwUnk2) const override JMP_THIS(0x6F7970);
	virtual int GetWeaponRange(int idxWeapon) const override JMP_THIS(0x7012C0);
	virtual DamageState ReceiveDamage(
		int* pDamage,
		int distanceFromEpicenter,
		WarheadTypeClass* pWH,
		ObjectClass* pAttacker,
		bool ignoreDefenses,
		bool preventPassengerEscape,
		HouseClass* pAttackingHouse) override JMP_THIS(0x701900);
	virtual void Destroy() override JMP_THIS(0x710460);
	virtual DWORD GetPointsValue() const override JMP_THIS(0x707DD0);
	virtual void UpdatePosition(int dwUnk) override JMP_THIS(0x6F5090);
	virtual RadioCommand ReceiveCommand(TechnoClass* pSender, RadioCommand command, AbstractClass*& pInOut) override JMP_THIS(0x6F4AB0);
	virtual bool DiscoveredBy(HouseClass* pHouse) override JMP_THIS(0x6F4960);
	virtual bool IsBeingWarpedOut() const override JMP_THIS(0x70C5B0)
	virtual bool IsWarpingIn() const override JMP_THIS(0x70C5C0)
	virtual bool IsWarpingSomethingOut() const override JMP_THIS(0x70C5D0)
	virtual bool IsNotWarping() const override JMP_THIS(0x70C5F0)
	virtual LightConvertClass* GetRemapColour() const override { return nullptr; }

	//MissionClass
	virtual void Override_Mission(Mission mission, AbstractClass* target, AbstractClass* destination) override JMP_THIS(0x7013A0);
	virtual bool Mission_Revert() override JMP_THIS(0x7013E0);

	//TechnoClass
	virtual bool IsUnitFactory() const { return false; }
	virtual bool IsCloakable() const JMP_THIS(0x70C5A0);
	virtual bool CanScatter() const	JMP_THIS(0x6F3280);
	virtual bool BelongsToATeam() const { return false; }
	virtual bool ShouldSelfHealOneStep() const JMP_THIS(0x70BE80);
	virtual bool IsVoxel() const JMP_THIS(0x6F9E10);
	virtual bool vt_entry_29C() { return true; }
	virtual bool IsReadyToCloak() const JMP_THIS(0x6FBDC0);
	virtual bool ShouldNotBeCloaked() const JMP_THIS(0x6FBC90);
	virtual DirStruct* TurretFacing(DirStruct* pBuffer) const JMP_THIS(0x4E0150);
	virtual bool IsArmed() const JMP_THIS(0x701120); // GetWeapon(primary) && GetWeapon(primary)->WeaponType
	virtual void vt_entry_2B0() const JMP_THIS(0x70C620);
	virtual double GetStoragePercentage() const JMP_THIS(0x708BC0);
	virtual int GetPipFillLevel() const JMP_THIS(0x708C30);
	virtual int GetRefund() const JMP_THIS(0x70ADA0);
	virtual int GetThreatValue() const JMP_THIS(0x708B40);
	virtual bool vt_entry_2C4(DWORD dwUnk) { return true; }
	virtual DWORD vt_entry_2C8(DWORD dwUnk1, DWORD dwUnk2) JMP_THIS(0x6FDA00);
	virtual bool CanReachLocation(const CoordStruct& crd) JMP_THIS(0x707F60);	//From dp
	virtual int GetCrewCount() const JMP_THIS(0x6F3950) //??? bool Crewed
	virtual int GetAntiAirValue() const { return 0; }
	virtual int GetAntiArmorValue() const { return 0; }
	virtual int GetAntiInfantryValue() const { return 0; }
	virtual void GotHijacked() { }
	virtual int SelectWeapon(AbstractClass *pTarget) const JMP_THIS(0x6F3330);
	virtual int SelectNavalTargeting(AbstractClass *pTarget) const JMP_THIS(0x6F3820);
	virtual int GetZAdjustment() const JMP_THIS(0x704350);
	virtual ZGradient GetZGradient() const { return ZGradient::Deg90; }
	virtual CellStruct* GetSomeCellStruct(CellStruct* buffer) const JMP_THIS(0x459DB0);
	virtual void SetSomeCellStruct(CellStruct cell) { }
	virtual CellStruct* vt_entry_2FC(CellStruct* Buffer, DWORD dwUnk1, DWORD dwUnk2) const JMP_THIS(0x70AD50);
	virtual CoordStruct* vt_entry_300(CoordStruct* Buffer, DWORD dwUnk) const JMP_THIS(0x6F3D60);
	virtual DWORD vt_entry_304(DWORD dwUnk1, DWORD dwUnk2) const JMP_THIS(0x708C10);
	virtual DirStruct* GetRealFacing(DirStruct* pBuffer) const JMP_THIS(0x708D70);
	virtual InfantryTypeClass* GetCrew() const JMP_THIS(0x707D20);
	virtual bool vt_entry_310() const JMP_THIS(0x700D10);
	virtual bool CanDeploySlashUnload() const JMP_THIS(0x700D50);
	virtual int GetROF(int nWeapon) const JMP_THIS(0x6FCFA0);
	virtual int GetGuardRange(int dwUnk) const JMP_THIS(0x707E60);
	virtual bool vt_entry_320() const { return false; }
	virtual bool IsRadarVisible(int* pOutDetection) const JMP_THIS(0x70D1D0); // out value will be set to 1 if unit is cloaked and 2 if it is subterranean, otherwise it's unchanged
	virtual bool IsSensorVisibleToPlayer() const JMP_THIS(0x70D420);
	virtual bool IsSensorVisibleToHouse(HouseClass *House) const JMP_THIS(0x70D460);
	virtual bool IsEngineer() const { return false; }
	virtual void ProceedToNextPlanningWaypoint() { }
	virtual DWORD ScanForTiberium(DWORD dwUnk1, DWORD dwUnk2, DWORD dwUnk3) const JMP_THIS(0x70F8F0);
	virtual bool EnterGrinder() { return false; }
	virtual bool EnterBioReactor() { return false; }
	virtual bool EnterTankBunker() { return false; }
	virtual bool EnterBattleBunker() { return false; }
	virtual bool GarrisonStructure() { return false; }
	virtual bool IsPowerOnline() const { return false; }
	virtual void QueueVoice(int idxVoc) JMP_THIS(0x708D90);
	virtual int VoiceEnter() JMP_THIS(0x709020);
	virtual int VoiceHarvest() JMP_THIS(0x709060);
	virtual int VoiceSelect() JMP_THIS(0x708EB0);
	virtual int VoiceCapture() JMP_THIS(0x708DC0);
	virtual int VoiceMove() JMP_THIS(0x708FC0);
	virtual int VoiceDeploy() JMP_THIS(0x708E00);
	virtual int VoiceAttack(ObjectClass *pTarget) JMP_THIS(0x7090A0);
	virtual bool ClickedEvent(NetworkEvents event) JMP_THIS(0x6FFE00);

	// depending on the mission you click, cells/Target are not always needed
	virtual bool ClickedMission(Mission Mission, ObjectClass *pTarget, CellClass * TargetCell, CellClass *NearestTargetCellICanEnter) JMP_THIS(0x6FFBE0);
	virtual bool IsUnderEMP() const JMP_THIS(0x70EFD0)
	virtual bool IsParalyzed() const { return false; }
	virtual bool CanCheer() const { return false; }
	virtual void Cheer(bool force) { }
	virtual int GetDefaultSpeed() const JMP_THIS(0x70EFE0);
	virtual void DecreaseAmmo() JMP_THIS(0x70D670)
	virtual void AddPassenger(FootClass* pPassenger) JMP_THIS(0x710670);
	virtual bool CanDisguiseAs(AbstractClass* pTarget) const JMP_THIS(0x70EF00);
	virtual bool TargetAndEstimateDamage(DWORD dwUnk1, DWORD dwUnk2) JMP_THIS(0x709820); //dwUnk1 CoordStruct*?
	virtual void Stun() JMP_THIS(0x6FCD40);
	virtual bool TriggersCellInset(AbstractClass *pTarget) JMP_THIS(0x6F7660);
	virtual bool IsCloseEnough(AbstractClass *pTarget, int idxWeapon) const JMP_THIS(0x6F77B0);
	virtual bool IsCloseEnoughToAttack(AbstractClass* pTarget) const JMP_THIS(0x6F7780);
	virtual bool IsCloseEnoughToAttackCoords(const CoordStruct& Coords) const JMP_THIS(0x6F7930);
	virtual bool vt_entry_3B4(AbstractClass* pTarget) const JMP_THIS(0x6F78D0);
	virtual void Destroyed(ObjectClass* pKiller) = 0;
	virtual FireError GetFireErrorWithoutRange(AbstractClass* pTarget, int nWeaponIndex) const { return this->GetFireError(pTarget, nWeaponIndex, false); }
	virtual FireError GetFireError(AbstractClass *pTarget, int nWeaponIndex, bool ignoreRange) const JMP_THIS(0x6FC0B0);
	virtual CellClass* SelectAutoTarget(TargetFlags TargetFlags, int CurrentThreat, bool OnlyTargetHouseEnemy) JMP_THIS(0x6F8DF0);
	virtual void SetTarget(AbstractClass *pTarget) JMP_THIS(0x6FCDB0);
	virtual BulletClass* Fire(AbstractClass* pTarget, int nWeaponIndex) JMP_THIS(0x6FDD50);
	// clears target and destination and puts in guard mission
	virtual void Guard() JMP_THIS(0x70F850);
	virtual bool SetOwningHouse(HouseClass* pHouse, bool announce = true) JMP_THIS(0x7014A0);
	virtual void vt_entry_3D8(DWORD dwUnk1, DWORD dwUnk2, DWORD dwUnk3) JMP_THIS(0x70B280);
	virtual bool Crash(ObjectClass* Killer) { return false; }
	virtual bool IsAreaFire() const	JMP_THIS(0x70DD50);
	virtual int IsNotSprayAttack1() const { return !this->GetTechnoType()->SprayAttack; }
	virtual int vt_entry_3E8() const { return 1; }
	virtual int IsNotSprayAttack2() const { return !this->GetTechnoType()->SprayAttack; }
	virtual WeaponStruct* GetDeployWeapon() const JMP_THIS(0x70E120)//{ return this->GetWeapon(this->IsNotSprayAttack1()); }
	virtual WeaponStruct* GetTurretWeapon() const JMP_THIS(0x70E1A0);
	virtual WeaponStruct* GetWeapon(int nWeaponIndex) const	JMP_THIS(0x70E140);
	virtual bool HasTurret() const JMP_THIS(0x41BFA0);
	virtual bool CanOccupyFire() const { return false; }
	virtual int GetOccupyRangeBonus() const { return 0; }
	virtual int GetOccupantCount() const { return 0; }
	virtual void OnFinishRepair() JMP_THIS(0x701410);
	virtual void UpdateCloak(bool bUnk = 1) JMP_THIS(0x6FB740);
	virtual void CreateGap() JMP_THIS(0x6FB170);
	virtual void DestroyGap() JMP_THIS(0x6FB470);
	virtual void vt_entry_41C() JMP_THIS(0x70B570);
	virtual void Sensed() JMP_THIS(0x6F4EB0);
	virtual void Reload() JMP_THIS(0x6FB010);
	virtual void vt_entry_428() { }
	// Returns target's coordinates if on attack mission & have target, otherwise own coordinates.
	virtual CoordStruct* GetAttackCoordinates(CoordStruct* pCrd) const JMP_THIS(0x705CA0);
	virtual bool IsNotWarpingIn() const { return !this->IsWarpingIn(); }
	virtual bool vt_entry_434(DWORD dwUnk) const { return false; }
	virtual void DrawActionLines(bool Force, DWORD dwUnk) { }
	virtual DWORD GetDisguiseFlags(DWORD existingFlags) const JMP_THIS(0x70ED80);
	virtual bool IsClearlyVisibleTo(HouseClass *House) const JMP_THIS(0x70EE30); // can House see right through my disguise?
	virtual void DrawVoxel(
		const VoxelStruct& Voxel,
		DWORD dwUnk2,
		short Facing,
		const IndexClass<VoxelIndexKey, VoxelCacheStruct*>& VoxelIndex,
		const RectangleStruct& Rect,
		const Point2D& Location,
		const Matrix3D& Matrix,
		int Intensity,
		DWORD dwUnk9,
		DWORD dwUnk10) JMP_THIS(0x706640);
	virtual void vt_entry_448(DWORD dwUnk1, DWORD dwUnk2) { }
	virtual void DrawHealthBar(Point2D *pLocation, RectangleStruct *pBounds, bool bUnk) const JMP_THIS(0x6F64A0);
	virtual void DrawPipScalePips(Point2D *pLocation, Point2D *pOriginalLocation, RectangleStruct *pBounds) const JMP_THIS(0x709A90);
	virtual void DrawVeterancyPips(Point2D *pLocation, RectangleStruct *pBounds) const JMP_THIS(0x70A990);
	virtual void DrawExtraInfo(Point2D const& location, Point2D const& originalLocation, RectangleStruct const& bounds) const JMP_THIS(0x70AA60);
	virtual void Uncloak(bool bPlaySound) JMP_THIS(0x7036C0);
	virtual void Cloak(bool bPlaySound) JMP_THIS(0x703770);
	virtual DWORD vt_entry_464(DWORD dwUnk) const JMP_THIS(0x70D190);
	virtual void UpdateRefinerySmokeSystems() { }
	virtual void DisguiseAs(AbstractClass* pTarget) JMP_THIS(0x70E280);
	virtual void ClearDisguise() JMP_THIS(0x41C030)
	virtual bool IsItTimeForIdleActionYet() const JMP_THIS(0x7099E0);
	virtual bool UpdateIdleAction() { return false; }
	virtual void vt_entry_47C(AbstractClass* dwUnk) { }
	virtual void SetDestination(AbstractClass* pDest, bool bUnk) { }
	virtual bool EnterIdleMode(DWORD dwUnk1, DWORD dwUnk2) JMP_THIS(0x709A40);
	virtual void UpdateSight(DWORD dwUnk1, DWORD dwUnk2, DWORD dwUnk3, DWORD dwUnk4, DWORD dwUnk5) JMP_THIS(0x70AF50);
	virtual void vt_entry_48C(DWORD dwUnk1, DWORD dwUnk2, DWORD dwUnk3, DWORD dwUnk4) JMP_THIS(0x70B1D0);
	virtual bool ForceCreate(CoordStruct& coord, DWORD dwUnk = 0) = 0;
	virtual void RadarTrackingStart() JMP_THIS(0x70CC90);
	virtual void RadarTrackingStop() JMP_THIS(0x70CCC0);
	virtual void RadarTrackingFlash() JMP_THIS(0x70CCF0);
	virtual void RadarTrackingUpdate(bool bUnk) JMP_THIS(0x70D990);
	virtual void vt_entry_4A4(DWORD dwUnk) JMP_THIS(0x70F000);
	virtual void vt_entry_4A8() { }
	virtual bool vt_entry_4AC() const { return false; }
	virtual bool vt_entry_4B0() const { return false; }
	virtual int vt_entry_4B4() const { return -1; }
	virtual CoordStruct* vt_entry_4B8(CoordStruct* pCrd) { *pCrd = { -1,-1,-1 }; return pCrd; }
	virtual bool CanUseWaypoint() const { return this->GetTechnoType()->CanUseWaypoint(); }
	virtual bool CanAttackOnTheMove() const JMP_THIS(0x70F090)
	virtual bool vt_entry_4C4() const { return false; }
	virtual bool vt_entry_4C8() { return false; }
	virtual void vt_entry_4CC() { }
	virtual bool vt_entry_4D0() { return false; }

	//non-virtual
	bool sub_703B10() const JMP_THIS(0x703B10);
	int sub_703E70() const JMP_THIS(0x703E70);
	int sub_704000() const JMP_THIS(0x704000);
	int sub_704240() const JMP_THIS(0x704240);
	bool sub_70D8F0() JMP_THIS(0x70D8F0);
	bool sub_70DCE0() const { return this->CurrentTurretNumber != -1; }
	//void Draw_A_SHP() JMP_THIS(0x705E00);

	bool IsDrainSomething()
		{ return this->DrainTarget != nullptr; }

	int GetCurrentTurretNumber() const
		{ JMP_THIS(0x70DCF0); }

	bool HasMultipleTurrets() const
		{ JMP_THIS(0x70DC60); }

	bool IsDeactivated() const
		{ JMP_THIS(0x70FBD0); }

	// (re-)starts the reload timer
	void StartReloading()
		{ JMP_THIS(0x6FB080); }

	bool ShouldSuppress(CellStruct *coords) const
		{ JMP_THIS(0x6F79A0); }

	// smooth operator
	const char* get_ID() const {
		auto const pType = this->GetType();
		return pType ? pType->get_ID() : nullptr;
	}

	int TimeToBuild() const
		{ JMP_THIS(0x6F47A0); }

	bool IsMindControlled() const
		{ JMP_THIS(0x7105E0); }

	bool CanBePermaMindControlled() const
		{ JMP_THIS(0x53C450); }

	LaserDrawClass* CreateLaser(ObjectClass *pTarget, int idxWeapon, WeaponTypeClass *pWeapon, const CoordStruct &Coords)
		{ JMP_THIS(0x6FD210); }

	// DP-Kratos
	EBolt* Electric_Zap(AbstractClass *pTarget, WeaponTypeClass *pWeapon, const CoordStruct &Coords)
		{ JMP_THIS(0x6FD460); }

	/*
	 *  Cell->AddThreat(this->Owner, -this->ThreatPosed);
	 *  this->ThreatPosed = 0;
	 *  int Threat = this->CalculateThreat(); // this is another gem of a function, to be revealed another time...
	 *  this->ThreatPosed = Threat;
	 *  Cell->AddThreat(this->Owner, Threat);
	 */
	void UpdateThreatInCell(CellClass *Cell)
		{ JMP_THIS(0x70F6E0); }

// CanTargetWhatAmI is a bitfield, if(!(CanTargetWhatAmI & (1 << tgt->WhatAmI())) { fail; }

// slave of the next one
	bool CanAutoTargetObject(
		TargetFlags targetFlags,
		int canTargetWhatAmI,
		int wantedDistance,
		TechnoClass* pTarget,
		int* pThreatPosed,
		DWORD dwUnk,
		CoordStruct* pSourceCoords) const
			{ JMP_THIS(0x6F7CA0); }

// called by AITeam Attack Target Type and autoscan
	bool TryAutoTargetObject(
		TargetFlags targetFlags,
		int canTargetWhatAmI,
		CellStruct* pCoords,
		DWORD dwUnk1,
		DWORD* dwUnk2,
		int* pThreatPosed,
		DWORD dwUnk3)
			{ JMP_THIS(0x6F8960); }

	void Reactivate()
		{ JMP_THIS(0x70FBE0); }

	void Deactivate()
		{ JMP_THIS(0x70FC90); }


	// this should be the transport, but it's unused
	// marks passenger as "InOpenTopped" for targeting, range scanning and other purposes
	void EnteredOpenTopped(TechnoClass* pWho)
		{ JMP_THIS(0x710470); }

	// this should be the transport, but it's unused
	// reverses the above
	void ExitedOpenTopped(TechnoClass* pWho)
		{ JMP_THIS(0x7104A0); }

	// called when the source unit dies - passengers are about to get kicked out, this basically calls ->ExitedOpenTransport on each passenger
	void MarkPassengersAsExited()
		{ JMP_THIS(0x7104C0); }

	// for gattlings
	void SetCurrentWeaponStage(int idx)
		{ JMP_THIS(0x70DDD0); }

	void SetFocus(AbstractClass* pFocus)
		{ JMP_THIS(0x70C610); }

	void DrawObject(
		SHPStruct* pSHP,
		int nFrame,
		Point2D* pLocation,
		RectangleStruct* pBounds,
		int,
		int,
		int nZAdjust,
		ZGradient eZGradientDescIdx,
		int,
		int nBrightness,
		int TintColor,
		SHPStruct* pZShape,
		int nZFrame,
		int nZOffsetX,
		int nZOffsetY,
		int);

	void SetGattlingValue(int value)
		{ JMP_THIS(0x70DE00); }

	int __fastcall ClearPlanningTokens(NetworkEvent* pEvent)
		{ JMP_STD(0x6386E0); }

	void SetTargetForPassengers(AbstractClass* pTarget)
		{ JMP_THIS(0x710550); }

	// returns the house that created this object (factoring in Mind Control)
	HouseClass * GetOriginalOwner()
		{ JMP_THIS(0x70F820); }

	void FireDeathWeapon(int additionalDamage)
		{ JMP_THIS(0x70D690); }

	bool HasAbility(Ability ability) const
		{ JMP_THIS(0x70D0D0); }

	void ClearSidebarTabObject() const
		{ JMP_THIS(0x734270); }

	//TechnoClass::GetRemapColour
	LightConvertClass* GetDrawer() const
		{ JMP_THIS(0x705D70); }

	bool TypeIs(const char* pID) const
	{
		//AbstractTypeClass* pType = this->GetType();
		//return pType->IsThis(pID);
		JMP_THIS(0x5F3E50);
	}

	void DrainBuilding(BuildingClass* pBuilding)
	{ JMP_THIS(0x70FD70); }

	//FirePowerMultiplier, ArmorMultiplier
	int CalculateDamage(TechnoClass* pTarget, WeaponTypeClass* pWeapon) const
	{ JMP_THIS(0x6FDB80); }

	CoordStruct* sub_70BCB0(CoordStruct* pCrd)
	{ JMP_THIS(0x70BCB0); }

	void KillPassengers(TechnoClass* pSource)
	{ JMP_THIS(0x707CB0); }

	void UpdateIronTint()
	{ JMP_THIS(0x70E5A0); }

	void UpdateAirStrikeTint()
	{ JMP_THIS(0x70E920); }

	void SetTurrent(int turrentIdx)
	{ JMP_THIS(0x70DC70); }

	int GetEffectTintIntensity(int currentIntensity)
	{ JMP_THIS(0x70E360); }

	int GetInvulnerabilityTintIntensity(int currentIntensity)
	{ JMP_THIS(0x70E380); }

	int GetAirstrikeTintIntensity(int currentIntensity)
	{ JMP_THIS(0x70E4B0); }

	int CombatDamage(int nWeaponIndex) const
	{ JMP_THIS(0x6F3970); }

	WeaponStruct* GetPrimaryWeapon() const
	{ JMP_THIS(0x70E1A0); }

	int GetIonCannonValue(AIDifficulty difficulty) const;

	int GetIonCannonValue(AIDifficulty difficulty, int maxHealth) const
	{
		// what TS does
		if(maxHealth > 0 && this->Health > maxHealth) {
			return (this->WhatAmI() == AbstractType::Building) ? 3 : 1;
		}

		return this->GetIonCannonValue(difficulty);
	}

	CoordStruct GetFLH(int idxWeapon, const CoordStruct& base) const
	{
		CoordStruct ret;
		this->GetFLH(&ret, idxWeapon, base);
		return ret;
	}

	DirStruct TurretFacing() const
	{
		DirStruct ret;
		this->TurretFacing(&ret);
		return ret;
	}

	DirStruct GetRealFacing() const
	{
		DirStruct ret;
		this->GetRealFacing(&ret);
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

	//Constructor
	TechnoClass(HouseClass* pOwner) noexcept
		: TechnoClass(noinit_t())
	{ JMP_THIS(0x6F2B40); }

protected:
	explicit __forceinline TechnoClass(noinit_t) noexcept
		: RadioClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	DECLARE_PROPERTY(FlashData, Flashing);
	DECLARE_PROPERTY(StageClass, Animation); // how the unit animates
	DECLARE_PROPERTY(PassengersClass, Passengers);
	TechnoClass*     Transporter; // unit carrying me
	int              unknown_int_120;
	int              CurrentTurretNumber; // for IFV/gattling/charge turrets
	int              unknown_int_128;
	AnimClass*       BehindAnim;
	AnimClass*       DeployAnim;
	bool             InAir;
	int              CurrentWeaponNumber; // for IFV/gattling
	Rank             CurrentRanking; // only used for promotion detection
	int              CurrentGattlingStage;
	int              GattlingValue; // sum of RateUps and RateDowns
	int              TurretAnimFrame;
	HouseClass*      InitialOwner; // only set in ctor
	DECLARE_PROPERTY(VeterancyStruct, Veterancy);
	PROTECTED_PROPERTY(DWORD, align_154);
	double           ArmorMultiplier;
	double           FirepowerMultiplier;
	DECLARE_PROPERTY(CDTimerClass, IdleActionTimer); // MOO
	DECLARE_PROPERTY(CDTimerClass, RadarFlashTimer);
	DECLARE_PROPERTY(CDTimerClass, TargetingTimer); //Duration = 45 on init!
	DECLARE_PROPERTY(CDTimerClass, IronCurtainTimer);
	DECLARE_PROPERTY(CDTimerClass, IronTintTimer); // how often to alternate the effect color
	int              IronTintStage; // ^
	DECLARE_PROPERTY(CDTimerClass, AirstrikeTimer);
	DECLARE_PROPERTY(CDTimerClass, AirstrikeTintTimer); // tracks alternation of the effect color
	DWORD            AirstrikeTintStage; //  ^
	int              ForceShielded;	//0 or 1, NOT a bool - is this under ForceShield as opposed to IC?
	bool             Deactivated; //Robot Tanks without power for instance
	TechnoClass*     DrainTarget; // eg Disk -> PowerPlant, this points to PowerPlant
	TechnoClass*     DrainingMe;  // eg Disk -> PowerPlant, this points to Disk
	AnimClass*       DrainAnim;
	bool             Disguised;
	DWORD            DisguiseCreationFrame;
	DECLARE_PROPERTY(CDTimerClass, InfantryBlinkTimer); // Rules->InfantryBlinkDisguiseTime , detects mirage firing per description
	DECLARE_PROPERTY(CDTimerClass, DisguiseBlinkTimer); // disguise disruption timer
	bool             UnlimboingInfantry;
	DECLARE_PROPERTY(CDTimerClass, ReloadTimer);
	DWORD            unknown_208;
	DWORD            unknown_20C;

	// WARNING! this is actually an index of HouseTypeClass es, but it's being changed to fix typical WW bugs.
	DECLARE_PROPERTY(IndexBitfield<HouseClass *>, DisplayProductionTo); // each bit corresponds to one player on the map, telling us whether that player has (1) or hasn't (0) spied this building, and the game should display what's being produced inside it to that player. The bits are arranged by player ID, i.e. bit 0 refers to house #0 in HouseClass::Array, 1 to 1, etc.; query like ((1 << somePlayer->ArrayIndex) & someFactory->DisplayProductionToHouses) != 0

	int              Group; //0-9, assigned by CTRL+Number, these kinds // also set by aimd TeamType->Group !
	AbstractClass*   Focus; // when told to guard a unit or such; distinguish undeploy and selling
	HouseClass*      Owner;
	CloakState       CloakState;
	DECLARE_PROPERTY(StageClass, CloakProgress); // phase from [opaque] -> [fading] -> [transparent] , [General]CloakingStages= long
	DECLARE_PROPERTY(CDTimerClass, CloakDelayTimer); // delay before cloaking again
	float            WarpFactor; // don't ask! set to 0 in CTOR, never modified, only used as ((this->Fetch_ID) + this->WarpFactor) % 400 for something in cloak ripple
	bool             unknown_bool_250;
	CoordStruct      LastSightCoords;
	int              LastSightRange;
	int              LastSightHeight;
	bool             GapSuperCharged; // GapGenerator, when SuperGapRadiusInCells != GapRadiusInCells, you can deploy the gap to boost radius
	bool             GeneratingGap; // is currently generating gap
	int              GapRadius;
	bool             BeingWarpedOut; // is being warped by CLEG
	bool             WarpingOut; // phasing in after chrono-jump
	bool             unknown_bool_272;
	BYTE             unused_273;
	TemporalClass*   TemporalImUsing; // CLEG attacking Power Plant : CLEG's this
	TemporalClass*   TemporalTargetingMe; 	// CLEG attacking Power Plant : PowerPlant's this
	bool             IsImmobilized; // by chrono aftereffects
	DWORD            unknown_280;
	int              ChronoLockRemaining; // countdown after chronosphere warps things around
	CoordStruct      ChronoDestCoords; // teleport loco and chsphere set this
	AirstrikeClass*  Airstrike; //Boris
	bool             Berzerk;
	DWORD            BerzerkDurationLeft;
	DWORD            SprayOffsetIndex; // hardcoded array of xyz offsets for sprayattack, 0 - 7, see 6FE0AD
	bool             Uncrushable; // DeployedCrushable fiddles this, otherwise all 0

	// unless source is Pushy=
	// abs_Infantry source links with abs_Unit target and vice versa - can't attack others until current target flips
	// no checking whether source is Infantry, but no update for other types either
	// old Brute hack
	FootClass*       DirectRockerLinkedUnit;
	FootClass*       LocomotorTarget; // mag->LocoTarget = victim
	FootClass*       LocomotorSource; // victim->LocoSource = mag
	AbstractClass*   Target; //if attacking
	AbstractClass*   LastTarget;
	CaptureManagerClass* CaptureManager; //for Yuris
	TechnoClass*     MindControlledBy;
	bool             MindControlledByAUnit;
	AnimClass*       MindControlRingAnim;
	HouseClass*      MindControlledByHouse; //used for a TAction
	SpawnManagerClass* SpawnManager;
	TechnoClass*     SpawnOwner; // on DMISL , points to DRED and such
	SlaveManagerClass* SlaveManager;
	TechnoClass*     SlaveOwner; // on SLAV, points to YAREFN
	HouseClass*      OriginallyOwnedByHouse; //used for mind control

		//units point to the Building bunkering them, building points to Foot contained within
	TechnoClass*     BunkerLinkedItem;

	float            PitchAngle; // not exactly, and it doesn't affect the drawing, only internal state of a dropship
	DECLARE_PROPERTY(CDTimerClass, DiskLaserTimer);
	int				 ROF;
	int              Ammo;
	int              Value; // set to actual cost when this gets queued in factory, updated only in building's 42C


	ParticleSystemClass* FireParticleSystem;
	ParticleSystemClass* SparkParticleSystem;
	ParticleSystemClass* NaturalParticleSystem;
	ParticleSystemClass* DamageParticleSystem;
	ParticleSystemClass* RailgunParticleSystem;
	ParticleSystemClass* unk1ParticleSystem;
	ParticleSystemClass* unk2ParticleSystem;
	ParticleSystemClass* FiringParticleSystem;

	WaveClass*       Wave; //Beams


	// rocking effect
	float            AngleRotatedSideways; // in this frame, in radians - if abs() exceeds pi/2, it dies
	float            AngleRotatedForwards; // same

	// set these and leave the previous two alone!
	// if these are set, the unit will roll up to pi/4, by this step each frame, and balance back
	float            RockingSidewaysPerFrame; // left to right - positive pushes left side up
	float            RockingForwardsPerFrame; // back to front - positive pushes ass up

	int              HijackerInfantryType; // mutant hijacker

	DECLARE_PROPERTY(StorageClass, Tiberium);
	DWORD            unknown_34C;

	DECLARE_PROPERTY(TransitionTimer, UnloadTimer); // times the deploy, unload, etc. cycles

	DECLARE_PROPERTY(FacingClass, BarrelFacing);
	DECLARE_PROPERTY(FacingClass, PrimaryFacing);
	DECLARE_PROPERTY(FacingClass, SecondaryFacing);
	int              CurrentBurstIndex;
	DECLARE_PROPERTY(CDTimerClass, TargetLaserTimer);
	short            unknown_short_3C8;
	WORD             unknown_3CA;
	bool             CountedAsOwned; // is this techno contained in OwningPlayer->Owned... counts?
	bool             IsSinking;
	bool             WasSinkingAlready; // if(IsSinking && !WasSinkingAlready) { play SinkingSound; WasSinkingAlready = 1; }
	bool             unknown_bool_3CF;
	bool             unknown_bool_3D0;
	bool             HasBeenAttacked; // ReceiveDamage when not HouseClass_IsAlly
	bool             Cloakable;
	bool             IsPrimaryFactory; // doubleclicking a warfac/barracks sets it as primary
	bool             Spawned;
	bool             IsInPlayfield;
	DECLARE_PROPERTY(RecoilData, TurretRecoil);
	DECLARE_PROPERTY(RecoilData, BarrelRecoil);
	bool             unknown_bool_418;
	bool             unknown_bool_419;
	bool             IsOwnedByCurrentPlayer; // Returns true if owned by the player on this computer
	bool             DiscoveredByCurrentPlayer;
	bool             DiscoveredByComputer;
	bool             unknown_bool_41D;
	bool             unknown_bool_41E;
	bool             unknown_bool_41F;
	char             SightIncrease; // used for LeptonsPerSightIncrease
	bool             RecruitableA; // these two are like Lenny and Carl, weird purpose and never seen separate
	bool             RecruitableB; // they're usually set on preplaced objects in maps
	bool             IsRadarTracked;
	bool             IsOnCarryall;
	bool             IsCrashing;
	bool             WasCrashingAlready;
	bool             IsBeingManipulated;
	TechnoClass*     BeingManipulatedBy; // set when something is being molested by a locomotor such as magnetron the pointee will be marked as the killer of whatever the victim falls onto
	HouseClass*      ChronoWarpedByHouse;
	bool             unknown_bool_430;
	bool             IsMouseHovering;
	bool             unknown_bool_432;
	TeamClass*       OldTeam;
	bool             CountedAsOwnedSpecial; // for absorbers, infantry uses this to manually control OwnedInfantry count
	bool             Absorbed; // in UnitAbsorb/InfantryAbsorb or smth, lousy memory
	bool             unknown_bool_43A;
	DWORD            unknown_43C;
	DECLARE_PROPERTY(DynamicVectorClass<int>, CurrentTargetThreatValues);
	DECLARE_PROPERTY(DynamicVectorClass<AbstractClass*>, CurrentTargets);

	// if DistributedFire=yes, this is used to determine which possible targets should be ignored in the latest threat scan
	DECLARE_PROPERTY(DynamicVectorClass<AbstractClass*>, AttackedTargets);

	DECLARE_PROPERTY(AudioController, Audio3);

	BOOL            unknown_BOOL_49C;
	BOOL            TurretIsRotating;

	DECLARE_PROPERTY(AudioController, Audio4);

	bool             unknown_bool_4B8;
	DWORD            unknown_4BC;

	DECLARE_PROPERTY(AudioController, Audio5);

	bool             unknown_bool_4D4;
	DWORD            unknown_4D8;

	DECLARE_PROPERTY(AudioController, Audio6);

	DWORD            QueuedVoiceIndex;
	DWORD            unknown_4F4;
	bool             unknown_bool_4F8;
	DWORD            unknown_4FC;	//gets initialized with the current Frame, but this is NOT a TimerStruct!
	DWORD            unknown_500;
	DWORD            EMPLockRemaining;
	DWORD            ThreatPosed; // calculated to include cargo etc
	DWORD            ShouldLoseTargetNow;
	RadBeam*         FiringRadBeam;
	PlanningTokenClass* PlanningToken;
	ObjectTypeClass* Disguise;
	HouseClass*      DisguisedAsHouse;
};
