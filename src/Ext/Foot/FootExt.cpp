#include "FootExt.h"

#include <InfantryClass.h>
#include <LocomotionClass.h>
#include <MapClass.h>

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

void FootExt::PutFootsTo(const std::vector<FootClass*> foots, const CellStruct& cell, bool overlap)
{
	PutFootsTo(foots, CellClass::Cell2Coord(cell), overlap);
}

void FootExt::PutFootsTo(const std::vector<FootClass*> foots, const CoordStruct& location, bool overlap)
{
	for (FootClass* pTarget : foots)
	{

		CellClass* pCell = nullptr;
		int height = pTarget->GetHeight();
		const TechnoTypeClass* pTargetType = pTarget->GetTechnoType();

		if (overlap)
		{
			pCell = MapClass::Instance->TryGetCellAt(CellClass::Coord2Cell(location));
		}
		else
		{
			bool allowBridges = pTargetType->SpeedType != SpeedType::Float;
			CellStruct cellDest =
				MapClass::Instance->NearByLocation
				(
					CellClass::Coord2Cell(location),
					pTargetType->SpeedType,
					-1,
					pTargetType->MovementZone,
					false,
					1,
					1,
					true,
					false,
					false,
					allowBridges,
					CellStruct::Empty,
					false,
					false
				);
			pCell = MapClass::Instance->TryGetCellAt(cellDest);
		}

		CoordStruct crdDest;

		if (pCell != nullptr)
		{
			crdDest = pCell->GetCoordsWithBridge();
		}
		else
		{
			crdDest = location;
			crdDest.Z = MapClass::Instance->GetCellFloorHeight(crdDest);
		}

		crdDest.Z += height;
		CellStruct cellDest = CellClass::Coord2Cell(crdDest);
		pTarget->Limbo();
		ILocomotion* pLoco = pTarget->Locomotor.release();
		pTarget->Locomotor.reset(LocomotionClass::CreateInstance(pTargetType->Locomotor).release());
		pTarget->Locomotor->Link_To_Object(pTarget);
		pLoco->Release();
		++Unsorted::IKnowWhatImDoing;
		pTarget->Unlimbo(crdDest, pTarget->PrimaryFacing.Current().GetDir());
		--Unsorted::IKnowWhatImDoing;
		pTarget->MarkAllOccupationBits(crdDest);
		pTarget->Guard();
	}
}
