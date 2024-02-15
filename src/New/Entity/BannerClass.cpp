#include "BannerClass.h"

#include <wchar.h>
#include <BitFont.h>
#include <WWMouseClass.h>

#include <Ext/Scenario/Body.h>
#include <Utilities/EnumFunctions.h>

std::vector<std::unique_ptr<BannerClass>> BannerClass::Array;

template <typename T>
bool BannerClass::Serialize(T& Stm)
{
	return Stm
		.Process(this->Type)
		.Process(this->Id)
		.Process(this->FrameIdx)
		.Process(this->Rate)
		.Process(this->Position)
		.Process(this->Variable)
		.Process(this->IsGlobalVariable)
		.Process(this->Length)
		.Process(this->FadeLevel)
		.Process(this->FadeLevel_Rate)
		.Process(this->Duration)
		.Process(this->Initilize)
		.Process(this->PAL)
		.Success();
}

bool BannerClass::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return Serialize(Stm);
}

bool BannerClass::Save(PhobosStreamWriter& Stm) const
{
	return const_cast<BannerClass*>(this)->Serialize(Stm);
}

void BannerClass::Clear()
{
	BannerClass::Array.clear();
}

bool BannerClass::SaveGlobals(PhobosStreamWriter& stm)
{
	return stm
		.Process(BannerClass::Array)
		.Success();
}

bool BannerClass::LoadGlobals(PhobosStreamReader& stm)
{
	return stm
		.Process(BannerClass::Array)
		.Success();
}

void BannerClass::GetPos(int& x, int& y, int width, int height)
{
	int align = this->Type->Align;
	int imagealign = this->Type->ImageAlign.Get(align);
	int anchor = this->Type->Anchor;

	auto pBound = DSurface::Composite()->GetRect();
	pBound.Height -= 32;

	if (this->Type->Content_SHP_FullScreen.isset())
	{
		align = 0;
		anchor = 0;
		imagealign = 0;
	}

	if (align == 2)
	{
		x = pBound.Width - width;

		if (imagealign == 0)
			x = pBound.Width;
		else if (imagealign == 1)
			x = pBound.Width - int(width / 2);
	}
	else if (align == 1)
	{
		x = (pBound.Width - width) / 2;

		if (imagealign == 0)
			x = pBound.Width / 2;
		else if (imagealign == 2)
			x = int(pBound.Width / 2) - width;
	}
	else
	{
		if (imagealign == 1)
			x = -int(width / 2);
		else if (imagealign == 2)
			x = -width;
	}

	if (anchor == 2)
	{
		y = pBound.Height - height;
	}
	else if (anchor == 1)
	{
		y = (pBound.Height - height) / 2;
	}

	x += +this->Type->Offset.Get().X;
	y += +this->Type->Offset.Get().Y;
}

int BannerClass::GetTranslucentLevel_MouseHover(int x, int y, int width, int height, bool timeover)
{
	if (!this->Type->Content_SHP_Transparency_MouseHover.isset() || !this->Initilize || timeover)
	{
		return this->FadeLevel;
	}

	int level;
	Point2D vMouse = { WWMouseClass::Instance->GetX() ,WWMouseClass::Instance->GetY() };
	if (!WWMouseClass::Instance->IsRefCountNegative() && vMouse.X > x && vMouse.X < x + width &&
		vMouse.Y > y && vMouse.Y < y + height)
	{
		level = this->Type->Content_SHP_Transparency_MouseHover.Get();
	}
	else
	{
		level = this->Type->Content_SHP_Transparency.Get();
	}

	if (this->FadeLevel < level)
	{
		this->FadeLevel++;
	}
	else if (this->FadeLevel > level)
	{
		this->FadeLevel--;
	}

	return this->FadeLevel;
}

bool BannerClass::Render()
{
	if (!this)
	{
		return false;
	}

	int x = 0;
	int y = 0;

	if (this->Type->Percentage)
	{
		x = (int)((double)this->Position.X / 100.f * (double)DSurface::Composite->Width);
		y = (int)((double)this->Position.Y / 100.f * (double)DSurface::Composite->Height);
	}
	else
	{
		x = this->Position.X;
		y = this->Position.Y;
	}

	switch (this->Type->Type)
	{
	case BannerType::PCX:
		return this->RenderPCX(x, y);
	case BannerType::SHP:
		return this->RenderSHP(x, y);
	case BannerType::CSF:
		return this->RenderCSF(x, y);
	default:
		break;
	}

	return false;
}

bool BannerClass::RenderPCX(int x, int y)
{
	if (strcmp(this->Type->Content_PCX.data(), "") == 0)
	{
		return false;
	}

	if (this->Duration >= 0)
	{
		if (this->Duration > 0)
		{
			this->Duration--;
		}
		else
		{
			return false;
		}
	}

	int xsize = 0;
	int ysize = 0;
	BSurface* pcx;
	char filename[0x20];
	strcpy(filename, this->Type->Content_PCX.data());
	_strlwr_s(filename);

	PCX::Instance->LoadFile(filename);
	pcx = PCX::Instance->GetSurface(filename);

	if (!pcx)
	{
		return false;
	}

	int x2 = 0;
	int y2 = 0;
	this->GetPos(x2, y2, pcx->Width, pcx->Height);

	x = x + x2;
	y = y + y2;

	xsize = pcx->Width;
	ysize = pcx->Height;
	RectangleStruct bounds = { x, y, xsize, ysize };

	PCX::Instance->BlitToSurface(&bounds, DSurface::Composite, pcx);

	return true;
}

bool BannerClass::RenderSHP(int x, int y)
{
	if (!this->Type->Content_SHP.isset())
		return false;

	const auto pSHP = this->Type->Content_SHP.Get();
	if (!this->PAL)
	{
		if (this->Type->Content_SHP_DrawInShroud.Get())
		{
			if (this->Type->Content_PAL.GetConvert())
				this->PAL = this->Type->Content_PAL.GetConvert();
			else
				this->PAL = FileSystem::PALETTE_PAL;
		}
		else
		{
			const auto pPAL = FileSystem::LoadPALFile(this->Type->Content_PAL_Name.data(), DSurface::Composite);
			if (pPAL)
				this->PAL = pPAL;
			else
				this->PAL = FileSystem::PALETTE_PAL;
		}
	}

	if (!pSHP || !this->PAL)
	{
		return false;
	}

	bool timeover = false;
	if (this->Duration >= 0 && this->Initilize)
	{
		if (this->Duration > 0)
		{
			this->Duration--;
		}
		else
		{
			timeover = true;

			if (this->Type->Content_SHP_FadeOut.Get())
			{
				this->FadeLevel++;
				if (this->FadeLevel >= 4)
				{
					return false;
				}
			}
			else
			{
				return false;
			}
		}
	}

	if (this->Type->Content_SHP_FadeIn.Get())
	{
		if (!this->Initilize && !timeover)
		{
			if (this->FadeLevel > this->Type->Content_SHP_Transparency.Get())
			{
				if (this->FadeLevel_Rate > 0)
				{
					this->FadeLevel--;
					this->FadeLevel_Rate = 0;
				}
				else
				{
					this->FadeLevel_Rate++;
				}
			}
			else
			{
				this->Initilize = true;
			}
		}
	}
	else if (this->Type->Content_SHP_FadeOut.Get())
	{
		this->Initilize = true;

		if (this->Type->Content_SHP_Transparency.Get() > 0)
		{
			if (this->FadeLevel < this->Type->Content_SHP_Transparency.Get())
			{
				if (this->FadeLevel_Rate > 0)
				{
					this->FadeLevel++;
					this->FadeLevel_Rate = 0;
				}
				else
				{
					this->FadeLevel_Rate++;
				}
			}
			else
			{
				return false;
			}
		}
		else
		{
			this->FadeLevel++;
			if (this->FadeLevel >= 4)
			{
				return false;
			}
		}
	}

	if (!this->InPause)
	{
		if (this->Type->Content_SHP_Play.Get())
		{
			if (this->Rate >= 0)
			{
				if (this->Type->Content_SHP_Remove.Get())
				{
					if (this->FrameIdx >= this->Type->Content_SHP_End.Get(0))
					{
						if (this->Rate >= this->Type->Content_SHP_Rate.Get())
						{
							this->FrameIdx--;
							this->Rate = 0;
						}
						else
						{
							this->Rate++;
						}
					}
				}
				else
				{
					if (this->FrameIdx <= this->Type->Content_SHP_End.Get((pSHP->Frames - 1)))
					{
						if (this->Rate >= this->Type->Content_SHP_Rate.Get())
						{
							this->FrameIdx++;
							this->Rate = 0;
						}
						else
						{
							this->Rate++;
						}
					}
				}

				if (this->Rate == 0 && this->Type->Content_SHP_Sound.isset())
					VocClass::PlayGlobal(this->Type->Content_SHP_Sound.Get(), 0x2000, 1.0f);
			}
		}

		if (this->Type->Content_SHP_Remove.Get())
		{
			if (this->FrameIdx < this->Type->Content_SHP_End.Get(0))
			{
				this->Rate = -1;
				return false;
			}
		}
		else
		{
			if (this->FrameIdx > this->Type->Content_SHP_End.Get((pSHP->Frames - 1)))
			{
				if (this->Type->Content_SHP_Loop.Get())
					this->FrameIdx = 0;
				else
				{
					this->Rate = -1;

					if (this->Type->Content_SHP_OnlyOne.Get())
					{
						return false;
					}
					else
						this->FrameIdx = this->Type->Content_SHP_End.Get(pSHP->Frames - 1);
				}
			}
		}
	}

	auto nFlag = this->Type->Content_SHP_DrawInShroud.Get() ? BlitterFlags::None :
		BlitterFlags::Alpha;

	if (this->Type->Content_SHP_FullScreen.isset())
	{
		auto offset = this->Type->Content_SHP_FullScreen.Get();

		if (offset.X == -1)
			offset.X = pSHP->Width;

		if (offset.Y == -1)
			offset.Y = pSHP->Height;

		for (y = this->Type->Offset.Get().Y; y <= DSurface::ViewBounds->Height; y = y + offset.Y)
		{
			for (x = this->Type->Offset.Get().X; x <= DSurface::ViewBounds->Width; x = x + offset.X)
			{
				Point2D vPos = { x, y };
				int level = GetTranslucentLevel_MouseHover(vPos.X, vPos.Y, pSHP->Width, pSHP->Height, timeover);
				auto flag = EnumFunctions::GetTranslucentLevel(level);

				if (flag != BlitterFlags::None)
				{
					nFlag = nFlag | flag;
				}

				DSurface::Composite->DrawSHP(this->PAL, pSHP, this->FrameIdx, vPos, DSurface::ViewBounds,
					nFlag);
			}
		}
	}
	else
	{
		int x2 = 0;
		int y2 = 0;
		this->GetPos(x2, y2, pSHP->Width, pSHP->Height);

		x = x + x2;
		y = y + y2;
		Point2D vPos = { x, y };
		int level = GetTranslucentLevel_MouseHover(vPos.X, vPos.Y, pSHP->Width, pSHP->Height, timeover);
		auto flag = EnumFunctions::GetTranslucentLevel(level);

		if (flag != BlitterFlags::None)
		{
			nFlag = nFlag | flag;
		}

		DSurface::Composite->DrawSHP(this->PAL, pSHP, this->FrameIdx, vPos, DSurface::ViewBounds,
			nFlag);
	}

	return true;
}

bool BannerClass::RenderCSF(int x, int y)
{
	if (this->Type->Content_CSF.Get().empty())
	{
		return false;
	}

	if (this->Initilize)
	{
		if (this->Duration >= 0)
		{
			if (this->Duration > 0)
			{
				this->Duration--;
			}
			else
			{
				return false;
			}
		}
	}
	RectangleStruct rect;
	DSurface::Composite->GetRect(&rect);
	Point2D pos(x, y);

	std::wstring text = this->Type->Content_CSF.Get().Text;

	const auto& variables = ScenarioExt::Global()->Variables[this->IsGlobalVariable != 0];
	const auto& it = variables.find(this->Variable);

	if (it != variables.end())
		text += std::to_wstring(it->second.Value);

	TextPrintType textFlags = TextPrintType::UseGradPal
		| TextPrintType::Center
		| TextPrintType::Metal12
		| (this->Type->Content_CSF_DrawBackground.Get()
			? TextPrintType::Background
			: TextPrintType::LASTPOINT);

	Point2D loc { pos.X + 1,pos.Y + 1 };
	DSurface::Composite->DrawText
	(
		text.c_str(),
		rect,
		loc,
		Drawing::RGB_To_Int(this->Type->Content_CSF_ColorShadow.Get()),
		0,
		textFlags
	);

	DSurface::Composite->DrawText
	(
		text.c_str(),
		rect,
		pos,
		Drawing::RGB_To_Int(this->Type->Content_CSF_Color.Get()),
		0,
		textFlags
	);

	return true;
}

void BannerClass::Create(BannerTypeClass* pBannerType, int id, CoordStruct position, int variable, bool isGlobal)
{
	BannerClass::Array.push_back(std::make_unique<BannerClass>(pBannerType, id,
		position, variable, isGlobal));
}

void BannerClass::Delete(std::unique_ptr<BannerClass>& Banner)
{
	auto it = std::find(BannerClass::Array.begin(), BannerClass::Array.end(), Banner);
	if (it != BannerClass::Array.end())
	{
		Banner = nullptr;
		BannerClass::Array.erase(it);
	}
}

void BannerClass::Update()
{
	if (!BannerClass::Array.empty())
	{
		for (auto& pBanner : BannerClass::Array)
		{
			if (!pBanner)
			{
				BannerClass::Delete(pBanner);
				continue;
			}

			if (!pBanner->Type)
			{
				BannerClass::Delete(pBanner);
				continue;
			}

			if (!pBanner->Render())
			{
				BannerClass::Delete(pBanner);
				continue;
			}
		}
	}
}
