#pragma once

#include <PowerClass.h>
#include <StageClass.h>

class ColorScheme;
class FactoryClass;

// SidebarClass::StripClass::BuildType
struct BuildType
{
	int               ItemIndex{ -1 };
	AbstractType      ItemType{ AbstractType::None };
	bool              IsAlt{ false }; // set on buildings that go on tab 2
	FactoryClass*     CurrentFactory{ nullptr };
	DWORD             unknown_10{ 0 };
	StageClass        Progress{}; // 0 to 54, how much of this object is constructed (gclock anim level)
	int               FlashEndFrame{ 0 };

	BuildType() = default;

	BuildType(int itemIndex, AbstractType itemType) :
		ItemIndex(itemIndex),
		ItemType(itemType)
	{ /*JMP_THIS(0x6AC7C0);*/ }

	bool operator == (const BuildType& rhs) const
	{
		return ItemIndex == rhs.ItemIndex && ItemType == rhs.ItemType;
	}

	bool operator != (const BuildType& rhs) const
	{
		return ItemIndex != rhs.ItemIndex || ItemType != rhs.ItemType;
	}

	bool operator < (const BuildType& rhs) const
	{
		return SortsBefore(this->ItemType, this->ItemIndex, rhs.ItemType, rhs.ItemIndex);
	}

	static bool __stdcall SortsBefore(AbstractType leftType, int leftIndex, AbstractType rightType, int rightIndex)
	{ JMP_STD(0x6A8420); }
};

// SidebarClass::StripClass
struct StripClass
{
	StageClass        Progress;
	bool              AllowedToDraw; // prevents redrawing when layouting the list
	PROTECTED_PROPERTY(BYTE, align_1D[3]);
	Point2D           Location;
	RectangleStruct   Bounds;
	int               Index; // the index of this tab
	bool              NeedsRedraw;
	BYTE              unknown_3D;
	BYTE              unknown_3E;
	BYTE              unknown_3F;
	DWORD             unknown_40;
	int               TopRowIndex; // scroll position, which row is topmost visible
	DWORD             unknown_48;
	DWORD             unknown_4C;
	DWORD             unknown_50;
	int               CameoCount; // filled cameos
	BuildType         Cameos[75];
};

class NOVTABLE SidebarClass : public PowerClass
{
public:
	//Static
	static constexpr constant_ptr<SidebarClass, 0x87F7E8u> const Instance{};

	static constexpr reference<wchar_t, 0xB07BC4u, 0x42u> const TooltipBuffer{};

	void SidebarNeedsRepaint(int mode = 0)
	{
		this->SidebarNeedsRedraw = true;
		this->SidebarBackgroundNeedsRedraw = true;
		this->Tabs[this->ActiveTabIndex].AllowedToDraw = true;
		this->Tabs[this->ActiveTabIndex].NeedsRedraw = true;
		this->RedrawSidebar(mode);
		SidebarClass::Draw(1);
	}

	void RepaintSidebar(int tab = 0)
	{ JMP_THIS(0x6A60A0); }

	bool AddCameo(AbstractType absType, int typeIdx)
	{ JMP_THIS(0x6A6300); }

	//Destructor
	virtual ~SidebarClass() override JMP_THIS(0x6AC7F0);

	//GScreenClass
	virtual void One_Time() override JMP_THIS(0x6A5000);
	virtual void Init_Clear() override JMP_THIS(0x6A5030);
	virtual void vt_entry_20() override JMP_THIS(0x6A5310);
	virtual void Update(const int& keyCode, const Point2D& mouseCoords) override JMP_THIS(0x6A7780);
	virtual void Draw(DWORD dwUnk) override JMP_THIS(0x6A6C30);

	//DisplayClass
	virtual HRESULT Load(IStream* pStm) override JMP_THIS(0x6AC5D0);
	virtual HRESULT Save(IStream* pStm) override JMP_THIS(0x6AC5E0);
	virtual const wchar_t* GetToolTip(UINT nDlgID) override JMP_THIS(0x6AC210);
	virtual void CloseWindow() override JMP_THIS(0x6ABD30); //prolly wrong naming

	//RadarClass
	virtual void DisposeOfArt() override JMP_THIS(0x6A5BF0);
	virtual void Init_For_House() override JMP_THIS(0x6A5840);

	//SidebarClass
	virtual bool vt_entry_D8(int dwUnk) R0;

	//Non-virtual

	// which tab does the 'th object of that type belong in?
	static int __fastcall GetObjectTabIdx(AbstractType absType, int typeIdx, int unused)
		{ JMP_STD(0x6ABC60); }

	// which tab does the 'th object of that type belong in?
	static int __fastcall GetObjectTabIdx(AbstractType absType, BuildCat buildCat, bool isNaval)
		{ JMP_STD(0x6ABCD0); }

protected:
	//Constructor
	SidebarClass() noexcept	//don't need this
	{ JMP_THIS(0x6A4E60); }

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:
	StripClass Tabs[0x4];
	DWORD unknown_5394;
	DWORD unknown_5398;
	int ActiveTabIndex;
	DWORD unknown_53A0;
	bool HideObjectNameInTooltip; // see 0x6A9343
	bool IsSidebarActive;
	bool SidebarNeedsRedraw;
	bool SidebarBackgroundNeedsRedraw;
	bool unknown_bool_53A8;

	//Information for the Diplomacy menu, I believe
	HouseClass* DiplomacyHouses[0x8];		//8 players max!
	int DiplomacyKills[0x8];		//total amount of kills per house
	int DiplomacyOwned[0x8];		//total amount of currently owned unit/buildings per house
	int DiplomacyPowerDrain[0x8];	//current power drain per house
	ColorScheme* DiplomacyColors[0x8];		//color scheme per house
	DWORD unknown_544C[0x8];			//??? per house - unused
	DWORD unknown_546C[0x8];			//??? per house - unused
	DWORD unknown_548C[0x8];			//??? per house - unused
	DWORD unknown_54AC[0x8];			//??? per house - unused
	DWORD unknown_54CC[0x8];			//??? per house - unused
	DWORD unknown_54EC[0x8];			//??? per house - unused
	BYTE unknown_550C;
	int DiplomacyNumHouses;			//possibly?

	bool unknown_bool_5514;
	bool unknown_bool_5515;
	PROTECTED_PROPERTY(BYTE, padding_5516[2]);
};
