#include "FootExt.h"

#include <InfantryClass.h>

bool __fastcall FootExt::EnterInfantry(FootClass* pThis, InfantryClass* pInf)
{
	if (pThis->DistanceFrom(pInf) <= Unsorted::LeptonsPerCell)
	{
		if (pInf->SendCommand(RadioCommand::QueryCanEnter, pThis) == RadioCommand::AnswerPositive)
		{
			pInf->SetFocus(nullptr);
			pInf->OnBridge = false;
			pInf->unknown_C4 = 0;
			pInf->GattlingValue = 0;
			pInf->CurrentGattlingStage = 0;

			if (pInf->MindControlledBy != nullptr)
				pInf->MindControlledBy->CaptureManager->Free(pInf);

			pInf->Transporter = pThis;
			pThis->AddPassenger(pInf);

			if (pThis->GetTechnoType()->OpenTopped)
				pThis->EnteredOpenTopped(pInf);

			pInf->Undiscover();
		}
		else
		{
			pInf->SetDestination(nullptr, true);
			pInf->QueueMission(Mission::Guard, false);
			pInf->Scatter(CoordStruct::Empty, true, true);
		}

		return true;
	}

	return false;
}
