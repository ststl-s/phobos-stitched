/*
	Weapons
*/

#pragma once

#include <AbstractTypeClass.h>
#include <BulletTypeClass.h>
#include <WarheadTypeClass.h>

//forward declarations
class AnimTypeClass;
class BulletTypeClass;
class ParticleSystemTypeClass;
class WarheadTypeClass;

class NOVTABLE WeaponTypeClass : public AbstractTypeClass
{
public:
	static const AbstractType AbsID = AbstractType::WeaponType;

	//Array
	ABSTRACTTYPE_ARRAY(WeaponTypeClass, 0x887568u);

	//static
	//static WeaponTypeClass* __fastcall FindOrAllocate(const char* pID) JMP_STD(0x772FA0);
	//static int __fastcall FindIndex(WeaponTypeClass* pWeapon) JMP_STD(0x773030);

	//it may not be here because it doesn't make sense
	//just used in INIClass::INI_READ(ArmorType, 0x4753F0)
	static Armor __fastcall FindArmor(const char* pName) JMP_STD(0x772A50);

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override JMP_STD(0x772C90);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x772CD0);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override JMP_STD(0x772EB0);

	//Destructor
	virtual ~WeaponTypeClass() override RX;// JMP_THIS(0x7730F0);

	//AbstractClass
	virtual AbstractType WhatAmI() const override { return AbstractType::WeaponType; }
	virtual int Size() const override { return 0x160; }
	virtual void ComputeCRC(CRCEngine& crc) const override JMP_THIS(0x772AE0);

	//AbstractTypeClass
	virtual bool LoadFromINI(CCINIClass* pINI) override JMP_THIS(0x772080);

	void CalculateSpeed()
	{ JMP_THIS(0x7729F0); }

	TargetFlags WeaponCanTargetTypes()
	{
		JMP_THIS(0x772A90);
		/*BulletTypeClass* pBulletType = this->Projectile;
		TargetFlags res = TargetFlags::None;
		if (pBulletType->AA)
			res |= TargetFlags::Air;
		if (pBulletType->AG)
			res |= TargetFlags::GoundTechnos;
		return res;*/
	}

	bool CanTargetWall()
	{
		JMP_THIS(0x772AC0)
		/*WarheadTypeClass* pWH = this->Warhead;
		return pWH && pWH->Wall;*/
	}

	DWORD sub_773070(DWORD dwUnk)
	{ JMP_THIS(0x773070); }

	//Constructor
	WeaponTypeClass(const char* pID = nullptr)
		: WeaponTypeClass(noinit_t())
	{ JMP_THIS(0x771C70); }

protected:
	explicit __forceinline WeaponTypeClass(noinit_t)
		: AbstractTypeClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	int AmbientDamage;
	int Burst;
	BulletTypeClass* Projectile;
	int Damage;
	int Speed;
	WarheadTypeClass* Warhead;
	int ROF;
	int Range; // int(256 * ini value)
	int MinimumRange; // int(256 * ini value)
	TypeList<int> Report;		//sound indices
	TypeList<int> DownReport;	//sound indices
	TypeList<AnimTypeClass*> Anim;
	AnimTypeClass* OccupantAnim;
	AnimTypeClass* AssaultAnim;
	AnimTypeClass* OpenToppedAnim;
	ParticleSystemTypeClass* AttachedParticleSystem;
	ColorStruct LaserInnerColor;
	ColorStruct LaserOuterColor;
	ColorStruct LaserOuterSpread;
	bool UseFireParticles;
	bool UseSparkParticles;
	bool OmniFire;
	bool DistributedWeaponFire;
	bool IsRailgun;
	bool Lobber;
	bool Bright;
	bool IsSonic;
	bool Spawner;
	bool LimboLaunch;
	bool DecloakToFire;
	bool CellRangefinding;
	bool FireOnce;
	bool NeverUse;
	bool RevealOnFire;
	bool TerrainFire;
	bool SabotageCursor;
	bool MigAttackCursor;
	bool DisguiseFireOnly;
	int DisguiseFakeBlinkTime;
	bool InfiniteMindControl;
	bool FireWhileMoving;
	bool DrainWeapon;
	bool FireInTransport;
	bool Suicide;
	bool TurboBoost;
	bool Supress;
	bool Camera;
	bool Charges;
	bool IsLaser;
	bool DiskLaser;
	bool IsLine;
	bool IsBigLaser;
	bool IsHouseColor;
	char LaserDuration;
	bool IonSensitive;
	bool AreaFire;
	bool IsElectricBolt;
	bool DrawBoltAsLaser;
	bool IsAlternateColor;
	bool IsRadBeam;
	bool IsRadEruption;
	PROTECTED_PROPERTY(BYTE, align_156[2]);
	int RadLevel;
	bool IsMagBeam;
	PROTECTED_PROPERTY(BYTE, align_15D[3]);
};
