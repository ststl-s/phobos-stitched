#include <JumpjetLocomotionClass.h>
#include <UnitClass.h>

#include <Ext/TechnoType/Body.h>

// Bugfix: Jumpjet turn to target when attacking
// TODO: This is still not the right place to treat jumpjet turning
DEFINE_HOOK(0x54AEC0, JumpjetLocomotion_Process_TurnToTarget, 0x8)
{
	GET_STACK(ILocomotion*, iLoco, 0x4);
	const auto pLoco = static_cast<JumpjetLocomotionClass*>(iLoco);
	const auto pThis = pLoco->Owner;
	const auto pType = pThis->GetTechnoType();
	const auto pTarget = pThis->Target;

	if (pTarget && pThis->IsInAir() && !pType->TurretSpins && pThis->WhatAmI() == AbstractType::Unit)
	{
		const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
		if (pTypeExt->JumpjetTurnToTarget.Get(RulesExt::Global()->JumpjetTurnToTarget))
		{
			CoordStruct& source = pThis->Location;
			CoordStruct target = pTarget->GetCoords();
			DirStruct tgtDir = DirStruct(Math::arctanfoo(source.Y - target.Y, target.X - source.X));

			if (pThis->GetRealFacing().value32() != tgtDir.value32())
				pLoco->LocomotionFacing.turn(tgtDir);
		}
	}

	return 0;
}

// Bugfix: Align jumpjet turret's facing with body's
DEFINE_HOOK(0x736BF3, UnitClass_UpdateRotation_TurretFacing, 0x6)
{
	GET(UnitClass*, pThis, ESI);

	if (!pThis->Target)
	{
		auto pType = pThis->Type;
		if (pType->JumpJet && !pType->TurretSpins)
		{
			pThis->SecondaryFacing.turn(pThis->PrimaryFacing.current());
			pThis->unknown_49C = pThis->PrimaryFacing.in_motion();
			return 0x736C09;
		}
	}

	return 0;
}

// Bugfix: Jumpjet detect cloaked objects beneath
// I think JumpjetLocomotionClass::State is probably an enum, where
// 0 - On ground
// 1 - Taking off from ground
// 2 - Hovering in air
// 3 - Moving in air
// 4 - Deploying to land
// 5 - Crashing
DEFINE_HOOK(0x54C14B, JumpjetLocomotionClass_State3_54BFF0_UpdateMotion, 0x7)
{
	GET(FootClass*, pLinkedTo, EDI);
	//if (pLinkedTo->GetTechnoType()->Sensors) // Not sure if it's forgotten to be updated or due to other reasons, disable the check to see if there's any side effects
	pLinkedTo->UpdatePosition(2);

	return 0;
}
