#include "Body.h"

#include <Utilities/Macro.h>

#include <Ext/TechnoType/Body.h>

DEFINE_HOOK(0x4DA86E, FootClass_AI_UpdateAttachedLocomotion, 0x0)
{
	GET(FootClass* const, pThis, ESI);
	auto const pExt = TechnoExt::ExtMap.Find(pThis);

	if (!pExt->ParentAttachment)
		pThis->Locomotor->Process();

	return 0x4DA87A;
}

DEFINE_HOOK(0x707CB3, TechnoClass_KillCargo_HandleAttachments, 0x6)
{
	GET(TechnoClass*, pThis, EBX);
	GET_STACK(TechnoClass*, pSource, STACK_OFFSET(0x4, 0x4));

	TechnoExt::DestroyAttachments(pThis, pSource);

	return 0;
}

DEFINE_HOOK(0x5F6609, ObjectClass_RemoveThis_TechnoClass, 0x9)
{
	GET(TechnoClass*, pThis, ESI);

	pThis->KillPassengers(nullptr);  // restored code
	TechnoExt::HandleDestructionAsChild(pThis);

	return 0x5F6612;
}

DEFINE_HOOK(0x73F528, UnitClass_CanEnterCell_SkipChildren, 0x0)
{
	enum { IgnoreOccupier = 0x73FC10, Continue = 0x73F530 };

	GET(UnitClass*, pThis, EBX);
	GET(ObjectClass*, pOccupier, ESI);

	if (pThis == pOccupier
		|| TechnoExt::IsChildOf(abstract_cast<TechnoClass*>(pOccupier), pThis))
		return IgnoreOccupier;

	return Continue;
}

DEFINE_HOOK(0x51C251, InfantryClass_CanEnterCell_SkipChildren, 0x0)
{
	enum { IgnoreOccupier = 0x51C70F, Continue = 0x51C259 };

	GET(InfantryClass*, pThis, EBP);
	GET(ObjectClass*, pOccupier, ESI);

	if (pThis == pOccupier
		|| TechnoExt::IsChildOf(abstract_cast<TechnoClass*>(pOccupier), pThis))
		return IgnoreOccupier;

	return Continue;
}

DEFINE_HOOK(0x6CC763, SuperClass_Place_ChronoWarp_SkipChildren, 0x6)
{
	enum { Skip = 0x6CCCCA, Continue = 0 };

	GET(FootClass* const, pFoot, ESI);
	auto const pExt = TechnoExt::ExtMap.Find(pFoot);

	return pExt->ParentAttachment ? Skip : Continue;
}

void ParentClickedWaypoint(TechnoClass* pThis, int idxPath, signed char idxWP)
{
	// Rewrite of the original code
	pThis->AssignPlanningPath(idxPath, idxWP);

	if ((pThis->AbstractFlags & AbstractFlags::Foot) == AbstractFlags::Foot)
		pThis->unknown_bool_430 = false;

	// Children handling
	if (auto const& pExt = TechnoExt::ExtMap.Find(pThis))
	{
		for (auto const& pAttachment : pExt->ChildAttachments)
		{
			if (pAttachment->Child && pAttachment->GetType()->InheritCommands)
				ParentClickedWaypoint(pAttachment->Child, idxPath, idxWP);
		}

		if (pExt->ParentAttachment && pExt->ParentAttachment->GetType()->InheritCommands_Parent)
			ParentClickedWaypoint(pExt->ParentAttachment->Parent, idxPath, idxWP);
	}
}

void ParentClickedAction(TechnoClass* pThis, ObjectClass* pTarget, CellStruct* pCell, CellStruct* pSecondCell)
{
	// Rewrite of the original code
	if (pTarget)
	{
		Action whatAction = pThis->MouseOverObject(pTarget, false);
		pThis->ObjectClickedAction(whatAction, pTarget, false);
	}
	else
	{
		Action whatAction = pThis->MouseOverCell(pCell, false, false);
		pThis->CellClickedAction(whatAction, pCell, pSecondCell, false);
	}

	Unsorted::MoveFeedback = false;

	// Children handling
	if (auto const& pExt = TechnoExt::ExtMap.Find(pThis))
	{
		for (auto const& pAttachment : pExt->ChildAttachments)
		{
			if (pAttachment->Child && pAttachment->GetType()->InheritCommands)
				ParentClickedAction(pAttachment->Child, pTarget, pCell, pSecondCell);
		}

		if (pExt->ParentAttachment && pExt->ParentAttachment->GetType()->InheritCommands_Parent)
			ParentClickedAction(pExt->ParentAttachment->Parent, pTarget, pCell, pSecondCell);
	}
}

DEFINE_HOOK(0x4AE7B3, DisplayClass_ActiveClickWith_Iterate, 0x0)
{
	GET_STACK(int, idxPath, STACK_OFFSET(0x18, -0x8));
	GET_STACK(unsigned char, idxWP, STACK_OFFSET(0x18, -0xC));

	for (auto const& pObject : ObjectClass::CurrentObjects.get())
	{
		if (auto pTechno = abstract_cast<TechnoClass*>(pObject))
			ParentClickedWaypoint(pTechno, idxPath, idxWP);
	}

	GET_STACK(ObjectClass* const, pTarget, STACK_OFFSET(0x18, +0x4));
	LEA_STACK(CellStruct* const, pCell, STACK_OFFSET(0x18, +0x8));
	GET_STACK(Action const, action, STACK_OFFSET(0x18, +0xC));

	CellStruct invalidCell { -1, -1 };
	CellStruct* pSecondCell = &invalidCell;

	if (action == Action::Move || action == Action::PatrolWaypoint || action == Action::NoMove)
		pSecondCell = pCell;

	for (auto const& pObject : ObjectClass::CurrentObjects.get())
	{
		if (auto pTechno = abstract_cast<TechnoClass*>(pObject))
			ParentClickedAction(pTechno, pTarget, pCell, pSecondCell);
	}

	Unsorted::MoveFeedback = true;

	return 0x4AE99B;
}

namespace TechnoAttachmentTemp
{
	bool stopPressed = false;
}

DEFINE_HOOK(0x730EA0, StopCommand_Context_Set, 0x5)
{
	TechnoAttachmentTemp::stopPressed = true;
	return 0;
}

namespace TechnoAttachmentTemp
{
	TechnoClass* pParent = nullptr;
}

DEFINE_HOOK(0x6FFE00, TechnoClass_ClickedEvent_Context_Set, 0x5)
{
	TechnoAttachmentTemp::pParent = R->ECX<TechnoClass*>();
	return 0;
}

DEFINE_HOOK_AGAIN(0x6FFEB1, TechnoClass_ClickedEvent_HandleChildren, 0x6)
DEFINE_HOOK(0x6FFE4F, TechnoClass_ClickedEvent_HandleChildren, 0x6)
{
	if (TechnoAttachmentTemp::stopPressed && TechnoAttachmentTemp::pParent)
	{
		if (auto const& pExt = TechnoExt::ExtMap.Find(TechnoAttachmentTemp::pParent))
		{
			for (auto const& pAttachment : pExt->ChildAttachments)
			{
				if (pAttachment->Child && pAttachment->GetType()->InheritCommands)
					pAttachment->Child->ClickedEvent(NetworkEvents::Idle);
			}

			if (pExt->ParentAttachment && pExt->ParentAttachment->GetType()->InheritCommands_Parent)
				pExt->ParentAttachment->Parent->ClickedEvent(NetworkEvents::Idle);
		}
	}

	return 0;
}

DEFINE_HOOK(0x730F1C, StopCommand_Context_Unset, 0x5)
{
	TechnoAttachmentTemp::stopPressed = false;
	return 0;
}

/*
namespace TechnoAttachmentTemp
{
	TechnoClass* pParent;
}
DEFINE_HOOK(0x6FFBE0, TechnoClass_PlayerAssignMission_Context_Set, 0x6)
{
	TechnoAttachmentTemp::pParent = R->ECX<TechnoClass*>();
	return 0;
}
DEFINE_HOOK_AGAIN(0x6FFDEB, TechnoClass_PlayerAssignMission_HandleChildren, 0x5)
DEFINE_HOOK(0x6FFCAE, TechnoClass_PlayerAssignMission_HandleChildren, 0x5)
{
	GET_STACK(Mission, mission, STACK_OFFSET(0x98, +0x4));
	switch (mission)
	{
	case Mission::Move:
	case Mission::AttackMove:
	case Mission::Enter:
	case Mission::QMove:
		return 0;
	}
	GET_STACK(ObjectClass* const, pTarget, STACK_OFFSET(0x98, +0x8));
	GET_STACK(CellClass* const, pTargetCell, STACK_OFFSET(0x98, +0xC));
	GET_STACK(CellClass* const, pCellNearTarget, STACK_OFFSET(0x98, +0x10));
	auto const& pExt = TechnoExt::ExtMap.Find(TechnoAttachmentTemp::pParent);
	bool oldFeedback = Unsorted::MoveFeedback;
	Unsorted::MoveFeedback = false;
	for (auto const& pAttachment : pExt->ChildAttachments)
	{
		// Recursive call, PlayerAssignMission == ClickedMission
		if (pAttachment->Child && pAttachment->GetType()->InheritCommands)
			pAttachment->Child->ClickedMission(mission, pTarget, pTargetCell, pCellNearTarget);
	}
	Unsorted::MoveFeedback = oldFeedback;
	// PlayerAssignMission returns bool which indicates whether the player is in planning mode.
	// This can't change when we handle children so we don't adjust the return value - Kerbiter
	return 0;
}
*/

// DEFINE_HOOK(0x6CCCCA, SuperClass_Place_ChronoWarp_HandleAttachment, 0x0)
// {
// 	enum { Loop = 0x6CC742, Break = 0x6CCCD5 };
//
// 	GET(FootClass*, pFoot, ESI)
//
// 	pFoot = abstract_cast<FootClass*>(pFoot->NextObject);
//
// 	return pFoot ? Loop : Break;
// }

// TODO
// 0x4DEAE0 IC for footclass
// 0x457C90 IC (forceshield) for buildings
// 0x6CCCCA Chrono Warp
// 0x4694BB Temporal warhead
// ...

DEFINE_HOOK(0x469672, BulletClass_Logics_Locomotor_CheckIfAttached, 0x6)
{
	enum { SkipInfliction = 0x469AA4, ContinueCheck = 0x0 };

	GET(FootClass*, pThis, EDI);
	auto const& pExt = TechnoExt::ExtMap.Find(pThis);

	return pExt->ParentAttachment
		? SkipInfliction
		: ContinueCheck;
}

DEFINE_HOOK(0x6FC3F4, TechnoClass_CanFire_HandleAttachmentLogics, 0x6)
{
	enum { ReturnFireErrorIllegal = 0x6FC86A, ContinueCheck = 0x0 };

	GET(TechnoClass*, pThis, ESI);
	GET(TechnoClass*, pTarget, EBP);
	GET(WeaponTypeClass*, pWeapon, EDI);

	//auto const& pExt = TechnoExt::ExtMap.Find(pThis);
	//auto const& pTargetExt = TechnoExt::ExtMap.Find(pTarget);

	bool illegalParentTargetWarhead = pWeapon->Warhead
		&& pWeapon->Warhead->IsLocomotor;

	if (illegalParentTargetWarhead
		&& TechnoExt::IsChildOf(pThis, pTarget))
		return ReturnFireErrorIllegal;

	if (pTarget)
	{
		const auto pWHExt = WarheadTypeExt::ExtMap.Find(pWeapon->Warhead);
		if (pWHExt->MindControl_Permanent)
		{
			if (pTarget->GetTechnoType()->ImmuneToPsionics || pTarget->MindControlledByAUnit || pTarget->MindControlledBy)
			{
				R->EAX(FireError::CANT);
				return ReturnFireErrorIllegal;
			}
		}

		TechnoExt::ExtData* pTargetExt = TechnoExt::ExtMap.Find(pTarget);
		if (pTargetExt->Attacker_Weapon && pTargetExt->Attacker)
		{
			const auto pAttackerWeaponExt = WeaponTypeExt::ExtMap.Find(pTargetExt->Attacker_Weapon);
			if (pTargetExt->Attacker != pThis)
			{
				if (pTargetExt->Attacker_Weapon == pWeapon)
				{
					R->EAX(FireError::CANT);
					return ReturnFireErrorIllegal;
				}
				else
				{
					if (!pAttackerWeaponExt->OnlyAllowOneFirer_OtherWeapons.empty())
					{
						auto it = std::find(pAttackerWeaponExt->OnlyAllowOneFirer_OtherWeapons.begin(), pAttackerWeaponExt->OnlyAllowOneFirer_OtherWeapons.end(), pWeapon);
						size_t idx = it - pAttackerWeaponExt->OnlyAllowOneFirer_OtherWeapons.begin();

						if (!(it == pAttackerWeaponExt->OnlyAllowOneFirer_OtherWeapons.end() || idx >= pAttackerWeaponExt->OnlyAllowOneFirer_OtherWeapons.size()))
						{
							R->EAX(FireError::CANT);
							return ReturnFireErrorIllegal;
						}
					}

					if (!pAttackerWeaponExt->OnlyAllowOneFirer_IgnoreWeapons.empty())
					{
						auto it = std::find(pAttackerWeaponExt->OnlyAllowOneFirer_IgnoreWeapons.begin(), pAttackerWeaponExt->OnlyAllowOneFirer_IgnoreWeapons.end(), pWeapon);
						size_t idx = it - pAttackerWeaponExt->OnlyAllowOneFirer_IgnoreWeapons.begin();

						if (it == pAttackerWeaponExt->OnlyAllowOneFirer_IgnoreWeapons.end() || idx >= pAttackerWeaponExt->OnlyAllowOneFirer_IgnoreWeapons.size())
						{
							R->EAX(FireError::CANT);
							return ReturnFireErrorIllegal;
						}
					}
				}
			}
		}
	}

	return ContinueCheck;
}

// TODO WhatWeaponShouldIUse

DEFINE_HOOK(0x6F3283, TechnoClass_CanScatter_CheckIfAttached, 0x8)
{
	enum { ReturnFalse = 0x6F32C5, ContinueCheck = 0x0 };

	GET(TechnoClass*, pThis, ECX);
	auto const& pExt = TechnoExt::ExtMap.Find(pThis);

	return pExt->ParentAttachment
		? ReturnFalse
		: ContinueCheck;
}

// TODO maybe? handle scatter for InfantryClass and possibly AircraftClass

DEFINE_HOOK(0x736FB6, UnitClass_FiringAI_ForbidAttachmentRotation, 0x6)
{
	enum { SkipBodyRotation = 0x737063, ContinueCheck = 0x0 };

	GET(UnitClass*, pThis, ESI);
	auto const& pExt = TechnoExt::ExtMap.Find(pThis);

	return pExt->ParentAttachment
		? SkipBodyRotation
		: ContinueCheck;
}

DEFINE_HOOK(0x736A2F, UnitClass_RotationAI_ForbidAttachmentRotation, 0x7)
{
	enum { SkipBodyRotation = 0x736A8E, ContinueCheck = 0x0 };

	GET(UnitClass*, pThis, ESI);
	auto const& pExt = TechnoExt::ExtMap.Find(pThis);

	return pExt->ParentAttachment
		? SkipBodyRotation
		: ContinueCheck;
}

Action __fastcall UnitClass_MouseOverCell(UnitClass* pThis, void* _, CellStruct const* pCell, bool checkFog, bool ignoreForce)
{
	JMP_THIS(0x7404B0);
}

Action __fastcall UnitClass_MouseOverCell_Wrapper(UnitClass* pThis, void* _, CellStruct const* pCell, bool checkFog, bool ignoreForce)
{
	Action result = UnitClass_MouseOverCell(pThis, _, pCell, checkFog, ignoreForce);

	auto const& pExt = TechnoExt::ExtMap.Find(pThis);
	if (!pExt->ParentAttachment)
		return result;

	switch (result)
	{
	case Action::GuardArea:
	case Action::AttackMoveNav:
	case Action::PatrolWaypoint:
	case Action::Harvest:
	case Action::Move:
		result = Action::NoMove;
		break;
	case Action::EnterTunnel:
		result = Action::NoEnterTunnel;
		break;
	}

	return result;
}

DEFINE_JUMP(VTABLE, 0x7F5CE0, GET_OFFSET(UnitClass_MouseOverCell_Wrapper))

DEFINE_HOOK(0x4D74EC, FootClass_ObjectClickedAction_HandleAttachment, 0x6)
{
	enum { ReturnFalse = 0x4D77EC, Continue = 0x0 };

	GET(FootClass*, pThis, ESI);
	GET_STACK(Action, action, STACK_OFFSET(0x108, +0x4));
	auto const& pExt = TechnoExt::ExtMap.Find(pThis);

	if (!pExt->ParentAttachment)
		return Continue;

	switch (action)
	{
	case Action::Move:
	case Action::AttackMoveNav:
	case Action::NoMove:
	case Action::Enter:
	case Action::NoEnter:
	case Action::Capture:
	case Action::Repair:
	case Action::Sabotage:
	case Action::GuardArea:
		return ReturnFalse;
	default:
		return Continue;
	}
}

DEFINE_HOOK(0x4D7D58, FootClass_CellClickedAction_HandleAttachment, 0x6)
{
	enum { ReturnFalse = 0x4D7D62, Continue = 0x0 };

	GET(FootClass*, pThis, ESI);
	GET_STACK(Action, action, STACK_OFFSET(0x24, +0x4));
	auto const& pExt = TechnoExt::ExtMap.Find(pThis);

	if (!pExt->ParentAttachment)
		return Continue;

	switch (action)
	{
	case Action::Move:
	case Action::AttackMoveNav:
	case Action::NoMove:
	case Action::Enter:
	case Action::NoEnter:
	case Action::Harvest:
	case Action::Capture:
	case Action::Sabotage:
	case Action::GuardArea:
	case Action::EnterTunnel:
	case Action::NoEnterTunnel:
	case Action::PatrolWaypoint:
		return ReturnFalse;
	default:
		return Continue;
	}
}

// DEFINE_HOOK(0x41BE02, AbstractClass_RenderCoord_AttachedCoord, 0x7)
// {
// 	enum { Return = 0x41BE2A, Continue = 0x0 };

// 	GET(AbstractClass* const, pThis, ECX);

// 	if (auto const& pThisAsTechno = abstract_cast<TechnoClass*>(pThis))
// 	{
// 		auto const& pExt = TechnoExt::ExtMap.Find(pThisAsTechno);
// 		if (pExt && pExt->ParentAttachment)
// 		{
// 			R->EAX<CoordStruct*>(&pExt->ParentAttachment->GetChildLocation());
// 			return Return;
// 		}
// 	}

// 	return Continue;
// }

CoordStruct __fastcall ObjectClass_GetRenderCoords(ObjectClass* pThis, void* _)
{
	JMP_THIS(0x41BE00);
}

CoordStruct __fastcall BuildingClass_GetRenderCoords(BuildingClass* pThis, void* _)
{
	JMP_THIS(0x459EF0);
}

CoordStruct __fastcall TechnoClass_GetRenderCoords(TechnoClass* pThis, void* _)
{
	auto const& pExt = TechnoExt::ExtMap.Find(pThis);
	if (pExt && pExt->ParentAttachment)
	{
		// The parent origin is our origin, we will offset later in draw function
		return pExt->ParentAttachment->Cache.TopLevelParent->GetRenderCoords();
	}

	return ObjectClass_GetRenderCoords(pThis, _);
}

// TODO hook matrix
// 6F3B88 DONE
// 6F3DA4 DONE
// 73B5B5 DONE
// 73C864 DONE

// DEFINE_HOOK(0x6F3B88, TechnoClass_FireCoord_AttachmentAdjust, 0x6)
// {
// 	enum { Skip = 0x6F3B9E };

// 	GET(TechnoClass*, pThis, EBX);

// 	R->EAX(&TechnoExt::GetAttachmentTransform(pThis));
// 	return Skip;
// }

// DEFINE_HOOK(0x6F3DA4, TechnoClass_firecoord_6F3D60_AttachmentAdjust, 0x6)
// {
// 	enum { Skip = 0x6F3DBA };

// 	GET(TechnoClass*, pThis, EBX);

// 	R->EAX(&TechnoExt::GetAttachmentTransform(pThis));
// 	return Skip;
// }

DEFINE_HOOK(0x73B5B5, UnitClass_DrawVoxel_AttachmentAdjust, 0x6)
{
	enum { Skip = 0x73B5CE };

	GET(UnitClass*, pThis, EBP);
	LEA_STACK(VoxelIndexKey*, pKey, STACK_OFFSET(0x1C4, -0x1B0));

	Matrix3D transform = TechnoExt::GetAttachmentTransform(pThis, pKey);
	R->EAX(&transform);

	return Skip;
}

DEFINE_HOOK(0x73C864, UnitClass_drawcode_AttachmentAdjust, 0x6)
{
	enum { Skip = 0x73C87D };

	GET(UnitClass*, pThis, EBP);
	LEA_STACK(VoxelIndexKey*, pKey, STACK_OFFSET(0x128, -0xC8));

	Matrix3D transform = TechnoExt::GetAttachmentTransform(pThis, pKey);
	R->EAX(&transform);

	return Skip;
}

DEFINE_HOOK(0x73C29F, UnitClass_DrawVoxel_AttachmentAdjustShadow, 0x6)
{
	enum { Skip = 0x73C2B8 };

	GET(UnitClass*, pThis, EBP);
	LEA_STACK(VoxelIndexKey*, pKey, STACK_OFFSET(0x1F0, -0x1DC));

	Matrix3D transform = TechnoExt::GetAttachmentTransform(pThis, pKey, true);
	R->EAX(&transform);

	return Skip;
}

DEFINE_HOOK(0x414953, AircraftClass_DrawIt_AttachmentAdjust, 0x6)
{
	enum { Skip = 0x41496C };

	GET(AircraftClass*, pThis, EBP);
	LEA_STACK(VoxelIndexKey*, pKey, STACK_OFFSET(0xD8, -0xC8));

	Matrix3D transform = TechnoExt::GetAttachmentTransform((TechnoClass*)pThis, pKey);
	R->EAX(&transform);

	return Skip;
}

DEFINE_HOOK(0x41480D, AircraftClass_DrawIt_AttachmentAdjustShadow, 0x6)
{
	enum { Skip = 0x414826 };

	GET(AircraftClass*, pThis, EBP);
	LEA_STACK(VoxelIndexKey*, pKey, STACK_OFFSET(0xD4, -0xC4));

	Matrix3D transform = TechnoExt::GetAttachmentTransform((TechnoClass*)pThis, pKey, true);
	R->EAX(&transform);

	return Skip;
}

// TODO merge hooks

// TODO aircraft
// normal matrix 0x414969
// shadow matrix 0x414823

// TODO hook shadow matrix

// BuildingClass::GetRenderCoords already has it's own override,
// should hook it if you ever want to dip into that - Kerbiter

// FIXME it's probably not a good idea to mix vtable replacements with manual hooks
// DEFINE_JUMP(VTABLE, 0x7F4A0C, GET_OFFSET(TechnoClass_GetRenderCoords)) // TechnoClass
// DEFINE_JUMP(VTABLE, 0x7E8D40, GET_OFFSET(TechnoClass_GetRenderCoords)) // FootClass
// DEFINE_JUMP(VTABLE, 0x7F5D1C, GET_OFFSET(TechnoClass_GetRenderCoords)) // UnitClass
// DEFINE_JUMP(VTABLE, 0x7EB104, GET_OFFSET(TechnoClass_GetRenderCoords)) // InfantryClass
// DEFINE_JUMP(VTABLE, 0x7E2350, GET_OFFSET(TechnoClass_GetRenderCoords)) // AircraftClass


// // DEFINE_JUMP(VTABLE, 0x7F4A74, GET_OFFSET(TechnoClass_DrawIt))

// // skip children before parent drawn
// DEFINE_HOOK(0x73CEC0, UnitClass_DrawIt_HandleChildren , 0x5)
// {
// 	GET(UnitClass*, pThis, ECX);

// 	if (TechnoExt::IsAttached(pThis))
// }

namespace TechnoAttachmentTemp
{
	ObjectClass* pThis;
	Point2D realDrawPoint = Point2D::Empty;
}

DEFINE_HOOK(0x5F4B13, ObjectClass_Render_Context_Set, 0x5)
{
	GET(ObjectClass*, pThis, ECX);

	TechnoAttachmentTemp::pThis = pThis;

	return 0;
}

// Swap the render coord after the draw rect is calculated
DEFINE_HOOK(0x5F4CC3, ObjectClass_Render_AttachmentAdjust, 0x6)
{
	LEA_STACK(Point2D*, drawPoint, STACK_OFFSET(0x30, -0x18));
	TechnoAttachmentTemp::realDrawPoint = *drawPoint;

	ObjectClass* pThis = TechnoAttachmentTemp::pThis;
	if (pThis->AbstractFlags & AbstractFlags::Techno)
		TacticalClass::Instance->CoordsToClient(TechnoExt::GetTopLevelParent((TechnoClass*)pThis)->GetRenderCoords(), drawPoint);

	return 0;
}

DEFINE_HOOK(0x6D9EF0, TacticalClass_AddSelectable_SwapCoord, 0x6)
{
	REF_STACK(TechnoClass*, pTechno, STACK_OFFSET(0x0, 0x4));
	REF_STACK(int, x, STACK_OFFSET(0x0, 0x8));
	REF_STACK(int, y, STACK_OFFSET(0x0, 0xC));

	if (pTechno == TechnoAttachmentTemp::pThis)
	{
		x = TechnoAttachmentTemp::realDrawPoint.X;
		y = TechnoAttachmentTemp::realDrawPoint.Y;
	}

	return 0;
}

// YSort for attachments
int __fastcall TechnoClass_SortY_Wrapper(ObjectClass* pThis)
{
	auto const pTechno = abstract_cast<TechnoClass*>(pThis);

	if (pTechno)
	{
		const auto pExt = TechnoExt::ExtMap.Find(pTechno);

		if (pExt->ParentAttachment)
		{
			const auto ySortPosition = pExt->ParentAttachment->GetType()->YSortPosition.Get();
			const auto pParentTechno = pExt->ParentAttachment->Parent;

			if (ySortPosition != AttachmentYSortPosition::Default && pParentTechno)
			{
				int parentYSort = pParentTechno->GetYSort();

				return parentYSort + (ySortPosition == AttachmentYSortPosition::OverParent ? 1 : -1);
			}
		}
	}

	return pThis->Get_YSort();
}

DEFINE_JUMP(CALL, 0x449413, GET_OFFSET(TechnoClass_SortY_Wrapper))   // BuildingClass
DEFINE_JUMP(VTABLE, 0x7E235C, GET_OFFSET(TechnoClass_SortY_Wrapper)) // AircraftClass
DEFINE_JUMP(VTABLE, 0x7EB110, GET_OFFSET(TechnoClass_SortY_Wrapper)) // InfantryClass
DEFINE_JUMP(VTABLE, 0x7F5D28, GET_OFFSET(TechnoClass_SortY_Wrapper)) // UnitClass
