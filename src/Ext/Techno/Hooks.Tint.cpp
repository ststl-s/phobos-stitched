#include "Body.h"

#include <Helpers/Macro.h>

#include <Ext/Anim/Body.h>

#include <Utilities/Macro.h>
#include <Utilities/GeneralUtils.h>

// YRDynamicPatcher-Kratos-0.7\DynamicPatcher\ExtensionHooks\TechnoExt.cs
DEFINE_HOOK(0x7063FF, TechnoClass_DrawSHP_Colour, 0x7)
{
	GET(TechnoClass*, pThis, ESI);

	auto pExt = TechnoExt::ExtMap.Find(pThis);

	if (pExt && pThis->WhatAmI() != AbstractType::Building)
	{
		for (const auto& pAE : pExt->AttachEffects)
		{
			if (!pAE->IsActive() || pAE->Type->Tint_Colors.empty())
				continue;

			R->EAX(pAE->GetCurrentTintColor());
		}
	}

	if (pThis && pExt && pExt->AllowToPaint && pThis->WhatAmI() != AbstractType::Building)
	{
		if (!pExt->Paint_IgnoreTintStatus && (pThis->IsIronCurtained() || pThis->ForceShielded || pThis->Berzerk))
			return 0;

		auto Color = Drawing::RGB_To_Int(pExt->ColorToPaint);
		R->EAX(Color);
	}

	if (pThis->Airstrike && pThis->Airstrike->Target == pThis)
		R->EAX(GeneralUtils::GetColorFromColorAdd(TechnoTypeExt::ExtMap.Find(pThis->Airstrike->Owner->GetTechnoType())->LaserTargetColor.Get(RulesClass::Instance->LaserTargetColor)));

	return 0;
}

// Ares-Version0A\src\Misc\Placeholders.cpp
DEFINE_HOOK(0x43D52D, BuildingClass_Draw_Tint, 0x5)
{
	GET(BuildingClass*, pThis, ESI);

	auto pExt = TechnoExt::ExtMap.Find(pThis);

	if (pExt)
	{
		for (const auto& pAE : pExt->AttachEffects)
		{
			if (!pAE->IsActive() || pAE->Type->Tint_Colors.empty())
				continue;

			R->EDI(pAE->GetCurrentTintColor());
			pThis->MarkForRedraw();
		}
	}

	if (pThis && pExt && pExt->AllowToPaint)
	{
		if (!pExt->Paint_IgnoreTintStatus && (pThis->IsIronCurtained() || pThis->ForceShielded || pThis->Berzerk))
			return 0;

		auto Color = Drawing::RGB_To_Int(pExt->ColorToPaint);
		R->EDI(Color);
		pThis->MarkForRedraw();
	}

	if (pThis->Airstrike && pThis->Airstrike->Target == pThis)
	{
		R->EDI(GeneralUtils::GetColorFromColorAdd(TechnoTypeExt::ExtMap.Find(pThis->Airstrike->Owner->GetTechnoType())->LaserTargetColor.Get(RulesClass::Instance->LaserTargetColor)));
		pThis->MarkForRedraw();
	}

	return 0;
}

// YRDynamicPatcher-Kratos-0.7\DynamicPatcher\ExtensionHooks\TechnoExt.cs
DEFINE_HOOK(0x706640, TechnoClass_DrawVXL_Tint, 0x5)
{
	GET(TechnoClass*, pThis, ECX);

	auto pExt = TechnoExt::ExtMap.Find(pThis);

	if (pExt)
	{
		for (const auto& pAE : pExt->AttachEffects)
		{
			if (!pAE->IsActive() || pAE->Type->Tint_Colors.empty())
				continue;

			int Color = pAE->GetCurrentTintColor();

			if (pThis->WhatAmI() != AbstractType::Building)
				R->ESI(Color);

			R->Stack(0x24, Color);
		}
	}

	if (pThis && pExt && pExt->AllowToPaint)
	{
		if (!pExt->Paint_IgnoreTintStatus && (pThis->IsIronCurtained() || pThis->ForceShielded || pThis->Berzerk))
			return 0;

		DWORD dwColor = Drawing::RGB_To_Int(pExt->ColorToPaint);

		if (pThis->WhatAmI() != AbstractType::Building)
			R->ESI(dwColor);

		R->Stack(0x24, dwColor);
	}

	if (pThis->Airstrike && pThis->Airstrike->Target == pThis)
		R->Stack(0x24, GeneralUtils::GetColorFromColorAdd(TechnoTypeExt::ExtMap.Find(pThis->Airstrike->Owner->GetTechnoType())->LaserTargetColor.Get(RulesClass::Instance->LaserTargetColor)));

	return 0;
}

// YRDynamicPatcher-Kratos-0.7\DynamicPatcher\ExtensionHooks\TechnoExt.cs
// Ares-Version0A\src\Misc\Placeholders.cpp
DEFINE_HOOK(0x73C15F, UnitClass_DrawVXL_Tint, 0x7)
{
	GET(UnitClass*, pUnit, EBP);

	if (auto pThis = abstract_cast<TechnoClass*>(pUnit))
	{
		auto pExt = TechnoExt::ExtMap.Find(pThis);

		if (pExt)
		{
			for (const auto& pAE : pExt->AttachEffects)
			{
				if (!pAE->IsActive() || pAE->Type->Tint_Colors.empty())
					continue;

				R->ESI(pAE->GetCurrentTintColor());
			}
		}

		if (pExt && pExt->AllowToPaint)
		{
			if (!pExt->Paint_IgnoreTintStatus && (pThis->IsIronCurtained() || pThis->ForceShielded || pThis->Berzerk))
				return 0;

			auto Color = Drawing::RGB_To_Int(pExt->ColorToPaint);
			R->ESI(Color);
		}
	}

	if (pUnit->Airstrike && pUnit->Airstrike->Target == pUnit)
		R->ESI(GeneralUtils::GetColorFromColorAdd(TechnoTypeExt::ExtMap.Find(pUnit->Airstrike->Owner->GetTechnoType())->LaserTargetColor.Get(RulesClass::Instance->LaserTargetColor)));

	return 0;
}

// YRDynamicPatcher-Kratos-0.7\DynamicPatcher\ExtensionHooks\AnimExt.cs
DEFINE_HOOK(0x423630, AnimClass_DrawBuildAnim_Tint, 0x6)
{
	GET(AnimClass*, pAnim, ESI);

	if (pAnim && pAnim->IsBuildingAnim)
	{
		CoordStruct location = pAnim->GetCoords();
		BuildingClass* pBuilding = MapClass::Instance->TryGetCellAt(location)->GetBuilding();
		if (auto pThis = abstract_cast<TechnoClass*>(pBuilding))
		{
			auto pExt = TechnoExt::ExtMap.Find(pThis);

			if (pExt)
			{
				for (const auto& pAE : pExt->AttachEffects)
				{
					if (!pAE->IsActive() || pAE->Type->Tint_Colors.empty())
						continue;

					R->EBP(pAE->GetCurrentTintColor());
				}
			}

			if (pExt && pExt->AllowToPaint)
			{
				if (!pExt->Paint_IgnoreTintStatus && (pThis->IsIronCurtained() || pThis->ForceShielded || pThis->Berzerk))
					return 0;

				auto Color = Drawing::RGB_To_Int(pExt->ColorToPaint);
				R->EBP(Color);
			}

			if (pThis->Airstrike && pThis->Airstrike->Target == pThis)
				R->EBP(GeneralUtils::GetColorFromColorAdd(TechnoTypeExt::ExtMap.Find(pThis->Airstrike->Owner->GetTechnoType())->LaserTargetColor.Get(RulesClass::Instance->LaserTargetColor)));
		}
	}

	return 0;
}

// 葱bos的亮度
DEFINE_HOOK(0x706389, TechnoClass_DrawShape_Intensity, 0x6)
{
	GET(TechnoClass*, pThis, ESI);

	const auto pExt = TechnoExt::ExtMap.Find(pThis);
	if (!pExt)
		return 0;

	GET(int, intensity, EBP);

	const auto whatAmI = pThis->WhatAmI();
	const bool isBuilding = whatAmI == AbstractType::Building;
	double intensityFactor = TechnoExt::GetDeactivateDim(pThis, isBuilding);

	if (intensityFactor != 1.0)
		R->EBP(std::clamp(static_cast<int>(intensity * intensityFactor), 0, 2000));

	if (pExt->CurrtenIntensityFactor != intensityFactor)
	{
		pThis->MarkForRedraw();
		pExt->CurrtenIntensityFactor = intensityFactor;
		Debug::Log("name:%s, level: %d\n", pThis->get_ID(), intensityFactor);
	}

	return 0;
}

DEFINE_HOOK(0x7067EF, TechnoClass_DrawVoxel_Intensity, 0x8)
{
	GET(TechnoClass*, pThis, EBP);

	const auto pExt = TechnoExt::ExtMap.Find(pThis);
	if (!pExt)
		return 0;
	GET(int, intensity, EDI);
	const auto whatAmI = pThis->WhatAmI();
	const bool isBuilding = whatAmI == AbstractType::Building;

	double intensityFactor = 1.0;

	if (whatAmI != AbstractType::Unit)
		intensityFactor *= TechnoExt::GetDeactivateDim(pThis, isBuilding);

	if (intensityFactor != 1.0)
	{
		R->EDI(std::clamp(static_cast<int>(intensity * intensityFactor), 0, 2000));
	}

	if (pExt->CurrtenIntensityFactor != intensityFactor)
	{
		pThis->MarkForRedraw();
		pExt->CurrtenIntensityFactor = intensityFactor;
	}

	if (R->EAX() != R->ESI())
		return pExt->ParentAttachment ? 0x706875 : 0x7067F7;

	return 0x706879;
}

DEFINE_HOOK(0x43FA19, BuildingClass_Mark_TintIntensity, 0x7)
{
	GET(BuildingClass*, pThis, EDI);

	const auto pExt = TechnoExt::ExtMap.Find(pThis);
	if (!pExt)
		return 0;

	GET(int, intensity, ESI);
	double intensityFactor = 1.0;

	intensityFactor *= TechnoExt::GetDeactivateDim(pThis, true);

	if (intensityFactor != 1.0)
		R->ESI(std::clamp(static_cast<int>(intensity * intensityFactor), 0, 2000));

	return 0;
}

DEFINE_HOOK(0x51946D, InfantryClass_Draw_TintIntensity, 0x6)
{
	GET(InfantryClass*, pThis, EBP);

	const auto pExt = TechnoExt::ExtMap.Find(pThis);
	if (!pExt)
		return 0;

	GET(int, intensity, ESI);

	if (pThis->IsIronCurtained())
		intensity = pThis->GetInvulnerabilityTintIntensity(intensity);

	if (pThis->Airstrike && pThis->Airstrike->Target == pThis)
		intensity = pThis->GetAirstrikeTintIntensity(intensity);

	double intensityFactor = 1.0;

	intensityFactor *= TechnoExt::GetDeactivateDim(pThis, false);
	R->ESI(std::clamp(static_cast<int>(intensity * intensityFactor), 0, 2000));

	return 0;
}

namespace AnimDrawTemp
{
	BuildingClass* ParentBuilding;
}

DEFINE_HOOK(0x0423420, AnimClass_Draw_ParentBuildingCheck, 0x6)
{
	GET(AnimClass*, pThis, ESI);
	GET(BuildingClass*, pBuilding, EAX);

	AnimDrawTemp::ParentBuilding = pBuilding;

	if (!pBuilding)
	{
		const auto pExt = AnimExt::ExtMap.Find(pThis);
		R->EAX(pExt->ParentBuilding);
		AnimDrawTemp::ParentBuilding = pExt->ParentBuilding;
	}

	return 0;
}

DEFINE_HOOK(0x4235D3, AnimClass_Draw_TintColor, 0x6)
{
	const auto pBuilding = AnimDrawTemp::ParentBuilding;

	if (!pBuilding)
		return 0;

	AnimDrawTemp::ParentBuilding = nullptr;

	GET(AnimClass*, pThis, ESI);
	REF_STACK(int, intensity, STACK_OFFSET(0x110, -0xD8));
	double intensityFactor = 1.0;
	const auto pExt = TechnoExt::ExtMap.Find(pBuilding);
	if (!pExt)
		return 0;

	intensityFactor *= TechnoExt::GetDeactivateDim(pBuilding, true);

	if (pThis->Type->UseNormalLight)
		intensity = 1000;
	else if (intensityFactor != 1.0)
		intensity = std::clamp(static_cast<int>(intensity * intensityFactor), 0, 2000);

	return 0;
}

// 葱的空袭激光颜色
DEFINE_JUMP(LJMP, 0x6D481D, 0x6D482D)

namespace LaserTargetTemp
{
	TechnoClass* Owner = nullptr;
	int Color = 0;
}

DEFINE_HOOK(0x6D48E3, TacticalClass_Render_AirstrikeLaserTarget_GetOwner, 0x6)
{
	LaserTargetTemp::Owner = R->ESI<TechnoClass*>();
	return 0;
}

DEFINE_HOOK(0x7058F6, Sub_705860_AitrstrikeTargetLaser, 0x5)
{
	enum { SkipGameCode = 0x70597A };

	REF_STACK(ColorStruct, color, STACK_OFFSET(0x70, -0x60));
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(LaserTargetTemp::Owner->GetTechnoType());
	const int colorIndex = pTypeExt->AirstrikeLaserColor.Get(pTypeExt->LaserTargetColor.Get(RulesExt::Global()->AirstrikeLaserColor.Get(RulesClass::Instance->LaserTargetColor)));
	LaserTargetTemp::Color = GeneralUtils::GetColorFromColorAdd(colorIndex);
	color = Drawing::Int_To_RGB(LaserTargetTemp::Color);

	return SkipGameCode;
}

DEFINE_HOOK(0x705986, Sub_705860_AitrstrikeTargetPoint, 0x6)
{
	enum { SkipGameCode = 0x7059C7 };

	R->ECX(R->Stack<DWORD>(STACK_OFFSET(0x70, -0x38)));
	R->ESI(LaserTargetTemp::Color);

	return SkipGameCode;
}

DEFINE_HOOK(0x43D39C, BuildingClass_Draw_LaserTargetColor, 0x6)
{
	enum { SkipGameCode = 0x43D3A2 };

	GET(BuildingClass*, pThis, ESI);
	GET(RulesClass*, pRules, ECX);
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Airstrike->Owner->GetTechnoType());
	R->EAX(pTypeExt->LaserTargetColor.Get(pRules->LaserTargetColor));

	return SkipGameCode;
}

DEFINE_HOOK(0x43DC36, BuildingClass_DrawFogged_LaserTargetColor, 0x6)
{
	enum { SkipGameCode = 0x43DC3C };

	GET(BuildingClass*, pThis, EBP);
	GET(RulesClass*, pRules, ECX);
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Airstrike->Owner->GetTechnoType());
	R->EAX(pTypeExt->LaserTargetColor.Get(pRules->LaserTargetColor));

	return SkipGameCode;
}

DEFINE_HOOK(0x42343C, AnimClass_Draw_LaserTargetColor, 0x6)
{
	enum { SkipGameCode = 0x423448 };

	GET(BuildingClass*, pThis, ECX);
	const auto pRules = RulesClass::Instance();
	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Airstrike->Owner->GetTechnoType());
	R->ECX(pRules);
	R->EAX(pTypeExt->LaserTargetColor.Get(pRules->LaserTargetColor));

	return SkipGameCode;
}
