//Locomotor = {92612C46-F71F-11d1-AC9F-006008055BB5}

#pragma once

#include <LocomotionClass.h>

//https://github.com/chaserli
//https://github.com/Phobos-developers/Phobos/pull/744
enum class JumpJetState
{
	Ground = 0,
	TakeOff = 1,
	Hovering = 2,
	Moving = 3,
	Deploying = 4,
	Crashing = 5
	/*
	1 - Taking off from the ground
	2 - Hovering in the air
	3 - Moving in air
	4 - Deploying to land
	5 - Crashing or attacked by locomotor warhead
	*/
};

class NOVTABLE JumpjetLocomotionClass : public LocomotionClass, public IPiggyback
{
public:
	//IUnknown
	virtual HRESULT __stdcall QueryInterface(REFIID iid, void** ppvObject) JMP_STD(0x54DC60);
	virtual ULONG __stdcall AddRef() JMP_STD(0x54DF50);
	virtual ULONG __stdcall Release() JMP_STD(0x54DF60);

	//IPiggyback
	virtual HRESULT __stdcall Begin_Piggyback(ILocomotion* pointer) override JMP_STD(0x54DA00);
	virtual HRESULT __stdcall End_Piggyback(ILocomotion** pointer) override JMP_STD(0x54DA50);
	virtual bool __stdcall Is_Ok_To_End() override JMP_STD(0x54DB00);
	virtual HRESULT __stdcall Piggyback_CLSID(GUID* classid) override JMP_STD(0x54DB50);
	virtual bool __stdcall Is_Piggybacking() override JMP_STD(0x54DF70);

	//ILocomotion
	virtual HRESULT __stdcall Link_To_Object(void* pointer) override JMP_STD(0x54AD30);
	virtual bool __stdcall Is_Moving() override JMP_STD(0x54AE50);
	virtual CoordStruct* __stdcall Destination(CoordStruct* pcoord) override JMP_STD(0x54AE60);
	virtual CoordStruct* __stdcall Head_To_Coord(CoordStruct* pcoord) override JMP_STD(0x54D9B0);
	virtual Matrix3D __stdcall Draw_Matrix(union VoxelIndexKey* pIndex) override JMP_STD(0x54DCC0);
	virtual bool __stdcall Process() override JMP_STD(0x54AEC0);
	virtual void __stdcall Move_To(CoordStruct to) override JMP_STD(0x54B1C0);
	virtual void __stdcall Stop_Moving() override JMP_STD(0x54B4D0);
	virtual void __stdcall Do_Turn(DirStruct dir) override JMP_STD(0x54B6E0);
	virtual Layer __stdcall In_Which_Layer() override JMP_STD(0x54B8D0);
	virtual bool __stdcall Is_Moving_Now() override JMP_STD(0x54D0D0);
	virtual void __stdcall Mark_All_Occupation_Bits(int mark) override JMP_STD(0x54D930);

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) JMP_STD(0x54B710);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) JMP_STD(0x54B750);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) JMP_STD(0x54B7E0);

	//Destructor
	virtual ~JumpjetLocomotionClass() JMP_THIS(0x54DFA0);

	//LocomotionClass
	virtual	int Size() JMP_THIS(0x54DF90);

	//JumpjetLocomotionClass

	//Constructor
	JumpjetLocomotionClass()
		: LocomotionClass(noinit_t())
	{
		JMP_THIS(0x54AC40);
	}

protected:
	explicit __forceinline JumpjetLocomotionClass(noinit_t)
		: LocomotionClass(noinit_t())
	{
	}

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	int TurnRate;
	int Speed;
	float Climb;
	float Crash;
	int Height;
	float Accel;
	float Wobbles;
	int Deviation;
	bool NoWobbles;
	BYTE unknown_3D;
	BYTE unknown_3E;
	BYTE unknown_3F;
	CoordStruct DestinationCoords;
	bool IsMoving;
	BYTE unknown_4D;
	BYTE unknown_4E;
	BYTE unknown_4F;
	JumpJetState State;
	FacingClass LocomotionFacing;
	BYTE unknown_6C;
	BYTE unknown_6D;
	BYTE unknown_6E;
	BYTE unknown_6F;
	double CurrentSpeed;
	double MaxSpeed;
	int CurrentHeight;
	BYTE unknown_84;
	BYTE unknown_85;
	BYTE unknown_86;
	BYTE unknown_87;
	double CurrentWobbles;
	bool DestinationReached;
	BYTE unknown_91;
	BYTE unknown_92;
	BYTE unknown_93;
	ILocomotion* Raider;
};

static_assert(sizeof(JumpjetLocomotionClass) == 0x98);
