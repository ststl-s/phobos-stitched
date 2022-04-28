#include "AttachmentClass.h"

#include <BulletClass.h>
#include <BulletTypeClass.h>
#include <WarheadTypeClass.h>
#include <DriveLocomotionClass.h>

#include <ObjBase.h>

#include <Ext/Techno/Body.h>
#include <Utilities/PointerMapper.h>

std::vector<std::unique_ptr<AttachmentClass>> AttachmentClass::Array;

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
		if (this->Child = static_cast<TechnoClass*>(pChildType->CreateObject(this->Parent->Owner)))
		{
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
	Debug::Log("[Attachment] Attachment Data[0x%X]\n", this->Data);
	Debug::Log("[Attachment] AttachmentTypeIdx[%d]\n", this->Data->Type.Get());
	Debug::Log("TechnoType[0x%X]\n", this->Data->TechnoType[0]);
	AttachmentTypeClass* pType = this->GetType();
	Debug::Log("[Attachment] pType[0x%X]\n", pType);

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

		//auto pChildExt = TechnoExt::ExtMap.Find(this->Child);
		//pChildExt->ParentAttachment = nullptr;
		//this->Child = nullptr;
	}
	/*
	auto it = std::find_if(AttachmentClass::Array.begin(), AttachmentClass::Array.end(), [this](std::unique_ptr<AttachmentClass>& ptr)
			{
				return ptr.get() == this;
			});
	if (it != AttachmentClass::Array.end())
		AttachmentClass::Array.erase(it);
	*/
}

void AttachmentClass::ChildDestroyed()
{
	AttachmentTypeClass* pType = this->GetType();
	if (pType->DestructionWeapon_Parent != nullptr )
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
	stm
		.Process(this->Data)
		.Process(this->Parent)
		.Process(this->Child);
	Debug::Log("Data[0x%X],Parent[0x%X],Child[0x%X]\n", this->Data, this->Parent, this->Child);
	return stm.Success();
};

bool AttachmentClass::Load(PhobosStreamReader& stm, bool RegisterForChange)
{
	return Serialize(stm);
}

bool AttachmentClass::Save(PhobosStreamWriter& stm) const
{
	return const_cast<AttachmentClass*>(this)->Serialize(stm);
}

bool AttachmentClass::LoadGlobals(PhobosStreamReader& Stm)
{
	Debug::Log("[Attachment] Attachment Load Global\n");
	Array.clear();

	for (auto& it : ExistTechnoTypeExt::Array)
	{
		Debug::Log("[AttachmentData] Process TechnoTypeExt[0x%X]\n", it);
		for (auto& AttachmentData : it->AttachmentData)
		{
			Debug::Log("[AttachmentData] Process[0x%X]\n", &AttachmentData);
			Debug::Log("[AttachmentData] TechnoTypeOld[0x%X],new[0x%X]\n", AttachmentData.TechnoType[0], PointerMapper::Map[reinterpret_cast<long>(AttachmentData.TechnoType[0])]);
			AttachmentData.TechnoType[0] = reinterpret_cast<TechnoTypeClass*>(PointerMapper::Map[reinterpret_cast<long>(AttachmentData.TechnoType[0])]);
		}
	}

	ExistTechnoTypeExt::Array.clear();

	size_t Count = 0;
	if (!Stm.Load(Count))
		return false;

	std::map<TechnoClass*, AttachmentClass*> Exist_Parent;
	std::map<TechnoClass*, AttachmentClass*> Exist_Child;

	for (size_t i = 0; i < Count; i++)
	{
		AttachmentClass* oldPtr = nullptr;
		decltype(Name) name;

		Stm.Load(oldPtr);
		Stm.Load(name);

		std::unique_ptr<AttachmentClass> newPtr(nullptr);
		newPtr.reset(FindOrAllocate(name.data()));
		PhobosSwizzle::Instance.RegisterChange(oldPtr, newPtr.get());

		//newPtr->Load(Stm, true);
		TechnoTypeExt::ExtData::AttachmentDataEntry* pData = nullptr;
		TechnoClass* pParent = nullptr;
		TechnoClass* pChild = nullptr;

		Savegame::detail::Selector::ReadFromStream(Stm, pData, false);
		Savegame::detail::Selector::ReadFromStream(Stm, pParent, false);
		Savegame::detail::Selector::ReadFromStream(Stm, pChild, false);

		Debug::Log("[Attachment] Before Swizzle pData[0x%X],pParent[0x%X],pChild[0x%X]\n", pData, pParent, pChild);

		newPtr->Data = reinterpret_cast<TechnoTypeExt::ExtData::AttachmentDataEntry*>(PointerMapper::Map[reinterpret_cast<long>(pData)]);
		newPtr->Parent = reinterpret_cast<TechnoClass*>(PointerMapper::Map[reinterpret_cast<long>(pParent)]);
		newPtr->Child = reinterpret_cast<TechnoClass*>(PointerMapper::Map[reinterpret_cast<long>(pChild)]);

		//CoordStruct FLH = { 0,0,0 };
		//bool IsOnTurret = false;
		//int AttachmentTypeIdx = -1;
		//int TechnoTypeIdx = -1;

		//Stm.Load(FLH);
		//Stm.Load(IsOnTurret);
		//Stm.Load(AttachmentTypeIdx);
		//Stm.Load(TechnoTypeIdx);

		//newPtr->Data->FLH = FLH;
		//newPtr->Data->IsOnTurret = IsOnTurret;
		//newPtr->Data->Type = AttachmentTypeIdx;
		//newPtr->Data->TechnoType = TechnoTypeIdx;

		Debug::Log("[Attachment] Attachment Load: oldPtr[0x%X],newPtr[0x%X]\n", oldPtr, newPtr.get());
		Debug::Log("[Attachment] newPtr->Parent[0x%X],Child[0x%X],Data[0x%X],Name[%s]\n", newPtr->Parent, newPtr->Child, newPtr->Data, newPtr->Name.data());
		//Debug::Log("[Attachment] Data: TechnoType[0x%X],AttachmentTypeIdx[%d]\n", newPtr->Data->TechnoType[0], newPtr->Data->Type.Get());

		Exist_Parent[newPtr->Parent] = newPtr.get();
		Exist_Child[newPtr->Child] = newPtr.get();


		Array.push_back(std::move(newPtr));
	}
	for (auto it : ExistTechnoExt::Array)
	{
		{
			auto itTmp = Exist_Parent.find(it->OwnerObject());
			if (itTmp != Exist_Parent.end())
			{
				it->ChildAttachments.push_back(itTmp->second);
				Debug::Log("[Attachment] Set Parent Techno[0x%X]\n", it->OwnerObject());
			}
		}
		{
			auto itTmp = Exist_Child.find(it->OwnerObject());
			if (itTmp != Exist_Child.end())
			{
				Debug::Log("[Attachment] Set Child Techno[0x%X]\n", it->OwnerObject());
				it->ParentAttachment = itTmp->second;
			}
		}
	}
	ExistTechnoExt::Array.clear();
	PointerMapper::Map.clear();
	Debug::Log("[Attachment] Attachment Load Global Finish: Loaded %u Items\n", Count);
	return true;
}

bool AttachmentClass::SaveGlobals(PhobosStreamWriter& Stm)
{
	Debug::Log("[Attachment] Attachment Save Global\n");
	Stm.Save(Array.size());
	Debug::Log("[Attachment] Attachment Array Size[%u]\n", Array.size());

	for (const auto& item : Array)
	{
		Stm.Save(item.get());
		Stm.Save(item->Name);
		Stm.Save(item->Data);
		Stm.Save(item->Parent);
		Stm.Save(item->Child);
		//Stm.Save(item->Data->FLH.Get());
		//Stm.Save(item->Data->IsOnTurret.Get());
		//Stm.Save(item->Data->Type.Get());
		//Stm.Save(item->Data->TechnoType.Get());
		Debug::Log("[Attachment] Saved oldPtr[0x%X]\n", item.get());
		Debug::Log("[Attachment] item->Parent[0x%X],Child[0x%X],Data[0x%X],Name[%s]\n", item->Parent, item->Child, item->Data, item->Name.data());
	}
	Debug::Log("[Attachment] Attachment Save Global Finish\n");
	return true;
}

AttachmentClass* AttachmentClass::FindOrAllocate(const char* Name)
{
	//if (AttachmentClass* find = Find(Name))
	//	return find;
	//always allocate

	Array.push_back(std::make_unique<AttachmentClass>(Name));

	return Array.back().get();
}

#pragma endregion 