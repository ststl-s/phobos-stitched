//Locomotor = {4A582743-9839-11d1-B709-00A024DDAFD1}

#pragma once

#include <LocomotionClass.h>

class __declspec(align(4)) NOVTABLE TunnelLocomotionClass : public LocomotionClass
{
public:

	enum State
	{
		Idle = 0,
		PreDigIn = 1,
		DiggingIn = 2,
		DugIn = 3,
		Digging = 4,
		PreDigOut = 5,
		DiggingOut = 6,
		DugOut = 7
	};

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override JMP_STD(0x72A110);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x72A150);

	//Destructor
	virtual ~TunnelLocomotionClass() override RX;//JMP_THIS(0x72A210);

	//LocomotionClass
	virtual int Size() override JMP_THIS(0x72A200);

	//ILocomotion
	virtual bool __stdcall Is_Moving() override JMP_STD(0x728A50);
	virtual CoordStruct* __stdcall Destination(CoordStruct* pBuffer) override JMP_STD(0x728A90);
	virtual Move __stdcall Can_Enter_Cell(CellStruct cell) override JMP_STD(0x72A090);
	virtual bool __stdcall Is_To_Have_Shadow() override JMP_STD(0x72A060);
	virtual Matrix3D __stdcall Draw_Matrix(VoxelIndexKey* pIndex) override JMP_STD(0x729B40);
	virtual VisualType __stdcall Visual_Character(VARIANT_BOOL unused) override JMP_STD(0x7291D0);
	virtual int __stdcall Z_Adjust() override JMP_STD(0x729E50);
	virtual ZGradient __stdcall Z_Gradient()  override JMP_STD(0x72A020);
	virtual bool __stdcall Process() override JMP_STD(0x728E30);
	virtual void __stdcall Move_To(CoordStruct to) override JMP_STD(0x728AF0);
	virtual void __stdcall Stop_Moving() override JMP_STD(0x728C00);
	virtual void __stdcall Do_Turn(DirStruct dir) override JMP_STD(0x72A0E0);
	virtual Layer __stdcall In_Which_Layer() override JMP_STD(0x72A1A0);
	virtual bool __stdcall Is_Moving_Now() override JMP_STD(0x728A60);
	virtual FireError __stdcall Can_Fire() override JMP_STD(0x72A1C0);
	virtual bool __stdcall Is_Surfacing() override JMP_STD(0x72A1E0);

	//non-virtual

	bool ProcessPreDigIn()
	{ JMP_THIS(0x7291F0); }

	bool ProcessDiggingIn()
	{ JMP_THIS(0x729370); }

	bool ProcessDugIn()
	{ JMP_THIS(0x7294E0); }

	bool ProcessDigging()
	{ JMP_THIS(0x729580); }

	bool ProcessPreDigOut()
	{ JMP_THIS(0x7298F0); }

	bool ProcessDiggingOut()
	{ JMP_THIS(0x729AA0); }

	bool ProcessDugOut()
	{ JMP_THIS(0x729480); }

	TunnelLocomotionClass()
		: TunnelLocomotionClass(noinit_t())
	{ JMP_THIS(0x728A00); }

protected:
	explicit __forceinline TunnelLocomotionClass(noinit_t)
		: LocomotionClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	TunnelLocomotionClass::State State;
	CoordStruct Coords;
	RateTimer DigTimer;
	bool bool38;
};

static_assert(sizeof(TunnelLocomotionClass) == 0x3C);

template<>
__forceinline TunnelLocomotionClass* locomotion_cast<TunnelLocomotionClass*>(ILocomotion* pThis)
{
	CLSID locoCLSID;
	return (SUCCEEDED(static_cast<LocomotionClass*>(pThis)->GetClassID(&locoCLSID)) && locoCLSID == LocomotionClass::CLSIDs::Tunnel) ?
		static_cast<TunnelLocomotionClass*>(pThis) : nullptr;
}
