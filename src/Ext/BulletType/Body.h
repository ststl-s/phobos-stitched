#pragma once
#include <BulletTypeClass.h>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

#include <New/Type/LaserTrailTypeClass.h>

#include <Ext/Bullet/Trajectories/PhobosTrajectory.h>

class BulletTypeExt
{
public:
	using base_type = BulletTypeClass;

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
		Valueable<bool> Shrapnel_PriorityVerses;
		Nullable<Leptons> ClusterScatter_Min;
		Nullable<Leptons> ClusterScatter_Max;

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
			, Shrapnel_PriorityVerses { false }
			, TrajectoryType { nullptr }
			, Trajectory_Speed { 100.0 }
			, ClusterScatter_Min {}
			, ClusterScatter_Max {}
			, BallisticScatter_Min {}
			, BallisticScatter_Max {}
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
