#pragma once

#include <BulletTypeClass.h>

#include <Ext/Bullet/Trajectories/PhobosTrajectory.h>

#include <New/Type/LaserTrailTypeClass.h>

#include <Utilities/Container.h>

class BulletTypeExt
{
public:
	using base_type = BulletTypeClass;

	static constexpr DWORD Canary = 0xF00DF00D;
	static constexpr size_t ExtPointerOffset = 0x18;

	class ExtData final : public Extension<BulletTypeClass>
	{
	public:
		Valueable<int> Strength;
		Valueable<bool> Strength_UseDamage;
		Nullable<ArmorType> Armor;
		Valueable<bool> Interceptable;
		Valueable<bool> Interceptable_DeleteOnIntercept;
		Nullable<WeaponTypeClass*> Interceptable_WeaponOverride;
		Valueable<bool> Interceptable_InterfereOnIntercept;
		ValueableIdxVector<LaserTrailTypeClass> LaserTrail_Types;
		Nullable<double> Gravity;
		Valueable<bool> Shrapnel_AffectsGround;
		Valueable<bool> Shrapnel_AffectsBuildings;
		Valueable<bool> Shrapnel_PriorityVersus;
		Valueable<bool> Shrapnel_IncludeAir;
		Valueable<bool> Shrapnel_IgnoreZeroVersus;
		Valueable<bool> Shrapnel_IgnoreCell;
		Valueable<double> Shrapnel_Chance;
		Nullable<Leptons> ClusterScatter_Min;
		Nullable<Leptons> ClusterScatter_Max;
		Valueable<bool> ImmuneToBlackhole;
		Valueable<bool> ImmuneToBlackhole_Destory;
		Valueable<bool> DetonateOnWay;
		Valueable<int> DetonateOnWay_Delay;
		Nullable<WeaponTypeClass*> DetonateOnWay_Weapon;
		Valueable<int> DetonateOnWay_LineDistance;
		Valueable<bool> DetonateOnWay_OnCell;

		Valueable<WeaponTypeClass*> ReturnWeapon;

		// Ares 0.7
		Nullable<Leptons> BallisticScatter_Min;
		Nullable<Leptons> BallisticScatter_Max;

		// Trajactories
		PhobosTrajectoryType* TrajectoryType;
		Valueable<double> Trajectory_Speed;

		ExtData(BulletTypeClass* OwnerObject) : Extension<BulletTypeClass>(OwnerObject)
			, Strength { 0 }
			, Strength_UseDamage { false }
			, Armor {}
			, Interceptable { false }
			, Interceptable_DeleteOnIntercept { false }
			, Interceptable_WeaponOverride {}
			, Interceptable_InterfereOnIntercept { false }
			, LaserTrail_Types {}
			, Gravity {}
			, Shrapnel_AffectsGround { false }
			, Shrapnel_AffectsBuildings { false }
			, Shrapnel_PriorityVersus { false }
			, Shrapnel_IncludeAir { false }
			, Shrapnel_IgnoreZeroVersus { false }
			, Shrapnel_IgnoreCell { false }
			, Shrapnel_Chance { 1.0 }
			, TrajectoryType { nullptr }
			, Trajectory_Speed { 100.0 }
			, ClusterScatter_Min {}
			, ClusterScatter_Max {}
			, BallisticScatter_Min {}
			, BallisticScatter_Max {}
			, ImmuneToBlackhole { false }
			, ImmuneToBlackhole_Destory { true }
			, DetonateOnWay { false }
			, DetonateOnWay_Delay { 5 }
			, DetonateOnWay_Weapon {}
			, DetonateOnWay_LineDistance { 128 }
			, DetonateOnWay_OnCell { false }
			, ReturnWeapon { nullptr }
		{ }

		virtual ~ExtData() = default;

		virtual void LoadFromINIFile(CCINIClass* pINI) override;
		// virtual void Initialize() override;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<BulletTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static double GetAdjustedGravity(BulletTypeClass* pType);
	static BulletTypeClass* GetDefaultBulletType();
};
