#include <TechnoClass.h>
#include <Ext/TechnoType/Body.h>

DEFINE_HOOK(0x6B6D49, SpawnManagerClass_CTOR_InitializedSpawns, 0xA)
{
	GET(SpawnManagerClass*, pThis, ESI);
	GET_STACK(int, nodeIdx, STACK_OFFS(0x1C, -0x4));

	TechnoClass* pTechno = pThis->Owner;
	TechnoTypeClass* pTechnoType = pTechno->GetTechnoType();
	HouseClass* pHouse = pTechno->Owner;
	auto pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pTechnoType);
	int typeIdx = 0;

	if (!pTechnoTypeExt->Spawn_Types.HasValue())
	{
		R->EAX(pTechnoType->Spawns->CreateObject(pHouse));

		return 0x6B6D53;
	}

	for (int nums = 0; nums <= nodeIdx; typeIdx++)
	{
		nums += pTechnoTypeExt->Spawn_Nums[typeIdx];
	}

	AircraftTypeClass* pSpawnType = abstract_cast<AircraftTypeClass*>(pTechnoTypeExt->Spawn_Types[--typeIdx]);
	R->EAX(pSpawnType->CreateObject(pHouse));
	pThis->SpawnType = pSpawnType;

	return 0x6B6D53;
}

DEFINE_HOOK(0x6B7265, SpawnManagerClass_AI_UpdateTimer, 0x6)
{
	GET(SpawnManagerClass* const, pThis, ESI);

	if (pThis->Owner && pThis->Status == SpawnManagerStatus::Launching && pThis->CountDockedSpawns() != 0)
	{
		R->EAX(0);
	}

	return 0;
}

DEFINE_HOOK(0x6B7282, SpawnManagerClass_AI_PromoteSpawns, 0x5)
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
	int nodeIdx;
	int typeIdx;
};

DEFINE_HOOK(0x6B7287, SpawnManagerClass_Update_Context, 0x6)
{
	GET(SpawnManagerClass*, pThis, ESI);

	SpawnUpdateContext::nodeIdx = R->EBX();
	int& typeIdx = SpawnUpdateContext::typeIdx;
	TechnoTypeExt::ExtData* pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Owner->GetTechnoType());

	if (!pTechnoTypeExt->Spawn_Types.HasValue())
	{
		typeIdx = -1;

		return 0x6B78EA;
	}

	for (int nums = 0; nums <= SpawnUpdateContext::nodeIdx; typeIdx++)
	{
		nums += pTechnoTypeExt->Spawn_Nums[typeIdx];
	}

	--typeIdx;

	return 0;
}

DEFINE_HOOK_AGAIN(0x6B73BE, SpawnManagerClass_AI_SpawnTimer, 0x6)
DEFINE_HOOK(0x6B73AD, SpawnManagerClass_AI_SpawnTimer, 0x5)
{
	GET(SpawnManagerClass* const, pThis, ESI);

	int nodeIdx = SpawnUpdateContext::nodeIdx;

	if (pThis->Owner)
	{
		if (auto const pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Owner->GetTechnoType()))
		{
			if (pTypeExt->Spawner_DelayFrams_PerSpawn.HasValue() && nodeIdx < static_cast<int>(pTypeExt->Spawner_DelayFrams_PerSpawn.size()))
			{
				R->ECX(pTypeExt->Spawner_DelayFrams_PerSpawn[nodeIdx]);
			}
			else if (pTypeExt->Spawner_DelayFrames.isset())
			{
				R->ECX(pTypeExt->Spawner_DelayFrames.Get());
			}
		}
	}

	return 0;
}

DEFINE_HOOK(0x6B78E0, SpawnManagerClass_Update_Regen, 0xA)
{
	GET(SpawnManagerClass*, pThis, ESI);

	TechnoClass* pTechno = pThis->Owner;
	TechnoTypeClass* pTechnoType = pTechno->GetTechnoType();
	HouseClass* pHouse = pTechno->Owner;
	auto pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pTechnoType);
	int typeIdx = SpawnUpdateContext::typeIdx;

	if (typeIdx < 0)
	{
		R->EAX(pTechnoType->Spawns->CreateObject(pHouse));

		return 0x6B78EA;
	}

	R->EAX(pTechnoTypeExt->Spawn_Types[typeIdx]->CreateObject(pHouse));

	return 0x6B78EA;
}

DEFINE_HOOK(0x6B7D08, SpawnManagerClass_UnlinkPointer_RegenTimer, 0x6)
{
	GET(SpawnManagerClass*, pThis, ESI);
	GET(size_t, nodeIdx, EAX);

	auto pTechnoTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Owner->GetTechnoType());

	if (!pTechnoTypeExt->Spawn_Types.HasValue() || nodeIdx >= pTechnoTypeExt->Spawn_RegenRatePerSpawn.size())
	{
		return 0;
	}

	R->EDX(pTechnoTypeExt->Spawn_RegenRatePerSpawn[nodeIdx]);

	return 0;
}
