#pragma once

#include <BulletClass.h>

#include <Ext/BulletType/Body.h>

#include <Utilities/Constructs.h>
#include <Utilities/Container.h>
#include <Utilities/Enum.h>

class LaserTrailClass;
class PhobosTrajectory;

class BulletExt
{
public:
	using base_type = BulletClass;

	class ExtData final : public Extension<BulletClass>
	{
	public:
		BulletTypeExt::ExtData* TypeExtData = nullptr;
		HouseClass* FirerHouse = nullptr;
		int CurrentStrength = 0;
		bool IsInterceptor = false;
		InterceptedStatus InterceptedStatus = InterceptedStatus::None;
		bool Interfere = false;
		bool Interfered = false;
		bool InterfereToSource = false;
		TechnoClass* InterfereToSelf = nullptr;
		bool DetonateOnInterception = false;
		bool SnappedToTarget = false;
		std::vector<std::unique_ptr<LaserTrailClass>> LaserTrails;

		PhobosTrajectory* Trajectory = nullptr;

		bool ShouldDirectional = false;
		DirStruct BulletDir;

		CDTimerClass DetonateOnWay_Timer;

		ExtData(BulletClass* OwnerObject) : Extension<BulletClass>(OwnerObject)
			, TypeExtData { nullptr }
			, FirerHouse { nullptr }
			, CurrentStrength { 0 }
			, IsInterceptor { false }
			, InterceptedStatus { InterceptedStatus::None }
			, DetonateOnInterception { true }
			, Interfere { false }
			, Interfered { false }
			, SnappedToTarget { false }
			, LaserTrails {}
			, Trajectory { nullptr }
			, ShouldDirectional { false }
			, BulletDir {}
			, InterfereToSource { false }
			, InterfereToSelf { nullptr }
			, DetonateOnWay_Timer {}
		{ }

		virtual ~ExtData() = default;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

		void InterceptBullet(TechnoClass* pSource, WeaponTypeClass* pWeapon);
		void ApplyRadiationToCell(CellStruct Cell, int Spread, int RadLevel);
		void InitializeLaserTrails();

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<BulletExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static void DrawElectricLaserWeapon(BulletClass* pThis, WeaponTypeClass* pWeaponType);
	static void DrawElectricLaser
	(
		CoordStruct posFire,
		CoordStruct posEnd,
		int length,
		ColorStruct color,
		double amplitude,
		int duration,
		int thickness,
		bool isSupported
	);
	static CoordStruct CalculateInaccurate(const BulletTypeClass* pBulletType);

	static ExtContainer ExtMap;
};
