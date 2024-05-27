#include "Body.h"

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

DEFINE_HOOK(0x6EA6BE, TeamClass_CanAddMember_Types, 0x6)
{
	enum { SkipGameCode = 0x6EA6F2 };

	GET(TeamClass*, pTeam, EBP);
	GET(FootClass*, pFoot, ESI);
	GET(int*, index, EBX);

	const auto pTaskForce = pTeam->Type->TaskForce;
	const auto pFootType = pFoot->GetTechnoType();
	const auto pFootTypeExt = TechnoTypeExt::ExtMap.Find(pFootType);

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
