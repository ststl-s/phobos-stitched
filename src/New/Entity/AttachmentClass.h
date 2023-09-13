#pragma once

#include <Ext/TechnoType/Body.h>

#include <New/Type/AttachmentTypeClass.h>

class TechnoClass;

class AttachmentClass
{
public:
	static std::vector<AttachmentClass*> Array;

	TechnoTypeExt::ExtData::AttachmentDataEntry* Data = nullptr;
	TechnoClass* Parent = nullptr;
	TechnoClass* Child = nullptr;
	HouseClass* ChildOwner = nullptr;
	int RestoreCount = 0;
	CoordStruct OriginFLH = { 0, 0, 0 };

	// volatile, don't serialize
	// if you ever change the tree structure, you need to call CacheTreeData()
	struct Cache
	{
		TechnoClass* TopLevelParent = nullptr;

		int LastUpdateFrame = 0;
		Matrix3D ChildTransform;
		int ShadowLastUpdateFrame = 0;
		Matrix3D ChildShadowTransform;
	} Cache;

	AttachmentClass(TechnoTypeExt::ExtData::AttachmentDataEntry* data,
		TechnoClass* pParent,
		TechnoClass* pChild,
		HouseClass* pChildOwner
	) :
		Data(data),
		Parent(pParent),
		Child(pChild),
		ChildOwner(pChildOwner)
	{
		this->InitCacheData();
		Array.push_back(this);
	}

	AttachmentClass() :
		Data(),
		Parent(),
		Child(),
		ChildOwner()
	{
		Array.push_back(this);
	}

	~AttachmentClass();

	void InitCacheData();
	Matrix3D GetUpdatedTransform(VoxelIndexKey* pKey = nullptr, bool shadow = false);

	AttachmentTypeClass* GetType();
	TechnoTypeClass* GetChildType();
	Matrix3D GetChildTransformForLocation();
	CoordStruct GetChildLocation();

	void Initialize();
	void CreateChild();
	void AI();
	void Destroy(TechnoClass* pSource);
	void DestroyParent(TechnoClass* pSource);
	void ChildDestroyed();
	void SetFLHoffset();

	void Unlimbo();
	void Limbo();

	bool AttachChild(TechnoClass* pChild);
	bool DetachChild(bool force = false);

	void InvalidatePointer(void* ptr);

	bool Load(PhobosStreamReader& stm, bool registerForChange);
	bool Save(PhobosStreamWriter& stm) const;

private:
	template <typename T>
	bool Serialize(T& stm);
};
