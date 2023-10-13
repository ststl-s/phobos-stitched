/*
	Projectiles
*/

#pragma once

#include <ObjectTypeClass.h>

//forward declarations
class AnimTypeClass;
class BulletClass;
class ColorScheme;
class CellClass;
class TechnoClass;
class WeaponTypeClass;
class WarheadTypeClass;

class NOVTABLE BulletTypeClass : public ObjectTypeClass
{
public:
	static const AbstractType AbsID = AbstractType::BulletType;

	//Array
	ABSTRACTTYPE_ARRAY(BulletTypeClass, 0xA83C80u);

	//static
	//static BulletTypeClass* __fastcall FindOrAllocate(const char* pID) JMP_STD(0x46C790);

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override JMP_STD(0x46C750);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x46C6A0);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override JMP_STD(0x46C730);

	//Destructor
	virtual ~BulletTypeClass() override RX;// JMP_THIS(0x46C890);

	//AbstractClass
	virtual void PointerExpired(AbstractClass* pAbstract, bool removed) override JMP_THIS(0x46C820);
	virtual AbstractType WhatAmI() const override { return AbstractType::BulletType; }
	virtual int Size() const override { return 0x2F8; }
	virtual void ComputeCRC(CRCEngine& crc) const override JMP_THIS(0x46C560);

	//AbstractTypeClass
	virtual bool LoadFromINI(CCINIClass* pINI) override JMP_THIS(0x46BEE0);

	//ObjectTypeClass
	virtual CoordStruct* vt_entry_6C(CoordStruct* pDest, CoordStruct* pSrc) const override JMP_THIS(0x46C4F0);
	virtual bool SpawnAtMapCoords(CellStruct* pMapCoords, HouseClass* pOwner) override { return false; } //return false directly again
	virtual ObjectClass* CreateObject(HouseClass* owner) override { return nullptr; } //again

	bool Rotates() const
	{
		return !this->NoRotate;
	}

	void SetScaledSpawnDelay(int delay)
	{
		JMP_THIS(0x46C840);
		//this->ScaledSpawnDelay = delay;
	}

	BulletClass* __fastcall CreateBullet
	(
		AbstractClass* Target,
		TechnoClass* Owner,
		int Damage,
		WarheadTypeClass *WH,
		int Speed,
		bool Bright
	)
	{ JMP_STD(0x46B050); }

	//Constructor
	BulletTypeClass(const char* pID) noexcept
		: BulletTypeClass(noinit_t())
	{ JMP_THIS(0x46BBC0); }

protected:
	explicit __forceinline BulletTypeClass(noinit_t) noexcept
		: ObjectTypeClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	bool Airburst;
	bool Floater;
	bool SubjectToCliffs;
	bool SubjectToElevation;
	bool SubjectToWalls;
	bool VeryHigh;
	bool Shadow;
	bool Arcing;
	bool Dropping;
	bool Level;
	bool Inviso;
	bool Proximity;
	bool Ranged;
	bool NoRotate; // actually has opposite meaning of Rotates. false means Rotates=yes.
	bool Inaccurate;
	bool FlakScatter;
	bool AA;
	bool AG;
	bool Degenerates;
	bool Bouncy;
	bool AnimPalette;
	bool FirersPalette;
	PROTECTED_PROPERTY(BYTE, align_2AA[2]);
	int Cluster;
	WeaponTypeClass* AirburstWeapon;
	WeaponTypeClass* ShrapnelWeapon;
	int ShrapnelCount;
	int DetonationAltitude;
	bool Vertical;
	PROTECTED_PROPERTY(BYTE, align_2C1[7]);
	double Elasticity;
	int Acceleration;
	ColorScheme* Color;
	AnimTypeClass* Trailer;
	int ROT;
	int CourseLockDuration;
	int SpawnDelay;
	int ScaledSpawnDelay;
	bool Scalable;
	PROTECTED_PROPERTY(BYTE, align_2ED[3]);
	int Arm;
	byte AnimLow;
	byte AnimHigh;
	byte AnimRate;
	bool Flat;
};
