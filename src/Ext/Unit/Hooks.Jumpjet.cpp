#include <JumpjetLocomotionClass.h>
#include <UnitClass.h>

#include <Ext/Rules/Body.h>
#include <Ext/TechnoType/Body.h>

#include <Utilities/Macro.h>

DEFINE_HOOK(0x54B8E9, JumpjetLocomotionClass_In_Which_Layer_Deviation, 0x6)
{
	GET(FootClass* const, pThis, EAX);

	if (pThis->IsInAir())
	{
		if (auto const pExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType()))
		{
			if (!pExt->JumpjetAllowLayerDeviation.Get(RulesExt::Global()->JumpjetAllowLayerDeviation.Get()))
			{
				R->EDX(INT32_MAX); // Override JumpjetHeight / CruiseHeight check so it always results in 3 / Layer::Air.
				return 0x54B96B;
			}
		}
	}

	return 0;
}

// I think JumpjetLocomotionClass::State is probably an enum, where
// 0 - On ground
// 1 - Taking off from ground
// 2 - Hovering in air
// 3 - Moving in air
// 4 - Deploying to land
// 5 - Crashing
// 6 - Invalid?

// Bugfix: Jumpjet turn to target when attacking
// The way vanilla game handles facing turning is a total mess, so even though this is not the most correct place to do it, given that 0x54BF5B has something similar, I just do it here too
// TODO : The correct fix : 0x736FC4 for stucking at FireError::FACING, 0x736EE9 for something else like OmniFire etc.
DEFINE_HOOK(0x54BD93, JumpjetLocomotionClass_State2_54BD30_TurnToTarget, 0x6)
{
	enum { ContinueNoTarget = 0x54BDA1, EndFunction = 0x54BFDE };
	GET(JumpjetLocomotionClass* const, pLoco, ESI);
	GET(FootClass* const, pLinkedTo, EDI);

	const auto pTarget = pLinkedTo->Target;
	if (!pTarget)
		return ContinueNoTarget;

	if (const auto pThis = abstract_cast<UnitClass*>(pLinkedTo))
	{
		const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);
		if (pTypeExt->JumpjetTurnToTarget.Get(RulesExt::Global()->JumpjetTurnToTarget))
		{
			CoordStruct& source = pThis->Location;
			CoordStruct target = pTarget->GetCoords();
			DirStruct tgtDir = DirStruct { Math::atan2(source.Y - target.Y, target.X - source.X) };

			if (pThis->GetRealFacing().GetFacing<32>() != tgtDir.GetFacing<32>())
				pLoco->LocomotionFacing.SetDesired(tgtDir);
		}
	}

	R->EAX(pTarget);
	return EndFunction;
}
#ifdef REMOVE_THIS_AFTER_V03_RELEASE
// Bugfix: Align jumpjet turret's facing with body's
DEFINE_HOOK(0x736BF3, UnitClass_UpdateRotation_TurretFacing, 0x6)
{
	GET(UnitClass* const, pThis, ESI);
	// I still don't know why jumpjet loco behaves differently for the moment
	// so I don't check jumpjet loco or InAir here, feel free to change if it doesn't break performance.
	if (!pThis->Target && !pThis->Type->TurretSpins && (pThis->Type->JumpJet || pThis->Type->BalloonHover))
	{
		pThis->SecondaryFacing.SetDesired(pThis->PrimaryFacing.Current());
		pThis->TurretIsRotating = pThis->SecondaryFacing.IsRotating();
		return 0x736C09;
	}

	return 0;
}
#else

DEFINE_HOOK(0x736BA3, UnitClass_UpdateRotation_TurretFacing_TemporaryFix, 0x6)
{
	GET(UnitClass* const, pThis, ESI);
	enum { SkipCheckDestination = 0x736BCA, GetDirectionTowardsDestination = 0x736BBB };
	// When jumpjets arrived at their FootClass::Destination, they seems stuck at the Move mission
	// and therefore the turret facing was set to DirStruct{atan2(0,0)}==DirType::East at 0x736BBB
	// that's why they will come back to normal when giving stop command explicitly
	auto pType = pThis->Type;
	// so the best way is to fix the Mission if necessary, but I don't know how to do it
	// so I skipped jumpjets check temporarily, and in most cases Jumpjet/BallonHover should cover most of it
	if (!pType->TurretSpins && (pType->JumpJet || pType->BalloonHover))
		return SkipCheckDestination;

	return 0;
}

#endif
// Bugfix: Jumpjet detect cloaked objects beneath
DEFINE_HOOK(0x54C036, JumpjetLocomotionClass_State3_54BFF0_UpdateSensors, 0x7)
{
	GET(FootClass* const, pLinkedTo, ECX);
	GET(CellStruct const, currentCell, EAX);

	// Copied from FootClass::UpdatePosition
	if (pLinkedTo->GetTechnoType()->SensorsSight)
	{
		CellStruct const lastCell = pLinkedTo->LastJumpjetMapCoords;
		if (lastCell != currentCell)
		{
			pLinkedTo->RemoveSensorsAt(lastCell);
			pLinkedTo->AddSensorsAt(currentCell);
		}
	}
	// Something more may be missing

	return 0;
}

//TODO : Issue #690 #655


// Fix initial facing when jumpjet locomotor is being attached
DEFINE_HOOK(0x54AE44, JumpjetLocomotionClass_LinkToObject_FixFacing, 0x7)
{
	GET(ILocomotion*, iLoco, EBP);
	auto const pThis = static_cast<JumpjetLocomotionClass*>(iLoco);

	pThis->LocomotionFacing.SetCurrent(pThis->LinkedTo->PrimaryFacing.Current());
	pThis->LocomotionFacing.SetDesired(pThis->LinkedTo->PrimaryFacing.Desired());

	return 0;
}

// Fix initial facing when jumpjet locomotor on unlimbo
void __stdcall JumpjetLocomotionClass_Unlimbo(ILocomotion* pThis)
{
	auto const pThisLoco = static_cast<JumpjetLocomotionClass*>(pThis);

	pThisLoco->LocomotionFacing.SetCurrent(pThisLoco->LinkedTo->PrimaryFacing.Current());
	pThisLoco->LocomotionFacing.SetDesired(pThisLoco->LinkedTo->PrimaryFacing.Desired());
}

DEFINE_JUMP(VTABLE, 0x7ECDB8, GET_OFFSET(JumpjetLocomotionClass_Unlimbo))

//烈葱的JJ坠毁在建筑上旋转的修复
DEFINE_HOOK(0x54D31A, JumpjetLocomotionClass_Sub_54D0F0_ZDeltaFix, 0x6)
{
	GET(JumpjetLocomotionClass*, pLoco, ESI);
	return pLoco->State == JumpjetLocomotionClass::State::Crashing ? 0x54D350 : 0;
}
