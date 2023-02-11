#pragma once

#include <GeneralStructures.h>
#include <SpecificStructures.h>
#include <Ext/TechnoType/Body.h>
#include <New/Type/ShieldTypeClass.h>

class TechnoClass;
class WarheadTypeClass;

class ShieldClass
{
public:
	static std::vector<ShieldClass*> Array;

	ShieldClass();
	ShieldClass(TechnoClass* pTechno, bool isAttached);
	ShieldClass(TechnoClass* pTechno) : ShieldClass(pTechno, false) { };
	~ShieldClass();

	int ReceiveDamage(args_ReceiveDamage* args);
	bool CanBeTargeted(WeaponTypeClass* pWeapon);
	bool CanBePenetrated(const WarheadTypeClass* pWarhead);
	void BreakShield(AnimTypeClass* pBreakAnim = nullptr, WeaponTypeClass* pBreakWeapon = nullptr);

	void SetRespawn(int duration, double amount, int rate, bool resetTimer);
	void SetSelfHealing(int duration, double amount, int rate, bool resetTimer);
	void KillAnim();
	void AI_Temporal();
	void AI();

	void DrawShieldBar(HealthBarTypeClass* pShieldBar, int iLength, const Point2D& location, const RectangleStruct& bound);
	double GetHealthRatio();
	void SetHP(int amount);
	int GetHP();
	bool IsActive();
	bool IsAvailable();
	bool IsBrokenAndNonRespawning();
	ShieldTypeClass* GetType();
	int GetFramesSinceLastBroken();
	void SetAnimationVisibility(bool visible);

	void ReplaceArmor(int armorIdx);
	void SetArmorReplaced(bool replaced = false);
	int GetArmorIndex() const;

	bool IsGreenSP();
	bool IsYellowSP();
	bool IsRedSP();

	static void SyncShieldToAnother(TechnoClass* pFrom, TechnoClass* pTo);
	static bool ShieldIsBrokenTEvent(ObjectClass* pAttached);

	static void PointerGotInvalid(void* ptr, bool removed);
	bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	bool Save(PhobosStreamWriter& Stm) const;

private:
	template <typename T>
	bool Serialize(T& Stm);

	void UpdateType();

	void SelfHealing();
	int GetPercentageAmount(double iStatus);

	void RespawnShield();

	void CreateAnim();
	void UpdateIdleAnim();
	AnimTypeClass* GetIdleAnimType();

	void WeaponNullifyAnim(AnimTypeClass* pHitAnim = nullptr, int hitDir = -1, int shieldDamage = 0);
	void ResponseAttack();
	void ShieldStolen(args_ReceiveDamage* args, int shieldDamage);

	void CloakCheck();
	void OnlineCheck();
	void TemporalCheck();
	bool ConvertCheck();

	void DrawShieldBar_Building(HealthBarTypeClass* pShieldBar, int iLength, const Point2D& location, const RectangleStruct& bound);
	void DrawShieldBar_Other(HealthBarTypeClass* pShieldBar, int iLength, const Point2D& location, const RectangleStruct& bound);
	void DrawShieldBar_Picture(HealthBarTypeClass* pShieldBar, int iLength, const Point2D& location, const RectangleStruct& bound);
	int DrawShieldBar_Pip(HealthBarTypeClass* pShieldBar, const bool isBuilding);
	int DrawShieldBar_PipAmount(int iLength);

	/// Properties ///
	TechnoClass* Techno = nullptr;
	char TechnoID[0x18] = "";
	int HP = 0;
	AnimClass* IdleAnim = nullptr;
	bool Cloak = false;
	bool Online = false;
	bool Temporal = false;
	bool Available = false;
	bool Attached = false;
	bool AreAnimsHidden = false;
	bool ArmorReplaced = false;
	int ReplacedArmorIdx = 0;

	double SelfHealing_Warhead = 0.0;
	int SelfHealing_Rate_Warhead = 0;
	double Respawn_Warhead = 0.0;
	int Respawn_Rate_Warhead = 0;

	int LastBreakFrame = 0;
	double LastTechnoHealthRatio = 0.0;

	ShieldTypeClass* Type = nullptr;

	struct Timers
	{
		Timers() :
			SelfHealing { }
			, SelfHealing_Warhead { }
			, Respawn { }
			, Respawn_Warhead { }
		{ }

		CDTimerClass SelfHealing;
		CDTimerClass SelfHealing_Warhead;
		CDTimerClass Respawn;
		CDTimerClass Respawn_Warhead;

	} Timers;
};
