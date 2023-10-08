#pragma once

#include <YRPPCore.h>
#include <GeneralStructures.h>
#include <ArrayClasses.h>
#include <IndexClass.h>

struct ToolTip
{
	ToolTip()
	{ JMP_THIS(0x6D1810); }

	unsigned int GadgetID;
	RectangleStruct Bounds;
	const char* Text;
	bool field_18;
};

struct ToolTipManagerData
{
	RectangleStruct Dimension;
	wchar_t HelpText[0x100];
};

class NOVTABLE ToolTipManager
{
public:

	//Destructor
	virtual ~ToolTipManager() JMP_THIS(0x724C10);

	// ToolTipManager
	virtual bool Update(ToolTipManagerData& from) JMP_THIS(0x724AA0);
	virtual void MarkToRedraw(ToolTipManagerData& from) JMP_THIS(0x724AB0);
	virtual void Draw(bool onSidebar) JMP_THIS(0x724B80);
	virtual void DrawText(ToolTipManagerData& from) JMP_THIS(0x724BB0);
	virtual wchar_t* GetToolTipText(unsigned int id) JMP_THIS(0x724BD0);

	//Non virtual
	void SetState(bool state)
	{ JMP_THIS(0x7241A0); }

	void ProcessMessage(MSG* pMSG)
	{ JMP_THIS(0x724200); }

	int GetTimerDelay()
	{ JMP_THIS(0x724510); }

	void SetTimerDelay(int value)
	{ JMP_THIS(0x724520); }

	void SaveTimerDelay()
	{ JMP_THIS(0x724530); }

	void RestoreTimeDelay()
	{ JMP_THIS(0x724540); }

	int GetLifeTime()
	{ JMP_THIS(0x724550); }

	void SetLifeTime(int value)
	{ JMP_THIS(0x724560); }

	int GetToolTipCount()
	{ JMP_THIS(0x724570); }

	bool Add(ToolTip& toolTip)
	{ JMP_THIS(0x724580); }

	void Remove(unsigned int id)
	{ JMP_THIS(0x724730); }

	bool Find(unsigned int id, ToolTip& toolTip)
	{ JMP_THIS(0x7248C0); }

	ToolTip* FindFromPosition(Point2D& point)
	{ JMP_THIS(0x724A30); }

	bool Process()
	{ JMP_THIS(0x724AD0); }

	void Hide()
	{ JMP_THIS(0x724BE0); }

	bool IsToolTipShowing()
	{ JMP_THIS(0x724C00); }

	//Statics

	//Constructors
	ToolTipManager(HWND hWnd) noexcept
		: ToolTipManager(noinit_t())
	{ JMP_THIS(0x724000); }

protected:
	explicit __forceinline ToolTipManager(noinit_t)  noexcept
	{
	}

	//Properties
public:
	ToolTip* CurrentToolTip;
	HWND hWnd;
	bool IsActive;
	Point2D CurrentMousePosition;
	ToolTipManagerData CurrentToolTipData;
	int ToolTipDelay;
	int LastToolTipDelay;
	int ToolTipLifeTime;
	DynamicVectorClass<ToolTip*> ToolTips;
	IndexClass<int, ToolTip*> ToolTipIndex;
};

static_assert(sizeof(ToolTipManager) == 0x260);
