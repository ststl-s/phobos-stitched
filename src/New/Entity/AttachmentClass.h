#pragma once

#include <algorithm>

#include <GeneralStructures.h>

#include <New/Type/AttachmentTypeClass.h>
#include <Ext/TechnoType/Body.h>
#include <vector>

class AttachmentClass
{
public:
	static std::vector<AttachmentClass*> Array;

	TechnoTypeExt::ExtData::AttachmentDataEntry* Data;
	TechnoClass* Parent;
	TechnoClass* Child;

	AttachmentClass(TechnoTypeExt::ExtData::AttachmentDataEntry* data,
		TechnoClass* pParent, TechnoClass* pChild = nullptr) :
		Data(data),
		Parent(pParent),
		Child(pChild)
	{
		Array.emplace_back(this);
	}

	AttachmentClass() :
		Data(),
		Parent(),
		Child()
	{
		Array.emplace_back(this);
	}

	~AttachmentClass()
	{
		auto it = std::find(Array.begin(), Array.end(), this);
	
		if (it != Array.end())
			Array.erase(it);
	}

	AttachmentTypeClass* GetType();
	TechnoTypeClass* GetChildType();

	void Initialize();
	void CreateChild();
	void AI();
	void Uninitialize();
	void ChildDestroyed();

	void Unlimbo();
	void Limbo();

	bool AttachChild(TechnoClass* pChild);
	bool DetachChild(bool force = false);

	bool Load(PhobosStreamReader& stm, bool registerForChange);
	bool Save(PhobosStreamWriter& stm) const;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

private:
	template <typename T>
	bool Serialize(T& stm);

};