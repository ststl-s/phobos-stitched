#pragma once

#include <AnimClass.h>
#include <ParticleSystemClass.h>

#include <Utilities/Container.h>

class AnimExt
{
public:
	using base_type = AnimClass;

	static constexpr DWORD Canary = 0xAAAAAAAA;
	static constexpr size_t ExtPointerOffset = 0xD0;

	class ExtData final : public Extension<AnimClass>
	{
	public:
		short DeathUnitFacing = 0;
		DirStruct DeathUnitTurretFacing;
		bool FromDeathUnit = false;
		bool DeathUnitHasTurret = false;
		bool IsAttachEffectAnim = false;
		TechnoClass* Invoker = nullptr;
		HouseClass* InvokerHouse = nullptr;
		ParticleSystemClass* AttachedSystem = nullptr;
		BuildingClass* ParentBuilding = nullptr; // This is a failsafe that is only set if this is a building animation and the building is not on same cell as the animation.

		ExtData(AnimClass* OwnerObject) : Extension<AnimClass>(OwnerObject)
			, DeathUnitFacing { 0 }
			, DeathUnitTurretFacing {}
			, FromDeathUnit { false }
			, DeathUnitHasTurret { false }
			, IsAttachEffectAnim { false }
			, Invoker {}
			, InvokerHouse {}
			, AttachedSystem {}
			, ParentBuilding {}
		{ }

		virtual ~ExtData()
		{
			this->DeleteAttachedSystem();
		}

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override
		{
			AnnounceInvalidPointer(this->Invoker, ptr);
			AnnounceInvalidPointer(this->InvokerHouse, ptr);
			AnnounceInvalidPointer(this->AttachedSystem, ptr);
			AnnounceInvalidPointer(this->ParentBuilding, ptr);
		}

		virtual void InitializeConstants() override;

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

		void SetInvoker(TechnoClass* pInvoker);
		void SetInvoker(TechnoClass* pInvoker, HouseClass* pInvokerHouse);
		void CreateAttachedSystem();
		void DeleteAttachedSystem();

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<AnimExt>
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
			case AbstractType::Building:
			case AbstractType::Infantry:
			case AbstractType::Unit:
				return false;
			default:
				return true;
			}
		}
	};

	static ExtContainer ExtMap;

	static bool SetAnimOwnerHouseKind(AnimClass* pAnim, HouseClass* pInvoker, HouseClass* pVictim, bool defaultToVictimOwner = true, bool defaultToInvokerOwner = false);
	static HouseClass* GetOwnerHouse(AnimClass* pAnim, HouseClass* pDefaultOwner = nullptr);
};
