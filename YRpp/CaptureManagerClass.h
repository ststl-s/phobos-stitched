/*
	CaptureManager - used for mind control.
*/

#pragma once

#include <AbstractClass.h>

class HouseClass;
class TechnoClass;

struct ControlNode
{
	explicit ControlNode() noexcept { }

	TechnoClass* Techno;
	HouseClass* OriginalOwner;
	DECLARE_PROPERTY(CDTimerClass, LinkDrawTimer);
};

class NOVTABLE CaptureManagerClass : public AbstractClass
{
public:
	static const AbstractType AbsID = AbstractType::CaptureManager;

	//Static
	static constexpr constant_ptr<DynamicVectorClass<CaptureManagerClass*>, 0x89E0F0u> const Array{};

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override JMP_STD(0x472960);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x472720);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override JMP_STD(0x4728E0);

	//Destructor
	virtual ~CaptureManagerClass() override JMP_THIS(0x4729C0);

	//AbstractClass
	virtual AbstractType WhatAmI() const override { return AbstractType::CaptureManager; }
	virtual int Size() const override { return 0x50; }
	virtual void ComputeCRC(CRCEngine& crc) const override JMP_THIS(0x4726F0);

	//non-virtual
	bool Capture(TechnoClass* pTarget)
	{ JMP_THIS(0x471D40); }

	bool Free(TechnoClass* pTarget)
	{ JMP_THIS(0x471FF0); }

	void FreeAll()
	{ JMP_THIS(0x472140); }

	int NumControlNodes() const
	{ return ControlNodes.Count; }

	bool CanCapture(TechnoClass *pTarget) const
	{ JMP_THIS(0x471C90); }

	bool CannotControlAnyMore() const
	{ JMP_THIS(0x4722A0); }

	bool IsControllingSomething() const
	{ JMP_THIS(0x4722C0); }

	bool IsOverloading(bool& wasDamageApplied) const
	{ JMP_THIS(0x4726C0); }

	void HandleOverload()
	{ JMP_THIS(0x471A50); }

	bool NeedsToDrawLinks() const
	{ JMP_THIS(0x472640); }

	bool DrawLinks()
	{ JMP_THIS(0x472160); }

	void DecideFate(TechnoClass* pTechno)
	{ JMP_THIS(0x4723B0); }

	HouseClass* GetOriginalOwner(TechnoClass *pTechno) const
	{ JMP_THIS(0x4722F0); }

	//Constructor
	CaptureManagerClass(TechnoClass* pOwner, int nMaxControlNodes, bool bInfiniteControl) noexcept
		: CaptureManagerClass(noinit_t())
	{ JMP_THIS(0x4717D0); }

protected:
	explicit __forceinline CaptureManagerClass(noinit_t) noexcept
		: AbstractClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	DynamicVectorClass<ControlNode*> ControlNodes;
	int MaxControlNodes;
	bool InfiniteMindControl;
	bool OverloadDeathSoundPlayed; // Has the mind control death sound played already?
	int OverloadPipState; // Used to create the red overloading pip by returning true in IsOverloading's wasDamageApplied for 10 frames.
	TechnoClass* Owner;
	int OverloadDamageDelay; // Decremented every frame. If it reaches zero, OverloadDamage is applied.
};
