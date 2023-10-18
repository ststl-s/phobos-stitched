//Locomotor = {4A582747-9839-11d1-B709-00A024DDAFD1}

#pragma once

#include <LocomotionClass.h>

class NOVTABLE TeleportLocomotionClass : public LocomotionClass, public IPiggyback
{
public:
	//IUnknown
	virtual HRESULT __stdcall QueryInterface(REFIID iid, void** ppvObject) override JMP_STD(0x719E30);
	virtual ULONG __stdcall AddRef() override JMP_STD(0x71A0E0);
	virtual ULONG __stdcall Release() override JMP_STD(0x71A0F0);

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override JMP_STD(0x719C60);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x719CA0);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override JMP_STD(0x719D40);

	//Destructor
	virtual ~TeleportLocomotionClass() override RX;//JMP_THIS(0x71A130);

	//LocomotionClass
	virtual	int Size() override JMP_THIS(0x71A120);

	//TeleportLocomotionClass
	virtual void vt_entry_28(TeleportLocomotionClass* unknown) JMP_THIS(0x719BF0);
	virtual bool IsStill() JMP_THIS(0x718090);

	//ILocomotion
	virtual bool __stdcall Is_Moving() override JMP_STD(0x718080);
	virtual CoordStruct* __stdcall Destination(CoordStruct* pcoord) override JMP_STD(0x7180A0);
	virtual bool __stdcall Process() override JMP_STD(0x7192F0);
	virtual void __stdcall Move_To(CoordStruct to) override JMP_STD(0x718100);
	virtual void __stdcall Stop_Moving() override JMP_STD(0x718230);
	virtual void __stdcall Do_Turn(DirStruct coord) override JMP_STD(0x7192C0);
	virtual Layer __stdcall In_Which_Layer() override JMP_STD(0x719E20);
	virtual void __stdcall Mark_All_Occupation_Bits(int mark) override JMP_STD(0x71A090);

	//IPiggyback
	virtual HRESULT __stdcall Begin_Piggyback(ILocomotion* pointer) override JMP_STD(0x719E90);
	virtual HRESULT __stdcall End_Piggyback(ILocomotion** pointer) override JMP_STD(0x719EE0);
	virtual bool __stdcall Is_Ok_To_End() override JMP_STD(0x719F30);
	virtual HRESULT __stdcall Piggyback_CLSID(CLSID* pClassID) override JMP_STD(0x719F80);
	virtual bool __stdcall Is_Piggybacking() override JMP_STD(0x71A100);

	//Constructor
	TeleportLocomotionClass()
		: TeleportLocomotionClass(noinit_t())
	{ JMP_THIS(0x718000); }

protected:
	explicit __forceinline TeleportLocomotionClass(noinit_t)
		: LocomotionClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	CoordStruct MovingDestination;	//Current destination
	CoordStruct LastCoords; //Marked occupation bits there
	bool Moving;	//Is currently moving
	bool unknown_bool_35;
	bool unknown_bool_36;
	int State;
	CDTimerClass Timer;
	ILocomotion* Piggybackee;
};

template<>
__forceinline TeleportLocomotionClass* locomotion_cast<TeleportLocomotionClass*>(ILocomotion* pThis)
{
	CLSID locoCLSID;
	return (SUCCEEDED(static_cast<LocomotionClass*>(pThis)->GetClassID(&locoCLSID)) && locoCLSID == LocomotionClass::CLSIDs::Teleport) ?
		static_cast<TeleportLocomotionClass*>(pThis) : nullptr;
}
