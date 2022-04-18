#pragma once
#include <map>
#include <vector>
#include <Ext/Techno/Body.h>

class PointerMapper
{
public:
	static std::map<long, long> Map;

	static void AddMapping(long was, long is);
};

//???why this class here
class ExistTechnoExt
{
public:
	static std::vector<TechnoExt::ExtData*> Array;
};

class ExistTechnoTypeExt
{
public:
	static std::vector<TechnoTypeExt::ExtData*> Array;
};