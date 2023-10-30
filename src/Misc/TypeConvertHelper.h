#pragma once

#include <vector>

#include <Utilities/Enum.h>
#include <Utilities/Template.h>

class TypeConvertGroup
{
public:
	std::vector<TechnoTypeClass*> FromTypes;
	TechnoTypeClass* ToType = nullptr;
	AffectedHouse AppliedTo = AffectedHouse::None;
	AnimTypeClass* Anim = nullptr;

	TypeConvertGroup() = default;
	TypeConvertGroup(std::vector<TechnoTypeClass*>& fromTypes, TechnoTypeClass* pToType, AnimTypeClass* pAnim, AffectedHouse affectedHouse);

	bool Load(PhobosStreamReader& stm, bool registerForChange);
	bool Save(PhobosStreamWriter& stm) const;

private:
	template <typename T>
	bool Serialize(T& stm);
};

namespace TypeConvertHelper
{
	void Convert(FootClass* pTargetFoot, const std::vector<TypeConvertGroup>& convertPairs, HouseClass* pOwner);
}
