#include "LaserTrailTypeClass.h"

#include <Utilities/TemplateDef.h>
#include <HouseClass.h>

Enumerable<LaserTrailTypeClass>::container_t Enumerable<LaserTrailTypeClass>::Array;

const char* Enumerable<LaserTrailTypeClass>::GetMainSection()
{
	return "LaserTrailTypes";
}

void LaserTrailTypeClass::LoadFromINI(CCINIClass* pINI)
{
	const char* section = this->Name;

	INI_EX exINI(pINI);

	this->IsHouseColor.Read(exINI, section, "IsHouseColor");
	this->IsRainbowColor.Read(exINI, section, "IsRainbowColor");

	for (size_t i = 0; i <= this->Colors.size(); ++i)
	{
		Nullable<ColorStruct> color;
		_snprintf_s(Phobos::readBuffer, Phobos::readLength, "Color%d", i);
		color.Read(exINI, section, Phobos::readBuffer);

		if (i == this->Colors.size() && !color.isset())
		{
			if (i == 0)
			{
				_snprintf_s(Phobos::readBuffer, Phobos::readLength, "Color");
				color.Read(exINI, section, Phobos::readBuffer);
				if (!color.isset())
					break;
			}
			else
				break;
		}
		else if (!color.isset())
		{
			if (i == 0)
			{
				_snprintf_s(Phobos::readBuffer, Phobos::readLength, "Color");
				color.Read(exINI, section, Phobos::readBuffer);
				if (!color.isset())
					continue;
			}
			else
				continue;
		}

		if (i == this->Colors.size())
			this->Colors.push_back(color);
		else
			this->Colors[i] = color;
	}

	this->TransitionDuration.Read(exINI, section, "TransitionDuration");
	this->FadeDuration.Read(exINI, section, "FadeDuration");
	this->Thickness.Read(exINI, section, "Thickness");
	this->SegmentLength.Read(exINI, section, "SegmentLength");
	this->IgnoreVertical.Read(exINI, section, "IgnoreVertical");
	this->IsIntense.Read(exINI, section, "IsIntense");

	this->IsEBolt.Read(exINI, section, "IsEBolt");
}

template <typename T>
void LaserTrailTypeClass::Serialize(T& Stm)
{
	Stm
		.Process(this->IsHouseColor)
		.Process(this->IsRainbowColor)
		.Process(this->Colors)
		.Process(this->TransitionDuration)
		.Process(this->FadeDuration)
		.Process(this->Thickness)
		.Process(this->SegmentLength)
		.Process(this->IgnoreVertical)
		.Process(this->IsIntense)
		.Process(this->IsEBolt)
		;
}

void LaserTrailTypeClass::LoadFromStream(PhobosStreamReader& Stm)
{
	this->Serialize(Stm);
}

void LaserTrailTypeClass::SaveToStream(PhobosStreamWriter& Stm)
{
	this->Serialize(Stm);
}
