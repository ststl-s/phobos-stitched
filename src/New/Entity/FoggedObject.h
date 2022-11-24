#pragma once

#include <AbstractClass.h>
#include <BuildingClass.h>

#include <Ext/Cell/Body.h>

class TerrainClass;

class FoggedObject
{
public:
	static DynamicVectorClass<FoggedObject*> FoggedObjects;

	static void SaveGlobal(IStream* pStm);
	static void LoadGlobal(IStream* pStm);

	explicit FoggedObject() noexcept;
	explicit FoggedObject(BuildingClass* pBld, bool IsVisible) noexcept;
	explicit FoggedObject(TerrainClass* pTerrain) noexcept;

	// Handles Smudge and Overlay Construct
	explicit FoggedObject(CellClass* pCell, bool IsOverlay) noexcept;

	void Load(IStream* pStm);
	void Save(IStream* pStm);

	//Destructor
	virtual ~FoggedObject();

	void Render(const RectangleStruct& viewRect) const;

	static RectangleStruct Union(const RectangleStruct& rect1, const RectangleStruct& rect2);
protected:
	inline int GetIndexID() const;

	void RenderAsBuilding(const RectangleStruct& viewRect) const;
	void RenderAsSmudge(const RectangleStruct& viewRect) const;
	void RenderAsOverlay(const RectangleStruct& viewRect) const;
	void RenderAsTerrain(const RectangleStruct& viewRect) const;

	static char BuildingVXLDrawer[sizeof(BuildingClass)];
public:
	enum class CoveredType : char
	{
		Building = 1,
		Terrain,
		Smudge,
		Overlay
	};

	CoordStruct Location;
	CoveredType CoveredType;
	RectangleStruct Bound;
	bool Visible { true };

	union
	{
		struct
		{
			int Overlay = 0;
			unsigned char OverlayData = 0;
		} OverlayData;
		struct
		{
			TerrainTypeClass* Type = nullptr;
			int Frame = 0;
			bool Flat = false;
		} TerrainData;
		struct
		{
			HouseClass* Owner = nullptr;
			BuildingTypeClass* Type = nullptr;
			int ShapeFrame = 0;
			FacingClass PrimaryFacing;
			FacingClass BarrelFacing;
			RecoilData TurretRecoil;
			RecoilData BarrelRecoil;
			bool IsFirestormWall = false;
			int TurretAnimFrame = 0;
			struct
			{
				AnimTypeClass* AnimType = nullptr;
				int AnimFrame = 0;
				int ZAdjust = 0;
			} Anims[21];
		} BuildingData;
		struct
		{
			int Smudge = 0;
			int SmudgeData = 0;
			int Height = 0;
		} SmudgeData;
	};
};
