#include <InfantryClass.h>

#include <Helpers/Macro.h>

#include <Ext/Techno/Body.h>

DEFINE_HOOK(0x51E7A6, InfantryClass_GetCursorOverObject_PilotCanEnter, 0x6)
{
	GET(InfantryClass*, pSelected, EDI);
	GET(ObjectClass*, pTarget, ESI);

	enum { Capture = 0x51E84B, DontMindMe = 0x51E7AC };

	if (const auto pTechno = abstract_cast<TechnoClass*>(pTarget))
	{
		if (!pTechno->GetTechnoType()->IsTrain)
		{
			TechnoExt::ExtData* pExt = TechnoExt::ExtMap.Find(pSelected);
			auto pTypeExt = pExt->TypeExtData;
			if (pTypeExt->Pilot && (pTypeExt->Pilot_IgnoreTrainable ? true : pTechno->GetTechnoType()->Trainable))
			{
				if (TechnoExt::GetActionPilot(pSelected, pTechno) != PhobosAction::None)
					return Capture;
			}
		}
	}
	R->ECX(pSelected->Owner);

	return DontMindMe;
}

DEFINE_HOOK(0x519675, InfantryClass_UpdatePosition_BeforeInfantrySpecific, 0xB)
{
	enum { Destroy = 0x51A010, Handle = 0x51967F };

	GET(InfantryClass*, pThis, ESI);

	if (pThis->GetCurrentMission() == Mission::Capture)
	{
		if (const auto pDest = abstract_cast<TechnoClass*>(pThis->Destination))
		{
			if (pThis->DistanceFrom(pDest) <= 1.5 * Unsorted::LeptonsPerCell)
			{
				bool finalize = TechnoExt::PerformActionPilot(pThis, pDest);

				if (finalize)
					return Destroy;
			}
		}
	}

	R->EAX(pThis->GetCurrentMission());

	return Handle;
}
