#pragma once

#include <Utilities/Enum.h>
#include <Utilities/Enumerable.h>
#include <Utilities/Template.h>

class AttachmentTypeClass final : public Enumerable<AttachmentTypeClass>
{
public:
	Valueable<bool> RestoreAtCreation; // whether to spawn the attachment initially
	Valueable<int> RestoreDelay;
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
	Valueable<bool> InheritWeaponOwner;
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
	Valueable<bool> InheritWeaponOwner_Parent;
	Valueable<bool> CanBeForceDetached;
	Nullable<WeaponTypeClass*> ForceDetachWeapon_Child;
	Nullable<WeaponTypeClass*> ForceDetachWeapon_Parent;
	Nullable<WeaponTypeClass*> DestructionWeapon_Child;
	Nullable<WeaponTypeClass*> DestructionWeapon_Parent;
	Nullable<Mission> ParentDestructionMission;
	Nullable<Mission> ChildDestructionMission;
	Nullable<Mission> ParentDetachmentMission;
	Nullable<Mission> ChildDetachmentMission;
	Valueable<CoordStruct> FLHoffset;
	Valueable<bool> MoveSelectToParent;
	Valueable<bool> MoveTargetToParent;
	Valueable<int> MaxCount;

	AttachmentTypeClass(const char* pTitle = NONE_STR) : Enumerable<AttachmentTypeClass>(pTitle)
		, RestoreAtCreation { true }
		, RestoreDelay { -1 }
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
		, InheritWeaponOwner { true }
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
		, InheritWeaponOwner_Parent { false }
		, YSortPosition { AttachmentYSortPosition::Default }
		, CanBeForceDetached { false }
		, ForceDetachWeapon_Child { }
		, ForceDetachWeapon_Parent { }
		, DestructionWeapon_Child { }
		, DestructionWeapon_Parent { }
		, ParentDestructionMission { }
		, ChildDestructionMission { }
		, ParentDetachmentMission { }
		, ChildDetachmentMission { }
		, FLHoffset { { 0, 0, 0 } }
		, MoveSelectToParent { false }
		, MoveTargetToParent { false }
		, MaxCount { -1 }
	{ }

	virtual ~AttachmentTypeClass() override = default;

	virtual void LoadFromINI(CCINIClass* pINI) override;
	virtual void LoadFromStream(PhobosStreamReader& Stm);
	virtual void SaveToStream(PhobosStreamWriter& Stm);

private:
	template <typename T>
	void Serialize(T& Stm);
};
