#pragma once

#include <Interfaces.h>
#include <FootClass.h>
#include <Unsorted.h>
#include <YRCom.h>
#include <Helpers/ComPtr.h>
#include <Helpers/CompileTime.h>

class LocomotionClass : public IPersistStream, public ILocomotion
{
public:
	class CLSIDs
	{
	public:
		static constexpr reference<CLSID const, 0x7E9A30u> const Drive {};
		static constexpr reference<CLSID const, 0x7E9AC0u> const Jumpjet {};
		static constexpr reference<CLSID const, 0x7E9A40u> const Hover {};
		static constexpr reference<CLSID const, 0x7E9AD0u> const Rocket {};
		static constexpr reference<CLSID const, 0x7E9A50u> const Tunnel {};
		static constexpr reference<CLSID const, 0x7E9A60u> const Walk {};
		static constexpr reference<CLSID const, 0x7E9A70u> const Droppod {};
		static constexpr reference<CLSID const, 0x7E9A80u> const Fly {};
		static constexpr reference<CLSID const, 0x7E9A90u> const Teleport {};
		static constexpr reference<CLSID const, 0x7E9AA0u> const Mech {};
		static constexpr reference<CLSID const, 0x7E9AB0u> const Ship {};
	};
	//IUnknown
	virtual HRESULT __stdcall QueryInterface(REFIID iid, void** ppvObject) JMP_STD(0x55A9B0);
	virtual ULONG __stdcall AddRef() JMP_STD(0x55A95F);
	virtual ULONG __stdcall Release() JMP_STD(0x55A970);

	//IPersist
	//virtual HRESULT __stdcall GetClassID(CLSID* pClassID) R0;

	//IPersistStream
	virtual HRESULT __stdcall IsDirty() JMP_STD(0x4B4C30);
	//virtual HRESULT __stdcall Load(IStream* pStm) R0;
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) JMP_STD(0x55AA60);

	virtual HRESULT __stdcall GetSizeMax(ULARGE_INTEGER* pcbSize) JMP_STD(0x55AB40);

	virtual ~LocomotionClass() JMP_THIS(0x5172F0); // should be SDDTOR in fact
	virtual int Size() = 0;

	// ILocomotion
	// virtual HRESULT __stdcall QueryInterface(REFIID iid, void** ppvObject) R0;
	// virtual ULONG __stdcall AddRef() R0;
	// virtual ULONG __stdcall Release() R0;

	// ILocomotion
	virtual HRESULT __stdcall Link_To_Object(void* pointer) override JMP_STD(0x55A710);
	virtual bool __stdcall Is_Moving() override { return false; }
	virtual CoordStruct* __stdcall Destination(CoordStruct* pcoord) override JMP_STD(0x55AC70);
	virtual CoordStruct* __stdcall Head_To_Coord(CoordStruct* pcoord) override JMP_STD(0x55ACA0);
	virtual Move __stdcall Can_Enter_Cell(CellStruct cell) override { return Move::OK; }
	virtual bool __stdcall Is_To_Have_Shadow() override { return true; }
	virtual Matrix3D __stdcall Draw_Matrix(VoxelIndexKey* pIndex) override JMP_STD(0x55A730);
	virtual Matrix3D __stdcall Shadow_Matrix(VoxelIndexKey* pIndex) override JMP_STD(0x55A7D0);
	virtual Point2D* __stdcall Draw_Point(Point2D* pPoint) override { *pPoint = { 0,0 }; return pPoint; }
	virtual Point2D* __stdcall Shadow_Point(Point2D* pPoint) override JMP_STD(0x55A8C0);
	virtual VisualType __stdcall Visual_Character(VARIANT_BOOL unused) override { return VisualType::Normal; }
	virtual int __stdcall Z_Adjust() override { return 0; }
	virtual ZGradient __stdcall Z_Gradient()  override { return ZGradient::Deg90; }
	virtual bool __stdcall Process() override { return true; }
	virtual void __stdcall Move_To(CoordStruct to) override { }
	virtual void __stdcall Stop_Moving() override { }
	virtual void __stdcall Do_Turn(DirStruct coord) override { }
	virtual void __stdcall Unlimbo() override { }
	virtual void __stdcall Tilt_Pitch_AI() override { }
	virtual bool __stdcall Power_On() override JMP_STD(0x55A8F0);
	virtual bool __stdcall Power_Off() override JMP_STD(0x55A910);
	virtual bool __stdcall Is_Powered() override JMP_STD(0x55A930);
	virtual bool __stdcall Is_Ion_Sensitive() override { return false; }
	virtual bool __stdcall Push(DirStruct dir) override { return false; }
	virtual bool __stdcall Shove(DirStruct dir) override { return false; }
	virtual void __stdcall Force_Track(int track, CoordStruct coord) override { }
	//virtual Layer __stdcall In_Which_Layer() = 0;
	virtual void __stdcall Force_Immediate_Destination(CoordStruct coord) override { }
	virtual void __stdcall Force_New_Slope(int ramp) override { }
	virtual bool __stdcall Is_Moving_Now() override JMP_STD(0x4B6610);
	virtual int __stdcall Apparent_Speed() override JMP_STD(0x55AD10);
	virtual int __stdcall Drawing_Code() override { return 0; }
	virtual FireError __stdcall Can_Fire() override { return FireError::OK; }
	virtual int __stdcall Get_Status() override { return 0; }
	virtual void __stdcall Acquire_Hunter_Seeker_Target() override { }
	virtual bool __stdcall Is_Surfacing() override { return false; }
	virtual void __stdcall Mark_All_Occupation_Bits(int mark) override { }
	virtual bool __stdcall Is_Moving_Here(CoordStruct to) override { return false; }
	virtual bool __stdcall Will_Jump_Tracks() override { return false; }
	virtual bool __stdcall Is_Really_Moving_Now() override JMP_STD(0x4B4C50);
	virtual void __stdcall Stop_Movement_Animation() override { }
	virtual void __stdcall Clear_Coords() override { }
	virtual void __stdcall Lock() override { }
	virtual void __stdcall Unlock() override { }
	virtual int __stdcall Get_Track_Number() override { return -1; }
	virtual int __stdcall Get_Track_Index() override { return -1; }
	virtual int __stdcall Get_Speed_Accum() override { return -1; }

	// Non virtuals
	static HRESULT TryPiggyback(IPiggyback** Piggy, ILocomotion** Loco)
	{ PUSH_VAR32(Loco); SET_REG32(ECX, Piggy); CALL(0x45AF20); }

	static HRESULT CreateInstance(ILocomotion** ppv, const CLSID* rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext)
	{ PUSH_VAR32(dwClsContext); PUSH_VAR32(pUnkOuter); PUSH_VAR32(rclsid); SET_REG32(ECX, ppv); CALL(0x41C250); }

	// these two are identical, why do they both exist...
	static void AddRef1(LocomotionClass** Loco)
	{ SET_REG32(ECX, Loco); CALL(0x45A170); }

	static void AddRef2(LocomotionClass** Loco)
	{ SET_REG32(ECX, Loco); CALL(0x6CE270); }

	static void ChangeLocomotorTo(FootClass* Object, const CLSID& clsid)
	{
		// remember the current one
		YRComPtr<ILocomotion> Original(Object->Locomotor);

		// create a new locomotor and link it
		auto NewLoco = CreateInstance(clsid);
		NewLoco->Link_To_Object(Object);

		// get piggy interface and piggy original
		YRComPtr<IPiggyback> Piggy(NewLoco);
		Piggy->Begin_Piggyback(Original.get());

		// replace the current locomotor
		Object->Locomotor = std::move(NewLoco);
	}

	// creates a new instance by class ID. returns a pointer to ILocomotion
	static YRComPtr<ILocomotion> CreateInstance(const CLSID& rclsid)
	{
		return YRComPtr<ILocomotion>(rclsid, nullptr,
			CLSCTX_INPROC_SERVER | CLSCTX_INPROC_HANDLER | CLSCTX_LOCAL_SERVER);
	}

	// finds out whether a locomotor is currently piggybacking and restores
	// the original locomotor. this function ignores Is_Ok_To_End().
	static bool End_Piggyback(YRComPtr<ILocomotion>& pLoco)
	{
		if (!pLoco)
		{
			Game::RaiseError(E_POINTER);
		}

		if (YRComPtr<IPiggyback> pPiggy = pLoco)
		{
			if (pPiggy->Is_Piggybacking())
			{
				// this frees the current locomotor
				pLoco.reset(nullptr);

				// this restores the old one
				auto res = pPiggy->End_Piggyback(pLoco.pointer_to());
				if (FAILED(res))
				{
					Game::RaiseError(res);
				}
				return (res == S_OK);
			}
		}

		return false;
	}

	//Constructors
	explicit LocomotionClass() { JMP_THIS(0x55A6C0); }
	LocomotionClass(LocomotionClass& another) { JMP_THIS(0x55A6C0); }

protected:
	explicit __forceinline LocomotionClass(noinit_t)  noexcept
	{
	}

	//Properties
public:

	FootClass* Owner;
	FootClass* LinkedTo;
	bool Powered;
	bool Dirty;
	int RefCount;
};

template <typename T = LocomotionClass*>
__forceinline T locomotion_cast(ILocomotion* iLoco)
{
	using Base = std::remove_const_t<std::remove_pointer_t<T>>;
	static_assert(std::is_base_of<LocomotionClass, Base>::value,
		"locomotion_cast: T is required to be a sub-class of LocomotionClass.");

	return static_cast<T>(iLoco);
}

template<typename T = LocomotionClass*>
__forceinline T locomotion_cast(YRComPtr<ILocomotion>& comLoco)
{
	return locomotion_cast<T>(comLoco.get());
}
