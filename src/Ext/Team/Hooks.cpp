#include "Body.h"
#include <UnitClass.h>

#include <Ext/TechnoType/Body.h>

#include <Helpers/Macro.h>

// Bugfix: TAction 7,80,107.
DEFINE_HOOK(0x65DF81, TeamTypeClass_CreateMembers_WhereTheHellIsIFV, 0x7)
{
	GET(FootClass* const, pPayload, EAX);
	GET(FootClass* const, pTransport, ESI);
	GET(TeamClass* const, pTeam, EBP);

	const bool isTransportOpenTopped = pTransport->GetTechnoType()->OpenTopped;

	for (auto pNext = pPayload; pNext; pNext = abstract_cast<FootClass*>(pNext->NextObject))
	{
		if (pNext && pNext != pTransport && pNext->Team == pTeam)
		{
			pNext->Transporter = pTransport;
			if (isTransportOpenTopped)
				pTransport->EnteredOpenTopped(pNext);
		}
	}

	pPayload->SetLocation(pTransport->Location);
	pTransport->AddPassenger(pPayload); // ReceiveGunner is done inside FootClass::AddPassenger
	// Ares' CreateInitialPayload doesn't work here
	return 0x65DF8D;
}

DEFINE_HOOK(0x4DE652, FootClass_AddPassenger_NumPassengerGeq0, 0x7)
{
	enum { GunnerReception = 0x4DE65B, EndFuntion = 0x4DE666 };
	GET(FootClass* const, pThis, ESI);
	// Replace NumPassengers==1 check to allow multipassenger IFV using the fix above
	return pThis->Passengers.NumPassengers > 0 ? GunnerReception : EndFuntion;
}

DEFINE_HOOK(0x6EA6BE, TeamClass_CanAddMember_Consideration, 0x6)
{
	enum { SkipGameCode = 0x6EA6F2 };

	GET(TeamClass*, pTeam, EBP);
	GET(FootClass*, pFoot, ESI);
	GET(int*, index, EBX);

	const auto pFootType = pFoot->GetTechnoType();
	const auto pFootTypeExt = TechnoTypeExt::ExtMap.Find(pFootType);
	const auto pTaskForce = pTeam->Type->TaskForce;

	do
	{
		const auto pType = pTaskForce->Entries[*index].Type;

		if (pType == pFootType || pFootTypeExt->TeamMember_ConsideredAs.Contains(pType))
			break;

		*index = *index + 1;
	}
	while (pTaskForce->CountEntries > *index);

	return SkipGameCode;
}

DEFINE_HOOK(0x6EA8E7, TeamClass_LiberateMember_Consideration, 0x5)
{
	enum { SkipGameCode = 0x6EA91B };

	GET(TeamClass*, pTeam, EDI);
	GET(FootClass*, pMember, EBP);
	int idx = 0;

	const auto pMemberType = pMember->GetTechnoType();
	const auto pMemberTypeExt = TechnoTypeExt::ExtMap.Find(pMemberType);
	const auto pTaskForce = pTeam->Type->TaskForce;

	do
	{
		const auto pSearchType = pTaskForce->Entries[idx].Type;

		if (pSearchType == pMemberType || pMemberTypeExt->TeamMember_ConsideredAs.Contains(pSearchType))
			break;

		idx++;
	}
	while (pTaskForce->CountEntries > idx);

	R->Stack(STACK_OFFSET(0x14, 0x8), idx);
	return SkipGameCode;
}

DEFINE_HOOK(0x6EAD73, TeamClass_Sub_6EAA90_Consideration, 0x7)
{
	enum { ContinueCheck = 0x6EAD8F, SkipThisMember = 0x6EADB3 };

	GET(TeamClass*, pTeam, ECX);
	GET(UnitClass*, pMember, ESI);
	GET_STACK(int, idx, STACK_OFFSET(0x3C, 0x4));

	const auto pMemberType = pMember->Type;
	const auto pTaskForce = pTeam->Type->TaskForce;
	const auto pSearchType = pTaskForce->Entries[idx].Type;

	if (pSearchType == pMemberType)
		return ContinueCheck;

	const auto pMemberTypeExt = TechnoTypeExt::ExtMap.Find(pMemberType);

	return pMemberTypeExt->TeamMember_ConsideredAs.Contains(pSearchType) ? ContinueCheck : SkipThisMember;
}

DEFINE_HOOK(0x6EF57F, TeamClass_GetTaskForceMissingMemberTypes_Consideration, 0x5)
{
	enum { ContinueIn = 0x6EF584, SkipThisMember = 0x6EF5A5 };

	GET(int, idx, EAX);

	if (idx != -1)
		return ContinueIn;

	GET(DynamicVectorClass<TechnoTypeClass*>*, vector, ESI);
	GET(FootClass*, pMember, EDI);
	const auto pMemberTypeExt = TechnoTypeExt::ExtMap.Find(pMember->GetTechnoType());

	for (const auto pConsideType : pMemberTypeExt->TeamMember_ConsideredAs)
	{
		idx = vector->FindItemIndex(pConsideType);

		if (idx != -1)
		{
			R->EAX(idx);
			return ContinueIn;
		}
	}

	return SkipThisMember;
}
