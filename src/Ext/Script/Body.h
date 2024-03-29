#pragma once

#include <ScriptClass.h>

#include <Utilities/Container.h>

enum class PhobosScripts : unsigned int
{
	TimedAreaGuard = 71,
	LoadIntoTransports = 72,
	WaitUntilFullAmmo = 73,
	RepeatAttackCloserThreat = 74,
	RepeatAttackFartherThreat = 75,
	RepeatAttackCloser = 76,
	RepeatAttackFarther = 77,
	SingleAttackCloserThreat = 78,
	SingleAttackFartherThreat = 79,
	SingleAttackCloser = 80,
	SingleAttackFarther = 81,
	DecreaseCurrentAITriggerWeight = 82,
	IncreaseCurrentAITriggerWeight = 83,
	RepeatAttackTypeCloserThreat = 84,
	RepeatAttackTypeFartherThreat = 85,
	RepeatAttackTypeCloser = 86,
	RepeatAttackTypeFarther = 87,
	SingleAttackTypeCloserThreat = 88,
	SingleAttackTypeFartherThreat = 89,
	SingleAttackTypeCloser = 90,
	SingleAttackTypeFarther = 91,
	WaitIfNoTarget = 92,
	TeamWeightReward = 93,
	PickRandomScript = 94,
	MoveToEnemyCloser = 95,
	MoveToEnemyFarther = 96,
	MoveToFriendlyCloser = 97,
	MoveToFriendlyFarther = 98,
	MoveToTypeEnemyCloser = 99,
	MoveToTypeEnemyFarther = 100,
	MoveToTypeFriendlyCloser = 101,
	MoveToTypeFriendlyFarther = 102,
	ModifyTargetDistance = 103,
	RandomAttackTypeCloser = 104,
	RandomAttackTypeFarther = 105,
	RandomMoveToTypeEnemyCloser = 106,
	RandomMoveToTypeEnemyFarther = 107,
	RandomMoveToTypeFriendlyCloser = 108,
	RandomMoveToTypeFriendlyFarther = 109,
	SetMoveMissionEndMode = 110,
	UnregisterGreatSuccess = 111,
	GatherAroundLeader = 112,
	RandomSkipNextAction = 113,
	SetHouseAngerModifier = 114,
	OverrideOnlyTargetHouseEnemy = 115,
	ModifyHateHouseIndex = 116,
	ModifyHateHousesList = 117,
	ModifyHateHousesList1Random = 118,
	SetTheMostHatedHouseMinorNoRandom = 119,
	SetTheMostHatedHouseMajorNoRandom = 120,
	SetTheMostHatedHouseRandom = 121,
	ResetAngerAgainstHouses = 122,
	AggroHouse = 123,
	StopForceJumpCountdown = 124,
	NextLineForceJumpCountdown = 125,
	SameLineForceJumpCountdown = 126,
	SetSideIdxForManagingTriggers = 127,
	SetHouseIdxForManagingTriggers = 128,
	ManageAllAITriggers = 129,
	EnableTriggersFromList = 130,
	DisableTriggersFromList = 131,
	EnableTriggersWithObjects = 132,
	DisableTriggersWithObjects = 133,
	AbortActionAfterSuccessKill = 134,
	ConditionalJumpSetCounter = 135,
	ConditionalJumpSetComparatorMode = 136,
	ConditionalJumpSetComparatorValue = 137,
	ConditionalJumpSetIndex = 138,
	ConditionalJumpResetVariables = 139,
	ConditionalJumpIfFalse = 140,
	ConditionalJumpIfTrue = 141,
	ConditionalJumpManageKillsCounter = 142,
	ConditionalJumpCheckAliveHumans = 143,
	ConditionalJumpCheckHumanIsMostHated = 144,
	ConditionalJumpKillEvaluation = 145,
	ConditionalJumpCheckObjects = 146,
	ConditionalJumpCheckCount = 147,
	ConditionalJumpManageResetIfJump = 148,
	ForceGlobalOnlyTargetHouseEnemy = 150,
	JumpBackToPreviousScript = 151,
	UnloadFromTransports = 152,

	// Variables
	LocalVariableSet = 500,
	LocalVariableAdd = 501,
	LocalVariableMinus = 502,
	LocalVariableMultiply = 503,
	LocalVariableDivide = 504,
	LocalVariableMod = 505,
	LocalVariableLeftShift = 506,
	LocalVariableRightShift = 507,
	LocalVariableReverse = 508,
	LocalVariableXor = 509,
	LocalVariableOr = 510,
	LocalVariableAnd = 511,
	GlobalVariableSet = 512,
	GlobalVariableAdd = 513,
	GlobalVariableMinus = 514,
	GlobalVariableMultiply = 515,
	GlobalVariableDivide = 516,
	GlobalVariableMod = 517,
	GlobalVariableLeftShift = 518,
	GlobalVariableRightShift = 519,
	GlobalVariableReverse = 520,
	GlobalVariableXor = 521,
	GlobalVariableOr = 522,
	GlobalVariableAnd = 523,
	LocalVariableSetByLocal = 524,
	LocalVariableAddByLocal = 525,
	LocalVariableMinusByLocal = 526,
	LocalVariableMultiplyByLocal = 527,
	LocalVariableDivideByLocal = 528,
	LocalVariableModByLocal = 529,
	LocalVariableLeftShiftByLocal = 530,
	LocalVariableRightShiftByLocal = 531,
	LocalVariableReverseByLocal = 532,
	LocalVariableXorByLocal = 533,
	LocalVariableOrByLocal = 534,
	LocalVariableAndByLocal = 535,
	GlobalVariableSetByLocal = 536,
	GlobalVariableAddByLocal = 537,
	GlobalVariableMinusByLocal = 538,
	GlobalVariableMultiplyByLocal = 539,
	GlobalVariableDivideByLocal = 540,
	GlobalVariableModByLocal = 541,
	GlobalVariableLeftShiftByLocal = 542,
	GlobalVariableRightShiftByLocal = 543,
	GlobalVariableReverseByLocal = 544,
	GlobalVariableXorByLocal = 545,
	GlobalVariableOrByLocal = 546,
	GlobalVariableAndByLocal = 547,
	LocalVariableSetByGlobal = 548,
	LocalVariableAddByGlobal = 549,
	LocalVariableMinusByGlobal = 550,
	LocalVariableMultiplyByGlobal = 551,
	LocalVariableDivideByGlobal = 552,
	LocalVariableModByGlobal = 553,
	LocalVariableLeftShiftByGlobal = 554,
	LocalVariableRightShiftByGlobal = 555,
	LocalVariableReverseByGlobal = 556,
	LocalVariableXorByGlobal = 557,
	LocalVariableOrByGlobal = 558,
	LocalVariableAndByGlobal = 559,
	GlobalVariableSetByGlobal = 560,
	GlobalVariableAddByGlobal = 561,
	GlobalVariableMinusByGlobal = 562,
	GlobalVariableMultiplyByGlobal = 563,
	GlobalVariableDivideByGlobal = 564,
	GlobalVariableModByGlobal = 565,
	GlobalVariableLeftShiftByGlobal = 566,
	GlobalVariableRightShiftByGlobal = 567,
	GlobalVariableReverseByGlobal = 568,
	GlobalVariableXorByGlobal = 569,
	GlobalVariableOrByGlobal = 570,
	GlobalVariableAndByGlobal = 571,
	ChangeTeamGroup = 572,
	DistributedLoading = 573,
	FollowFriendlyByGroup = 574,
	FollowEnemyByGroup = 575,
};

class FootClass;
class TeamClass;

class ScriptExt
{
public:
	using base_type = ScriptClass;

	static constexpr DWORD Canary = 0x3B3B3B3B;

	class ExtData final : public Extension<ScriptClass>
	{
	public:
		// Nothing yet

		ExtData(ScriptClass* OwnerObject) : Extension<ScriptClass>(OwnerObject)
			// Nothing yet
		{
		}

		virtual ~ExtData() = default;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }

		virtual void LoadFromStream(PhobosStreamReader& Stm);
		virtual void SaveToStream(PhobosStreamWriter& Stm);

	};

	class ExtContainer final : public Container<ScriptExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static void ProcessAction(TeamClass* pTeam);
	static void ExecuteTimedAreaGuardAction(TeamClass* pTeam);
	static void LoadIntoTransports(TeamClass* pTeam);
	static void WaitUntilFullAmmoAction(TeamClass* pTeam);
	static void Mission_Gather_NearTheLeader(TeamClass* pTeam, int countdown);
	static void Mission_Attack(TeamClass* pTeam, bool repeatAction, int calcThreatMode, int attackAITargetType, int IdxAITargetTypeItem);
	static TechnoClass* GreatestThreat(TechnoClass* pTechno, int method, int calcThreatMode, HouseClass* onlyTargetThisHouseEnemy, int attackAITargetType, int idxAITargetTypeItem, bool agentMode);
	static bool EvaluateObjectWithMask(TechnoClass* pTechno, int mask, int attackAITargetType, int idxAITargetTypeItem, TechnoClass* pTeamLeader);

	static void DecreaseCurrentTriggerWeight(TeamClass* pTeam, bool forceJumpLine, double modifier);
	static void IncreaseCurrentTriggerWeight(TeamClass* pTeam, bool forceJumpLine, double modifier);
	static void WaitIfNoTarget(TeamClass* pTeam, int attempts);
	static void TeamWeightReward(TeamClass* pTeam, double award);
	static void PickRandomScript(TeamClass* pTeam, int idxScriptsList);
	static void Mission_Move(TeamClass* pTeam, int calcThreatMode, bool pickAllies, int attackAITargetType, int idxAITargetTypeItem);
	static TechnoClass* FindBestObject(TechnoClass* pTechno, int method, int calcThreatMode, bool pickAllies, int attackAITargetType, int idxAITargetTypeItem);
	static void UnregisterGreatSuccess(TeamClass* pTeam);

	static void TeamMemberSetGroup(TeamClass* pTeam, int group);

	static void DistributedLoadOntoTransport(TeamClass* pTeam, bool loadOnlyFirstLevel);

	static bool IsValidFriendlyTarget(TeamClass* pTeam, int group, TechnoClass* target, bool isSelfNaval, bool isSelfAircraft, bool isFriendly);

	static void FollowTargetByGroup(TeamClass* pTeam, int group, bool isFriendly);

	static void ConditionalJumpIfTrue(TeamClass* pTeam, int newScriptLine);
	static void ConditionalJumpIfFalse(TeamClass* pTeam, int newScriptLine);
	static void ConditionalJump_KillEvaluation(TeamClass* pTeam);
	static void ConditionalJump_ManageKillsCounter(TeamClass* pTeam, int enable);
	static void ConditionalJump_SetIndex(TeamClass* pTeam, int index);
	static void ConditionalJump_SetComparatorValue(TeamClass* pTeam, int value);
	static void ConditionalJump_SetComparatorMode(TeamClass* pTeam, int value);
	static void ConditionalJump_SetCounter(TeamClass* pTeam, int value);
	static void SetAbortActionAfterSuccessKill(TeamClass* pTeam, int enable);
	static void ConditionalJump_ResetVariables(TeamClass* pTeam);
	static void ConditionalJump_CheckHumanIsMostHated(TeamClass* pTeam);
	static void ConditionalJump_CheckAliveHumans(TeamClass* pTeam, int mode);
	static void ConditionalJump_CheckObjects(TeamClass* pTeam);
	static void ConditionalJump_CheckCount(TeamClass* pTeam, int modifier);
	static void ConditionalJump_ManageResetIfJump(TeamClass* pTeam, int enable);

	static void Mission_Attack_List(TeamClass* pTeam, bool repeatAction, int calcThreatMode, int attackAITargetType);
	static void Mission_Attack_List1Random(TeamClass* pTeam, bool repeatAction, int calcThreatMode, int attackAITargetType);
	static void Mission_Move_List(TeamClass* pTeam, int calcThreatMode, bool pickAllies, int attackAITargetType);
	static void Mission_Move_List1Random(TeamClass* pTeam, int calcThreatMode, bool pickAllies, int attackAITargetType, int idxAITargetTypeItem);
	static void SetCloseEnoughDistance(TeamClass* pTeam, double distance);
	static void SetMoveMissionEndMode(TeamClass* pTeam, int mode);
	static void SkipNextAction(TeamClass* pTeam, int successPercentage);
	static void Set_ForceJump_Countdown(TeamClass* pTeam, bool repeatLine, int count);
	static void Stop_ForceJump_Countdown(TeamClass* pTeam);

	static void ResetAngerAgainstHouses(TeamClass* pTeam);
	static void SetHouseAngerModifier(TeamClass* pTeam, int modifier);
	static void ModifyHateHouses_List(TeamClass* pTeam, int idxHousesList);
	static void ModifyHateHouses_List1Random(TeamClass* pTeam, int idxHousesList);
	static void ModifyHateHouse_Index(TeamClass* pTeam, int idxHouse);
	static void SetTheMostHatedHouse(TeamClass* pTeam, int mask, int mode, bool random);
	static void OverrideOnlyTargetHouseEnemy(TeamClass* pTeam, int mode);
	static void AggroHouse(TeamClass* pTeam, int index);
	static void DebugAngerNodesData();
	static void ManageTriggersFromList(TeamClass* pTeam, int idxAITriggerType, bool isEnabled);
	static void ManageAllTriggersFromHouse(TeamClass* pTeam, HouseClass* pHouse, int sideIdx, int houseIdx, bool isEnabled);
	static void ForceGlobalOnlyTargetHouseEnemy(TeamClass* pTeam, int mode);

	static bool IsExtVariableAction(int action);
	static void VariablesHandler(TeamClass* pTeam, PhobosScripts eAction, int nArg);
	template<bool IsGlobal, class _Pr>
	static void VariableOperationHandler(TeamClass* pTeam, int nVariable, int Number);
	template<bool IsSrcGlobal, bool IsGlobal, class _Pr>
	static void VariableBinaryOperationHandler(TeamClass* pTeam, int nVariable, int nVarToOperate);
	static FootClass* FindTheTeamLeader(TeamClass* pTeam);
	static void SetSideIdxForManagingTriggers(TeamClass* pTeam, int sideIdx);
	static void SetHouseIdxForManagingTriggers(TeamClass* pTeam, int houseIdx);
	static void ManageAITriggers(TeamClass* pTeam, int enabled);
	static void ManageTriggersWithObjects(TeamClass* pTeam, int idxAITargetType, bool isEnabled);
	static void JumpBackToPreviousScript(TeamClass* pTeam);
	static void UnloadFromTransports(TeamClass* pTeam);

	static ExtContainer ExtMap;

private:
	static void ModifyCurrentTriggerWeight(TeamClass* pTeam, bool forceJumpLine, double modifier);
	static bool MoveMissionEndStatus(TeamClass* pTeam, TechnoClass* pFocus, FootClass* pLeader, int mode);
	static bool ConditionalJump_MakeEvaluation(int comparatorMode, int studiedValue, int comparatorValue);
	static HouseClass* GetTheMostHatedHouse(TeamClass* pTeam, int mask, int mode);
	static void UpdateEnemyHouseIndex(HouseClass* pHouse);
};
