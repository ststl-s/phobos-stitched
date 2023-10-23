/*
	Projectiles
*/

#pragma once

#include <ObjectClass.h>
#include <BulletTypeClass.h>

class TechnoClass;
class ObjectClass;
class WarheadTypeClass;

struct BulletData
{
	CDTimerClass UnknownTimer;
	CDTimerClass ArmTimer;
	CoordStruct Location;
	int Distance;
};

// the velocities along the axes, or something like that
using BulletVelocity = Vector3D<double>; // :3 -pd

class NOVTABLE BulletClass : public ObjectClass
{
public:
	static const AbstractType AbsID = AbstractType::Bullet;

	//Array
	static constexpr constant_ptr<DynamicVectorClass<BulletClass*>, 0xA8ED40u> const Array{};

	static constexpr reference<DynamicVectorClass<BulletClass*>, 0x89DE18u> const ScalableBullets{};

	//IUnknown
	virtual ULONG __stdcall AddRef() override JMP_STD(0x46AFD0);
	virtual ULONG __stdcall Release() override JMP_STD(0x46AFF0);

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override JMP_STD(0x46B560);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x46AE70);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override JMP_STD(0x46AFB0);

	//Destructor
	virtual ~BulletClass() override RX;// JMP_THIS(0x46B5C0);

	//AbstractClass
	virtual void PointerExpired(AbstractClass* pAbstract, bool removed) override JMP_THIS(0x4684E0);
	virtual AbstractType WhatAmI() const override { return AbstractType::Bullet; }
	virtual int Size() const override { return 0x160; }
	virtual void Update() override JMP_THIS(0x4666E0);

	//ObjectClass
	virtual Layer InWhichLayer() const override JMP_THIS(0x468B90);
	virtual ObjectTypeClass* GetType() const override { return this->Type; }
	virtual CellStruct const* GetFoundationData(bool includeBib = false) const override JMP_THIS(0x466660);
	virtual void DrawIt(Point2D* pLocation, RectangleStruct* pBounds) const override JMP_THIS(0x468090);
	virtual bool UpdatePlacement(PlacementType value) override JMP_THIS(0x4666C0);

	//BulletClass
	virtual BYTE GetAnimFrame() const JMP_THIS(0x468000);
	virtual void SetTarget(AbstractClass* pTarget) JMP_THIS(0x46B5A0);
	virtual bool MoveTo(const CoordStruct& dest, const BulletVelocity& velocity) JMP_THIS(0x468670);

	// non-virtual
	// after CoCreateInstance creates a bullet, this configures it
	void Construct
	(
		BulletTypeClass* pType,
		AbstractClass* pTarget,
		TechnoClass* pOwner,
		int damage,
		WarheadTypeClass* pWarhead,
		int speed,
		bool bright
	)
	{ JMP_THIS(0x4664C0); }

	bool MoveTo(const CoordStruct& dest, const Vector3D<int>& velocity)
	{
		this->MoveTo(dest, velocity.X, velocity.Y, velocity.Z);
	}

	bool MoveTo(const CoordStruct& dest, double xVelocity, double yVelocity, double zVelocity)
	{
		this->MoveTo(dest, Vector3D<double>(xVelocity, yVelocity, zVelocity));
	}

	// calls Detonate with the appropriate coords
	void Explode(bool destroy = false)
	{ JMP_THIS(0x468D80); }

	// detonate the bullet at specific coords
	void Detonate(const CoordStruct& coords)
	{ JMP_THIS(0x4690B0); }

	// spawns off the proper amount of shrapnel projectiles
	void Shrapnel()
	{ JMP_THIS(0x46A310); }

	static void ApplyRadiationToCell(CellStruct cell, int radius, int amount)
	{ JMP_STD(0x46ADE0); }

	// this bullet will miss and hit the ground instead.
	// if the original target is in air, it will disappear.
	void LoseTarget()
	{ JMP_THIS(0x468430); }

	bool IsHoming() const
	{ return this->Type->ROT > 0; }

	void SetWeaponType(WeaponTypeClass *pWeapon)
	{ this->WeaponType = pWeapon; }

	WeaponTypeClass * GetWeaponType() const
	{ return this->WeaponType; }

	// only called in UnitClass::Fire if Type->Scalable
	void InitScalable()
	{ JMP_THIS(0x46B280); }

	// call only after the target, args, etc., have been set
	void NukeMaker()
	{ JMP_THIS(0x46B310); }

	// helpers
	CoordStruct GetTargetCoords() const
	{
		if (this->Target)
			return this->Target->GetCoords();
		else
			return this->GetCoords();
	}

	//Constructor
protected:
	BulletClass() noexcept
		: BulletClass(noinit_t())
	{ JMP_THIS(0x466380); }

protected:
	explicit __forceinline BulletClass(noinit_t) noexcept
		: ObjectClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	BulletTypeClass* Type;
	TechnoClass* Owner;
	bool unknown_B4;
	PROTECTED_PROPERTY(BYTE, align_B5[3]);
	BulletData Data;
	bool Bright;
	PROTECTED_PROPERTY(BYTE, align_E1[3]);
	DWORD unknown_E4;
	BulletVelocity Velocity;
	DWORD unknown_100;
	bool unknown_104;
	PROTECTED_PROPERTY(BYTE, align_105[3]);
	DWORD unknown_108;
	AbstractClass* Target;
	int Speed;
	int InheritedColor;
	DWORD unknown_118;
	DWORD unknown_11C;
	double unknown_120;
	WarheadTypeClass* WH;
	byte AnimFrame;
	byte AnimRateCounter;
	PROTECTED_PROPERTY(BYTE, align_12E[2]);
	WeaponTypeClass* WeaponType;
	CoordStruct SourceCoords;
	CoordStruct TargetCoords;
	CellStruct LastMapCoords;
	int DamageMultiplier;
	AnimClass* NextAnim;
	bool SpawnNextAnim;
	PROTECTED_PROPERTY(BYTE, align_159[3]);
	int Range;
};
