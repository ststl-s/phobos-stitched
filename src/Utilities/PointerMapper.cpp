#include "PointerMapper.h"

#include <Syringe.h>
#include <Helpers/Macro.h>
#include <Utilities/Debug.h>

std::map<long, long> PointerMapper::Map;

void PointerMapper::AddMapping(void* was, void* is)
{
	if (was == nullptr || is == nullptr)
		return;

	/*
	if (Exist(was))
		Debug::Log("[PhobosPointerMapper] Warning: [0x%X] has changed both to [0x%X] and [0x%X]!\n", was, is, Mapping(was));
	*/
	Map[reinterpret_cast<long>(was)] = reinterpret_cast<long>(is);
}

void* PointerMapper::Mapping(void* was)
{
	if (Map.count(reinterpret_cast<long>(was)))
		return reinterpret_cast<void*>(Map[reinterpret_cast<long>(was)]);
	return nullptr;
}

bool PointerMapper::Exist(void* was)
{
	return Map.find(reinterpret_cast<long>(was)) != Map.end();
}

DEFINE_HOOK(0x6CF316, SwizzleManagerClass_Here_I_Am, 0x4)
{
	GET_STACK(void*, oldP, 0x8);
	GET_STACK(void*, newP, 0xC);
	PointerMapper::AddMapping(oldP, newP);
	return 0;
}