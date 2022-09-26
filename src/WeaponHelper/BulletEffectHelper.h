#pragma once

#include <LaserDrawClass.h>
#include <RadBeam.h>
#include <EBolt.h>

#include "EffectTypes.h"

class BulletEffectHelper
{
public:
	static void RedCrosshair(const CoordStruct sourcePos, int length, int thickness = 1, int duration = 1)
	{
		Crosshair(sourcePos, length, ColorStruct::Red, ColorStruct::Black, thickness, duration);
	}

	static void RedCell(const CoordStruct& sourcePos, int length, int thickness = 1, int duration = 1, bool crosshair = false)
	{
		if (crosshair)
		{
			RedCrosshair(sourcePos, length, thickness, duration);
		}
		Cell(sourcePos, length, ColorStruct::Red, ColorStruct::Black, thickness, duration);
	}

	static void GreenCrosshair(const CoordStruct& sourcePos, int length, int thickness = 1, int duration = 1)
	{
		Crosshair(sourcePos, length, ColorStruct::Green, ColorStruct::Black, thickness, duration);
	}

	static void GreenCell(const CoordStruct& sourcePos, int length, int thickness = 1, int duration = 1, bool crosshair = false)
	{
		if (crosshair)
		{
			GreenCrosshair(sourcePos, length, thickness, duration);
		}
		Cell(sourcePos, length, ColorStruct::Green, ColorStruct::Black, thickness, duration);
	}

	static void BlueCrosshair(const CoordStruct& sourcePos, int length, int thickness = 1, int duration = 1)
	{
		Crosshair(sourcePos, length, ColorStruct::Blue, ColorStruct::Black, thickness, duration);
	}

	static void BlueCell(const CoordStruct& sourcePos, int length, int thickness = 1, int duration = 1, bool crosshair = false)
	{
		if (crosshair)
		{
			BlueCrosshair(sourcePos, length, thickness, duration);
		}
		Cell(sourcePos, length, ColorStruct::Blue, ColorStruct::Black, thickness, duration);
	}

	static void Crosshair(const CoordStruct& sourcePos, int length, ColorStruct lineColor, ColorStruct outerColor = ColorStruct(0, 0, 0), int thickness = 1, int duration = 1)
	{
		DrawLine(sourcePos, sourcePos + CoordStruct(length, 0, 0), lineColor, outerColor, thickness, duration);
		DrawLine(sourcePos, sourcePos + CoordStruct(-length, 0, 0), lineColor, outerColor, thickness, duration);
		DrawLine(sourcePos, sourcePos + CoordStruct(0, -length, 0), lineColor, outerColor, thickness, duration);
		DrawLine(sourcePos, sourcePos + CoordStruct(0, length, 0), lineColor, outerColor, thickness, duration);
	}

	static void Cell(const CoordStruct& sourcePos, int length, ColorStruct lineColor, ColorStruct outerColor = ColorStruct(0, 0, 0), int thickness = 1, int duration = 1)
	{
		CoordStruct p1 = sourcePos + CoordStruct(length, length, 0);
		CoordStruct p2 = sourcePos + CoordStruct(-length, length, 0);
		CoordStruct p3 = sourcePos + CoordStruct(-length, -length, 0);
		CoordStruct p4 = sourcePos + CoordStruct(length, -length, 0);
		DrawLine(p1, p2, lineColor, outerColor, thickness, duration);
		DrawLine(p2, p3, lineColor, outerColor, thickness, duration);
		DrawLine(p3, p4, lineColor, outerColor, thickness, duration);
		DrawLine(p4, p1, lineColor, outerColor, thickness, duration);
	}

	static void RedLineZ(const CoordStruct& sourcePos, int length, int thickness = 1, int duration = 1)
	{
		RedLine(sourcePos, sourcePos + CoordStruct(0, 0, length), thickness, duration);
	}

	static void RedLine(const CoordStruct& sourcePos, const CoordStruct& targetPos, int thickness = 1, int duration = 1)
	{
		DrawLine(sourcePos, targetPos, ColorStruct::Red, ColorStruct::Black, thickness, duration);
	}

	static void GreenLineZ(const CoordStruct& sourcePos, int length, int thickness = 1, int duration = 1)
	{
		GreenLine(sourcePos, sourcePos + CoordStruct(0, 0, length), thickness, duration);
	}

	static void GreenLine(const CoordStruct& sourcePos, const CoordStruct& targetPos, int thickness = 1, int duration = 1)
	{
		DrawLine(sourcePos, targetPos, ColorStruct::Green, ColorStruct::Black, thickness, duration);
	}

	static void BlueLineZ(const CoordStruct& sourcePos, int length, int thickness = 1, int duration = 1)
	{
		BlueLine(sourcePos, sourcePos + CoordStruct(0, 0, length), thickness, duration);
	}

	static void BlueLine(const CoordStruct& sourcePos, const CoordStruct& targetPos, int thickness = 1, int duration = 1)
	{
		DrawLine(sourcePos, targetPos, ColorStruct::Blue, ColorStruct::Black, thickness, duration);
	}

	static void DrawLine(const CoordStruct& sourcePos, const CoordStruct& targetPos, ColorStruct innerColor, ColorStruct outerColor = ColorStruct::Black, int thickness = 2, int duration = 15)
	{
		WeaponLaserType type(innerColor, outerColor, duration, thickness);
		type.InnerColor = innerColor;
		type.OuterColor = outerColor;
		type.Thickness = thickness;
		type.Duration = duration;
		DrawLine(sourcePos, targetPos, type);
	}

	static void DrawLine(const CoordStruct& sourcePos, const CoordStruct& targetPos, const WeaponLaserType& type, ColorStruct houseColor = ColorStruct::Black)
	{
		ColorStruct innerColor = type.InnerColor;
		ColorStruct outerColor = type.OuterColor;

		if (houseColor != ColorStruct::Black)
		{
			innerColor = houseColor;
			outerColor = ColorStruct::Black;
		}

		LaserDrawClass* pLaser = GameCreate<LaserDrawClass>(sourcePos, targetPos, innerColor, outerColor, type.OuterSpread, type.Duration);
		pLaser->Thickness = type.Thickness;
		pLaser->IsHouseColor = type.Fade ? true : type.IsHouseColor;
		pLaser->IsSupported = type.IsSupported ? true : (type.Thickness > 5 && !type.Fade);
	}

	static void DrawBeam(const CoordStruct& sourcePos, const CoordStruct& targetPos, WeaponBeamType type, ColorStruct customColor = ColorStruct::Black)
	{
		ColorStruct beamColor = type.BeamColor;

		if (customColor != ColorStruct::Black)
		{
			beamColor = customColor;
		}

		RadBeam* pRadBeam = RadBeam::Allocate(type.BeamType);

		if (pRadBeam != nullptr)
		{
			pRadBeam->SetCoordsSource(sourcePos);
			pRadBeam->SetCoordsTarget(targetPos);
			pRadBeam->SetColor(beamColor);
			pRadBeam->Period = type.Period;
			pRadBeam->Amplitude = type.Amplitude;
		}
	}

	static void DrawBolt(const CoordStruct& sourcePos, const CoordStruct& targetPos, bool alternate = false)
	{
		WeaponEBoltType type(alternate);
		DrawBolt(sourcePos, targetPos, type);
	}

	static void DrawBolt(const CoordStruct& sourcePos, const CoordStruct& targetPos, const WeaponEBoltType& type)
	{
		EBolt* pBolt = GameCreate<EBolt>();

		pBolt->AlternateColor = type.IsAlternateColor;
		pBolt->Fire(sourcePos, targetPos, 0);

		if (!pBolt.IsNull)
		{
			EBoltExt ext = EBoltExt.ExtMap.Find(pBolt);
			if (null != ext)
			{
				ext.Color1 = type.Color1;
				ext.Color2 = type.Color2;
				ext.Color3 = type.Color3;
				ext.Disable1 = type.Disable1;
				ext.Disable2 = type.Disable2;
				ext.Disable3 = type.Disable3;
			}
			pBolt.Ref.AlternateColor = type.IsAlternateColor;
			pBolt.Ref.Fire(sourcePos, targetPos, 0);
		}
	}

	public static void DrawBolt(Pointer<TechnoClass> pShooter, Pointer<AbstractClass> pTarget, Pointer<WeaponTypeClass> pWeapon, CoordStruct sourcePos)
	{
		Pointer<EBolt> pEBolt = pShooter.Ref.Electric_Zap(pTarget, pWeapon, sourcePos);
	}

	public static void DrawParticele(CoordStruct sourcePos, CoordStruct targetPos, string systemName)
	{
		Pointer<ParticleSystemTypeClass> psType = ParticleSystemTypeClass.ABSTRACTTYPE_ARRAY.Find(systemName);
		if (!psType.IsNull)
		{
			BulletEffectHelper.DrawParticele(psType, sourcePos, targetPos);
		}
	}

	public static void DrawParticele(Pointer<ParticleSystemTypeClass> psType, CoordStruct sourcePos, CoordStruct targetPos)
	{
		BulletEffectHelper.DrawParticele(psType, sourcePos, Pointer<TechnoClass>.Zero, targetPos);
	}

	public static void DrawParticele(Pointer<ParticleSystemTypeClass> psType, CoordStruct sourcePos, Pointer<TechnoClass> pOwner, CoordStruct targetPos)
	{
		BulletEffectHelper.DrawParticele(psType, sourcePos, Pointer<AbstractClass>.Zero, pOwner, targetPos);
	}

	public static void DrawParticele(Pointer<ParticleSystemTypeClass> psType, CoordStruct sourcePos, Pointer<AbstractClass> pTarget, Pointer<TechnoClass> pOwner, CoordStruct targetPos)
	{
		YRMemory.Create<ParticleSystemClass>(psType, sourcePos, pTarget, pOwner, targetPos);
	}
};
