#pragma once

#include <AbstractTypeClass.h>

class NOVTABLE CampaignClass : public AbstractTypeClass
{

public:
	static const AbstractType AbsID = AbstractType::Campaign;

	ABSTRACTTYPE_ARRAY(CampaignClass, 0xA83CF8);

	//IPersist
	virtual HRESULT __stdcall GetClassID(CLSID* pClassID) override JMP_STD(0x46CF80);

	//IPersistStream
	virtual HRESULT __stdcall Load(IStream* pStm) override JMP_STD(0x46D000);
	virtual HRESULT __stdcall Save(IStream* pStm, BOOL fClearDirty) override JMP_STD(0x46D050);

	//Destructor
	virtual ~CampaignClass() override RX;// JMP_THIS(0x46D090);

	//AbstractClass
	virtual AbstractType WhatAmI() const override JMP_THIS(0x46D080);
	virtual int Size() const override JMP_THIS(0x46D070);
	virtual void ComputeCRC(CRCEngine& crc) const override JMP_THIS(0x46CFC0);

	//AbstractTypeClass
	virtual bool LoadFromINI(CCINIClass* pINI) override JMP_THIS(0x46CCD0);

	CampaignClass(const char* pID) noexcept
		: CampaignClass(noinit_t())
	{
		JMP_THIS(0x46CB60);
	}

protected:
	explicit __forceinline CampaignClass(noinit_t) noexcept
		: AbstractTypeClass(noinit_t())
	{
	}

public:
	static void __fastcall CreateFromINIList(CCINIClass* pINI)
	{ JMP_STD(0x46CE10); }

	static signed int __fastcall FindIndex(const char* pID)
	{ JMP_STD(0x46CC90); }

public:
	int idxCD;
	char Scenario[512];
	int FinalMovie;
	wchar_t Description[128];
};
