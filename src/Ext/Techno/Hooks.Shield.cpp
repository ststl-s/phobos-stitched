#include "Body.h"

#include <TunnelLocomotionClass.h>

#include <Utilities/Macro.h>

// Ares-hook jmp to this offset
DEFINE_HOOK(0x71A88D, TemporalClass_AI_Shield, 0x0)
{
	GET(TemporalClass*, pThis, ESI);
	if (auto const pTarget = pThis->Target)
	{
		const auto pExt = TechnoExt::ExtMap.Find(pTarget);

		if (const auto pShieldData = pExt->Shield.get())
		{
			if (pShieldData->IsAvailable())
				pShieldData->AI_Temporal();
		}
	}

	// Recovering vanilla instructions that were broken by a hook call
	return R->EAX<int>() <= 0 ? 0x71A895 : 0x71AB08;
}

DEFINE_HOOK(0x6F6AC4, TechnoClass_Remove_Shield, 0x5)
{
	GET(TechnoClass*, pThis, ECX);
	const auto pExt = TechnoExt::ExtMap.Find(pThis);

	if (pExt->Shield)
		pExt->Shield->KillAnim();

	return 0;
}

DEFINE_HOOK_AGAIN(0x44A03C, DeploysInto_UndeploysInto_SyncShieldStatus, 0x6) //BuildingClass_Mi_Selling_SyncShieldStatus
DEFINE_HOOK(0x739956, DeploysInto_UndeploysInto_SyncShieldStatus, 0x6) //UnitClass_Deploy_SyncShieldStatus
{
	GET(TechnoClass*, pFrom, EBP);
	GET(TechnoClass*, pTo, EBX);

	ShieldClass::SyncShieldToAnother(pFrom, pTo);
	GiftBoxClass::SyncToAnotherTechno(pFrom, pTo);
	TechnoExt::SyncIronCurtainStatus(pFrom, pTo);

	return 0;
}

DEFINE_HOOK(0x6F65D1, TechnoClass_DrawHealthBar_DrawBuildingShieldBar, 0x6)
{
	GET(TechnoClass*, pThis, ESI);
	GET(int, iLength, EBX);
	GET_STACK(const Point2D*, pLocation, STACK_OFFSET(0x4C, 0x4));
	GET_STACK(const RectangleStruct*, pBound, STACK_OFFSET(0x4C, 0x8));

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());

	const auto pExt = TechnoExt::ExtMap.Find(pThis);

	if (const auto pShieldData = pExt->Shield.get())
	{
		if (pShieldData->IsAvailable())
		{
			pShieldData->DrawShieldBar(iLength, *pLocation, *pBound);
		}
	}

	TechnoExt::ProcessDigitalDisplays(pThis);

	const bool customhealthbar = pTypeExt->UseCustomHealthBar.Get(RulesExt::Global()->CustomHealthBar.Get());

	if (pTypeExt->UseUnitHealthBar)
	{
		TechnoExt::DrawHealthBar_Other(pThis, iLength, *pLocation, *pBound);
	}
	else if (customhealthbar)
	{
		TechnoExt::DrawHealthBar_Building(pThis, iLength, *pLocation, *pBound);
	}

	if (customhealthbar || pTypeExt->UseUnitHealthBar)
		R->EBX(0);

	return 0;
}

DEFINE_HOOK(0x6F683C, TechnoClass_DrawHealthBar_DrawOtherShieldBar, 0x7)
{
	GET(TechnoClass*, pThis, ESI);
	GET_STACK(const Point2D*, pLocation, STACK_OFFSET(0x4C, 0x4));
	GET_STACK(const RectangleStruct*, pBound, STACK_OFFSET(0x4C, 0x8));

	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType());
	const auto pExt = TechnoExt::ExtMap.Find(pThis);
	const int iLength = pThis->WhatAmI() == AbstractType::Infantry ? 8 : 17;

	if (const auto pShieldData = pExt->Shield.get())
	{
		if (pShieldData->IsAvailable())
		{
			pShieldData->DrawShieldBar(iLength, *pLocation, *pBound);
		}
	}

	if (Phobos::Config::EnableSelectBox)
	{
		if (RulesExt::Global()->UseSelectBox)
		{
			if (pThis->WhatAmI() == AbstractType::Infantry)
				TechnoExt::DrawSelectBox(pThis, *pLocation, *pBound, true);
			else
				TechnoExt::DrawSelectBox(pThis, *pLocation, *pBound, false);
		}
	}

	TechnoExt::ProcessDigitalDisplays(pThis);

	if (pTypeExt->UseCustomHealthBar.Get(RulesExt::Global()->CustomHealthBar))
	{
		TechnoExt::DrawHealthBar_Other(pThis, iLength, *pLocation, *pBound);

		return 0x6F6AB6;
	}

	return 0;
}

DEFINE_HOOK(0x70A6FD, TechnoClass_Draw_GroupID, 0x6)
{
	GET(TechnoClass*, pThis, EBP);
	GET_STACK(const Point2D*, pLocation, STACK_OFFSET(0x74, 0x4));

	R->EDI(-1);

	if (const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->GetTechnoType()))
	{
		if (pThis->WhatAmI() == AbstractType::Building)
			TechnoExt::DrawGroupID_Building(pThis, *pLocation);
		else
			TechnoExt::DrawGroupID_Other(pThis, *pLocation);
	}

	return 0x70A703;
}


DEFINE_HOOK(0x728F74, TunnelLocomotionClass_Process_KillAnims, 0x5)
{
	GET(ILocomotion*, pThis, ESI);

	const auto pLoco = static_cast<TunnelLocomotionClass*>(pThis);
	const auto pExt = TechnoExt::ExtMap.Find(pLoco->LinkedTo);

	if (const auto pShieldData = pExt->Shield.get())
		pShieldData->SetAnimationVisibility(false);

	return 0;
}

DEFINE_HOOK(0x728E5F, TunnelLocomotionClass_Process_RestoreAnims, 0x7)
{
	GET(ILocomotion*, pThis, ESI);

	const auto pLoco = static_cast<TunnelLocomotionClass*>(pThis);

	if (pLoco->State == TunnelLocomotionClass::State::PreDigOut)
	{
		const auto pExt = TechnoExt::ExtMap.Find(pLoco->LinkedTo);

		if (const auto pShieldData = pExt->Shield.get())
			pShieldData->SetAnimationVisibility(true);
	}

	return 0;
}

#pragma region HealingWeapons

#pragma region TechnoClass_EvaluateObject

namespace EvaluateObjectTemp
{
	WeaponTypeClass* PickedWeapon = nullptr;
}

DEFINE_HOOK(0x6F7E24, TechnoClass_EvaluateObject_SetContext, 0x6)
{
	GET(WeaponTypeClass*, pWeapon, EBP);

	EvaluateObjectTemp::PickedWeapon = pWeapon;

	return 0;
}

double __fastcall HealthRatio_Wrapper(TechnoClass* pTechno)
{
	double result = pTechno->GetHealthPercentage();

	if (result >= 1.0)
	{
		if (const auto pExt = TechnoExt::ExtMap.Find(pTechno))
		{
			if (const auto pShieldData = pExt->Shield.get())
			{
				if (pShieldData->IsActive())
				{
					const auto pWH = EvaluateObjectTemp::PickedWeapon ? EvaluateObjectTemp::PickedWeapon->Warhead : nullptr;

					if (!pShieldData->CanBePenetrated(pWH))
						result = pExt->Shield->GetHealthRatio();
				}
			}
		}
	}

	return result;
}

DEFINE_JUMP(CALL, 0x6F7F51, GET_OFFSET(HealthRatio_Wrapper))

#pragma endregion TechnoClass_EvaluateObject

class AresScheme
{
	static inline ObjectClass* LinkedObj = nullptr;
public:
	static void __cdecl Prefix(TechnoClass* pThis, ObjectClass* pObj, int nWeaponIndex)
	{
		if (LinkedObj)
			return;

		if (nWeaponIndex < 0)
			nWeaponIndex = pThis->SelectWeapon(pObj);

		if (const auto pTechno = abstract_cast<TechnoClass*>(pObj))
		{
			if (const auto pExt = TechnoExt::ExtMap.Find(pTechno))
			{
				if (const auto pShieldData = pExt->Shield.get())
				{
					if (pShieldData->IsActive())
					{
						const auto pWeapon = pThis->GetWeapon(nWeaponIndex)->WeaponType;

						if (pWeapon && !pShieldData->CanBePenetrated(pWeapon->Warhead))
						{
							const auto shieldRatio = pExt->Shield->GetHealthRatio();

							if (shieldRatio < 1.0)
							{
								LinkedObj = pObj;
								--LinkedObj->Health;
							}
						}
					}
				}
			}
		}
	}

	static void __cdecl Suffix()
	{
		if (LinkedObj)
		{
			++LinkedObj->Health;
			LinkedObj = nullptr;
		}
	}

};

#pragma region UnitClass_GetFireError_Heal

FireError __fastcall UnitClass__GetFireError(UnitClass* pThis, void* _, ObjectClass* pObj, int nWeaponIndex, bool ignoreRange)
{
	JMP_THIS(0x740FD0);
}

FireError __fastcall UnitClass__GetFireError_Wrapper(UnitClass* pThis, void* _, ObjectClass* pObj, int nWeaponIndex, bool ignoreRange)
{
	AresScheme::Prefix(pThis, pObj, nWeaponIndex);
	auto const result = UnitClass__GetFireError(pThis, _, pObj, nWeaponIndex, ignoreRange);
	AresScheme::Suffix();
	return result;
}
DEFINE_JUMP(VTABLE, 0x7F6030, GET_OFFSET(UnitClass__GetFireError_Wrapper))
#pragma endregion UnitClass_GetFireError_Heal

#pragma region InfantryClass_GetFireError_Heal
FireError __fastcall InfantryClass__GetFireError(InfantryClass* pThis, void* _, ObjectClass* pObj, int nWeaponIndex, bool ignoreRange)
{
	JMP_THIS(0x51C8B0);
}
FireError __fastcall InfantryClass__GetFireError_Wrapper(InfantryClass* pThis, void* _, ObjectClass* pObj, int nWeaponIndex, bool ignoreRange)
{
	AresScheme::Prefix(pThis, pObj, nWeaponIndex);
	auto const result = InfantryClass__GetFireError(pThis, _, pObj, nWeaponIndex, ignoreRange);
	AresScheme::Suffix();
	return result;
}
DEFINE_JUMP(VTABLE, 0x7EB418, GET_OFFSET(InfantryClass__GetFireError_Wrapper))
#pragma endregion InfantryClass_GetFireError_Heal

#pragma region UnitClass__WhatAction
Action __fastcall UnitClass__WhatAction(UnitClass* pThis, void* _, ObjectClass* pObj, bool ignoreForce)
{
	JMP_THIS(0x73FD50);
}

Action __fastcall UnitClass__WhatAction_Wrapper(UnitClass* pThis, void* _, ObjectClass* pObj, bool ignoreForce)
{
	AresScheme::Prefix(pThis, pObj, -1);
	Action result = UnitClass__WhatAction(pThis, _, pObj, ignoreForce);
	AresScheme::Suffix();

	auto const& pExt = TechnoExt::ExtMap.Find(pThis);
	if (!pExt->ParentAttachment)
		return result;

	switch (result)
	{
	case Action::Repair:
		result = Action::NoRepair;
		break;

	case Action::Self_Deploy:
		if (pThis->Type->DeploysInto)
			result = Action::NoDeploy;
		break;

	case Action::Sabotage:
	case Action::Capture:
	case Action::Enter:
		result = Action::NoEnter;
		break;

	case Action::GuardArea:
	case Action::AttackMoveNav:
	case Action::Move:
		result = Action::NoMove;
		break;
	}

	return result;
}
DEFINE_JUMP(VTABLE, 0x7F5CE4, GET_OFFSET(UnitClass__WhatAction_Wrapper))
#pragma endregion UnitClass__WhatAction

#pragma region InfantryClass__WhatAction
Action __fastcall InfantryClass__WhatAction(InfantryClass* pThis, void* _, ObjectClass* pObj, bool ignoreForce)
{
	JMP_THIS(0x51E3B0);
}

Action __fastcall InfantryClass__WhatAction_Wrapper(InfantryClass* pThis, void* _, ObjectClass* pObj, bool ignoreForce)
{
	AresScheme::Prefix(pThis, pObj, -1);
	Action result = InfantryClass__WhatAction(pThis, _, pObj, ignoreForce);
	AresScheme::Suffix();

	return result;
}
DEFINE_JUMP(VTABLE, 0x7EB0CC, GET_OFFSET(InfantryClass__WhatAction_Wrapper))
#pragma endregion InfantryClass__WhatAction
#pragma endregion HealingWeapons
