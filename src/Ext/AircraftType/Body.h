#pragma once

#include <AircraftTypeClass.h>

#include <Utilities/Container.h>
#include <Utilities/Template.h>

class AircraftTypeExt
{
public:
	using base_type = AircraftTypeClass;

	class ExtData final : public Extension<AircraftTypeClass>
	{
	public:
		//by 俊哥
		Valueable<bool> Attack_OnUnit;
		Valueable<bool> Fighter_AreaGuard;
		Valueable<bool> Fighter_AutoFire;
		Valueable<bool> Fighter_FindRangeAroundSelf;
		Valueable<int> Fighter_GuardRange;
		Valueable<int> Fighter_Ammo;
		Valueable<int> Fighter_GuardRadius;
		Valueable<int> Fighter_ChaseRange;
		Valueable<bool> Fighter_CanAirToAir;

		ExtData(AircraftTypeClass* OwnerObject) : Extension<AircraftTypeClass>(OwnerObject)
			//by 俊哥
			, Attack_OnUnit { false }
			, Fighter_AreaGuard { false }
			, Fighter_AutoFire { false }
			, Fighter_FindRangeAroundSelf { false }
			, Fighter_GuardRange { 10 }
			, Fighter_Ammo { 1 }
			, Fighter_GuardRadius { 5 }
			, Fighter_ChaseRange { 30 }
			, Fighter_CanAirToAir { true }
		{ }

		virtual ~ExtData() = default;

		virtual void LoadFromINIFile(CCINIClass* pINI) override;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override
		{ }

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
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
