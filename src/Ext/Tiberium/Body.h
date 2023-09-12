#pragma once

#include <TiberiumClass.h>

#include <Utilities/Container.h>
#include <Utilities/Template.h>

class TiberiumExt
{
public:
	using base_type = TiberiumClass;

	class ExtData final : public Extension<TiberiumClass>
	{
	public:
		Nullable<ColorStruct> MinimapColor;

		ExtData(TiberiumClass* OwnerObject) : Extension<TiberiumClass>(OwnerObject)
			, MinimapColor {}
		{ }

		virtual ~ExtData() = default;

		virtual void LoadFromINIFile(CCINIClass* pINI) override;

		virtual void InvalidatePointer(void* ptr, bool bRemoved) override { }

		virtual void LoadFromStream(PhobosStreamReader& Stm) override;
		virtual void SaveToStream(PhobosStreamWriter& Stm) override;

	private:
		template <typename T>
		void Serialize(T& Stm);
	};

	class ExtContainer final : public Container<TiberiumExt>
	{
	public:
		ExtContainer();
		~ExtContainer();
	};

	static ExtContainer ExtMap;

	static bool LoadGlobals(PhobosStreamReader& Stm);
	static bool SaveGlobals(PhobosStreamWriter& Stm);
};
