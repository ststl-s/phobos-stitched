#pragma once

#include <TerrainTypeClass.h>

#include <Utilities/Container.h>
#include <Utilities/Template.h>

class TerrainTypeExt
{
public:
	using base_type = TerrainTypeClass;

	class ExtData final : public Extension<TerrainTypeClass>
	{
	public:
		Valueable<int> SpawnsTiberium_Type;
		Valueable<int> SpawnsTiberium_Range;
		Valueable<PartialVector2D<int>> SpawnsTiberium_GrowthStage;
		Valueable<PartialVector2D<int>> SpawnsTiberium_CellsPerAnim;
		Nullable<AnimTypeClass*> DestroyAnim;
		NullableIdx<VocClass> DestroySound;
		Nullable<ColorStruct> MinimapColor;
		Valueable<bool> IsPassable;
		Valueable<bool> CanBeBuiltOn;
		Valueable<int> CrushableLevel;

		ExtData(TerrainTypeClass* OwnerObject) : Extension<TerrainTypeClass>(OwnerObject)
			, SpawnsTiberium_Type { 0 }
			, SpawnsTiberium_Range { 1 }
			, SpawnsTiberium_GrowthStage { { 3, 0 } }
			, SpawnsTiberium_CellsPerAnim { { 1, 0 } }
			, DestroyAnim {}
			, DestroySound {}
			, MinimapColor {}
			, IsPassable { false }
			, CanBeBuiltOn { false }
			, CrushableLevel { this->OwnerObject()->Crushable ? 0 : 10 }
		{ }

		virtual ~ExtData() = default;

		virtual void LoadFromINIFile(CCINIClass* pINI) override;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

		int GetTiberiumGrowthStage();
		int GetCellsPerAnim();

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<TerrainTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static void Remove(TerrainClass* pTerrain);
};
