#pragma once

#include <map>
#include <vector>

#include <New/Type/TunnelTypeClass.h>

class TunnelClass
{
private:
	static std::unordered_map<int, std::unique_ptr<TunnelClass>> Instances;
public:

	TunnelTypeClass* Type = nullptr;
	std::vector<FootClass*> Passengers;

	TunnelClass() = default;
	TunnelClass(TunnelClass& other) = delete;
	TunnelClass(TunnelTypeClass* pType);
	~TunnelClass() = default;

	static std::unique_ptr<TunnelClass>& __fastcall GetInstance(TunnelTypeClass* pType);

	bool CanEnter(const FootClass* const pFoot) const;
	bool CanUnload() const;
	void Enter(FootClass* pFoot);
	void Leave(FootClass* pFoot);

	bool Load(PhobosStreamReader& stm, bool registerForChange);
	bool Save(PhobosStreamWriter& stm) const;

	bool LoadGlobals(PhobosStreamReader& stm);
	bool SaveGlobals(PhobosStreamWriter& stm);
};
