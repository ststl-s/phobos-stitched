#include "AttackedWeapon.h"

#include <Utilities/EnumFunctions.h>
#include <Utilities/TemplateDef.h>

#include <Ext/Techno/Body.h>

AttackedWeaponTypeClass::AttackedWeaponTypeClass()
	: AttackedWeapon {}
	, AttackedWeapon_FireToAttacker {}
	, AttackedWeapon_ROF {}
	, AttackedWeapon_IgnoreROF {}
	, AttackedWeapon_IgnoreRange {}
	, AttackedWeapon_Range {}
	, AttackedWeapon_ResponseWarhead {}
	, AttackedWeapon_NoResponseWarhead {}
	, AttackedWeapon_ResponseZeroDamage {}
	, AttackedWeapon_ResponseHouse {}
	, AttackedWeapon_ActiveMaxHealth {}
	, AttackedWeapon_ActiveMinHealth {}
	, AttackedWeapon_FLHs {}
{ }

void AttackedWeaponTypeClass::LoadFromINI(CCINIClass* pINI, const char* pSection)
{
	INI_EX exINI(pINI);

	this->AttackedWeapon.Read(exINI, pSection, "AttackedWeapon");
	this->AttackedWeapon_Veteran.Read(exINI, pSection, "AttackedWeapon.Veteran");
	this->AttackedWeapon_Elite.Read(exINI, pSection, "AttackedWeapon.Elite");
	this->AttackedWeapon_FireToAttacker.Read(exINI, pSection, "AttackedWeapon.FireToAttacker");
	this->AttackedWeapon_ROF.Read(exINI, pSection, "AttackedWeapon.ROF");
	this->AttackedWeapon_IgnoreROF.Read(exINI, pSection, "AttackedWeapon.IgnoreROF");
	this->AttackedWeapon_IgnoreRange.Read(exINI, pSection, "AttackedWeapon.IgnoreRange");
	this->AttackedWeapon_Range.Read(exINI, pSection, "AttackedWeapon.Range");
	this->AttackedWeapon_ResponseWarhead.Read(exINI, pSection, "AttackedWeapon.ResponseWarhead");
	this->AttackedWeapon_NoResponseWarhead.Read(exINI, pSection, "AttackedWeapon.NoResponseWarhead");
	this->AttackedWeapon_ResponseZeroDamage.Read(exINI, pSection, "AttackedWeapon.ResponseZeroDamage");
	this->AttackedWeapon_ActiveMaxHealth.Read(exINI, pSection, "AttackedWeapon.ActiveMaxHealth");
	this->AttackedWeapon_ActiveMinHealth.Read(exINI, pSection, "AttackedWeapon.ActiveMinHealth");

	char tempBuffer[256];

	for (size_t i = 0; i < AttackedWeapon.size(); i++)
	{
		Valueable<CoordStruct> flh;
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "AttackedWeapon%u.FLH", i);
		flh.Read(exINI, pSection, tempBuffer);
		AttackedWeapon_FLHs.emplace_back(flh.Get());

		Nullable<AffectedHouse> responseHouse;
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "AttackedWeapon%u.AffectedHouse", i);
		responseHouse.Read(exINI, pSection, tempBuffer);

		if (responseHouse.isset())
			AttackedWeapon_ResponseHouse.emplace_back(responseHouse.Get());
		else
			AttackedWeapon_ResponseHouse.emplace_back(AffectedHouse::All);
	}
}

bool AttackedWeaponTypeClass::ValidAttackedWeapon() const
{
	return !AttackedWeapon.empty()
		&& !AttackedWeapon_Veteran.empty()
		&& !AttackedWeapon_Elite.empty();
}

bool AttackedWeaponTypeClass::Load(PhobosStreamReader& stm, bool registerForChange)
{
	return this->Serialize(stm);
}

bool AttackedWeaponTypeClass::Save(PhobosStreamWriter& stm) const
{
	return const_cast<AttackedWeaponTypeClass*>(this)->Serialize(stm);
}

template <typename T>
bool AttackedWeaponTypeClass::Serialize(T& stm)
{
	return stm
		.Process(this->AttackedWeapon)
		.Process(this->AttackedWeapon_Veteran)
		.Process(this->AttackedWeapon_Elite)
		.Process(this->AttackedWeapon_ROF)
		.Process(this->AttackedWeapon_FireToAttacker)
		.Process(this->AttackedWeapon_IgnoreROF)
		.Process(this->AttackedWeapon_IgnoreRange)
		.Process(this->AttackedWeapon_Range)
		.Process(this->AttackedWeapon_ResponseWarhead)
		.Process(this->AttackedWeapon_NoResponseWarhead)
		.Process(this->AttackedWeapon_ResponseZeroDamage)
		.Process(this->AttackedWeapon_ResponseHouse)
		.Process(this->AttackedWeapon_ActiveMaxHealth)
		.Process(this->AttackedWeapon_ActiveMinHealth)
		.Process(this->AttackedWeapon_FLHs)
		.Success();
}

void AttackedWeaponTypeClass::ProcessAttackedWeapon(TechnoClass* pTechno, args_ReceiveDamage* args, bool beforeDamage) const
{
	if (!TechnoExt::IsReallyAlive(pTechno) || pTechno == args->Attacker)
		return;

	TechnoTypeClass* pTechnoType = pTechno->GetTechnoType();
	const ValueableVector<WeaponTypeClass*>* pWeapons = &this->AttackedWeapon;

	if (pTechno->Veterancy.IsVeteran() && !this->AttackedWeapon_Veteran.empty())
		pWeapons = &this->AttackedWeapon_Veteran;

	if (pTechno->Veterancy.IsElite() && !this->AttackedWeapon_Elite.empty())
		pWeapons = &this->AttackedWeapon_Elite;

	if (pWeapons->empty())
		return;

	std::vector<CDTimerClass>& timers = TechnoExt::ExtMap.Find(pTechno)->GetAttackedTimer(this);

	WarheadTypeClass* pWH = args->WH;
	WarheadTypeExt::ExtData* pWHExt = WarheadTypeExt::ExtMap.Find(pWH);
	HouseClass* pOwner = pTechno->GetOwningHouse();
	HouseClass* pAttacker = args->SourceHouse;

	if (pWHExt->AttackedWeapon_ForceNoResponse.Get()
		|| !pWHExt->AttackedWeapon_ResponseTechno.empty() && !pWHExt->AttackedWeapon_ResponseTechno.Contains(pTechnoType)
		|| !pWHExt->AttackedWeapon_NoResponseTechno.empty() && pWHExt->AttackedWeapon_NoResponseTechno.Contains(pTechnoType))
		return;

	//Debug::Log("[AttackedWeapon] Warhead Pass\n");

	if (!this->AttackedWeapon_ResponseWarhead.empty() && !this->AttackedWeapon_ResponseWarhead.Contains(pWH)
		|| !this->AttackedWeapon_NoResponseWarhead.empty() && this->AttackedWeapon_NoResponseWarhead.Contains(pWH))
		return;

	//Debug::Log("[AttackedWeapon] Techno Pass\n");

	const ValueableVector<int>& vROF = this->AttackedWeapon_ROF;
	const ValueableVector<bool>& vFireToAttacker = this->AttackedWeapon_FireToAttacker;
	const ValueableVector<bool>& vIgnoreROF = this->AttackedWeapon_IgnoreROF;
	const ValueableVector<bool>& vIgnoreRange = this->AttackedWeapon_IgnoreRange;
	const ValueableVector<Leptons>& vRange = this->AttackedWeapon_Range;
	const ValueableVector<bool>& vReponseZeroDamage = this->AttackedWeapon_ResponseZeroDamage;
	const std::vector<AffectedHouse>& vAffectHouse = this->AttackedWeapon_ResponseHouse;
	const ValueableVector<int>& vMaxHP = this->AttackedWeapon_ActiveMaxHealth;
	const ValueableVector<int>& vMinHP = this->AttackedWeapon_ActiveMinHealth;
	const std::vector<CoordStruct>& vFLH = this->AttackedWeapon_FLHs;

	while (timers.size() < pWeapons->size())
		timers.emplace_back(-1);

	while (timers.size() > pWeapons->size())
		timers.pop_back();

	for (size_t i = 0; i < pWeapons->size(); i++)
	{
		WeaponTypeClass* pWeapon = pWeapons->at(i);
		int iMaxHP = i < vMaxHP.size() ? vMaxHP[i] : INT_MAX;
		int iMinHP = i < vMinHP.size() ? vMinHP[i] : 0;
		int iROF = i < vROF.size() ? vROF[i] : pWeapon->ROF;
		CDTimerClass& timer = timers[i];
		bool bIgnoreROF = i < vIgnoreROF.size() ? vIgnoreROF[i] : false;
		bool bIsInROF = bIgnoreROF ? false : timers[i].Completed();
		bool bResponseZeroDamage = i < vReponseZeroDamage.size() ? vReponseZeroDamage[i] : false;

		if (iMaxHP < iMinHP)
			Debug::Log("[AttackedWeapon::Warning] TechnoType[%s] attacked weapon index[%u](start from 0) ActiveMaxHealth[%d] less than ActiveMinHealth[%d] !\n",
				pTechnoType->get_ID(), i, iMaxHP, iMinHP);

		if (pWeapon == nullptr
			|| bIsInROF
			|| bResponseZeroDamage && !beforeDamage
			|| !bResponseZeroDamage && (beforeDamage || *args->Damage == 0)
			|| pTechno->Health < iMinHP
			|| pTechno->Health > iMaxHP)
			continue;

		bool bFireToAttacker = i < vFireToAttacker.size() ? vFireToAttacker[i] : false;
		bool bIgnoreRange = i < vIgnoreRange.size() ? vIgnoreRange[i] : false;
		AffectedHouse affectedHouse = vAffectHouse[i];
		int iRange = i < vRange.size() ? vRange[i] : pWeapon->Range;
		CoordStruct crdFLH = vFLH[i];

		if (!EnumFunctions::CanTargetHouse(affectedHouse, pOwner, pAttacker))
			continue;

		if (bFireToAttacker)
		{
			if (TechnoExt::IsReallyAlive(args->Attacker))
			{
				if (bIgnoreRange || iRange >= pTechno->DistanceFrom(args->Attacker))
				{
					if (!bIgnoreROF)
						timer.Start(iROF);

					WeaponStruct weaponStruct;
					weaponStruct.WeaponType = pWeapon;
					weaponStruct.FLH = crdFLH;
					TechnoExt::SimulatedFire(pTechno, weaponStruct, args->Attacker);
				}
			}
		}
		else
		{
			if (!bIgnoreROF)
				timer.Start(iROF);
			else
				timer.Start(1);

			WeaponTypeExt::DetonateAt(pWeapon, pTechno->GetCoords(), pTechno);
		}
	}
}
