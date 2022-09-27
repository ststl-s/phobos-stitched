#pragma once

#include <EBolt.h>

#include <Utilities/Container.h>
#include <Utilities/Template.h>
#include <Utilities/Savegame.h>

class EBoltExt
{
public:
	using base_type = EBolt;

	class ExtData final : public Extension<EBolt>
	{
	public:

		ColorStruct Color1;
		ColorStruct Color2;
		ColorStruct Color3;
		Vector3D<bool> Disable;

		ExtData(EBolt* OwnerObject) : Extension<EBolt>(OwnerObject)
			, Color1 { ColorStruct::Black }
			, Color2 { ColorStruct::Black }
			, Color3 { ColorStruct::Black }
			, Disable { false, false, false }
		{ }

		virtual ~ExtData() = default;

		virtual void LoadFromStream(PhobosStreamReader& stm) override;
		virtual void SaveToStream(PhobosStreamWriter& stm) override;

	private:
		template <typename T>
		void Serialize(T& stm);
	};

	class ExtContainer final : public Container<EBoltExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;
};
