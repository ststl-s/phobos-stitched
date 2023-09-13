#include "Body.h"

#include <Helpers/Enumerators.h>

#include <Ext/BuildingType/Body.h>
#include <Ext/House/Body.h>
#include <Ext/Scenario/Body.h>
#include <Ext/Team/Body.h>
#include <Ext/Techno/Body.h>

#include <Utilities/TemplateDef.h>
#include <Utilities/SavegameDef.h>

template<> const DWORD Extension<ScriptClass>::Canary = 0x3B3B3B3B;
ScriptExt::ExtContainer ScriptExt::ExtMap;

// =============================
// load / save

void ScriptExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	// Nothing yet
}

void ScriptExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	// Nothing yet
}

// =============================
// container

ScriptExt::ExtContainer::ExtContainer() : Container("ScriptClass")
{
}

ScriptExt::ExtContainer::~ExtContainer() = default;

void ScriptExt::ProcessAction(TeamClass* pTeam)
{
	const int action = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Action;
	const int argument = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument;
	switch (static_cast<PhobosScripts>(action))
	{
	case PhobosScripts::TimedAreaGuard:
		ScriptExt::ExecuteTimedAreaGuardAction(pTeam);
		break;
	case PhobosScripts::LoadIntoTransports:
		ScriptExt::LoadIntoTransports(pTeam);
		break;
	case PhobosScripts::WaitUntilFullAmmo:
		ScriptExt::WaitUntilFullAmmoAction(pTeam);
		break;
	case PhobosScripts::RepeatAttackCloserThreat:
		// Threats that are close have more priority. Kill until no more targets.
		ScriptExt::Mission_Attack(pTeam, true, 0, -1, -1);
		break;
	case PhobosScripts::RepeatAttackFartherThreat:
		// Threats that are far have more priority. Kill until no more targets.
		ScriptExt::Mission_Attack(pTeam, true, 1, -1, -1);
		break;
	case PhobosScripts::RepeatAttackCloser:
		// Closer targets from Team Leader have more priority. Kill until no more targets.
		ScriptExt::Mission_Attack(pTeam, true, 2, -1, -1);
		break;
	case PhobosScripts::RepeatAttackFarther:
		// Farther targets from Team Leader have more priority. Kill until no more targets.
		ScriptExt::Mission_Attack(pTeam, true, 3, -1, -1);
		break;
	case PhobosScripts::SingleAttackCloserThreat:
		// Threats that are close have more priority. 1 kill only (good for xx=49,0 combos)
		ScriptExt::Mission_Attack(pTeam, false, 0, -1, -1);
		break;
	case PhobosScripts::SingleAttackFartherThreat:
		// Threats that are far have more priority. 1 kill only (good for xx=49,0 combos)
		ScriptExt::Mission_Attack(pTeam, false, 1, -1, -1);
		break;
	case PhobosScripts::SingleAttackCloser:
		// Closer targets from Team Leader have more priority. 1 kill only (good for xx=49,0 combos)
		ScriptExt::Mission_Attack(pTeam, false, 2, -1, -1);
		break;
	case PhobosScripts::SingleAttackFarther:
		// Farther targets from Team Leader have more priority. 1 kill only (good for xx=49,0 combos)
		ScriptExt::Mission_Attack(pTeam, false, 3, -1, -1);
		break;
	case PhobosScripts::DecreaseCurrentAITriggerWeight:
		ScriptExt::DecreaseCurrentTriggerWeight(pTeam, true, 0);
		break;
	case PhobosScripts::IncreaseCurrentAITriggerWeight:
		ScriptExt::IncreaseCurrentTriggerWeight(pTeam, true, 0);
		break;
	case PhobosScripts::RepeatAttackTypeCloserThreat:
		// Threats specific targets that are close have more priority. Kill until no more targets.
		ScriptExt::Mission_Attack_List(pTeam, true, 0, -1);
		break;
	case PhobosScripts::RepeatAttackTypeFartherThreat:
		// Threats specific targets that are far have more priority. Kill until no more targets.
		ScriptExt::Mission_Attack_List(pTeam, true, 1, -1);
		break;
	case PhobosScripts::RepeatAttackTypeCloser:
		// Closer specific targets targets from Team Leader have more priority. Kill until no more targets.
		ScriptExt::Mission_Attack_List(pTeam, true, 2, -1);
		break;
	case PhobosScripts::RepeatAttackTypeFarther:
		// Farther specific targets targets from Team Leader have more priority. Kill until no more targets.
		ScriptExt::Mission_Attack_List(pTeam, true, 3, -1);
		break;
	case PhobosScripts::SingleAttackTypeCloserThreat:
		// Threats specific targets that are close have more priority. 1 kill only (good for xx=49,0 combos)
		ScriptExt::Mission_Attack_List(pTeam, false, 0, -1);
		break;
	case PhobosScripts::SingleAttackTypeFartherThreat:
		// Threats specific targets that are far have more priority. 1 kill only (good for xx=49,0 combos)
		ScriptExt::Mission_Attack_List(pTeam, false, 1, -1);
		break;
	case PhobosScripts::SingleAttackTypeCloser:
		// Closer specific targets from Team Leader have more priority. 1 kill only (good for xx=49,0 combos)
		ScriptExt::Mission_Attack_List(pTeam, false, 2, -1);
		break;
	case PhobosScripts::SingleAttackTypeFarther:
		// Farther specific targets from Team Leader have more priority. 1 kill only (good for xx=49,0 combos)
		ScriptExt::Mission_Attack_List(pTeam, false, 3, -1);
		break;
	case PhobosScripts::WaitIfNoTarget:
		ScriptExt::WaitIfNoTarget(pTeam, -1);
		break;
	case PhobosScripts::TeamWeightReward:
		ScriptExt::TeamWeightReward(pTeam, 0);
		break;
	case PhobosScripts::PickRandomScript:
		ScriptExt::PickRandomScript(pTeam, -1);
		break;
	case PhobosScripts::MoveToEnemyCloser:
		// Move to the closest enemy target
		ScriptExt::Mission_Move(pTeam, 2, false, -1, -1);
		break;
	case PhobosScripts::MoveToEnemyFarther:
		// Move to the farther enemy target
		ScriptExt::Mission_Move(pTeam, 3, false, -1, -1);
		break;
	case PhobosScripts::MoveToFriendlyCloser:
		// Move to the closest friendly target
		ScriptExt::Mission_Move(pTeam, 2, true, -1, -1);
		break;
	case PhobosScripts::MoveToFriendlyFarther:
		// Move to the farther friendly target
		ScriptExt::Mission_Move(pTeam, 3, true, -1, -1);
		break;
	case PhobosScripts::MoveToTypeEnemyCloser:
		// Move to the closest specific enemy target
		ScriptExt::Mission_Move_List(pTeam, 2, false, -1);
		break;
	case PhobosScripts::MoveToTypeEnemyFarther:
		// Move to the farther specific enemy target
		ScriptExt::Mission_Move_List(pTeam, 3, false, -1);
	case PhobosScripts::MoveToTypeFriendlyCloser:
		// Move to the closest specific friendly target
		ScriptExt::Mission_Move_List(pTeam, 2, true, -1);
		break;
	case PhobosScripts::MoveToTypeFriendlyFarther:
		// Move to the farther specific friendly target
		ScriptExt::Mission_Move_List(pTeam, 3, true, -1);
		break;
	case PhobosScripts::ModifyTargetDistance:
		// AISafeDistance equivalent for Mission_Move()
		ScriptExt::SetCloseEnoughDistance(pTeam, -1);
		break;
	case PhobosScripts::RandomAttackTypeCloser:
		// Pick 1 closer random objective from specific list for attacking it
		ScriptExt::Mission_Attack_List1Random(pTeam, true, 2, -1);
		break;
	case PhobosScripts::RandomAttackTypeFarther:
		// Pick 1 farther random objective from specific list for attacking it
		ScriptExt::Mission_Attack_List1Random(pTeam, true, 3, -1);
		break;
	case PhobosScripts::RandomMoveToTypeEnemyCloser:
		// Pick 1 closer enemy random objective from specific list for moving to it
		ScriptExt::Mission_Move_List1Random(pTeam, 2, false, -1, -1);
		break;
	case PhobosScripts::RandomMoveToTypeEnemyFarther:
		// Pick 1 farther enemy random objective from specific list for moving to it
		ScriptExt::Mission_Move_List1Random(pTeam, 3, false, -1, -1);
		break;
	case PhobosScripts::RandomMoveToTypeFriendlyCloser:
		// Pick 1 closer friendly random objective from specific list for moving to it
		ScriptExt::Mission_Move_List1Random(pTeam, 2, true, -1, -1);
		break;
	case PhobosScripts::RandomMoveToTypeFriendlyFarther:
		// Pick 1 farther friendly random objective from specific list for moving to it
		ScriptExt::Mission_Move_List1Random(pTeam, 3, true, -1, -1);
		break;
	case PhobosScripts::SetMoveMissionEndMode:
		// Set the condition for ending the Mission_Move Actions.
		ScriptExt::SetMoveMissionEndMode(pTeam, -1);
		break;
	case PhobosScripts::UnregisterGreatSuccess:
		// Un-register success for AITrigger weight adjustment (this is the opposite of 49,0)
		ScriptExt::UnregisterGreatSuccess(pTeam);
		break;
	case PhobosScripts::GatherAroundLeader:
		ScriptExt::Mission_Gather_NearTheLeader(pTeam, -1);
		break;
	case PhobosScripts::RandomSkipNextAction:
		ScriptExt::SkipNextAction(pTeam, -1);
		break;
	case PhobosScripts::SetHouseAngerModifier:
		ScriptExt::SetHouseAngerModifier(pTeam, 0);
		break;
	case PhobosScripts::OverrideOnlyTargetHouseEnemy:
		ScriptExt::OverrideOnlyTargetHouseEnemy(pTeam, -1);
		break;
	case PhobosScripts::ModifyHateHouseIndex:
		ScriptExt::ModifyHateHouse_Index(pTeam, -1);
		break;
	case PhobosScripts::ModifyHateHousesList:
		ScriptExt::ModifyHateHouses_List(pTeam, -1);
		break;
	case PhobosScripts::ModifyHateHousesList1Random:
		ScriptExt::ModifyHateHouses_List1Random(pTeam, -1);
		break;
	case PhobosScripts::SetTheMostHatedHouseMinorNoRandom:
		// <, no random
		ScriptExt::SetTheMostHatedHouse(pTeam, 0, 0, false);
		break;
	case PhobosScripts::SetTheMostHatedHouseMajorNoRandom:
		// >, no random
		ScriptExt::SetTheMostHatedHouse(pTeam, 0, 1, false);
		break;
	case PhobosScripts::SetTheMostHatedHouseRandom:
		// random
		ScriptExt::SetTheMostHatedHouse(pTeam, 0, 0, true);
		break;
	case PhobosScripts::ResetAngerAgainstHouses:
		ScriptExt::ResetAngerAgainstHouses(pTeam);
		break;
	case PhobosScripts::AggroHouse:
		ScriptExt::AggroHouse(pTeam, -1);
		break;
	case PhobosScripts::StopForceJumpCountdown:
		// Stop Timed Jump
		ScriptExt::Stop_ForceJump_Countdown(pTeam);
		break;
	case PhobosScripts::NextLineForceJumpCountdown:
		// Start Timed Jump that jumps to the next line when the countdown finish (in frames)
		ScriptExt::Set_ForceJump_Countdown(pTeam, false, -1);
		break;
	case PhobosScripts::SameLineForceJumpCountdown:
		// Start Timed Jump that jumps to the same line when the countdown finish (in frames)
		ScriptExt::Set_ForceJump_Countdown(pTeam, true, -1);
		break;
	case PhobosScripts::ForceGlobalOnlyTargetHouseEnemy:
		ScriptExt::ForceGlobalOnlyTargetHouseEnemy(pTeam, -1);
		break;
	case PhobosScripts::SetSideIdxForManagingTriggers:
		ScriptExt::SetSideIdxForManagingTriggers(pTeam, -1);
		break;
	case PhobosScripts::SetHouseIdxForManagingTriggers:
		ScriptExt::SetHouseIdxForManagingTriggers(pTeam, 1000000);
		break;
	case PhobosScripts::ManageAllAITriggers:
		ScriptExt::ManageAITriggers(pTeam, -1);
		break;
	case PhobosScripts::EnableTriggersFromList:
		ScriptExt::ManageTriggersFromList(pTeam, -1, true);
		break;
	case PhobosScripts::DisableTriggersFromList:
		ScriptExt::ManageTriggersFromList(pTeam, -1, false);
		break;
	case PhobosScripts::EnableTriggersWithObjects:
		ScriptExt::ManageTriggersWithObjects(pTeam, -1, true);
		break;
	case PhobosScripts::DisableTriggersWithObjects:
		ScriptExt::ManageTriggersWithObjects(pTeam, -1, false);
		break;
	case PhobosScripts::AbortActionAfterSuccessKill:
		ScriptExt::SetAbortActionAfterSuccessKill(pTeam, -1);
		break;
	case PhobosScripts::ConditionalJumpSetCounter:
		ScriptExt::ConditionalJump_SetCounter(pTeam, -100000000);
		break;
	case PhobosScripts::ConditionalJumpSetComparatorMode:
		ScriptExt::ConditionalJump_SetComparatorMode(pTeam, -1);
		break;
	case PhobosScripts::ConditionalJumpSetComparatorValue:
		ScriptExt::ConditionalJump_SetComparatorValue(pTeam, -1);
		break;
	case PhobosScripts::ConditionalJumpSetIndex:
		ScriptExt::ConditionalJump_SetIndex(pTeam, -1000000);
		break;
	case PhobosScripts::ConditionalJumpResetVariables:
		ScriptExt::ConditionalJump_ResetVariables(pTeam);
		break;
	case PhobosScripts::ConditionalJumpIfFalse:
		ScriptExt::ConditionalJumpIfFalse(pTeam, -1);
		break;
	case PhobosScripts::ConditionalJumpIfTrue:
		ScriptExt::ConditionalJumpIfTrue(pTeam, -1);
		break;
	case PhobosScripts::ConditionalJumpManageKillsCounter:
		ScriptExt::ConditionalJump_ManageKillsCounter(pTeam, -1);
		break;
	case PhobosScripts::ConditionalJumpCheckAliveHumans:
		ScriptExt::ConditionalJump_CheckAliveHumans(pTeam, -1);
		break;
	case PhobosScripts::ConditionalJumpCheckHumanIsMostHated:
		ScriptExt::ConditionalJump_CheckHumanIsMostHated(pTeam);
		break;
	case PhobosScripts::ConditionalJumpKillEvaluation:
		ScriptExt::ConditionalJump_KillEvaluation(pTeam);
		break;
	case PhobosScripts::ConditionalJumpCheckObjects:
		ScriptExt::ConditionalJump_CheckObjects(pTeam);
		break;
	case PhobosScripts::ConditionalJumpCheckCount:
		ScriptExt::ConditionalJump_CheckCount(pTeam, 0);
		break;
	case PhobosScripts::ConditionalJumpManageResetIfJump:
		ScriptExt::ConditionalJump_ManageResetIfJump(pTeam, -1);
		break;
	case PhobosScripts::JumpBackToPreviousScript:
		ScriptExt::JumpBackToPreviousScript(pTeam);
		break;
	case PhobosScripts::UnloadFromTransports:
		ScriptExt::UnloadFromTransports(pTeam);
		break;
	default:
		// Do nothing because or it is a wrong Action number or it is an Ares/YR action...
		if (action > 70 && !IsExtVariableAction(action))
		{
			// Unknown new action. This action finished
			pTeam->StepCompleted = true;
			Debug::Log("[%s] [%s] (line %d): Unknown Script Action: %d\n", pTeam->Type->ID, pTeam->CurrentScript->Type->ID, pTeam->CurrentScript->CurrentMission, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Action);
		}
		break;
	}

	if (IsExtVariableAction(action))
		VariablesHandler(pTeam, static_cast<PhobosScripts>(action), argument);
}

void ScriptExt::ExecuteTimedAreaGuardAction(TeamClass* pTeam)
{
	auto pScript = pTeam->CurrentScript;
	auto pScriptType = pScript->Type;

	if (pTeam->GuardAreaTimer.TimeLeft == 0 && !pTeam->GuardAreaTimer.InProgress())
	{
		for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
			pUnit->QueueMission(Mission::Area_Guard, true);

		pTeam->GuardAreaTimer.Start(15 * pScriptType->ScriptActions[pScript->CurrentMission].Argument);
	}

	if (pTeam->GuardAreaTimer.Completed())
	{
		pTeam->GuardAreaTimer.Stop(); // Needed
		pTeam->StepCompleted = true;
	}
}

void ScriptExt::LoadIntoTransports(TeamClass* pTeam)
{
	DynamicVectorClass<FootClass*> transports;

	// Collect available transports
	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
	{
		auto const pType = pUnit->GetTechnoType();

		if (pType->Passengers > 0)
			if (pUnit->Passengers.NumPassengers < pType->Passengers)
				if (pUnit->Passengers.GetTotalSize() < pType->Passengers)
					transports.AddItem(pUnit);
	}

	// Now load units into transports
	for (auto pTransport : transports)
	{
		for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
		{
			auto const pTransportType = pTransport->GetTechnoType();
			auto const pUnitType = pUnit->GetTechnoType();
			if (pTransport != pUnit
				&& pUnitType->WhatAmI() != AbstractType::AircraftType
				&& !pUnit->InLimbo
				&& !pUnitType->ConsideredAircraft
				&& pUnit->Health > 0)
			{
				if (pUnit->GetTechnoType()->Size > 0
					&& pUnitType->Size <= pTransportType->SizeLimit
					&& pUnitType->Size <= pTransportType->Passengers - pTransport->Passengers.GetTotalSize())
				{
					// All fine
					if (pUnit->GetCurrentMission() != Mission::Enter)
					{
						pUnit->QueueMission(Mission::Enter, false);
						pUnit->SetTarget(nullptr);
						pUnit->SetDestination(pTransport, true);

						return;
					}
				}
			}
		}
	}

	// Is loading
	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
		if (pUnit->GetCurrentMission() == Mission::Enter)
			return;

	auto pExt = TeamExt::ExtMap.Find(pTeam);
	if (pExt)
	{
		FootClass* pLeaderUnit = FindTheTeamLeader(pTeam);
		pExt->TeamLeader = pLeaderUnit;
	}

	// This action finished
	if (pTeam->CurrentScript->HasNextMission())
		++pTeam->CurrentScript->CurrentMission;

	pTeam->StepCompleted = true;
}

void ScriptExt::WaitUntilFullAmmoAction(TeamClass* pTeam)
{
	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
	{
		if (!pUnit->InLimbo && pUnit->Health > 0)
		{
			if (pUnit->GetTechnoType()->Ammo > 0 && pUnit->Ammo < pUnit->GetTechnoType()->Ammo)
			{
				// If an aircraft object have AirportBound it must be evaluated
				if (auto pAircraft = abstract_cast<AircraftClass*>(pUnit))
				{
					if (pAircraft->Type->AirportBound)
					{
						// Reset last target, at long term battles this prevented the aircraft to pick a new target (rare vanilla YR bug)
						pUnit->SetTarget(nullptr);
						pUnit->LastTarget = nullptr;
						// Fix YR bug (when returns from the last attack the aircraft switch in loop between Mission::Enter & Mission::Guard, making it impossible to land in the dock)
						if (pUnit->IsInAir() && pUnit->CurrentMission != Mission::Enter)
							pUnit->QueueMission(Mission::Enter, true);

						return;
					}
				}
				else if (pUnit->GetTechnoType()->Reload != 0) // Don't skip units that can reload themselves
					return;
			}
		}
	}

	pTeam->StepCompleted = true;
}

void ScriptExt::Mission_Gather_NearTheLeader(TeamClass* pTeam, int countdown = -1)
{
	FootClass* pLeaderUnit = nullptr;
	int initialCountdown = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument;
	bool gatherUnits = false;
	auto pExt = TeamExt::ExtMap.Find(pTeam);

	// This team has no units! END
	if (!pTeam)
	{
		// This action finished
		pTeam->StepCompleted = true;
		return;
	}

	// Load countdown
	if (pExt->Countdown_RegroupAtLeader >= 0)
		countdown = pExt->Countdown_RegroupAtLeader;

	// Gather permanently until all the team members are near of the Leader
	if (initialCountdown == 0)
		gatherUnits = true;

	// Countdown updater
	if (initialCountdown > 0)
	{
		if (countdown > 0)
		{
			countdown--; // Update countdown
			gatherUnits = true;
		}
		else if (countdown == 0) // Countdown ended
			countdown = -1;
		else // Start countdown.
		{
			countdown = initialCountdown * 15;
			gatherUnits = true;
		}

		// Save counter
		pExt->Countdown_RegroupAtLeader = countdown;
	}

	if (!gatherUnits)
	{
		// This action finished
		pTeam->StepCompleted = true;
		return;
	}
	else
	{
		// Move all around the leader, the leader always in "Guard Area" Mission or simply in Guard Mission
		int nTogether = 0;
		int nUnits = -1; // Leader counts here
		double closeEnough;

		// Find the Leader
		pLeaderUnit = pExt->TeamLeader;
		if (!pLeaderUnit
			|| !pLeaderUnit->IsAlive
			|| pLeaderUnit->Health <= 0
			|| pLeaderUnit->InLimbo
			|| !(pLeaderUnit->IsOnMap || (pLeaderUnit->GetTechnoType()->IsSubterranean))
			|| pLeaderUnit->Transporter
			|| pLeaderUnit->Absorbed)
		{
			pLeaderUnit = FindTheTeamLeader(pTeam);
			pExt->TeamLeader = pLeaderUnit;
		}

		if (!pLeaderUnit)
		{
			pExt->Countdown_RegroupAtLeader = -1;
			// This action finished
			pTeam->StepCompleted = true;

			return;
		}

		// Leader's area radius where the Team members are considered "near" to the Leader
		if (pExt->CloseEnough > 0)
		{
			closeEnough = pExt->CloseEnough;
			pExt->CloseEnough = -1; // This a one-time-use value
		}
		else
		{
			closeEnough = RulesClass::Instance->CloseEnough / 256.0;
		}

		// The leader should stay calm & be the group's center
		if (pLeaderUnit->Locomotor->Is_Moving_Now())
			pLeaderUnit->SetDestination(nullptr, false);

		pLeaderUnit->QueueMission(Mission::Guard, false);

		// Check if units are around the leader
		for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
		{
			if (pUnit
				&& pUnit->IsAlive
				&& pUnit->Health > 0
				&& !pUnit->InLimbo
				&& pUnit->IsOnMap
				&& !pUnit->Absorbed)
			{
				auto pTypeUnit = pUnit->GetTechnoType();

				if (!pTypeUnit)
					continue;

				if (pUnit == pLeaderUnit)
				{
					nUnits++;
					continue;
				}

				// Aircraft case
				if (pTypeUnit->WhatAmI() == AbstractType::AircraftType && pUnit->Ammo <= 0 && pTypeUnit->Ammo > 0)
				{
					auto pAircraft = static_cast<AircraftTypeClass*>(pUnit->GetTechnoType());

					if (pAircraft->AirportBound)
					{
						// This aircraft won't count for the script action
						pUnit->QueueMission(Mission::Return, false);
						pUnit->Mission_Enter();

						continue;
					}
				}

				nUnits++;

				if ((pUnit->DistanceFrom(pLeaderUnit) / 256.0) > closeEnough)
				{
					// Leader's location is too far from me. Regroup
					if (pUnit->Destination != pLeaderUnit)
					{
						pUnit->SetDestination(pLeaderUnit, false);
						pUnit->QueueMission(Mission::Move, false);
					}
				}
				else
				{
					// Is near of the leader, then protect the area
					if (pUnit->GetCurrentMission() != Mission::Area_Guard || pUnit->GetCurrentMission() != Mission::Attack)
						pUnit->QueueMission(Mission::Area_Guard, true);

					nTogether++;
				}
			}
		}


		if (nUnits >= 0
			&& nUnits == nTogether
			&& (initialCountdown == 0
				|| (initialCountdown > 0
					&& countdown <= 0)))
		{
			pExt->Countdown_RegroupAtLeader = -1;
			// This action finished
			pTeam->StepCompleted = true;

			return;
		}
	}
}

void ScriptExt::Mission_Attack(TeamClass* pTeam, bool repeatAction = true, int calcThreatMode = 0, int attackAITargetType = -1, int idxAITargetTypeItem = -1)
{
	auto pScript = pTeam->CurrentScript;
	int scriptArgument = pScript->Type->ScriptActions[pScript->CurrentMission].Argument; // This is the target type
	TechnoClass* selectedTarget = nullptr;
	HouseClass* enemyHouse = nullptr;
	bool noWaitLoop = false;
	FootClass* pLeaderUnit = nullptr;
	TechnoTypeClass* pLeaderUnitType = nullptr;
	bool bAircraftsWithoutAmmo = false;
	TechnoClass* pFocus = nullptr;
	bool agentMode = false;
	bool pacifistTeam = true;
	auto pTeamData = TeamExt::ExtMap.Find(pTeam);

	if (!pScript)
		return;

	if (!pTeamData)
	{
		pTeam->StepCompleted = true;
		Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d) Jump to next line: %d = %d,%d -> (Reason: ExtData found)\n", pTeam->Type->ID, pScript->CurrentMission, pScript->Type->ScriptActions[pScript->CurrentMission].Action, pScript->Type->ScriptActions[pScript->CurrentMission].Argument, pScript->Type->ID, pScript->CurrentMission + 1, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Action, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Argument);

		return;
	}

	auto pHouseExt = HouseExt::ExtMap.Find(pTeam->Owner);
	if (!pHouseExt)
	{
		// This action finished
		pTeam->StepCompleted = true;
		return;
	}

	// When the new target wasn't found it sleeps some few frames before the new attempt. This can save cycles and cycles of unnecessary executed lines.
	if (pTeamData->WaitNoTargetCounter > 0)
	{
		if (pTeamData->WaitNoTargetTimer.InProgress())
			return;

		pTeamData->WaitNoTargetTimer.Stop();
		noWaitLoop = true;
		pTeamData->WaitNoTargetCounter = 0;

		if (pTeamData->WaitNoTargetAttempts > 0)
			pTeamData->WaitNoTargetAttempts--;
	}

	// This team has no units!
	if (!pTeam)
	{
		if (pTeamData->CloseEnough > 0)
			pTeamData->CloseEnough = -1;

		// This action finished
		pTeam->StepCompleted = true;
		Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d) Jump to next line: %d = %d,%d -> (Reason: No team members alive)\n", pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission, pScript->Type->ScriptActions[pScript->CurrentMission].Action, pScript->Type->ScriptActions[pScript->CurrentMission].Argument, pScript->CurrentMission + 1, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Action, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Argument);

		return;
	}

	pFocus = abstract_cast<TechnoClass*>(pTeam->Focus);
	if (!pFocus
		|| !pFocus->IsAlive
		|| pFocus->Health <= 0
		|| pFocus->InLimbo
		|| !pFocus->IsOnMap
		|| pFocus->Absorbed)
	{
		pTeam->Focus = nullptr;
		pFocus = nullptr;
	}

	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
	{
		auto pKillerTechnoData = TechnoExt::ExtMap.Find(pUnit);
		if (pKillerTechnoData && pKillerTechnoData->LastKillWasTeamTarget)
		{
			// Time for Team award check! (if set any)
			if (pTeamData->NextSuccessWeightAward > 0)
			{
				IncreaseCurrentTriggerWeight(pTeam, false, pTeamData->NextSuccessWeightAward);
				pTeamData->NextSuccessWeightAward = 0;
			}

			// Let's clean the Killer mess
			pKillerTechnoData->LastKillWasTeamTarget = false;
			pFocus = nullptr;
			pTeam->Focus = nullptr;

			if (!repeatAction)
			{
				// If the previous Team's Target was killed by this Team Member and the script was a 1-time-use then this script action must be finished.
				for (auto pTeamUnit = pTeam->FirstUnit; pTeamUnit; pTeamUnit = pTeamUnit->NextTeamMember)
				{
					// Let's reset all Team Members objective
					auto pKillerTeamUnitData = TechnoExt::ExtMap.Find(pTeamUnit);
					pKillerTeamUnitData->LastKillWasTeamTarget = false;

					if (pTeamUnit->GetTechnoType()->WhatAmI() == AbstractType::AircraftType)
					{
						pTeamUnit->SetTarget(nullptr);
						pTeamUnit->LastTarget = nullptr;
						pTeamUnit->SetFocus(nullptr); // Lets see if this works or bug my little aircrafts
						pTeamUnit->CurrentTargets.Clear(); // Lets see if this works or bug my little aircrafts
						pTeamUnit->QueueMission(Mission::Guard, true);
					}
				}

				pTeamData->IdxSelectedObjectFromAIList = -1;

				// This action finished
				pTeam->StepCompleted = true;
				Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d) Force the jump to next line: %d = %d,%d (This action wont repeat)\n", pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission, pScript->Type->ScriptActions[pScript->CurrentMission].Action, pScript->Type->ScriptActions[pScript->CurrentMission].Argument, pScript->CurrentMission + 1, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Action, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Argument);

				return;
			}
		}
	}

	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
	{
		if (pUnit && pUnit->IsAlive && !pUnit->InLimbo)
		{
			auto pUnitType = pUnit->GetTechnoType();
			if (pUnitType)
			{
				if (pUnitType->WhatAmI() == AbstractType::AircraftType
					&& !pUnit->IsInAir()
					&& abstract_cast<AircraftTypeClass*>(pUnitType)->AirportBound
					&& pUnit->Ammo < pUnitType->Ammo)
				{
					bAircraftsWithoutAmmo = true;
					pUnit->CurrentTargets.Clear();
				}

				bool pacifistUnit = true;
				if (pUnit->Veterancy.IsElite())
				{
					if (pUnitType->EliteWeapon[0].WeaponType || pUnitType->EliteWeapon[1].WeaponType
						|| (pUnitType->IsGattling && pUnitType->EliteWeapon[pUnit->CurrentWeaponNumber].WeaponType))
					{
						pacifistTeam = false;
						pacifistUnit = false;
					}
				}
				else
				{
					if (pUnitType->Weapon[0].WeaponType || pUnitType->Weapon[1].WeaponType
						|| (pUnitType->IsGattling && pUnitType->Weapon[pUnit->CurrentWeaponNumber].WeaponType))
					{
						pacifistTeam = false;
						pacifistUnit = false;
					}
				}

				// Any Team member (infantry) is a special agent? If yes ignore some checks based on Weapons.
				if (pUnitType->WhatAmI() == AbstractType::InfantryType)
				{
					auto pTypeInf = abstract_cast<InfantryTypeClass*>(pUnitType);
					if ((pTypeInf->Agent && pTypeInf->Infiltrate) || pTypeInf->Engineer)
					{
						agentMode = true;
					}
				}
			}
		}
	}

	// Find the Leader
	pLeaderUnit = pTeamData->TeamLeader;
	if (!pLeaderUnit
		|| !pLeaderUnit->IsAlive
		|| pLeaderUnit->Health <= 0
		|| pLeaderUnit->InLimbo
		|| !(pLeaderUnit->IsOnMap || (pLeaderUnit->GetTechnoType()->IsSubterranean))
		|| pLeaderUnit->Transporter
		|| pLeaderUnit->Absorbed)
	{
		pLeaderUnit = FindTheTeamLeader(pTeam);
		pTeamData->TeamLeader = pLeaderUnit;
	}

	if (!pLeaderUnit || bAircraftsWithoutAmmo || (pacifistTeam && !agentMode))
	{
		pTeamData->IdxSelectedObjectFromAIList = -1;
		if (pTeamData->WaitNoTargetAttempts != 0)
		{
			pTeamData->WaitNoTargetTimer.Stop();
			pTeamData->WaitNoTargetCounter = 0;
			pTeamData->WaitNoTargetAttempts = 0;
		}

		// This action finished
		pTeam->StepCompleted = true;
		Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d) Jump to next line: %d = %d,%d -> (Reason: No Leader found | Exists Aircrafts without ammo | Team members have no weapons)\n", pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission, pScript->Type->ScriptActions[pScript->CurrentMission].Action, pScript->Type->ScriptActions[pScript->CurrentMission].Argument, pScript->CurrentMission + 1, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Action, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Argument);

		return;
	}

	pLeaderUnitType = pLeaderUnit->GetTechnoType();
	bool leaderWeaponsHaveAA = false;
	bool leaderWeaponsHaveAG = false;
	// Note: Replace these lines when I have access to Combat_Damage() method in YRpp if that is better
	WeaponTypeClass* WeaponType1 = pLeaderUnit->Veterancy.IsElite() ?
		pLeaderUnitType->EliteWeapon[0].WeaponType :
		pLeaderUnitType->Weapon[0].WeaponType;

	WeaponTypeClass* WeaponType2 = pLeaderUnit->Veterancy.IsElite() ?
		pLeaderUnitType->EliteWeapon[1].WeaponType :
		pLeaderUnitType->Weapon[1].WeaponType;

	WeaponTypeClass* WeaponType3 = WeaponType1;

	if (pLeaderUnitType->IsGattling)
	{
		WeaponType3 = pLeaderUnit->Veterancy.IsElite() ?
			pLeaderUnitType->EliteWeapon[pLeaderUnit->CurrentWeaponNumber].WeaponType :
			pLeaderUnitType->Weapon[pLeaderUnit->CurrentWeaponNumber].WeaponType;

		WeaponType1 = WeaponType3;
	}

	// Weapon check used for filtering Leader targets.
	// Note: the Team Leader is picked for this task, be careful with leadership rating values in your mod
	if ((WeaponType1 && WeaponType1->Projectile->AA) || (WeaponType2 && WeaponType2->Projectile->AA))
		leaderWeaponsHaveAA = true;

	if ((WeaponType1 && WeaponType1->Projectile->AG) || (WeaponType2 && WeaponType2->Projectile->AG) || agentMode)
		leaderWeaponsHaveAG = true;

	// Special case: a Leader with OpenTopped tag
	if (pLeaderUnitType->OpenTopped && pLeaderUnit->Passengers.NumPassengers > 0)
	{
		for (NextObject j(pLeaderUnit->Passengers.FirstPassenger->NextObject); j && abstract_cast<FootClass*>(*j); ++j)
		{
			auto passenger = static_cast<FootClass*>(*j);
			auto pPassengerType = passenger->GetTechnoType();

			if (pPassengerType)
			{
				// Note: Replace these lines when I have access to Combat_Damage() method in YRpp if that is better
				WeaponTypeClass* passengerWeaponType1 = passenger->Veterancy.IsElite() ?
					pPassengerType->EliteWeapon[0].WeaponType :
					pPassengerType->Weapon[0].WeaponType;

				WeaponTypeClass* passengerWeaponType2 = passenger->Veterancy.IsElite() ?
					pPassengerType->EliteWeapon[1].WeaponType :
					pPassengerType->Weapon[1].WeaponType;

				if (pPassengerType->IsGattling)
				{
					WeaponTypeClass* passengerWeaponType3 = passenger->Veterancy.IsElite() ?
						pPassengerType->EliteWeapon[passenger->CurrentWeaponNumber].WeaponType :
						pPassengerType->Weapon[passenger->CurrentWeaponNumber].WeaponType;

					passengerWeaponType1 = passengerWeaponType3;
				}

				// Used for filtering targets.
				// Note: the units inside a openTopped Leader are used for this task
				if ((passengerWeaponType1 && passengerWeaponType1->Projectile->AA)
					|| (passengerWeaponType2 && passengerWeaponType2->Projectile->AA))
				{
					leaderWeaponsHaveAA = true;
				}

				if ((passengerWeaponType1 && passengerWeaponType1->Projectile->AG)
					|| (passengerWeaponType2 && passengerWeaponType2->Projectile->AG))
				{
					leaderWeaponsHaveAG = true;
				}
			}
		}
	}

	bool onlyTargetHouseEnemy = pTeam->Type->OnlyTargetHouseEnemy;

	if (pTeamData->OnlyTargetHouseEnemyMode != -1)
	{
		onlyTargetHouseEnemy = pTeamData->OnlyTargetHouseEnemy;
	}

	if (pHouseExt->ForceOnlyTargetHouseEnemyMode != -1)
		onlyTargetHouseEnemy = pHouseExt->ForceOnlyTargetHouseEnemy;

	if (!pFocus && !bAircraftsWithoutAmmo)
	{
		// This part of the code is used for picking a new target.

		// Favorite Enemy House case. If set, AI will focus against that House
		if (onlyTargetHouseEnemy && pLeaderUnit->Owner->EnemyHouseIndex >= 0)
			enemyHouse = HouseClass::Array->GetItem(pLeaderUnit->Owner->EnemyHouseIndex);

		int targetMask = scriptArgument;
		selectedTarget = GreatestThreat(pLeaderUnit, targetMask, calcThreatMode, enemyHouse, attackAITargetType, idxAITargetTypeItem, agentMode);

		if (selectedTarget)
		{
			Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d) Leader [%s] (UID: %lu) selected [%s] (UID: %lu) as target.\n", pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission, pScript->Type->ScriptActions[pScript->CurrentMission].Action, pScript->Type->ScriptActions[pScript->CurrentMission].Argument, pLeaderUnit->GetTechnoType()->get_ID(), pLeaderUnit->UniqueID, selectedTarget->GetTechnoType()->get_ID(), selectedTarget->UniqueID);

			pTeam->Focus = selectedTarget;
			pTeamData->WaitNoTargetAttempts = 0; // Disable Script Waits if there are any because a new target was selected
			pTeamData->WaitNoTargetTimer.Stop();
			pTeamData->WaitNoTargetCounter = 0; // Disable Script Waits if there are any because a new target was selected

			for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
			{
				if (pUnit->IsAlive && !pUnit->InLimbo)
				{
					auto pUnitType = pUnit->GetTechnoType();
					if (pUnitType && pUnit != selectedTarget && pUnit->Target != selectedTarget)
					{
						pUnit->CurrentTargets.Clear();
						if (pUnitType->Underwater && pUnitType->LandTargeting == LandTargetingType::Land_Not_OK
							&& selectedTarget->GetCell()->LandType != LandType::Water) // Land not OK for the Naval unit
						{
							// Naval units like Submarines are unable to target ground targets
							// except if they have anti-ground weapons. Ignore the attack
							pUnit->CurrentTargets.Clear();
							pUnit->SetTarget(nullptr);
							pUnit->SetFocus(nullptr);
							pUnit->SetDestination(nullptr, false);
							pUnit->QueueMission(Mission::Area_Guard, true);

							continue;
						}

						// Aircraft hack. I hate how this game auto-manages the aircraft missions.
						if (pUnitType->WhatAmI() == AbstractType::AircraftType
							&& pUnit->Ammo > 0 && pUnit->GetHeight() <= 0)
						{
							pUnit->SetDestination(selectedTarget, false);
							pUnit->QueueMission(Mission::Attack, true);
						}

						pUnit->SetTarget(selectedTarget);

						if (pUnit->IsEngineer())
						{
							pUnit->QueueMission(Mission::Capture, true);
						}
						else
						{
							// Aircraft hack. I hate how this game auto-manages the aircraft missions.
							if (pUnitType->WhatAmI() != AbstractType::AircraftType)
							{
								pUnit->QueueMission(Mission::Attack, true);
								pUnit->ObjectClickedAction(Action::Attack, selectedTarget, false);

								if (pUnit->GetCurrentMission() != Mission::Attack)
									pUnit->Mission_Attack();

								if (pUnit->GetCurrentMission() == Mission::Move && pUnitType->JumpJet)
									pUnit->Mission_Attack();
							}
						}

						// Spy case
						if (pUnitType->WhatAmI() == AbstractType::InfantryType)
						{
							auto pInfantryType = abstract_cast<InfantryTypeClass*>(pUnitType);

							if (pInfantryType && pInfantryType->Infiltrate && pInfantryType->Agent)
							{
								// Check if target is an structure and see if spiable
								if (pUnit->GetCurrentMission() != Mission::Enter)
									pUnit->Mission_Enter();
							}
						}

						// Tanya / Commando C4 case
						if ((pUnitType->WhatAmI() == AbstractType::InfantryType && (abstract_cast<InfantryTypeClass*>(pUnitType)->C4 || pUnit->HasAbility(Ability::C4))) && pUnit->GetCurrentMission() != Mission::Sabotage)
						{
							pUnit->Mission_Attack();
							pUnit->QueueMission(Mission::Sabotage, true);
						}
					}
					else
					{
						pUnit->QueueMission(Mission::Attack, true);
						pUnit->ObjectClickedAction(Action::Attack, selectedTarget, false);
						pUnit->Mission_Attack();
					}
				}
			}
		}
		else
		{
			// No target was found with the specific criteria.
			if (!noWaitLoop)
			{
				pTeamData->WaitNoTargetCounter = 30;
				pTeamData->WaitNoTargetTimer.Start(30);
			}

			if (pTeamData->IdxSelectedObjectFromAIList >= 0)
				pTeamData->IdxSelectedObjectFromAIList = -1;

			if (pTeamData->WaitNoTargetAttempts != 0)
			{
				// No target? let's wait some frames
				pTeamData->WaitNoTargetCounter = 30;
				pTeamData->WaitNoTargetTimer.Start(30);

				return;
			}

			// This action finished
			pTeam->StepCompleted = true;
			Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d) Jump to next line: %d = %d,%d (Leader [%s] (UID: %lu) can't find a new target)\n", pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission, pScript->Type->ScriptActions[pScript->CurrentMission].Action, pScript->Type->ScriptActions[pScript->CurrentMission].Argument, pScript->CurrentMission + 1, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Action, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Argument, pLeaderUnit->GetTechnoType()->get_ID(), pLeaderUnit->UniqueID);

			return;
		}
	}
	else
	{
		// This part of the code is used for updating the "Attack" mission in each team unit

		if (pFocus
			&& pFocus->IsAlive
			&& pFocus->Health > 0
			&& !pFocus->InLimbo
			&& !pFocus->GetTechnoType()->Immune
			&& ((pFocus->IsInAir() && leaderWeaponsHaveAA)
				|| (!pFocus->IsInAir() && leaderWeaponsHaveAG))
			&& pFocus->IsOnMap
			&& !pFocus->Absorbed
			&& pFocus->Owner != pLeaderUnit->Owner
			&& (!pLeaderUnit->Owner->IsAlliedWith(pFocus)
				|| (pLeaderUnit->Owner->IsAlliedWith(pFocus)
					&& pFocus->IsMindControlled()
					&& !pLeaderUnit->Owner->IsAlliedWith(pFocus->MindControlledBy))))
		{

			bool bForceNextAction = false;

			for (auto pUnit = pTeam->FirstUnit; pUnit && !bForceNextAction; pUnit = pUnit->NextTeamMember)
			{
				auto pUnitType = pUnit->GetTechnoType();

				if (pUnitType
					&& pUnit->IsAlive
					&& pUnit->Health > 0
					&& !pUnit->InLimbo)
				{
					// Aircraft case 1
					if ((pUnitType->WhatAmI() == AbstractType::AircraftType
						&& abstract_cast<AircraftTypeClass*>(pUnitType)->AirportBound)
						&& pUnit->Ammo > 0
						&& (pUnit->Target != pFocus && !pUnit->InAir))
					{
						pUnit->SetTarget(pFocus);
						continue;
					}

					// Naval units like Submarines are unable to target ground targets except if they have nti-ground weapons. Ignore the attack
					if (pUnitType->Underwater
						&& pUnitType->LandTargeting == LandTargetingType::Land_Not_OK
						&& pFocus->GetCell()->LandType != LandType::Water) // Land not OK for the Naval unit
					{
						pUnit->CurrentTargets.Clear();
						pUnit->SetTarget(nullptr);
						pUnit->SetFocus(nullptr);
						pUnit->SetDestination(nullptr, false);
						pUnit->QueueMission(Mission::Area_Guard, true);
						bForceNextAction = true;

						continue;
					}

					// Aircraft case 2
					if (pUnitType->WhatAmI() == AbstractType::AircraftType
						&& pUnit->GetCurrentMission() != Mission::Attack
						&& pUnit->GetCurrentMission() != Mission::Enter)
					{
						if (pUnit->InAir)
						{
							if (pUnit->Ammo > 0)
							{
								pUnit->QueueMission(Mission::Attack, true);

								if (pFocus)
									pUnit->ObjectClickedAction(Action::Attack, pFocus, false);

								pUnit->Mission_Attack();
							}
							else
							{
								pUnit->ForceMission(Mission::Enter);
								pUnit->Mission_Enter();
								pUnit->SetFocus(pUnit);
								pUnit->LastTarget = nullptr;
								pUnit->SetTarget(pUnit);
							}
						}
						else
						{
							if (pUnit->Ammo > 0)
							{
								pUnit->QueueMission(Mission::Attack, true);

								if (pFocus)
									pUnit->ObjectClickedAction(Action::Attack, pFocus, false);

								pUnit->Mission_Attack();
							}
							else
							{
								pUnit->ForceMission(Mission::Enter);
								pUnit->Mission_Enter();
								pUnit->SetFocus(pUnit);
								pUnit->LastTarget = nullptr;
								pUnit->SetTarget(pUnit);
							}
						}

						continue;
					}

					// Tanya / Commando C4 case
					if ((pUnitType->WhatAmI() == AbstractType::InfantryType
						&& abstract_cast<InfantryTypeClass*>(pUnitType)->C4
						|| pUnit->HasAbility(Ability::C4)) && pUnit->GetCurrentMission() != Mission::Sabotage)
					{
						pUnit->Mission_Attack();
						pUnit->QueueMission(Mission::Sabotage, true);

						continue;
					}

					// Other cases
					if (pUnitType->WhatAmI() != AbstractType::AircraftType)
					{
						if (pUnit->Target != pFocus)
							pUnit->SetTarget(pFocus);

						if (pUnit->GetCurrentMission() != Mission::Attack
							&& pUnit->GetCurrentMission() != Mission::Unload
							&& pUnit->GetCurrentMission() != Mission::Selling)
						{
							pUnit->QueueMission(Mission::Attack, false);
						}

						continue;
					}
				}
			}

			if (bForceNextAction)
			{
				pTeamData->IdxSelectedObjectFromAIList = -1;
				pTeam->StepCompleted = true;
				Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d) Jump to NEXT line: %d = %d,%d (Naval is unable to target ground)\n", pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission, pScript->Type->ScriptActions[pScript->CurrentMission].Action, pScript->Type->ScriptActions[pScript->CurrentMission].Argument, pScript->CurrentMission + 1, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Action, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Argument);

				return;
			}
		}
		else
		{
			pTeam->Focus = nullptr;
		}
	}
}

TechnoClass* ScriptExt::GreatestThreat(TechnoClass* pTechno, int method, int calcThreatMode = 0, HouseClass * onlyTargetThisHouseEnemy = nullptr, int attackAITargetType = -1, int idxAITargetTypeItem = -1, bool agentMode = false)
{
	TechnoClass* bestObject = nullptr;
	double bestVal = -1;
	bool unitWeaponsHaveAA = false;
	bool unitWeaponsHaveAG = false;

	// Generic method for targeting
	for (int i = 0; i < TechnoClass::Array->Count; i++)
	{
		auto object = TechnoClass::Array->GetItem(i);
		auto objectType = object->GetTechnoType();
		auto pTechnoType = pTechno->GetTechnoType();

		if (!object || !objectType || !pTechnoType)
			continue;

		// Note: the TEAM LEADER is picked for this task, be careful with leadership values in your mod
		int weaponIndex = pTechno->SelectWeapon(object);
		auto weaponType = pTechno->GetWeapon(weaponIndex)->WeaponType;

		if (weaponType && weaponType->Projectile->AA)
			unitWeaponsHaveAA = true;

		if ((weaponType && weaponType->Projectile->AG) || agentMode)
			unitWeaponsHaveAG = true;

		int weaponDamage = 0;

		if (weaponType)
		{
			if (weaponType->AmbientDamage > 0)
				weaponDamage = MapClass::GetTotalDamage(weaponType->AmbientDamage, weaponType->Warhead, objectType->Armor, 0) + MapClass::GetTotalDamage(weaponType->Damage, weaponType->Warhead, objectType->Armor, 0);
			else
				weaponDamage = MapClass::GetTotalDamage(weaponType->Damage, weaponType->Warhead, objectType->Armor, 0);
		}

		// If the target can't be damaged then isn't a valid target
		if (weaponType && weaponDamage <= 0 && !agentMode)
			continue;

		if (!agentMode)
		{
			if (object->IsInAir() && !unitWeaponsHaveAA)
				continue;

			if (!object->IsInAir() && !unitWeaponsHaveAG)
				continue;
		}

		// Don't pick underground units
		if (object->InWhichLayer() == Layer::Underground)
			continue;

		// Stealth ground unit check
		if (object->CloakState == CloakState::Cloaked && !objectType->Naval)
			continue;

		// Submarines aren't a valid target
		if (object->CloakState == CloakState::Cloaked
			&& objectType->Underwater
			&& (pTechnoType->NavalTargeting == NavalTargetingType::Underwater_Never || pTechnoType->NavalTargeting == NavalTargetingType::Naval_None))
		{
			continue;
		}

		// Land not OK for the Naval unit
		if (objectType->Naval
			&& pTechnoType->LandTargeting == LandTargetingType::Land_Not_OK
			&& object->GetCell()->LandType != LandType::Water)
		{
			continue;
		}

		// OnlyTargetHouseEnemy forces targets of a specific (hated) house
		if (onlyTargetThisHouseEnemy && object->Owner != onlyTargetThisHouseEnemy)
			continue;

		if (object != pTechno
			&& object->IsAlive
			&& object->Health > 0
			&& !object->InLimbo
			&& !objectType->Immune
			&& !object->Transporter
			&& object->IsOnMap
			&& !object->Absorbed
			&& !object->TemporalTargetingMe
			&& !object->BeingWarpedOut
			&& object->Owner != pTechno->Owner
			&& (!pTechno->Owner->IsAlliedWith(object)
				|| (pTechno->Owner->IsAlliedWith(object)
					&& object->IsMindControlled()
					&& !pTechno->Owner->IsAlliedWith(object->MindControlledBy))))
		{
			double value = 0;

			if (EvaluateObjectWithMask(object, method, attackAITargetType, idxAITargetTypeItem, pTechno))
			{
				CellStruct newCell;
				newCell.X = (short)object->Location.X;
				newCell.Y = (short)object->Location.Y;

				bool isGoodTarget = false;
				if (calcThreatMode == 0 || calcThreatMode == 1)
				{
					// Threat affected by distance
					double threatMultiplier = 128.0;
					double objectThreatValue = objectType->ThreatPosed;

					if (objectType->SpecialThreatValue > 0)
					{
						double const& TargetSpecialThreatCoefficientDefault = RulesClass::Instance->TargetSpecialThreatCoefficientDefault;
						objectThreatValue += objectType->SpecialThreatValue * TargetSpecialThreatCoefficientDefault;
					}

					// Is Defender house targeting Attacker House? if "yes" then more Threat
					if (pTechno->Owner == HouseClass::Array->GetItem(object->Owner->EnemyHouseIndex))
					{
						double const& EnemyHouseThreatBonus = RulesClass::Instance->EnemyHouseThreatBonus;
						objectThreatValue += EnemyHouseThreatBonus;
					}

					// Extra threat based on current health. More damaged == More threat (almost destroyed objects gets more priority)
					objectThreatValue += object->Health * (1 - object->GetHealthPercentage());
					value = (objectThreatValue * threatMultiplier) / ((pTechno->DistanceFrom(object) / 256.0) + 1.0);

					if (calcThreatMode == 0)
					{
						// Is this object very FAR? then LESS THREAT against pTechno.
						// More CLOSER? MORE THREAT for pTechno.
						if (value > bestVal || bestVal < 0)
							isGoodTarget = true;
					}
					else
					{
						// Is this object very FAR? then MORE THREAT against pTechno.
						// More CLOSER? LESS THREAT for pTechno.
						if (value < bestVal || bestVal < 0)
							isGoodTarget = true;
					}
				}
				else
				{
					// Selection affected by distance
					if (calcThreatMode == 2)
					{
						// Is this object very FAR? then LESS THREAT against pTechno.
						// More CLOSER? MORE THREAT for pTechno.
						value = pTechno->DistanceFrom(object); // Note: distance is in leptons (*256)

						if (value < bestVal || bestVal < 0)
							isGoodTarget = true;
					}
					else
					{
						if (calcThreatMode == 3)
						{
							// Is this object very FAR? then MORE THREAT against pTechno.
							// More CLOSER? LESS THREAT for pTechno.
							value = pTechno->DistanceFrom(object); // Note: distance is in leptons (*256)

							if (value > bestVal || bestVal < 0)
								isGoodTarget = true;
						}
					}
				}

				if (isGoodTarget)
				{
					bestObject = object;
					bestVal = value;
				}
			}
		}
	}

	return bestObject;
}

// TODO: Too many redundant abstract_cast here
bool ScriptExt::EvaluateObjectWithMask(TechnoClass* pTechno, int mask, int attackAITargetType = -1, int idxAITargetTypeItem = -1, TechnoClass* pTeamLeader = nullptr)
{
	if (!pTechno)
		return false;

	WeaponTypeClass* WeaponType1 = nullptr;
	WeaponTypeClass* WeaponType2 = nullptr;
	WeaponTypeClass* WeaponType3 = nullptr;
	BuildingClass* pBuilding = nullptr;
	BuildingTypeClass* pTypeBuilding = nullptr;
	TechnoTypeExt::ExtData* pTypeTechnoExt = nullptr;
	BuildingTypeExt::ExtData* pBuildingExt = nullptr;
	TechnoTypeClass* pTechnoType = pTechno->GetTechnoType();
	auto const& BuildTech = RulesClass::Instance->BuildTech;
	auto const& BaseUnit = RulesClass::Instance->BaseUnit;
	auto const& NeutralTechBuildings = RulesClass::Instance->NeutralTechBuildings;
	int nSuperWeapons = 0;
	double distanceToTarget = 0;
	TechnoClass* pTarget = nullptr;

	// Special case: validate target if is part of a technos list in [AITargetTypes] section
	if (attackAITargetType >= 0 && RulesExt::Global()->AITargetTypesLists.Count > 0)
	{
		DynamicVectorClass<TechnoTypeClass*> objectsList = RulesExt::Global()->AITargetTypesLists.GetItem(attackAITargetType);

		for (int i = 0; i < objectsList.Count; i++)
		{
			if (objectsList.GetItem(i) == pTechnoType)
				return true;
		}

		return false;
	}

	switch (mask)
	{
	case 1:
		// Anything ;-)
		if (!pTechno->Owner->IsNeutral())
			return true;

		break;

	case 2:
		// Building
		pTypeBuilding = abstract_cast<BuildingTypeClass*>(pTechnoType);

		if (!pTechno->Owner->IsNeutral()
			&& (pTechnoType->WhatAmI() == AbstractType::BuildingType
				|| (pTypeBuilding
					&& !(pTypeBuilding->Artillary
						|| pTypeBuilding->TickTank
						|| pTypeBuilding->ICBMLauncher
						|| pTypeBuilding->SensorArray))))
		{
			return true;
		}

		break;

	case 3:
		// Harvester
		pTypeBuilding = abstract_cast<BuildingTypeClass*>(pTechno->GetTechnoType());

		if (!pTechno->Owner->IsNeutral()
			&& ((pTechnoType->WhatAmI() == AbstractType::UnitType
				&& (abstract_cast<UnitTypeClass*>(pTechnoType)->Harvester
					|| abstract_cast<UnitTypeClass*>(pTechnoType)->ResourceGatherer))
				|| (pTypeBuilding
					&& pTechnoType->WhatAmI() == AbstractType::BuildingType
					&& pTypeBuilding->ResourceGatherer)))
		{
			return true;
		}

		break;

	case 4:
		// Infantry
		if (!pTechno->Owner->IsNeutral() && pTechnoType->WhatAmI() == AbstractType::InfantryType)
			return true;

		break;

	case 5:
		pTypeBuilding = abstract_cast<BuildingTypeClass*>(pTechnoType);

		// Vehicle, Aircraft, Deployed vehicle into structure
		if (!pTechno->Owner->IsNeutral()
			&& (pTechnoType->WhatAmI() == AbstractType::UnitType
				|| (pTechnoType->WhatAmI() == AbstractType::BuildingType
					&& (pTypeBuilding
						&& (pTypeBuilding->Artillary
							|| pTypeBuilding->TickTank
							|| pTypeBuilding->ICBMLauncher
							|| pTypeBuilding->SensorArray)))
				|| (pTechnoType->WhatAmI() == AbstractType::AircraftType)))
		{
			return true;
		}

		break;

	case 6:
		// Factory
		if (!pTechno->Owner->IsNeutral()
			&& pTechnoType->WhatAmI() == AbstractType::BuildingType
			&& abstract_cast<BuildingClass*>(pTechno)->Factory != nullptr)
		{
			return true;
		}

		break;

	case 7:
		pTypeBuilding = abstract_cast<BuildingTypeClass*>(pTechnoType);

		// Defense
		if (!pTechno->Owner->IsNeutral()
			&& pTechnoType->WhatAmI() == AbstractType::BuildingType
			&& pTypeBuilding->IsBaseDefense)
		{
			return true;
		}

		break;

	case 8:
		pTarget = abstract_cast<TechnoClass*>(pTechno->Target);

		if (pTeamLeader && pTarget)
		{
			// The possible Target is aiming against me? Revenge!
			if (abstract_cast<TechnoClass*>(pTechno->Target)->Owner == pTeamLeader->Owner)
				return true;

			for (int i = 0; i < pTechno->CurrentTargets.Count; i++)
			{
				if (abstract_cast<TechnoClass*>(pTechno->CurrentTargets.GetItem(i))->Owner == pTeamLeader->Owner)
					return true;
			}

			// Note: Replace these lines when I have access to Combat_Damage() method in YRpp if that is better
			WeaponType1 = pTechno->Veterancy.IsElite() ?
				pTechnoType->EliteWeapon[0].WeaponType :
				pTechnoType->Weapon[0].WeaponType;

			WeaponType2 = pTechno->Veterancy.IsElite() ?
				pTechnoType->EliteWeapon[1].WeaponType :
				pTechnoType->Weapon[1].WeaponType;

			WeaponType3 = WeaponType1;

			if (pTechnoType->IsGattling)
			{
				WeaponType3 = pTechno->Veterancy.IsElite() ?
					pTechnoType->EliteWeapon[pTechno->CurrentWeaponNumber].WeaponType :
					pTechnoType->Weapon[pTechno->CurrentWeaponNumber].WeaponType;

				WeaponType1 = WeaponType3;
			}

			// Then check if this possible target is too near of the Team Leader
			distanceToTarget = pTeamLeader->DistanceFrom(pTechno) / 256.0;

			if (!pTechno->Owner->IsNeutral()
				&& ((WeaponType1 && distanceToTarget <= (WeaponType1->Range / 256.0 * 4.0))
					|| (WeaponType2 && distanceToTarget <= (WeaponType2->Range / 256.0 * 4.0))
					|| (pTeamLeader->GetTechnoType()->GuardRange > 0
						&& distanceToTarget <= (pTeamLeader->GetTechnoType()->GuardRange / 256.0 * 2.0))))
			{
				return true;
			}
		}

		break;

	case 9:
		pTypeBuilding = abstract_cast<BuildingTypeClass*>(pTechnoType);

		// Power Plant
		if (!pTechno->Owner->IsNeutral()
			&& pTechnoType->WhatAmI() == AbstractType::BuildingType
			&& pTypeBuilding->PowerBonus > 0)
		{
			return true;
		}

		break;

	case 10:
		// Occupied Building
		if (pTechnoType->WhatAmI() == AbstractType::BuildingType)
		{
			pBuilding = abstract_cast<BuildingClass*>(pTechno);

			if (pBuilding && pBuilding->Occupants.Count > 0)
				return true;
		}

		break;

	case 11:
		pTypeBuilding = abstract_cast<BuildingTypeClass*>(pTechnoType);

		// Civilian Tech
		if (pTechnoType->WhatAmI() == AbstractType::BuildingType
			&& NeutralTechBuildings.Items)
		{
			for (int i = 0; i < NeutralTechBuildings.Count; i++)
			{
				auto pTechObject = NeutralTechBuildings.GetItem(i);
				if (_stricmp(pTechObject->ID, pTechno->get_ID()) == 0)
					return true;
			}
		}

		// Other cases of civilian Tech Structures
		if (pTechnoType->WhatAmI() == AbstractType::BuildingType
			&& pTypeBuilding->Unsellable
			&& pTypeBuilding->Capturable
			&& pTypeBuilding->TechLevel < 0
			&& pTypeBuilding->NeedsEngineer
			&& !pTypeBuilding->BridgeRepairHut)
		{
			return true;
		}

		break;

	case 12:
		// Refinery
		pTypeBuilding = abstract_cast<BuildingTypeClass*>(pTechnoType);

		if (!pTechno->Owner->IsNeutral()
			&& ((pTechnoType->WhatAmI() == AbstractType::UnitType
				&& !abstract_cast<UnitTypeClass*>(pTechnoType)->Harvester
				&& abstract_cast<UnitTypeClass*>(pTechnoType)->ResourceGatherer)
				|| (pTypeBuilding
					&& pTechnoType->WhatAmI() == AbstractType::BuildingType
					&& (pTypeBuilding->Refinery
						|| pTypeBuilding->ResourceGatherer))))
		{
			return true;
		}

		break;

	case 13:
		// Mind Controller
		WeaponType1 = pTechno->Veterancy.IsElite() ?
			pTechnoType->EliteWeapon[0].WeaponType :
			pTechnoType->Weapon[0].WeaponType;

		WeaponType2 = pTechno->Veterancy.IsElite() ?
			pTechnoType->EliteWeapon[1].WeaponType :
			pTechnoType->Weapon[1].WeaponType;

		WeaponType3 = WeaponType1;

		if (pTechnoType->IsGattling)
		{
			WeaponType3 = pTechno->Veterancy.IsElite() ?
				pTechnoType->EliteWeapon[pTechno->CurrentWeaponNumber].WeaponType :
				pTechnoType->Weapon[pTechno->CurrentWeaponNumber].WeaponType;

			WeaponType1 = WeaponType3;
		}

		if (!pTechno->Owner->IsNeutral()
			&& ((WeaponType1 && WeaponType1->Warhead->MindControl)
				|| (WeaponType2 && WeaponType2->Warhead->MindControl)))
		{
			return true;
		}

		break;

	case 14:
		// Aircraft and Air Unit
		if (!pTechno->Owner->IsNeutral()
			&& (pTechnoType->WhatAmI() == AbstractType::AircraftType
				|| pTechnoType->JumpJet
				|| pTechno->IsInAir()))
		{
			return true;
		}

		break;

	case 15:
		// Naval Unit & Structure
		if (!pTechno->Owner->IsNeutral()
			&& (pTechnoType->Naval
				|| pTechno->GetCell()->LandType == LandType::Water))
		{
			return true;
		}

		break;

	case 16:
		// Cloak Generator, Gap Generator, Radar Jammer or Inhibitor
		pTypeBuilding = abstract_cast<BuildingTypeClass*>(pTechnoType);
		pTypeTechnoExt = TechnoTypeExt::ExtMap.Find(pTechnoType);

		if (!pTechno->Owner->IsNeutral()
			&& ((pTypeTechnoExt
				&& (pTypeTechnoExt->RadarJamRadius > 0
					|| pTypeTechnoExt->InhibitorRange.isset()))
				|| (pTypeBuilding && (pTypeBuilding->GapGenerator
					|| pTypeBuilding->CloakGenerator))))
		{
			return true;
		}

		break;

	case 17:
		pTypeBuilding = abstract_cast<BuildingTypeClass*>(pTechnoType);

		// Ground Vehicle
		if (!pTechno->Owner->IsNeutral()
			&& (pTechnoType->WhatAmI() == AbstractType::UnitType
				|| (pTechnoType->WhatAmI() == AbstractType::BuildingType
					&& pTypeBuilding->UndeploysInto
					&& !pTypeBuilding->BaseNormal)
				&& !pTechno->IsInAir()
				&& !pTechnoType->Naval))
		{
			return true;
		}

		break;

	case 18:
		// Economy: Harvester, Refinery or Resource helper
		pTypeBuilding = abstract_cast<BuildingTypeClass*>(pTechnoType);

		if (!pTechno->Owner->IsNeutral()
			&& ((pTechnoType->WhatAmI() == AbstractType::UnitType
				&& (abstract_cast<UnitTypeClass*>(pTechnoType)->Harvester
					|| abstract_cast<UnitTypeClass*>(pTechnoType)->ResourceGatherer))
				|| (pTypeBuilding
					&& pTechnoType->WhatAmI() == AbstractType::BuildingType
					&& (pTypeBuilding->Refinery
						|| pTypeBuilding->OrePurifier
						|| pTypeBuilding->ResourceGatherer))))
		{
			return true;
		}

		break;

	case 19:
		pTypeBuilding = abstract_cast<BuildingTypeClass*>(pTechnoType);

		// Infantry Factory
		if (!pTechno->Owner->IsNeutral()
			&& pTechnoType->WhatAmI() == AbstractType::BuildingType
			&& pTypeBuilding->Factory == AbstractType::InfantryType)
		{
			return true;
		}

		break;

	case 20:
		pTypeBuilding = abstract_cast<BuildingTypeClass*>(pTechnoType);

		// Land Vehicle Factory
		if (!pTechno->Owner->IsNeutral()
			&& pTechnoType->WhatAmI() == AbstractType::BuildingType
			&& pTypeBuilding->Factory == AbstractType::UnitType
			&& !pTypeBuilding->Naval)
		{
			return true;
		}

		break;

	case 21:
		pTypeBuilding = abstract_cast<BuildingTypeClass*>(pTechnoType);

		// is Aircraft Factory
		if (!pTechno->Owner->IsNeutral()
			&& (pTechnoType->WhatAmI() == AbstractType::BuildingType
				&& (pTypeBuilding->Factory == AbstractType::AircraftType
					|| pTypeBuilding->Helipad)))
		{
			return true;
		}

		break;

	case 22:
		pTypeBuilding = abstract_cast<BuildingTypeClass*>(pTechnoType);
		// Radar & SpySat
		if (!pTechno->Owner->IsNeutral()
			&& (pTechnoType->WhatAmI() == AbstractType::BuildingType
				&& (pTypeBuilding->Radar
					|| pTypeBuilding->SpySat)))
		{
			return true;
		}

		break;

	case 23:
		// Buildable Tech
		if (!pTechno->Owner->IsNeutral()
			&& pTechnoType->WhatAmI() == AbstractType::BuildingType
			&& BuildTech.Items)
		{
			for (int i = 0; i < BuildTech.Count; i++)
			{
				auto pTechObject = BuildTech.GetItem(i);
				if (_stricmp(pTechObject->ID, pTechno->get_ID()) == 0)
					return true;
			}
		}

		break;

	case 24:
		pTypeBuilding = abstract_cast<BuildingTypeClass*>(pTechnoType);

		// Naval Factory
		if (!pTechno->Owner->IsNeutral()
			&& pTechnoType->WhatAmI() == AbstractType::BuildingType
			&& pTypeBuilding->Factory == AbstractType::UnitType
			&& pTypeBuilding->Naval)
		{
			return true;
		}

		break;

	case 25:
		// Super Weapon building
		pTypeBuilding = abstract_cast<BuildingTypeClass*>(pTechnoType);
		pBuildingExt = BuildingTypeExt::ExtMap.Find(static_cast<BuildingTypeClass*>(pTypeBuilding));

		if (pBuildingExt)
			nSuperWeapons = pBuildingExt->SuperWeapons.size();

		if (!pTechno->Owner->IsNeutral()
			&& pTechnoType->WhatAmI() == AbstractType::BuildingType
			&& (pTypeBuilding->SuperWeapon >= 0
				|| pTypeBuilding->SuperWeapon2 >= 0
				|| nSuperWeapons > 0))
		{
			return true;
		}

		break;

	case 26:
		pTypeBuilding = abstract_cast<BuildingTypeClass*>(pTechnoType);

		// Construction Yard
		if (!pTechno->Owner->IsNeutral()
			&& pTechnoType->WhatAmI() == AbstractType::BuildingType
			&& pTypeBuilding->Factory == AbstractType::BuildingType
			&& pTypeBuilding->ConstructionYard)
		{
			return true;
		}
		else
		{
			if (pTechnoType->WhatAmI() == AbstractType::UnitType && BaseUnit.Items)
			{
				for (int i = 0; i < BaseUnit.Count; i++)
				{
					auto pMCVObject = BaseUnit.GetItem(i);
					if (_stricmp(pMCVObject->ID, pTechno->get_ID()) == 0)
						return true;
				}
			}
		}

		break;

	case 27:
		// Any Neutral object
		if (pTechno->Owner->IsNeutral())
			return true;

		break;

	case 28:
		// Cloak Generator & Gap Generator
		pTypeBuilding = abstract_cast<BuildingTypeClass*>(pTechnoType);

		if (!pTechno->Owner->IsNeutral()
			&& (pTypeBuilding && (pTypeBuilding->GapGenerator
				|| pTypeBuilding->CloakGenerator)))
		{
			return true;
		}

		break;

	case 29:
		// Radar Jammer
		pTypeTechnoExt = TechnoTypeExt::ExtMap.Find(pTechnoType);

		if (!pTechno->Owner->IsNeutral()
			&& (pTypeTechnoExt
				&& (pTypeTechnoExt->RadarJamRadius > 0)))
			return true;

		break;

	case 30:
		// Inhibitor
		pTypeTechnoExt = TechnoTypeExt::ExtMap.Find(pTechnoType);

		if (!pTechno->Owner->IsNeutral()
			&& (pTypeTechnoExt
				&& pTypeTechnoExt->InhibitorRange.isset()))
		{
			return true;
		}

		break;

	case 31:
		// Naval Unit
		if (!pTechno->Owner->IsNeutral()
			&& pTechnoType->WhatAmI() != AbstractType::BuildingType
			&& (pTechnoType->Naval
				|| pTechno->GetCell()->LandType == LandType::Water))
		{
			return true;
		}

		break;

	case 32:
		pTypeBuilding = abstract_cast<BuildingTypeClass*>(pTechnoType);

		// Any non-building unit
		if (!pTechno->Owner->IsNeutral()
			&& (pTechnoType->WhatAmI() != AbstractType::BuildingType
				|| (pTechnoType->WhatAmI() == AbstractType::BuildingType
					&& pTypeBuilding
					&& (pTypeBuilding->Artillary
						|| pTypeBuilding->TickTank
						|| pTypeBuilding->ICBMLauncher
						|| pTypeBuilding->SensorArray
						|| pTypeBuilding->ResourceGatherer))))
		{
			return true;
		}

		break;

	case 33:
		pTypeBuilding = abstract_cast<BuildingTypeClass*>(pTechnoType);

		// Capturable Structure or Repair Hut
		if (pTypeBuilding
			&& (pTypeBuilding->Capturable
				|| (pTypeBuilding->BridgeRepairHut
					&& pTypeBuilding->Repairable)))
		{
			return true;
		}

		break;

	case 34:
		if (pTeamLeader)
		{
			// Inside the Area Guard of the Team Leader
			distanceToTarget = pTeamLeader->DistanceFrom(pTechno) / 256.0; // Caution, DistanceFrom() return leptons

			if (!pTechno->Owner->IsNeutral()
				&& (pTeamLeader->GetTechnoType()->GuardRange > 0
					&& distanceToTarget <= ((pTeamLeader->GetTechnoType()->GuardRange / 256.0) * 2.0)))
			{
				return true;
			}
		}

		break;

	case 35:
		pTypeBuilding = abstract_cast<BuildingTypeClass*>(pTechnoType);

		// Land Vehicle Factory & Naval Factory
		if (!pTechno->Owner->IsNeutral()
			&& pTechnoType->WhatAmI() == AbstractType::BuildingType
			&& pTypeBuilding->Factory == AbstractType::UnitType)
		{
			return true;
		}

		break;

	case 36:
		// Building that isn't a defense
		pTypeBuilding = abstract_cast<BuildingTypeClass*>(pTechnoType);

		if (!pTechno->Owner->IsNeutral()
			&& pTypeBuilding
			&& !pTypeBuilding->IsBaseDefense
			&& !(pTypeBuilding->Artillary
				|| pTypeBuilding->TickTank
				|| pTypeBuilding->ICBMLauncher
				|| pTypeBuilding->SensorArray))
		{
			return true;
		}

		break;

	default:
		break;
	}

	// The possible target doesn't fit in te masks
	return false;
}

void ScriptExt::DecreaseCurrentTriggerWeight(TeamClass* pTeam, bool forceJumpLine = true, double modifier = 0)
{
	if (modifier <= 0)
		modifier = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument;

	if (modifier <= 0)
		modifier = RulesClass::Instance->AITriggerFailureWeightDelta;
	else
		modifier = modifier * (-1);

	ModifyCurrentTriggerWeight(pTeam, forceJumpLine, modifier);

	// This action finished
	if (forceJumpLine)
		pTeam->StepCompleted = true;

	return;
}

void ScriptExt::IncreaseCurrentTriggerWeight(TeamClass* pTeam, bool forceJumpLine = true, double modifier = 0)
{
	if (modifier <= 0)
		modifier = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument;

	if (modifier <= 0)
		modifier = abs(RulesClass::Instance->AITriggerSuccessWeightDelta);

	ScriptExt::ModifyCurrentTriggerWeight(pTeam, forceJumpLine, modifier);

	// This action finished
	if (forceJumpLine)
		pTeam->StepCompleted = true;

	return;
}

void ScriptExt::ModifyCurrentTriggerWeight(TeamClass* pTeam, bool forceJumpLine = true, double modifier = 0)
{
	AITriggerTypeClass* pTriggerType = nullptr;
	auto pTeamType = pTeam->Type;
	bool found = false;

	for (int i = 0; i < AITriggerTypeClass::Array->Count && !found; i++)
	{
		auto pTriggerTeam1Type = AITriggerTypeClass::Array->GetItem(i)->Team1;
		auto pTriggerTeam2Type = AITriggerTypeClass::Array->GetItem(i)->Team2;

		if (pTeamType
			&& ((pTriggerTeam1Type && pTriggerTeam1Type == pTeamType)
				|| (pTriggerTeam2Type && pTriggerTeam2Type == pTeamType)))
		{
			found = true;
			pTriggerType = AITriggerTypeClass::Array->GetItem(i);
		}
	}

	if (found)
	{
		pTriggerType->Weight_Current += modifier;

		if (pTriggerType->Weight_Current > pTriggerType->Weight_Maximum)
		{
			pTriggerType->Weight_Current = pTriggerType->Weight_Maximum;
		}
		else
		{
			if (pTriggerType->Weight_Current < pTriggerType->Weight_Minimum)
				pTriggerType->Weight_Current = pTriggerType->Weight_Minimum;
		}
	}
}

void ScriptExt::WaitIfNoTarget(TeamClass* pTeam, int attempts = 0)
{
	// This method modifies the new attack actions preventing Team's Trigger to jump to next script action
	// attempts == number of times the Team will wait if Mission_Attack(...) can't find a new target.
	if (attempts < 0)
		attempts = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument;

	auto pTeamData = TeamExt::ExtMap.Find(pTeam);
	if (pTeamData)
	{
		if (attempts <= 0)
			pTeamData->WaitNoTargetAttempts = -1; // Infinite waits if no target
		else
			pTeamData->WaitNoTargetAttempts = attempts;
	}

	// This action finished
	pTeam->StepCompleted = true;

	return;
}

void ScriptExt::TeamWeightReward(TeamClass* pTeam, double award = 0)
{
	if (award <= 0)
		award = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument;

	auto pTeamData = TeamExt::ExtMap.Find(pTeam);
	if (pTeamData)
	{
		if (award > 0)
			pTeamData->NextSuccessWeightAward = award;
	}

	// This action finished
	pTeam->StepCompleted = true;

	return;
}

void ScriptExt::PickRandomScript(TeamClass* pTeam, int idxScriptsList = -1)
{
	if (idxScriptsList <= 0)
		idxScriptsList = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument;

	bool changeFailed = true;

	if (idxScriptsList >= 0)
	{
		if (idxScriptsList < RulesExt::Global()->AIScriptsLists.Count)
		{
			DynamicVectorClass<ScriptTypeClass*> objectsList = RulesExt::Global()->AIScriptsLists.GetItem(idxScriptsList);

			if (objectsList.Count > 0)
			{
				int IdxSelectedObject = ScenarioClass::Instance->Random.RandomRanged(0, objectsList.Count - 1);

				ScriptTypeClass* pNewScript = objectsList.GetItem(IdxSelectedObject);
				if (pNewScript->ActionsCount > 0)
				{
					changeFailed = false;
					TeamExt::ExtMap.Find(pTeam)->PreviousScriptList.push_back(pTeam->CurrentScript);
					pTeam->CurrentScript = nullptr;
					pTeam->CurrentScript = GameCreate<ScriptClass>(pNewScript);

					// Ready for jumping to the first line of the new script
					pTeam->CurrentScript->CurrentMission = -1;
					pTeam->StepCompleted = true;

					return;
				}
				else
				{
					pTeam->StepCompleted = true;
					Debug::Log("DEBUG: [%s] Aborting Script change because [%s] has 0 Action scripts!\n", pTeam->Type->ID, pNewScript->ID);

					return;
				}
			}
		}
	}

	// This action finished
	if (changeFailed)
	{
		pTeam->StepCompleted = true;
		Debug::Log("DEBUG: [%s] [%s] Failed to change the Team Script with a random one!\n", pTeam->Type->ID, pTeam->CurrentScript->Type->ID);
	}
}

void ScriptExt::Mission_Attack_List(TeamClass* pTeam, bool repeatAction, int calcThreatMode, int attackAITargetType)
{
	auto pTeamData = TeamExt::ExtMap.Find(pTeam);
	if (pTeamData)
		pTeamData->IdxSelectedObjectFromAIList = -1;

	if (attackAITargetType < 0)
		attackAITargetType = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument;

	if (RulesExt::Global()->AITargetTypesLists.Count > 0
		&& RulesExt::Global()->AITargetTypesLists.GetItem(attackAITargetType).Count > 0)
	{
		ScriptExt::Mission_Attack(pTeam, repeatAction, calcThreatMode, attackAITargetType, -1);
	}
}

void ScriptExt::Mission_Move(TeamClass* pTeam, int calcThreatMode = 0, bool pickAllies = false, int attackAITargetType = -1, int idxAITargetTypeItem = -1)
{
	auto pScript = pTeam->CurrentScript;
	int scriptArgument = pScript->Type->ScriptActions[pScript->CurrentMission].Argument; // This is the target type
	TechnoClass* selectedTarget = nullptr;
	bool noWaitLoop = false;
	FootClass* pLeaderUnit = nullptr;
	TechnoTypeClass* pLeaderUnitType = nullptr;
	bool bAircraftsWithoutAmmo = false;
	TechnoClass* pFocus = nullptr;
	auto pTeamData = TeamExt::ExtMap.Find(pTeam);

	if (!pScript)
		return;

	if (!pTeamData)
	{
		pTeam->StepCompleted = true;
		Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d) Jump to next line: %d = %d,%d -> (Reason: ExtData found)\n", pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission, pScript->Type->ScriptActions[pScript->CurrentMission].Action, pScript->Type->ScriptActions[pScript->CurrentMission].Argument, pScript->CurrentMission + 1, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Action, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Argument);

		return;
	}

	// When the new target wasn't found it sleeps some few frames before the new attempt. This can save cycles and cycles of unnecessary executed lines.
	if (pTeamData->WaitNoTargetCounter > 0)
	{
		if (pTeamData->WaitNoTargetTimer.InProgress())
			return;

		pTeamData->WaitNoTargetTimer.Stop();
		noWaitLoop = true;
		pTeamData->WaitNoTargetCounter = 0;

		if (pTeamData->WaitNoTargetAttempts > 0)
			pTeamData->WaitNoTargetAttempts--;
	}

	// This team has no units!
	if (!pTeam)
	{
		if (pTeamData->CloseEnough > 0)
			pTeamData->CloseEnough = -1;

		// This action finished
		pTeam->StepCompleted = true;
		Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d) Jump to next line: %d = %d,%d -> (Reason: No team members alive)\n", pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission, pScript->Type->ScriptActions[pScript->CurrentMission].Action, pScript->Type->ScriptActions[pScript->CurrentMission].Argument, pScript->CurrentMission + 1, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Action, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Argument);

		return;
	}

	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
	{
		if (pUnit && pUnit->IsAlive && !pUnit->InLimbo)
		{
			auto pUnitType = pUnit->GetTechnoType();
			if (pUnitType)
			{
				if (pUnitType->WhatAmI() == AbstractType::AircraftType
					&& !pUnit->IsInAir()
					&& abstract_cast<AircraftTypeClass*>(pUnitType)->AirportBound
					&& pUnit->Ammo < pUnitType->Ammo)
				{
					bAircraftsWithoutAmmo = true;
					pUnit->CurrentTargets.Clear();
				}
			}
		}
	}

	// Find the Leader
	pLeaderUnit = pTeamData->TeamLeader;
	if (!pLeaderUnit
		|| !pLeaderUnit->IsAlive
		|| pLeaderUnit->Health <= 0
		|| pLeaderUnit->InLimbo
		|| !(pLeaderUnit->IsOnMap || (pLeaderUnit->GetTechnoType()->IsSubterranean))
		|| pLeaderUnit->Transporter
		|| pLeaderUnit->Absorbed)
	{
		pLeaderUnit = FindTheTeamLeader(pTeam);
		pTeamData->TeamLeader = pLeaderUnit;
	}

	if (!pLeaderUnit || bAircraftsWithoutAmmo)
	{
		pTeamData->IdxSelectedObjectFromAIList = -1;

		if (pTeamData->CloseEnough > 0)
			pTeamData->CloseEnough = -1;

		if (pTeamData->WaitNoTargetAttempts != 0)
		{
			pTeamData->WaitNoTargetTimer.Stop();
			pTeamData->WaitNoTargetCounter = 0;
			pTeamData->WaitNoTargetAttempts = 0;
		}

		// This action finished
		pTeam->StepCompleted = true;
		Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d) Jump to next line: %d = %d,%d -> (Reasons: No Leader | Aircrafts without ammo)\n", pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission, pScript->Type->ScriptActions[pScript->CurrentMission].Action, pScript->Type->ScriptActions[pScript->CurrentMission].Argument, pScript->CurrentMission + 1, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Action, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Argument);

		return;
	}

	pLeaderUnitType = pLeaderUnit->GetTechnoType();
	pFocus = abstract_cast<TechnoClass*>(pTeam->Focus);

	if (!pFocus && !bAircraftsWithoutAmmo)
	{
		// This part of the code is used for picking a new target.

		int targetMask = scriptArgument;
		selectedTarget = FindBestObject(pLeaderUnit, targetMask, calcThreatMode, pickAllies, attackAITargetType, idxAITargetTypeItem);

		if (selectedTarget)
		{
			Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d) Leader [%s] (UID: %lu) selected [%s] (UID: %lu) as destination target.\n", pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission, pScript->Type->ScriptActions[pScript->CurrentMission].Action, pScript->Type->ScriptActions[pScript->CurrentMission].Argument, pLeaderUnit->GetTechnoType()->get_ID(), pLeaderUnit->UniqueID, selectedTarget->GetTechnoType()->get_ID(), selectedTarget->UniqueID);

			pTeam->Focus = selectedTarget;
			pTeamData->WaitNoTargetAttempts = 0; // Disable Script Waits if there are any because a new target was selected
			pTeamData->WaitNoTargetTimer.Stop();
			pTeamData->WaitNoTargetCounter = 0; // Disable Script Waits if there are any because a new target was selected

			for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
			{
				if (pUnit->IsAlive
					&& (pUnit->IsOnMap || (pUnit->GetTechnoType()->IsSubterranean))
					&& !pUnit->InLimbo
					&& !pUnit->Transporter)
				{
					auto pUnitType = pUnit->GetTechnoType();

					if (pUnit && pUnitType)
					{
						pUnit->CurrentTargets.Clear();

						if (pUnitType->Underwater && pUnitType->LandTargeting == LandTargetingType::Land_Not_OK && selectedTarget->GetCell()->LandType != LandType::Water) // Land not OK for the Naval unit
						{
							// Naval units like Submarines are unable to target ground targets except if they have anti-ground weapons. Ignore the attack
							pUnit->CurrentTargets.Clear();
							pUnit->SetTarget(nullptr);
							pUnit->SetFocus(nullptr);
							pUnit->SetDestination(nullptr, false);
							pUnit->QueueMission(Mission::Area_Guard, true);

							continue;
						}

						pUnit->SetTarget(nullptr);
						pUnit->SetFocus(nullptr);
						pUnit->SetDestination(nullptr, false);
						pUnit->ForceMission(Mission::Guard);

						// Get a cell near the target
						pUnit->QueueMission(Mission::Move, false);
						CoordStruct coord = TechnoExt::PassengerKickOutLocation(selectedTarget, pUnit);
						CellClass* pCellDestination = MapClass::Instance->TryGetCellAt(coord);
						pUnit->SetDestination(pCellDestination, true);

						// Aircraft hack. I hate how this game auto-manages the aircraft missions.
						if (pUnitType->WhatAmI() == AbstractType::AircraftType && pUnit->Ammo > 0 && pUnit->GetHeight() <= 0)
							pUnit->QueueMission(Mission::Move, false);
					}
				}
			}
		}
		else
		{
			// No target was found with the specific criteria.

			if (!noWaitLoop)
			{
				pTeamData->WaitNoTargetCounter = 30;
				pTeamData->WaitNoTargetTimer.Start(30);
			}

			if (pTeamData->IdxSelectedObjectFromAIList >= 0)
				pTeamData->IdxSelectedObjectFromAIList = -1;

			if (pTeamData->WaitNoTargetAttempts != 0)
			{
				pTeamData->WaitNoTargetCounter = 30;
				pTeamData->WaitNoTargetTimer.Start(30); // No target? let's wait some frames

				return;
			}

			if (pTeamData->CloseEnough >= 0)
				pTeamData->CloseEnough = -1;

			// This action finished
			pTeam->StepCompleted = true;
			Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d) Jump to next line: %d = %d,%d (new target NOT FOUND)\n", pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission, pScript->Type->ScriptActions[pScript->CurrentMission].Action, pScript->Type->ScriptActions[pScript->CurrentMission].Argument, pScript->CurrentMission + 1, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Action, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Argument);

			return;
		}
	}
	else
	{
		// This part of the code is used for updating the "Move" mission in each team unit

		int moveDestinationMode = 0;
		moveDestinationMode = pTeamData->MoveMissionEndMode;
		bool bForceNextAction = ScriptExt::MoveMissionEndStatus(pTeam, pFocus, pLeaderUnit, moveDestinationMode);

		if (bForceNextAction)
		{
			pTeamData->MoveMissionEndMode = 0;
			pTeamData->IdxSelectedObjectFromAIList = -1;

			if (pTeamData->CloseEnough >= 0)
				pTeamData->CloseEnough = -1;

			// This action finished
			pTeam->StepCompleted = true;
			Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d) Jump to next line: %d = %d,%d (Reason: Reached destination)\n", pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission, pScript->Type->ScriptActions[pScript->CurrentMission].Action, pScript->Type->ScriptActions[pScript->CurrentMission].Argument, pScript->CurrentMission + 1, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Action, pScript->Type->ScriptActions[pScript->CurrentMission + 1].Argument);

			return;
		}
	}
}

TechnoClass* ScriptExt::FindBestObject(TechnoClass* pTechno, int method, int calcThreatMode = 0, bool pickAllies = false, int attackAITargetType = -1, int idxAITargetTypeItem = -1)
{
	TechnoClass* bestObject = nullptr;
	double bestVal = -1;
	HouseClass* enemyHouse = nullptr;

	// Favorite Enemy House case. If set, AI will focus against that House
	if (!pickAllies && pTechno->BelongsToATeam())
	{
		auto pFoot = abstract_cast<FootClass*>(pTechno);
		if (pFoot && pFoot->Team)
		{
			int enemyHouseIndex = pFoot->Team->FirstUnit->Owner->EnemyHouseIndex;
			bool onlyTargetHouseEnemy = pFoot->Team->Type->OnlyTargetHouseEnemy;

			auto pHouseExt = HouseExt::ExtMap.Find(pFoot->Team->Owner);

			if (pHouseExt && pHouseExt->ForceOnlyTargetHouseEnemyMode != -1)
				onlyTargetHouseEnemy = pHouseExt->ForceOnlyTargetHouseEnemy;

			auto pTeamData = TeamExt::ExtMap.Find(pFoot->Team);
			if (pTeamData)
			{
				if (pTeamData->OnlyTargetHouseEnemyMode != -1)
				{
					onlyTargetHouseEnemy = pTeamData->OnlyTargetHouseEnemy;
				}
			}

			if (onlyTargetHouseEnemy && enemyHouseIndex >= 0)
				enemyHouse = HouseClass::Array->GetItem(enemyHouseIndex);
		}
	}

	// Generic method for targeting
	for (int i = 0; i < TechnoClass::Array->Count; i++)
	{
		auto object = TechnoClass::Array->GetItem(i);
		auto objectType = object->GetTechnoType();
		auto pTechnoType = pTechno->GetTechnoType();

		if (!object || !objectType || !pTechnoType)
			continue;

		if (enemyHouse && enemyHouse != object->Owner)
			continue;

		// Don't pick underground units
		if (object->InWhichLayer() == Layer::Underground)
			continue;

		// Stealth ground unit check
		if (object->CloakState == CloakState::Cloaked && !objectType->Naval)
			continue;

		// Submarines aren't a valid target
		if (object->CloakState == CloakState::Cloaked
			&& objectType->Underwater
			&& (pTechnoType->NavalTargeting == NavalTargetingType::Underwater_Never
				|| pTechnoType->NavalTargeting == NavalTargetingType::Naval_None))
		{
			continue;
		}

		// Land not OK for the Naval unit
		if (objectType->Naval
			&& pTechnoType->LandTargeting == LandTargetingType::Land_Not_OK
			&& object->GetCell()->LandType != LandType::Water)
		{
			continue;
		}

		if (object != pTechno
			&& object->IsAlive
			&& !object->InLimbo
			&& object->IsOnMap
			&& !object->Transporter
			&& !object->Absorbed
			&& ((pickAllies && pTechno->Owner->IsAlliedWith(object))
				|| (!pickAllies && !pTechno->Owner->IsAlliedWith(object))))
		{
			double value = 0;

			if (EvaluateObjectWithMask(object, method, attackAITargetType, idxAITargetTypeItem, pTechno))
			{
				CellStruct newCell;
				newCell.X = (short)object->Location.X;
				newCell.Y = (short)object->Location.Y;

				bool isGoodTarget = false;

				if (calcThreatMode == 0 || calcThreatMode == 1)
				{
					// Threat affected by distance
					double threatMultiplier = 128.0;
					double objectThreatValue = objectType->ThreatPosed;

					if (objectType->SpecialThreatValue > 0)
					{
						double const& TargetSpecialThreatCoefficientDefault = RulesClass::Instance->TargetSpecialThreatCoefficientDefault;
						objectThreatValue += objectType->SpecialThreatValue * TargetSpecialThreatCoefficientDefault;
					}

					// Is Defender house targeting Attacker House? if "yes" then more Threat
					if (pTechno->Owner == HouseClass::Array->GetItem(object->Owner->EnemyHouseIndex))
					{
						double const& EnemyHouseThreatBonus = RulesClass::Instance->EnemyHouseThreatBonus;
						objectThreatValue += EnemyHouseThreatBonus;
					}

					// Extra threat based on current health. More damaged == More threat (almost destroyed objects gets more priority)
					objectThreatValue += object->Health * (1 - object->GetHealthPercentage());
					value = (objectThreatValue * threatMultiplier) / ((pTechno->DistanceFrom(object) / 256.0) + 1.0);

					if (calcThreatMode == 0)
					{
						// Is this object very FAR? then LESS THREAT against pTechno.
						// More CLOSER? MORE THREAT for pTechno.
						if (value > bestVal || bestVal < 0)
							isGoodTarget = true;
					}
					else
					{
						// Is this object very FAR? then MORE THREAT against pTechno.
						// More CLOSER? LESS THREAT for pTechno.
						if (value < bestVal || bestVal < 0)
							isGoodTarget = true;
					}
				}
				else
				{
					// Selection affected by distance
					if (calcThreatMode == 2)
					{
						// Is this object very FAR? then LESS THREAT against pTechno.
						// More CLOSER? MORE THREAT for pTechno.
						value = pTechno->DistanceFrom(object); // Note: distance is in leptons (*256)

						if (value < bestVal || bestVal < 0)
							isGoodTarget = true;
					}
					else
					{
						if (calcThreatMode == 3)
						{
							// Is this object very FAR? then MORE THREAT against pTechno.
							// More CLOSER? LESS THREAT for pTechno.
							value = pTechno->DistanceFrom(object); // Note: distance is in leptons (*256)

							if (value > bestVal || bestVal < 0)
								isGoodTarget = true;
						}
					}
				}

				if (isGoodTarget)
				{
					bestObject = object;
					bestVal = value;
				}
			}
		}
	}

	return bestObject;
}

void ScriptExt::Mission_Attack_List1Random(TeamClass* pTeam, bool repeatAction, int calcThreatMode, int attackAITargetType)
{
	auto pScript = pTeam->CurrentScript;
	bool selected = false;
	int idxSelectedObject = -1;
	DynamicVectorClass<int> validIndexes;

	auto pTeamData = TeamExt::ExtMap.Find(pTeam);
	if (pTeamData && pTeamData->IdxSelectedObjectFromAIList >= 0)
	{
		idxSelectedObject = pTeamData->IdxSelectedObjectFromAIList;
		selected = true;
	}

	if (attackAITargetType < 0)
		attackAITargetType = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument;

	if (attackAITargetType >= 0
		&& attackAITargetType < RulesExt::Global()->AITargetTypesLists.Count)
	{
		DynamicVectorClass<TechnoTypeClass*> objectsList = RulesExt::Global()->AITargetTypesLists.GetItem(attackAITargetType);

		if (idxSelectedObject < 0 && objectsList.Count > 0 && !selected)
		{
			// Finding the objects from the list that actually exists in the map
			for (int i = 0; i < TechnoClass::Array->Count; i++)
			{
				auto pTechno = TechnoClass::Array->GetItem(i);
				auto pTechnoType = TechnoClass::Array->GetItem(i)->GetTechnoType();
				bool found = false;

				for (int j = 0; j < objectsList.Count && !found; j++)
				{
					auto objectFromList = objectsList.GetItem(j);

					if (pTechnoType == objectFromList
						&& pTechno->IsAlive
						&& !pTechno->InLimbo
						&& pTechno->IsOnMap
						&& !pTechno->Absorbed
						&& (!pTeam->FirstUnit->Owner->IsAlliedWith(pTechno)
							|| (pTeam->FirstUnit->Owner->IsAlliedWith(pTechno)
								&& pTechno->IsMindControlled()
								&& !pTeam->FirstUnit->Owner->IsAlliedWith(pTechno->MindControlledBy))))
					{
						validIndexes.AddItem(j);
						found = true;
					}
				}
			}

			if (validIndexes.Count > 0)
			{
				idxSelectedObject = validIndexes.GetItem(ScenarioClass::Instance->Random.RandomRanged(0, validIndexes.Count - 1));
				selected = true;
				Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d) Picked a random Techno from the list index [AITargetTypes][%d][%d] = %s\n", pTeam->Type->ID, pTeam->CurrentScript->Type->ID, pScript->CurrentMission, pScript->Type->ScriptActions[pScript->CurrentMission].Action, pScript->Type->ScriptActions[pScript->CurrentMission].Argument, attackAITargetType, idxSelectedObject, objectsList.GetItem(idxSelectedObject)->ID);
			}
		}

		if (selected)
			pTeamData->IdxSelectedObjectFromAIList = idxSelectedObject;

		Mission_Attack(pTeam, repeatAction, calcThreatMode, attackAITargetType, idxSelectedObject);
	}

	// This action finished
	if (!selected)
	{
		pTeam->StepCompleted = true;
		Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d) Failed to pick a random Techno from the list index [AITargetTypes][%d]! Valid Technos in the list: %d\n", pTeam->Type->ID, pTeam->CurrentScript->Type->ID, pScript->CurrentMission, pScript->Type->ScriptActions[pScript->CurrentMission].Action, pScript->Type->ScriptActions[pScript->CurrentMission].Argument, attackAITargetType, validIndexes.Count);
	}
}

void ScriptExt::Mission_Move_List(TeamClass* pTeam, int calcThreatMode, bool pickAllies, int attackAITargetType)
{
	auto pTeamData = TeamExt::ExtMap.Find(pTeam);
	if (pTeamData)
		pTeamData->IdxSelectedObjectFromAIList = -1;

	if (attackAITargetType < 0)
		attackAITargetType = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument;

	if (RulesExt::Global()->AITargetTypesLists.Count > 0
		&& RulesExt::Global()->AITargetTypesLists.GetItem(attackAITargetType).Count > 0)
	{
		Mission_Move(pTeam, calcThreatMode, pickAllies, attackAITargetType, -1);
	}
}

void ScriptExt::Mission_Move_List1Random(TeamClass* pTeam, int calcThreatMode, bool pickAllies, int attackAITargetType, int idxAITargetTypeItem = -1)
{
	auto pScript = pTeam->CurrentScript;
	bool selected = false;
	int idxSelectedObject = -1;
	DynamicVectorClass<int> validIndexes;

	auto pTeamData = TeamExt::ExtMap.Find(pTeam);
	if (pTeamData && pTeamData->IdxSelectedObjectFromAIList >= 0)
	{
		idxSelectedObject = pTeamData->IdxSelectedObjectFromAIList;
		selected = true;
	}

	if (attackAITargetType < 0)
		attackAITargetType = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument;

	if (attackAITargetType >= 0
		&& attackAITargetType < RulesExt::Global()->AITargetTypesLists.Count)
	{
		DynamicVectorClass<TechnoTypeClass*> objectsList = RulesExt::Global()->AITargetTypesLists.GetItem(attackAITargetType);

		// Still no random target selected
		if (idxSelectedObject < 0 && objectsList.Count > 0 && !selected)
		{
			// Finding the objects from the list that actually exists in the map
			for (int i = 0; i < TechnoClass::Array->Count; i++)
			{
				auto pTechno = TechnoClass::Array->GetItem(i);
				auto pTechnoType = TechnoClass::Array->GetItem(i)->GetTechnoType();
				bool found = false;

				for (int j = 0; j < objectsList.Count && !found; j++)
				{
					auto objectFromList = objectsList.GetItem(j);

					if (pTechnoType == objectFromList
						&& pTechno->IsAlive
						&& !pTechno->InLimbo
						&& pTechno->IsOnMap
						&& !pTechno->Absorbed
						&& ((pickAllies
							&& pTeam->FirstUnit->Owner->IsAlliedWith(pTechno))
							|| (!pickAllies
								&& !pTeam->FirstUnit->Owner->IsAlliedWith(pTechno))))
					{
						validIndexes.AddItem(j);
						found = true;
					}
				}
			}

			if (validIndexes.Count > 0)
			{
				idxSelectedObject = validIndexes.GetItem(ScenarioClass::Instance->Random.RandomRanged(0, validIndexes.Count - 1));
				selected = true;
				Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d) Picked a random Techno from the list index [AITargetTypes][%d][%d] = %s\n", pTeam->Type->ID, pTeam->CurrentScript->Type->ID, pScript->CurrentMission, pScript->Type->ScriptActions[pScript->CurrentMission].Action, pScript->Type->ScriptActions[pScript->CurrentMission].Argument, attackAITargetType, idxSelectedObject, objectsList.GetItem(idxSelectedObject)->ID);
			}
		}

		if (selected)
			pTeamData->IdxSelectedObjectFromAIList = idxSelectedObject;

		Mission_Move(pTeam, calcThreatMode, pickAllies, attackAITargetType, idxSelectedObject);
	}

	// This action finished
	if (!selected)
	{
		pTeam->StepCompleted = true;
		Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d) Failed to pick a random Techno from the list index [AITargetTypes][%d]! Valid Technos in the list: %d\n", pTeam->Type->ID, pTeam->CurrentScript->Type->ID, pScript->CurrentMission, pScript->Type->ScriptActions[pScript->CurrentMission].Action, pScript->Type->ScriptActions[pScript->CurrentMission].Argument, attackAITargetType, validIndexes.Count);
	}
}

void ScriptExt::SetCloseEnoughDistance(TeamClass* pTeam, double distance = -1)
{
	// This passive method replaces the CloseEnough value from rulesmd.ini by a custom one. Used by Mission_Move()
	if (distance <= 0)
		distance = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument;

	auto pTeamData = TeamExt::ExtMap.Find(pTeam);
	if (pTeamData)
	{
		if (distance > 0)
			pTeamData->CloseEnough = distance;
	}

	if (distance <= 0)
		pTeamData->CloseEnough = RulesClass::Instance->CloseEnough / 256.0;

	// This action finished
	pTeam->StepCompleted = true;

	return;
}

void ScriptExt::UnregisterGreatSuccess(TeamClass* pTeam)
{
	pTeam->AchievedGreatSuccess = false;
	pTeam->StepCompleted = true;
}

void ScriptExt::SetMoveMissionEndMode(TeamClass* pTeam, int mode = 0)
{
	// This passive method replaces the CloseEnough value from rulesmd.ini by a custom one. Used by Mission_Move()
	if (mode < 0 || mode > 2)
		mode = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument;

	auto pTeamData = TeamExt::ExtMap.Find(pTeam);
	if (pTeamData)
	{
		if (mode >= 0 && mode <= 2)
			pTeamData->MoveMissionEndMode = mode;
	}

	// This action finished
	pTeam->StepCompleted = true;

	return;
}

bool ScriptExt::MoveMissionEndStatus(TeamClass* pTeam, TechnoClass* pFocus, FootClass* pLeader = nullptr, int mode = 0)
{
	if (!pTeam || !pFocus || mode < 0)
		return false;

	if (mode != 2 && mode != 1 && !pLeader)
		return false;

	double closeEnough = RulesClass::Instance->CloseEnough / 256.0;

	auto pTeamData = TeamExt::ExtMap.Find(pTeam);
	if (pTeamData && pTeamData->CloseEnough > 0)
		closeEnough = pTeamData->CloseEnough;

	bool bForceNextAction = mode == 2;

	// Team already have a focused target
	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
	{
		if (pUnit
			&& pUnit->IsAlive
			&& pUnit->Health > 0
			&& !pUnit->InLimbo
			&& !pUnit->TemporalTargetingMe
			&& !pUnit->BeingWarpedOut
			&& !pUnit->Transporter)
		{
			if (mode == 2)
			{
				// Default mode: all members in range
				if ((pUnit->DistanceFrom(pUnit->Destination) / 256.0) > closeEnough)
				{
					bForceNextAction = false;

					if (pUnit->GetTechnoType()->WhatAmI() == AbstractType::AircraftType && pUnit->Ammo > 0)
						pUnit->QueueMission(Mission::Move, false);

					continue;
				}
				else
				{
					if (pUnit->GetTechnoType()->WhatAmI() == AbstractType::AircraftType && pUnit->Ammo <= 0)
					{
						pUnit->QueueMission(Mission::Return, false);
						pUnit->Mission_Enter();

						continue;
					}
				}
			}
			else
			{
				if (mode == 1)
				{
					// Any member in range
					if ((pUnit->DistanceFrom(pUnit->Destination) / 256.0) > closeEnough)
					{
						if (pUnit->GetTechnoType()->WhatAmI() == AbstractType::AircraftType && pUnit->Ammo > 0)
							pUnit->QueueMission(Mission::Move, false);

						continue;
					}
					else
					{
						bForceNextAction = true;

						if (pUnit->GetTechnoType()->WhatAmI() == AbstractType::AircraftType && pUnit->Ammo <= 0)
						{
							pUnit->QueueMission(Mission::Return, false);
							pUnit->Mission_Enter();

							continue;
						}
					}
				}
				else
				{
					// All other cases: Team Leader mode in range
					if (pLeader)
					{
						if ((pUnit->DistanceFrom(pUnit->Destination) / 256.0) > closeEnough)
						{
							if (pUnit->GetTechnoType()->WhatAmI() == AbstractType::AircraftType && pUnit->Ammo > 0)
								pUnit->QueueMission(Mission::Move, false);

							continue;
						}
						else
						{
							if (pUnit->IsTeamLeader)
								bForceNextAction = true;

							if (pUnit->GetTechnoType()->WhatAmI() == AbstractType::AircraftType && pUnit->Ammo <= 0)
							{
								pUnit->QueueMission(Mission::Return, false);
								pUnit->Mission_Enter();

								continue;
							}
						}
					}
					else
					{
						break;
					}
				}
			}
		}
	}

	return bForceNextAction;
}

void ScriptExt::SkipNextAction(TeamClass* pTeam, int successPercentage = 0)
{
	// This team has no units! END
	if (!pTeam)
	{
		// This action finished
		pTeam->StepCompleted = true;
		Debug::Log("DEBUG: [%s] [%s] (line: %d) Jump to next line: %d = %d,%d -> (No team members alive)\n",
			pTeam->Type->ID, pTeam->CurrentScript->Type->ID, pTeam->CurrentScript->CurrentMission, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Action, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument,
			pTeam->CurrentScript->CurrentMission + 1, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission + 1].Action,
			pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission + 1].Argument);

		return;
	}

	if (successPercentage < 0 || successPercentage > 100)
		successPercentage = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument;

	if (successPercentage < 0)
		successPercentage = 0;

	if (successPercentage > 100)
		successPercentage = 100;

	int percentage = ScenarioClass::Instance->Random.RandomRanged(1, 100);

	if (percentage <= successPercentage)
	{
		Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d) Next script line skipped successfuly. Next line will be: %d = %d,%d\n",
			pTeam->Type->ID, pTeam->CurrentScript->Type->ID, pTeam->CurrentScript->CurrentMission, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Action, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument, pTeam->CurrentScript->CurrentMission + 2,
			pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission + 2].Action, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission + 2].Argument);
		pTeam->CurrentScript->CurrentMission++;
	}

	// This action finished
	pTeam->StepCompleted = true;
}

void ScriptExt::ResetAngerAgainstHouses(TeamClass* pTeam)
{
	// Invalid team
	if (!pTeam)
	{
		// This action finished
		pTeam->StepCompleted = true;
		return;
	}

	for (auto& angerNode : pTeam->Owner->AngerNodes)
	{
		angerNode.AngerLevel = 0;
	}

	pTeam->Owner->EnemyHouseIndex = -1;
	ScriptExt::DebugAngerNodesData();

	// This action finished
	pTeam->StepCompleted = true; // This action finished - FS-21
}

void ScriptExt::SetHouseAngerModifier(TeamClass* pTeam, int modifier = 0)
{
	auto pTeamData = TeamExt::ExtMap.Find(pTeam);

	if (!pTeam || !pTeamData)
	{
		// This action finished
		pTeam->StepCompleted = true;
		return;
	}

	if (modifier <= 0)
		modifier = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument;

	if (modifier < 0)
		modifier = 0;

	pTeamData->AngerNodeModifier = modifier;

	// This action finished
	pTeam->StepCompleted = true;
}

void ScriptExt::ModifyHateHouses_List(TeamClass* pTeam, int idxHousesList = -1)
{
	auto pTeamData = TeamExt::ExtMap.Find(pTeam);
	bool changeFailed = true;

	if (!pTeam || !pTeamData)
	{
		// This action finished
		pTeam->StepCompleted = true;
		return;
	}

	if (idxHousesList <= 0)
		idxHousesList = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument;

	if (idxHousesList >= 0)
	{
		if (idxHousesList < RulesExt::Global()->AIHousesLists.Count)
		{
			DynamicVectorClass<HouseTypeClass*> objectsList = RulesExt::Global()->AIHousesLists.GetItem(idxHousesList);

			if (objectsList.Count > 0)
			{
				for (auto pHouseType : objectsList)
				{
					for (auto& angerNode : pTeam->Owner->AngerNodes)
					{
						HouseTypeClass* angerNodeType = angerNode.House->Type;

						if (_stricmp(angerNodeType->ID, pHouseType->ID) == 0)
						{
							angerNode.AngerLevel += pTeamData->AngerNodeModifier;
							changeFailed = false;
						}
					}
				}
			}
		}
	}

	// This action finished
	if (changeFailed)
	{
		pTeam->StepCompleted = true;
		Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d): Failed to modify hate values against other houses\n", pTeam->Type->ID, pTeam->CurrentScript->Type->ID, pTeam->CurrentScript->CurrentMission, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Action, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument);
	}

	ScriptExt::UpdateEnemyHouseIndex(pTeam->Owner);
	ScriptExt::DebugAngerNodesData();

	// This action finished
	pTeam->StepCompleted = true;
}

void ScriptExt::ModifyHateHouses_List1Random(TeamClass* pTeam, int idxHousesList = -1)
{
	auto pTeamData = TeamExt::ExtMap.Find(pTeam);
	int changes = 0;

	if (!pTeam || !pTeamData)
	{
		// This action finished
		pTeam->StepCompleted = true;
		return;
	}

	if (idxHousesList <= 0)
		idxHousesList = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument;

	if (idxHousesList >= 0)
	{
		if (idxHousesList < RulesExt::Global()->AIHousesLists.Count)
		{
			DynamicVectorClass<HouseTypeClass*> objectsList = RulesExt::Global()->AIHousesLists.GetItem(idxHousesList);
			if (objectsList.Count > 0)
			{
				int IdxSelectedObject = ScenarioClass::Instance->Random.RandomRanged(0, objectsList.Count - 1);
				HouseTypeClass* pHouseType = objectsList.GetItem(IdxSelectedObject);

				for (auto& angerNode : pTeam->Owner->AngerNodes)
				{
					if (angerNode.House->Defeated)
						continue;

					HouseTypeClass* angerNodeType = angerNode.House->Type;

					if (_stricmp(angerNodeType->ID, pHouseType->ID) == 0)
					{
						angerNode.AngerLevel += pTeamData->AngerNodeModifier;
						changes++;
					}
				}
			}
		}
	}

	// This action finished
	if (changes == 0)
	{
		pTeam->StepCompleted = true;
		Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d): Failed to modify hate values against other houses\n", pTeam->Type->ID, pTeam->CurrentScript->Type->ID, pTeam->CurrentScript->CurrentMission, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Action, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument);
	}

	ScriptExt::UpdateEnemyHouseIndex(pTeam->Owner);
	ScriptExt::DebugAngerNodesData();

	// This action finished
	pTeam->StepCompleted = true;
}

void ScriptExt::SetTheMostHatedHouse(TeamClass* pTeam, int mask = 0, int mode = 1, bool random = false)
{
	auto pTeamData = TeamExt::ExtMap.Find(pTeam);

	if (!pTeam || !pTeamData)
	{
		// This action finished
		pTeam->StepCompleted = true;
		return;
	}

	if (mask == 0)
		mask = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument;

	if (mask == 0)
	{
		// This action finished
		pTeam->StepCompleted = true;
		return;
	}

	DynamicVectorClass<HouseClass*> objectsList;
	int IdxSelectedObject = -1;
	HouseClass* selectedHouse = nullptr;
	int highestHateLevel = 0;
	int newHateLevel = 5000;

	if (pTeamData->AngerNodeModifier > 0)
		newHateLevel = pTeamData->AngerNodeModifier;

	// Find the highest House hate value
	for (auto& angerNode : pTeam->Owner->AngerNodes)
	{
		if (pTeam->Owner == angerNode.House
			|| angerNode.House->Defeated
			|| pTeam->Owner->IsAlliedWith(angerNode.House)
			|| angerNode.House->Type->MultiplayPassive)
		{
			continue;
		}

		if (random)
		{
			objectsList.AddItem(angerNode.House);
		}
		else
		{
			if (angerNode.AngerLevel > highestHateLevel)
				highestHateLevel = angerNode.AngerLevel;
		}
	}

	newHateLevel += highestHateLevel;

	// Pick a enemy house
	if (random)
	{
		if (objectsList.Count > 0)
		{
			IdxSelectedObject = ScenarioClass::Instance->Random.RandomRanged(0, objectsList.Count - 1);
			selectedHouse = objectsList.GetItem(IdxSelectedObject);
		}
	}
	else
	{
		selectedHouse = GetTheMostHatedHouse(pTeam, mask, mode);
	}

	if (selectedHouse)
	{
		for (auto& angerNode : pTeam->Owner->AngerNodes)
		{
			if (angerNode.House->Defeated)
				continue;

			if (angerNode.House == selectedHouse)
			{
				angerNode.AngerLevel = newHateLevel;
				Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d): Picked a new house as enemy [%s]\n", pTeam->Type->ID, pTeam->CurrentScript->Type->ID, pTeam->CurrentScript->CurrentMission, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Action, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument, angerNode.House->Type->ID);
			}
		}

		ScriptExt::UpdateEnemyHouseIndex(pTeam->Owner);
	}
	else
	{
		Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d): Failed to pick a new hated house\n", pTeam->Type->ID, pTeam->CurrentScript->Type->ID, pTeam->CurrentScript->CurrentMission, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Action, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument);
	}

	// This action finished
	pTeam->StepCompleted = true;
}

HouseClass* ScriptExt::GetTheMostHatedHouse(TeamClass* pTeam, int mask = 0, int mode = 1)
{
	// Note regarding "mode": 1 is used for ">" comparisons and 0 for "<"
	if (mode <= 0)
		mode = 0;
	else
		mode = 1;

	auto pTeamData = TeamExt::ExtMap.Find(pTeam);
	FootClass* pLeaderUnit = nullptr;
	int bestUnitLeadershipValue = -1;
	//bool teamLeaderFound = false;

	if (!pTeam || !pTeamData || mask == 0)
	{
		// This action finished
		pTeam->StepCompleted = true;
		return nullptr;
	}

	// Find the Team Leader
	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
	{
		if (pUnit && pUnit->IsAlive && !pUnit->InLimbo)
		{
			auto pUnitType = pUnit->GetTechnoType();
			if (pUnitType)
			{
				// The team leader will be used for selecting targets, if there are living Team Members then always exists 1 Leader.
				int unitLeadershipRating = pUnitType->LeadershipRating;
				if (unitLeadershipRating > bestUnitLeadershipValue)
				{
					pLeaderUnit = pUnit;
					bestUnitLeadershipValue = unitLeadershipRating;
				}
			}
		}
	}

	if (!pLeaderUnit)
	{
		// This action finished
		pTeam->StepCompleted = true;
		return nullptr;
	}

	double objectDistance = -1;
	double enemyDistance = -1;
	double enemyThreatValue[8] = { 0 };
	HouseClass* enemyHouse = nullptr;
	double const& TargetSpecialThreatCoefficientDefault = RulesClass::Instance->TargetSpecialThreatCoefficientDefault;
	long houseMoney = -1;
	int enemyPower = -1000000000;
	int enemyKills = -1;

	if (mask == -2)
	{
		// Based on House economy
		for (auto& pHouse : *HouseClass::Array)
		{
			if (pLeaderUnit->Owner == pHouse
				|| pHouse->IsObserver()
				|| pHouse->Defeated
				|| pHouse->Type->MultiplayPassive
				|| pLeaderUnit->Owner->IsAlliedWith(pHouse))
			{
				continue;
			}

			if (mode == 0)
			{
				// The poorest is selected
				if (pHouse->Available_Money() < houseMoney || houseMoney < 0)
				{
					houseMoney = pHouse->Available_Money();
					enemyHouse = pHouse;
				}
			}
			else
			{
				// The richest is selected
				if (pHouse->Available_Money() > houseMoney || houseMoney < 0)
				{
					houseMoney = pHouse->Available_Money();
					enemyHouse = pHouse;
				}
			}
		}

		if (enemyHouse)
			Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d): Selected House [%s] (index: %d)\n", pTeam->Type->ID, pTeam->CurrentScript->Type->ID, pTeam->CurrentScript->CurrentMission, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Action, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument, enemyHouse->Type->ID, enemyHouse->ArrayIndex);

		return enemyHouse;
	}

	if (mask == -3)
	{
		// Based on Human Controlled check
		for (auto& pHouse : *HouseClass::Array)
		{
			if (pLeaderUnit->Owner == pHouse
				|| !pHouse->IsControlledByHuman()
				|| pHouse->Defeated
				|| pHouse->Type->MultiplayPassive
				|| pLeaderUnit->Owner->IsAlliedWith(pHouse))
			{
				continue;
			}

			CoordStruct houseLocation;
			houseLocation.X = pHouse->BaseSpawnCell.X;
			houseLocation.Y = pHouse->BaseSpawnCell.Y;
			houseLocation.Z = 0;
			objectDistance = pLeaderUnit->Location.DistanceFrom(houseLocation); // Note: distance is in leptons (*256)

			if (mode == 0)
			{
				// mode 0: Based in NEAREST human enemy unit
				if (objectDistance < enemyDistance || enemyDistance == -1)
				{
					enemyDistance = objectDistance;
					enemyHouse = pHouse;
				}
			}
			else
			{
				// mode 1: Based in FARTHEST human enemy unit
				if (objectDistance > enemyDistance || enemyDistance == -1)
				{
					enemyDistance = objectDistance;
					enemyHouse = pHouse;
				}
			}
		}

		if (enemyHouse)
			Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d): selected House [%s] (index: %d)\n", pTeam->Type->ID, pTeam->CurrentScript->Type->ID, pTeam->CurrentScript->CurrentMission, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Action, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument, enemyHouse->Type->ID, enemyHouse->ArrayIndex);

		return enemyHouse;
	}

	if (mask == -4 || mask == -5 || mask == -6)
	{
		int checkedHousePower = 0;

		// House power check
		for (auto& pHouse : *HouseClass::Array)
		{
			if (pLeaderUnit->Owner == pHouse
				|| pHouse->Defeated
				|| pHouse->Type->MultiplayPassive
				|| pLeaderUnit->Owner->IsAlliedWith(pHouse))
			{
				continue;
			}

			if (mask == -4)
				checkedHousePower = pHouse->Power_Drain();

			if (mask == -5)
				checkedHousePower = pHouse->PowerOutput;

			if (mask == -6)
				checkedHousePower = pHouse->PowerOutput - pHouse->Power_Drain();

			if (mode == 0)
			{
				// mode 0: Selection based in lower value power in house
				if ((checkedHousePower < enemyPower) || enemyPower == -1000000000)
				{
					enemyPower = checkedHousePower;
					enemyHouse = pHouse;
				}
			}
			else
			{
				// mode 1: Selection based in higher value power in house
				if ((checkedHousePower > enemyPower) || enemyPower == -1000000000)
				{
					enemyPower = checkedHousePower;
					enemyHouse = pHouse;
				}
			}
		}

		if (enemyHouse)
			Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d): selected House [%s] (index: %d)\n", pTeam->Type->ID, pTeam->CurrentScript->Type->ID, pTeam->CurrentScript->CurrentMission, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Action, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument, enemyHouse->Type->ID, enemyHouse->ArrayIndex);

		return enemyHouse;
	}

	if (mask == -7)
	{
		// Based on House kills
		for (auto& pHouse : *HouseClass::Array)
		{
			if (pLeaderUnit->Owner == pHouse
				|| pHouse->IsObserver()
				|| pHouse->Defeated
				|| pHouse->Type->MultiplayPassive
				|| pLeaderUnit->Owner->IsAlliedWith(pHouse))
			{
				continue;
			}

			int currentKills = pHouse->TotalKilledUnits + pHouse->TotalKilledUnits;

			if (mode == 0)
			{
				// The pacifist is selected
				if (currentKills < enemyKills || enemyKills < 0)
				{
					enemyKills = currentKills;
					enemyHouse = pHouse;
				}
			}
			else
			{
				// The major killer is selected
				if (currentKills > enemyKills || enemyKills < 0)
				{
					enemyKills = currentKills;
					enemyHouse = pHouse;
				}
			}
		}

		if (enemyHouse)
			Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d): selected House [%s] (index: %d)\n", pTeam->Type->ID, pTeam->CurrentScript->Type->ID, pTeam->CurrentScript->CurrentMission, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Action, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument, enemyHouse->Type->ID, enemyHouse->ArrayIndex);

		return enemyHouse;
	}

	// Depending the mode check what house will be selected as the most hated
	for (auto pTechno : *TechnoClass::Array)
	{
		if (!pTechno->Owner->Defeated
			&& pTechno->Owner != pTeam->Owner
			&& pTechno->IsAlive
			&& !pTechno->InLimbo
			&& pTechno->IsOnMap
			&& !pTechno->Owner->IsAlliedWith(pTeam->Owner)
			&& !pTechno->Owner->Type->MultiplayPassive)
		{
			if (mask < 0)
			{
				if (mask == -1)
				{
					// mask -1: Based on object distances
					objectDistance = pLeaderUnit->DistanceFrom(pTechno); // Note: distance is in leptons (*256)

					if (mode == 0)
					{
						// mode 0: Based in NEAREST enemy unit
						if (objectDistance < enemyDistance || enemyDistance == -1)
						{
							enemyDistance = objectDistance;
							enemyHouse = pTechno->Owner;
						}
					}
					else
					{
						// mode 1: Based in FARTHEST enemy unit
						if (objectDistance > enemyDistance || enemyDistance == -1)
						{
							enemyDistance = objectDistance;
							enemyHouse = pTechno->Owner;
						}
					}
				}
			}
			else
			{
				// mask > 0 : Threat based on the new types in the new attack actions
				if (ScriptExt::EvaluateObjectWithMask(pTechno, mask, -1, -1, pLeaderUnit))
				{
					auto pTechnoType = pTechno->GetTechnoType();

					if (pTechnoType)
					{
						enemyThreatValue[pTechno->Owner->ArrayIndex] += pTechnoType->ThreatPosed;

						if (pTechnoType->SpecialThreatValue > 0)
						{
							enemyThreatValue[pTechno->Owner->ArrayIndex] += pTechnoType->SpecialThreatValue * TargetSpecialThreatCoefficientDefault;
						}
					}
				}
			}
		}
	}

	if (mask > 0)
	{
		double value = -1;

		for (int i = 0; i < 8; i++)
		{
			if (mode == 0)
			{
				// Select House with LESS threat
				if ((enemyThreatValue[i] < value || value == -1) && !HouseClass::Array->GetItem(i)->Defeated)
				{
					value = enemyThreatValue[i];
					enemyHouse = HouseClass::Array->GetItem(i);
				}
			}
			else
			{
				// Select House with MORE threat
				if ((enemyThreatValue[i] > value || value == -1) && !HouseClass::Array->GetItem(i)->Defeated)
				{
					value = enemyThreatValue[i];
					enemyHouse = HouseClass::Array->GetItem(i);
				}
			}
		}
	}

	if (enemyHouse)
		Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d): selected House [%s] (index: %d)\n", pTeam->Type->ID, pTeam->CurrentScript->Type->ID, pTeam->CurrentScript->CurrentMission, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Action, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument, enemyHouse->Type->ID, enemyHouse->ArrayIndex);

	return enemyHouse;
}

void ScriptExt::OverrideOnlyTargetHouseEnemy(TeamClass* pTeam, int mode = -1)
{
	auto pTeamData = TeamExt::ExtMap.Find(pTeam);
	if (!pTeam || !pTeamData)
	{
		// This action finished
		pTeam->StepCompleted = true;
		return;
	}

	if (mode < 0 || mode > 2)
		mode = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument;

	if (mode < -1 || mode > 2)
		mode = -1;

	pTeamData->OnlyTargetHouseEnemyMode = mode;
	/*
	Modes:
		0  -> Force "False"
		1  -> Force "True"
		2  -> Force "Random boolean"
		-1 -> Use default value in OnlyTargetHouseEnemy tag
		Note: only works for new Actions, not vanilla YR actions
	*/
	switch (mode)
	{
	case 0:
		pTeamData->OnlyTargetHouseEnemy = false;
		break;

	case 1:
		pTeamData->OnlyTargetHouseEnemy = true;
		break;

	case 2:
		pTeamData->OnlyTargetHouseEnemy = (bool)ScenarioClass::Instance->Random.RandomRanged(0, 1);;
		break;

	default:
		pTeamData->OnlyTargetHouseEnemy = pTeam->Type->OnlyTargetHouseEnemy;
		pTeamData->OnlyTargetHouseEnemyMode = -1;
		break;
	}

	Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d): New Team -> OnlyTargetHouseEnemy value: %d\n", pTeam->Type->ID, pTeam->CurrentScript->Type->ID, pTeam->CurrentScript->CurrentMission, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Action, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument, pTeamData->OnlyTargetHouseEnemy);
	// This action finished
	pTeam->StepCompleted = true;
}

void ScriptExt::ModifyHateHouse_Index(TeamClass* pTeam, int idxHouse = -1)
{
	auto pTeamData = TeamExt::ExtMap.Find(pTeam);

	if (!pTeam || !pTeamData)
	{
		// This action finished
		pTeam->StepCompleted = true;
		return;
	}

	if (idxHouse < 0)
		idxHouse = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument;

	if (idxHouse < 0)
	{
		// This action finished
		pTeam->StepCompleted = true;
		return;
	}
	else
	{
		for (auto& angerNode : pTeam->Owner->AngerNodes)
		{
			if (angerNode.House->ArrayIndex == idxHouse && !angerNode.House->Defeated)
			{
				angerNode.AngerLevel += pTeamData->AngerNodeModifier;
				Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d): Modified anger level against [%s](index: %d) with value: %d\n", pTeam->Type->ID, pTeam->CurrentScript->Type->ID, pTeam->CurrentScript->CurrentMission, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Action, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument, angerNode.House->Type->ID, angerNode.House->ArrayIndex, angerNode.AngerLevel);
			}
		}
	}

	ScriptExt::UpdateEnemyHouseIndex(pTeam->Owner);
	ScriptExt::DebugAngerNodesData();

	// This action finished
	pTeam->StepCompleted = true;
}

// The selected house will become the most hated of the map (the effects are only visible if the other houses are enemy of the selected house)
void ScriptExt::AggroHouse(TeamClass* pTeam, int index = -1)
{
	auto pTeamData = TeamExt::ExtMap.Find(pTeam);

	if (!pTeam || !pTeamData)
	{
		// This action finished
		pTeam->StepCompleted = true;
		return;
	}

	DynamicVectorClass<HouseClass*> objectsList;
	HouseClass* selectedHouse = nullptr;
	int newHateLevel = 5000;

	if (pTeamData->AngerNodeModifier > 0)
		newHateLevel = pTeamData->AngerNodeModifier;

	// Store the list of playable houses for later
	for (auto& angerNode : pTeam->Owner->AngerNodes)
	{
		if (!angerNode.House->Defeated && !angerNode.House->Type->MultiplayPassive && !angerNode.House->IsObserver())
		{
			objectsList.AddItem(angerNode.House);
		}
	}

	// Include the own House if we are looking for ANY Human player
	if (index == -3)
	{
		if (!pTeam->Owner->Defeated
			&& !pTeam->Owner->Type->MultiplayPassive
			&& !pTeam->Owner->IsObserver()
			&& !pTeam->Owner->IsControlledByHuman())
		{
			objectsList.AddItem(pTeam->Owner);
		}
	}

	// Positive indexes are specific house indexes. -1 is translated as "pick 1 random" & -2 is the owner of the Team executing the script action
	if (objectsList.Count > 0)
	{
		if (index < 0)
		{
			if (index == -1)
				index = ScenarioClass::Instance->Random.RandomRanged(0, objectsList.Count - 1);

			if (index == -2)
				index = pTeam->Owner->ArrayIndex;
		}
	}
	else
	{
		// This action finished
		pTeam->StepCompleted = true;
		return;
	}

	// Note: at most each "For" lasts 10 loops: 8 players + Civilian + Special houses
	if (index != -3)
	{
		for (auto& pHouse : *HouseClass::Array)
		{
			if (!pHouse->Defeated && pHouse->ArrayIndex == index)
				selectedHouse = pHouse;
		}
	}

	if (selectedHouse || index == -3)
	{
		// For each playable house set the selected house as the one with highest hate value;
		for (auto& pHouse : objectsList)
		{
			int highestHateLevel = 0;

			for (auto& angerNode : pHouse->AngerNodes)
			{
				if (angerNode.AngerLevel > highestHateLevel)
					highestHateLevel = angerNode.AngerLevel;
			}

			for (auto& angerNode : pHouse->AngerNodes)
			{
				if (index == -3)
				{
					if (angerNode.House->IsControlledByHuman())
					{
						angerNode.AngerLevel = highestHateLevel + newHateLevel;
					}
				}
				else
				{
					if (selectedHouse == angerNode.House)
					{
						angerNode.AngerLevel = highestHateLevel + newHateLevel;
					}
				}
			}

			ScriptExt::UpdateEnemyHouseIndex(pHouse);
		}
	}
	else
	{
		Debug::Log("DEBUG: [%s] [%s] (line: %d = %d,%d): Failed to pick a new hated house with index: %d\n", pTeam->Type->ID, pTeam->CurrentScript->Type->ID, pTeam->CurrentScript->CurrentMission, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Action, pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument, index);
	}

	ScriptExt::DebugAngerNodesData();

	// This action finished
	pTeam->StepCompleted = true;
}

void ScriptExt::UpdateEnemyHouseIndex(HouseClass* pHouse)
{
	int angerLevel = 0;
	int index = -1;

	for (auto& angerNode : pHouse->AngerNodes)
	{
		if (!angerNode.House->Defeated
			&& !pHouse->IsAlliedWith(angerNode.House)
			&& angerNode.AngerLevel > angerLevel)
		{
			angerLevel = angerNode.AngerLevel;
			index = angerNode.House->ArrayIndex;
		}
	}

	pHouse->EnemyHouseIndex = index;
}

void ScriptExt::TeamMemberSetGroup(TeamClass* pTeam, int nGroup)
{
	// All member did not match the team type
	// the remaining script will not continue
	// This script marks the end of this team's action
	// - FrozenFog

	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
		pUnit->Group = nGroup;

	pTeam->StepCompleted = true;
}

void ScriptExt::DistributedLoadOntoTransport(TeamClass* pTeam, bool bLoadOnlyFirstLevel)
{
	// Wait for timer stop
	if (pTeam->GuardAreaTimer.TimeLeft > 0)
	{
		--pTeam->GuardAreaTimer.TimeLeft;
		return;
	}

	// If anyone is entering transport, means this script is now proceeding
	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
	{
		if (pUnit->GetCurrentMission() == Mission::Enter)
		{
			return;
		}
	}

	// If anyone is moving, stop now, and add timer
	// why team member will converge upon team creation
	// fuck you westwood - FrozenFog
	pTeam->Focus = nullptr;
	pTeam->QueuedFocus = nullptr;

	bool bStillMoving = false;
	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
	{
		if (pUnit->CurrentMission == Mission::Move || pUnit->Locomotor->Is_Moving())
		{
			TechnoExt::Stop(pUnit, Mission::Wait);
			bStillMoving = true;
		}
	}

	if (bStillMoving)
	{
		pTeam->GuardAreaTimer.Start(45);
		return;
	}

	// Now we're talking
	struct TransportStruct
	{
		bool operator<(const TransportStruct& another) const
		{
			return Foot->GetTechnoType()->Size < another.Foot->GetTechnoType()->Size;
		}

		TransportStruct(FootClass* pFoot, double space)
			: Foot { pFoot }, Space { space }
		{}

		FootClass* Foot;
		double Space;
	};
	std::vector<TransportStruct> transports;
	std::vector<FootClass*> passengers;

	// Find max SizeLimit to determine which type is considered as transport
	double maxSizeLimit = 0;
	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
	{
		auto const pType = pUnit->GetTechnoType();
		if (bLoadOnlyFirstLevel || TechnoExt::GetSizeLeft(pUnit) > 0)
			maxSizeLimit = std::max(maxSizeLimit, pType->SizeLimit);
	}

	// No transports remaining
	if (maxSizeLimit == 0)
	{
		pTeam->StepCompleted = true;
		return;
	}

	// All member share this SizeLimit will consider as transport
	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
	{
		if (pUnit->GetTechnoType()->SizeLimit == maxSizeLimit)
			transports.emplace_back(pUnit, TechnoExt::GetSizeLeft(pUnit));
		else
			passengers.push_back(pUnit);
	}

	// If there are no passengers
	// then this script is done
	if (passengers.empty())
	{
		pTeam->StepCompleted = true;
		return;
	}

	// Load logic
	// range prioritize
	bool bIsPassengerLoading = false;
	// larger size first
	std::sort(passengers.begin(), passengers.end());
	for (auto const pPassenger : passengers)
	{
		auto pPassengerType = pPassenger->GetTechnoType();
		// Is legal loadable unit ?
		if (pPassengerType->WhatAmI() != AbstractType::AircraftType &&
				!pPassengerType->ConsideredAircraft &&
				TechnoExt::IsReallyAlive(pPassenger))
		{
			TransportStruct* pTargetTransport = nullptr;
			double distance = std::numeric_limits<double>::infinity();
			for (auto& transport : transports)
			{
				auto const pTransport = transport.Foot;
				const double& currentSpace = transport.Space;
				auto const pTransportType = pTransport->GetTechnoType();

				// Can unit load onto this car ?
				if (currentSpace > 0 &&
					pPassengerType->Size > 0 &&
					pPassengerType->Size <= pTransportType->SizeLimit &&
					pPassengerType->Size <= currentSpace)
				{

					double d = pPassenger->DistanceFrom(pTransport);
					if (d < distance)
					{
						pTargetTransport = &transport;
						distance = d;
					}
				}
			}
			// This is nearest available transport
			if (pTargetTransport)
			{
				// Get on the car
				if (pPassenger->GetCurrentMission() != Mission::Enter)
				{
					pPassenger->QueueMission(Mission::Enter, true);
					pPassenger->SetTarget(nullptr);
					pPassenger->SetDestination(pTargetTransport->Foot, false);
					pTargetTransport->Space -= pPassengerType->Size;
					bIsPassengerLoading = true;
				}
			}
		}
	}
	// If no one is loading, this script is done
	if (!bIsPassengerLoading)
		pTeam->StepCompleted = true;

	// Load logic
	// speed prioritize
	//for (auto pTransport : transports)
	//{
	//	DynamicVectorClass<FootClass*> loadedUnits;
	//	auto pTransportType = pTransport->GetTechnoType();
	//	double currentSpace = pTransportType->Passengers - pTransport->Passengers.GetTotalSize();
	//	if (currentSpace == 0) continue;
	//	for (auto pUnit : passengers)
	//	{
	//		auto pUnitType = pUnit->GetTechnoType();
	//		// Is legal loadable unit ?
	//		if (pUnitType->WhatAmI() != AbstractType::AircraftType &&
	//			!pUnit->InLimbo &&
	//			!pUnitType->ConsideredAircraft &&
	//			pUnit->Health > 0 &&
	//			pUnit != pTransport)
	//		{
	//			// Can unit load onto this car ?
	//			if (pUnitType->Size > 0
	//				&& pUnitType->Size <= pTransportType->SizeLimit
	//				&& pUnitType->Size <= currentSpace)
	//			{
	//				// Get on the car
	//				if (pUnit->GetCurrentMission() != Mission::Enter)
	//				{
	//					transportsNoSpace = false;
	//					pUnit->QueueMission(Mission::Enter, true);
	//					pUnit->SetTarget(nullptr);
	//					pUnit->SetDestination(pTransport, false);
	//					currentSpace -= pUnitType->Size;
	//					loadedUnits.AddItem(pUnit);
	//				}
	//			}
	//		}
	//		if (currentSpace == 0) break;
	//	}
	//	for (auto pRemove : loadedUnits) passengers.Remove(pRemove);
	//	loadedUnits.Clear();
	//}
	//if (transportsNoSpace)
	//{
	//	pTeam->StepCompleted = true;
	//	return;
	//}
}

bool ScriptExt::IsValidFriendlyTarget(TeamClass* pTeam, int nGroup, TechnoClass* pTarget, bool bIsSelfNaval, bool bIsSelfAircraft, bool bIsFriendly)
{
	if (!pTarget)
		return false;

	if (TechnoExt::IsReallyAlive(pTarget) && pTarget->Group == nGroup)
	{
		auto const pType = pTarget->GetTechnoType();

		// Friendly?
		if (bIsFriendly != pTeam->Owner->IsAlliedWith(pTarget->Owner))
			return false;

		// Only aircraft team can follow friendly aircraft
		if (bIsSelfAircraft)
			return true;
		else if (pType->ConsideredAircraft)
			return false;

		// If team is naval, only follow friendly naval
		if (bIsSelfNaval != pType->Naval)
			return false;

		// No underground
		if (pTarget->InWhichLayer() == Layer::Underground)
			return false;

		return true;
	}
	return false;
}

void ScriptExt::FollowTargetByGroup(TeamClass* pTeam, int nGroup, bool bIsFriendly)
{
	bool bIsSelfNaval = true, bIsSelfAircraft = true;
	double distMin = std::numeric_limits<double>::infinity();
	CellStruct* pLocation = nullptr;
	TechnoClass* pTarget = nullptr;
	// Use timer to reduce unnecessary cycle
	if (pTeam->GuardAreaTimer.TimeLeft <= 0)
	{
		// If all member is naval, will only follow friendly navals
		for (auto pMember = pTeam->FirstUnit; pMember; pMember = pMember->NextTeamMember)
		{
			if (!pLocation)
				pLocation = &(pMember->GetCell()->MapCoords);

			auto const pMemberType = pMember->GetTechnoType();
			bIsSelfNaval = bIsSelfNaval && pMemberType->Naval;
			bIsSelfAircraft = bIsSelfAircraft && pMemberType->ConsideredAircraft;
		}

		// If previous target is valid, skip this check
		auto const pDest = abstract_cast<TechnoClass*>(pTeam->Focus);
		if (ScriptExt::IsValidFriendlyTarget(pTeam, nGroup, pDest, bIsSelfNaval, bIsSelfAircraft, bIsFriendly))
		{
			for (auto pMember = pTeam->FirstUnit; pMember; pMember = pMember->NextTeamMember)
			{
				double d = pMember->GetMapCoords().DistanceFrom(pDest->GetCell()->MapCoords);
				if (d * 256 > RulesClass::Instance->CloseEnough)
				{
					if (bIsSelfAircraft)
					{
						pMember->SetDestination(pDest, false);
						pMember->QueueMission(Mission::Move, true);
					}
					else
					{
						pMember->QueueMission(Mission::Area_Guard, true);
						pMember->SetTarget(nullptr);
						pMember->SetFocus(pDest);
					}
				}
				else
				{
					if (!bIsSelfAircraft)
					{
						pMember->SetDestination(nullptr, false);
						pMember->CurrentMission = Mission::Area_Guard;
						pMember->Focus = nullptr;
					}
					else
					{
						pMember->QueueMission(Mission::Area_Guard, true);
					}
				}
			}
			pTeam->GuardAreaTimer.Start(30);
			return;
		}

		// Now looking for target
		for (auto pTechno : *TechnoClass::Array)
		{
			if (ScriptExt::IsValidFriendlyTarget(pTeam, nGroup, pTechno, bIsSelfNaval, bIsSelfAircraft, bIsFriendly))
			{
				// candidate
				double distance = pTechno->GetCell()->MapCoords.DistanceFromSquared(*pLocation);
				if (distance < distMin)
				{
					pTarget = pTechno;
					distMin = distance;
				}
			}
		}

		if (pTarget)
		{
			if (bIsSelfAircraft)
			{
				for (auto pMember = pTeam->FirstUnit; pMember; pMember = pMember->NextTeamMember)
				{
					//pMember->SetTarget(target);
					//pMember->SetFocus(target);
					pMember->SetDestination(pTarget, false);
					pMember->QueueMission(Mission::Move, true);
				}
			}
			else
			{
				for (auto pMember = pTeam->FirstUnit; pMember; pMember = pMember->NextTeamMember)
				{
					pMember->QueueMission(Mission::Area_Guard, true);
					pMember->SetTarget(nullptr);
					pMember->SetFocus(pTarget);
				}
			}

			pTeam->Focus = pTarget;
			pTeam->GuardAreaTimer.Start(30);
		}
		else // If there's no valid target, continue script
		{
			pTeam->StepCompleted = true;
		}
	}
	else
	{
		--pTeam->GuardAreaTimer.TimeLeft;
	}
}

void ScriptExt::VariablesHandler(TeamClass* pTeam, PhobosScripts eAction, int nArg)
{
	struct operation_set { int operator()(const int& a, const int& b) { return b; } };
	struct operation_add { int operator()(const int& a, const int& b) { return a + b; } };
	struct operation_minus { int operator()(const int& a, const int& b) { return a - b; } };
	struct operation_multiply { int operator()(const int& a, const int& b) { return a * b; } };
	struct operation_divide { int operator()(const int& a, const int& b) { return a / b; } };
	struct operation_mod { int operator()(const int& a, const int& b) { return a % b; } };
	struct operation_leftshift { int operator()(const int& a, const int& b) { return a << b; } };
	struct operation_rightshift { int operator()(const int& a, const int& b) { return a >> b; } };
	struct operation_reverse { int operator()(const int& a, const int& b) { return ~a; } };
	struct operation_xor { int operator()(const int& a, const int& b) { return a ^ b; } };
	struct operation_or { int operator()(const int& a, const int& b) { return a | b; } };
	struct operation_and { int operator()(const int& a, const int& b) { return a & b; } };

	int nLoArg = LOWORD(nArg);
	int nHiArg = HIWORD(nArg);

	switch (eAction)
	{
	case PhobosScripts::LocalVariableSet:
		VariableOperationHandler<false, operation_set>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableAdd:
		VariableOperationHandler<false, operation_add>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableMinus:
		VariableOperationHandler<false, operation_minus>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableMultiply:
		VariableOperationHandler<false, operation_multiply>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableDivide:
		VariableOperationHandler<false, operation_divide>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableMod:
		VariableOperationHandler<false, operation_mod>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableLeftShift:
		VariableOperationHandler<false, operation_leftshift>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableRightShift:
		VariableOperationHandler<false, operation_rightshift>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableReverse:
		VariableOperationHandler<false, operation_reverse>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableXor:
		VariableOperationHandler<false, operation_xor>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableOr:
		VariableOperationHandler<false, operation_or>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableAnd:
		VariableOperationHandler<false, operation_and>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableSet:
		VariableOperationHandler<true, operation_set>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableAdd:
		VariableOperationHandler<true, operation_add>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableMinus:
		VariableOperationHandler<true, operation_minus>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableMultiply:
		VariableOperationHandler<true, operation_multiply>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableDivide:
		VariableOperationHandler<true, operation_divide>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableMod:
		VariableOperationHandler<true, operation_mod>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableLeftShift:
		VariableOperationHandler<true, operation_leftshift>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableRightShift:
		VariableOperationHandler<true, operation_rightshift>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableReverse:
		VariableOperationHandler<true, operation_reverse>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableXor:
		VariableOperationHandler<true, operation_xor>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableOr:
		VariableOperationHandler<true, operation_or>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableAnd:
		VariableOperationHandler<true, operation_and>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableSetByLocal:
		VariableBinaryOperationHandler<false, false, operation_set>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableAddByLocal:
		VariableBinaryOperationHandler<false, false, operation_add>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableMinusByLocal:
		VariableBinaryOperationHandler<false, false, operation_minus>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableMultiplyByLocal:
		VariableBinaryOperationHandler<false, false, operation_multiply>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableDivideByLocal:
		VariableBinaryOperationHandler<false, false, operation_divide>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableModByLocal:
		VariableBinaryOperationHandler<false, false, operation_mod>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableLeftShiftByLocal:
		VariableBinaryOperationHandler<false, false, operation_leftshift>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableRightShiftByLocal:
		VariableBinaryOperationHandler<false, false, operation_rightshift>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableReverseByLocal:
		VariableBinaryOperationHandler<false, false, operation_reverse>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableXorByLocal:
		VariableBinaryOperationHandler<false, false, operation_xor>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableOrByLocal:
		VariableBinaryOperationHandler<false, false, operation_or>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableAndByLocal:
		VariableBinaryOperationHandler<false, false, operation_and>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableSetByLocal:
		VariableBinaryOperationHandler<false, true, operation_set>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableAddByLocal:
		VariableBinaryOperationHandler<false, true, operation_add>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableMinusByLocal:
		VariableBinaryOperationHandler<false, true, operation_minus>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableMultiplyByLocal:
		VariableBinaryOperationHandler<false, true, operation_multiply>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableDivideByLocal:
		VariableBinaryOperationHandler<false, true, operation_divide>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableModByLocal:
		VariableBinaryOperationHandler<false, true, operation_mod>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableLeftShiftByLocal:
		VariableBinaryOperationHandler<false, true, operation_leftshift>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableRightShiftByLocal:
		VariableBinaryOperationHandler<false, true, operation_rightshift>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableReverseByLocal:
		VariableBinaryOperationHandler<false, true, operation_reverse>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableXorByLocal:
		VariableBinaryOperationHandler<false, true, operation_xor>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableOrByLocal:
		VariableBinaryOperationHandler<false, true, operation_or>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableAndByLocal:
		VariableBinaryOperationHandler<false, true, operation_and>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableSetByGlobal:
		VariableBinaryOperationHandler<true, false, operation_set>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableAddByGlobal:
		VariableBinaryOperationHandler<true, false, operation_add>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableMinusByGlobal:
		VariableBinaryOperationHandler<true, false, operation_minus>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableMultiplyByGlobal:
		VariableBinaryOperationHandler<true, false, operation_multiply>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableDivideByGlobal:
		VariableBinaryOperationHandler<true, false, operation_divide>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableModByGlobal:
		VariableBinaryOperationHandler<true, false, operation_mod>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableLeftShiftByGlobal:
		VariableBinaryOperationHandler<true, false, operation_leftshift>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableRightShiftByGlobal:
		VariableBinaryOperationHandler<true, false, operation_rightshift>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableReverseByGlobal:
		VariableBinaryOperationHandler<true, false, operation_reverse>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableXorByGlobal:
		VariableBinaryOperationHandler<true, false, operation_xor>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableOrByGlobal:
		VariableBinaryOperationHandler<true, false, operation_or>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::LocalVariableAndByGlobal:
		VariableBinaryOperationHandler<true, false, operation_and>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableSetByGlobal:
		VariableBinaryOperationHandler<true, true, operation_set>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableAddByGlobal:
		VariableBinaryOperationHandler<true, true, operation_add>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableMinusByGlobal:
		VariableBinaryOperationHandler<true, true, operation_minus>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableMultiplyByGlobal:
		VariableBinaryOperationHandler<true, true, operation_multiply>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableDivideByGlobal:
		VariableBinaryOperationHandler<true, true, operation_divide>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableModByGlobal:
		VariableBinaryOperationHandler<true, true, operation_mod>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableLeftShiftByGlobal:
		VariableBinaryOperationHandler<true, true, operation_leftshift>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableRightShiftByGlobal:
		VariableBinaryOperationHandler<true, true, operation_rightshift>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableReverseByGlobal:
		VariableBinaryOperationHandler<true, true, operation_reverse>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableXorByGlobal:
		VariableBinaryOperationHandler<true, true, operation_xor>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableOrByGlobal:
		VariableBinaryOperationHandler<true, true, operation_or>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::GlobalVariableAndByGlobal:
		VariableBinaryOperationHandler<true, true, operation_and>(pTeam, nLoArg, nHiArg); break;
	case PhobosScripts::ChangeTeamGroup:
		TeamMemberSetGroup(pTeam, nArg); break;
	case PhobosScripts::DistributedLoading:
		DistributedLoadOntoTransport(pTeam, nArg == 0); break;
	case PhobosScripts::FollowFriendlyByGroup:
		FollowTargetByGroup(pTeam, nArg, true); break;
	case PhobosScripts::FollowEnemyByGroup:
		FollowTargetByGroup(pTeam, nArg, false); break;
	}
}

template<bool IsGlobal, class _Pr>
void ScriptExt::VariableOperationHandler(TeamClass* pTeam, int nVariable, int Number)
{
	auto itr = ScenarioExt::Global()->Variables[IsGlobal].find(nVariable);
	if (itr != ScenarioExt::Global()->Variables[IsGlobal].end())
	{
		itr->second.Value = _Pr()(itr->second.Value, Number);
		if (IsGlobal)
			TagClass::NotifyGlobalChanged(nVariable);
		else
			TagClass::NotifyLocalChanged(nVariable);
	}
	pTeam->StepCompleted = true;
}

template<bool IsSrcGlobal, bool IsGlobal, class _Pr>
void ScriptExt::VariableBinaryOperationHandler(TeamClass* pTeam, int nVariable, int nVarToOperate)
{
	auto itr = ScenarioExt::Global()->Variables[IsSrcGlobal].find(nVarToOperate);
	if (itr != ScenarioExt::Global()->Variables[IsSrcGlobal].end())
		VariableOperationHandler<IsGlobal, _Pr>(pTeam, nVariable, itr->second.Value);

	pTeam->StepCompleted = true;
}

FootClass* ScriptExt::FindTheTeamLeader(TeamClass* pTeam)
{
	FootClass* pLeaderUnit = nullptr;
	int bestUnitLeadershipValue = -1;
	bool teamLeaderFound = false;

	if (!pTeam)
		return pLeaderUnit;

	// Find the Leader or promote a new one
	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
	{
		if (!pUnit)
			continue;

		bool isValidUnit = pUnit->IsAlive
			&& pUnit->Health > 0
			&& !pUnit->InLimbo
			&& (pUnit->IsOnMap || (pUnit->GetTechnoType()->IsSubterranean))
			&& !pUnit->Transporter
			&& !pUnit->Absorbed;

		// Preventing >1 leaders in teams
		if (teamLeaderFound || !isValidUnit)
		{
			pUnit->IsTeamLeader = false;
			continue;
		}

		if (pUnit->IsTeamLeader)
		{
			pLeaderUnit = pUnit;
			teamLeaderFound = true;

			continue;
		}

		auto pUnitType = pUnit->GetTechnoType();
		if (!pUnitType)
			continue;

		// The team Leader will be used for selecting targets, if there are living Team Members then always exists 1 Leader.
		int unitLeadershipRating = pUnitType->LeadershipRating;
		if (unitLeadershipRating > bestUnitLeadershipValue)
		{
			pLeaderUnit = pUnit;
			bestUnitLeadershipValue = unitLeadershipRating;
		}
	}

	if (pLeaderUnit)
		pLeaderUnit->IsTeamLeader = true;

	return pLeaderUnit;
}

bool ScriptExt::IsExtVariableAction(int action)
{
	auto eAction = static_cast<PhobosScripts>(action);
	return eAction >= PhobosScripts::LocalVariableAdd && eAction <= PhobosScripts::GlobalVariableAndByGlobal;
}

void ScriptExt::DebugAngerNodesData()
{
	Debug::Log("DEBUG: Updated AngerNodes lists of every playable House:\n");

	for (auto pHouse : *HouseClass::Array)
	{
		if (pHouse->IsObserver())
			Debug::Log("Player %d [Observer] ", pHouse->ArrayIndex);
		else
			Debug::Log("Player %d [%s]: ", pHouse->ArrayIndex, pHouse->Type->ID);

		int i = 0;

		for (auto& angerNode : pHouse->AngerNodes)
		{
			if (!pHouse->IsObserver())
				Debug::Log("%d:%d", angerNode.House->ArrayIndex, angerNode.AngerLevel);

			if (i < HouseClass::Array->Count - 2 && !pHouse->IsObserver())
				Debug::Log(", ");

			i++;
		}

		if (!pHouse->IsObserver())
			Debug::Log(" -> Main Enemy House: %d\n", pHouse->EnemyHouseIndex);
		else
			Debug::Log("\n");
	}
}

void ScriptExt::Set_ForceJump_Countdown(TeamClass* pTeam, bool repeatLine = false, int count = 0)
{
	if (!pTeam)
		return;

	auto pTeamData = TeamExt::ExtMap.Find(pTeam);
	if (!pTeamData)
		return;

	if (count <= 0)
		count = 15 * pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument;

	if (count > 0)
	{
		pTeamData->ForceJump_InitialCountdown = count;
		pTeamData->ForceJump_Countdown.Start(count);
		pTeamData->ForceJump_RepeatMode = repeatLine;
	}
	else
	{
		pTeamData->ForceJump_InitialCountdown = -1;
		pTeamData->ForceJump_Countdown.Stop();
		pTeamData->ForceJump_Countdown = -1;
		pTeamData->ForceJump_RepeatMode = false;
	}

	auto pScript = pTeam->CurrentScript;

	// This action finished
	pTeam->StepCompleted = true;
	Debug::Log("DEBUG: [%s] [%s](line: %d = %d,%d) Set Timed Jump -> (Countdown: %d, repeat action: %d)\n", pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission, pScript->Type->ScriptActions[pScript->CurrentMission].Action, pScript->Type->ScriptActions[pScript->CurrentMission].Argument, count, repeatLine);
}

void ScriptExt::Stop_ForceJump_Countdown(TeamClass* pTeam)
{
	if (!pTeam)
		return;

	auto pTeamData = TeamExt::ExtMap.Find(pTeam);
	if (!pTeamData)
		return;

	pTeamData->ForceJump_InitialCountdown = -1;
	pTeamData->ForceJump_Countdown.Stop();
	pTeamData->ForceJump_Countdown = -1;
	pTeamData->ForceJump_RepeatMode = false;

	auto pScript = pTeam->CurrentScript;

	// This action finished
	pTeam->StepCompleted = true;
	Debug::Log("DEBUG: [%s] [%s](line: %d = %d,%d): Stopped Timed Jump\n", pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission, pScript->Type->ScriptActions[pScript->CurrentMission].Action, pScript->Type->ScriptActions[pScript->CurrentMission].Argument);

	return;
}

void ScriptExt::ForceGlobalOnlyTargetHouseEnemy(TeamClass* pTeam, int mode = -1)
{
	if (!pTeam)
		return;

	auto pHouseExt = HouseExt::ExtMap.Find(pTeam->Owner);
	if (!pHouseExt)
	{
		// This action finished
		pTeam->StepCompleted = true;
		return;
	}

	if (mode < 0 || mode > 2)
		mode = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument;

	if (mode < -1 || mode > 2)
		mode = -1;

	HouseExt::ForceOnlyTargetHouseEnemy(pTeam->Owner, mode);

	// This action finished
	pTeam->StepCompleted = true;
}

void ScriptExt::ManageTriggersFromList(TeamClass* pTeam, int idxAITriggerType = -1, bool isEnabled = false)
{
	auto pScript = pTeam->CurrentScript;

	if (idxAITriggerType < 0)
		idxAITriggerType = pScript->Type->ScriptActions[pScript->CurrentMission].Argument;

	if (idxAITriggerType < 0)
		return;

	if (RulesExt::Global()->AITriggersLists.Count <= 0)
		return;

	DynamicVectorClass<AITriggerTypeClass*> objectsList = RulesExt::Global()->AITriggersLists.GetItem(idxAITriggerType);

	for (auto pTrigger : *AITriggerTypeClass::Array)
	{
		if (objectsList.FindItemIndex(pTrigger) >= 0)
		{
			pTrigger->IsEnabled = isEnabled;
		}
	}

	// This action finished
	pTeam->StepCompleted = true;
}

void ScriptExt::ManageAllTriggersFromHouse(TeamClass* pTeam, HouseClass* pHouse = nullptr, int sideIdx = -1, int houseIdx = -1, bool isEnabled = true)
{
	// if pHouse is set then it overwrites any argument
	if (pHouse)
	{
		houseIdx = pHouse->ArrayIndex;
		sideIdx = pHouse->SideIndex;
	}

	if (sideIdx < 0)
		return;

	for (auto pTrigger : *AITriggerTypeClass::Array)
	{
		if ((houseIdx == -1 || houseIdx == pTrigger->HouseIndex) && (sideIdx == 0 || sideIdx == pTrigger->SideIndex))
		{
			pTrigger->IsEnabled = isEnabled;
		}
	}

	// This action finished
	pTeam->StepCompleted = true;
}

void ScriptExt::SetSideIdxForManagingTriggers(TeamClass* pTeam, int sideIdx = -1)
{
	if (!pTeam)
		return;

	auto pScript = pTeam->CurrentScript;

	if (sideIdx < 0)
		sideIdx = pScript->Type->ScriptActions[pScript->CurrentMission].Argument;

	if (sideIdx < -1)
		sideIdx = -1;

	if (auto pTeamData = TeamExt::ExtMap.Find(pTeam))
		pTeamData->TriggersSideIdx = sideIdx;

	// This action finished
	pTeam->StepCompleted = true;
}

void ScriptExt::SetHouseIdxForManagingTriggers(TeamClass* pTeam, int houseIdx = 1000000)
{
	if (!pTeam)
		return;

	auto pScript = pTeam->CurrentScript;

	if (houseIdx == 1000000)
		houseIdx = pScript->Type->ScriptActions[pScript->CurrentMission].Argument;

	houseIdx = HouseExt::GetHouseIndex(houseIdx, pTeam, nullptr);

	if (houseIdx < -1)
		houseIdx = -1;

	if (auto pTeamData = TeamExt::ExtMap.Find(pTeam))
		pTeamData->TriggersHouseIdx = houseIdx;

	// This action finished
	pTeam->StepCompleted = true;
}

void ScriptExt::ManageAITriggers(TeamClass* pTeam, int enabled = -1)
{
	if (!pTeam)
		return;

	if (auto pTeamData = TeamExt::ExtMap.Find(pTeam))
	{
		int sideIdx = pTeamData->TriggersSideIdx;
		int houseIdx = pTeamData->TriggersHouseIdx;
		pTeamData->TriggersSideIdx = -1;
		pTeamData->TriggersHouseIdx = -1;
		auto pScript = pTeam->CurrentScript;
		bool isEnabled = false;

		if (enabled < 0)
			enabled = pScript->Type->ScriptActions[pScript->CurrentMission].Argument;

		if (enabled >= 1)
			isEnabled = true;

		ScriptExt::ManageAllTriggersFromHouse(pTeam, nullptr, sideIdx, houseIdx, isEnabled);
	}

	// This action finished
	pTeam->StepCompleted = true;
}

void ScriptExt::ManageTriggersWithObjects(TeamClass* pTeam, int idxAITargetType = -1, bool isEnabled = false)
{
	auto pScript = pTeam->CurrentScript;

	if (idxAITargetType < 0)
		idxAITargetType = pScript->Type->ScriptActions[pScript->CurrentMission].Argument;

	if (idxAITargetType < 0)
		return;

	if (RulesExt::Global()->AITargetTypesLists.Count <= 0)
		return;

	DynamicVectorClass<TechnoTypeClass*> objectsList = RulesExt::Global()->AITargetTypesLists.GetItem(idxAITargetType);

	if (objectsList.Count == 0)
		return;

	for (auto pTrigger : *AITriggerTypeClass::Array)
	{
		DynamicVectorClass<TechnoTypeClass*> entriesList;

		if (pTrigger->Team1)
		{
			for (auto entry : pTrigger->Team1->TaskForce->Entries)
			{
				if (entry.Amount > 0)
				{
					entriesList.AddItem(entry.Type);
				}
			}
		}

		if (pTrigger->Team2)
		{
			for (auto entry : pTrigger->Team2->TaskForce->Entries)
			{
				if (entry.Amount > 0)
				{
					entriesList.AddItem(entry.Type);
				}
			}
		}

		if (entriesList.Count > 0)
		{
			for (auto entry : entriesList)
			{
				if (objectsList.FindItemIndex(entry) >= 0)
				{
					pTrigger->IsEnabled = isEnabled;
					break;
				}
			}
		}
	}

	// This action finished
	pTeam->StepCompleted = true;
}

// 1-based like the original action '6,n' (so the first script line is n=1)
void ScriptExt::ConditionalJumpIfTrue(TeamClass* pTeam, int newScriptLine = -1)
{
	if (!pTeam)
	{
		// This action finished
		pTeam->StepCompleted = true;

		return;
	}

	auto pTeamData = TeamExt::ExtMap.Find(pTeam);
	if (!pTeamData)
	{
		// This action finished
		pTeam->StepCompleted = true;

		return;
	}

	auto pScript = pTeam->CurrentScript;
	int scriptArgument = newScriptLine;

	if (scriptArgument < 1)
		scriptArgument = pScript->Type->ScriptActions[pScript->CurrentMission].Argument;

	// if by mistake you put as first line=0 this corrects it because for WW/EALA this script argument is 1-based
	if (scriptArgument < 1)
		scriptArgument = 1;

	if (pTeamData->ConditionalJump_Evaluation)
	{
		Debug::Log("DEBUG: [%s] [%s] %d = %d,%d - Conditional Jump was a success! - New Line: %d = %d,%d\n", pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission, pScript->Type->ScriptActions[pScript->CurrentMission].Action, pScript->Type->ScriptActions[pScript->CurrentMission].Argument, scriptArgument - 1, pScript->Type->ScriptActions[scriptArgument - 1].Action, pScript->Type->ScriptActions[scriptArgument - 1].Argument);

		// Start conditional jump!
		// This is magic: for example, for jumping into line 0 of the script list you have to point to the "-1" line so in the next AI iteration the current line will be increased by 1 and then it will point to the desired line 0
		pScript->CurrentMission = scriptArgument - 2;

		// Cleaning Conditional Jump related variables
		if (pTeamData->ConditionalJump_ResetVariablesIfJump)
			ScriptExt::ConditionalJump_ResetVariables(pTeam);
	}

	// This action finished
	pTeam->StepCompleted = true;

	return;
}

// 1-based like the original action '6,n' (so the first script line is n=1)
void ScriptExt::ConditionalJumpIfFalse(TeamClass* pTeam, int newScriptLine = -1)
{
	if (!pTeam)
	{
		// This action finished
		pTeam->StepCompleted = true;

		return;
	}

	auto pTeamData = TeamExt::ExtMap.Find(pTeam);
	if (!pTeamData)
	{
		// This action finished
		pTeam->StepCompleted = true;

		return;
	}

	auto pScript = pTeam->CurrentScript;
	int scriptArgument = newScriptLine;

	if (scriptArgument < 1)
		scriptArgument = pScript->Type->ScriptActions[pScript->CurrentMission].Argument;

	// if by mistake you put as first line=0 this corrects it because for WW/EALA this script argument is 1-based
	if (scriptArgument < 1)
		scriptArgument = 1;

	if (!pTeamData->ConditionalJump_Evaluation)
	{
		Debug::Log("DEBUG: [%s] [%s] %d = %d,%d - Conditional Jump was a success! - New Line: %d = %d,%d\n", pTeam->Type->ID, pScript->Type->ID, pScript->CurrentMission, pScript->Type->ScriptActions[pScript->CurrentMission].Action, pScript->Type->ScriptActions[pScript->CurrentMission].Argument, scriptArgument - 1, pScript->Type->ScriptActions[scriptArgument - 1].Action, pScript->Type->ScriptActions[scriptArgument - 1].Argument);

		// Start conditional jump!
		// This is magic: for example, for jumping into line 0 of the script list you have to point to the "-1" line so in the next AI iteration the current line will be increased by 1 and then it will point to the desired line 0
		pScript->CurrentMission = scriptArgument - 2;

		// Cleaning Conditional Jump related variables
		if (pTeamData->ConditionalJump_ResetVariablesIfJump)
			ScriptExt::ConditionalJump_ResetVariables(pTeam);
	}

	// This action finished
	pTeam->StepCompleted = true;

	return;
}

void ScriptExt::ConditionalJump_KillEvaluation(TeamClass* pTeam)
{
	if (!pTeam)
	{
		// This action finished
		pTeam->StepCompleted = true;

		return;
	}

	auto pTeamData = TeamExt::ExtMap.Find(pTeam);
	if (!pTeamData)
	{
		// This action finished
		pTeam->StepCompleted = true;

		return;
	}

	if (!pTeamData->ConditionalJump_EnabledKillsCount)
		return;

	if (pTeamData->ConditionalJump_Counter < 0)
		pTeamData->ConditionalJump_Counter = 0;

	int counter = pTeamData->ConditionalJump_Counter;
	int comparator = pTeamData->ConditionalJump_ComparatorValue;
	pTeamData->ConditionalJump_Evaluation = ScriptExt::ConditionalJump_MakeEvaluation(pTeamData->ConditionalJump_ComparatorMode, counter, comparator);

	// This action finished
	pTeam->StepCompleted = true;

	return;
}

void ScriptExt::ConditionalJump_ManageKillsCounter(TeamClass* pTeam, int enable = -1)
{
	if (!pTeam)
	{
		// This action finished
		pTeam->StepCompleted = true;

		return;
	}

	auto pTeamData = TeamExt::ExtMap.Find(pTeam);
	if (!pTeamData)
	{
		// This action finished
		pTeam->StepCompleted = true;

		return;
	}

	auto pScript = pTeam->CurrentScript;
	int scriptArgument = enable;

	if (scriptArgument < 0 || scriptArgument > 1)
		scriptArgument = pScript->Type->ScriptActions[pScript->CurrentMission].Argument;

	if (scriptArgument <= 0)
		scriptArgument = 0;
	else
		scriptArgument = 1;

	if (scriptArgument <= 0)
		pTeamData->ConditionalJump_EnabledKillsCount = false;
	else
		pTeamData->ConditionalJump_EnabledKillsCount = true;

	// This action finished
	pTeam->StepCompleted = true;

	return;
}

void ScriptExt::ConditionalJump_SetIndex(TeamClass* pTeam, int index = -1000000)
{
	if (!pTeam)
	{
		// This action finished
		pTeam->StepCompleted = true;

		return;
	}

	auto pTeamData = TeamExt::ExtMap.Find(pTeam);
	if (!pTeamData)
	{
		// This action finished
		pTeam->StepCompleted = true;

		return;
	}

	auto pScript = pTeam->CurrentScript;
	int scriptArgument = index;

	if (scriptArgument == -1000000)
		scriptArgument = pScript->Type->ScriptActions[pScript->CurrentMission].Argument;

	//	if (scriptArgument < 0)
	//		scriptArgument = -1;

	pTeamData->ConditionalJump_Index = scriptArgument;

	// This action finished
	pTeam->StepCompleted = true;

	return;
}

void ScriptExt::ConditionalJump_SetComparatorValue(TeamClass* pTeam, int value = -1)
{
	if (!pTeam)
	{
		// This action finished
		pTeam->StepCompleted = true;

		return;
	}

	auto pTeamData = TeamExt::ExtMap.Find(pTeam);
	if (!pTeamData)
	{
		// This action finished
		pTeam->StepCompleted = true;

		return;
	}

	auto pScript = pTeam->CurrentScript;
	int scriptArgument = value;

	if (scriptArgument < 0)
		scriptArgument = pScript->Type->ScriptActions[pScript->CurrentMission].Argument;

	//	if (scriptArgument < 0)
	//		scriptArgument = 0;

	pTeamData->ConditionalJump_ComparatorValue = scriptArgument;

	// This action finished
	pTeam->StepCompleted = true;

	return;
}

// Possible values are 3:">=" -> 0:"<", 1:"<=", 2:"==", 3:">=", 4:">", 5:"!="
void ScriptExt::ConditionalJump_SetComparatorMode(TeamClass* pTeam, int value = -1)
{
	if (!pTeam)
	{
		// This action finished
		pTeam->StepCompleted = true;

		return;
	}

	auto pTeamData = TeamExt::ExtMap.Find(pTeam);
	if (!pTeamData)
	{
		// This action finished
		pTeam->StepCompleted = true;

		return;
	}

	auto pScript = pTeam->CurrentScript;
	int scriptArgument = value;

	if (scriptArgument < 0 || scriptArgument > 5)
		scriptArgument = pScript->Type->ScriptActions[pScript->CurrentMission].Argument;

	if (scriptArgument < 0 || scriptArgument > 5)
		scriptArgument = 3; // >=

	pTeamData->ConditionalJump_ComparatorMode = scriptArgument;

	// This action finished
	pTeam->StepCompleted = true;

	return;
}

void ScriptExt::ConditionalJump_SetCounter(TeamClass* pTeam, int value = -100000000)
{
	if (!pTeam)
	{
		// This action finished
		pTeam->StepCompleted = true;

		return;
	}

	auto pTeamData = TeamExt::ExtMap.Find(pTeam);
	if (!pTeamData)
	{
		// This action finished
		pTeam->StepCompleted = true;

		return;
	}

	auto pScript = pTeam->CurrentScript;

	if (value == -100000000)
		value = pScript->Type->ScriptActions[pScript->CurrentMission].Argument;

	pTeamData->ConditionalJump_Counter = value;

	// This action finished
	pTeam->StepCompleted = true;

	return;
}

void ScriptExt::ConditionalJump_ResetVariables(TeamClass* pTeam)
{
	if (!pTeam)
	{
		// This action finished
		pTeam->StepCompleted = true;

		return;
	}

	auto pTeamData = TeamExt::ExtMap.Find(pTeam);
	if (!pTeamData)
	{
		// This action finished
		pTeam->StepCompleted = true;

		return;
	}

	// Cleaning Conditional Jump related variables
	pTeamData->ConditionalJump_Evaluation = false;
	pTeamData->ConditionalJump_ComparatorMode = 3; // >=
	pTeamData->ConditionalJump_ComparatorValue = 1;
	pTeamData->ConditionalJump_EnabledKillsCount = false;
	pTeamData->ConditionalJump_Counter = 0;
	pTeamData->AbortActionAfterKilling = false;
	pTeamData->ConditionalJump_Index = -1000000;

	// This action finished
	pTeam->StepCompleted = true;

	return;
}

void ScriptExt::ConditionalJump_ManageResetIfJump(TeamClass* pTeam, int enable = -1)
{
	if (!pTeam)
	{
		// This action finished
		pTeam->StepCompleted = true;

		return;
	}

	auto pTeamData = TeamExt::ExtMap.Find(pTeam);
	if (!pTeamData)
	{
		// This action finished
		pTeam->StepCompleted = true;

		return;
	}

	auto pScript = pTeam->CurrentScript;

	if (enable < 0)
		enable = pScript->Type->ScriptActions[pScript->CurrentMission].Argument;

	if (enable > 0)
		pTeamData->ConditionalJump_ResetVariablesIfJump = true;
	else
		pTeamData->ConditionalJump_ResetVariablesIfJump = false;

	// This action finished
	pTeam->StepCompleted = true;

	return;
}

void ScriptExt::SetAbortActionAfterSuccessKill(TeamClass* pTeam, int enable = -1)
{
	if (!pTeam)
	{
		// This action finished
		pTeam->StepCompleted = true;

		return;
	}

	auto pTeamData = TeamExt::ExtMap.Find(pTeam);
	if (!pTeamData)
	{
		// This action finished
		pTeam->StepCompleted = true;

		return;
	}

	int scriptArgument = enable;
	if (scriptArgument < 0)
	{
		auto pScript = pTeam->CurrentScript;
		scriptArgument = pScript->Type->ScriptActions[pScript->CurrentMission].Argument;
	}

	if (scriptArgument >= 1)
		pTeamData->AbortActionAfterKilling = true;
	else
		pTeamData->AbortActionAfterKilling = false;

	// This action finished
	pTeam->StepCompleted = true;

	return;
}

// Count objects from [AITargetTypes] lists
void ScriptExt::ConditionalJump_CheckObjects(TeamClass* pTeam)
{
	long countValue = 0;

	if (!pTeam)
	{
		// This action finished
		pTeam->StepCompleted = true;

		return;
	}

	auto pTeamData = TeamExt::ExtMap.Find(pTeam);
	if (!pTeamData)
	{
		// This action finished
		pTeam->StepCompleted = true;

		return;
	}

	int index = pTeamData->ConditionalJump_Index;

	if (index >= 0 && RulesExt::Global()->AITargetTypesLists.Count > 0)
	{
		DynamicVectorClass<TechnoTypeClass*> objectsList = RulesExt::Global()->AITargetTypesLists.GetItem(index);

		if (objectsList.Count == 0)
			return;

		for (auto const pTechno : *TechnoClass::Array())
		{
			if (auto pTechnoType = pTechno->GetTechnoType())
			{
				if (pTechno->IsAlive
					&& pTechno->Health > 0
					&& !pTechno->InLimbo
					&& pTechno->IsOnMap
					&& (!pTeam->FirstUnit->Owner->IsAlliedWith(pTechno)
						|| (pTeam->FirstUnit->Owner->IsAlliedWith(pTechno)
							&& pTechno->IsMindControlled()
							&& !pTeam->FirstUnit->Owner->IsAlliedWith(pTechno->MindControlledBy))))
				{
					for (int i = 0; i < objectsList.Count; i++)
					{
						if (objectsList.GetItem(i) == pTechnoType)
						{
							countValue++;
							break;
						}
					}
				}
			}
		}

		int comparatorValue = pTeamData->ConditionalJump_ComparatorValue;
		pTeamData->ConditionalJump_Evaluation = ScriptExt::ConditionalJump_MakeEvaluation(pTeamData->ConditionalJump_ComparatorMode, countValue, comparatorValue);
	}

	// This action finished
	pTeam->StepCompleted = true;
}

// A simple counter. The count can be increased or decreased
void ScriptExt::ConditionalJump_CheckCount(TeamClass* pTeam, int modifier = 0)
{
	if (!pTeam)
	{
		// This action finished
		pTeam->StepCompleted = true;

		return;
	}

	auto pTeamData = TeamExt::ExtMap.Find(pTeam);
	if (!pTeamData)
	{
		// This action finished
		pTeam->StepCompleted = true;

		return;
	}
	auto pScript = pTeam->CurrentScript;

	if (modifier == 0)
		modifier = pScript->Type->ScriptActions[pScript->CurrentMission].Argument;

	if (modifier == 0)
	{
		// This action finished
		pTeam->StepCompleted = true;
		return;
	}

	pTeamData->ConditionalJump_Counter += modifier;
	int currentCount = pTeamData->ConditionalJump_Counter;
	int comparatorValue = pTeamData->ConditionalJump_ComparatorValue;
	pTeamData->ConditionalJump_Evaluation = ScriptExt::ConditionalJump_MakeEvaluation(pTeamData->ConditionalJump_ComparatorMode, currentCount, comparatorValue);

	// This action finished
	pTeam->StepCompleted = true;
}

bool ScriptExt::ConditionalJump_MakeEvaluation(int comparatorMode, int studiedValue, int comparatorValue)
{
	int result = false;

	// Comparators are like in [AITriggerTypes] from aimd.ini
	switch (comparatorMode)
	{
	case 0:
		// <
		if (studiedValue < comparatorValue)
			result = true;
		break;
	case 1:
		// <=
		if (studiedValue <= comparatorValue)
			result = true;
		break;
	case 2:
		// ==
		if (studiedValue == comparatorValue)
			result = true;
		break;
	case 3:
		// >=
		if (studiedValue >= comparatorValue)
			result = true;
		break;
	case 4:
		// >
		if (studiedValue > comparatorValue)
			result = true;
		break;
	case 5:
		// !=
		if (studiedValue != comparatorValue)
			result = true;
		break;
	default:
		break;
	}

	return result;
}

void ScriptExt::ConditionalJump_CheckHumanIsMostHated(TeamClass* pTeam)
{
	if (!pTeam)
	{
		// This action finished
		pTeam->StepCompleted = true;

		return;
	}

	auto pTeamData = TeamExt::ExtMap.Find(pTeam);
	if (!pTeamData)
	{
		// This action finished
		pTeam->StepCompleted = true;

		return;
	}

	HouseClass* pEnemyHouse = nullptr;

	if (auto pHouse = pTeam->Owner)
	{
		int angerLevel = -1;
		bool isHumanHouse = false;

		for (auto pNode : pHouse->AngerNodes)
		{
			if (!pNode.House->Type->MultiplayPassive
				&& !pNode.House->Defeated
				&& !pNode.House->IsObserver()
				&& ((pNode.AngerLevel > angerLevel
					&& !pHouse->IsAlliedWith(pNode.House))
					|| angerLevel < 0))
			{
				angerLevel = pNode.AngerLevel;
				pEnemyHouse = pNode.House;
			}
		}

		if (pEnemyHouse && pEnemyHouse->IsControlledByHuman())
		{
			isHumanHouse = true;
		}

		pTeamData->ConditionalJump_Evaluation = isHumanHouse;
	}

	// This action finished
	pTeam->StepCompleted = true;
}

void ScriptExt::ConditionalJump_CheckAliveHumans(TeamClass* pTeam, int mode = 0)
{
	if (!pTeam)
	{
		// This action finished
		pTeam->StepCompleted = true;

		return;
	}

	auto pTeamData = TeamExt::ExtMap.Find(pTeam);
	if (!pTeamData)
	{
		// This action finished
		pTeam->StepCompleted = true;

		return;
	}

	if (mode < 0 || mode > 2)
		mode = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument;

	if (mode < 0 || mode > 2)
		mode = 0;

	if (auto pHouse = pTeam->Owner)
	{
		pTeamData->ConditionalJump_Evaluation = false;

		// Find an alive human House
		for (auto pNode : pHouse->AngerNodes)
		{
			if (!pNode.House->Type->MultiplayPassive
				&& !pNode.House->Defeated
				&& !pNode.House->IsObserver()
				&& pNode.House->IsControlledByHuman())
			{
				if (mode == 1 && !pHouse->IsAlliedWith(pNode.House)) // Mode 1: Enemy humans
				{
					pTeamData->ConditionalJump_Evaluation = true;
					break;
				}
				else if (mode == 2 && !pHouse->IsAlliedWith(pNode.House)) // Mode 2: Friendly humans
				{
					pTeamData->ConditionalJump_Evaluation = true;
					break;
				}

				// mode 0: Any human
				pTeamData->ConditionalJump_Evaluation = true;
				break;
			}
		}

		// If we are looking for any human the own House should be checked
		if (mode == 0 && pHouse->IsControlledByHuman())
			pTeamData->ConditionalJump_Evaluation = true;
	}

	// This action finished
	pTeam->StepCompleted = true;
}

void ScriptExt::JumpBackToPreviousScript(TeamClass* pTeam)
{
	auto pTeamData = TeamExt::ExtMap.Find(pTeam);
	if (!pTeamData->PreviousScriptList.empty())
	{
		pTeam->CurrentScript = nullptr;
		pTeam->CurrentScript = pTeamData->PreviousScriptList.back();
		pTeamData->PreviousScriptList.pop_back();
		pTeam->StepCompleted = true;
		return;
	}
	else
	{
		Debug::Log("DEBUG: Can't find the previous script! This script action must be used after PickRandomScript.\n");
		pTeam->StepCompleted = true;
		return;
	}
}

void TransportsReturn(TeamClass* pTeam, FootClass* pTransport)
{
	if (pTeam->Type->TransportsReturnOnUnload)
	{
		if (!pTransport->GetTechnoType()->HasPrimary)
		{
			auto pScript = pTeam->CurrentScript;
			Debug::Log("DEBUG: [%s] [%s](line: %d = %d,%d): Transport [%s] (UID: %lu) has no weapon and thus cannot return.\n",
				pTeam->Type->ID,
				pScript->Type->ID,
				pScript->CurrentMission,
				pScript->Type->ScriptActions[pScript->CurrentMission].Action,
				pScript->Type->ScriptActions[pScript->CurrentMission].Argument,
				pTransport->GetTechnoType()->get_ID(),
				pTransport->UniqueID);
		}
		pTransport->SetDestination(pTeam->SpawnCell, false);
		pTransport->QueueMission(Mission::Move, false);
	}
	return;
}

void ScriptExt::UnloadFromTransports(TeamClass* pTeam)
{
	if (!pTeam)
		return;

	auto pTeamData = TeamExt::ExtMap.Find(pTeam);
	if (!pTeamData)
		return;

	bool stillMoving = false;
	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
	{
		if (pUnit->CurrentMission == Mission::Move || (pUnit->Locomotor->Is_Moving() && !pUnit->GetTechnoType()->JumpJet))
		{
			pUnit->ForceMission(Mission::Wait);
			pUnit->CurrentTargets.Clear();
			pUnit->SetTarget(nullptr);
			pUnit->SetFocus(nullptr);
			pUnit->SetDestination(nullptr, true);
			stillMoving = true;
		}
	}
	if (stillMoving)
	{
		pTeam->GuardAreaTimer.Start(45);
		return;
	}

	double maxSizeLimit = 0;
	DynamicVectorClass<FootClass*> transports;
	DynamicVectorClass<FootClass*> AllPassengers;
	int argument = pTeam->CurrentScript->Type->ScriptActions[pTeam->CurrentScript->CurrentMission].Argument;

	if (argument > 3)
	{
		for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
		{
			maxSizeLimit = std::max(maxSizeLimit, pUnit->GetTechnoType()->SizeLimit);
		}
		for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
		{
			if (!pUnit->GetTechnoType()->OpenTopped
				&& !pUnit->GetTechnoType()->Gunner
				&& pUnit->Passengers.NumPassengers > 0
				&& pUnit->GetTechnoType()->SizeLimit == maxSizeLimit) //Battle Fortress and IFV are not transports.
			{
				AllPassengers.AddItem(pUnit->Passengers.FirstPassenger);
				for (NextObject i(pUnit->Passengers.FirstPassenger->NextObject); i && abstract_cast<FootClass*>(*i); i++)
				{
					auto passenger = static_cast<FootClass*>(*i);
					AllPassengers.AddItem(passenger);
				}
				pUnit->QueueMission(Mission::Unload, true);
			}
		}
	}
	else
	{
		for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
		{
			if (!pUnit->GetTechnoType()->OpenTopped && !pUnit->GetTechnoType()->Gunner && pUnit->Passengers.NumPassengers > 0) //Battle Fortress and IFV are not transports.
			{
				AllPassengers.AddItem(pUnit->Passengers.FirstPassenger);
				for (NextObject i(pUnit->Passengers.FirstPassenger->NextObject); i && abstract_cast<FootClass*>(*i); i++)
				{
					auto passenger = static_cast<FootClass*>(*i);
					AllPassengers.AddItem(passenger);
				}
				pUnit->QueueMission(Mission::Unload, true);
			}
		}
	}

	if (pTeamData->AllPassengers.Count == 0)
		pTeamData->AllPassengers = AllPassengers;

	//unload in progress
	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
		if (pUnit->GetCurrentMission() == Mission::Unload)
			return;

	for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
	{
		if (!pUnit->GetTechnoType()->OpenTopped && !pUnit->GetTechnoType()->Gunner && pUnit->GetTechnoType()->Passengers > 0) //Battle Fortress and IFV are not transports.
		{
			transports.AddItem(pUnit);
		}
	}

	//no valid transports
	if (transports.Count == 0)
	{
		pTeam->StepCompleted = true;
		return;
	}

	//Save all
	if (argument == 0 || argument == 4)
	{
		for (int i = 0; i < pTeamData->AllPassengers.Count; i++)
		{
			auto pFoot = (pTeamData->AllPassengers)[i];

			// Must be owner
			if (pFoot
				&& !pFoot->InLimbo && pFoot->Health > 0
				&& pFoot->Owner == pTeam->Owner)
			{
				pTeam->AddMember(pFoot, true);
			}
		}
	}

	//Liberate passengers
	else if (argument == 1)
	{
		for (auto pPassengers : pTeamData->AllPassengers)
		{
			pTeam->LiberateMember(pPassengers);
		}
	}
	//Liberate transports
	else if (argument == 2)
	{
		for (auto pTransport : transports)
		{
			TransportsReturn(pTeam, pTransport);
			pTeam->LiberateMember(pTransport);
		}
		for (int i = 0; i < pTeamData->AllPassengers.Count; i++)
		{
			auto pFoot = (pTeamData->AllPassengers)[i];

			// Must be owner
			if (pFoot
				&& !pFoot->InLimbo && pFoot->Health > 0
				&& pFoot->Owner == pTeam->Owner)
			{
				pTeam->AddMember(pFoot, true);
			}
		}
	}
	//Liberate whole team
	else if (argument == 3)
	{
		for (auto pTransport : transports)
		{
			TransportsReturn(pTeam, pTransport);
		}
		for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
		{
			pTeam->LiberateMember(pUnit);
			//this team is over :)
			pTeam->CurrentScript->ClearMission();
		}
	}
	//Liberate passengers and transports that don't have a maxSizeLimit.
	else if (argument == 5)
	{
		for (auto pTransport : transports)
		{
			if (pTransport->GetTechnoType()->SizeLimit != maxSizeLimit)
			{
				pTeam->LiberateMember(pTransport);
			}
		}
		for (auto pPassengers : pTeamData->AllPassengers)
		{
			pTeam->LiberateMember(pPassengers);
		}
	}
	//Liberate transports that have a maxSizeLimit.
	else if (argument == 6)
	{
		for (auto pTransport : transports)
		{
			if (pTransport->GetTechnoType()->SizeLimit == maxSizeLimit)
			{
				TransportsReturn(pTeam, pTransport);
				pTeam->LiberateMember(pTransport);
			}
			for (int i = 0; i < pTeamData->AllPassengers.Count; i++)
			{
				auto pFoot = (pTeamData->AllPassengers)[i];

				// Must be owner
				if (pFoot
					&& !pFoot->InLimbo && pFoot->Health > 0
					&& pFoot->Owner == pTeam->Owner)
				{
					pTeam->AddMember(pFoot, true);
				}
			}
		}
	}
	//Liberate whole team. Only transports with maxSizeLimit will return.
	else if (argument == 7)
	{
		for (auto pTransport : transports)
		{
			if (pTransport->GetTechnoType()->SizeLimit == maxSizeLimit)
			{
				TransportsReturn(pTeam, pTransport);
			}
		}
		for (auto pUnit = pTeam->FirstUnit; pUnit; pUnit = pUnit->NextTeamMember)
		{
			pTeam->LiberateMember(pUnit);
			//this team is over :)
			pTeam->CurrentScript->ClearMission();
		}
	}

	// This action finished
	pTeamData->AllPassengers.Clear();
	pTeam->StepCompleted = true;
	return;
}
