#pragma once


/*
https://github.com/ChrisLv-CN/YRDynamicPatcher-Kratos/blob/main/DynamicPatcher/Projects/Extension/Kraotos/CustomWeapon/CustomWeaponManager.cs
author: https://github.com/ChrisLv-CN
*/

#include <queue>
#include <vector>

#include <TechnoClass.h>

#include <Ext/Techno/Body.h>

class AttachFireData
{
public:
	bool UseROF;
	bool CheckRange;
	bool RadialFire;
	int RadialAngle;
	bool SimulateBurst;
	int SimulateBurstDelay;
	int SimulateBurstMode;
	bool OnlyFireInTransport;
	bool UseAlternateFLH;

	AttachFireData()
		: UseROF(true)
		, CheckRange(false)
		, RadialFire(false)
		, RadialAngle(180)
		, SimulateBurst(false)
		, SimulateBurstDelay(7)
		, SimulateBurstMode(0)
		, OnlyFireInTransport(false)
		, UseAlternateFLH(false)
	{ }
};

class SimulateBurst
{
public:
	WeaponTypeClass* Weapon;
	TechnoClass* Owner;
	AbstractClass* Target;
	CoordStruct FLH;
	AttachFireData FireData;
	int Burst;
	int MinRange;
	int Range;
	int FlipY;
	int Index;
	CDTimerClass Timer;
	int Flag;

	SimulateBurst
	(
		WeaponTypeClass* pWeapon,
		TechnoClass* pOwner,
		AbstractClass* pTarget,
		CoordStruct flh,
		int burst,
		int minRange,
		int range,
		AttachFireData fireData,
		int flipY
	) : Weapon(pWeapon)
		, Owner(pOwner)
		, Target(pTarget)
		, FLH(flh)
		, Burst(burst)
		, MinRange(minRange)
		, Range(range)
		, FireData(fireData)
		, FlipY(flipY)
		, Flag(flipY)
		, Index(0)
		, Timer(fireData.SimulateBurstDelay)
	{ }

	SimulateBurst& operator = (const SimulateBurst& other)
	{
		memcpy_s(this, sizeof(SimulateBurst), &other, sizeof(SimulateBurst));
		Timer.Start(FireData.SimulateBurstDelay);
	}

	bool CanFire()
	{
		if (Timer.Expired())
		{
			Timer.Start(FireData.SimulateBurstDelay);

			return true;
		}

		return false;
	}

	void CountOne()
	{
		Index++;

		switch (FireData.SimulateBurstMode)
		{
		case 1:
			// 左右切换
			FlipY *= -1;
			break;
		case 2:
			// 左一半右一半
			FlipY = (Index < Burst / 2.0) ? Flag : -Flag;
			break;
		default:
			break;
		}
	}
};


class CustomWeaponManager
{
public:

	std::queue<SimulateBurst> SimulateBurstQueue;

	void Update(TechnoClass* pOwner);

	bool FireCustomWeapon
	(
		TechnoClass* pAttacker,
		TechnoClass* pOwner,
		AbstractClass* pTarget,
		WeaponTypeClass* pWeapon,
		const CoordStruct& flh,
		const CoordStruct& bulletSourcePos,
		double rofMult
	);

	void SimulateBurstFire
	(
		TechnoClass* pAttacker,
		TechnoClass* pOwner,
		AbstractClass* pTarget,
		WeaponTypeClass* pWeapon,
		SimulateBurst& burst
	);

	void SimulateBurstFireOnce
	(
		TechnoClass* pAttacker,
		TechnoClass* pOwner,
		AbstractClass* pTarget,
		WeaponTypeClass* pWeapon,
		SimulateBurst& burst
	);

	TechnoClass* WhoIsAttacker(TechnoClass* pAttacker);
	
	static void Clear();
	static bool SaveGlobals(PhobosStreamWriter& stm);
	static bool LoadGlobals(PhobosStreamReader& stm);
};

