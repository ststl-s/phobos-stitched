#include "Body.h"

#include <BulletClass.h>
#include <UnitClass.h>
#include <Ext/House/Body.h>
#include <BitFont.h>
#include <Misc/FlyingStrings.h>

#include <Ext/House/Body.h>
#include <Ext/WarheadType/Body.h>

//After TechnoClass_AI?
DEFINE_HOOK(0x43FE69, BuildingClass_AI, 0xA)
{
	GET(BuildingClass*, pThis, ESI);

	// Do not search this up again in any functions called here because it is costly for performance - Starkku
	auto pExt = BuildingExt::ExtMap.Find(pThis);

	/*
	// Set only if unset or type has changed - Not currently useful as building type does not change.
	auto pType = pThis->Type;

	if (!pExt->TypeExtData || pExt->TypeExtData->OwnerObject() != pType)
		pExt->TypeExtData = BuildingTypeExt::ExtMap.Find(pType);
	*/

	pExt->DisplayGrinderRefund();
	pExt->ApplyPoweredKillSpawns();

	return 0;
}

DEFINE_HOOK(0x7396D2, UnitClass_TryToDeploy_Transfer, 0x5)
{
	GET(UnitClass*, pUnit, EBP);
	GET(BuildingClass*, pStructure, EBX);

	if (pUnit->Type->DeployToFire && pUnit->Target)
		pStructure->LastTarget = pUnit->Target;

	if (auto pStructureExt = BuildingExt::ExtMap.Find(pStructure))
		pStructureExt->DeployedTechno = true;

	return 0;
}

DEFINE_HOOK(0x449ADA, BuildingClass_MissionConstruction_DeployToFireFix, 0x0)
{
	GET(BuildingClass*, pThis, ESI);

	auto pExt = BuildingExt::ExtMap.Find(pThis);
	if (pExt && pExt->DeployedTechno && pThis->LastTarget)
	{
		pThis->Target = pThis->LastTarget;
		pThis->QueueMission(Mission::Attack, false);
	}
	else
	{
		pThis->QueueMission(Mission::Guard, false);
	}

	return 0x449AE8;
}

DEFINE_HOOK(0x4401BB, Factory_AI_PickWithFreeDocks, 0x6)
{
	GET(BuildingClass*, pBuilding, ESI);

	if (Phobos::Config::AllowParallelAIQueues && !RulesExt::Global()->ForbidParallelAIQueues_Aircraft)
		return 0;

	if (!pBuilding)
		return 0;

	HouseClass* pOwner = pBuilding->Owner;

	if (!pOwner)
		return 0;

	if (pOwner->Type->MultiplayPassive
		|| pOwner->IsCurrentPlayer()
		|| pOwner->IsNeutral())
		return 0;

	if (pBuilding->Type->Factory == AbstractType::AircraftType)
	{
		if (pBuilding->Factory
			&& !BuildingExt::HasFreeDocks(pBuilding))
		{
			auto BuildingExt = BuildingExt::ExtMap.Find(pBuilding);
			if (!BuildingExt)
				return 0;

			BuildingExt::UpdatePrimaryFactoryAI(pBuilding);
		}
	}

	return 0;
}

DEFINE_HOOK(0x44D455, BuildingClass_Mission_Missile_EMPPulseBulletWeapon, 0x8)
{
	GET(BuildingClass*, pThis, ESI);
	GET(WeaponTypeClass*, pWeapon, EBP);
	GET_STACK(BulletClass*, pBullet, STACK_OFFSET(0xF0, -0xA4));

	pBullet->SetWeaponType(pWeapon);

	CoordStruct src = pThis->GetFLH(0, pThis->GetRenderCoords());
	CoordStruct dest = pBullet->Target->GetCoords();

	if (pWeapon->IsLaser)
	{
		GameCreate<LaserDrawClass>(src, dest, pWeapon->LaserInnerColor, pWeapon->LaserOuterColor, pWeapon->LaserOuterSpread, pWeapon->LaserDuration);
	}

	return 0;
}

DEFINE_HOOK(0x44224F, BuildingClass_ReceiveDamage_DamageSelf, 0x5)
{
	enum { SkipCheck = 0x442268 };

	REF_STACK(args_ReceiveDamage const, receiveDamageArgs, STACK_OFFSET(0x9C, 0x4));

	if (auto const pWHExt = WarheadTypeExt::ExtMap.Find(receiveDamageArgs.WH))
	{
		if (pWHExt->AllowDamageOnSelf)
			return SkipCheck;
	}

	return 0;
}

DEFINE_HOOK(0x4502F4, BuildingClass_Update_Factory_Phobos, 0x6)
{
	GET(BuildingClass*, pThis, ESI);
	HouseClass* pOwner = pThis->Owner;

	if (pOwner->Production && Phobos::Config::AllowParallelAIQueues)
	{
		auto pOwnerExt = HouseExt::ExtMap.Find(pOwner);
		BuildingClass** currFactory = nullptr;
		switch (pThis->Type->Factory)
		{
		case AbstractType::BuildingType:
			currFactory = &pOwnerExt->Factory_BuildingType;
			break;
		case AbstractType::UnitType:
			currFactory = pThis->Type->Naval ? &pOwnerExt->Factory_NavyType : &pOwnerExt->Factory_VehicleType;
			break;
		case AbstractType::InfantryType:
			currFactory = &pOwnerExt->Factory_InfantryType;
			break;
		case AbstractType::AircraftType:
			currFactory = &pOwnerExt->Factory_AircraftType;
			break;
		}

		if (!currFactory)
		{
			Game::RaiseError(E_POINTER);
		}
		else if (!*currFactory)
		{
			*currFactory = pThis;
			return 0;
		}
		else if (*currFactory != pThis)
		{
			enum { Skip = 0x4503CA };

			switch (pThis->Type->Factory)
			{
			case AbstractType::BuildingType:
				if (RulesExt::Global()->ForbidParallelAIQueues_Building)
					return Skip;
			case AbstractType::InfantryType:
				if (RulesExt::Global()->ForbidParallelAIQueues_Infantry)
					return Skip;
			case AbstractType::AircraftType:
				if (RulesExt::Global()->ForbidParallelAIQueues_Aircraft)
					return Skip;
			case AbstractType::UnitType:
				if (pThis->Type->Naval ? RulesExt::Global()->ForbidParallelAIQueues_Navy : RulesExt::Global()->ForbidParallelAIQueues_Vehicle)
					return Skip;

			}
		}
	}

	return 0;
}

DEFINE_HOOK(0x4CA07A, FactoryClass_AbandonProduction_Phobos, 0x8)
{
	GET(FactoryClass*, pFactory, ESI);

	if (!Phobos::Config::AllowParallelAIQueues)
		return 0;

	HouseClass* pOwner = pFactory->Owner;
	HouseExt::ExtData* pOwnerExt = HouseExt::ExtMap.Find(pOwner);
	TechnoClass* pTechno = pFactory->Object;

	switch (pTechno->WhatAmI())
	{
	case AbstractType::Building:
		if (RulesExt::Global()->ForbidParallelAIQueues_Building)
			pOwnerExt->Factory_BuildingType = nullptr;
		break;
	case AbstractType::Unit:
		if (!pTechno->GetTechnoType()->Naval)
		{
			if (RulesExt::Global()->ForbidParallelAIQueues_Vehicle)
				pOwnerExt->Factory_VehicleType = nullptr;
		}
		else
		{
			if (RulesExt::Global()->ForbidParallelAIQueues_Navy)
				pOwnerExt->Factory_NavyType = nullptr;
		}
		break;
	case AbstractType::Infantry:
		if (RulesExt::Global()->ForbidParallelAIQueues_Infantry)
			pOwnerExt->Factory_InfantryType = nullptr;
		break;
	case AbstractType::Aircraft:
		if (RulesExt::Global()->ForbidParallelAIQueues_Aircraft)
			pOwnerExt->Factory_AircraftType = nullptr;
		break;
	}

	return 0;
}

DEFINE_HOOK(0x444119, BuildingClass_KickOutUnit_UnitType_Phobos, 0x6)
{
	GET(UnitClass*, pUnit, EDI);
	GET(BuildingClass*, pFactory, ESI);

	HouseExt::ExtData* pHouseExt = HouseExt::ExtMap.Find(pFactory->Owner);

	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pUnit->GetTechnoType());
	if (!pTypeExt->RandomProduct.empty())
	{
		int idx = ScenarioClass::Instance->Random(0, int(pTypeExt->RandomProduct.size()) - 1);
		TechnoTypeClass* pType = pTypeExt->RandomProduct[idx];
		UnitClass* pNewUnit = static_cast<UnitClass*>(pType->CreateObject(pUnit->GetOwningHouse()));
		pUnit->UnInit();
		R->EDI(pNewUnit);
		pUnit = pNewUnit;
	}
	if (!pUnit->Type->Naval)
	{
		pHouseExt->Factory_VehicleType = nullptr;
	}
	else
		pHouseExt->Factory_VehicleType = nullptr;

	return 0;
}

DEFINE_HOOK(0x444131, BuildingClass_KickOutUnit_InfantryType_Phobos, 0x6)
{
	GET(HouseClass*, pHouse, EAX);
	GET(InfantryClass*, pInf, EDI);

	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pInf->GetTechnoType());
	if (!pTypeExt->RandomProduct.empty())
	{
		int idx = ScenarioClass::Instance->Random(0, int(pTypeExt->RandomProduct.size()) - 1);
		TechnoTypeClass* pType = pTypeExt->RandomProduct[idx];
		InfantryClass* pNewInf = static_cast<InfantryClass*>(pType->CreateObject(pHouse));
		pInf->UnInit();
		R->EDI(pNewInf);
		pInf = pNewInf;
	}

	HouseExt::ExtMap.Find(pHouse)->Factory_InfantryType = nullptr;
	return 0;
}

DEFINE_HOOK(0x44531F, BuildingClass_KickOutUnit_BuildingType_Phobos, 0xA)
{
	GET(HouseClass*, pHouse, EAX);
	HouseExt::ExtMap.Find(pHouse)->Factory_BuildingType = nullptr;
	return 0;
}

DEFINE_HOOK(0x443CCA, BuildingClass_KickOutUnit_AircraftType_Phobos, 0xA)
{
	GET(HouseClass*, pHouse, EDX);
	GET(AircraftClass*, pAircraft, EBP);

	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pAircraft->GetTechnoType());
	if (!pTypeExt->RandomProduct.empty())
	{
		int idx = ScenarioClass::Instance->Random(0, int(pTypeExt->RandomProduct.size()) - 1);
		TechnoTypeClass* pType = pTypeExt->RandomProduct[idx];
		AircraftClass* pNewAircraft = static_cast<AircraftClass*>(pType->CreateObject(pHouse));
		pAircraft->UnInit();
		R->EBP(pNewAircraft);
		pAircraft = pNewAircraft;
	}

	HouseExt::ExtMap.Find(pHouse)->Factory_AircraftType = nullptr;
	return 0;
}

DEFINE_HOOK(0x51A2F1, Enter_Bio_Reactor_Sound, 0x6)
{
	GET(TechnoClass*, pThis, EDI);
	
	if (auto pBld = abstract_cast<BuildingClass*>(pThis))
	{
		CoordStruct coords = pThis->GetCoords();

		if (const auto pTypeExt = BuildingTypeExt::ExtMap.Find(pBld->Type))
			VocClass::PlayAt(pTypeExt->EnterBioReactorSound.Get(RulesClass::Instance->EnterBioReactorSound), coords);

		return 0x51A30F;
	}

	return 0;
}

DEFINE_HOOK(0x44DBBC, Leave_Bio_Reactor_Sound, 0x7)
{
	GET(BuildingClass*, pThis, EBP);
	
	CoordStruct coords = pThis->GetCoords();

	if (const auto pTypeExt = BuildingTypeExt::ExtMap.Find(pThis->Type))
		VocClass::PlayAt(pTypeExt->LeaveBioReactorSound.Get(RulesClass::Instance->LeaveBioReactorSound), coords);

	return 0x44DBDA;
}

// Note: We need another hook for when Phobos is run w/o Ares (or with Ares but with SpyEffect.Custom=no),
// that will handle our logic earlier and disable the next hook by writing a very specific number to a free register.
DEFINE_HOOK(0x45759D, BuildingClass_Infiltrate_Vanilla, 0x5)
{
	GET_STACK(HouseClass*, pInfiltratorHouse, STACK_OFFSET(0x14, -0x4));
	GET(BuildingClass*, pBuilding, EBP);

	BuildingExt::HandleInfiltrate(pBuilding, pInfiltratorHouse);
	R->EAX<int>(0x77777777);
	return 0;
}

// Note: Ares hooks at the start of the function and overwrites it entirely, so we have to do it here.
DEFINE_HOOK(0x4575A2, BuildingClass_Infiltrate_Custom, 0xE)
{
	// Check if we've handled it already
	if (R->EAX<int>() == 0x77777777)
	{
		R->EAX<int>(0);
		return 0;
	}

	GET_STACK(HouseClass*, pInfiltratorHouse, -0x4);
	GET(BuildingClass*, pBuilding, ECX);

	BuildingExt::HandleInfiltrate(pBuilding, pInfiltratorHouse);
	return 0;
}
