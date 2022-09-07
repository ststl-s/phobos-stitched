#pragma once

#include <YRPPCore.h>
#include <Interfaces.h>
#include <Helpers/CompileTime.h>

class NOVTABLE GScreenClass : public IGameMap
{
public:
	//Static
	static constexpr constant_ptr<GScreenClass, 0x87F7E8u> const Instance{};

	//IUnknown
	virtual HRESULT __stdcall QueryInterface(REFIID iid, void** ppvObject) JMP_STD(0x4F4240);
	virtual ULONG __stdcall AddRef() { return 1; }
	virtual ULONG __stdcall Release() { return 1; }

	//IGameMap
	//virtual long __stdcall Is_Visible(CellStruct cell) = 0;

	//Destructor
	virtual ~GScreenClass() JMP_THIS(0x4F4C00);

	//GScreenClass
	virtual void One_Time() JMP_THIS(0x4F42A0);
	
	virtual void vt_entry_18()
	{
		this->Init_Clear();
		this->vt_entry_20();
		//JMP_THIS(0x4F42B0);
	}
	
	virtual void Init_Clear() { this->Bitfield = 2; }
	virtual void vt_entry_20() JMP_THIS(0x4F42E0);
	virtual void GetInputAndUpdate(DWORD* outKeyCode, int* outMouseX, int* outMouseY) JMP_THIS(0x4F4320);
	virtual void Update(const int& keyCode, const Point2D& mouseCoords) JMP_THIS(0x4F4BB0);
	virtual bool vt_entry_2C(DWORD dwUnk) JMP_THIS(0x4F43F0);
	virtual bool vt_entry_30(DWORD dwUnk) JMP_THIS(0x4F4410);
	virtual bool vt_entry_34(DWORD dwUnk) JMP_THIS(0x4F4450);
	virtual void MarkNeedsRedraw(int dwUnk) JMP_THIS(0x4F42F0);
	virtual void DrawOnTop() JMP_THIS(0x4F4480);
	virtual void Draw(DWORD dwUnk) { };
	virtual void vt_entry_44() JMP_THIS(0x4F45B0);
	virtual bool SetCursor(MouseCursorType idxCursor, bool miniMap) = 0;
	virtual bool UpdateCursor(MouseCursorType idxCursor, bool miniMap) = 0;
	virtual bool RestoreCursor() = 0;
	virtual void UpdateCursorMinimapState(bool miniMap) = 0;

	void Render() { JMP_THIS(0x4F4480); }

protected:
	//Constuctor
	GScreenClass() { JMP_THIS(0x4F4220); }	//don't need this

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:
	int ScreenShakeX;
	int ScreenShakeY;
	int Bitfield;	//default is 2
};
