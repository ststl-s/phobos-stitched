#pragma once
#include <EBolt.h>

#include <Utilities/Container.h>
#include <Utilities/Template.h>

class EBoltExt
{
	using base_type = EBolt;

	class ExtData final : public Extension<EBolt>
	{
		ColorStruct Color1;
		ColorStruct Color2;
		ColorStruct Color3;
		const WeaponStruct* Weapon;

		ExtData() = default;
	};

	class ExtContainer final : public Container<EBoltExt>
	{
	public:
		ExtContainer();
		~ExtContainer();

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override;
	};

	static ExtContainer ExtMap;

};
