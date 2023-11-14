#pragma once

#include <Utilities/Enumerable.h>
#include <Utilities/Template.h>

class TunnelTypeClass final : public Enumerable<TunnelTypeClass>
{
public:

	Valueable<int> Passengers;
	Valueable<double> MaxSize;

	TunnelTypeClass(const char* pName = NONE_STR);

	virtual ~TunnelTypeClass() = default;
	virtual void LoadFromINI(CCINIClass* pINI) override;
	virtual void LoadFromStream(PhobosStreamReader& stm) override;
	virtual void SaveToStream(PhobosStreamWriter& stm) override;
};
