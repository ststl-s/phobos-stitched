#include "Body.h"

#include <TechnoClass.h>
#include <Ext/Techno/Body.h>
#include <Ext/House/Body.h>

#include <Misc/GScreenCreate.h>

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

void ExtraPhobosNetEvent::Handlers::RaiseAutoRepair(HouseClass* pHouse)
{
	EventClass Event {};
	Event.Type = static_cast<NetworkEvents>(ExtraPhobosNetEvent::Events::AutoRepair);

	TargetClass house;
	house.m_ID = pHouse->ArrayIndex;

	SpecialClick1 Datas { house };
	memcpy(&Event.Data.nothing, &Datas, SpecialClick1::size());

	EventClass::AddEvent(Event);
}

void ExtraPhobosNetEvent::Handlers::RespondToAutoRepair(EventClass* pEvent)
{
	TargetClass* ID = reinterpret_cast<TargetClass*>(pEvent->Data.nothing.Data);
	if (auto pHouse = HouseClass::Array()->GetItem(ID->m_ID))
	{
		if (const auto pHouseExt = HouseExt::ExtMap.Find(pHouse))
		{
			pHouseExt->AutoRepair = !pHouseExt->AutoRepair;
		}
	}
}

void ExtraPhobosNetEvent::Handlers::RaiseSpreadAttack(TechnoClass* pTechno)
{
	pTechno->ClickedEvent(static_cast<NetworkEvents>(ExtraPhobosNetEvent::Events::SpreadAttack));
}

void ExtraPhobosNetEvent::Handlers::RespondToSpreadAttack(EventClass* pEvent)
{
	auto pTarget = &pEvent->Data.Target.Whom;

	if (auto pTechno = pTarget->As_Techno())
	{
		if (const auto pExt = TechnoExt::ExtMap.Find(pTechno))
		{
			pExt->SpreadAttackCommand();
		}
	}
}

void ExtraPhobosNetEvent::Handlers::RaiseToSelectSW(HouseClass* pHouse)
{
	EventClass Event {};
	Event.Type = static_cast<NetworkEvents>(ExtraPhobosNetEvent::Events::ToSelectSW);

	TargetClass house;
	house.m_ID = pHouse->ArrayIndex;

	SpecialClick1 Datas { house };
	memcpy(&Event.Data.nothing, &Datas, SpecialClick1::size());

	EventClass::AddEvent(Event);
}

void ExtraPhobosNetEvent::Handlers::RespondToSelectSW(EventClass* pEvent)
{
	TargetClass* ID = reinterpret_cast<TargetClass*>(pEvent->Data.nothing.Data);
	if (auto pHouse = HouseClass::Array()->GetItem(ID->m_ID))
	{
		if (const auto pHouseExt = HouseExt::ExtMap.Find(pHouse))
		{
			HouseExt::SetSelectSWList(pHouse);
			pHouseExt->ToSelectSW = true;
		}
	}
}

void ExtraPhobosNetEvent::Handlers::RaiseCreateBuilding(CoordStruct coord)
{
	EventClass Event {};
	Event.Type = static_cast<NetworkEvents>(ExtraPhobosNetEvent::Events::CreateBuilding);

	TargetClass x;
	x.m_ID = coord.X;

	TargetClass y;
	y.m_ID = coord.Y;

	TargetClass z;
	z.m_ID = coord.Z;

	CoordStructClick Datas { x,y,z };
	memcpy(&Event.Data.nothing, &Datas, CoordStructClick::size());

	EventClass::AddEvent(Event);
}

void ExtraPhobosNetEvent::Handlers::RespondToCreateBuilding(EventClass* pEvent)
{
	auto pHouse = HouseClass::Array()->GetItem(pEvent->HouseIndex);
	if (pHouse->Defeated || pHouse->IsObserver())
		return;

	TargetClass* ID = reinterpret_cast<TargetClass*>(pEvent->Data.nothing.Data);
	CoordStruct coord;

	coord.X = ID->m_ID;

	++ID;
	coord.Y = ID->m_ID;

	++ID;
	coord.Z = ID->m_ID;

	GScreenCreate::Active(coord);
}

void ExtraPhobosNetEvent::Handlers::RaiseCreateBuildingAuto(TechnoClass* pTechno)
{
	pTechno->ClickedEvent(static_cast<NetworkEvents>(ExtraPhobosNetEvent::Events::CreateBuildingAuto));
}

void ExtraPhobosNetEvent::Handlers::RespondToCreateBuildingAuto(EventClass* pEvent)
{
	auto pTarget = &pEvent->Data.Target.Whom;

	if (auto pTechno = pTarget->As_Techno())
	{
		if (const auto pHouseExt = HouseExt::ExtMap.Find(pTechno->Owner))
		{
			pHouseExt->AutoFire = !pHouseExt->AutoFire;
		}
	}
}

void ExtraPhobosNetEvent::Handlers::RaiseBackwarp(TechnoClass* pTechno)
{
	pTechno->ClickedEvent(static_cast<NetworkEvents>(ExtraPhobosNetEvent::Events::Backwarp));
}

void ExtraPhobosNetEvent::Handlers::RespondToBackwarp(EventClass* pEvent)
{
	auto pTarget = &pEvent->Data.Target.Whom;
	
	if (auto pTechno = pTarget->As_Techno())
	{
		if (const auto pExt = TechnoExt::ExtMap.Find(pTechno))
		{
			pExt->BackwarpActive();
		}
	}
}

void ExtraPhobosNetEvent::Handlers::RaiseAutoCreateBuilding(CoordStruct coord)
{
	EventClass Event {};
	Event.Type = static_cast<NetworkEvents>(ExtraPhobosNetEvent::Events::AutoCreateBuilding);

	TargetClass x;
	x.m_ID = coord.X;

	TargetClass y;
	y.m_ID = coord.Y;

	TargetClass z;
	z.m_ID = coord.Z;

	CoordStructClick Datas { x,y,z };
	memcpy(&Event.Data.nothing, &Datas, CoordStructClick::size());

	EventClass::AddEvent(Event);
}

void ExtraPhobosNetEvent::Handlers::RespondToAutoCreateBuilding(EventClass* pEvent)
{
	TargetClass* ID = reinterpret_cast<TargetClass*>(pEvent->Data.nothing.Data);
	CoordStruct coord;

	coord.X = ID->m_ID;

	++ID;
	coord.Y = ID->m_ID;

	++ID;
	coord.Z = ID->m_ID;

	GScreenCreate::Active(coord, true);
}

void ExtraPhobosNetEvent::Handlers::RaiseUpdateGScreenCreate()
{
	EventClass Event {};
	Event.Type = static_cast<NetworkEvents>(ExtraPhobosNetEvent::Events::UpdateGScreenCreate);

	EventClass::AddEvent(Event);
}

void ExtraPhobosNetEvent::Handlers::RespondToUpdateGScreenCreate(EventClass* pEvent)
{
	GScreenCreate::UpdateAll();
}

void ExtraPhobosNetEvent::Handlers::RaiseToCheckSelectSW(HouseClass* pHouse)
{
	EventClass Event {};
	Event.Type = static_cast<NetworkEvents>(ExtraPhobosNetEvent::Events::CheckSelectSW);

	TargetClass house;
	house.m_ID = pHouse->ArrayIndex;

	SpecialClick1 Datas { house };
	memcpy(&Event.Data.nothing, &Datas, SpecialClick1::size());

	EventClass::AddEvent(Event);
}

void ExtraPhobosNetEvent::Handlers::RespondToCheckSelectSW(EventClass* pEvent)
{
	TargetClass* ID = reinterpret_cast<TargetClass*>(pEvent->Data.nothing.Data);
	if (auto pHouse = HouseClass::Array()->GetItem(ID->m_ID))
	{
		HouseExt::SelectSW(pHouse);
	}
}

void ExtraPhobosNetEvent::Handlers::RaiseAttachEffect(TechnoClass* pTechno)
{
	pTechno->ClickedEvent(static_cast<NetworkEvents>(ExtraPhobosNetEvent::Events::AttachEffect));
}

void ExtraPhobosNetEvent::Handlers::RespondToAttachEffect(EventClass* pEvent)
{
	auto pTarget = &pEvent->Data.Target.Whom;

	if (auto pTechno = pTarget->As_Techno())
	{
		if (const auto pExt = TechnoExt::ExtMap.Find(pTechno))
		{
			pExt->AttachEffectCommand();
		}
	}
}

void ExtraPhobosNetEvent::Handlers::RaiseEnterPassenger(TechnoClass* pTechno)
{
	pTechno->ClickedEvent(static_cast<NetworkEvents>(ExtraPhobosNetEvent::Events::EnterPassenger));
}

void ExtraPhobosNetEvent::Handlers::RespondToEnterPassenger(EventClass* pEvent)
{
	auto pTarget = &pEvent->Data.Target.Whom;

	if (auto pTechno = pTarget->As_Techno())
	{
		if (const auto pExt = TechnoExt::ExtMap.Find(pTechno))
		{
			pExt->EnterPassengerCommand();
		}
	}
}
