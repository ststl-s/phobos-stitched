#include "PointerMapper.h"

std::map<long, long> PointerMapper::Map;
std::vector<TechnoExt::ExtData*> ExistTechnoExt::Array;
std::vector<TechnoTypeExt::ExtData*> ExistTechnoTypeExt::Array;

void PointerMapper::AddMapping(long was, long is)
{
	Map[was] = is;
}