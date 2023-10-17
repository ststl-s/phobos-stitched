//Locomotor = {2BEA74E1-7CCA-11d3-BE14-00104B62A16C}

#pragma once

#include <LocomotionClass.h>

class __declspec(align(4)) NOVTABLE ShipLocomotionClass : public LocomotionClass, public IPiggyback
{
public:

	//IUnknown
	virtual HRESULT __stdcall QueryInterface(REFIID iid, void** ppvObject) override JMP_STD(0x69EE30);
	virtual ULONG __stdcall AddRef() override JMP_STD(0x6A4260);
	virtual ULONG __stdcall Release() override JMP_STD(0x6A3E60);

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override JMP_STD(0x6A3E60);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x69EE90);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override JMP_STD(0x69EF10);

	//Destructor
	virtual ~ShipLocomotionClass() override RX;//JMP_THIS(0x6A42B0);

	//LocomotionClass
	int Size() override JMP_THIS(0x6A42A0);

	//ILocomotion
	virtual bool __stdcall Is_Moving() override JMP_STD(0x69F290)
	virtual CoordStruct* __stdcall Destination(CoordStruct* pBuffer) override JMP_STD(0x69F3A0);
	virtual CoordStruct* __stdcall Head_To_Coord(CoordStruct* pBuffer) override JMP_STD(0x69F3D0);
	virtual Matrix3D __stdcall Draw_Matrix(VoxelIndexKey* pIndex) override JMP_STD(0x69F670);
	virtual Matrix3D __stdcall Shadow_Matrix(VoxelIndexKey* pIndex) override JMP_STD(0x69FB20);
	virtual int __stdcall Z_Adjust() override JMP_STD(0x6A3EA0);
	virtual ZGradient __stdcall Z_Gradient()  override JMP_STD(0x6A3EB0);
	virtual bool __stdcall Process() override JMP_STD(0x69FC10);
	virtual void __stdcall Move_To(CoordStruct to) override JMP_STD(0x69F450);
	virtual void __stdcall Stop_Moving() override JMP_STD(0x69F510);
	virtual void __stdcall Do_Turn(DirStruct dir) override JMP_STD(0x6905C0);
	virtual void __stdcall Unlimbo() override JMP_STD(0x69FBE0);
	virtual void __stdcall Force_Track(int track, CoordStruct coord) override JMP_STD(0x6A0310);
	virtual Layer __stdcall In_Which_Layer() override JMP_STD(0x6A3E50);
	virtual void __stdcall Force_New_Slope(int ramp) override JMP_STD(0x69F250);
	virtual bool __stdcall Is_Moving_Now() override JMP_STD(0x69F330);
	virtual void __stdcall Mark_All_Occupation_Bits(int mark) override JMP_STD(0x6A3F00);
	virtual bool __stdcall Is_Moving_Here(CoordStruct to) override JMP_STD(0x6A3F50);
	virtual bool __stdcall Will_Jump_Tracks() override JMP_STD(0x6A4130);
	virtual void __stdcall Unlock() override JMP_STD(0x6A4210);
	virtual void __stdcall Lock() override JMP_STD(0x6A4220);
	virtual int __stdcall Get_Track_Number() override JMP_STD(0x6A4230);
	virtual int __stdcall Get_Track_Index() override JMP_STD(0x6A4240);
	virtual int __stdcall Get_Speed_Accum() override JMP_STD(0x6A4250);

	//IPiggyback
	virtual HRESULT __stdcall Begin_Piggyback(ILocomotion* pointer) override JMP_STD(0x69EFF0);
	virtual HRESULT __stdcall End_Piggyback(ILocomotion** pointer) override JMP_STD(0x69F040);
	virtual bool __stdcall Is_Ok_To_End() override JMP_STD(0x69F080);
	virtual HRESULT __stdcall Piggyback_CLSID(CLSID* pClassID) override JMP_STD(0x69ED20);
	virtual bool __stdcall Is_Piggybacking() override JMP_STD(0x6A4280);

	//Constructor
	ShipLocomotionClass()
		: ShipLocomotionClass(noinit_t())
	{ JMP_THIS(0x69EC50); }

protected:
	explicit __forceinline ShipLocomotionClass(noinit_t)
		: LocomotionClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	DWORD Ramp1;
	DWORD Ramp2;
	RateTimer SlopeTimer;
	CoordStruct Destination;
	CoordStruct HeadToCoord;
	int SpeedAccum;
	double movementspeed_50;
	DWORD TrackNumber;
	int TrackIndex;
	bool IsOnShortTrack;
	BYTE IsTurretLockedDown;
	bool IsRotating;
	bool IsDriving;
	bool IsRocking;
	bool IsLocked;
	ILocomotion* Piggybackee;
};

static_assert(sizeof(ShipLocomotionClass) == 0x70);

