/*
	DiskLasers are the floating disks' purple lasers.
*/

#pragma once

#include <AbstractClass.h>

class LaserDrawClass;
class TechnoClass;
class WeaponTypeClass;

class NOVTABLE DiskLaserClass : public AbstractClass
{
public:
	static const AbstractType AbsID = AbstractType::DiskLaser;

	// static
	static constexpr constant_ptr<DynamicVectorClass<DiskLaserClass*>, 0x8A0208u> const Array{};

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override JMP_STD(0x4A7C30);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x4A7B90);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override JMP_STD(0x4A7C10);

	//Destructor
	virtual ~DiskLaserClass() override RX;// JMP_THIS(0x4A7C90);

	//AbstractClass
	virtual AbstractType WhatAmI() const override JMP_THIS(0x4A7C80);
	virtual int Size() const override JMP_THIS(0x4A7C70);
	virtual void ComputeCRC(CRCEngine& crc) const override JMP_THIS(0x4A7B80);
	virtual void Update() override JMP_THIS(0x4A7340);

	//non-virtual
	void Fire(TechnoClass* pOwner, AbstractClass* pTarget, WeaponTypeClass* pWeapon, int damage)
	{ JMP_THIS(0x4A71A0); }

	void PointerGotInvalid(AbstractClass* pInvalid)
	{ JMP_THIS(0x4A7900); }

	//Constructor
	DiskLaserClass() noexcept
		: DiskLaserClass(noinit_t())
	{ JMP_THIS(0x4A7A30); }

protected:
	explicit __forceinline DiskLaserClass(noinit_t) noexcept
		: AbstractClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	TechnoClass* Owner;
	TechnoClass* Target;
	WeaponTypeClass* Weapon;
	DWORD unknown_30;
	DWORD unknown_34;
	DWORD unknown_38;
	int Damage;
};
