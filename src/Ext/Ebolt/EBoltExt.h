#pragma once
#include <EBolt.h>

#include <Utilities/Container.h>
#include <Utilities/Template.h>

class EBoltExt
{
public:
	using base_type = EBolt;

	class ExtData final : public Extension<EBolt>
	{
		WeaponStruct Weapon;
	public:

		ExtData(EBolt* ownerObject);

		void SetWeapon(const WeaponStruct& weapon);
		void SetWeapon(WeaponTypeClass* pWeapon);
		const WeaponStruct& GetWeapon() const;
		virtual void LoadFromStream(PhobosStreamReader& stm) override;
		virtual void SaveToStream(PhobosStreamWriter& stm) override;
		virtual void InvalidatePointer(void* ptr, bool removed) override;
	};

	class ExtContainer final : public Container<EBoltExt>
	{
	public:
		ExtContainer();
		~ExtContainer();

		virtual void InvalidatePointer(void* ptr, bool removed) override;
	};

	static ExtContainer ExtMap;

};
