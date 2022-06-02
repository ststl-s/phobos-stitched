#pragma once

#include <AnimTypeClass.h>

#include <Utilities/Container.h>
#include <Utilities/Enum.h>
#include <Utilities/Constructs.h>
#include <Utilities/Template.h>

#include <SuperClass.h>

class AnimTypeExt
{
public:
	using base_type = AnimTypeClass;

	class ExtData final : public Extension<AnimTypeClass>
	{
	public:
		CustomPalette Palette;
		Valueable<UnitTypeClass*> CreateUnit;
		Valueable<unsigned short> CreateUnit_Facing;
		Valueable<bool> CreateUnit_InheritDeathFacings;
		Valueable<bool> CreateUnit_InheritTurretFacings;
		Valueable<bool> CreateUnit_RemapAnim;
		Valueable<bool> CreateUnit_RandomFacing;
		Valueable<Mission> CreateUnit_Mission;
		Valueable<OwnerHouseKind> CreateUnit_Owner;
		Valueable<bool> CreateUnit_ConsiderPathfinding;
		Valueable<int> XDrawOffset;
		Valueable<int> HideIfNoOre_Threshold;
		Nullable<bool> Layer_UseObjectLayer;
		Valueable<bool> UseCenterCoordsIfAttached;
		Nullable<WeaponTypeClass*> Weapon;
		Valueable<int> Damage_Delay;
		Valueable<bool> Damage_DealtByInvoker;
		Valueable<bool> Damage_ApplyOnce;
		Valueable<bool> ExplodeOnWater;
		Valueable<bool> Warhead_Detonate;
		NullableVector<AnimTypeClass*> SplashAnims;
		Valueable<bool> SplashAnims_PickRandom;
		NullableVector<SuperWeaponTypeClass*> SuperWeapons;
		Valueable<bool> SuperWeapons_RealLaunch;

		ExtData(AnimTypeClass* OwnerObject) : Extension<AnimTypeClass>(OwnerObject)
			, Palette { CustomPalette::PaletteMode::Temperate }
			, CreateUnit_Facing { 0 }
			, CreateUnit_RandomFacing { true }
			, CreateUnit_InheritDeathFacings { false }
			, CreateUnit_InheritTurretFacings { false }
			, CreateUnit_RemapAnim { false }
			, CreateUnit_Mission { Mission::Guard }
			, CreateUnit_Owner { OwnerHouseKind::Victim }
			, CreateUnit_ConsiderPathfinding { false }
			, XDrawOffset { 0 }
			, HideIfNoOre_Threshold { 0 }
			, Layer_UseObjectLayer {}
			, UseCenterCoordsIfAttached { false }
			, Weapon {}
			, Damage_Delay { 0 }
			, Damage_DealtByInvoker { false }
			, Damage_ApplyOnce { false }
			, SuperWeapons {}
			, SuperWeapons_RealLaunch { false }
			, ExplodeOnWater { false }
			, Warhead_Detonate { false }
			, SplashAnims {}
			, SplashAnims_PickRandom { false }
		{ }

		virtual ~ExtData() = default;

		virtual void LoadFromINIFile(CCINIClass* pINI) override;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<AnimTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static const void ProcessDestroyAnims(UnitClass* pThis, TechnoClass* pKiller = nullptr);
};
