#include "TemperatureTypeClass.h"

#include <Ext/Techno/Body.h>
#include <Ext/WeaponType/Body.h>

#include <Misc/PhobosGlobal.h>

#include <Utilities/TemplateDef.h>

Enumerable<TemperatureTypeClass>::container_t Enumerable<TemperatureTypeClass>::Array;

const char* Enumerable<TemperatureTypeClass>::GetMainSection()
{
	return "TemperatureTypes";
}

void TemperatureTypeClass::LoadFromINI(CCINIClass* pINI)
{
	const char* pSection = this->Name;

	INI_EX exINI(pINI);

	for (int i = 0;; i++)
	{
		char key[0x40];
		Nullable<AttachEffectTypeClass*> attachEffect;
		Nullable<WeaponTypeClass*> weapon;
		Nullable<double> below;

		sprintf_s(key, "AttachEffect%d", i);
		attachEffect.Read(exINI, pSection, key, true);
		sprintf_s(key, "Weapon%d", i);
		weapon.Read(exINI, pSection, key, true);
		sprintf_s(key, "Percent%d", i);
		below.Read(exINI, pSection, key);

		if (below.isset() && (attachEffect.isset() || weapon.isset()))
			Effects.emplace(below, std::make_pair(attachEffect.Get(), weapon.Get()));
		else
			break;
	}

	Minimum.Read(exINI, pSection, "Minimum");
	HeatUp_Frame.Read(exINI, pSection, "HeatUp.Frame");
	HeatUp_Percent.Read(exINI, pSection, "HeatUp.Percent");
	Enable.Read(exINI, pSection, "Enable");
	HeatUp_Delay.Read(exINI, pSection, "HeatUp.Delay");
}

void TemperatureTypeClass::Update(TechnoClass* pTechno) const
{
	if (!Enable)
		return;

	TechnoTypeClass* pType = pTechno->GetTechnoType();
	TechnoTypeExt::ExtData* pTypeExt = TechnoTypeExt::ExtMap.Find(pType);

	if (pTypeExt->Temperatrue_Disable[ArrayIndex])
		return;

	TechnoExt::ExtData* pExt = TechnoExt::ExtMap.Find(pTechno);
	int& iTemp = pExt->Temperature[ArrayIndex];
	int	iTempMax = pTypeExt->Temperature[ArrayIndex];
	CDTimerClass& timer = pExt->Temperature_HeatUpTimer[ArrayIndex];
	CDTimerClass& delayTimer = pExt->Temperature_HeatUpDelayTimer[ArrayIndex];

	if (delayTimer.Completed() && timer.Expired())
	{
		int iHeatUpAmount =
			pTypeExt->Temperature_HeatUpAmount.count(ArrayIndex)
			? pTypeExt->Temperature_HeatUpAmount[ArrayIndex]
			: std::max(Game::F2I(iTempMax * HeatUp_Percent), 1);
		iTemp += iHeatUpAmount;
		iTemp = std::min(iTemp, iTempMax);
		iTemp = std::max(iTemp, Minimum.Get());
		timer.Start(
			pTypeExt->Temperature_HeatUpFrame.count(ArrayIndex)
			? pTypeExt->Temperature_HeatUpFrame[ArrayIndex]
			: HeatUp_Frame);
	}

	auto it = Effects.lower_bound(static_cast<double>(iTemp) / iTempMax);

	if (it != Effects.end())
	{
		AttachEffectTypeClass* pAE = it->second.first;
		WeaponTypeClass* pWeapon = it->second.second;
		CDTimerClass& weaponTimer = pExt->Temperature_WeaponTimer[ArrayIndex];

		if (pWeapon != nullptr && weaponTimer.Expired())
		{
			WeaponTypeExt::DetonateAt(pWeapon, pTechno, PhobosGlobal::Global()->GetGenericStand());
			weaponTimer.Start(pWeapon->ROF);
		}

		TechnoExt::AttachEffect(pTechno, PhobosGlobal::Global()->GetGenericStand(), pAE);
	}
}

#pragma region(save/load)

template <typename T>
void TemperatureTypeClass::Serialize(T& stm)
{
	stm
		.Process(this->Effects)
		.Process(this->Minimum)
		.Process(this->HeatUp_Frame)
		.Process(this->HeatUp_Percent)
		.Process(this->HeatUp_Delay)
		.Process(this->Enable)
		;
}

void TemperatureTypeClass::LoadFromStream(PhobosStreamReader& stm)
{
	Serialize(stm);
}

void TemperatureTypeClass::SaveToStream(PhobosStreamWriter& stm)
{
	Serialize(stm);
}

#pragma endregion(save/load)
