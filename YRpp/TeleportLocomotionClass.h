//Locomotor = {4A582747-9839-11d1-B709-00A024DDAFD1}

#pragma once

#include <LocomotionClass.h>

class NOVTABLE TeleportLocomotionClass : public LocomotionClass, public IPiggyback
{
public:
	//IUnknown
	virtual HRESULT __stdcall QueryInterface(REFIID iid, void** ppvObject) override R0;
	virtual ULONG __stdcall AddRef() override R0;
	virtual ULONG __stdcall Release() override R0;

	//IPiggyback
	virtual HRESULT __stdcall Begin_Piggyback(ILocomotion* pointer) override R0;
	virtual HRESULT __stdcall End_Piggyback(ILocomotion** pointer) override R0;
	virtual bool __stdcall Is_Ok_To_End() override R0;
	virtual HRESULT __stdcall Piggyback_CLSID(GUID* classid) override R0;
	virtual bool __stdcall Is_Piggybacking() override R0;

	//ILocomotion
	virtual bool __stdcall Is_Moving() override R0;
	virtual CoordStruct* __stdcall Destination(CoordStruct* pcoord) override R0;
	virtual bool __stdcall Process() override R0;
	virtual void __stdcall Move_To(CoordStruct to) override RX;
	virtual void __stdcall Stop_Moving() override RX;
	virtual void __stdcall Do_Turn(DirStruct coord) override RX;
	virtual Layer __stdcall In_Which_Layer() override RT(Layer);
	virtual void __stdcall Mark_All_Occupation_Bits(int mark) override RX;
	virtual void __stdcall Clear_Coords() override RX;

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override R0;

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override R0;
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override R0;

	//Destructor
	virtual ~TeleportLocomotionClass() override RX;

	//LocomotionClass
	virtual	int Size() override R0;

	//TeleportLocomotionClass
	virtual void vt_entry_28(DWORD dwUnk) RX;
	virtual bool IsStill() R0;

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
