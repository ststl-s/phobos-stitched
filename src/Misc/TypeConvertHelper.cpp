#include "Misc/TypeConvertHelper.h"

#include <Ext/Techno/Body.h>

#include <Utilities/EnumFunctions.h>
#include <Utilities/TemplateDef.h>

TypeConvertGroup::TypeConvertGroup(std::vector<TechnoTypeClass*>& fromTypes, TechnoTypeClass* pToType, AnimTypeClass* pAnim, AffectedHouse affectedHouse)
	: FromTypes(fromTypes)
	, ToType(pToType)
	, Anim(pAnim)
	, AppliedTo(affectedHouse)
{ }

void TypeConvertHelper::Convert(FootClass* pTargetFoot, const std::vector<TypeConvertGroup>& convertPairs, HouseClass* pOwner)
{
	for (const auto& [fromTypes, toType, affectedHouses, anim] : convertPairs)
	{
		if (toType == nullptr) continue;

		if (!EnumFunctions::CanTargetHouse(affectedHouses, pOwner, pTargetFoot->Owner))
			continue;

		if (!fromTypes.empty())
		{
			for (const auto& from : fromTypes)
			{
				// Check if the target matches upgrade-from TechnoType and it has something to upgrade to
				if (from == pTargetFoot->GetTechnoType())
				{
					TechnoExt::ExtData* pExt = TechnoExt::ExtMap.Find(pTargetFoot);
					pExt->SetNeedConvert(toType, false, anim);
					//TechnoExt::Convert(pTargetFoot, toType);
					//TechnoExt::ConvertToType(pTargetFoot, toType);
					break;
				}
			}
		}
		else
		{
			TechnoExt::ExtData* pExt = TechnoExt::ExtMap.Find(pTargetFoot);
			pExt->SetNeedConvert(toType, false, anim);
			//TechnoExt::Convert(pTargetFoot, toType);
			//TechnoExt::ConvertToType(pTargetFoot, toType);
		}
	}
}

bool TypeConvertGroup::Load(PhobosStreamReader& stm, bool registerForChange)
{
	return this->Serialize(stm);
}

bool TypeConvertGroup::Save(PhobosStreamWriter& stm) const
{
	return const_cast<TypeConvertGroup*>(this)->Serialize(stm);
}

template <typename T>
bool TypeConvertGroup::Serialize(T& stm)
{
	return stm
		.Process(this->FromTypes)
		.Process(this->ToType)
		.Process(this->AppliedTo)
		.Process(this->Anim)
		.Success();
}
