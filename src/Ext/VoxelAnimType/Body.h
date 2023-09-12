#pragma once

#include <VoxelAnimTypeClass.h>

#include <Utilities/Container.h>
#include <Utilities/Template.h>

class LaserTrailTypeClass;

class VoxelAnimTypeExt
{
public:
	using base_type = VoxelAnimTypeClass;

	class ExtData final : public Extension<VoxelAnimTypeClass>
	{
	public:

		ValueableIdxVector<LaserTrailTypeClass> LaserTrail_Types;
		Valueable<bool> ExplodeOnWater;
		Valueable<bool> Warhead_Detonate;
		NullableVector<AnimTypeClass*> SplashAnims;
		Valueable<bool> SplashAnims_PickRandom;

		ExtData(VoxelAnimTypeClass* OwnerObject) : Extension<VoxelAnimTypeClass>(OwnerObject)
			, LaserTrail_Types()
			, ExplodeOnWater { false }
			, Warhead_Detonate { false }
			, SplashAnims {}
			, SplashAnims_PickRandom { false }
		{
		}

		virtual ~ExtData() = default;
		virtual size_t Size() const { return sizeof(*this); }
		virtual void LoadFromINIFile(CCINIClass* pINI) override;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }
		virtual void Initialize() override;
		virtual void LoadFromStream(PhobosStreamReader& Stm)override;
		virtual void SaveToStream(PhobosStreamWriter& Stm)override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<VoxelAnimTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
		virtual void InvalidatePointer(void* ptr, bool bRemoved) override;
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
};
