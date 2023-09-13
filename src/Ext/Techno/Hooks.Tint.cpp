#include "Body.h"

#include <Helpers/Macro.h>

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
		}
	}

	if (pThis && pExt && pExt->AllowToPaint)
	{
		if (!pExt->Paint_IgnoreTintStatus && (pThis->IsIronCurtained() || pThis->ForceShielded || pThis->Berzerk))
			return 0;

		auto Color = Drawing::RGB_To_Int(pExt->ColorToPaint);
		R->EDI(Color);
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
		}
	}

	return 0;
}
