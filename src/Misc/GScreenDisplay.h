#pragma once

#include <vector>

#include <GeneralStructures.h>

class ConvertClass;
struct SHPStruct;

class GScreenDisplay
{
private:

	struct Item
	{
		int CreationFrame;
		ConvertClass* AnimPal;
		SHPStruct* AnimSHP;
		Point2D Pos;
		int TranslucentLevel;
		int FrameKeep;
		int LoopCount;

		int FileFrames;
		int FrameCount;
		int CurrentFrameIndex;
		int Loop;
	};

	static std::vector<Item> Data;

	struct ItemSW
	{
		ConvertClass* AnimPal;
		SHPStruct* AnimSHP;
		int TranslucentLevel;
		int FrameKeep;
		int Index;

		int FileFrames;
		int FrameCount;
		int CurrentFrameIndex;
	};
	static std::vector<ItemSW> DataSW;

public:
	static void Add(ConvertClass* ShowAnimPAL, SHPStruct* ShowAnimSHP, Point2D posAnim, int translucentLevel, int frameKeep, int loopCount);
	static void UpdateAll();
	static void Add(ConvertClass* ShowAnimPAL, SHPStruct* ShowAnimSHP, int translucentLevel, int frameKeep, int index);
	static void UpdateSW();
};
