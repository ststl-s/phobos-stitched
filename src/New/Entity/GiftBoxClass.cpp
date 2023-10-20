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

bool GiftBoxClass::OpenDisallowed()
{
	TechnoClass* pTechno = this->Techno;

	if (pTechno == nullptr)
		return true;

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

	return pTechno->Absorbed
		|| pTechno->InOpenToppedTransport
		|| pTechno->InLimbo
		|| bIsGarrisoned
		|| bIsOnWarfactory
		|| pTechno->TemporalTargetingMe != nullptr;
}

bool GiftBoxClass::CreateType(int nIndex, TechnoTypeExt::ExtData::GiftBoxDataEntry& nGboxData, CoordStruct nCoord, CoordStruct nDestCoord)
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

void GiftBoxClass::InitializeGiftBox(TechnoClass* pTechno)
{
	TechnoExt::ExtData* pTechnoExt = TechnoExt::ExtMap.Find(pTechno);
	TechnoTypeExt::ExtData* pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pTechno->GetTechnoType());

	if (pTechnoExt->AttachedGiftBox == nullptr && !pTechnoTypeExt->GiftBoxData.empty())
		pTechnoExt->AttachedGiftBox = std::make_unique<GiftBoxClass>(pTechno);
}

void GiftBoxClass::SyncToAnotherTechno(TechnoClass* pFrom, TechnoClass* pTo)
{
	const auto pFromExt = TechnoExt::ExtMap.Find(pFrom);
	const auto pToExt = TechnoExt::ExtMap.Find(pTo);

	if (pFromExt->AttachedGiftBox)
	{
		pToExt->AttachedGiftBox = std::make_unique<GiftBoxClass>(pTo);
		pToExt->AttachedGiftBox->Delay = pFromExt->AttachedGiftBox->Delay;
		pToExt->AttachedGiftBox->IsOpen = pFromExt->AttachedGiftBox->IsOpen;
		pToExt->AttachedGiftBox->IsEnabled = pFromExt->AttachedGiftBox->IsEnabled;
		pToExt->AttachedGiftBox->IsTechnoChange = true;
		pFromExt->AttachedGiftBox = nullptr;
	}
}

void __fastcall GiftBoxClass::AI(TechnoTypeExt::ExtData* pTechnoTypeExt)
{
	TechnoClass* pTechno = this->Techno;

	if (pTechno == nullptr)
		return;

	TechnoExt::ExtData* pTechnoExt = TechnoExt::ExtMap.Find(pTechno);
	GiftBoxClass* pGiftBox = pTechnoExt->AttachedGiftBox.get();

	if (pGiftBox == nullptr || pGiftBox->IsDiscard || !pTechnoTypeExt->GiftBoxData.empty())
		return;

	TechnoTypeExt::ExtData::GiftBoxDataEntry& giftData = pTechnoTypeExt->GiftBoxData;
	int iDelay = 0;

	if (pGiftBox->Delay == 0)
	{
		iDelay = giftData.Delay;

		int iDelayMin = giftData.DelayMinMax.Get().X;
		int iDelayMax = giftData.DelayMinMax.Get().Y;

		// Use RandomDelay Instead
		if (iDelayMax > 0)
			iDelay = ScenarioClass::Instance->Random.RandomRanged(iDelayMin, iDelayMax);

		pGiftBox->Delay = abs(iDelay);
	}

	pGiftBox->IsEnabled = !pGiftBox->OpenDisallowed();

	if (pGiftBox->Open())
	{
		CoordStruct nCoord = GiftBoxClass::GetRandomCoordsNear(giftData, pTechno->GetCoords());
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

		if (giftData.RandomType)
		{
			for (size_t nIndex = 0; nIndex < giftData.Types.size(); ++nIndex)
			{
				if (!pGiftBox->CreateType(nIndex, giftData, nCoord, nDestination))
					continue;
			}
		}
		else
		{
			auto nRandIdx = ScenarioClass::Instance->Random.RandomRanged(0, static_cast<int>(giftData.Types.size()) - 1);
			pGiftBox->CreateType(nRandIdx, giftData, nCoord, nDestination);
		}

		if (giftData.Remove)
		{
			// Limboing stuffs is not safe method depend on case
			// maybe need to check if anything else need to be handle
			pTechno->Undiscover();
			TechnoExt::KillSelf(pTechno, AutoDeathBehavior::Vanish);
		}
		else if (giftData.Destroy)
		{
			pTechno->TakeDamage(pTechno->Health);
		}
		else if (giftData.ApplyOnce)
		{
			this->IsDiscard = true;
		}
		else
		{
			iDelay = giftData.Delay.Get();
			int iDelayMin = giftData.DelayMinMax.Get().X;
			int iDelayMax = giftData.DelayMinMax.Get().Y;

			// Use RandomDelay Instead
			if (iDelayMax)
				iDelay = ScenarioClass::Instance->Random.RandomRanged(iDelayMin, iDelayMax);

			pGiftBox->Reset(abs(iDelay));
		}
	}
}

bool GiftBoxClass::Load(PhobosStreamReader& stm, bool registerForChange)
{
	return this->Serialize(stm);
}

bool GiftBoxClass::Save(PhobosStreamWriter& stm) const
{
	return const_cast<GiftBoxClass*>(this)->Serialize(stm);
}

template <typename T>
bool GiftBoxClass::Serialize(T& stm)
{
	return stm
		.Process(this->IsTechnoChange)
		.Process(this->Techno)
		.Process(this->IsEnabled)
		.Process(this->IsOpen)
		.Process(this->IsDiscard)
		.Process(this->Delay)
		.Success();
}
