#include "AttachmentTypeClass.h"

#include <Utilities/TemplateDef.h>

Enumerable<AttachmentTypeClass>::container_t Enumerable<AttachmentTypeClass>::Array;

const char* Enumerable<AttachmentTypeClass>::GetMainSection()
{
	return "AttachmentTypes";
}

void AttachmentTypeClass::LoadFromINI(CCINIClass* pINI)
{
	const char* section = this->Name;

	INI_EX exINI(pINI);

	this->RestoreAtCreation.Read(exINI, section, "RestoreAtCreation");
	this->RestoreDelay.Read(exINI, section, "RestoreDelay");

	this->InheritTilt.Read(exINI, section, "InheritTilt");
	this->InheritCommands.Read(exINI, section, "InheritCommands");
	this->InheritOwner.Read(exINI, section, "InheritOwner");
	this->InheritStateEffects.Read(exINI, section, "InheritStateEffects");
	this->InheritDestruction.Read(exINI, section, "InheritDestruction");
	this->InheritDestruction_Type.Read(exINI, section, "InheritDestruction.Type");
	this->InheritTarget.Read(exINI, section, "InheritTarget");
	this->InheritAmmo.Read(exINI, section, "InheritAmmo");
	this->InheritHealth.Read(exINI, section, "InheritHealth");
	this->InheritVeterancy.Read(exINI, section, "InheritVeterancy");
	this->InheritWeaponOwner.Read(exINI, section, "InheritWeaponOwner");

	this->LowSelectionPriority.Read(exINI, section, "LowSelectionPriority");
	this->YSortPosition.Read(exINI, section, "YSortPosition");

	this->Experience_ParentModifier.Read(exINI, section, "Experience.ParentModifier");
	this->Experience_ChildModifier.Read(exINI, section, "Experience.ChildModifier");

	this->InheritCommands_Parent.Read(exINI, section, "InheritCommands.Parent");
	this->InheritStateEffects_Parent.Read(exINI, section, "InheritStateEffects.Parent");
	this->InheritDestruction_Parent.Read(exINI, section, "InheritDestruction.Parent");
	this->InheritDestruction_Parent_Type.Read(exINI, section, "InheritDestruction.Parent.Type");
	this->InheritTarget_Parent.Read(exINI, section, "InheritTarget.Parent");
	this->InheritAmmo_Parent.Read(exINI, section, "InheritAmmo.Parent");
	this->InheritHealth_Parent.Read(exINI, section, "InheritHealth.Parent");
	this->InheritVeterancy_Parent.Read(exINI, section, "InheritVeterancy.Parent");
	this->InheritWeaponOwner_Parent.Read(exINI, section, "InheritWeaponOwner.Parent");

	this->CanBeForceDetached.Read(exINI, section, "CanBeForceDetached");

	this->ForceDetachWeapon_Child.Read(exINI, section, "ForceDetachWeapon.Child");
	this->ForceDetachWeapon_Parent.Read(exINI, section, "ForceDetachWeapon.Parent");
	this->DestructionWeapon_Child.Read(exINI, section, "DestructionWeapon.Child");
	this->DestructionWeapon_Parent.Read(exINI, section, "DestructionWeapon.Parent");

	this->ParentDestructionMission.Read(exINI, section, "ParentDestructionMission");
	this->ChildDestructionMission.Read(exINI, section, "ChildDestructionMission");
	this->ParentDetachmentMission.Read(exINI, section, "ParentDetachmentMission");
	this->ChildDetachmentMission.Read(exINI, section, "ChildDetachmentMission");

	this->FLHoffset.Read(exINI, section, "FLHoffset");
	this->MoveSelectToParent.Read(exINI, section, "MoveSelectToParent");
	this->MoveTargetToParent.Read(exINI, section, "MoveTargetToParent");
	this->MaxCount.Read(exINI, section, "MaxCount");
}

template <typename T>
void AttachmentTypeClass::Serialize(T& Stm)
{
	Stm
		.Process(this->RestoreAtCreation)
		.Process(this->RestoreDelay)
		.Process(this->InheritTilt)
		.Process(this->InheritCommands)
		.Process(this->InheritOwner)
		.Process(this->InheritStateEffects)
		.Process(this->InheritDestruction)
		.Process(this->InheritDestruction_Type)
		.Process(this->InheritTarget)
		.Process(this->InheritAmmo)
		.Process(this->InheritHealth)
		.Process(this->InheritVeterancy)
		.Process(this->InheritWeaponOwner)
		.Process(this->LowSelectionPriority)
		.Process(this->YSortPosition)
		.Process(this->Experience_ParentModifier)
		.Process(this->Experience_ChildModifier)
		.Process(this->InheritCommands_Parent)
		.Process(this->InheritOwner_Parent)
		.Process(this->InheritStateEffects_Parent)
		.Process(this->InheritDestruction_Parent)
		.Process(this->InheritDestruction_Parent_Type)
		.Process(this->InheritTarget_Parent)
		.Process(this->InheritAmmo_Parent)
		.Process(this->InheritHealth_Parent)
		.Process(this->InheritVeterancy_Parent)
		.Process(this->InheritWeaponOwner_Parent)
		.Process(this->CanBeForceDetached)
		.Process(this->ForceDetachWeapon_Child)
		.Process(this->ForceDetachWeapon_Parent)
		.Process(this->DestructionWeapon_Child)
		.Process(this->DestructionWeapon_Parent)
		.Process(this->ParentDestructionMission)
		.Process(this->ChildDestructionMission)
		.Process(this->ParentDetachmentMission)
		.Process(this->ChildDetachmentMission)
		.Process(this->FLHoffset)
		.Process(this->MoveSelectToParent)
		.Process(this->MoveTargetToParent)
		.Process(this->MaxCount)
		;
}

void AttachmentTypeClass::LoadFromStream(PhobosStreamReader& Stm)
{
	this->Serialize(Stm);
}

void AttachmentTypeClass::SaveToStream(PhobosStreamWriter& Stm)
{
	this->Serialize(Stm);
}
