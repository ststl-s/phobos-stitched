#include "TunnelClass.h"

#include <BuildingClass.h>

#include <Helpers/Macro.h>

#include <Ext/TechnoType/Body.h>

std::unordered_map<int, std::unique_ptr<TunnelClass>> TunnelClass::Instances;

TunnelClass::TunnelClass(TunnelTypeClass* pType)
	: Type(pType)
{ }

std::unique_ptr<TunnelClass>& __fastcall TunnelClass::GetInstance(TunnelTypeClass* pType)
{
	if (Instances.contains(pType->ArrayIndex))
		return Instances.at(pType->ArrayIndex);

	Instances.emplace(pType->ArrayIndex, std::make_unique<TunnelClass>(pType));

	return Instances.at(pType->ArrayIndex);
}

bool TunnelClass::CanEnter(const FootClass* const pFoot) const
{
	return static_cast<int>(this->Passengers.size()) < this->Type->Passengers
		&& pFoot->GetTechnoType()->Size <= this->Type->MaxSize;
}

bool TunnelClass::CanUnload() const
{
	return !this->Passengers.empty();
}

void TunnelClass::Enter(FootClass* pFoot)
{
	pFoot->SetFocus(nullptr);
	pFoot->OnBridge = false;
	pFoot->unknown_C4 = 0;
	pFoot->GattlingValue = 0;
	pFoot->CurrentGattlingStage = 0;
	pFoot->Limbo();

	const auto it = std::find(this->Passengers.cbegin(), this->Passengers.cend(), pFoot);

	if (it == this->Passengers.cend())
		this->Passengers.emplace_back(pFoot);
}

void TunnelClass::Leave(FootClass* pFoot)
{
	const auto it = std::find(this->Passengers.cbegin(), this->Passengers.cend(), pFoot);

	if (it != this->Passengers.cend())
		this->Passengers.erase(it);
}

bool TunnelClass::Load(PhobosStreamReader& stm, bool registerForChange)
{
	return stm
		.Process(this->Type)
		.Process(this->Passengers)
		.Success()
		;
}

bool TunnelClass::Save(PhobosStreamWriter& stm) const
{
	return stm
		.Process(this->Type)
		.Process(this->Passengers)
		.Success()
		;
}

bool TunnelClass::LoadGlobals(PhobosStreamReader& stm)
{
	return stm
		.Process(Instances)
		.Success()
		;
}

bool TunnelClass::SaveGlobals(PhobosStreamWriter& stm)
{
	return stm
		.Process(Instances)
		.Success()
		;
}

//DEFINE_HOOK(0x43C326, BuildingClass_ReceiveRadioCommand_QueryCanEnter_Tunnel, 0xA)
//{
//	GET(BuildingClass*, pThis, ESI);
//	GET(FootClass*, pSender, EDI);
//
//	const auto pTypeExt = TechnoTypeExt::ExtMap.Find(pThis->Type);
//
//	if (pTypeExt->Tunnel != nullptr)
//	{
//		auto& pTunnel = TunnelClass::GetInstance(pTypeExt->Tunnel);
//
//		if(pTunnel->CanEnter(pSender))
//			
//	}
//}
