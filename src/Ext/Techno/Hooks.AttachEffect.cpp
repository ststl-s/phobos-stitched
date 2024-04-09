#include <Ext/Anim/Body.h>
#include <Ext/AnimType/Body.h>
#include <Ext/Building/BuildingExt.h>
#include <Ext/House/Body.h>
#include <Ext/HouseType/Body.h>
#include <Ext/Techno/Body.h>

#include <Utilities/EnumFunctions.h>
#include <Utilities/Macro.h>

#include <New/Type/CrateTypeClass.h>
#include <New/Entity/CrateClass.h>

#define Max(a, b) (a > b ? a : b)
#define Min(a, b) (a < b ? a : b)

// ROF
DEFINE_HOOK(0x6FD1F1, TechnoClass_GetROF, 0x5)
{
	GET(TechnoClass*, pThis, ESI);
	GET(int, rof, EBP);

	TechnoExt::ExtData* pExt = nullptr;

	if (pThis->Transporter != nullptr)
		pExt = TechnoExt::ExtMap.Find(pThis->Transporter);
	else
		pExt = TechnoExt::ExtMap.Find(pThis);

	if (pThis->WhatAmI() == AbstractType::Building)
	{
		auto pBuildingExt = BuildingExt::ExtMap.Find(static_cast<BuildingClass*>(pThis));

		if (pBuildingExt->BuildingROFFix > 0)
		{
			rof = pBuildingExt->BuildingROFFix;
			pBuildingExt->BuildingROFFix = -1;
		}
	}

	rof = Game::F2I(rof * pExt->AEBuffs.ROFMul);
	rof += pExt->AEBuffs.ROF;
	rof = Max(rof, 1);
	R->EBP(rof);

	return 0;
}

// FirePower
DEFINE_HOOK(0x46B050, BulletTypeClass_CreateBullet, 0x6)
{
	GET_STACK(TechnoClass*, pOwner, 0x4);
	REF_STACK(int, damage, 0x8);

	if (pOwner == nullptr)
		return 0;

	const TechnoExt::ExtData* pTechnoExt = nullptr;

	if (pOwner->Transporter != nullptr)
		pTechnoExt = TechnoExt::ExtMap.Find(pOwner->Transporter);
	else
		pTechnoExt = TechnoExt::ExtMap.Find(pOwner);

	if (pTechnoExt == nullptr)
		return 0;

	damage = Game::F2I(damage * pTechnoExt->AEBuffs.FirepowerMul);
	damage += pTechnoExt->AEBuffs.Firepower;

	return 0;
}

// Speed
DEFINE_HOOK(0x4DB221, FootClass_GetCurrentSpeed, 0x5)
{
	GET(FootClass*, pThis, ESI);
	GET(int, speed, EDI);

	TechnoExt::ExtData* pExt = TechnoExt::ExtMap.Find(pThis);

	speed = Game::F2I(speed * pExt->AEBuffs.SpeedMul);
	speed += pExt->AEBuffs.Speed;
	speed = Math::max(0, speed);
	speed = Math::min(256, speed);
	R->EDI(speed);

	if (pThis->WhatAmI() != AbstractType::Unit)
		return 0x4DB23E;

	return 0x4DB226;
}

// Range
DEFINE_HOOK(0x6F7248, TechnoClass_InRange, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EBX);

	int range = pWeapon->Range;
	const auto pExt = TechnoExt::ExtMap.Find(pThis);
	range = Game::F2I(range * pExt->AEBuffs.RangeMul);
	range += pExt->AEBuffs.Range;
	R->EDI(range);

	return 0x6F724E;
}

// DisableWeapon
DEFINE_HOOK(0x6FC0B0, TechnoClass_GetFireError, 0x8)
{
	GET(TechnoClass*, pThis, ECX);
	GET_STACK(int, weaponIdx, 0x8);

	if (pThis->Transporter != nullptr)
	{
		TechnoExt::ExtData* pTransporterExt = TechnoExt::ExtMap.Find(pThis->Transporter);

		if (pTransporterExt->AEBuffs.DisableWeapon & DisableWeaponCate::Passenger)
		{
			R->EAX(FireError::CANT);
			return 0x6FC0EB;
		}
	}
	else
	{
		TechnoExt::ExtData* pExt = TechnoExt::ExtMap.Find(pThis);

		if (EnumFunctions::IsWeaponDisabled(pThis, pExt->AEBuffs.DisableWeapon, weaponIdx))
		{
			R->EAX(FireError::CANT);
			return 0x6FC0EB;
		}
	}

	return 0;
}

DEFINE_HOOK(0x70D690, TechnoClass_FireDeathWeapon_Supress, 0x7)
{
	GET(TechnoClass*, pThis, ECX);

	TechnoExt::ExtData* pExt = TechnoExt::ExtMap.Find(pThis);

	if (pExt->AEBuffs.DisableWeapon & DisableWeaponCate::Death)
		return 0x70D796;

	return 0;
}

DEFINE_HOOK(0x702583, TechnoClass_ReceiveDamage_NowDead_Explode, 0x6)
{
	GET(TechnoClass*, pThis, ESI);

	auto pExt = TechnoExt::ExtMap.Find(pThis);
	bool forceExplode = false;

	if (pExt->UnitDeathAnim != nullptr && (pThis->WhatAmI() == AbstractType::Unit || pThis->WhatAmI() == AbstractType::Aircraft))
	{
		if (auto const pAnim = GameCreate<AnimClass>(pExt->UnitDeathAnim, pThis->Location))
		{
			AnimExt::SetAnimOwnerHouseKind(pAnim, pExt->UnitDeathAnimOwner, pThis->Owner, true);
			auto pAnimExt = AnimExt::ExtMap.Find(pAnim);
			pAnimExt->DeathUnitFacing = static_cast<short>(pThis->PrimaryFacing.Current().GetFacing<256>());
			pAnimExt->FromDeathUnit = true;
			if (pThis->HasTurret())
			{
				pAnimExt->DeathUnitHasTurret = true;
				pAnimExt->DeathUnitTurretFacing = pThis->SecondaryFacing.Current();
			}
		}
	}

	ValueableVector<CrateTypeClass*> AllowCrateTypes;
	const auto pCell = MapClass::Instance->TryGetCellAt(pThis->Location);
	for (const auto pAE : pExt->GetActiveAE())
	{
		forceExplode |= pAE->Type->ForceExplode;

		if (!pAE->Type->CreateCrateTypes.empty() && pCell)
		{
			for (size_t i = 0; i < pAE->Type->CreateCrateTypes.size(); i++)
			{
				auto pType = pAE->Type->CreateCrateTypes[i];
				if (CrateClass::CanSpwan(pType, pCell, true) && CrateClass::CanExist(pType))
					AllowCrateTypes.emplace_back(pType);
			}
		}
	}

	if (!AllowCrateTypes.empty())
	{
		int idx = ScenarioClass::Instance->Random.RandomRanged(0, AllowCrateTypes.size() - 1);
		CrateClass::CreateCrate(AllowCrateTypes[idx], pCell, pThis->Owner);
	}

	if (pThis->WhatAmI() != AbstractType::Infantry)
	{
		auto pHouseTypeExt = HouseTypeExt::ExtMap.Find(pThis->Owner->Type);
		auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

		InfantryTypeClass* pilot = pExt->PilotType ? pExt->PilotType : pTypeExt->Pilot_CreateType.Get(pHouseTypeExt->PilotType);
		auto pPilotTypeExt = TechnoTypeExt::ExtMap.Find(pilot);

		HouseClass* pilotowner = (pExt->PilotType && pExt->PilotOwner) ? pExt->PilotOwner : pThis->Owner;

		if (pilot && pPilotTypeExt && !pilotowner->Defeated && (pPilotTypeExt->Pilot_IgnoreTrainable ? true : pThis->Veterancy.Veterancy >= 1.0))
		{
			if (auto const pPilot = static_cast<InfantryClass*>(pilot->CreateObject(pilotowner)))
			{
				if (!pCell)
				{
					pPilot->UnInit();
				}
				else
				{
					pPilot->OnBridge = (pCell->ContainsBridge() && (pThis->Location.Z >= pCell->GetCoordsWithBridge().Z));

					DirType nRandFacing = static_cast<DirType>(ScenarioClass::Instance->Random.RandomRanged(0, 255));
					++Unsorted::IKnowWhatImDoing();
					pPilot->Unlimbo(pThis->Location, nRandFacing);
					--Unsorted::IKnowWhatImDoing();

					if (auto const pController = pThis->MindControlledBy)
					{
						++Unsorted::IKnowWhatImDoing;
						pController->CaptureManager->Free(pThis);
						pController->CaptureManager->Capture(pPilot);
						--Unsorted::IKnowWhatImDoing;
						pPilot->QueueMission(Mission::Guard, true);
					}

					VocClass::PlayAt(TechnoTypeExt::ExtMap.Find(pilot)->Pilot_LeaveSound, pThis->Location, nullptr);

					VeterancyStruct* vstruct = &pPilot->Veterancy;
					vstruct->Veterancy = pThis->Veterancy.Veterancy;

					int health = pExt->PilotHealth > 0 ? pExt->PilotHealth : pilot->Strength;
					pPilot->Health = std::max(health, 10) / 2;

					if (pPilot->IsInAir())
						TechnoExt::FallenDown(pPilot);
					else
						TechnoExt::ExtMap.Find(pPilot)->WasFallenDown = true;
				}
			}
		}
	}

	if (forceExplode)
		return 0x702603;

	R->EDX(VTABLE_GET(pThis));
	R->EBP(pThis->GetWeapon(pThis->CurrentWeaponNumber)->WeaponType);
	R->ECX(pThis);

	return 0x702589;
}

DEFINE_HOOK(0x70D724, TechnoClass_FireDeathWeapon_ReplaceDeathWeapon, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EDI);

	enum { FireDeathWeapon = 0x70D735, SkipDeathWeapon = 0x70D72A };

	if (!SessionClass::IsSingleplayer())
	{
		R->EBP(R->EAX());
		return pWeapon == nullptr ? SkipDeathWeapon : FireDeathWeapon;
	}

	auto pExt = TechnoExt::ExtMap.Find(pThis);

	for (const auto& pAE : pExt->GetActiveAE())
	{
		if (pAE->Type->ReplaceDeathWeapon.isset())
			pWeapon = pAE->Type->ReplaceDeathWeapon;
	}

	R->EDI(pWeapon);
	R->EBP(pWeapon ? Game::F2I(pWeapon->Damage * pThis->GetTechnoType()->DeathWeaponDamageModifier) : 0);

	return pWeapon == nullptr ? SkipDeathWeapon : FireDeathWeapon;
}

//immune to mindcontrol
DEFINE_HOOK(0x471C90, CaptureManagerClass_CanCapture_AttachEffect, 0x6)
{
	GET(CaptureManagerClass*, pThis, ECX);
	GET_STACK(TechnoClass*, pTarget, 0x4);

	enum { SkipGameCode = 0x471D39 };

	if (!pTarget || !pTarget->IsAlive || pTarget == pThis->Owner)
	{
		R->EAX(false);

		return SkipGameCode;
	}

	if (auto pTargetExt = TechnoExt::ExtMap.Find(pTarget))
	{
		if (pTargetExt->AEBuffs.ImmuneMindControl)
		{
			R->EAX(false);

			return SkipGameCode;
		}

		if (pTarget->Passengers.NumPassengers > 0)
		{
			for (
				FootClass* pPassenger = pTarget->Passengers.GetFirstPassenger();
				pPassenger != nullptr;
				pPassenger = abstract_cast<FootClass*>(pPassenger->NextObject)
				)
			{
				auto pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pPassenger->GetTechnoType());

				if (pTechnoTypeExt->VehicleImmuneToMindControl)
				{
					R->EAX(false);

					return SkipGameCode;
				}
			}
		}
	}

	return 0;
}

//HideImage
//DEFINE_HOOK_AGAIN(0x43D290,TechnoClass_Draw_HideImage,0x5)	//Building
DEFINE_HOOK_AGAIN(0x73CEC0,TechnoClass_Draw_HideImage,0x5)	//Unit
DEFINE_HOOK_AGAIN(0x4144B0,TechnoClass_Draw_HideImage,0x5)	//Aircraft
DEFINE_HOOK(0x518F90, TechnoClass_Draw_HideImage, 0x7)	//Infantry
{
	GET(TechnoClass*, pThis, ECX);

	auto pExt = TechnoExt::ExtMap.Find(pThis);

	if (pExt->AEBuffs.HideImage)
	{
		switch (pThis->WhatAmI())
		{
			//case AbstractType::Building:
				//static_cast<BuildingClass*>(pThis)->DestroyNthAnim(BuildingAnimSlot::All);
				//return 0x43DA73;
		case AbstractType::Unit:
			return 0x73D446;
		case AbstractType::Infantry:
			return 0x519626;
		case AbstractType::Aircraft:
			return 0x4149FE;
		default:
			break;
		}
	}

	return 0;
}

DEFINE_HOOK(0x5184FF, InfantryClass_ReceiveDamage_InfDeathAnim, 0x6)
{
	GET(InfantryClass*, pThis, ESI);
	GET(InfantryTypeClass*, pType, EAX);
	LEA_STACK(args_ReceiveDamage*, args, STACK_OFFSET(0xD0, 0x4));

	enum { NotHuman = 0x518505, AnimOverriden = 0x5185F1, AresCode = 0x5185C8 };

	if (pType->NotHuman)
		return NotHuman;

	const auto pExt = TechnoExt::ExtMap.Find(pThis);

	AnimTypeClass* pAnimType = nullptr;

	for (const auto& pAE : pExt->GetActiveAE())
	{
		if (pAE->Type->InfDeathAnim != nullptr)
			pAnimType = pAE->Type->InfDeathAnim;
	}

	if (pAnimType != nullptr)
	{
		AnimClass* pAnim = GameCreate<AnimClass>(pAnimType, pThis->Location);
		pAnim->Owner = args->SourceHouse;

		if (pAnim->Owner != nullptr)
		{
			pAnim->LightConvert = ColorScheme::Array->GetItem(pAnim->Owner->ColorSchemeIndex)->LightConvert;
			pAnim->LightConvertIndex = pAnim->Owner->ColorSchemeIndex;
		}

		return AnimOverriden;
	}

	return AresCode;
}

DEFINE_HOOK(0x482956, CellClass_CrateBeingCollected_Cloak, 0xA)
{
	GET(int, distance, EAX);

	if (distance < RulesClass::Instance->CrateRadius)
	{
		GET(TechnoClass*, pThis, ECX);

		if (const auto pExt = TechnoExt::ExtMap.Find(pThis))
			pExt->Crate_Cloakable = true;
	}

	return 0;
}

DEFINE_HOOK(0x736D68, UnitClass_CanDeployNow_DisableDeployWeapon, 0x5)
{
	GET(UnitClass*, pThis, ESI);

	TechnoExt::ExtData* pExt = TechnoExt::ExtMap.Find(pThis);

	bool disabled = (pExt->AEBuffs.DisableWeapon & DisableWeaponCate::Deploy) == DisableWeaponCate::Deploy;

	R->EAX(!disabled);

	return 0;
}

DEFINE_HOOK(0x70E120, InfantryClass_CanDeployNow_DisableDeployWeapon, 0x6)
{
	GET(InfantryClass*, pThis, ECX);

	TechnoExt::ExtData* pExt = TechnoExt::ExtMap.Find(pThis);
	static const WeaponStruct noWeapon;

	if (pExt->AEBuffs.DisableWeapon & DisableWeaponCate::Deploy)
	{
		R->EAX(&noWeapon);
		return 0x70E137;
	}

	return 0;
}

const byte NoDeployCode[] =
{
	0x5F, //pop edi
	0x5E, //pop esi
	0x5D, //pop ebp
	0xB8, 0x1E, 0x00, 0x00, 0x00, // mov eax Action::NoDeploy
	0x5B, //pop EBX
	0x83, 0xC4, 0x0C, //add esp 0xC
	0xC2, 0x08, 0x00  //retn 8
};

DEFINE_HOOK(0x7000CD, TechnoClass_MouseOverObject_Self, 0x9)
{
	GET(TechnoClass*, pThis, ESI);

	enum { SkipDeploy = 0x700191, Deploy=0x7000DC };
	DWORD NoDeploy = reinterpret_cast<DWORD>(NoDeployCode);

	const TechnoTypeClass* pType = pThis->GetTechnoType();
	TechnoExt::ExtData* pExt = TechnoExt::ExtMap.Find(pThis);
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

	if (pTypeExt->Backwarp_Deploy && !pExt->BackwarpColdDown.Completed())
		return NoDeploy;

	if (pThis->Owner->IsControlledByCurrentPlayer())
	{
		if (pType->DeployFire && (pExt->AEBuffs.DisableWeapon & DisableWeaponCate::Deploy))
			return NoDeploy;

		/*if ((pType->DeploysInto != nullptr || pTypeExt->Convert_Deploy != nullptr)
			&& pTypeExt->Deploy_Cost > 0
			&& !pThis->Owner->CanTransactMoney(pTypeExt->Deploy_Cost))
			return NoDeploy;*/

		return Deploy;
	}

	if (ObjectClass::CurrentObjects->Count == 1
		&& pThis->Owner->IsInPlayerControl
		&& SessionClass::Instance->GameMode != GameMode::Campaign)
	{
		if (pThis->WhatAmI() == AbstractType::Unit
			&& pType->Passengers > 0
			&& pThis->Passengers.NumPassengers > 0)
			return Deploy;

		if (const auto pAircraft = abstract_cast<AircraftClass*>(pThis))
		{
			if (pAircraft->HasPassengers)
				return Deploy;
		}
	}

	return SkipDeploy;
}

DEFINE_HOOK(0x51EC9F, InfantryClass_MouseOverObject_Deploy, 0x5)
{
	GET(InfantryClass*, pThis, EDI);

	if (pThis->Type->Deployer)
	{
		TechnoExt::ExtData* pExt = TechnoExt::ExtMap.Find(pThis);

		if (pExt->AEBuffs.DisableWeapon & DisableWeaponCate::Deploy)
			return 0x51ED00;
	}

	return 0;
}

DEFINE_HOOK(0x51F738, InfantryClass_Mission_Unload_Disable, 0x5)
{
	GET(InfantryClass*, pThis, ESI);

	enum { Cannot = 0x51F7EE, Continue = 0 };

	TechnoExt::ExtData* pExt = TechnoExt::ExtMap.Find(pThis);

	if (pExt->AEBuffs.DisableWeapon & DisableWeaponCate::Deploy)
		return Cannot;

	return Continue;
}

DEFINE_HOOK(0x70C5A0, TechnoClass_IsCloakable, 0x6)
{
	GET(TechnoClass*, pThis, ECX);

	enum { retn = 0x70C5AB };

	const auto pExt = TechnoExt::ExtMap.Find(pThis);

	if (pExt->AEBuffs.Decloak)
	{
		R->EAX(false);
		return retn;
	}

	R->EAX(pExt->AEBuffs.Cloakable || pThis->Cloakable);
	return retn;
}

namespace Cache
{
	double Weight;
}

double* __fastcall TechnoClass_vt_entry_3D8_GetWeight(TechnoClass* pThis)
{
	const auto pExt = TechnoExt::ExtMap.Find(pThis);

	double weight = pThis->GetTechnoType()->Weight;

	Cache::Weight = weight * pExt->AEBuffs.WeightMul + pExt->AEBuffs.Weight;

	return &Cache::Weight;
}

//fdiv    qword ptr [eax+370h] -> fdiv    qword ptr [eax]
DEFINE_PATCH(0x70B3BA, 0x00, 0x00);

DEFINE_JUMP(CALL6, 0x70B3AE, GET_OFFSET(TechnoClass_vt_entry_3D8_GetWeight));

DEFINE_HOOK(0x737E00, UnitClass_ReceiveDamage_Sink, 0x6)
{
	GET(UnitClass*, pThis, ESI);

	const auto pExt = TechnoExt::ExtMap.Find(pThis);
	double weight = pThis->GetTechnoType()->Weight;

	weight = weight * pExt->AEBuffs.WeightMul + pExt->AEBuffs.Weight;

	if (weight < RulesClass::Instance->ShipSinkingWeight)
		return 0x737E63;

	return 0x737E18;
}

//心灵探测
DEFINE_HOOK(0x43B180, FootClass_ShouldDrawDashLine_AttachEffect, 0x5)
{
	enum{ ReturnFunction = 0x43B4A9 };

	GET(FootClass*, pThis, EBX);

	if (const auto pExt = TechnoExt::ExtMap.Find(pThis))
	{
		for (const auto& pAE : pExt->AttachEffects)
		{
			if (!pAE->IsActive())
				continue;

			const auto pAEType = pAE->Type;

			if (pAEType->PsychicDetection && EnumFunctions::CanTargetHouse(pAEType->PsychicDetection_ReceiveHouses, pAE->OwnerHouse, HouseClass::CurrentPlayer))
			{
				R->AL(true);
				return ReturnFunction;
			}
		}
	}

	return 0;
}

////狗哥太强啦
////hook from secsome
//DEFINE_HOOK_AGAIN(0x414826,AircraftClass_DrawAsVXL_DrawMatrix,0x5) //shadow
//DEFINE_HOOK(0x41496C, AircraftClass_DrawAsVXL_DrawMatrix, 0x5)	//body
//{
//	GET(AircraftClass*, pThis, EBP);
//	GET(Matrix3D*, pMatrix, EAX);
//
//	Matrix3D& matrix = MatrixCache::cache;
//	matrix = *pMatrix;
//
//	if (const auto pExt = TechnoExt::ExtMap.Find(pThis))
//	{
//		for (const auto pAE : pExt->GetActiveAE())
//		{
//			if (pAE->Type->Scale != 1.0f)
//			{
//				matrix.Scale(pAE->Type->Scale);
//
//				break;
//			}
//		}
//	}
//
//	R->EAX(&matrix);
//
//	return 0;
//}

//狗哥太强啦
//hook from secsome

//DEFINE_HOOK(0x4DAF10, FootClass_Draw_A_VXL, 0x5)
//{
//	GET(FootClass*, pThis, ECX);
//	GET_STACK(Matrix3D*, pMatrix, 0x1C);
//
//	const auto pExt = TechnoExt::ExtMap.Find(pThis);
//	for (const auto pAE : pExt->GetActiveAE())
//	{
//		if (pAE->Type->Scale != 1.0f)
//		{
//			pMatrix->Scale(pAE->Type->Scale);
//
//			break;
//		}
//	}
//
//	return 0;
//}
//
//DEFINE_HOOK(0x7067FB, TechnoClass_DrawVoxel_DisableCache, 0x5)
//{
//	GET(TechnoClass*, pThis, EBP);
//
//	bool scale = false;
//	const auto pExt = TechnoExt::ExtMap.Find(pThis);
//	for (const auto pAE : pExt->GetActiveAE())
//	{
//		if (pAE->Type->Scale != 1.0f)
//		{
//			scale = true;
//			break;
//		}
//	}
//
//	return scale ? 0x706875 : 0x0;
//}
//
////DEFINE_HOOK_AGAIN(0x73C504, UnitClass_DrawIt_VXL_Shadow_Matrix, 0x5)
////DEFINE_HOOK(0x73C59A, UnitClass_DrawIt_VXL_Shadow_Matrix, 0x5)
//DEFINE_HOOK_AGAIN(0x73C504, UnitClass_DrawIt_VXL_Shadow_Matrix, 0x5)
//DEFINE_HOOK(0x73C59E, UnitClass_DrawIt_VXL_Shadow_Matrix, 0x8)
//{
//	GET(UnitClass*, pThis, EBP);
//	GET(Matrix3D*, pMatrix, EAX);
//
//	const auto pExt = TechnoExt::ExtMap.Find(pThis);
//	for (const auto pAE : pExt->GetActiveAE())
//	{
//		if (pAE->Type->Scale != 1.0f)
//		{
//			pMatrix->Scale(pAE->Type->Scale);
//
//			break;
//		}
//	}
//
//	return 0;
//}
//
//DEFINE_HOOK(0x706C41, TechnoClass_DrawShadow_DisableCache, 0x5)
//{
//	GET(TechnoClass*, pThis, ESI);
//
//	bool scale = false;
//	const auto pExt = TechnoExt::ExtMap.Find(pThis);
//	for (const auto pAE : pExt->GetActiveAE())
//	{
//		if (pAE->Type->Scale != 1.0f)
//		{
//			scale = true;
//			break;
//		}
//	}
//
//	return scale ? 0x706CE1 : 0x0;
//}
