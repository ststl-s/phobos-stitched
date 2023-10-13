//Locomotor = {B7B49766-E576-11d3-9BD9-00104B972FE8}

#pragma once

#include <LocomotionClass.h>

class NOVTABLE RocketLocomotionClass : public LocomotionClass
{
public:

	//ILocomotion
	virtual bool __stdcall Is_Moving() override JMP_STD(0x661F50);
	virtual CoordStruct* __stdcall Destination(CoordStruct* pCoords) override JMP_STD(0x661FB0);
	virtual Matrix3D __stdcall Draw_Matrix(VoxelIndexKey* pIndex) override JMP_STD(0x663470);
	virtual bool __stdcall Process() override JMP_STD(0x6622C0);
	virtual void __stdcall Move_To(CoordStruct to) override JMP_STD(0x6632E0);
	virtual void __stdcall Stop_Moving() override JMP_STD(0x6633C0);
	virtual Layer __stdcall In_Which_Layer() override JMP_STD(0x663460);
	virtual bool __stdcall Is_Moving_Now() override JMP_STD(0x661F90);

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override JMP_STD(0x6633D0);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x663410);

	//Destructor
	virtual ~RocketLocomotionClass() override RX;// JMP_THIS(0x6635D0);

	//LocomotionClass
	virtual	int Size() override JMP_THIS(0x6635B0);

	//RocketLocomotionClass

	//Constructor
	RocketLocomotionClass()
		: RocketLocomotionClass(noinit_t())
	{ JMP_THIS(0x661EC0); }

protected:
	explicit __forceinline RocketLocomotionClass(noinit_t)
		: LocomotionClass(noinit_t())
	{ }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	CoordStruct MovingDestination;
	RateTimer MissionTimer;
	CDTimerClass TrailerTimer;
	int MissionState;
	DWORD unknown_44;
	double CurrentSpeed;
	bool unknown_bool_4C;
	bool SpawnerIsElite;
	float CurrentPitch;
	DWORD unknown_58;
	DWORD unknown_5C;
};

static_assert(sizeof(RocketLocomotionClass) == 0x60);
