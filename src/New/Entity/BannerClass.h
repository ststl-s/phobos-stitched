#pragma once

#include <Utilities/Template.h>

#include <New/Type/BannerTypeClass.h>

class BannerClass
{
public:
	static std::vector<std::unique_ptr<BannerClass>> Array;

	BannerTypeClass* Type = nullptr;
	int Id = 0;
	CoordStruct Position;
	int Variables[4]={ 0, 0, 0, 0 };
	bool IsGlobalVariable = false;

	BannerClass(BannerTypeClass* pBannerType, int id, CoordStruct position, int variable[4], bool isGlobalVariable) :
		Type(pBannerType),
		Id(id),
		Position(position)
	{
		this->Type->LoadImage();
		for (int i = 0; i < 4; i++)
			this->Variables[i] = variable[i];
	}

	BannerClass() = default;

	void Render();

	void InvalidatePointer(void* ptr) { };

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	bool Save(PhobosStreamWriter& Stm) const;

	static void Clear();
	static bool LoadGlobals(PhobosStreamReader& stm);
	static bool SaveGlobals(PhobosStreamWriter& stm);

private:
	template <typename T>
	bool Serialize(T& Stm);
	void RenderPCX(int x, int y);
	void RenderSHP(int x, int y);
	void RenderCSF(int x, int y);
};
