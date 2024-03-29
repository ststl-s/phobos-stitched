#pragma once

#include <AbstractClass.h>
#include <Audio.h>

//forward declarations
class ObjectClass;
class TechnoClass;
class HouseClass;

class NOVTABLE BombClass : public AbstractClass
{
public:
	static const AbstractType AbsID = AbstractType::Bomb;

	//global array
	static constexpr constant_ptr<DynamicVectorClass<BombClass*>, 0x89C668u> const Array {};

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override JMP_STD(0x438B00);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x438B40);
	virtual HRESULT __stdcall Save(IStream* pStm,BOOL fClearDirty) override JMP_STD(0x438BD0);

	//Destructor
	virtual ~BombClass() override RX;// JMP_THIS(0x4393F0);

	//AbstractClass
	virtual AbstractType WhatAmI() const override JMP_THIS(0x4393E0);
	virtual int	Size() const override JMP_THIS(0x4393D0);
	virtual void ComputeCRC(CRCEngine& crc) const override JMP_THIS(0x438A90);

	void Detonate()
		{ JMP_THIS(0x438720); }

	void Disarm()
		{ JMP_THIS(0x4389B0); }

	BOOL IsDeathBomb() const
		{ JMP_THIS(0x4389F0); }

	int GetCurrentFlickerFrame() const // which frame of the ticking bomb to draw
		{ JMP_THIS(0x438A00); }

	bool TimeToExplode() const
		{ JMP_THIS(0x438A70); }

	//Constructor
	//Bombs have a special constructor that just should not be called like this...
	//See BombListClass::Plant
	BombClass() noexcept
		: AbstractClass(noinit_t())
	{ JMP_THIS(0x4385D0); }

protected:
	explicit __forceinline BombClass(noinit_t) noexcept
		: AbstractClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	TechnoClass* Owner;		//Most likely Ivan.
	HouseClass* OwnerHouse;
	ObjectClass* Target; // attaching to objects is possible, but it will never detonate
	BOOL DeathBomb; // unused - if so, [General]CanDetonateDeathBomb applies instead of CanDetonateTimeBomb
	int PlantingFrame;
	int DetonationFrame;
	AudioController Audio;
	int TickSound;
	BOOL ShouldPlayTickingSound; // seems so
	bool Harmless; // (mostly) set to 0 on plant, 1 on detonation/removal ?
};
