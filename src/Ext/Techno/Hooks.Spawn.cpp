#include <TechnoClass.h>
#include <Ext/TechnoType/Body.h>

DEFINE_HOOK(0x6B6D49, SpawnManagerClass_CTOR_InitializedSpawns, 0xA)
{
	GET(SpawnManagerClass*, pThis, ESI);
	GET_STACK(size_t, nodeIdx, STACK_OFFS(0x1C, -0x4));

	TechnoClass* pTechno = pThis->Owner;
	TechnoTypeClass* pTechnoType = pTechno->GetTechnoType();
	HouseClass* pHouse = pTechno->Owner;
	auto pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pTechnoType);
	size_t typeIdx = 0;

	if (!pTechnoTypeExt->Spawn_Types.HasValue())
	{
		R->EAX(pTechnoType->Spawns->CreateObject(pHouse));

		return 0x6B6D53;
	}

	for (size_t nums = 0; nums <= nodeIdx; typeIdx++)
	{
		nums += pTechnoTypeExt->Spawn_Nums[typeIdx];
	}

	--typeIdx;
	AircraftTypeClass* pSpawnType = pTechnoTypeExt->Spawn_Types[typeIdx];
	R->EAX(pSpawnType->CreateObject(pHouse));
	pThis->SpawnType = pSpawnType;

	return 0x6B6D53;
}

DEFINE_HOOK(0x6B7265, SpawnManagerClass_Update_UpdateTimer, 0x6)
{
	GET(SpawnManagerClass* const, pThis, ESI);

	if (pThis->Owner && pThis->Status == SpawnManagerStatus::Launching && pThis->CountDockedSpawns() != 0)
	{
		R->EAX(0);
	}

	return 0;
}

DEFINE_HOOK(0x6B7282, SpawnManagerClass_Update_PromoteSpawns, 0x5)
{
	GET(SpawnManagerClass*, pThis, ESI);

	auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Owner->GetTechnoType());
	if (pTypeExt->Promote_IncludeSpawns)
	{
		for (auto node : pThis->SpawnedNodes)
		{
			TechnoClass* pTechno = node->Techno;

			if (pTechno && pTechno->Veterancy.Veterancy < pThis->Owner->Veterancy.Veterancy)
				pTechno->Veterancy.Add(pThis->Owner->Veterancy.Veterancy - pTechno->Veterancy.Veterancy);
		}
	}

	return 0;
}

namespace SpawnUpdateContext
{
	SpawnManagerClass* pThis;
	TechnoClass* pOwner;
	const TechnoTypeClass* pOwnerType;
	const TechnoTypeExt::ExtData* pOwnerTypeExt;
	size_t nodeIdx;
	size_t typeIdx;
};

DEFINE_HOOK(0x6B7287, SpawnManagerClass_Update_Context, 0x6)
{
	GET(SpawnManagerClass*, pThis, ESI);

	size_t nodeIdx = SpawnUpdateContext::nodeIdx = R->EBX();
	size_t& typeIdx = SpawnUpdateContext::typeIdx = 0;
	SpawnUpdateContext::pThis = pThis;
	SpawnUpdateContext::pOwner = pThis->Owner;
	SpawnUpdateContext::pOwnerType = pThis->Owner->GetTechnoType();
	const auto pTechnoTypeExt = SpawnUpdateContext::pOwnerTypeExt = TechnoTypeExt::ExtMap.Find(SpawnUpdateContext::pOwnerType);

	if (!SpawnUpdateContext::pOwnerTypeExt->Spawn_Types.HasValue())
	{
		return 0;
	}

	for (size_t nums = 0; nums <= nodeIdx; typeIdx++)
	{
		nums += pTechnoTypeExt->Spawn_Nums[typeIdx];
	}

	--typeIdx;
	pThis->SpawnType = pTechnoTypeExt->Spawn_Types[typeIdx];

	return 0;
}

DEFINE_HOOK_AGAIN(0x6B73BE, SpawnManagerClass_Update_SpawnTimer, 0x6)
DEFINE_HOOK(0x6B73AD, SpawnManagerClass_Update_SpawnTimer, 0x5)
{
	const size_t nodeIdx = SpawnUpdateContext::nodeIdx;
	const auto pTechnoTypeExt = SpawnUpdateContext::pOwnerTypeExt;

	R->ECX
	(
		nodeIdx < pTechnoTypeExt->Spawner_DelayFrams_PerSpawn.size() ?
		pTechnoTypeExt->Spawner_DelayFrams_PerSpawn[nodeIdx] :
		pTechnoTypeExt->Spawner_DelayFrames.Get(10)
	);

	return 0;
}

DEFINE_HOOK(0x6B781E, SpawnManagerClass_Update_Reload, 0x6)
{
	SpawnManagerClass* pThis = SpawnUpdateContext::pThis;
	const TechnoTypeClass* pTechnoType = SpawnUpdateContext::pOwnerType;
	const auto pTechnoTypeExt = SpawnUpdateContext::pOwnerTypeExt;
	const size_t nodeIdx = SpawnUpdateContext::nodeIdx;

	pThis->ReloadRate = nodeIdx < pTechnoTypeExt->Spawn_ReloadRate.size() ?
		pTechnoTypeExt->Spawn_ReloadRate[nodeIdx] :
		pTechnoType->SpawnReloadRate;

	return 0;
}

DEFINE_HOOK(0x6B78E0, SpawnManagerClass_Update_Regen, 0xA)
{
	TechnoClass* pTechno = SpawnUpdateContext::pOwner;
	const TechnoTypeClass* pTechnoType = SpawnUpdateContext::pOwnerType;
	HouseClass* pHouse = pTechno->Owner;
	const auto pTechnoTypeExt = SpawnUpdateContext::pOwnerTypeExt;
	const size_t typeIdx = SpawnUpdateContext::typeIdx;

	R->EAX
	(
		typeIdx < pTechnoTypeExt->Spawn_Types.size() ?
		pTechnoTypeExt->Spawn_Types[typeIdx]->CreateObject(pHouse) :
		pTechnoType->Spawns->CreateObject(pHouse)
	);

	return 0x6B78EA;
}

DEFINE_HOOK(0x6B7D08, SpawnManagerClass_UnlinkPointer_RegenTimer, 0x6)
{
	GET(SpawnManagerClass*, pThis, ESI);
	GET(size_t, nodeIdx, EAX);

	const TechnoTypeClass* pTechnoType = pThis->Owner->GetTechnoType();
	const auto pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pTechnoType);

	pThis->RegenRate = nodeIdx < pTechnoTypeExt->Spawn_RegenRate.size() ?
		pTechnoTypeExt->Spawn_RegenRate[nodeIdx] :
		pTechnoType->SpawnRegenRate;

	return 0;
}
