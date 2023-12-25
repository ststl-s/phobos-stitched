#include "Convert.h"

#include <Utilities/GeneralUtils.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>
#include <JumpjetLocomotionClass.h>

const char* ConvertCommandClass::GetName() const
{
	return "Convert";
}

const wchar_t* ConvertCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_CONVERT", L"Convert");
}

const wchar_t* ConvertCommandClass::GetUICategory() const
{
	return CATEGORY_CONTROL;
}

const wchar_t* ConvertCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_CONVERT_DESC", L"Let selected units convert if they can.");
}

void ConvertCommandClass::Execute(WWKey eInput) const
{
	if (this->CheckDebugDeactivated())
		return;

	auto pObjectToSelect = MapClass::Instance->NextObject(
		ObjectClass::CurrentObjects->Count ? ObjectClass::CurrentObjects->GetItem(0) : nullptr);

	auto pNextObject = pObjectToSelect;

	do
	{
		if (auto pTechno = abstract_cast<TechnoClass*>(pNextObject))
		{
			if (pTechno->IsSelected)
			{
				auto pExt = TechnoExt::ExtMap.Find(pTechno);
				auto pTypeExt = pExt->TypeExtData;

				if (pTypeExt->Convert_Command != nullptr)
				{
					TechnoExt::Convert(pTechno, pTypeExt->Convert_Command);
					const auto pFoot = abstract_cast<FootClass*>(pTechno);
					auto pType = pTechno->GetTechnoType();
					if (auto const pJJLoco = locomotion_cast<JumpjetLocomotionClass*>(pFoot->Locomotor))
					{
						if (pType->BalloonHover)
						{
							pJJLoco->State = JumpjetLocomotionClass::State::Hovering;
							pJJLoco->IsMoving = true;
							pJJLoco->DestinationCoords = pTechno->Location;
							pJJLoco->CurrentHeight = pType->JumpjetHeight;
						}
						else
							pJJLoco->Move_To(pTechno->Location);
					}
					else
					{
						if (pType->Locomotor != LocomotionClass::CLSIDs::Hover.get())
							pTechno->IsFallingDown = true;

						pExt->WasFallenDown = true;
					}
				}
			}
		}

		pNextObject = MapClass::Instance->NextObject(pNextObject);
	}
	while (pNextObject != pObjectToSelect);
}
