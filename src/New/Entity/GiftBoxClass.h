#pragma once

#include <Utilities/Template.h>

#include <Ext/TechnoType/Body.h>

// Name : GiftBox
// Original Author : ChrisLV-CN
// https://github.com/ChrisLv-CN/PatcherExtension/blob/main/MyExtension/GiftBox.cs
// Port : Otamaa

class GiftBoxClass
{
public:

	GiftBoxClass() = default;

	GiftBoxClass(TechnoClass* pTechno)
		: Techno(pTechno)
		, IsEnabled(false)
		, IsTechnoChange(false)
		, IsOpen(false)
		, Delay(0)
	{
		strcpy_s(this->TechnoID.data(), this->Techno->get_ID());
	}

	~GiftBoxClass() = default;

	bool Open()
	{
		return IsOpen ? false : CheckDelay();
	}

	bool CheckDelay()
	{
		//dont execute everytime if not enabled
		if (IsEnabled)
		{
			if (--Delay <= 0)
			{
				IsOpen = true;
				return true;
			}
		}

		return false;
	}

	void Reset(int delay)
	{
		Delay = delay;
		IsOpen = false;
		IsTechnoChange = false;
	}

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange);

	bool Save(PhobosStreamWriter& Stm) const;

	const void AI();
	const bool CreateType(int nAt, TechnoTypeExt::ExtData::GiftBoxDataEntry& nGbox, CoordStruct nCoord, CoordStruct nDestCoord);
	const bool OpenDisallowed();

	static void InitializeGiftBox(TechnoClass* pTechno);
	static CoordStruct GetRandomCoordsNear(TechnoTypeExt::ExtData::GiftBoxDataEntry& nGiftBox, CoordStruct nCoord);
	static void SyncToAnotherTechno(TechnoClass* pFrom, TechnoClass* pTo);

	TechnoClass* Techno = nullptr;
	bool IsEnabled = false;
	bool IsTechnoChange = false;
	bool IsOpen = false;
	int Delay = 0;
	PhobosFixedString<0x18> TechnoID { "" };

private:
	template <typename T>
	bool Serialize(T& stm);
};
