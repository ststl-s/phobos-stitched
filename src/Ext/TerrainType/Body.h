#pragma once

#include <map>

#include <TerrainTypeClass.h>

#include <Utilities/Constructs.h>
#include <Utilities/Container.h>
#include <Utilities/Template.h>

class TerrainTypeExt
{
public:
	using base_type = TerrainTypeClass;

	static constexpr DWORD Canary = 0xBEE78007;
	static constexpr size_t ExtPointerOffset = 0x18;

	static std::unordered_map<int, std::map<TintStruct, LightConvertClass*>> LightConvertCache;

	class ExtData final : public Extension<TerrainTypeClass>
	{
	public:
		CustomPalette Palette;
		Valueable<int> SpawnsTiberium_Type;
		Valueable<int> SpawnsTiberium_Range;
		Valueable<PartialVector2D<int>> SpawnsTiberium_GrowthStage;
		Valueable<PartialVector2D<int>> SpawnsTiberium_CellsPerAnim;
		Nullable<AnimTypeClass*> DestroyAnim;
		NullableIdx<VocClass> DestroySound;
		Nullable<ColorStruct> MinimapColor;
		Nullable<int> CrushableLevel;
		Valueable<bool> IsPassable;
		Valueable<bool> CanBeBuiltOn;

		ExtData(TerrainTypeClass* OwnerObject) : Extension<TerrainTypeClass>(OwnerObject)
			, Palette {}
			, SpawnsTiberium_Type { 0 }
			, SpawnsTiberium_Range { 1 }
			, SpawnsTiberium_GrowthStage { { 3, 0 } }
			, SpawnsTiberium_CellsPerAnim { { 1, 0 } }
			, DestroyAnim {}
			, DestroySound {}
			, MinimapColor {}
			, IsPassable { false }
			, CanBeBuiltOn { false }
			, CrushableLevel {}
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

	static void Clear();

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static void Remove(TerrainClass* pTerrain);
};
