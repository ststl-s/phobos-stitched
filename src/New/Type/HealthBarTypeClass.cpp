#include "HealthBarTypeClass.h"

#include <HouseClass.h>

#include <New/Type/DigitalDisplayTypeClass.h>

#include <Utilities/TemplateDef.h>

Enumerable<HealthBarTypeClass>::container_t Enumerable<HealthBarTypeClass>::Array;

const char* Enumerable<HealthBarTypeClass>::GetMainSection()
{
	return "HealthBarTypes";
}

void HealthBarTypeClass::LoadFromINI(CCINIClass* pINI)
{
	const char* pSection = this->Name;
	if (strcmp(pSection, NONE_STR) == 0)
		return;

	INI_EX exINI(pINI);

	this->GroupID_Offset.Read(exINI, pSection, "GroupID.Offset");

	this->UnitHealthBar.Read(exINI, pSection, "UnitHealthBar");
	this->UnitHealthBar_HideBrd.Read(exINI, pSection, "UnitHealthBar.HideBrd");
	this->UnitHealthBar_IgnoreBuildingHeight.Read(exINI, pSection, "UnitHealthBar.IgnoreBuildingHeight");

	this->SelfHealPips_Frame.Read(exINI, pSection, "SelfHeal");
	this->SelfHealPips_Offset.Read(exINI, pSection, "SelfHeal.Offset");

	this->Pips.Read(exINI, pSection, "Pips");
	this->Pips_Empty.Read(exINI, pSection, "Pips.Empty");
	this->PipsSHP.Read(exINI, pSection, "Pips.SHP");
	this->PipsPAL.LoadFromINI(pINI, pSection, "Pips.PAL");

	this->PipBrd.Read(exINI, pSection, "PipBrd");
	this->PipBrdOffset.Read(exINI, pSection, "PipBrd.Offset");
	this->PipBrdSHP.Read(exINI, pSection, "PipBrd.SHP");
	this->PipBrdPAL.LoadFromINI(pINI, pSection, "PipBrd.PAL");

	this->Length.Read(exINI, pSection, "Length");
	this->DrawOffset.Read(exINI, pSection, "DrawOffset");
	this->XOffset.Read(exINI, pSection, "XOffset");
	this->YOffset.Read(exINI, pSection, "YOffset");

	this->PictureHealthBar.Read(exINI, pSection, "PictureHealthBar");
	this->PictureSHP.Read(exINI, pSection, "PictureHealthBar.SHP");
	this->PicturePAL.LoadFromINI(pINI, pSection, "PictureHealthBar.PAL");
	this->PictureTransparency.Read(exINI, pSection, "PictureHealthBar.Transparency");

	DigitalDisplayTypeClass::LoadFromVecotrINIList(pINI, pSection, "DigitalDisplayTypes");
	this->DigitalDisplayTypes.Read(exINI, pSection, "DigitalDisplayTypes");
	this->DigitalDisplay_Disable.Read(exINI, pSection, "DigitalDisplay.Disable");
	this->DigitalDisplay_ShowEnemy.Read(exINI, pSection, "DigitalDisplay.ShowEnemy");
}

template <typename T>
void HealthBarTypeClass::Serialize(T& Stm)
{
	Stm
		.Process(this->GroupID_Offset)

		.Process(this->UnitHealthBar)
		.Process(this->UnitHealthBar_HideBrd)
		.Process(this->UnitHealthBar_IgnoreBuildingHeight)

		.Process(this->SelfHealPips_Offset)
		.Process(this->SelfHealPips_Frame)

		.Process(this->Pips)
		.Process(this->Pips_Empty)
		.Process(this->PipsSHP)
		.Process(this->PipsPAL)

		.Process(this->PipBrd)
		.Process(this->PipBrdOffset)
		.Process(this->PipBrdSHP)
		.Process(this->PipBrdPAL)

		.Process(this->Length)
		.Process(this->DrawOffset)
		.Process(this->XOffset)
		.Process(this->YOffset)

		.Process(this->PictureHealthBar)
		.Process(this->PictureSHP)
		.Process(this->PicturePAL)
		.Process(this->PictureTransparency)

		.Process(this->DigitalDisplay_Disable)
		.Process(this->DigitalDisplayTypes)
		.Process(this->DigitalDisplay_ShowEnemy)
		;
}

void HealthBarTypeClass::LoadFromStream(PhobosStreamReader& Stm)
{
	this->Serialize(Stm);
}

void HealthBarTypeClass::SaveToStream(PhobosStreamWriter& Stm)
{
	this->Serialize(Stm);
}
