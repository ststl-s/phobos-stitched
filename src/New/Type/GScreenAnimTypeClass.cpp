#include "GScreenAnimTypeClass.h"

#include <Utilities/TemplateDef.h>

Enumerable<GScreenAnimTypeClass>::container_t Enumerable<GScreenAnimTypeClass>::Array;

const char* Enumerable<GScreenAnimTypeClass>::GetMainSection()
{
	return "GScreenAnimTypes";
}

void GScreenAnimTypeClass::LoadFromINI(CCINIClass* pINI)
{
	const char* section = this->Name;

	INI_EX exINI(pINI);

	this->ShowAnim_SHP.Read(pINI, section, "ShowAnim.SHP");
	this->ShowAnim_PAL.Read(pINI, section, "ShowAnim.PAL");
	this->ShowAnim_Offset.Read(exINI, section, "ShowAnim.Offset");
	this->ShowAnim_TranslucentLevel.Read(exINI, section, "ShowAnim.TranslucentLevel");
	this->ShowAnim_FrameKeep.Read(exINI, section, "ShowAnim.FrameKeep");
	this->ShowAnim_LoopCount.Read(exINI, section, "ShowAnim.LoopCount");
	this->ShowAnim_CoolDown.Read(exINI, section, "ShowAnim.CoolDown");

	Debug::Log("[GScreenAnimTypeClass] LoadFromINIFile\n");

	SHP_ShowAnim = FileSystem::LoadSHPFile(ShowAnim_SHP);

	if (strcmp(ShowAnim_PAL.data(), "") == 0)
		PAL_ShowAnim = FileSystem::ANIM_PAL;
	else
		PAL_ShowAnim = FileSystem::LoadPALFile(ShowAnim_PAL.data(), DSurface::Composite);

	if (SHP_ShowAnim == nullptr)
		Debug::Log("[GScreenAnimTypeClass::Error] SHP file \"%s\" not found\n", ShowAnim_SHP.data());
	if (PAL_ShowAnim == nullptr)
		Debug::Log("[GScreenAnimTypeClass::Error] PAL file \"%s\" not found\n", ShowAnim_PAL.data());
}

template <typename T>
void GScreenAnimTypeClass::Serialize(T& Stm)
{
	Stm
		.Process(this->ShowAnim_SHP)
		.Process(this->ShowAnim_PAL)
		.Process(this->ShowAnim_Offset)
		.Process(this->ShowAnim_TranslucentLevel)
		.Process(this->ShowAnim_FrameKeep)
		.Process(this->ShowAnim_LoopCount)
		.Process(this->ShowAnim_CoolDown)
		;
}

void GScreenAnimTypeClass::LoadFromStream(PhobosStreamReader& Stm)
{
	this->Serialize(Stm);

	Debug::Log("[GScreenAnimTypeClass] LoadFromStream\n");

	SHP_ShowAnim = FileSystem::LoadSHPFile(ShowAnim_SHP);

	if (strcmp(ShowAnim_PAL.data(), "") == 0)
		PAL_ShowAnim = FileSystem::ANIM_PAL;
	else
		PAL_ShowAnim = FileSystem::LoadPALFile(ShowAnim_PAL.data(), DSurface::Composite);

	if (SHP_ShowAnim == nullptr)
		Debug::Log("[GScreenAnimTypeClass::Error] SHP file \"%s\" not found\n", ShowAnim_SHP.data());
	if (PAL_ShowAnim == nullptr)
		Debug::Log("[GScreenAnimTypeClass::Error] PAL file \"%s\" not found\n", ShowAnim_PAL.data());
}

void GScreenAnimTypeClass::SaveToStream(PhobosStreamWriter& Stm)
{
	this->Serialize(Stm);
}
