#pragma once

#include <HouseTypeClass.h>

#include <Utilities/Container.h>
#include <Utilities/Template.h>

class SuperWeaponTypeClass;
class AttachEffectTypeClass;

class HouseTypeExt
{
public:
	using base_type = HouseTypeClass;

	static constexpr DWORD Canary = 0xAFFEAFFE;

	class ExtData final : public Extension<HouseTypeClass>
	{
	public:
		Valueable<AttachEffectTypeClass*> AttachEffects_OnInit_Building;
		Valueable<AttachEffectTypeClass*> AttachEffects_OnInit_Vehicle;
		Valueable<AttachEffectTypeClass*> AttachEffects_OnInit_Infantry;
		Valueable<AttachEffectTypeClass*> AttachEffects_OnInit_Aircraft;
		Valueable<AttachEffectTypeClass*> AttachEffects_OnInit_Defense;

		Valueable<bool> CountryCrew;
		Nullable<InfantryTypeClass*> CountryCrew_Type;

		Valueable<bool> ScoreSuperWeapon_OnlyOnce;

		struct ScoreSuperWeaponDataEntry
		{
			ValueableIdx<SuperWeaponTypeClass> IdxType { };
			Valueable<int> Score { 0 };
			bool AlreadyGranted { false };
			int Index { 0 };

			bool Load(PhobosStreamReader& stm, bool registerForChange);
			bool Save(PhobosStreamWriter& stm) const;

		private:
			template <typename T>
			bool Serialize(T& stm);
		};

		ValueableVector<ScoreSuperWeaponDataEntry> ScoreSuperWeaponData;

		Nullable<AnimTypeClass*> Parachute_Anim;
		Nullable<int> Parachute_OpenHeight;

		Valueable<InfantryTypeClass*> PilotType;

		ExtData(HouseTypeClass* OwnerObject) : Extension<HouseTypeClass>(OwnerObject)
			, ScoreSuperWeaponData()
			, CountryCrew(false)
			, CountryCrew_Type()
			, ScoreSuperWeapon_OnlyOnce(false)
			, AttachEffects_OnInit_Building()
			, AttachEffects_OnInit_Vehicle()
			, AttachEffects_OnInit_Infantry()
			, AttachEffects_OnInit_Aircraft()
			, AttachEffects_OnInit_Defense()
			, Parachute_Anim()
			, Parachute_OpenHeight()
			, PilotType(nullptr)
		{ }

		virtual ~ExtData() = default;

		virtual void LoadFromINIFile(CCINIClass* pINI) override;
		virtual void Initialize() override;
		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<HouseTypeExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
		virtual void InvalidatePointer(void* ptr, bool bRemoved) override;
	};

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);

	static ExtContainer ExtMap;

	static AttachEffectTypeClass* GetAttachEffectOnInit(HouseTypeClass* pThis, TechnoClass* pTechno);
};
