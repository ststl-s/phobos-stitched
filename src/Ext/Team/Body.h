#pragma once

#include <TeamClass.h>

#include <Utilities/Container.h>
#include <Utilities/Template.h>

class TeamExt
{
public:
	using base_type = TeamClass;

	static constexpr DWORD Canary = 0x414B4B41;
	static constexpr size_t ExtPointerOffset = 0x18;

	class ExtData final : public Extension<TeamClass>
	{
	public:
		int WaitNoTargetAttempts;
		double NextSuccessWeightAward;
		int IdxSelectedObjectFromAIList;
		double CloseEnough;
		int Countdown_RegroupAtLeader;
		int MoveMissionEndMode;
		int WaitNoTargetCounter;
		CDTimerClass WaitNoTargetTimer;
		FootClass* TeamLeader;
		int AngerNodeModifier;
		bool OnlyTargetHouseEnemy;
		int OnlyTargetHouseEnemyMode;
		CDTimerClass ForceJump_Countdown;
		int ForceJump_InitialCountdown;
		bool ForceJump_RepeatMode;
		int TriggersSideIdx;
		int TriggersHouseIdx;
		bool ConditionalJump_Evaluation;
		int ConditionalJump_ComparatorMode;
		int ConditionalJump_ComparatorValue;
		int ConditionalJump_Counter;
		int ConditionalJump_Index;
		bool AbortActionAfterKilling;
		bool ConditionalJump_EnabledKillsCount;
		bool ConditionalJump_ResetVariablesIfJump;
		std::vector<ScriptClass*> PreviousScriptList;
		DynamicVectorClass<FootClass*> AllPassengers;

		ExtData(TeamClass* OwnerObject) : Extension<TeamClass>(OwnerObject)
			, WaitNoTargetAttempts { 0 }
			, NextSuccessWeightAward { 0 }
			, IdxSelectedObjectFromAIList { -1 }
			, CloseEnough { -1 }
			, Countdown_RegroupAtLeader { -1 }
			, MoveMissionEndMode { 0 }
			, WaitNoTargetCounter { 0 }
			, WaitNoTargetTimer { 0 }
			, TeamLeader { nullptr }
			, AngerNodeModifier { 5000 }
			, OnlyTargetHouseEnemy { false }
			, OnlyTargetHouseEnemyMode { -1 }
			, ForceJump_Countdown { -1 }
			, ForceJump_InitialCountdown { -1 }
			, ForceJump_RepeatMode { false }
			, TriggersSideIdx { -1 }
			, TriggersHouseIdx { -1 }
			, ConditionalJump_Evaluation { false }
			, ConditionalJump_ComparatorMode { 3 }
			, ConditionalJump_ComparatorValue { 1 }
			, ConditionalJump_Counter { 0 }
			, AbortActionAfterKilling { false }
			, ConditionalJump_Index { -1000000 }
			, ConditionalJump_EnabledKillsCount { false }
			, ConditionalJump_ResetVariablesIfJump { false }
			, PreviousScriptList { }
			, AllPassengers { }
		{ }

		virtual ~ExtData() = default;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override
		{
			AnnounceInvalidPointer(TeamLeader, ptr);
		}

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<TeamExt>
	{
	public:
		ExtContainer();
		~ExtContainer();

		virtual bool InvalidateExtDataIgnorable(void* const ptr) const override
		{
			auto const abs = static_cast<AbstractClass*>(ptr)->WhatAmI();
			switch (abs)
			{
			case AbstractType::Infantry:
			case AbstractType::Unit:
			case AbstractType::Aircraft:
				return false;
			default:
				return true;
			}
		}
	};

	static ExtContainer ExtMap;

};
