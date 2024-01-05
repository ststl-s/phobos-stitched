#include <ScenarioClass.h>
#include <TacticalClass.h>
#include <TerrainClass.h>

#include <Ext/TerrainType/Body.h>

#include <Utilities/Macro.h>

LightConvertClass* __fastcall InitLightConvert(const CustomPalette& palette, int terrainTypeIndex)
{
	if (!DSurface::Primary)
		return nullptr;

	TintStruct tint = TintStruct();
	ScenarioClass::Instance->ScenarioLighting(&tint.Red, &tint.Green, &tint.Blue);
	auto& cache = TerrainTypeExt::LightConvertCache[terrainTypeIndex];
	int shadeCount = tint.Red + tint.Green + tint.Blue < 2000 ? 27 : 53;
	
	if (cache.contains(tint) && cache.at(tint) != nullptr)
		return cache.at(tint);

	LightConvertClass* pLC = GameCreate<LightConvertClass>
		(
			palette.Palette.get(),
			&FileSystem::TEMPERAT_PAL,
			DSurface::Primary,
			tint.Red,
			tint.Green,
			tint.Blue,
			!cache.empty(),
			nullptr,
			shadeCount
		);

	LightConvertClass::Array->AddItem(pLC);
	cache[tint] = pLC;

	return pLC;
}

DEFINE_HOOK(0x71C1B0, TerrainClass_DrawIt, 0x5)
{
	GET(TerrainClass*, pThis, ECX);
	GET_STACK(Point2D*, pLocation, 0x4);
	GET_STACK(RectangleStruct*, pBounds, 0x8);

	enum { retn = 0x71C35A };

	SHPStruct* pShape = pThis->GetImage();

	if (pShape == nullptr)
		return retn;

	const TerrainTypeClass* pType = pThis->Type;
	const auto pTypeExt = TerrainTypeExt::ExtMap.Find(pType);
	const CellStruct cell = pThis->GetMapCoords();
	const CellClass* pCell = MapClass::Instance->GetCellAt(cell);
	int frame = 0;

	if (pType->IsAnimated)
		frame = pThis->Animation.Value;
	else if (pThis->TimeToDie)
		frame = pThis->TimeToDie + pThis->Animation.Value;
	else if (pThis->Health < 2)
		frame = 1;

	Point2D position(pLocation->X, pLocation->Y);
	int zAdjust = -TacticalClass::AdjustForZ(pThis->GetZ()) - 12;
	ConvertClass* pConvert = nullptr;
	int intensity = 0;
	BlitterFlags flags = BlitterFlags::None;

	if (pType->SpawnsTiberium)
	{
		pConvert = FileSystem::GRFTXT_TIBERIUM_PAL;
		intensity = pCell->Intensity_Normal;
		position.Y -= 16;
	}
	else
	{
		pConvert = pCell->LightConvert;
		intensity = pCell->Intensity_Terrain;
	}

	if (pType->IsAnimated || pThis->TimeToDie)
		flags = BlitterFlags::Flat | BlitterFlags::Alpha | BlitterFlags::bf_400 | BlitterFlags::Centered;
	else
		flags = BlitterFlags::ZReadWrite | BlitterFlags::Alpha | BlitterFlags::bf_400 | BlitterFlags::Centered;

	if (pTypeExt->Palette.GetConvert() != nullptr)
	{
		pConvert = InitLightConvert(pTypeExt->Palette, pType->ArrayIndex);
	}

	DSurface::Temp->DrawSHP
	(
		pConvert,
		pShape,
		frame,
		position,
		*pBounds,
		flags,
		0,
		zAdjust,
		ZGradient::Deg90,
		intensity
	);

	if (Game::bDrawShadow)
		DSurface::Temp->DrawSHP
		(
			pConvert,
			pShape,
			frame + pShape->Frames / 2,
			position,
			*pBounds,
			flags | BlitterFlags::Darken,
			0,
			zAdjust + 9,
			ZGradient::Ground
		);

	return retn;
}
