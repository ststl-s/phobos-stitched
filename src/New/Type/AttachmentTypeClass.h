#pragma once

#include <Utilities/Enum.h>
#include <Utilities/Enumerable.h>
#include <Utilities/Template.h>

#include <TechnoTypeClass.h>

class AttachmentTypeClass final : public Enumerable<AttachmentTypeClass>
{
public:
	Valueable<bool> RestoreAtCreation; // whether to spawn the attachment initially
	Valueable<bool> InheritTilt;
	Valueable<bool> InheritCommands;
	Valueable<bool> InheritOwner; // aka mind control inheritance
	Valueable<bool> InheritStateEffects; // phasing out, stealth etc.
	Valueable<bool> InheritDestruction;
	Valueable<AutoDeathBehavior> InheritDestruction_Type;
	Valueable<bool> InheritTarget;
	Valueable<bool> InheritAmmo;
	Valueable<bool> InheritHealth;
	Valueable<bool> InheritVeterancy;
	Valueable<bool> LowSelectionPriority;
	Valueable<AttachmentYSortPosition> YSortPosition;
	Valueable<double> Experience_ParentModifier;
	Valueable<double> Experience_ChildModifier;
	Valueable<bool> InheritCommands_Parent;
	Valueable<bool> InheritOwner_Parent;
	Valueable<bool> InheritStateEffects_Parent;
	Valueable<bool> InheritDestruction_Parent;
	Valueable<AutoDeathBehavior> InheritDestruction_Parent_Type;
	Valueable<bool> InheritTarget_Parent;
	Valueable<bool> InheritAmmo_Parent;
	Valueable<bool> InheritHealth_Parent;
	Valueable<bool> InheritVeterancy_Parent;
	// Valueable<bool> CanBeForceDetached;
	Nullable<WeaponTypeClass*> ForceDetachWeapon_Child;
	Nullable<WeaponTypeClass*> ForceDetachWeapon_Parent;
	Nullable<WeaponTypeClass*> DestructionWeapon_Child;
	Nullable<WeaponTypeClass*> DestructionWeapon_Parent;
	Nullable<Mission> ParentDestructionMission;
	Nullable<Mission> ChildDestructionMission;
	Nullable<Mission> ParentDetachmentMission;
	Nullable<Mission> ChildDetachmentMission;

	AttachmentTypeClass(const char* pTitle = NONE_STR) : Enumerable<AttachmentTypeClass>(pTitle)
		, RestoreAtCreation { true }
		, InheritTilt { true }
		, InheritCommands { true }
		, InheritOwner { true }
		, InheritStateEffects { true }
		, InheritDestruction { true }
		, InheritDestruction_Type { AutoDeathBehavior::Vanish }
		, InheritTarget { true }
		, InheritAmmo { true }
		, InheritHealth { true }
		, InheritVeterancy { true }
		, LowSelectionPriority { true }
		, Experience_ParentModifier { 1.0 }
		, Experience_ChildModifier { 1.0 }
		, InheritCommands_Parent { false }
		, InheritOwner_Parent { false }
		, InheritStateEffects_Parent { false }
		, InheritDestruction_Parent { false }
		, InheritDestruction_Parent_Type { AutoDeathBehavior::Kill }
		, InheritTarget_Parent { false }
		, InheritAmmo_Parent { false }
		, InheritHealth_Parent { false }
		, InheritVeterancy_Parent { false }
		, YSortPosition { AttachmentYSortPosition::Default }
		// , CanBeForceDetached { false }
		, ForceDetachWeapon_Child { }
		, ForceDetachWeapon_Parent { }
		, DestructionWeapon_Child { }
		, DestructionWeapon_Parent { }
		, ParentDestructionMission { }
		, ChildDestructionMission { }
		, ParentDetachmentMission { }
		, ChildDetachmentMission { }
	{ }

	virtual ~AttachmentTypeClass() override = default;

	virtual void LoadFromINI(CCINIClass* pINI) override;
	virtual void LoadFromStream(PhobosStreamReader& Stm);
	virtual void SaveToStream(PhobosStreamWriter& Stm);

private:
	template <typename T>
	void Serialize(T& Stm);
};
