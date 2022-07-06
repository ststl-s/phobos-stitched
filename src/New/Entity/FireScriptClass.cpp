#include "FireScriptClass.h"
#include <ScriptClass.h>

void FireScriptClass::ProcessScript(bool SelfCenter)
{
	CurrentFrame++;
	TotalFrame++;
	if (CurrentLine >= (int)Type->ScriptLines.size() || CurrentFrame < Type->ScriptLines[CurrentLine]->frame) return;
	auto Coord = Techno->Location;
	if (!SelfCenter) Coord = LocWhenFire;
	while (CurrentLine < (int)Type->ScriptLines.size() && 0 <= Type->ScriptLines[CurrentLine]->frame && Type->ScriptLines[CurrentLine]->frame <= CurrentFrame)
	{
		auto pLine = Type->ScriptLines[CurrentLine];
		CoordStruct tmp = { Coord.X + pLine->Offset.X, Coord.Y + pLine->Offset.Y ,Coord.Z };
		WeaponTypeExt::DetonateAt(pLine->Weapon, tmp, Techno);
		CurrentLine++;
	}
	while (CurrentLine < (int)Type->ScriptLines.size() && Type->ScriptLines[CurrentLine]->frame < 0)
	{
		auto pLine = Type->ScriptLines[CurrentLine];
		if (NowCycleTime.empty() || NowCycleTime.back().X != CurrentLine)
		{
			NowCycleTime.push_back({ CurrentLine, 1 });
			CurrentLine = -pLine->frame - 1;
			CurrentFrame = pLine->Offset.X;
		}
		else if (NowCycleTime.back().X == CurrentLine && NowCycleTime.back().Y < pLine->Offset.Y)
		{
			NowCycleTime.back().Y++;
			CurrentLine = -pLine->frame - 1;
			CurrentFrame = pLine->Offset.X;
		}
		else
		{
			NowCycleTime.pop_back();
			CurrentLine += 2;
		}
		while (CurrentLine < (int)Type->ScriptLines.size() && 0 <= Type->ScriptLines[CurrentLine]->frame && Type->ScriptLines[CurrentLine]->frame <= CurrentFrame)
		{
			auto _pLine = Type->ScriptLines[CurrentLine];
			CoordStruct tmp = { Coord.X + _pLine->Offset.X ,Coord.Y + _pLine->Offset.Y ,Coord.Z };
			WeaponTypeExt::DetonateAt(_pLine->Weapon, tmp, Techno);
			CurrentLine++;
		}
	}
}

template <typename T>
bool FireScriptClass::Serialize(T& Stm)
{
	return Stm
		.Process(this->Type)
		.Process(this->Techno)
		.Process(this->CurrentFrame)
		.Process(this->CurrentLine)
		.Process(this->TotalFrame)
		.Process(this->NowCycleTime)
		.Process(this->LocWhenFire)
		.Success();
}

bool FireScriptClass::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return Serialize(Stm);
}

bool FireScriptClass::Save(PhobosStreamWriter& Stm) const
{
	return const_cast<FireScriptClass*>(this)->Serialize(Stm);
}