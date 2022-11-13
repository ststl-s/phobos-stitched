#pragma once

#include <AbstractClass.h>

class TechnoClass;

class PlanningBranchClass;
// static_assert(sizeof(PlanningBranchClass) == 0x78);

class PlanningMemberClass
{
public:
	TechnoClass* Owner;
	DWORD Packet;
	int field_8;
	char field_C;
};
static_assert(sizeof(PlanningMemberClass) == 0x10);

class PlanningNodeClass
{
public:
	static constexpr constant_ptr<DynamicVectorClass<PlanningNodeClass*>, 0xAC4B30u> const Unknown1 {};
	static constexpr constant_ptr<DynamicVectorClass<PlanningNodeClass*>, 0xAC4C18u> const Unknown2 {};
	static constexpr constant_ptr<DynamicVectorClass<PlanningNodeClass*>, 0xAC4C98u> const Unknown3 {};

	static constexpr reference<bool, 0xAC4CF4u> const PlanningModeActive {};

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	DynamicVectorClass<PlanningMemberClass*> PlanningMembers;
	int field_18;
	bool field_1C;
	DynamicVectorClass<PlanningBranchClass*> PlanningBranches;
	//...
};

class PlanningTokenClass
{
public:
	static constexpr constant_ptr<DynamicVectorClass<PlanningTokenClass*>, 0xAC4C78u> const Array {};

	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:
	TechnoClass* OwnerUnit;
	DynamicVectorClass<PlanningNodeClass*> PlanningNodes;
	bool field_1C;
	bool field_1D;

	DECLARE_PROPERTY_ARRAY(DWORD, unknown_20_88, 0x1B);

	int field_8C;
	int ClosedLoopNodeCount;
	int StepsToClosedLoop;
	bool field_98;
	bool field_99;
};
static_assert(sizeof(PlanningTokenClass) == 0x9C);
