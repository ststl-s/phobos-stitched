#pragma once
#include <AircraftTypeClass.h>
#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

class AircraftTypeExt
{
public:
	using base_type = AircraftTypeClass;

	class ExtData final : public Extension<AircraftTypeClass>
	{
	public:

		Valueable<bool> Fire_KickOutPassenger;

		ExtData(AircraftTypeClass* OwnerObject) : Extension<AircraftTypeClass>(OwnerObject)
			, Fire_KickOutPassenger { true }
		{ }

		virtual ~ExtData() = default;

		virtual void LoadFromINIFile(CCINIClass * pINI) override;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override
		{ }

		virtual void LoadFromStream(PhobosStreamReader & Stm) override;
		virtual void SaveToStream(PhobosStreamWriter & Stm) override;

	private:
		template <typename T>
		void Serialize(T & Stm);
	};

	class ExtContainer final : public Container<AircraftTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();

		virtual bool Load(AircraftTypeClass* pThis, IStream* pStm) override;
	};

	static ExtContainer ExtMap;
	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
};
