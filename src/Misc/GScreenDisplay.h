#pragma once
#include <vector>

#include <Helpers/Macro.h>
#include <Utilities/Container.h>
#include <Utilities/TemplateDef.h>

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

public:
	static void Add(ConvertClass* ShowAnimPAL, SHPStruct* ShowAnimSHP, Point2D posAnim, int translucentLevel, int frameKeep, int loopCount);
	static void UpdateAll();
};