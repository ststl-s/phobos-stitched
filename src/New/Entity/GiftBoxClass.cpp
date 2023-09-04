#include "GiftBoxClass.h"

#include <InfantryClass.h>
#include <ScenarioClass.h>
#include <SlaveManagerClass.h>
#include <SpawnManagerClass.h>

#include <Helpers/Macro.h>

#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>

#include <Utilities/GeneralUtils.h>
#include <Utilities/TemplateDef.h>
#include <Utilities/SavegameDef.h>

const bool GiftBoxClass::OpenDisallowed()
{
	if (auto pTechno = this->Techno)
	{
		bool bIsOnWarfactory = false;
		if (pTechno->WhatAmI() == AbstractType::Unit)
		{
			if (auto pCell = pTechno->GetCell())
			{
				if (auto pBuildingBelow = pCell->GetBuilding())
				{
					if (auto pLinkedBuilding = specific_cast<BuildingClass*>(*pTechno->RadioLinks.Items))
					{
						bIsOnWarfactory = pLinkedBuilding->Type->WeaponsFactory && !pLinkedBuilding->Type->Naval && pBuildingBelow == pLinkedBuilding;
					}
				}
			}
		}

		bool bIsGarrisoned = false;
		if (auto pInfantry = specific_cast<InfantryClass*>(pTechno))
		{
			for (auto const& pBuildingGlobal : *BuildingClass::Array)
			{
				bIsGarrisoned = pBuildingGlobal->Occupants.Count > 0 && pBuildingGlobal->Occupants.FindItemIndex(pInfantry) != -1;
			}
		}
		return pTechno->Absorbed ||
			pTechno->InOpenToppedTransport ||
			pTechno->InLimbo ||
			bIsGarrisoned ||
			bIsOnWarfactory ||
			pTechno->TemporalTargetingMe;
	}
	return false;
}

const bool GiftBoxClass::CreateType(int nIndex, TechnoTypeExt::ExtData::GiftBoxDataEntry& nGboxData, CoordStruct nCoord, CoordStruct nDestCoord)
{
	TechnoTypeClass* pItem = nGboxData.Types[nIndex];

	if (pItem == nullptr || nIndex < 0 || nIndex >= static_cast<int>(nGboxData.Nums.size()))
		return false;

	bool bSuccess = false;
	int iNum = nGboxData.Nums[nIndex];
	HouseClass* pOwner = this->Techno->GetOwningHouse();

	for (int i = 0; i < iNum; ++i)
	{
		TechnoClass* pTechno = abstract_cast<TechnoClass*>(pItem->CreateObject(pOwner));
		if (pTechno != nullptr)
		{
			CellClass* pCell = MapClass::Instance->TryGetCellAt(nCoord);

			if (!pCell)
			{
				TechnoExt::KillSelf(pTechno, AutoDeathBehavior::Vanish);
				continue;
			}

			pTechno->OnBridge = pCell->ContainsBridge();

			if (pTechno->WhatAmI() == AbstractType::Building)
			{
				++Unsorted::IKnowWhatImDoing();
				bSuccess = pTechno->Unlimbo(nCoord, DirType::East);
				--Unsorted::IKnowWhatImDoing();
				pTechno->Location = nCoord;
			}
			else
			{
				FootClass* pFoot = abstract_cast<FootClass*>(pTechno);
				DirType nRandFacing = static_cast<DirType>(ScenarioClass::Instance->Random.RandomRanged(0, 255));
				bSuccess = pFoot->Unlimbo(CoordStruct { 0,0,100000 }, nRandFacing);
				pFoot->SetLocation(nCoord);

				CellClass* pCurrentCell = MapClass::Instance->TryGetCellAt(nCoord);
				CellClass* pCellDest = MapClass::Instance->TryGetCellAt(nDestCoord);

				if (pCellDest != nullptr)
					pCellDest->ScatterContent(CoordStruct::Empty, true, true, pFoot->OnBridge);
				else if (pCurrentCell != nullptr)
					pCurrentCell->ScatterContent(CoordStruct::Empty, true, true, pFoot->OnBridge);

				pFoot->SlaveOwner = nullptr;
				pFoot->Transporter = nullptr;
				pFoot->Absorbed = false;
				pFoot->LastMapCoords = pCell->MapCoords;
				pFoot->SetDestination(pCellDest, true);
				pFoot->QueueMission(Mission::Move, false);
				pFoot->ShouldEnterOccupiable = false;
				pFoot->ShouldGarrisonStructure = false;
			}

			if (bSuccess)
			{
				if (this->Techno->IsSelected)
					pTechno->Select();

				pTechno->DiscoveredBy(pOwner);
			}
			else
			{
				if (pTechno)
				{
					TechnoExt::KillSelf(pTechno, AutoDeathBehavior::Vanish);
				}
			}

		}
	}

	return bSuccess;
}

CoordStruct GiftBoxClass::GetRandomCoordsNear(TechnoTypeExt::ExtData::GiftBoxDataEntry& nGiftBox, CoordStruct nCoord)
{
	if (nGiftBox.CellRandomRange.Get() > 0)
	{
		int iRange = nGiftBox.CellRandomRange.Get();
		CellClass* nCellLoc = MapClass::Instance->TryGetCellAt(nCoord);
		if (nCellLoc != nullptr)
		{
			for (CellSpreadEnumerator it((size_t)abs(iRange)); it; ++it)
			{
				CellStruct const& offset = *it;

				if (offset == CellStruct::Empty)
					continue;

				if (auto pCell = MapClass::Instance->TryGetCellAt(CellClass::Cell2Coord(nCellLoc->MapCoords + offset)))
				{
					if (nGiftBox.EmptyCell &&
							(
								pCell->GetBuilding() != nullptr
								|| pCell->GetUnit(false) != nullptr
								|| pCell->GetInfantry(false) != nullptr
								)
						)
					{
						continue;
					}

					nCoord = CellClass::Cell2Coord(pCell->MapCoords);
					break;
				}
			}
		}
	}
	return nCoord;
}

void GiftBoxClass::SyncToAnotherTechno(TechnoClass* pFrom, TechnoClass* pTo)
{
	const auto pFromExt = TechnoExt::ExtMap.Find(pFrom);
	const auto pToExt = TechnoExt::ExtMap.Find(pTo);

	if (pFromExt->AttachedGiftBox)
	{
		pToExt->AttachedGiftBox = std::make_unique<GiftBoxClass>(pTo);
		strcpy_s(pToExt->AttachedGiftBox->TechnoID.data(), pFromExt->AttachedGiftBox->TechnoID);
		pToExt->AttachedGiftBox->Delay = pFromExt->AttachedGiftBox->Delay;
		pToExt->AttachedGiftBox->IsOpen = pFromExt->AttachedGiftBox->IsOpen;
		pToExt->AttachedGiftBox->IsEnabled = pFromExt->AttachedGiftBox->IsEnabled;
		pToExt->AttachedGiftBox->IsTechnoChange = true;
		pFromExt->AttachedGiftBox = nullptr;
	}
}

const void GiftBoxClass::AI()
{
	TechnoClass* pTechno = this->Techno;

	if (pTechno == nullptr)
		return;

	TechnoExt::ExtData* pTechnoExt = TechnoExt::ExtMap.Find(pTechno);
	GiftBoxClass* pGiftBox = pTechnoExt->AttachedGiftBox.get();

	if (pGiftBox != nullptr)
	{
		TechnoTypeClass* pType = pTechno->GetTechnoType();
		TechnoTypeExt::ExtData* pTypeExt = TechnoTypeExt::ExtMap.Find(pType);
		TechnoTypeExt::ExtData::GiftBoxDataEntry& nGiftBoxData = pTypeExt->GiftBoxData;
		const char* newID = pTechno->get_ID();
		int iDelay = 0;

		if (!IsTechnoChange && (strcmp(this->TechnoID, newID) != 0))
			strcpy_s(this->TechnoID.data(), newID);

		if (!pTypeExt->GiftBoxData)
			return;

		if (pGiftBox->Delay == 0)
		{
			iDelay = nGiftBoxData.Delay;

			int iDelayMin = nGiftBoxData.DelayMinMax.Get().X;
			int iDelayMax = nGiftBoxData.DelayMinMax.Get().Y;

			// Use RandomDelay Instead
			if (iDelayMax > 0)
				iDelay = ScenarioClass::Instance->Random.RandomRanged(iDelayMin, iDelayMax);

			pGiftBox->Delay = abs(iDelay);
		}

		pGiftBox->IsEnabled = !pGiftBox->OpenDisallowed();

		if (pGiftBox->Open())
		{
			CoordStruct nCoord = GiftBoxClass::GetRandomCoordsNear(nGiftBoxData, pTechno->GetCoords());
			CoordStruct nDestination = nCoord;

			if (pTechno->What_Am_I() != AbstractType::Building)
			{
				if (pTechno->Focus != nullptr)
				{
					nDestination = pTechno->Focus->GetCoords();
				}
				else
				{
					AbstractClass* pDest = abstract_cast<FootClass*>(pTechno)->Destination;
					if (pDest != nullptr)
						nDestination = pDest->GetCoords();

				}
			}

			if (!pTypeExt->GiftBoxData.RandomType)
			{
				for (size_t nIndex = 0; nIndex < nGiftBoxData.Types.size(); ++nIndex)
				{
					if (!pGiftBox->CreateType(nIndex, pTypeExt->GiftBoxData, nCoord, nDestination))
						continue;
				}
			}
			else
			{
				auto nRandIdx = ScenarioClass::Instance->Random.RandomRanged(0, static_cast<int>(nGiftBoxData.Types.size()) - 1);
				pGiftBox->CreateType(nRandIdx, pTypeExt->GiftBoxData, nCoord, nDestination);
			}

			if (nGiftBoxData.Remove)
			{
				// Limboing stuffs is not safe method depend on case
				// maybe need to check if anything else need to be handle
				pTechno->Undiscover();
				TechnoExt::KillSelf(pTechno, AutoDeathBehavior::Vanish);
			}
			else if (nGiftBoxData.Destroy)
			{
				pTechno->TakeDamage(pTechno->Health);
			}
			else if (nGiftBoxData.ApplyOnce)
			{
				pTechnoExt->AttachedGiftBox = nullptr;
			}
			else
			{
				iDelay = nGiftBoxData.Delay.Get();
				int iDelayMin = nGiftBoxData.DelayMinMax.Get().X;
				int iDelayMax = nGiftBoxData.DelayMinMax.Get().Y;

				// Use RandomDelay Instead
				if (iDelayMax)
					iDelay = ScenarioClass::Instance->Random.RandomRanged(iDelayMin, iDelayMax);

				pGiftBox->Reset(abs(iDelay));
			}
		}
	}
}

DEFINE_HOOK(0x6F6CA0, TechnoClass_Put_GiftBox, 0x7)
{
	GET(TechnoClass* const, pThis, ECX);

	auto pTechnoExt = TechnoExt::ExtMap.Find(pThis);
	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	if (!pTechnoExt->AttachedGiftBox.get() && pTypeExt->GiftBoxData)
		pTechnoExt->AttachedGiftBox = std::make_unique<GiftBoxClass>(pThis);

	return 0;
}
