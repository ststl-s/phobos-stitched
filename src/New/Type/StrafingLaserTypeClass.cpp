#include "StrafingLaserTypeClass.h"

#include <Utilities/TemplateDef.h>
#include <HouseClass.h>

Enumerable<StrafingLaserTypeClass>::container_t Enumerable<StrafingLaserTypeClass>::Array;

const char* Enumerable<StrafingLaserTypeClass>::GetMainSection()
{
	return "StrafingLaserTypes";
}

void StrafingLaserTypeClass::AddNewINIList(CCINIClass* pINI, const char* pSection, const char* pKey)
{
	DynamicVectorClass<StrafingLaserTypeClass*> objectsList;
	char* context = nullptr;

	pINI->ReadString(pSection, pKey, "", Phobos::readBuffer);

	for (char* cur = strtok_s(Phobos::readBuffer, Phobos::readDelims, &context); cur; cur = strtok_s(nullptr, Phobos::readDelims, &context))
	{
		if (StrafingLaserTypeClass::Find(cur))
			continue;

		objectsList.AddItem(StrafingLaserTypeClass::Allocate(cur));
	}

	for (const auto pObj : objectsList)
	{
		pObj->LoadFromINI(pINI);
	}
}

void StrafingLaserTypeClass::LoadFromINI(CCINIClass* pINI)
{
	const char* pSection = this->Name;
	if (strcmp(pSection, NONE_STR) == 0)
		return;

	INI_EX exINI(pINI);

	this->InnerColor.Read(exINI, pSection, "InnerColor");
	this->OuterColor.Read(exINI, pSection, "OuterColor");
	this->OuterSpread.Read(exINI, pSection, " OuterSpread");
	this->IsHouseColor.Read(exINI, pSection, "IsHouseColor");
	this->IsSingleColor.Read(exINI, pSection, "IsSingleColor");
	this->IsSupported.Read(exINI, pSection, "IsSupported");
	this->Duration.Read(exINI, pSection, "Duration");
	this->Thickness.Read(exINI, pSection, "Thickness");
	this->EndThickness.Read(exINI, pSection, "EndThickness");

	this->FLH.Read(exINI, pSection, "FLH");
	this->SourceFLH.Read(exINI, pSection, "SourceFLH");
	this->TargetFLH.Read(exINI, pSection, "TargetFLH");
	this->InGround.Read(exINI, pSection, "InGround");
	this->SourceFromTarget.Read(exINI, pSection, "SourceFromTarget");
	this->Timer.Read(exINI, pSection, "Timer");
	this->Weapon.Read(exINI, pSection, "Weapon");
	this->Weapon_Delay.Read(exINI, pSection, "WeaponDelay");
	this->DetonateWeapon.Read(exINI, pSection, "DetonateWeapon");
}

template <typename T>
void StrafingLaserTypeClass::Serialize(T& Stm)
{
	Stm
		.Process(this->InnerColor)
		.Process(this->OuterColor)
		.Process(this->OuterSpread)
		.Process(this->IsHouseColor)
		.Process(this->IsSingleColor)
		.Process(this->IsSupported)
		.Process(this->Duration)
		.Process(this->Thickness)
		.Process(this->EndThickness)

		.Process(this->FLH)
		.Process(this->SourceFLH)
		.Process(this->TargetFLH)
		.Process(this->InGround)
		.Process(this->SourceFromTarget)
		.Process(this->Timer)
		.Process(this->Weapon)
		.Process(this->Weapon_Delay)
		.Process(this->DetonateWeapon)
		;
}

void StrafingLaserTypeClass::LoadFromStream(PhobosStreamReader& Stm)
{
	this->Serialize(Stm);
}

void StrafingLaserTypeClass::SaveToStream(PhobosStreamWriter& Stm)
{
	this->Serialize(Stm);
}
