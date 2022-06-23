#pragma once
#include <AircraftClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

// TODO: Implement proper extended AircraftClass.

class AircraftExt
{
public:
	using base_type = AircraftClass;

	class ExtData final : public Extension<AircraftClass>
	{
	public:
		ExtData(AircraftClass* OwnerObject) : Extension<AircraftClass>(OwnerObject)
		{ }

		virtual ~ExtData() = default;

		// virtual void LoadFromINIFile(CCINIClass* pINI) override;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override
		{ }

		virtual void LoadFromStream(PhobosStreamReader & Stm) override;
		virtual void SaveToStream(PhobosStreamWriter & Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<AircraftExt>
	{
	public:
		ExtContainer();
		~ExtContainer();

		virtual bool InvalidateExtDataIgnorable(void* const ptr) const override
		{
			auto const abs = static_cast<AbstractClass*>(ptr)->WhatAmI();
			switch (abs)
			{
			case AbstractType::Aircraft:
				return false;
			default:
				return true;
			}
		}
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static void FireBurst(AircraftClass* pThis, AbstractClass* pTarget, int shotNumber);
};

