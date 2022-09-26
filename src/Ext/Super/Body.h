#pragma once
#include <SuperClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

class SuperExt
{
public:
	using base_type = SuperClass;

	class ExtData final : public Extension<SuperClass>
	{
	public:

		int StoredTime;
		CDTimerClass StoredTimer;

		ExtData(base_type* OwnerObject)
			: Extension<base_type>(OwnerObject)
			, StoredTime(0)
			, StoredTimer()
		{}

		void UpdateStored();

		virtual ~ExtData() = default;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;

		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:

		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<SuperExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;
	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
};
