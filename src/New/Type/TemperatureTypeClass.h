#pragma once
#include <map>

#include <Utilities/Enumerable.h>

#include <New/Type/AttachEffectTypeClass.h>

class TemperatureTypeClass final : public Enumerable<TemperatureTypeClass>
{
public:

	std::map<double, std::pair<AttachEffectTypeClass*, WeaponTypeClass*>> Effects;
	Valueable<int> Minimum;
	Valueable<int> HeatUp_Frame;
	Valueable<double> HeatUp_Percent;
	const size_t ArrayIndex;
	
	TemperatureTypeClass(const char* pName = NONE_STR) : Enumerable<TemperatureTypeClass>(pName)
		, Minimum { 0 }
		, HeatUp_Frame { 60 }
		, HeatUp_Percent { 0.1 }
		, ArrayIndex { Array.size() }
	{ }

	virtual ~TemperatureTypeClass() = default;
	virtual void LoadFromINI(CCINIClass* pINI) override;
	virtual void LoadFromStream(PhobosStreamReader& stm) override;
	virtual void SaveToStream(PhobosStreamWriter& stm) override;

	void Update(TechnoClass* pTechno) const;

private:
	template <typename T>
	void Serialize(T& stm);
};
