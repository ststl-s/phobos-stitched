#pragma once

#include <AircraftClass.h>

#include <Ext/AircraftType/Body.h>

#include <Utilities/Container.h>

// TODO: Implement proper extended AircraftClass.

class AircraftExt
{
public:
	using base_type = AircraftClass;

	static constexpr DWORD Canary = 0x3939618A;

	class ExtData final : public Extension<AircraftClass>
	{
	public:
		AircraftTypeExt::ExtData* TypeExtData = nullptr;
		int Strafe_FireCount = -1;

		//by 俊哥
		bool isAreaProtecting = false;
		bool isAreaGuardReloading = false;
		CoordStruct areaProtectTo = { -1,-1,-1 };
		int areaGuardTargetCheckRof = 30;
		int currentAreaProtectedIndex = 0;
		std::vector<CoordStruct> areaGuardCoords = {};
		int AreaROF = 30;
		AbstractClass* CurrentTarget = nullptr;

		ExtData(AircraftClass* OwnerObject) : Extension<AircraftClass>(OwnerObject)
		{ }

		virtual ~ExtData() = default;

		void Aircraft_AreaGuard();
		bool FighterIsCloseEngouth(const CoordStruct& coords);
		void AircraftClass_SetTargetFix();

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override
		{ }

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

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

