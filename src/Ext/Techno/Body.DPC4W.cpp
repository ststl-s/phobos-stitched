#include "Body.h"

#include <Ext/WarheadType/Body.h>

#include <Utilities/GeneralUtils.h>
#include <Utilities/Helpers.Alex.h>

void TechnoExt::ExtData::AircraftClass_SetTargetFix()
{
	const auto pThis = this->OwnerObject();
	const auto pType = pThis->GetTechnoType();
	const auto pTypeExt = this->TypeExtData;

	if (!TechnoExt::IsActive(pThis))
		return;

	if (pType->WhatAmI() != AbstractType::AircraftType)
		return;

	if (!pTypeExt->Attack_OnUnit.Get())
		return;

	if (!pThis->Target || pThis->Target->IsInAir())
		return;

	if (abstract_cast<InfantryClass*>(pThis->Target) ||
		abstract_cast<UnitClass*>(pThis->Target))
		return;

	const auto coords = pThis->Target->GetCoords();
	const auto pCell = MapClass::Instance->GetCellAt(CellClass::Coord2Cell(coords));

	if (const auto pUnit = pCell->GetUnit(false))
	{
		pThis->SetTarget(pUnit);
	}
	else if (const auto pInf = pCell->GetInfantry(false))
	{
		pThis->SetTarget(pInf);
	}
}

//这部分是战机巡航逻辑
void TechnoExt::ExtData::Aircraft_AreaGuard()
{
	const auto pThis = this->OwnerObject();
	const auto pType = pThis->GetTechnoType();
	const auto pTypeExt = this->TypeExtData;

	if (!pThis->Owner->IsControlledByHuman())
		return;

	if (!TechnoExt::IsActive(pThis))
		return;

	if (pType->WhatAmI() != AbstractType::AircraftType)
		return;

	if (!pTypeExt->Fighter_AreaGuard.Get())
		return;

	if (pThis->CurrentMission == Mission::Move)
	{
		this->isAreaProtecting = false;
		this->isAreaGuardReloading = false;
		return;
	}

	if (const auto pFoot = abstract_cast<FootClass*>(pThis))
	{
		if (this->areaGuardCoords.empty())
		{
			const auto radius = pTypeExt->Fighter_GuardRadius.Get() * 256;

			this->areaGuardCoords.push_back({ 0,radius,0 });
			this->areaGuardCoords.push_back({ (int)(0.85 * radius), (int)(0.85 * radius), 0 });
			this->areaGuardCoords.push_back({ radius, 0, 0 });
			this->areaGuardCoords.push_back({ (int)(0.85 * radius), (int)(-0.85 * radius), 0 });
			this->areaGuardCoords.push_back({ 0, -radius, 0 });
			this->areaGuardCoords.push_back({ (int)(-0.85 * radius), (int)(-0.85 * radius), 0 });
			this->areaGuardCoords.push_back({ -radius, 0, 0 });
			this->areaGuardCoords.push_back({ (int)(-0.85 * radius), (int)(0.85 * radius), 0 });
		}

		if (!this->isAreaProtecting)
		{
			if (pThis->CurrentMission == Mission::Area_Guard)
			{
				this->isAreaProtecting = true;
				this->areaProtectTo = pFoot->Destination->GetCoords();
			}
		}

		if (isAreaProtecting)
		{
			//没弹药的情况下返回机场
			if (pThis->Ammo == 0 && !this->isAreaGuardReloading)
			{
				pThis->SetTarget(nullptr);
				pThis->SetDestination(nullptr, false);
				pThis->ForceMission(Mission::Stop);
				this->isAreaGuardReloading = true;
				return;
			}

			//填弹完毕后继续巡航
			if (this->isAreaGuardReloading)
			{
				if (pThis->Ammo >= pTypeExt->Fighter_Ammo.Get())
				{
					this->isAreaGuardReloading = false;
					pThis->ForceMission(Mission::Area_Guard);
				}
				else
				{
					if (pThis->CurrentMission != Mission::Sleep && pThis->CurrentMission != Mission::Enter)
					{
						if (pThis->CurrentMission == Mission::Guard)
						{
							pThis->ForceMission(Mission::Sleep);
						}
						else
						{
							pThis->ForceMission(Mission::Enter);
						}
						return;
					}
				}
			}

			if (pThis->CurrentMission == Mission::Move)
			{
				this->isAreaProtecting = false;
				return;
			}
			else if (pThis->CurrentMission == Mission::Attack)
			{
				bool skip = true;
				if (this->isAreaProtecting && pTypeExt->Fighter_ChaseRange.Get() != -1 &&
					this->areaProtectTo.X >= 0 &&
					this->areaProtectTo.Y >= 0 &&
					this->areaProtectTo.Z >= 0)
				{
					const auto sourceDest = pTypeExt->Fighter_FindRangeAroundSelf.Get() ? pThis->GetCoords() : this->areaProtectTo;
					if (pThis->Target)
					{
						//超出追击距离停止追击
						const auto distance = sourceDest.DistanceFrom(pThis->Target->GetCoords());
						int targetdistance = pTypeExt->Fighter_ChaseRange.Get() * 256;

						if (distance > targetdistance)
						{
							pThis->SetTarget(nullptr);
							pThis->ForceMission(Mission::Stop);
						}
					}
				}

				if (skip)
				{
					return;
				}
			}
			else if (pThis->CurrentMission == Mission::Enter)
			{
				if (this->isAreaGuardReloading)
				{
					return;
				}
				else
				{
					pThis->ForceMission(Mission::Stop);
				}
			}
			else if (pThis->CurrentMission == Mission::Sleep)
			{
				if (this->isAreaGuardReloading)
				{
					return;
				}
			}

			if (this->areaProtectTo.X >= 0 &&
				this->areaProtectTo.Y >= 0 &&
				this->areaProtectTo.Z >= 0)
			{
				auto dest = this->areaProtectTo;

				if (pTypeExt->Fighter_AutoFire.Get())
				{
					const auto targetDest = pTypeExt->Fighter_FindRangeAroundSelf.Get() ? pThis->GetCoords() : dest;

					if (this->areaGuardTargetCheckRof-- <= 0)
					{
						this->areaGuardTargetCheckRof = 30;

						const auto TargetList = Helpers::Alex::getCellSpreadItems(targetDest,
							(double)pTypeExt->Fighter_GuardRange.Get(), pTypeExt->Fighter_CanAirToAir.Get());

						TechnoClass* pTarget = nullptr;
						for (const auto pTechno : TargetList)
						{
							if (pTechno->CurrentMission == Mission::Harmless)
								continue;

							if (pTechno->InLimbo || pTechno->GetTechnoType()->WhatAmI() == AbstractType::BuildingType)
								continue;

							if (pTechno->IsCloakable())
								continue;

							if (pTechno->IsIronCurtained())
								continue;

							if (pTechno->Owner == pThis->Owner || pTechno->Owner->Allies.Contains(pThis->Owner))
								continue;

							int idx = pThis->SelectWeapon(pTechno);
							const auto pWeapon = pThis->GetWeapon(idx)->WeaponType;
							if (!pWeapon || !pWeapon->Projectile || !pWeapon->Warhead)
								continue;

							if (!pWeapon->Projectile->AA && pTechno->IsInAir())
								continue;

							if (pWeapon->Warhead->MindControl && pTechno->IsMindControlled())
								continue;

							if (pWeapon->Warhead->IvanBomb && pTechno->AttachedBomb)
								continue;

							if (pWeapon->Warhead->BombDisarm && !pTechno->AttachedBomb)
								continue;

							if (GeneralUtils::GetWarheadVersusArmor(pWeapon->Warhead, pTechno->GetTechnoType()->Armor) == 0.0)
								continue;

							pTarget = pTechno;
							break;
						}

						if (TechnoExt::IsReallyAlive(pTarget))
						{
							pThis->SetTarget(pTarget);
							pThis->ForceMission(Mission::Stop);
							pThis->ForceMission(Mission::Attack);
							return;
						}
					}
				}

				int count = this->areaGuardCoords.size() - 1;
				if (this->currentAreaProtectedIndex > count)
				{
					this->currentAreaProtectedIndex = 0;
				}
				const CoordStruct& destplus = this->areaGuardCoords[this->currentAreaProtectedIndex];
				dest.X += destplus.X;
				dest.Y += destplus.Y;

				if (FighterIsCloseEngouth(dest))
				{
					this->currentAreaProtectedIndex++;
				}
				//}

				if (this->AreaROF <= 0)
				{
					pFoot->Locomotor.get()->Move_To(dest);
					const auto nCell = CellClass::Coord2Cell(dest);
					if (const auto pCell = MapClass::Instance->TryGetCellAt(nCell))
					{
						pThis->SetDestination(pCell, false);
					}

					this->AreaROF = 30;
				}
				else
				{
					this->AreaROF--;
				}
			}
		}
	}
}

bool TechnoExt::ExtData::FighterIsCloseEngouth(CoordStruct coord)
{
	const auto pThis = this->OwnerObject();

	const auto ownerLocation = pThis->GetCoords();
	CoordStruct sameHeightCoord
	{
		coord.X,
		coord.Y,
		ownerLocation.Z
	};

	const auto  disctance = sameHeightCoord.DistanceFrom(pThis->GetCoords());
	return disctance < 2000;
}
