#pragma once

#include <algorithm>

#include <GeneralStructures.h>

#include <New/Type/AttachmentTypeClass.h>
#include <Ext/TechnoType/Body.h>
#include <vector>

class AttachmentClass
{
public:
	static std::vector<std::unique_ptr<AttachmentClass>> Array;

	TechnoTypeExt::ExtData::AttachmentDataEntry* Data;
	TechnoClass* Parent;
	TechnoClass* Child;

	AttachmentClass(TechnoTypeExt::ExtData::AttachmentDataEntry* data,
		TechnoClass* pParent, TechnoClass* pChild = nullptr) :
		Data(data),
		Parent(pParent),
		Child(pChild)
	{
		Name = ("AttachmentClass " + std::to_string(Array.size()) + " Item").c_str();
	}

	AttachmentClass(const char* Name = "<none>") :
		Data(),
		Parent(),
		Child()
	{
		this->Name = Name;
	}

	~AttachmentClass() = default;
	//auto position = std::find(Array.begin(), Array.end(), this);
	//if (position != Array.end())
	//Array.erase(position);

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

	static AttachmentClass* Find(const char* Name);
	static int FindIndex(const char* Name);
	static AttachmentClass* FindOrAllocate(const char* Name);

private:
	template <typename T>
	bool Serialize(T& stm);

	PhobosFixedString<0x30> Name;
};