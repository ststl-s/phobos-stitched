#include <Ext/Techno/Body.h>

DEFINE_HOOK(0x6FD1F1, TechnoClass_GetROF, 0x5)
{
	GET(int, iROF, EBP);
	GET(TechnoClass*, pThis, ESI);

	TechnoExt::ExtData* pExt = TechnoExt::ExtMap.Find(pThis);

	for (auto& pAE: pExt->AttachEffects)
	{
		iROF *= pAE->Type->ROF_Multiplier;
		iROF = std::max(iROF, 1);
	}

	for (auto& pAE : pExt->AttachEffects)
	{
		iROF += pAE->Type->ROF;
		iROF = std::max(iROF, 1);
	}

	R->EBP(iROF);

	return 0;
}
