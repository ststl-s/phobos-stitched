#include "TunnelTypeClass.h"

#include <Utilities/TemplateDef.h>

Enumerable<TunnelTypeClass>::container_t Enumerable<TunnelTypeClass>::Array;

const char* Enumerable<TunnelTypeClass>::GetMainSection()
{
	return "TunnelType";
}

TunnelTypeClass::TunnelTypeClass(const char* pName)
	: Enumerable<TunnelTypeClass>(pName)
	, Passengers(0)
	, MaxSize(INT_MAX)
{ }

void TunnelTypeClass::LoadFromINI(CCINIClass* pINI)
{
	INI_EX exINI(pINI);

	const char* section = this->Name;

	this->Passengers.Read(exINI, section, "Passengers");
	this->MaxSize.Read(exINI, section, "MaxSize");
}

void TunnelTypeClass::LoadFromStream(PhobosStreamReader& stm)
{
	stm
		.Process(this->Passengers)
		.Process(this->MaxSize)
		;
}

void TunnelTypeClass::SaveToStream(PhobosStreamWriter& stm)
{
	stm
		.Process(this->Passengers)
		.Process(this->MaxSize)
		;
}
