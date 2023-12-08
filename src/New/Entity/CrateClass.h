#pragma once

#include <AnimClass.h>

#include <New/Type/CrateTypeClass.h>

class CrateClass
{
	struct UninitAnim
	{
		void operator () (AnimClass* const pAnim) const
		{
			if (pAnim != nullptr)
			{
				pAnim->DetachFromObject(pAnim->OwnerObject, false);
				pAnim->UnInit();
			}
		}
	};

	static std::unordered_map<int, int> Crate_Exist;

public:

	CrateTypeClass* Type = nullptr;
	int Duration = 0;
	CDTimerClass Timer;
	CellClass* Location = nullptr;
	bool Initialized = false;
	AnimClass* Anim = nullptr;
	bool IsInvalid = false;
	ValueableVector<CrateEffect> CrateEffects;
	ValueableVector<int> CrateEffects_Weights;
	ValueableVector<ObjectClass*> LastObjectList;
	HouseClass* OwnerHouse;

	CrateClass(CrateClass& other) = delete;
	CrateClass() = default;
	CrateClass(CrateTypeClass* pType, CellClass* pCell, int duration, HouseClass* pHouse);

	~CrateClass();

	void Init();
	void Update();
	void OpenCrate(TechnoClass* pTechno);
	void CreateImage();
	void KillImage();
	void DetonateWeapons(TechnoClass* pTechno);
	void AttachEffects(TechnoClass* pTechno);
	void TransactMoney(TechnoClass* pTechno);
	void RevealSight(TechnoClass* pTechno);
	void GapRadar(TechnoClass* pTechno);
	void GrantSuperWeapons(TechnoClass* pTechno);
	void RadarBlackout(TechnoClass* pTechno);
	void PowerBlackout(TechnoClass* pTechno);
	void CreateUnits(TechnoClass* pTechno);
	bool IsActive() const;

	static bool CanExist(CrateTypeClass* pType);
	static bool CheckMinimum(CrateTypeClass* pType);
	static bool CanSpwan(CrateTypeClass* pType, CellClass* pCell);
	static void CreateCrate(CrateTypeClass* pType, CellClass* pCell, HouseClass* pHouse = nullptr);

	bool Load(PhobosStreamReader& stm, bool registerForChange);
	bool Save(PhobosStreamWriter& stm) const;
	static void Clear();
	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

private:

	template <typename T>
	bool Serialize(T& stm);
};
