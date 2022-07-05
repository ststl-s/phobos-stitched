#include "NewSWType.h"

std::vector<std::unique_ptr<NewSWType>> NewSWType::Array;

void NewSWType::Register(std::unique_ptr<NewSWType> pType)
{
	pType->SetTypeIndex(static_cast<int>(Array.size()));
	Array.emplace_back(std::move(pType));
}

void NewSWType::Init()
{
	if (!Array.empty())
		return;
}

int NewSWType::GetNewSWTypeIdx(const char* TypeID)
{
	for (const auto& it : Array)
	{
		if (!_strcmpi(it->GetTypeID(), TypeID))
			return it->GetTypeIndex();
	}

	return -1;
}

NewSWType* NewSWType::GetNthItem(int idx)
{
		return Array[idx].get();
}

int NewSWType::GetTypeIndex()
{
	return this->TypeIndex;
}

void NewSWType::SetTypeIndex(int idx)
{
	this->TypeIndex = idx;
}