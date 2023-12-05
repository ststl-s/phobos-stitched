#pragma once

#include <ObjectClass.h>

#include <Utilities/Container.h>

class ObjectExt
{
public:
	using base_type = ObjectClass;

	class ExtData final : public Extension<ObjectClass>
	{
	public:
		ExtData(ObjectClass* OwnerObject) : Extension<ObjectClass>(OwnerObject)
		{ }

		virtual ~ExtData() = default;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override
		{ }

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<ObjectExt>
	{
	public:
		ExtContainer();
		~ExtContainer();

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override
		{ };
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static bool __fastcall IsReallyAlive(const ObjectClass* const pObject);
};
