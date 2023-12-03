#include "Body.h"
#include <GameStrings.h>

#include <Ext/HouseType/Body.h>

// SellSound and EVA dehardcode
DEFINE_HOOK(0x4D9F7B, FootClass_Sell, 0x6)
{
	enum { ReadyToVanish = 0x4D9FCB };
	GET(FootClass*, pThis, ESI);

	int money = pThis->GetRefund();
	pThis->Owner->GiveMoney(money);

	if (pThis->Owner->IsInPlayerControl)
	{
		const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

		if (pThis->WhatAmI() == AbstractType::Building)
			VoxClass::PlayIndex(pTypeExt->EVA_Sold.Get(VoxClass::FindIndex(GameStrings::EVA_StructureSold)));
		else
			VoxClass::PlayIndex(pTypeExt->EVA_Sold.Get(VoxClass::FindIndex(GameStrings::EVA_UnitSold)));
		//WW used VocClass::PlayGlobal to play the SellSound, why did they do that?
		VocClass::PlayAt(pTypeExt->SellSound.Get(RulesClass::Instance->SellSound), pThis->Location);
	}

	if (RulesExt::Global()->DisplayIncome.Get())
		FlyingStrings::AddMoneyString(money, pThis->Owner, RulesExt::Global()->DisplayIncome_Houses.Get(), pThis->Location);

	return ReadyToVanish;
}

// Rewrite 0x449BC0
// true: undeploy into vehicle; false: sell
bool __forceinline BuildingCanUndeploy(BuildingClass* pThis)
{
	auto pType = pThis->Type;

	if (!pType->UndeploysInto)
		return false;

	if (pType->ConstructionYard)
	{
		// Canyards can't undeploy if MCVRedeploy=no or MindControlledBy YURIX
		if (!GameModeOptionsClass::Instance->MCVRedeploy || pThis->MindControlledBy)
			return false;
	}

	// Move Focus check outside Conyard check to allow generic Unsellable=no buildings to be sold
	return pThis->Focus;
}

// Skip SessionClass::IsCampaign() checks, where inlined not exactly the function above but sth similar
DEFINE_JUMP(LJMP, 0x443A9A, 0x443AA3); //BuildingClass_SetRallyPoint
DEFINE_JUMP(LJMP, 0x44375E, 0x443767); //BuildingClass_CellClickedAction
DEFINE_JUMP(LJMP, 0x44F602, 0x44F60B); //BuildingClass_IsControllable

DEFINE_HOOK(0x449CC1, BuildingClass_Mi_Selling_EVASold_UndeploysInto, 0x6)
{
	enum { CreateUnit = 0x449D5E, SkipTheEntireShit = 0x44A1E8 };
	GET(BuildingClass*, pThis, EBP);

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);
	// Fix Conyards can't play EVA_StructureSold
	if (pThis->Owner->IsInPlayerControl && (!pThis->Focus || !pThis->Type->UndeploysInto))
		VoxClass::PlayIndex(pTypeExt->EVA_Sold.Get(VoxClass::FindIndex(GameStrings::EVA_StructureSold)));

	return BuildingCanUndeploy(pThis) ? CreateUnit : SkipTheEntireShit;
}

DEFINE_HOOK(0x44A7CF, BuildingClass_Mi_Selling_PlaySellSound, 0x6)
{
	enum { FinishPlaying = 0x44A85B };
	GET(BuildingClass*, pThis, EBP);

	if (!BuildingCanUndeploy(pThis))
	{
		const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);
		VocClass::PlayAt(pTypeExt->SellSound.Get(RulesClass::Instance->SellSound), pThis->Location);
	}

	const auto pHouseTypeExt = HouseTypeExt::ExtMap.Find(pThis->Owner->Type);
	const auto pExt = TechnoExt::ExtMap.Find(pThis);
	const auto pTypeExt = pExt->TypeExtData;

	InfantryTypeClass* pilot = pExt->PilotType ? pExt->PilotType : pTypeExt->Pilot_CreateType.Get(pHouseTypeExt->PilotType);
	auto pPilotTypeExt = TechnoTypeExt::ExtMap.Find(pilot);

	HouseClass* pilotowner = (pExt->PilotType && pExt->PilotOwner) ? pExt->PilotOwner : pThis->Owner;

	if (pilot && !pilotowner->Defeated && (pPilotTypeExt->Pilot_IgnoreTrainable ? true : pThis->Veterancy.Veterancy >= 1.0))
	{
		if (auto const pPilot = static_cast<InfantryClass*>(pilot->CreateObject(pilotowner)))
		{
			const auto pCell = MapClass::Instance->TryGetCellAt(pThis->Location);
			if (!pCell)
			{
				pPilot->UnInit();
			}
			else
			{
				pPilot->OnBridge = (pCell->ContainsBridge() && (pThis->Location.Z >= pCell->GetCoordsWithBridge().Z));

				DirType nRandFacing = static_cast<DirType>(ScenarioClass::Instance->Random.RandomRanged(0, 255));
				++Unsorted::IKnowWhatImDoing();
				pPilot->Unlimbo(pThis->Location, nRandFacing);
				--Unsorted::IKnowWhatImDoing();

				if (auto const pController = pThis->MindControlledBy)
				{
					++Unsorted::IKnowWhatImDoing;
					pController->CaptureManager->Free(pThis);
					pController->CaptureManager->Capture(pPilot);
					--Unsorted::IKnowWhatImDoing;
					pPilot->QueueMission(Mission::Guard, true);
				}

				VocClass::PlayAt(TechnoTypeExt::ExtMap.Find(pilot)->Pilot_LeaveSound, pThis->Location, nullptr);

				VeterancyStruct* vstruct = &pPilot->Veterancy;
				vstruct->Veterancy = pThis->Veterancy.Veterancy;

				int health = pExt->PilotHealth > 0 ? pExt->PilotHealth : pilot->Strength;
				pPilot->Health = std::max(health, 10);

				if (pPilot->IsInAir())
					TechnoExt::FallenDown(pPilot);
				else
					TechnoExt::ExtMap.Find(pPilot)->WasFallenDown = true;
			}
		}
	}

	return FinishPlaying;
}

DEFINE_HOOK(0x44A8E5, BuildingClass_Mi_Selling_SetTarget, 0x6)
{
	enum { ResetTarget = 0x44A937, SkipShit = 0x44A95E };
	GET(BuildingClass*, pThis, EBP);

	return BuildingCanUndeploy(pThis) ? ResetTarget : SkipShit;
}

DEFINE_HOOK(0x44A964, BuildingClass_Mi_Selling_VoiceDeploy, 0x6)
{
	enum { CanDeploySound = 0x44A9CA, SkipShit = 0x44AA3D };
	GET(BuildingClass*, pThis, EBP);

	return BuildingCanUndeploy(pThis) ? CanDeploySound : SkipShit;
}

DEFINE_HOOK(0x44AB22, BuildingClass_Mi_Selling_EVASold_Plug, 0x6)
{
	enum { SkipVoxPlay = 0x44AB3B };
	GET(BuildingClass*, pThis, EBP);

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);

	if (pThis->Owner->IsInPlayerControl)
		VoxClass::PlayIndex(pTypeExt->EVA_Sold.Get(VoxClass::FindIndex(GameStrings::EVA_StructureSold)));

	return SkipVoxPlay;
}
