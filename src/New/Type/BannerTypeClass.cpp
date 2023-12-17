#include "BannerTypeClass.h"

#include <Utilities/TemplateDef.h>
#include <Utilities/GeneralUtils.h>

Enumerable<BannerTypeClass>::container_t Enumerable<BannerTypeClass>::Array;

const char* Enumerable<BannerTypeClass>::GetMainSection()
{
	return "BannerTypes";
}

void BannerTypeClass::LoadFromINI(CCINIClass* pINI)
{
	const char* section = this->Name;

	INI_EX exINI(pINI);

	this->Content_PCX.Read(pINI, section, "Content.PCX");
	this->Content_SHP.Read(exINI, section, "Content.SHP");
	this->Content_CSF.Read(exINI, section, "Content.CSF");
	this->Align.Read(exINI, section, "Align");
	this->Anchor.Read(exINI, section, "Anchor");
	this->ImageAlign.Read(exINI, section, "ImageAlign");
	this->Percentage.Read(exINI, section, "Percentage");
	this->Offset.Read(exINI, section, "Offset");
	this->Duration.Read(exINI, section, "Duration");

	if (this->Content_PCX)
	{
		this->Type = BannerType::PCX;
	}
	else if (this->Content_SHP.isset())
	{
		this->Type = BannerType::SHP;
		this->Content_PAL.LoadFromINI(pINI, section, "Content.SHP.Palette");
		this->Content_PAL_Name.Read(pINI, section, "Content.SHP.Palette");
		this->Content_SHP_FadeIn.Read(exINI, section, "Content.SHP.FadeIn");
		this->Content_SHP_FadeOut.Read(exINI, section, "Content.SHP.FadeOut");
		this->Content_SHP_Play.Read(exINI, section, "Content.SHP.Play");
		this->Content_SHP_Loop.Read(exINI, section, "Content.SHP.Loop");
		this->Content_SHP_Rate.Read(exINI, section, "Content.SHP.Rate");
		this->Content_SHP_Transparency.Read(exINI, section, "Content.SHP.Transparency");
		this->Content_SHP_Transparency_MouseHover.Read(exINI, section, "Content.SHP.Transparency.MouseHover");
		this->Content_SHP_OnlyOne.Read(exINI, section, "Content.SHP.OnlyOne");
		this->Content_SHP_Remove.Read(exINI, section, "Content.SHP.Remove");
		this->Content_SHP_Frame.Read(exINI, section, "Content.SHP.Frame");
		this->Content_SHP_End.Read(exINI, section, "Content.SHP.End");
		this->Content_SHP_Sound.Read(exINI, section, "Content.SHP.Sound");
		this->Content_SHP_FullScreen.Read(exINI, section, "Content.SHP.FullScreen");
		this->Content_SHP_DrawInShroud.Read(exINI, section, "Content.SHP.DrawInShroud");
	}
	else if (this->Content_CSF.Get())
	{
		this->Type = BannerType::CSF;
		this->Content_CSF_Color.Read(exINI, section, "Content.CSF.Color");
		this->Content_CSF_ColorShadow.Read(exINI, section, "Content.CSF.ColorShadow");
		this->Content_CSF_DrawBackground.Read(exINI, section, "Content.CSF.DrawBackground");
		this->Content_CSF_FadeIn.Read(exINI, section, "Content.CSF.FadeIn");
		this->Content_CSF_Sound.Read(exINI, section, "Content.CSF.Sound");
	}
}

template <typename T>
void BannerTypeClass::Serialize(T& Stm)
{
	Stm
		.Process(this->Content_PCX)
		.Process(this->Content_SHP)
		.Process(this->Content_PAL)
		.Process(this->Content_PAL_Name)
		.Process(this->Content_SHP_FadeIn)
		.Process(this->Content_SHP_FadeOut)
		.Process(this->Content_SHP_Play)
		.Process(this->Content_SHP_Loop)
		.Process(this->Content_SHP_Rate)
		.Process(this->Content_SHP_Transparency)
		.Process(this->Content_SHP_Transparency_MouseHover)
		.Process(this->Content_SHP_OnlyOne)
		.Process(this->Content_SHP_Remove)
		.Process(this->Content_SHP_Frame)
		.Process(this->Content_SHP_End)
		.Process(this->Content_SHP_Sound)
		.Process(this->Content_SHP_FullScreen)
		.Process(this->Content_SHP_DrawInShroud)
		.Process(this->Content_CSF)
		.Process(this->Content_CSF_Color)
		.Process(this->Content_CSF_ColorShadow)
		.Process(this->Content_CSF_DrawBackground)
		.Process(this->Content_CSF_FadeIn)
		.Process(this->Content_CSF_Sound)
		.Process(this->Type)
		.Process(this->Align)
		.Process(this->Anchor)
		.Process(this->ImageAlign)
		.Process(this->Percentage)
		.Process(this->Offset)
		.Process(this->Duration)
		;
}

void BannerTypeClass::LoadFromStream(PhobosStreamReader& Stm)
{
	this->Serialize(Stm);
}

void BannerTypeClass::SaveToStream(PhobosStreamWriter& Stm)
{
	this->Serialize(Stm);
}
