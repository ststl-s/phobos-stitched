#include "CrateClass.h"

#include <Ext/Anim/Body.h>
#include <Ext/Techno/Body.h>
#include <Ext/WeaponType/Body.h>
#include <Ext/House/Body.h>

#include <Misc/PhobosGlobal.h>
#include <Misc/FlyingStrings.h>

#include <Utilities/EnumFunctions.h>
#include <Utilities/Helpers.Alex.h>
#include <Utilities/TemplateDef.h>

#include <JumpjetLocomotionClass.h>

std::unordered_map<int, int> CrateClass::Crate_Exist;

CrateClass::CrateClass(CrateTypeClass* pType, CellClass* pCell, int duration, HouseClass* pHouse)
	: Type(pType)
	, Duration(duration)
	, Timer()
	, Location(pCell)
	, Initialized(false)
	, Anim(nullptr)
	, IsInvalid(false)
	, CrateEffects()
	, CrateEffects_Weights()
	, LastObjectList()
	, OwnerHouse(pHouse)
{
	++Crate_Exist[Type->ArrayIndex];

	if (Type->Weapon_Weight > 0 && !Type->Weapons.empty())
	{
		CrateEffects.emplace_back(CrateEffect::Weapon);
		CrateEffects_Weights.emplace_back(Type->Weapon_Weight);
	}

	if (Type->AttachEffect_Weight > 0 && !Type->AttachEffects.empty())
	{
		CrateEffects.emplace_back(CrateEffect::AttachEffect);
		CrateEffects_Weights.emplace_back(Type->AttachEffect_Weight);
	}

	if (Type->Money_Weight > 0 && !(Type->Money.Get().X == 0 && Type->Money.Get().Y == 0))
	{
		CrateEffects.emplace_back(CrateEffect::Money);
		CrateEffects_Weights.emplace_back(Type->Money_Weight);
	}

	if (Type->SpySat_Weight > 0 && !(Type->SpySat_Range.Get().X == 0 && Type->SpySat_Range.Get().Y == 0))
	{
		CrateEffects.emplace_back(CrateEffect::SpySat);
		CrateEffects_Weights.emplace_back(Type->SpySat_Weight);
	}

	if (Type->Darkness_Weight > 0)
	{
		CrateEffects.emplace_back(CrateEffect::Darkness);
		CrateEffects_Weights.emplace_back(Type->Darkness_Weight);
	}

	if (Type->SuperWeapon_Weight > 0 && !Type->SuperWeapons.empty())
	{
		CrateEffects.emplace_back(CrateEffect::SuperWeapon);
		CrateEffects_Weights.emplace_back(Type->SuperWeapon_Weight);
	}

	if (Type->RadarJam_Weight > 0 && !(Type->RadarJam_Duration.Get().X == 0 && Type->RadarJam_Duration.Get().Y == 0) && Type->RadarJam_Duration.Get().X > 0)
	{
		CrateEffects.emplace_back(CrateEffect::RadarJam);
		CrateEffects_Weights.emplace_back(Type->RadarJam_Weight);
	}

	if (Type->PowerOutage_Weight > 0 && !(Type->PowerOutage_Duration.Get().X == 0 && Type->PowerOutage_Duration.Get().Y == 0) && Type->PowerOutage_Duration.Get().X > 0)
	{
		CrateEffects.emplace_back(CrateEffect::PowerOutage);
		CrateEffects_Weights.emplace_back(Type->PowerOutage_Weight);
	}

	if (Type->Unit_Weight > 0 && !Type->Units.empty())
	{
		CrateEffects.emplace_back(CrateEffect::Unit);
		CrateEffects_Weights.emplace_back(Type->Unit_Weight);
	}

	Init();
}

CrateClass::~CrateClass()
{
	this->Initialized = false;
	this->KillImage();

	--Crate_Exist[Type->ArrayIndex];
}

bool CrateClass::CanExist(CrateTypeClass* pType)
{
	if (pType != nullptr && (pType->Maximum < 0 || Crate_Exist[pType->ArrayIndex] < pType->Maximum))
		return true;

	return false;
}

bool CrateClass::CheckMinimum(CrateTypeClass* pType)
{
	if (pType != nullptr && Crate_Exist[pType->ArrayIndex] >= pType->Minimum)
		return true;

	return false;
}

bool CrateClass::CanSpwan(CrateTypeClass* pType, CellClass* pCell, bool ignoreoccupied)
{
	if (pCell->ContainsBridge())
		return false;

	if (!ignoreoccupied)
	{
		if (TechnoExt::IsReallyAlive(pCell->GetInfantry(false)) ||
			TechnoExt::IsReallyAlive(pCell->GetUnit(false)) ||
			TechnoExt::IsReallyAlive(pCell->GetBuilding()) ||
			TechnoExt::IsReallyAlive(pCell->GetAircraft(false)))
		{
			return false;
		}
	}

	bool iswater = pCell->Tile_Is_Water() && !pCell->ContainsBridge();

	bool occupied = false;
	auto& cratelist = PhobosGlobal::Global()->Crate_Cells;
	auto it = std::find(cratelist.begin(), cratelist.end(), pCell);
	if (it != cratelist.end())
		occupied = true;

	if (occupied)
		return false;
	else
	{
		if (iswater)
		{
			if (!pType->AllowSpwanOnWater)
				return false;
		}
		else
		{
			if (!pType->AllowSpwanOnLand || pCell->Passability)
				return false;
		}
	}

	return true;
}

void CrateClass::Init()
{

	if (!Initialized)
	{
		Timer.Start(Duration < 0 ? (1 << 30) : Duration);
		Initialized = true;
	}

	CreateImage();
}

void CrateClass::CreateImage()
{
	const CrateTypeClass* pType = this->Type;
	auto ImageType = (this->Location->Tile_Is_Water() && !this->Location->ContainsBridge()) ? pType->WaterImage : pType->Image;

	if (ImageType == nullptr || this->Anim != nullptr)
		return;

	const CoordStruct& location = this->Location->ContainsBridge() ? this->Location->GetCoordsWithBridge() : this->Location->GetCoords();

	this->Anim = GameCreate<AnimClass>
		(
			ImageType,
			location
		);

	if (this->Anim != nullptr)
	{
		this->Anim->RemainingIterations = 0xFFU;
		if (this->OwnerHouse)
			this->Anim->Owner = this->OwnerHouse;
	}
}

void CrateClass::KillImage()
{
	if (this->Anim != nullptr && AnimExt::ExtMap.Find(this->Anim) != nullptr)
		this->Anim->UnInit();

	this->Anim = nullptr;
}

void CrateClass::Update()
{
	if (this->IsInvalid)
		return;

	if (!this->Initialized)
		this->Init();

	if (this->OwnerHouse && this->OwnerHouse->Defeated)
		this->OwnerHouse = nullptr;

	auto pObj = this->Location->FirstObject;

	ValueableVector<ObjectClass*> CurrentObjectList;
	size_t idx = 0;
	bool NeedUpdate = false;
	while (pObj)
	{
		CurrentObjectList.emplace_back(pObj);

		if (LastObjectList.size() <= idx)
			NeedUpdate = true;
		else if (LastObjectList[idx] != pObj)
			NeedUpdate = true;

		pObj = pObj->NextObject;
		idx++;
	}

	if (!NeedUpdate)
		return;

	LastObjectList = CurrentObjectList;

	for (size_t i = 0; i < LastObjectList.size(); i++)
	{
		if (this->OwnerHouse && !EnumFunctions::CanTargetHouse(this->Type->AllowPick_Houses.Get(), LastObjectList[i]->GetOwningHouse(), this->OwnerHouse))
			continue;

		if (const auto pBld = abstract_cast<BuildingClass*>(LastObjectList[i]))
		{
			this->IsInvalid = true;
			break;
		}
		else if (const auto pUnit = abstract_cast<UnitClass*>(LastObjectList[i]))
		{
			if ((!this->Type->AllowPick_UnitTypes.empty() && !this->Type->AllowPick_UnitTypes.Contains(pUnit->GetTechnoType())) ||
				(!this->Type->DisallowPick_UnitTypes.empty() && this->Type->DisallowPick_UnitTypes.Contains(pUnit->GetTechnoType())))
				continue;

			OpenCrate(pUnit);
			break;
		}
		else if (const auto pInf = abstract_cast<InfantryClass*>(LastObjectList[i]))
		{
			if ((!this->Type->AllowPick_InfantryTypes.empty() && !this->Type->AllowPick_InfantryTypes.Contains(pInf->GetTechnoType())) ||
				(!this->Type->DisallowPick_InfantryTypes.empty() && this->Type->DisallowPick_InfantryTypes.Contains(pInf->GetTechnoType())))
				continue;

			OpenCrate(pInf);
			break;
		}
	}
}

void CrateClass::OpenCrate(TechnoClass* pTechno)
{
	if (pTechno == nullptr || this->IsInvalid || !TechnoExt::IsReallyAlive(pTechno))
		return;

	if (!this->CrateEffects.empty())
	{
		double dice = ScenarioClass::Instance->Random.RandomDouble();
		int idx = GeneralUtils::ChooseOneWeighted(dice, &this->CrateEffects_Weights);

		switch (this->CrateEffects[idx])
		{
		case CrateEffect::Weapon:
			DetonateWeapons(pTechno);
			break;
		case CrateEffect::AttachEffect:
			AttachEffects(pTechno);
			break;
		case CrateEffect::Money:
			TransactMoney(pTechno);
			break;
		case CrateEffect::SpySat:
			RevealSight(pTechno);
			break;
		case CrateEffect::Darkness:
			GapRadar(pTechno);
			break;
		case CrateEffect::SuperWeapon:
			GrantSuperWeapons(pTechno);
			break;
		case CrateEffect::RadarJam:
			RadarBlackout(pTechno);
			break;
		case CrateEffect::PowerOutage:
			PowerBlackout(pTechno);
			break;
		case CrateEffect::Unit:
			CreateUnits(pTechno);
			break;
		default:
			break;
		}
	}

	this->IsInvalid = true;
}

void CrateClass::DetonateWeapons(TechnoClass* pTechno)
{
	if (this->Type->Weapon_RandomPick)
	{
		int idx = ScenarioClass::Instance->Random.RandomRanged(0, this->Type->Weapons.size() - 1);
		WeaponTypeExt::DetonateAt(this->Type->Weapons[idx], pTechno, PhobosGlobal::Global()->GetGenericStand(), this->OwnerHouse);

		const CoordStruct& location = this->Location->ContainsBridge() ? this->Location->GetCoordsWithBridge() : this->Location->GetCoords();

		if (this->Type->Weapons_Anims[idx])
			GameCreate<AnimClass>(this->Type->Weapons_Anims[idx], location);

		if (this->Type->Weapons_Sounds[idx])
			VocClass::PlayAt(this->Type->Weapons_Sounds[idx], location, nullptr);

		if (this->Type->Weapons_Messages[idx] && HouseClass::CurrentPlayer == pTechno->Owner)
			MessageListClass::Instance->PrintMessage(this->Type->Weapons_Messages[idx].Text, RulesClass::Instance->MessageDelay, HouseClass::CurrentPlayer->ColorSchemeIndex);
	}
	else
	{
		for (size_t idx = 0; idx < this->Type->Weapons.size(); idx++)
		{
			double dice = ScenarioClass::Instance->Random.RandomDouble();
			if (this->Type->Weapons_Chances[idx] < dice)
				continue;

			const CoordStruct& location = this->Location->ContainsBridge() ? this->Location->GetCoordsWithBridge() : this->Location->GetCoords();
			if (TechnoExt::IsReallyAlive(pTechno))
				WeaponTypeExt::DetonateAt(this->Type->Weapons[idx], pTechno, PhobosGlobal::Global()->GetGenericStand(), this->OwnerHouse);
			else
				WeaponTypeExt::DetonateAt(this->Type->Weapons[idx], location, PhobosGlobal::Global()->GetGenericStand(), this->OwnerHouse);

			if (this->Type->Weapons_Anims[idx])
				GameCreate<AnimClass>(this->Type->Weapons_Anims[idx], location);

			if (this->Type->Weapons_Sounds[idx])
				VocClass::PlayAt(this->Type->Weapons_Sounds[idx], location, nullptr);

			if (this->Type->Weapons_Messages[idx] && HouseClass::CurrentPlayer == pTechno->Owner)
				MessageListClass::Instance->PrintMessage(this->Type->Weapons_Messages[idx].Text, RulesClass::Instance->MessageDelay, HouseClass::CurrentPlayer->ColorSchemeIndex);
		}
	}
}

void CrateClass::AttachEffects(TechnoClass* pTechno)
{
	if (this->Type->AttachEffect_RandomPick)
	{
		int idx = ScenarioClass::Instance->Random.RandomRanged(0, this->Type->AttachEffects.size() - 1);
		TechnoExt::AttachEffect(pTechno, nullptr, this->Type->AttachEffects[idx]);

		const CoordStruct& location = this->Location->ContainsBridge() ? this->Location->GetCoordsWithBridge() : this->Location->GetCoords();

		if (this->Type->AttachEffects[idx])
			GameCreate<AnimClass>(this->Type->AttachEffects_Anims[idx], location);

		if (this->Type->AttachEffects_Sounds[idx])
			VocClass::PlayAt(this->Type->AttachEffects_Sounds[idx], location, nullptr);

		if (this->Type->AttachEffects_Messages[idx] && HouseClass::CurrentPlayer == pTechno->Owner)
			MessageListClass::Instance->PrintMessage(this->Type->AttachEffects_Messages[idx].Text, RulesClass::Instance->MessageDelay, HouseClass::CurrentPlayer->ColorSchemeIndex);
	}
	else
	{
		for (size_t idx = 0; idx < this->Type->AttachEffects.size(); idx++)
		{
			double dice = ScenarioClass::Instance->Random.RandomDouble();
			if (this->Type->AttachEffects_Chances[idx] < dice)
				continue;

			const CoordStruct& location = this->Location->ContainsBridge() ? this->Location->GetCoordsWithBridge() : this->Location->GetCoords();

			TechnoExt::AttachEffect(pTechno, nullptr, this->Type->AttachEffects[idx]);

			if (this->Type->AttachEffects_Anims[idx])
				GameCreate<AnimClass>(this->Type->AttachEffects_Anims[idx], location);

			if (this->Type->AttachEffects_Sounds[idx])
				VocClass::PlayAt(this->Type->AttachEffects_Sounds[idx], location, nullptr);

			if (this->Type->AttachEffects_Messages[idx] && HouseClass::CurrentPlayer == pTechno->Owner)
				MessageListClass::Instance->PrintMessage(this->Type->AttachEffects_Messages[idx].Text, RulesClass::Instance->MessageDelay, HouseClass::CurrentPlayer->ColorSchemeIndex);
		}
	}
}

void CrateClass::TransactMoney(TechnoClass* pTechno)
{
	if (this->Type->Money.Get().Y && this->Type->Money.Get().Y > this->Type->Money.Get().X)
	{
		int money = ScenarioClass::Instance->Random.RandomRanged(this->Type->Money.Get().X, this->Type->Money.Get().Y);
		pTechno->Owner->TransactMoney(money);
		if (this->Type->Money_Display && money != 0)
			FlyingStrings::AddMoneyString(money, pTechno->Owner, this->Type->Money_DisplayHouses, pTechno->Location, this->Type->Money_DisplayOffset);
	}
	else
	{
		pTechno->Owner->TransactMoney(this->Type->Money.Get().X);
		if (this->Type->Money_Display)
			FlyingStrings::AddMoneyString(this->Type->Money.Get().X, pTechno->Owner, this->Type->Money_DisplayHouses, pTechno->Location, this->Type->Money_DisplayOffset);
	}

	const CoordStruct& location = this->Location->ContainsBridge() ? this->Location->GetCoordsWithBridge() : this->Location->GetCoords();

	if (this->Type->Money_Anim)
		GameCreate<AnimClass>(this->Type->Money_Anim, location);

	if (this->Type->Money_Sound.Get())
		VocClass::PlayAt(this->Type->Money_Sound, location, nullptr);

	if (this->Type->Money_Message.Get() && HouseClass::CurrentPlayer == pTechno->Owner)
		MessageListClass::Instance->PrintMessage(this->Type->Money_Message.Get().Text, RulesClass::Instance->MessageDelay, HouseClass::CurrentPlayer->ColorSchemeIndex);
}

void CrateClass::RevealSight(TechnoClass* pTechno)
{
	if (this->Type->SpySat_Range.Get().X < 0)
		MapClass::Instance->Reveal(pTechno->Owner);
	else if (this->Type->SpySat_Range.Get().Y && this->Type->SpySat_Range.Get().Y > this->Type->SpySat_Range.Get().X)
	{
		int range = ScenarioClass::Instance->Random.RandomRanged(this->Type->SpySat_Range.Get().X, this->Type->SpySat_Range.Get().Y);
		MapClass::Instance->RevealArea1(&pTechno->Location, range, pTechno->Owner, CellStruct::Empty, 0, 0, 0, 1);
	}
	else
		MapClass::Instance->RevealArea1(&pTechno->Location, this->Type->SpySat_Range.Get().X, pTechno->Owner, CellStruct::Empty, 0, 0, 0, 1);

	const CoordStruct& location = this->Location->ContainsBridge() ? this->Location->GetCoordsWithBridge() : this->Location->GetCoords();

	if (this->Type->SpySat_Anim)
		GameCreate<AnimClass>(this->Type->SpySat_Anim, location);

	if (this->Type->SpySat_Sound.Get())
		VocClass::PlayAt(this->Type->SpySat_Sound, location, nullptr);

	if (this->Type->SpySat_Message.Get() && HouseClass::CurrentPlayer == pTechno->Owner)
		MessageListClass::Instance->PrintMessage(this->Type->SpySat_Message.Get().Text, RulesClass::Instance->MessageDelay, HouseClass::CurrentPlayer->ColorSchemeIndex);
}

void CrateClass::GapRadar(TechnoClass* pTechno)
{
	if (pTechno->Owner->IsControlledByHuman() && !pTechno->Owner->IsObserver() && !pTechno->Owner->Defeated)
		pTechno->Owner->ReshroudMap();

	const CoordStruct& location = this->Location->ContainsBridge() ? this->Location->GetCoordsWithBridge() : this->Location->GetCoords();

	if (this->Type->Darkness_Anim)
		GameCreate<AnimClass>(this->Type->Darkness_Anim, location);

	if (this->Type->Darkness_Sound.Get())
		VocClass::PlayAt(this->Type->Darkness_Sound, location, nullptr);

	if (this->Type->Darkness_Message.Get() && HouseClass::CurrentPlayer == pTechno->Owner)
		MessageListClass::Instance->PrintMessage(this->Type->Darkness_Message.Get().Text, RulesClass::Instance->MessageDelay, HouseClass::CurrentPlayer->ColorSchemeIndex);
}

void CrateClass::GrantSuperWeapons(TechnoClass* pTechno)
{
	if (this->Type->SuperWeapon_RandomPick)
	{
		int idx = ScenarioClass::Instance->Random.RandomRanged(0, this->Type->SuperWeapons.size() - 1);

		SuperClass* pSuper = pTechno->Owner->Supers[this->Type->SuperWeapons[idx]];
		if (this->Type->SuperWeapons_Permanents[idx])
		{
			HouseExt::ExtData* pHouseExt = HouseExt::ExtMap.Find(pTechno->Owner);
			pHouseExt->SW_Permanents[pSuper->Type->ArrayIndex] = true;
		}

		if (pSuper->Granted)
			pSuper->RechargeTimer.TimeLeft -= pSuper->RechargeTimer.TimeLeft;
		else
		{
			bool granted;
			granted = pSuper->Grant(true, true, false);
			if (granted)
			{
				if (MouseClass::Instance->AddCameo(AbstractType::Special, this->Type->SuperWeapons[idx]))
					MouseClass::Instance->RepaintSidebar(1);
			}
		}

		const CoordStruct& location = this->Location->ContainsBridge() ? this->Location->GetCoordsWithBridge() : this->Location->GetCoords();

		if (this->Type->SuperWeapons[idx])
			GameCreate<AnimClass>(this->Type->SuperWeapons_Anims[idx], location);

		if (this->Type->SuperWeapons_Sounds[idx])
			VocClass::PlayAt(this->Type->SuperWeapons_Sounds[idx], location, nullptr);

		if (this->Type->SuperWeapons_Messages[idx] && HouseClass::CurrentPlayer == pTechno->Owner)
			MessageListClass::Instance->PrintMessage(this->Type->SuperWeapons_Messages[idx].Text, RulesClass::Instance->MessageDelay, HouseClass::CurrentPlayer->ColorSchemeIndex);
	}
	else
	{
		for (size_t idx = 0; idx < this->Type->SuperWeapons.size(); idx++)
		{
			double dice = ScenarioClass::Instance->Random.RandomDouble();
			if (this->Type->SuperWeapons_Chances[idx] < dice)
				continue;

			const CoordStruct& location = this->Location->ContainsBridge() ? this->Location->GetCoordsWithBridge() : this->Location->GetCoords();

			SuperClass* pSuper = pTechno->Owner->Supers[this->Type->SuperWeapons[idx]];
			if (this->Type->SuperWeapons_Permanents[idx])
			{
				HouseExt::ExtData* pHouseExt = HouseExt::ExtMap.Find(pTechno->Owner);
				pHouseExt->SW_Permanents[pSuper->Type->ArrayIndex] = true;
			}

			if (pSuper->Granted)
				pSuper->RechargeTimer.TimeLeft -= pSuper->RechargeTimer.TimeLeft;
			else
			{
				bool granted;
				granted = pSuper->Grant(true, true, false);
				if (granted)
				{
					if (MouseClass::Instance->AddCameo(AbstractType::Special, this->Type->SuperWeapons[idx]))
						MouseClass::Instance->RepaintSidebar(1);
				}
			}

			if (this->Type->SuperWeapons_Anims[idx])
				GameCreate<AnimClass>(this->Type->SuperWeapons_Anims[idx], location);

			if (this->Type->SuperWeapons_Sounds[idx])
				VocClass::PlayAt(this->Type->SuperWeapons_Sounds[idx], location, nullptr);

			if (this->Type->SuperWeapons_Messages[idx] && HouseClass::CurrentPlayer == pTechno->Owner)
				MessageListClass::Instance->PrintMessage(this->Type->SuperWeapons_Messages[idx].Text, RulesClass::Instance->MessageDelay, HouseClass::CurrentPlayer->ColorSchemeIndex);
		}
	}
}

void CrateClass::RadarBlackout(TechnoClass* pTechno)
{
	if (this->Type->RadarJam_Duration.Get().Y && this->Type->RadarJam_Duration.Get().Y > this->Type->RadarJam_Duration.Get().X)
	{
		int duration = ScenarioClass::Instance->Random.RandomRanged(this->Type->RadarJam_Duration.Get().X, this->Type->RadarJam_Duration.Get().Y);
		pTechno->Owner->RadarBlackoutTimer.Start(duration);
	}
	else
		pTechno->Owner->RadarBlackoutTimer.Start(this->Type->RadarJam_Duration.Get().X);

	pTechno->Owner->RecheckRadar = true;

	const CoordStruct& location = this->Location->ContainsBridge() ? this->Location->GetCoordsWithBridge() : this->Location->GetCoords();

	if (this->Type->RadarJam_Anim)
		GameCreate<AnimClass>(this->Type->RadarJam_Anim, location);

	if (this->Type->RadarJam_Sound.Get())
		VocClass::PlayAt(this->Type->RadarJam_Sound, location, nullptr);

	if (this->Type->RadarJam_Message.Get() && HouseClass::CurrentPlayer == pTechno->Owner)
		MessageListClass::Instance->PrintMessage(this->Type->RadarJam_Message.Get().Text, RulesClass::Instance->MessageDelay, HouseClass::CurrentPlayer->ColorSchemeIndex);
}

void CrateClass::PowerBlackout(TechnoClass* pTechno)
{
	if (this->Type->PowerOutage_Duration.Get().Y && this->Type->PowerOutage_Duration.Get().Y > this->Type->PowerOutage_Duration.Get().X)
	{
		int duration = ScenarioClass::Instance->Random.RandomRanged(this->Type->PowerOutage_Duration.Get().X, this->Type->PowerOutage_Duration.Get().Y);
		pTechno->Owner->PowerBlackoutTimer.Start(duration);
	}
	else
		pTechno->Owner->PowerBlackoutTimer.Start(this->Type->PowerOutage_Duration.Get().X);

	pTechno->Owner->RecheckPower = true;

	const CoordStruct& location = this->Location->ContainsBridge() ? this->Location->GetCoordsWithBridge() : this->Location->GetCoords();

	if (this->Type->PowerOutage_Anim)
		GameCreate<AnimClass>(this->Type->PowerOutage_Anim, location);

	if (this->Type->PowerOutage_Sound.Get())
		VocClass::PlayAt(this->Type->PowerOutage_Sound, location, nullptr);

	if (this->Type->PowerOutage_Message.Get() && HouseClass::CurrentPlayer == pTechno->Owner)
		MessageListClass::Instance->PrintMessage(this->Type->PowerOutage_Message.Get().Text, RulesClass::Instance->MessageDelay, HouseClass::CurrentPlayer->ColorSchemeIndex);
}

void CrateClass::CreateUnits(TechnoClass* pTechno)
{
	if (this->Type->Unit_RandomPick)
	{
		int idx = ScenarioClass::Instance->Random.RandomRanged(0, this->Type->Units.size() - 1);

		int num = 0;
		if (this->Type->Units_Nums[idx].X <= 0)
			return;

		if (this->Type->Units_Nums[idx].Y && this->Type->Units_Nums[idx].Y > this->Type->Units_Nums[idx].X)
			num = ScenarioClass::Instance->Random.RandomRanged(this->Type->Units_Nums[idx].X, this->Type->Units_Nums[idx].Y);
		else
			num = this->Type->Units_Nums[idx].X;

		const auto pOwner = pTechno->Owner;
		bool succeed = false;
		const CoordStruct& location = this->Location->ContainsBridge() ? this->Location->GetCoordsWithBridge() : this->Location->GetCoords();
		for (int i = 0; i < num; ++i)
		{
			if (const auto pUnit = abstract_cast<TechnoClass*>(this->Type->Units[idx]->CreateObject(pOwner)))
			{
				if (!this->Location)
				{
					pUnit->UnInit();
					continue;
				}

				pUnit->OnBridge = this->Location->ContainsBridge();

				if (const auto pFoot = abstract_cast<FootClass*>(pUnit))
				{
					this->Location->ScatterContent(CoordStruct::Empty, true, true, pUnit->OnBridge);

					DirType nRandFacing = static_cast<DirType>(ScenarioClass::Instance->Random.RandomRanged(0, 255));
					++Unsorted::IKnowWhatImDoing();
					succeed = pFoot->Unlimbo(location, nRandFacing);
					--Unsorted::IKnowWhatImDoing();

					pFoot->SlaveOwner = nullptr;
					pFoot->Transporter = nullptr;
					pFoot->Absorbed = false;
					pFoot->LastMapCoords = this->Location->MapCoords;
					pFoot->QueueMission(Mission::Move, false);
					pFoot->ShouldEnterOccupiable = false;
					pFoot->ShouldGarrisonStructure = false;
					if (auto const pJJLoco = locomotion_cast<JumpjetLocomotionClass*>(pFoot->Locomotor))
					{
						pJJLoco->LocomotionFacing.SetCurrent(DirStruct(static_cast<DirType>(nRandFacing)));

						if (this->Type->Units[idx]->BalloonHover)
						{
							pJJLoco->State = JumpjetLocomotionClass::State::Hovering;
							pJJLoco->IsMoving = true;
							pJJLoco->DestinationCoords = location;
							pJJLoco->CurrentHeight = this->Type->Units[idx]->JumpjetHeight;
						}
						else
							pJJLoco->Move_To(location);
					}
					else
						pFoot->Scatter(pFoot->Location, true, false);

					TechnoExt::FallenDown(pFoot);
				}
				else
				{
					++Unsorted::IKnowWhatImDoing();
					succeed = pUnit->Unlimbo(location, DirType::East);
					--Unsorted::IKnowWhatImDoing();
					pUnit->Location = location;
				}

				if (succeed)
					pUnit->DiscoveredBy(pOwner);
				else
					pUnit->UnInit();
			}
		}

		if (this->Type->Units_Anims[idx])
			GameCreate<AnimClass>(this->Type->Units_Anims[idx], location);

		if (this->Type->Units_Sounds[idx])
			VocClass::PlayAt(this->Type->Units_Sounds[idx], location, nullptr);

		if (this->Type->Units_Messages[idx] && HouseClass::CurrentPlayer == pTechno->Owner)
			MessageListClass::Instance->PrintMessage(this->Type->Units_Messages[idx].Text, RulesClass::Instance->MessageDelay, HouseClass::CurrentPlayer->ColorSchemeIndex);
	}
	else
	{
		for (size_t idx = 0; idx < this->Type->Units.size(); idx++)
		{
			double dice = ScenarioClass::Instance->Random.RandomDouble();
			if (this->Type->Units_Chances[idx] < dice)
				continue;

			int num = 0;
			if (this->Type->Units_Nums[idx].X <= 0)
				continue;

			if (this->Type->Units_Nums[idx].Y && this->Type->Units_Nums[idx].Y > this->Type->Units_Nums[idx].X)
				num = ScenarioClass::Instance->Random.RandomRanged(this->Type->Units_Nums[idx].X, this->Type->Units_Nums[idx].Y);
			else
				num = this->Type->Units_Nums[idx].X;

			const auto pOwner = pTechno->Owner;
			bool succeed = false;
			const CoordStruct& location = this->Location->ContainsBridge() ? this->Location->GetCoordsWithBridge() : this->Location->GetCoords();
			for (int i = 0; i < num; ++i)
			{
				if (const auto pUnit = abstract_cast<TechnoClass*>(this->Type->Units[idx]->CreateObject(pOwner)))
				{
					if (!this->Location)
					{
						pUnit->UnInit();
						continue;
					}

					pUnit->OnBridge = this->Location->ContainsBridge();

					if (const auto pFoot = abstract_cast<FootClass*>(pUnit))
					{
						this->Location->ScatterContent(CoordStruct::Empty, true, true, pUnit->OnBridge);

						DirType nRandFacing = static_cast<DirType>(ScenarioClass::Instance->Random.RandomRanged(0, 255));
						++Unsorted::IKnowWhatImDoing();
						succeed = pFoot->Unlimbo(location, nRandFacing);
						--Unsorted::IKnowWhatImDoing();

						pFoot->SlaveOwner = nullptr;
						pFoot->Transporter = nullptr;
						pFoot->Absorbed = false;
						pFoot->LastMapCoords = this->Location->MapCoords;
						pFoot->QueueMission(Mission::Move, false);
						pFoot->ShouldEnterOccupiable = false;
						pFoot->ShouldGarrisonStructure = false;
						pFoot->Scatter(pFoot->Location, true, false);
						TechnoExt::FallenDown(pFoot);
					}
					else
					{
						++Unsorted::IKnowWhatImDoing();
						succeed = pUnit->Unlimbo(location, DirType::East);
						--Unsorted::IKnowWhatImDoing();
						pUnit->Location = location;
					}

					if (succeed)
						pUnit->DiscoveredBy(pOwner);
					else
						pUnit->UnInit();
				}
			}

			if (this->Type->Units_Anims[idx])
				GameCreate<AnimClass>(this->Type->Units_Anims[idx], location);

			if (this->Type->Units_Sounds[idx])
				VocClass::PlayAt(this->Type->Units_Sounds[idx], location, nullptr);

			if (this->Type->Units_Messages[idx] && HouseClass::CurrentPlayer == pTechno->Owner)
				MessageListClass::Instance->PrintMessage(this->Type->Units_Messages[idx].Text, RulesClass::Instance->MessageDelay, HouseClass::CurrentPlayer->ColorSchemeIndex);
		}
	}
}

bool CrateClass::IsActive() const
{
	const bool active =
		this != nullptr
		&& !this->IsInvalid
		&& !this->Timer.Completed()
		;

	return active;
}

void CrateClass::CreateCrate(CrateTypeClass* pType, CellClass* pCell, HouseClass* pHouse)
{
	if (!CanExist(pType) || !CanSpwan(pType, pCell))
		return;

	auto crate = std::make_unique<CrateClass>(pType, pCell, pType->Duration, pHouse);
	PhobosGlobal::Global()->Crate_Cells.emplace_back(pCell);
	PhobosGlobal::Global()->Crate_List.emplace_back(std::move(crate));
}

template <typename T>
bool CrateClass::Serialize(T& stm)
{
	stm
		.Process(this->Type)
		.Process(this->Duration)
		.Process(this->Timer)
		.Process(this->Location)
		.Process(this->Initialized)
		.Process(this->Anim)
		.Process(this->IsInvalid)
		.Process(this->CrateEffects)
		.Process(this->CrateEffects_Weights)
		.Process(this->OwnerHouse)
		;

	return stm.Success();
}

bool CrateClass::Load(PhobosStreamReader& stm, bool registerForChange)
{
	return Serialize(stm);
}

bool CrateClass::Save(PhobosStreamWriter& stm) const
{
	return const_cast<CrateClass*>(this)->Serialize(stm);
}

void CrateClass::Clear()
{
	Crate_Exist.clear();
}

bool CrateClass::LoadGlobals(PhobosStreamReader& stm)
{
	return stm
		.Process(Crate_Exist)
		.Success();
}

bool CrateClass::SaveGlobals(PhobosStreamWriter& stm)
{
	return stm
		.Process(Crate_Exist)
		.Success();
}
