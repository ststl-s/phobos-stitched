#include "AttachmentClass.h"

#include <BulletClass.h>
#include <BulletTypeClass.h>
#include <WarheadTypeClass.h>
#include <DriveLocomotionClass.h>

#include <ObjBase.h>

#include <Ext/Techno/Body.h>
#include <Utilities/PointerMapper.h>

std::vector<AttachmentClass*> AttachmentClass::Array;

AttachmentTypeClass* AttachmentClass::GetType()
{
	return AttachmentTypeClass::Array[this->Data->Type].get();
}

TechnoTypeClass* AttachmentClass::GetChildType()
{
	return this->Data->TechnoType[0];
}

void AttachmentClass::Initialize()
{
	if (this->Child)
		return;

	if (this->GetType()->RestoreAtCreation)
		this->CreateChild();
}

void AttachmentClass::CreateChild()
{
	if (auto const pChildType = this->GetChildType())
	{
		this->Child = abstract_cast<TechnoClass*>(pChildType->CreateObject(this->Parent->Owner));
		if (this->Child != nullptr)
		{
			//this->Child->IsOnMap = false;
			auto const pChildExt = TechnoExt::ExtMap.Find(this->Child);
			pChildExt->ParentAttachment = this;
		}
		else
		{
			Debug::Log("[" __FUNCTION__ "] Failed to create child %s of parent %s!\n",
				pChildType->ID, this->Parent->GetTechnoType()->ID);
		}
	}
}

void AttachmentClass::AI()
{
	AttachmentTypeClass* pType = this->GetType();

	if (this->Child)
	{
		this->Child->SetLocation(TechnoExt::GetFLHAbsoluteCoords(
			this->Parent, this->Data->FLH, this->Data->IsOnTurret));

		this->Child->OnBridge = this->Parent->OnBridge;

		DirStruct childDir = this->Data->IsOnTurret
			? this->Parent->SecondaryFacing.current() : this->Parent->PrimaryFacing.current();

		this->Child->PrimaryFacing.set(childDir);

		if (pType->InheritTilt)
		{
			this->Child->AngleRotatedForwards = this->Parent->AngleRotatedForwards;
			this->Child->AngleRotatedSideways = this->Parent->AngleRotatedSideways;

			// DriveLocomotionClass doesn't tilt only with angles set, hence why we
			// do this monstrosity in order to inherit timer and ramp data - Kerbiter
			FootClass* pParentAsFoot = abstract_cast<FootClass*>(this->Parent);
			FootClass* pChildAsFoot = abstract_cast<FootClass*>(this->Child);
			if (pParentAsFoot && pChildAsFoot)
			{
				auto pParentLoco = static_cast<LocomotionClass*>(pParentAsFoot->Locomotor.get());
				auto pChildLoco = static_cast<LocomotionClass*>(pChildAsFoot->Locomotor.get());

				CLSID locoCLSID;
				if (SUCCEEDED(pParentLoco->GetClassID(&locoCLSID)) && locoCLSID == LocomotionClass::CLSIDs::Drive &&
					SUCCEEDED(pChildLoco->GetClassID(&locoCLSID)) && locoCLSID == LocomotionClass::CLSIDs::Drive)
				{
					auto pParentDriveLoco = static_cast<DriveLocomotionClass*>(pParentLoco);
					auto pChildDriveLoco = static_cast<DriveLocomotionClass*>(pChildLoco);

					pChildDriveLoco->SlopeTimer = pParentDriveLoco->SlopeTimer;
					pChildDriveLoco->Ramp1 = pParentDriveLoco->Ramp1;
					pChildDriveLoco->Ramp2 = pParentDriveLoco->Ramp2;
				}
			}
		}

		if (pType->InheritStateEffects)
		{
			this->Child->CloakState = this->Parent->CloakState;
			this->Child->BeingWarpedOut = this->Parent->BeingWarpedOut;
			this->Child->Deactivated = this->Parent->Deactivated;
			this->Child->Flash(this->Parent->Flashing.DurationRemaining);

			this->Child->IronCurtainTimer = this->Parent->IronCurtainTimer;
			this->Child->IdleActionTimer = this->Parent->IdleActionTimer;
			this->Child->IronTintTimer = this->Parent->IronTintTimer;
			this->Child->CloakDelayTimer = this->Parent->CloakDelayTimer;
			this->Child->ChronoLockRemaining = this->Parent->ChronoLockRemaining;
			this->Child->Berzerk = this->Parent->Berzerk;
			this->Child->ChronoWarpedByHouse = this->Parent->ChronoWarpedByHouse;
			this->Child->EMPLockRemaining = this->Parent->EMPLockRemaining;
			this->Child->ShouldLoseTargetNow = this->Parent->ShouldLoseTargetNow;
		}

		if (pType->InheritOwner)
			this->Child->SetOwningHouse(this->Parent->GetOwningHouse(), false);
	}
}

// Doesn't call destructor (to be managed by smart pointers)
void AttachmentClass::Uninitialize()
{
	if (this->Child)
	{
		auto pType = this->GetType();
		if (pType->DestructionWeapon_Child != nullptr)
			TechnoExt::FireWeaponAtSelf(this->Child, pType->DestructionWeapon_Child);

		if (!this->Child->InLimbo && pType->ParentDestructionMission.isset())
			this->Child->QueueMission(pType->ParentDestructionMission.Get(), false);
	}
}

void AttachmentClass::ChildDestroyed()
{
	AttachmentTypeClass* pType = this->GetType();
	if (pType->DestructionWeapon_Parent != nullptr)
		TechnoExt::FireWeaponAtSelf(this->Parent, pType->DestructionWeapon_Parent);

	//this->Child = nullptr;
}

void AttachmentClass::Unlimbo()
{
	if (this->Child)
	{
		CoordStruct childCoord = TechnoExt::GetFLHAbsoluteCoords(
			this->Parent, this->Data->FLH, this->Data->IsOnTurret);

		Direction::Value childDir = this->Data->IsOnTurret
			? this->Parent->SecondaryFacing.current().value256() : this->Parent->PrimaryFacing.current().value256();

		++Unsorted::IKnowWhatImDoing;
		this->Child->Unlimbo(childCoord, childDir);
		--Unsorted::IKnowWhatImDoing;
	}
}

void AttachmentClass::Limbo()
{
	if (this->Child)
		this->Child->Limbo();
}

bool AttachmentClass::AttachChild(TechnoClass* pChild)
{
	if (this->Child)
		return false;

	this->Child = pChild;

	auto pChildExt = TechnoExt::ExtMap.Find(this->Child);
	pChildExt->ParentAttachment = this;

	AttachmentTypeClass* pType = this->GetType();

	if (pType->InheritOwner)
	{
		if (auto pController = this->Child->MindControlledBy)
			pController->CaptureManager->FreeUnit(this->Child);
	}

	return true;
}

bool AttachmentClass::DetachChild(bool isForceDetachment)
{
	if (this->Child)
	{
		AttachmentTypeClass* pType = this->GetType();

		if (isForceDetachment)
		{
			if (pType->ForceDetachWeapon_Parent.isset())
				TechnoExt::FireWeaponAtSelf(this->Parent, pType->DestructionWeapon_Parent);

			if (pType->ForceDetachWeapon_Child.isset())
				TechnoExt::FireWeaponAtSelf(this->Child, pType->DestructionWeapon_Child);
		}

		if (!this->Child->InLimbo && pType->ParentDetachmentMission.isset())
			this->Child->QueueMission(pType->ParentDetachmentMission.Get(), false);

		if (pType->InheritOwner)
			this->Child->SetOwningHouse(this->Parent->GetOriginalOwner(), false);

		auto pChildExt = TechnoExt::ExtMap.Find(this->Child);
		pChildExt->ParentAttachment = nullptr;
		this->Child = nullptr;

		return true;
	}

	return false;
}

#pragma region Save/Load

template <typename T>
bool AttachmentClass::Serialize(T& stm)
{
	return stm
		.Process(this->Data)
		.Process(this->Parent)
		.Process(this->Child)
		.Success();
};

bool AttachmentClass::Load(PhobosStreamReader& stm, bool RegisterForChange)
{
	return Serialize(stm);
}

bool AttachmentClass::Save(PhobosStreamWriter& stm) const
{
	return const_cast<AttachmentClass*>(this)->Serialize(stm);
}

bool AttachmentClass::LoadGlobals(PhobosStreamReader& stm)
{
	stm.Process(Array);
	return stm.Success();
}

bool AttachmentClass::SaveGlobals(PhobosStreamWriter& stm)
{
	stm.Process(Array);
	return stm.Success();
}

#pragma endregion 