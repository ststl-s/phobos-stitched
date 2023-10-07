#include "GScreenDisplay.h"
#include <Drawing.h>
#include <Utilities/EnumFunctions.h>
#include <WWMouseClass.h>

std::vector<GScreenDisplay::Item> GScreenDisplay::Data;
std::vector<GScreenDisplay::ItemSW> GScreenDisplay::DataSW;

void GScreenDisplay::Add(ConvertClass* ShowAnimPAL, SHPStruct* ShowAnimSHP, Point2D posAnim, int translucentLevel, int frameKeep, int loopCount)
{
	Item item {};

	item.CreationFrame = Unsorted::CurrentFrame;

	item.AnimPal = ShowAnimPAL;
	item.AnimSHP = ShowAnimSHP;
	item.Pos = posAnim;
	item.TranslucentLevel = translucentLevel;
	item.FrameKeep = frameKeep;
	item.LoopCount = loopCount;

	item.FileFrames = ShowAnimSHP->Frames;
	item.FrameCount = 0;
	item.CurrentFrameIndex = 0;
	item.Loop = 0;

	Data.push_back(item);
}

void GScreenDisplay::Add(ConvertClass* ShowAnimPAL, SHPStruct* ShowAnimSHP, int translucentLevel, int frameKeep, int index)
{
	ItemSW item{};

	item.AnimPal = ShowAnimPAL;
	item.AnimSHP = ShowAnimSHP;
	item.TranslucentLevel = translucentLevel;
	item.FrameKeep = frameKeep;
	item.Index = index;

	item.FileFrames = ShowAnimSHP->Frames;
	item.FrameCount = 0;
	item.CurrentFrameIndex = 0;

	DataSW.push_back(item);
}

void GScreenDisplay::UpdateAll()
{
	if (Data.empty())
		return;

	for (int i = Data.size() - 1; i >= 0; --i)
	{
		auto& dataItem = Data[i];
		if (Unsorted::CurrentFrame > dataItem.CreationFrame)
		{
			// 透明度
			auto const flags = BlitterFlags::None | EnumFunctions::GetTranslucentLevel(dataItem.TranslucentLevel);
			// 绘制
			DSurface::Composite->DrawSHP
			(
				dataItem.AnimPal,
				dataItem.AnimSHP,
				dataItem.CurrentFrameIndex,
				dataItem.Pos,
				DSurface::ViewBounds,
				flags
			);

			dataItem.FrameCount++; // 游戏实际帧数计数器
			if (dataItem.FrameCount >= dataItem.FrameKeep) // 达到设定的FrameKeep，则下次换帧播放
			{
				dataItem.CurrentFrameIndex++; // 帧序号加1
				if (dataItem.CurrentFrameIndex >= dataItem.FileFrames) // 帧序号溢出则回到0号帧
				{
					dataItem.CurrentFrameIndex = 0;
					dataItem.Loop++; // 每次回到0号帧时，循环计数加1
				}
				dataItem.FrameCount = 0; // 每次换帧时，游戏实际帧数计数器归零
			}
		}
		if (dataItem.Loop == dataItem.LoopCount || Unsorted::CurrentFrame < dataItem.CreationFrame)
			Data.erase(Data.begin() + i);
	}
}

void GScreenDisplay::UpdateSW()
{
	if (DataSW.empty())
		return;

	int curIdx = Unsorted::CurrentSWType;
	if (curIdx == -1)
	{
		for (auto item : DataSW)
		{
			item.CurrentFrameIndex = 0;
		}
		return;
	}

	for (int i = DataSW.size() - 1; i >= 0; --i)
	{
		auto& dataItem = DataSW[i];

		if (dataItem.Index == curIdx)
		{
			Point2D pos = { WWMouseClass::Instance->GetX(), WWMouseClass::Instance->GetY() };
			Point2D modify = { dataItem.AnimSHP->Width >> 1, dataItem.AnimSHP->Height >> 1 };
			pos -= modify;

			auto const flags = BlitterFlags::None | EnumFunctions::GetTranslucentLevel(dataItem.TranslucentLevel);
			// 绘制
			DSurface::Composite->DrawSHP
			(
				dataItem.AnimPal,
				dataItem.AnimSHP,
				dataItem.CurrentFrameIndex,
				pos,
				DSurface::ViewBounds,
				flags
			);

			dataItem.FrameCount++; // 游戏实际帧数计数器
			if (dataItem.FrameCount >= dataItem.FrameKeep) // 达到设定的FrameKeep，则下次换帧播放
			{
				dataItem.CurrentFrameIndex++; // 帧序号加1
				if (dataItem.CurrentFrameIndex >= dataItem.FileFrames) // 帧序号溢出则回到0号帧
				{
					dataItem.CurrentFrameIndex = 0;
				}
				dataItem.FrameCount = 0; // 每次换帧时，游戏实际帧数计数器归零
			}
		}
	}
}

