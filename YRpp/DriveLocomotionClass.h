//Locomotor = {4A582741-9839-11d1-B709-00A024DDAFD1}

#pragma once

#include <LocomotionClass.h>

class NOVTABLE DriveLocomotionClass : public LocomotionClass, public IPiggyback
{
public:

	//IUnknown
	virtual HRESULT __stdcall QueryInterface(REFIID iid, void** ppvObject) JMP_STD(0x4AF720);
	virtual ULONG __stdcall AddRef() JMP_STD(0x4B4CB0);
	virtual ULONG __stdcall Release() JMP_STD(0x4B4CC0);

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) JMP_STD(0x4B4830);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) JMP_STD(0x4AF780);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) JMP_STD(0x4AF800);

	//Destructor
	virtual ~DriveLocomotionClass() override JMP_THIS(0x4B4D00);

	//LocomotionClass
	virtual int Size() override { return 0x70; }

	//ILocomotion
	virtual bool __stdcall Is_Moving() override JMP_STD(0x4AFB80);
	virtual CoordStruct* __stdcall Destination(CoordStruct* pcoord) override JMP_STD(0x4AFC90);
	virtual CoordStruct* __stdcall Head_To_Coord(CoordStruct* pcoord) override JMP_STD(0x4AFCC0);
	virtual Matrix3D* __stdcall Draw_Matrix(Matrix3D* pMatrix, int* facing) override JMP_STD(0x4AFF60);
	virtual Matrix3D* __stdcall Shadow_Matrix(Matrix3D* pMatrix, int* facing) override JMP_STD(0x4B0410);
	virtual int __stdcall Z_Adjust() override JMP_STD(0x4B4870);// { return 0; }
	virtual ZGradient __stdcall Z_Gradient()  override JMP_STD(0x4B4880);// { return this->LocomotionClass::Z_Gradient(); }
	virtual bool __stdcall Process() override JMP_STD(0x4B0500);
	virtual void __stdcall Move_To(CoordStruct to) override JMP_STD(0x4AFD40);
	virtual void __stdcall Stop_Moving() override JMP_STD(0x4AFE00);
	virtual void __stdcall Do_Turn(DirStruct coord) override JMP_STD(0x4B0EF0);
	virtual void __stdcall Unlimbo() override JMP_STD(0x4B04D0);
	virtual void __stdcall Force_Track(int track, CoordStruct coord) override JMP_STD(0x4B0C40);
	virtual Layer __stdcall In_Which_Layer() override JMP_STD(0x4B4820);// { return Layer::Ground; }
	virtual void __stdcall Force_New_Slope(int ramp) override JMP_STD(0x4AFB40);
	virtual bool __stdcall Is_Moving_Now() override JMP_STD(0x4AFC20);
	virtual void __stdcall Mark_All_Occupation_Bits(int mark) override JMP_STD(0x4B48D0);
	virtual bool __stdcall Is_Moving_Here(CoordStruct to) override JMP_STD(0x4B4920);
	virtual bool __stdcall Will_Jump_Tracks() override JMP_STD(0x4B4B00);
	virtual void __stdcall Lock() override JMP_STD(0x4B4BE0);
	virtual void __stdcall Unlock() override JMP_STD(0x4B4BF0);
	virtual int __stdcall Get_Track_Number() override JMP_STD(0x4B4C00);
	virtual int __stdcall Get_Track_Index() override JMP_STD(0x4B4C10);
	virtual int __stdcall Get_Speed_Accum() override { return -1; }

	//IPiggyback
	virtual HRESULT __stdcall Begin_Piggyback(ILocomotion* pointer) override JMP_STD(0x4AF8E0);	//Piggybacks a locomotor onto this one.
	virtual HRESULT __stdcall End_Piggyback(ILocomotion** pointer) override JMP_STD(0x4AF930);	//End piggyback process and restore locomotor interface pointer.
	virtual bool __stdcall Is_Ok_To_End() override JMP_STD(0x4AF970);	//Is it ok to end the piggyback process?
	virtual HRESULT __stdcall Piggyback_CLSID(GUID* classid) override JMP_STD(0x4AF610);	//Fetches piggybacked locomotor class ID.
	virtual bool __stdcall Is_Piggybacking() override JMP_STD(0x4B4CD0);	//Is it currently piggy backing another locomotor?

	//Constructor
	DriveLocomotionClass()
		: DriveLocomotionClass(noinit_t())
	{ JMP_THIS(0x4AF540); }

protected:
	explicit __forceinline DriveLocomotionClass(noinit_t)
		: LocomotionClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	DWORD Ramp1;
	DWORD Ramp2;
	RateTimer SlopeTimer;
	CoordStruct _Destination;
	CoordStruct _HeadToCoord;
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
	AbstractClass* Raider;
	int field_6C;
};

static_assert(sizeof(DriveLocomotionClass) == 0x70);
