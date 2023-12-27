#include "Body.h"

#include <TechnoClass.h>
#include <Ext/Techno/Body.h>

void ExtraPhobosNetEvent::Handlers::RaiseConvert(TechnoClass* pTechno)
{
	pTechno->ClickedEvent(static_cast<NetworkEvents>(ExtraPhobosNetEvent::Events::Convert));
}

void ExtraPhobosNetEvent::Handlers::RespondToConvert(EventClass* pEvent)
{
	auto pTarget = &pEvent->Data.Target.Whom;

	if (auto pTechno = pTarget->As_Techno())
	{
		if (const auto pExt = TechnoExt::ExtMap.Find(pTechno))
		{
			pExt->ConvertCommand();
		}
	}
}
