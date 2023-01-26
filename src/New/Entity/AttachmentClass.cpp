#include "AttachmentClass.h"

#include <Dir.h>
#include <BulletClass.h>
#include <BulletTypeClass.h>
#include <WarheadTypeClass.h>
#include <TunnelLocomotionClass.h>

#include <ObjBase.h>

#include <Ext/Techno/Body.h>

std::vector<AttachmentClass*> AttachmentClass::Array;

void AttachmentClass::InitCacheData()
{
	this->Cache.TopLevelParent = TechnoExt::GetTopLevelParent(this->Parent);
}

Matrix3D AttachmentClass::GetUpdatedTransform(VoxelIndexKey* pKey, bool isShadow)
{
	Matrix3D& transform = isShadow ? this->Cache.ChildShadowTransform : this->Cache.ChildTransform;
	int& lastUpdateFrame = isShadow ? this->Cache.ShadowLastUpdateFrame : this->Cache.LastUpdateFrame;

	if (Unsorted::CurrentFrame != lastUpdateFrame)
	{
		double& factor = *reinterpret_cast<double*>(0xB1D008);
		auto const flh = this->Data->FLH * factor;

		Matrix3D mtx = TechnoExt::GetAttachmentTransform(this->Parent, pKey, isShadow);
		mtx = TechnoExt::TransformFLHForTurret(this->Parent, mtx, this->Data->IsOnTurret, factor);
		mtx.Translate((float)flh.X, (float)flh.Y, (float)flh.Z);

		transform = mtx;

		lastUpdateFrame = Unsorted::CurrentFrame;
	}

	return transform;
}

AttachmentTypeClass* AttachmentClass::GetType()
{
	return this->Data->Type;
}

TechnoTypeClass* AttachmentClass::GetChildType()
{
	return this->Data->TechnoType;
}

Matrix3D AttachmentClass::GetChildTransformForLocation()
{
	auto const flh = this->Data->FLH;

	auto const pParentExt = TechnoExt::ExtMap.Find(this->Parent);

	Matrix3D mtx;
	if (pParentExt && pParentExt->ParentAttachment)
		mtx = pParentExt->ParentAttachment->GetChildTransformForLocation();
	else
		mtx = TechnoExt::GetTransform(this->Parent);

	mtx = TechnoExt::TransformFLHForTurret(this->Parent, mtx, this->Data->IsOnTurret);
	mtx.Translate((float)flh.X, (float)flh.Y, (float)flh.Z);

	return mtx;
}

CoordStruct AttachmentClass::GetChildLocation()
{
	auto& flh = this->Data->FLH;
	return TechnoExt::GetFLHAbsoluteCoords(this->Parent, flh, this->Data->IsOnTurret);

	/*
	// TODO it doesn't work correctly for some unexplicable reason
	auto result = this->GetChildTransformForLocation() * Vector3D<float>::Empty;
	// Resulting coords are mirrored along X axis, so we mirror it back
	result.Y *= -1;
	// apply as an offset to global object coords
	CoordStruct location = this->Cache.TopLevelParent->GetCoords();
	location += { std::lround(result.X), std::lround(result.Y), std::lround(result.Z) };
	return location;
	*/
}

AttachmentClass::~AttachmentClass()
{
	// clean up non-owning references
	if (this->Child)
	{
		auto const& pChildExt = TechnoExt::ExtMap.Find(Child);
		pChildExt->ParentAttachment = nullptr;
	}

	auto position = std::find(Array.begin(), Array.end(), this);
	if (position != Array.end())
		Array.erase(position);
}

void AttachmentClass::Initialize()
{
	if (this->Child)
		return;

	this->RestoreCount = this->GetType()->RestoreDelay;
	this->OriginFLH = this->Data->FLH;

	if (this->GetType()->RestoreAtCreation)
		this->CreateChild();
}

void AttachmentClass::CreateChild()
{
	if (auto const pChildType = this->GetChildType())
	{
		//if (pChildType->WhatAmI() != AbstractType::UnitType)
			//return;

		if (const auto pTechno = static_cast<TechnoClass*>(pChildType->CreateObject(this->Parent->Owner)))
		{
			this->SetFLHoffset();
			this->AttachChild(pTechno);

			/*
			bool selected = this->Parent->IsSelected;
			CellClass* pCell = MapClass::Instance->TryGetCellAt(CellClass::Coord2Cell(this->Parent->Location));

			CoordStruct crdDest;

			if (pCell != nullptr)
			{
				crdDest = pCell->GetCoordsWithBridge();
			}
			else
			{
				crdDest = this->Parent->Location;
				crdDest.Z = MapClass::Instance->GetCellFloorHeight(crdDest);
			}

			crdDest.Z += this->Parent->GetHeight();

			this->Parent->Limbo();
			++Unsorted::IKnowWhatImDoing;
			this->Parent->Unlimbo(crdDest, this->Parent->GetRealFacing().GetDir());
			--Unsorted::IKnowWhatImDoing;
			*/

			this->Limbo();
			this->Unlimbo();
			this->Child->UpdatePlacement(PlacementType::Redraw);

			/*
			if (selected)
			{
				this->Parent->Select();
			}
			*/
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
	if (this == nullptr || this->GetType() == nullptr || this->Data == nullptr)
		return;

	AttachmentTypeClass* pType = this->GetType();

	if (this->Child)
	{
		this->Child->SetLocation(this->GetChildLocation());
		if (this->Child->WhatAmI() == AbstractType::Building)
		{
			this->Child->UpdatePlacement(PlacementType::Redraw);
		}

		this->Child->OnBridge = this->Parent->OnBridge;

		DirStruct childDir = this->Data->IsOnTurret
			? this->Parent->SecondaryFacing.Current() : this->Parent->PrimaryFacing.Current();

		this->Child->PrimaryFacing.SetCurrent(childDir);
		// TODO handle secondary facing in case the turret is idle
		
		FootClass* pParentAsFoot = abstract_cast<FootClass*>(this->Parent);
		FootClass* pChildAsFoot = abstract_cast<FootClass*>(this->Child);
		if (pParentAsFoot && pChildAsFoot)
		{
			pChildAsFoot->TubeIndex = pParentAsFoot->TubeIndex;

			auto pParentLoco = static_cast<LocomotionClass*>(pParentAsFoot->Locomotor.get());
			auto pChildLoco = static_cast<LocomotionClass*>(pChildAsFoot->Locomotor.get());

			CLSID locoCLSID;
			if (SUCCEEDED(pParentLoco->GetClassID(&locoCLSID))
				&& (locoCLSID == LocomotionClass::CLSIDs::Tunnel) &&
				SUCCEEDED(pChildLoco->GetClassID(&locoCLSID))
				&& (locoCLSID == LocomotionClass::CLSIDs::Tunnel))
			{
				auto pParentTunnelLoco = static_cast<TunnelLocomotionClass*>(pParentLoco);
				auto pChildTunnelLoco = static_cast<TunnelLocomotionClass*>(pChildLoco);

				// FIXME I am not sure if fucking with RefCount is a good idea but it's used in TunnelLoco code
				pChildTunnelLoco->RefCount = pParentTunnelLoco->RefCount;

				pChildTunnelLoco->State = pParentTunnelLoco->State;
				pChildTunnelLoco->Coords = pParentTunnelLoco->Coords;
				pChildTunnelLoco->DigTimer = pParentTunnelLoco->DigTimer;
				pChildTunnelLoco->bool38 = pParentTunnelLoco->bool38;
			}
		}

		auto pChildExt = TechnoExt::ExtMap.Find(this->Child);
		auto pParentExt = TechnoExt::ExtMap.Find(this->Parent);

		//StateEffects
		if (pType->InheritStateEffects && pType->InheritStateEffects_Parent)
		{
			this->Child->CloakState > this->Parent->CloakState ?
				this->Child->CloakState = this->Parent->CloakState :
				this->Parent->CloakState = this->Child->CloakState;

			if (this->Child->BeingWarpedOut || this->Parent->BeingWarpedOut)
			{
				this->Child->BeingWarpedOut = true;
				this->Parent->BeingWarpedOut = true;
			}

			if (this->Child->Deactivated || this->Parent->Deactivated)
			{
				this->Child->Deactivated = true;
				this->Parent->Deactivated = true;
			}

			this->Child->Flashing.DurationRemaining > this->Parent->Flashing.DurationRemaining ?
				this->Parent->Flash(this->Child->Flashing.DurationRemaining) :
				this->Child->Flash(this->Parent->Flashing.DurationRemaining);

			this->Child->IronCurtainTimer.GetTimeLeft() > this->Parent->IronCurtainTimer.GetTimeLeft() ?
				this->Parent->IronCurtainTimer.TimeLeft = this->Child->IronCurtainTimer.GetTimeLeft() :
				this->Child->IronCurtainTimer.TimeLeft = this->Parent->IronCurtainTimer.GetTimeLeft();

			this->Child->IdleActionTimer.GetTimeLeft() > this->Parent->IdleActionTimer.GetTimeLeft() ?
				this->Parent->IdleActionTimer.TimeLeft = this->Child->IdleActionTimer.GetTimeLeft() :
				this->Child->IdleActionTimer.TimeLeft = this->Parent->IdleActionTimer.GetTimeLeft();

			this->Child->IronTintTimer.GetTimeLeft() > this->Parent->IronTintTimer.GetTimeLeft() ?
				this->Parent->IronTintTimer.TimeLeft = this->Child->IronTintTimer.GetTimeLeft() :
				this->Child->IronTintTimer.TimeLeft = this->Parent->IronTintTimer.GetTimeLeft();

			this->Child->CloakDelayTimer.GetTimeLeft() > this->Parent->CloakDelayTimer.GetTimeLeft() ?
				this->Parent->CloakDelayTimer.TimeLeft = this->Child->CloakDelayTimer.GetTimeLeft() :
				this->Child->CloakDelayTimer.TimeLeft = this->Parent->CloakDelayTimer.GetTimeLeft();

			this->Child->ChronoLockRemaining > this->Parent->ChronoLockRemaining ?
				this->Parent->ChronoLockRemaining = this->Child->ChronoLockRemaining :
				this->Child->ChronoLockRemaining = this->Parent->ChronoLockRemaining;

			if (this->Child->Berzerk || this->Parent->Berzerk)
			{
				this->Child->Berzerk = true;
				this->Parent->Berzerk = true;
			}

			if (this->Child->ChronoWarpedByHouse || this->Parent->ChronoWarpedByHouse)
			{
				if (this->Parent->ChronoWarpedByHouse)
					this->Child->ChronoWarpedByHouse = this->Parent->ChronoWarpedByHouse;
				else
					this->Parent->ChronoWarpedByHouse = this->Child->ChronoWarpedByHouse;
			}

			this->Child->EMPLockRemaining > this->Parent->EMPLockRemaining ?
				this->Parent->EMPLockRemaining = this->Child->EMPLockRemaining :
				this->Child->EMPLockRemaining = this->Parent->EMPLockRemaining;

			this->Child->ShouldLoseTargetNow > this->Parent->ShouldLoseTargetNow ?
				this->Parent->ShouldLoseTargetNow = this->Child->ShouldLoseTargetNow :
				this->Child->ShouldLoseTargetNow = this->Parent->ShouldLoseTargetNow;
		}
		else if (pType->InheritStateEffects)
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
		else if (pType->InheritStateEffects_Parent)
		{
			this->Parent->CloakState = this->Child->CloakState;
			this->Parent->BeingWarpedOut = this->Child->BeingWarpedOut;
			this->Parent->Deactivated = this->Child->Deactivated;
			this->Parent->Flash(this->Child->Flashing.DurationRemaining);

			this->Parent->IronCurtainTimer = this->Child->IronCurtainTimer;
			this->Parent->IdleActionTimer = this->Child->IdleActionTimer;
			this->Parent->IronTintTimer = this->Child->IronTintTimer;
			this->Parent->CloakDelayTimer = this->Child->CloakDelayTimer;
			this->Parent->ChronoLockRemaining = this->Child->ChronoLockRemaining;
			this->Parent->Berzerk = this->Child->Berzerk;
			this->Parent->ChronoWarpedByHouse = this->Child->ChronoWarpedByHouse;
			this->Parent->EMPLockRemaining = this->Child->EMPLockRemaining;
			this->Parent->ShouldLoseTargetNow = this->Child->ShouldLoseTargetNow;
		}

		//Target
		if (this->Child->Target != this->Parent->Target)
		{
			if (pType->InheritTarget && pType->InheritTarget_Parent)
			{
				if (pParentExt->LastTarget != this->Parent->Target)
				{
					this->Child->SetTarget(this->Parent->Target);
					pParentExt->LastTarget = this->Parent->Target;
					pChildExt->LastTarget = this->Child->Target;
				}
				else if (pChildExt->LastTarget != this->Child->Target)
				{
					this->Parent->SetTarget(this->Child->Target);
					pParentExt->LastTarget = this->Parent->Target;
					pChildExt->LastTarget = this->Child->Target;
				}
			}
			else if (pType->InheritTarget)
			{
				this->Child->SetTarget(this->Parent->Target);
			}
			else if (pType->InheritTarget_Parent)
			{
				this->Parent->SetTarget(this->Child->Target);
			}
		}

		//Ammo
		if (this->Parent->GetTechnoType()->Ammo > 0 && this->Child->GetTechnoType()->Ammo > 0)
		{
			if (this->Child->Ammo != this->Parent->Ammo)
			{
				if (pType->InheritAmmo && pType->InheritAmmo_Parent)
				{
					if (this->Child->Ammo > this->Parent->Ammo)
					{
						if (this->Parent->Ammo <= this->Child->GetTechnoType()->Ammo)
						{
							this->Child->Ammo = this->Parent->Ammo;
						}
						else
						{
							this->Child->Ammo = this->Child->GetTechnoType()->Ammo;
						}
					}
					else
					{
						if (this->Child->Ammo <= this->Parent->GetTechnoType()->Ammo)
						{
							this->Parent->Ammo = this->Child->Ammo;
						}
						else
						{
							this->Parent->Ammo = this->Child->GetTechnoType()->Ammo;
						}
					}
				}
				else if (pType->InheritAmmo)
				{
					if (this->Parent->Ammo > this->Child->GetTechnoType()->Ammo)
					{
						this->Child->Ammo = this->Child->GetTechnoType()->Ammo;
					}
					else
					{
						this->Child->Ammo = this->Parent->Ammo;
					}
				}
				else if (pType->InheritAmmo_Parent)
				{
					if (this->Child->Ammo > this->Parent->GetTechnoType()->Ammo)
					{
						this->Parent->Ammo = this->Parent->GetTechnoType()->Ammo;
					}
					else
					{
						this->Parent->Ammo = this->Child->Ammo;
					}
				}
			}
		}

		//Health
		if (this->Child->GetHealthPercentage() != this->Parent->GetHealthPercentage())
		{
			if (pType->InheritHealth && pType->InheritHealth_Parent)
			{
				this->Child->GetHealthPercentage() > this->Parent->GetHealthPercentage() ?
					this->Child->SetHealthPercentage(this->Parent->GetHealthPercentage()) :
					this->Parent->SetHealthPercentage(this->Child->GetHealthPercentage());
			}
			else if (pType->InheritHealth)
			{
				this->Child->SetHealthPercentage(this->Parent->GetHealthPercentage());
			}
			else if (pType->InheritHealth_Parent)
			{
				this->Parent->SetHealthPercentage(this->Child->GetHealthPercentage());
			}
		}

		//Veterancy
		if (this->Parent->GetTechnoType()->Trainable && this->Child->GetTechnoType()->Trainable)
		{
			if (this->Child->Veterancy.GetRemainingLevel() != this->Parent->Veterancy.GetRemainingLevel())
			{
				if (pType->InheritVeterancy && pType->InheritVeterancy_Parent)
				{
					int childrank = 0;
					int parentrank = 0;

					switch (this->Child->Veterancy.GetRemainingLevel())
					{
					case Rank::Rookie:
						childrank = 1;
						break;
					case Rank::Veteran:
						childrank = 2;
						break;
					case Rank::Elite:
						childrank = 3;
						break;
					default:
						break;
					}

					switch (this->Parent->Veterancy.GetRemainingLevel())
					{
					case Rank::Rookie:
						parentrank = 1;
						break;
					case Rank::Veteran:
						parentrank = 2;
						break;
					case Rank::Elite:
						parentrank = 3;
						break;
					default:
						break;
					}

					if (parentrank > childrank)
					{
						switch (parentrank)
						{
						case 1:
							this->Child->Veterancy.SetRookie();
							break;
						case 2:
							this->Child->Veterancy.SetVeteran();
							break;
						case 3:
							this->Child->Veterancy.SetElite();
							break;
						default:
							break;
						}
					}
					else
					{
						switch (childrank)
						{
						case 1:
							this->Parent->Veterancy.SetRookie();
							break;
						case 2:
							this->Parent->Veterancy.SetVeteran();
							break;
						case 3:
							this->Parent->Veterancy.SetElite();
							break;
						default:
							break;
						}
					}
				}
				else if (pType->InheritVeterancy)
				{
					switch (this->Parent->Veterancy.GetRemainingLevel())
					{
					case Rank::Rookie:
						this->Child->Veterancy.SetRookie();
						break;
					case Rank::Veteran:
						this->Child->Veterancy.SetVeteran();
						break;
					case Rank::Elite:
						this->Child->Veterancy.SetElite();
						break;
					default:
						break;
					}
				}
				else if (pType->InheritVeterancy_Parent)
				{
					switch (this->Child->Veterancy.GetRemainingLevel())
					{
					case Rank::Rookie:
						this->Parent->Veterancy.SetRookie();
						break;
					case Rank::Veteran:
						this->Parent->Veterancy.SetVeteran();
						break;
					case Rank::Elite:
						this->Parent->Veterancy.SetElite();
						break;
					default:
						break;
					}
				}
			}
		}

		//Owner
		if (pType->InheritOwner && pType->InheritOwner_Parent)
		{
			if (pParentExt->LastOwner != this->Parent->Owner)
			{
				this->Child->SetOwningHouse(this->Parent->GetOwningHouse(), false);
				pParentExt->LastOwner = this->Parent->GetOwningHouse();
				pChildExt->LastOwner = this->Child->GetOwningHouse();
			}
			else if (pChildExt->LastOwner != this->Parent->Owner)
			{
				this->Parent->SetOwningHouse(this->Child->GetOwningHouse(), false);
				pParentExt->LastOwner = this->Parent->GetOwningHouse();
				pChildExt->LastOwner = this->Child->GetOwningHouse();
			}
		}
		else if (pType->InheritOwner)
			this->Child->SetOwningHouse(this->Parent->GetOwningHouse(), false);
		else if (pType->InheritOwner_Parent)
			this->Parent->SetOwningHouse(this->Child->GetOwningHouse(), false);

		if (pType->MoveSelectToParent && this->Child->IsSelected)
		{
			this->Child->Deselect();
			this->Parent->Select();
		}
	}
	else
	{
		if (this->GetType()->RestoreDelay >= 0)
		{
			TechnoExt::AttachmentsRestore(this->Parent);
		}
	}
}

void AttachmentClass::Destroy(TechnoClass* pSource)
{
	if (this->Child)
	{
		if (this->Child->WhatAmI() != AbstractType::Building)
			TechnoExt::ChangeLocomotorTo(this->Child, this->Child->GetTechnoType()->Locomotor);

		auto pChildExt = TechnoExt::ExtMap.Find(this->Child);
		pChildExt->ParentAttachment = nullptr;

		auto pType = this->GetType();

		//if (pType->DestructionWeapon_Child.isset())
			//TechnoExt::FireWeaponAtSelf(this->Parent, pType->DestructionWeapon_Child);
		if (pType->DestructionWeapon_Parent.isset() && TechnoExt::IsReallyAlive(this->Child))
			TechnoExt::FireWeaponAtSelf(this->Child, pType->DestructionWeapon_Parent);


		if (pType->InheritDestruction)
		{
			if (pType->InheritDestruction_Type == AutoDeathBehavior::Kill)
			{
				this->Child->TakeDamage(this->Child->Health, pSource ? pSource->Owner : nullptr, pSource);
			}
			else
			{
				this->Child->KillPassengers(pSource);
				this->Child->RegisterDestruction(pSource);
				this->Child->UnInit();
			}
		}

		if (TechnoExt::IsReallyAlive(this->Child) && !this->Child->InLimbo && pType->ChildDestructionMission.isset())
			this->Child->QueueMission(pType->ChildDestructionMission.Get(), false);

		this->Child = nullptr;
	}
}

void AttachmentClass::DestroyParent(TechnoClass* pSource)
{
	if (this->Parent)
	{
		auto pParentExt = TechnoExt::ExtMap.Find(this->Parent);
		for (auto const& pAttachment : pParentExt->ChildAttachments)
		{
			auto pChildExt = TechnoExt::ExtMap.Find(pAttachment->Child);
			pChildExt->ParentAttachment = nullptr;
		}

		auto pType = this->GetType();

		if (pType->InheritDestruction_Parent)
		{
			if (pType->InheritDestruction_Parent_Type == AutoDeathBehavior::Kill)
			{
				this->Parent->TakeDamage(this->Parent->Health, pSource ? pSource->Owner : nullptr, pSource);
			}
			else
			{
				this->Parent->KillPassengers(pSource);
				this->Parent->RegisterDestruction(pSource);
				this->Parent->UnInit();
			}
		}

		this->Parent = nullptr;
	}
}

void AttachmentClass::ChildDestroyed()
{
	AttachmentTypeClass* pType = this->GetType();

	// if (pType->DestructionWeapon_Parent.isset())
		// TechnoExt::FireWeaponAtSelf(this->Child, pType->DestructionWeapon_Parent);
	if (pType->DestructionWeapon_Child.isset() && TechnoExt::IsReallyAlive(this->Parent))
		TechnoExt::FireWeaponAtSelf(this->Parent, pType->DestructionWeapon_Child);

	if (TechnoExt::IsReallyAlive(this->Parent) && !this->Parent->InLimbo && pType->ParentDestructionMission.isset())
		this->Parent->QueueMission(pType->ParentDestructionMission.Get(), false);

	this->Child = nullptr;
}

void AttachmentClass::Unlimbo()
{
	if (this->Child)
	{
		CoordStruct childCoord = TechnoExt::GetFLHAbsoluteCoords(
			this->Parent, this->Data->FLH, this->Data->IsOnTurret);

		DirType childDir = this->Data->IsOnTurret
			? this->Parent->SecondaryFacing.Current().GetDir() : this->Parent->PrimaryFacing.Current().GetDir();

		CellClass* pCell = MapClass::Instance->TryGetCellAt(CellClass::Coord2Cell(childCoord));

		CoordStruct crdDest;

		if (pCell != nullptr)
		{
			crdDest = pCell->GetCoordsWithBridge();
		}
		else
		{
			crdDest = childCoord;
			crdDest.Z = MapClass::Instance->GetCellFloorHeight(crdDest);
		}

		crdDest.Z += this->Parent->GetHeight() + this->Data->FLH.Z;

		++Unsorted::IKnowWhatImDoing;
		this->Child->Unlimbo(crdDest, childDir);
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

	// TODO fix properly
	this->Child->GetTechnoType()->DisableVoxelCache = true;
	this->Child->GetTechnoType()->DisableShadowCache = true;
	this->Child->GetTechnoType()->VoxelMainCache.Clear();
	this->Child->GetTechnoType()->VoxelShadowCache.Clear();
	this->Child->GetTechnoType()->VoxelTurretBarrelCache.Clear();
	this->Child->GetTechnoType()->VoxelTurretWeaponCache.Clear();

	AttachmentTypeClass* pType = this->GetType();

	if (!(pType->InheritOwner && pType->InheritOwner_Parent))
	{
		if (pType->InheritOwner)
		{
			if (auto pController = this->Child->MindControlledBy)
				pController->CaptureManager->Free(this->Child);
		}
		if (pType->InheritOwner_Parent)
		{
			if (auto pController = this->Parent->MindControlledBy)
				pController->CaptureManager->Free(this->Parent);
		}
	}

	return true;
}

bool AttachmentClass::DetachChild(bool isForceDetachment)
{
	if (this->Child)
	{
		if (this->Child->WhatAmI() != AbstractType::Building)
			TechnoExt::ChangeLocomotorTo(this->Child, this->Child->GetTechnoType()->Locomotor);

		AttachmentTypeClass* pType = this->GetType();

		if (isForceDetachment)
		{
			if (pType->ForceDetachWeapon_Parent.isset() && TechnoExt::IsReallyAlive(this->Parent))
				TechnoExt::FireWeaponAtSelf(this->Parent, pType->ForceDetachWeapon_Parent);

			if (pType->ForceDetachWeapon_Child.isset() && TechnoExt::IsReallyAlive(this->Child))
				TechnoExt::FireWeaponAtSelf(this->Child, pType->ForceDetachWeapon_Child);
		}

		if (!this->Parent->InLimbo && pType->ParentDetachmentMission.isset())
			this->Parent->QueueMission(pType->ParentDetachmentMission.Get(), false);

		// if (!this->Child->InLimbo && pType->ParentDetachmentMission.isset())
			// this->Child->QueueMission(pType->ParentDetachmentMission.Get(), false);
		if (!this->Child->InLimbo && pType->ChildDetachmentMission.isset())
			this->Child->QueueMission(pType->ChildDetachmentMission.Get(), false);

		// FIXME this won't work probably
		if (!(pType->InheritOwner && pType->InheritOwner_Parent))
		{
			if (pType->InheritOwner)
				this->Child->SetOwningHouse(this->Parent->GetOriginalOwner(), false);

			if (pType->InheritOwner_Parent)
				this->Parent->SetOwningHouse(this->Child->GetOriginalOwner(), false);
		}

		auto pChildExt = TechnoExt::ExtMap.Find(this->Child);
		pChildExt->ParentAttachment = nullptr;
		this->Child = nullptr;

		return true;
	}

	return false;
}


void AttachmentClass::InvalidatePointer(void* ptr)
{
	AnnounceInvalidPointer(this->Parent, ptr);
	AnnounceInvalidPointer(this->Child, ptr);
	AnnounceInvalidPointer(this->Cache.TopLevelParent, ptr);
}

void AttachmentClass::SetFLHoffset()
{
	if (this->GetType()->FLHoffset.Get() != CoordStruct { 0, 0, 0 })
	{
		auto const pParentExt = TechnoExt::ExtMap.Find(this->Parent);
		this->Data->FLH = this->OriginFLH;
		bool finished = false;
		do
		{
			for (size_t i = 0; i < pParentExt->ChildAttachments.size(); i++)
			{
				if (pParentExt->ChildAttachments[i]->GetType() == this->GetType() &&
					pParentExt->ChildAttachments[i].get() != this &&
					TechnoExt::IsReallyAlive(pParentExt->ChildAttachments[i]->Child))
				{
					if (this->Data->FLH == pParentExt->ChildAttachments[i]->Data->FLH)
					{
						this->Data->FLH += this->GetType()->FLHoffset.Get();
						break;
					}
				}

				if (i == pParentExt->ChildAttachments.size() - 1)
					finished = true;
			}
		}
		while (!finished);
	}
}

#pragma region Save/Load

template <typename T>
bool AttachmentClass::Serialize(T& stm)
{
	return stm
		.Process(this->Data)
		.Process(this->Parent)
		.Process(this->Child)
		.Process(this->RestoreCount)
		.Process(this->OriginFLH)
		.Success();
}

bool AttachmentClass::Load(PhobosStreamReader& stm, bool RegisterForChange)
{
	return Serialize(stm);
}

bool AttachmentClass::Save(PhobosStreamWriter& stm) const
{
	return const_cast<AttachmentClass*>(this)->Serialize(stm);
}

#pragma endregion
