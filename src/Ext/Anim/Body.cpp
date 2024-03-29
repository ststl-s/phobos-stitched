#include "Body.h"

#include <ParticleSystemClass.h>

#include <Ext/AnimType/Body.h>
#include <Ext/House/Body.h>

#include <Misc/SyncLogging.h>

#include <Utilities/Macro.h>
#include <Utilities/TemplateDef.h>

AnimExt::ExtContainer AnimExt::ExtMap;

void AnimExt::ExtData::SetInvoker(TechnoClass* pInvoker)
{
	this->Invoker = pInvoker;
	this->InvokerHouse = pInvoker ? pInvoker->Owner : nullptr;
}

void AnimExt::ExtData::SetInvoker(TechnoClass* pInvoker, HouseClass* pInvokerHouse)
{
	this->Invoker = pInvoker;
	this->InvokerHouse = pInvokerHouse;
}

void AnimExt::ExtData::CreateAttachedSystem()
{
	const auto pThis = this->OwnerObject();
	const auto pTypeExt = AnimTypeExt::ExtMap.Find(pThis->Type);

	if (pTypeExt && pTypeExt->AttachedSystem && !this->AttachedSystem)
	{
		if (auto const pSystem = GameCreate<ParticleSystemClass>(pTypeExt->AttachedSystem.Get(), pThis->Location, pThis->GetCell(), pThis, CoordStruct::Empty, nullptr))
			this->AttachedSystem = pSystem;
	}
}

void AnimExt::ExtData::DeleteAttachedSystem()
{
	if (this->AttachedSystem)
	{
		this->AttachedSystem->Owner = nullptr;
		this->AttachedSystem->UnInit();
		this->AttachedSystem = nullptr;
	}
}

//Modified from Ares
bool AnimExt::SetAnimOwnerHouseKind(AnimClass* pAnim, HouseClass* pInvoker, HouseClass* pVictim, bool defaultToVictimOwner, bool defaultToInvokerOwner)
{
	auto const pTypeExt = AnimTypeExt::ExtMap.Find(pAnim->Type);
	bool makeInf = pAnim->Type->MakeInfantry > -1;
	bool createUnit = pTypeExt->CreateUnit.Get();
	auto ownerKind = OwnerHouseKind::Default;
	HouseClass* pDefaultOwner = nullptr;

	if (defaultToVictimOwner)
		pDefaultOwner = pVictim;
	else if (defaultToInvokerOwner)
		pDefaultOwner = pInvoker;

	if (makeInf)
		ownerKind = pTypeExt->MakeInfantryOwner;

	if (createUnit)
		ownerKind = pTypeExt->CreateUnit_Owner;

	auto newOwner = HouseExt::GetHouseKind(ownerKind, true, pDefaultOwner, pInvoker, pVictim);

	if (newOwner)
	{
		pAnim->Owner = newOwner;
		bool isRemappable = false;

		if (makeInf)
			isRemappable = true;

		if (createUnit)
			isRemappable = pTypeExt->CreateUnit_RemapAnim;

		if (isRemappable && !newOwner->Defeated)
			pAnim->LightConvert = ColorScheme::Array->Items[newOwner->ColorSchemeIndex]->LightConvert;
	}

	return newOwner;
}

HouseClass* AnimExt::GetOwnerHouse(AnimClass* pAnim, HouseClass* pDefaultOwner)
{
	if (!pAnim)
		return pDefaultOwner;

	HouseClass* pTechnoOwner = nullptr;

	if (auto const pTechno = abstract_cast<TechnoClass*>(pAnim->OwnerObject))
		pTechnoOwner = pTechno->Owner;

	if (pAnim->Owner)
		return pAnim->Owner;
	else
		return  pTechnoOwner ? pTechnoOwner : pDefaultOwner;
}

void AnimExt::ExtData::InitializeConstants()
{
	CreateAttachedSystem();
}

// =============================
// load / save

template <typename T>
void AnimExt::ExtData::Serialize(T& Stm)
{
	Stm
		.Process(this->DeathUnitFacing)
		.Process(this->FromDeathUnit)
		.Process(this->DeathUnitTurretFacing)
		.Process(this->DeathUnitHasTurret)
		.Process(this->IsAttachEffectAnim)
		.Process(this->Invoker)
		.Process(this->InvokerHouse)
		.Process(this->AttachedSystem)
		.Process(this->ParentBuilding)
		;
}

void AnimExt::ExtData::LoadFromStream(PhobosStreamReader& Stm)
{
	Extension<AnimClass>::LoadFromStream(Stm);
	this->Serialize(Stm);
}

void AnimExt::ExtData::SaveToStream(PhobosStreamWriter& Stm)
{
	Extension<AnimClass>::SaveToStream(Stm);
	this->Serialize(Stm);
}

// =============================
// container

AnimExt::ExtContainer::ExtContainer() : Container("AnimClass") { }
AnimExt::ExtContainer::~ExtContainer() = default;

// =============================
// container hooks

namespace CTORTemp
{
	CoordStruct coords;
	unsigned int callerAddress;
}

DEFINE_HOOK(0x421EA0, AnimClass_CTOR_SetContext, 0x6)
{
	GET_STACK(CoordStruct*, coords, 0x8);
	GET_STACK(unsigned int, callerAddress, 0x0);

	CTORTemp::coords = *coords;
	CTORTemp::callerAddress = callerAddress;

	return 0;
}

DEFINE_HOOK_AGAIN(0x422126, AnimClass_CTOR, 0x5)
DEFINE_HOOK_AGAIN(0x422707, AnimClass_CTOR, 0x5)
DEFINE_HOOK(0x4228D2, AnimClass_CTOR, 0x5)
{
	if (!Phobos::IsLoadingSaveGame)
	{
		GET(AnimClass*, pItem, ESI);

		auto const callerAddress = CTORTemp::callerAddress;

		// Do this here instead of using a duplicate hook in SyncLogger.cpp
		if (!SyncLogger::HooksDisabled && pItem->UniqueID != -2)
			SyncLogger::AddAnimCreationSyncLogEvent(CTORTemp::coords, callerAddress);

		if (pItem && !pItem->Type)
		{
			Debug::Log("Attempting to create animation with null Type (Caller: %08x)!\n", callerAddress);
			return 0;
		}

		AnimExt::ExtMap.Allocate(pItem);
	}

	return 0;
}

DEFINE_HOOK(0x422967, AnimClass_DTOR, 0x6)
{
	GET(AnimClass*, pItem, ESI);

	AnimExt::ExtMap.Remove(pItem);

	return 0;
}

/*Crash when Anim called with GameDelete()
DEFINE_HOOK(0x426598, AnimClass_SDDTOR, 0x7)
{
	GET(AnimClass*, pItem, ESI);

	if(AnimExt::ExtMap.Find(pItem))
	AnimExt::ExtMap.Remove(pItem);

	return 0;
}
*/

DEFINE_HOOK_AGAIN(0x425280, AnimClass_SaveLoad_Prefix, 0x5)
DEFINE_HOOK(0x4253B0, AnimClass_SaveLoad_Prefix, 0x5)
{
	GET_STACK(AnimClass*, pItem, 0x4);
	GET_STACK(IStream*, pStm, 0x8);

	AnimExt::ExtMap.PrepareStream(pItem, pStm);

	return 0;
}

DEFINE_HOOK_AGAIN(0x425391, AnimClass_Load_Suffix, 0x7)
DEFINE_HOOK_AGAIN(0x4253A2, AnimClass_Load_Suffix, 0x7)
DEFINE_HOOK(0x425358, AnimClass_Load_Suffix, 0x7)
{
	AnimExt::ExtMap.LoadStatic();
	return 0;
}

DEFINE_HOOK(0x4253FF, AnimClass_Save_Suffix, 0x5)
{
	AnimExt::ExtMap.SaveStatic();
	return 0;
}

// Field D0 in AnimClass is mostly unused so by removing the few uses it has it can be used to store AnimExt pointer.
DEFINE_JUMP(LJMP, 0x42543A, 0x425448)

DEFINE_HOOK_AGAIN(0x421EF4, AnimClass_CTOR_ClearD0, 0x6)
DEFINE_HOOK(0x42276D, AnimClass_CTOR_ClearD0, 0x6)
{
	GET(AnimClass*, pThis, ESI);
	pThis->unknown_D0 = 0;
	return R->Origin() + 0x6;
}

