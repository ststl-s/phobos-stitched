#pragma once

#include <ToolTipManager.h>
#include <Drawing.h>

#include <Helpers/CompileTime.h>

class NOVTABLE CCToolTip : public ToolTipManager
{
public:
	// It's also used in MoneyFormat at 6A934A, not sure what side effect it might leads
	static constexpr reference<bool, 0x884B8C> HideName {};
	static constexpr reference<CCToolTip*, 0x887368> Instance {};
	static constexpr reference<RGBClass, 0xB0FA1C> ToolTipTextColor{};

	//Destructor
	virtual ~CCToolTip() override JMP_THIS(0x7784A0);

	// ToolTipManager
	virtual bool Update(ToolTipManagerData& from) override JMP_THIS(0x478BA0);
	virtual void MarkToRedraw(ToolTipManagerData& from) override JMP_THIS(0x478DB0);
	virtual void Draw(bool onSidebar) override JMP_THIS(0x478E10);
	virtual void DrawText(ToolTipManagerData& from) override JMP_THIS(0x478E30);
	virtual wchar_t* GetToolTipText(unsigned int id) override JMP_THIS(0x479050);

	//Properties
public:
	bool FullRedraw;
	int Delay;
};
