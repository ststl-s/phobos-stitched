//Locomotor = {4A582746-9839-11d1-B709-00A024DDAFD1}

#pragma once

#include <LocomotionClass.h>

class NOVTABLE FlyLocomotionClass : public LocomotionClass
{
public:

	//ILocomotion
	virtual HRESULT __stdcall Link_To_Object(void* pointer) override JMP_STD(0x4CCA20);
	virtual bool __stdcall Is_Moving() override JMP_STD(0x4CCA90);
	virtual CoordStruct* __stdcall Destination(CoordStruct* pcoord) override JMP_STD(0x4CCAE0);
	virtual Matrix3D __stdcall Draw_Matrix(union VoxelIndexKey* pIndex) override JMP_STD(0x4CF610);
	virtual Matrix3D __stdcall Shadow_Matrix(union VoxelIndexKey* pIndex) override JMP_STD(0x4CFB00);
	virtual Point2D* __stdcall Draw_Point(Point2D* pPoint) override JMP_STD(0x4CF830);
	virtual Point2D* __stdcall Shadow_Point(Point2D* pPoint) override JMP_STD(0x4CF940);
	virtual bool __stdcall Process() override JMP_STD(0x4CCB40);
	virtual void __stdcall Move_To(CoordStruct to) override JMP_STD(0x4CCC80);
	virtual void __stdcall Stop_Moving() override JMP_STD(0x4CCFD0);
	virtual void __stdcall Do_Turn(DirStruct coord) override JMP_STD(0x4CFC10);
	virtual bool __stdcall Power_Off() override JMP_STD(0x4CFD20);
	virtual bool __stdcall Is_Powered() override JMP_STD(0x4CFD90);
	virtual bool __stdcall Is_Ion_Sensitive() override JMP_STD(0x4CFDA0);
	virtual Layer __stdcall In_Which_Layer() override JMP_STD(0x4CFCF0);
	virtual bool __stdcall Is_Moving_Now() override JMP_STD(0x4CCAC0);
	virtual int __stdcall Apparent_Speed() override JMP_STD(0x4CFE20);
	virtual int __stdcall Get_Status() override JMP_STD(0x4CFE50);
	virtual void __stdcall Acquire_Hunter_Seeker_Target() override JMP_STD(0x4CFE80);

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override JMP_STD(0x4CFC80);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x4CFCC0);

	//Destructor
	virtual ~FlyLocomotionClass() override RX;// JMP_THIS(0x4D03A0);

	//LocomotionClass
	virtual	int Size() override JMP_THIS(0x4D0390);

	//FlyLocomotionClass

	//Constructor
	FlyLocomotionClass()
		: LocomotionClass(noinit_t())
	{ JMP_THIS(0x4CC9A0); }

protected:
	explicit __forceinline FlyLocomotionClass(noinit_t)
		: LocomotionClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	bool AirportBound;
	CoordStruct MovingDestination;
	CoordStruct XYZ2;
	bool HasMoveOrder;
	int FlightLevel;
	double TargetSpeed;
	double CurrentSpeed;
	char IsTakingOff;
	bool IsLanding;
	bool WasLanding;
	bool unknown_bool_53;
	DWORD unknown_54;
	DWORD unknown_58;
	bool IsElevating;
	bool unknown_bool_5D;
	bool unknown_bool_5E;
	bool unknown_bool_5F;
};
