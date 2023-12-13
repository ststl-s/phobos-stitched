#pragma once

#include <AbstractClass.h>

#include <Utilities/Container.h>

class AbstractExt
{
public:
	using base_type = AbstractClass;

	static constexpr DWORD Canary = 0x23EBD333;

	class ExtData final : public Extension<AbstractClass>
	{
	public:
		ExtData(AbstractClass* OwnerAbstract) : Extension<AbstractClass>(OwnerAbstract)
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

	class ExtContainer final : public Container<AbstractExt>
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

	inline static bool IsAbstract(const AbstractClass* const pAbstract)
	{
		return ExtMap.Find(pAbstract) != nullptr;
	}
};
