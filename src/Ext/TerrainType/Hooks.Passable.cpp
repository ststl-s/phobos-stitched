#include "Body.h"

#include <TerrainClass.h>

#include <Utilities/GeneralUtils.h>

// Passable TerrainTypes Hook #1 - Do not set occupy bits.
DEFINE_HOOK(0x71C110, TerrainClass_SetOccupyBit_PassableTerrain, 0x6)
{
	enum { Skip = 0x71C1A0 };

	GET(TerrainClass*, pThis, ECX);

	if (auto const pTypeExt = TerrainTypeExt::ExtMap.Find(pThis->Type))
	{
		if (pTypeExt->IsPassable)
			return Skip;
	}

	return 0;
}

// Passable TerrainTypes Hook #2 - Do not display attack cursor unless force-firing.
DEFINE_HOOK(0x7002E9, TechnoClass_WhatAction_PassableTerrain, 0x5)
{
	enum { Skip = 0x70020E };

	GET(ObjectClass*, pTarget, EDI);
	GET_STACK(bool, isForceFire, STACK_OFFS(0x1C, -0x8));

	if (pTarget->WhatAmI() == AbstractType::Terrain)
	{
		if (auto const pTypeExt = TerrainTypeExt::ExtMap.Find((abstract_cast<TerrainClass*>(pTarget))->Type))
		{
			if (pTypeExt->IsPassable && !isForceFire)
			{
				R->EBP(1);
				return Skip;
			}
		}
	}

	return 0;
}

// Passable TerrainTypes Hook #3 - Count passable TerrainTypes as completely passable.
DEFINE_HOOK(0x483D87, CellClass_CheckPassability_PassableTerrain, 0x5)
{
	enum { Skip = 0x483DCD, Return = 0x483E25 };

	GET(CellClass*, pThis, EDI);
	GET(ObjectClass*, pObject, ESI);

	if (auto const pTerrain = abstract_cast<TerrainClass*>(pObject))
	{
		if (auto const pTypeExt = TerrainTypeExt::ExtMap.Find(pTerrain->Type))
		{
			if (pTypeExt->IsPassable)
			{
				pThis->Passability = 0;
				return Return;
			}
		}
	}

	return Skip;
}

// Passable TerrainTypes Hook #4 - Allow placing buildings on top of them.
DEFINE_HOOK_AGAIN(0x47C80E, CellClass_IsClearTo_Build_PassableTerrain, 0x5)
DEFINE_HOOK(0x47C745, CellClass_IsClearTo_Build_PassableTerrain, 0x5)
{
	enum { Skip = 0x47C85F };

	GET(CellClass*, pThis, EDI);

	auto pTerrain = pThis->GetTerrain(false);

	if (pTerrain)
	{
		if (auto const pTypeExt = TerrainTypeExt::ExtMap.Find(pTerrain->Type))
		{
			if (pTypeExt->IsPassable && pTypeExt->IsPassable_CanBeBuiltOn)
				return Skip;
		}
	}

	return 0;
}

// Passable TerrainTypes Hook #5 - Allow placing laser fences on top of them.
DEFINE_HOOK(0x47C657, CellClass_IsClearTo_Build_PassableTerrain_LF, 0x6)
{
	enum { Skip = 0x47C6A0, Return = 0x47C6D1 };

	GET(CellClass*, pThis, EDI);

	auto pObj = pThis->FirstObject;

	if (pObj)
	{
		bool isEligible = true;

		while (true)
		{
			isEligible = pObj->WhatAmI() != AbstractType::Building;

			if (auto const pTerrain = abstract_cast<TerrainClass*>(pObj))
			{
				isEligible = false;
				auto const pTypeExt = TerrainTypeExt::ExtMap.Find(pTerrain->Type);

				if (pTypeExt->IsPassable && pTypeExt->IsPassable_CanBeBuiltOn)
					isEligible = true;
			}

			if (!isEligible)
				break;

			pObj = pObj->NextObject;

			if (!pObj)
				return Skip;
		}

		return Return;
	}

	return Skip;
}

// Passable TerrainTypes Hook #6 - Draw laser fence placement even if passable terrain is on way.
DEFINE_HOOK(0x6D57C1, TacticalClass_DrawLaserFencePlacement_PassableTerrain, 0x9)
{
	enum { ContinueChecks = 0x6D57D2, DontDraw = 0x6D59A6 };

	GET(CellClass*, pCell, ESI);

	if (auto const pTerrain = pCell->GetTerrain(false))
	{
		auto const pTypeExt = TerrainTypeExt::ExtMap.Find(pTerrain->Type);

		if (pTypeExt->IsPassable && pTypeExt->IsPassable_CanBeBuiltOn)
			return ContinueChecks;

		return DontDraw;
	}

	return ContinueChecks;
}

// Passable TerrainTypes Hook #7 - Allow creating overlays on passable terrain.
DEFINE_HOOK(0x5FD2B6, OverlayClass_Unlimbo_PassableTerrain, 0x9)
{
	enum { ContinueChecks = 0x5FD2CA, Disallow = 0x5FD2C3 };

	GET(CellClass*, pCell, EAX);

	if (auto const pTerrain = pCell->GetTerrain(false))
	{
		auto const pTypeExt = TerrainTypeExt::ExtMap.Find(pTerrain->Type);

		if (pTypeExt->IsPassable)
			return ContinueChecks;

		return Disallow;
	}

	return ContinueChecks;
}

// Passable TerrainTypes Hook #8 - Make passable for vehicles.
DEFINE_HOOK(0x73FB71, UnitClass_CanEnterCell_PassableTerrain, 0x5)
{
	enum { Return = 0x73FD37 };

	GET(TerrainClass*, pTarget, ESI);

	if (auto const pTypeExt = TerrainTypeExt::ExtMap.Find((abstract_cast<TerrainClass*>(pTarget))->Type))
	{
		if (pTypeExt->IsPassable)
		{
			R->EBP(0);
			return 0x73FD37;
		}
	}

	return 0;
}