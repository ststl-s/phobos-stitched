#include "SelectSW.h"

#include <Utilities/GeneralUtils.h>

#include <SessionClass.h>

#include <Ext/Network/Body.h>
#include <Ext/House/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/TechnoType/Body.h>

const char* SelectSWCommandClass::GetName() const
{
	return "Select SuperWeapon";
}

const wchar_t* SelectSWCommandClass::GetUIName() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_SELECT_SW", L"Select SW");
}

const wchar_t* SelectSWCommandClass::GetUICategory() const
{
	return CATEGORY_DEVELOPMENT;
}

const wchar_t* SelectSWCommandClass::GetUIDescription() const
{
	return GeneralUtils::LoadStringUnlessMissing("TXT_SELECT_SW_DESC", L"Select SuperWeapon for techno currently selected with hotkey.");
}

void SelectSWCommandClass::Execute(WWKey eInput) const
{
	auto PrintMessage = [](const wchar_t* pMessage)
		{
			MessageListClass::Instance->PrintMessage(
				pMessage,
				RulesClass::Instance->MessageDelay,
				HouseClass::CurrentPlayer->ColorSchemeIndex,
				true
			);
		};

	if (SessionClass::IsSingleplayer())
	{
		auto pHouse = HouseClass::CurrentPlayer.get();
		if (pHouse->Defeated)
			return;

		if (SessionClass::Instance->IsCampaign())
		{
			if (const auto pHouseExt = HouseExt::ExtMap.Find(pHouse))
			{
				for (auto pTechnoType : *TechnoTypeClass::Array)
				{
					auto pTypeExt = TechnoTypeExt::ExtMap.Find(pTechnoType);
					if (pTypeExt->SuperWeapon_Quick.empty())
						continue;

					const auto& vTechnos = HouseExt::GetOwnedTechno(pHouse, pTechnoType);
					for (size_t i = 0; i < vTechnos.size(); i++)
					{
						for (size_t j = 0; j < pTypeExt->SuperWeapon_Quick.size(); j++)
						{
							if (pHouseExt->ToSelectSW_List.Contains(pTypeExt->SuperWeapon_Quick[j]))
							{
								if (pTypeExt->SuperWeapon_Quick_RealLaunch.size() > j)
								{
									if (pTypeExt->SuperWeapon_Quick_RealLaunch[j] == false)
									{
										pHouseExt->ToSelectSW_RealLaunch[j] = false;
									}
								}
							}
							else
							{
								pHouseExt->ToSelectSW_List.emplace_back(pTypeExt->SuperWeapon_Quick[j]);
								if (pTypeExt->SuperWeapon_Quick_RealLaunch.size() > j)
								{
									pHouseExt->ToSelectSW_RealLaunch.emplace_back(pTypeExt->SuperWeapon_Quick_RealLaunch[j]);
								}
								else
								{
									pHouseExt->ToSelectSW_RealLaunch.emplace_back(true);
								}
							}
						}
					}
				}
				pHouseExt->ToSelectSW = true;
				HouseExt::SelectSW(pHouse);
			}
		}
		else
		{
			for (auto pTechno : *TechnoClass::Array)
			{
				if (pTechno->Owner == pHouse)
				{
					ExtraPhobosNetEvent::Handlers::RaiseToSelectSW(pTechno);
					break;
				}
			}
		}
	}
	else
	{
		PrintMessage(StringTable::LoadString("MSG:NotAvailableInMultiplayer"));
	}
}
