#pragma once

#include <GeneralStructures.h>
#include <PCX.h>
#include <TacticalClass.h>

#include <Utilities/SavegameDef.h>
#include <Utilities/Constructs.h>
#include <Utilities/Template.h>
#include <Helpers/Template.h>
#include <Utilities/Enum.h>

#include <New/Type/BannerTypeClass.h>

#include <vector>

class BannerClass
{
public:
	static std::vector<std::unique_ptr<BannerClass>> Array;

	BannerTypeClass* Type;
	int Id;
	int FrameIdx;
	int Rate;
	CoordStruct Position;
	int Variable;
	bool IsGlobalVariable;
	int Length;
	int FadeLevel;
	int FadeLevel_Rate;
	int Duration;
	bool Initilize;
	bool InPause;
	ConvertClass* PAL;

	BannerClass() :
		Type(),
		Id(),
		FrameIdx(0),
		Rate(-1),
		Position(),
		Variable(),
		IsGlobalVariable(),
		Length(0),
		FadeLevel(0),
		FadeLevel_Rate(0),
		Duration(-1),
		Initilize(false),
		InPause(false),
		PAL(nullptr)
	{
	}

	BannerClass(BannerTypeClass* pBannerType, int id, CoordStruct position, int variable, bool isGlobalVariable) :
		Type(pBannerType),
		Id(id),
		FrameIdx(0),
		Rate(-1),
		Position(position),
		Variable(variable),
		IsGlobalVariable(isGlobalVariable),
		Length(0),
		FadeLevel(0),
		FadeLevel_Rate(0),
		Duration(-1),
		Initilize(false),
		InPause(false),
		PAL(nullptr)
	{
		if (pBannerType->Type == BannerType::SHP)
		{
			if (pBannerType->Content_SHP_Play.Get())
				this->Rate = 0;
			else
				this->Rate = -1;

			if (pBannerType->Content_SHP_Remove.Get() && pBannerType->Content_SHP.isset())
			{
				int frame = pBannerType->Content_SHP.Get()->Frames - 1;
				this->FrameIdx = pBannerType->Content_SHP_Frame.Get(frame);
			}
			else
				this->FrameIdx = pBannerType->Content_SHP_Frame.Get(0);

			if (pBannerType->Content_SHP_FadeIn.Get())
				FadeLevel = 4;
			else if (pBannerType->Content_SHP_FadeOut.Get())
				FadeLevel = 0;
			else
				FadeLevel = pBannerType->Content_SHP_Transparency.Get();

			if (!this->PAL)
			{
				if (pBannerType->Content_SHP_DrawInShroud.Get())
				{
					if (pBannerType->Content_PAL.GetConvert())
						this->PAL = pBannerType->Content_PAL.GetConvert();
					else
						this->PAL = FileSystem::PALETTE_PAL;
				}
				else
				{
					const auto pPAL = FileSystem::LoadPALFile(pBannerType->Content_PAL_Name.data(), DSurface::Composite);
					if (pPAL)
						this->PAL = pPAL;
					else
						this->PAL = FileSystem::PALETTE_PAL;
				}
			}
		}

		if (pBannerType->Duration.Get() > 0)
			Duration = pBannerType->Duration.Get();
		else
			Duration = -1;
	}

	~BannerClass() = default;

	static void Create(BannerTypeClass* pBannerType, int id, CoordStruct position, int variable, bool isGlobal);
	static void Delete(std::unique_ptr<BannerClass>& Banner);
	static void Update();

	bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	bool Save(PhobosStreamWriter& Stm) const;

	static void Clear();
	static bool SaveGlobals(PhobosStreamWriter& stm);
	static bool LoadGlobals(PhobosStreamReader& stm);

private:
	template <typename T>
	bool Serialize(T& Stm);

	void GetPos(int& x, int& y, int width, int height);
	int GetTranslucentLevel_MouseHover(int x, int y, int width, int height, bool timeover);
	bool Render();
	bool RenderPCX(int x, int y);
	bool RenderSHP(int x, int y);
	bool RenderCSF(int x, int y);
};
