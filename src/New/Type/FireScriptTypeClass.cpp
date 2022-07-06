#include "FireScriptTypeClass.h"

#include <io.h>
#include <cstdlib>
#include <fstream>

ValueableVector<FireScriptTypeClass*> FireScriptTypeClass::ScriptArray;
const std::string FireScriptTypeClass::DefaultDir = "/Script/";

void FireScriptTypeClass::LoadFromDir(std::string ExtName)
{
	ScriptArray.clear();
	intptr_t Handle = -1;
	_finddata_t finder;
	char cwd[260];
	GetCurrentDirectory(260, cwd);
	std::string Tmp = cwd + DefaultDir + ExtName;
	Handle = _findfirst(Tmp.c_str(), &finder);
	Debug::Log("[FireScript::Info] Find ini in \"%s\"\n", Tmp.c_str());
	if (Handle == -1)
	{
		Debug::Log("[FireScript::Info] Empty dir: %s\n", Tmp.c_str());
		return;
	}
	int cnt = 0;
	do
	{
		if (finder.attrib == FILE_ATTRIBUTE_DIRECTORY) continue;
		//Debug::Log("[FireScript::Info] Find a file \"%s\"\n", finder.name);
		LoadFromFile(cwd + DefaultDir + finder.name, finder.name);
		cnt++;
	}
	while (_findnext(Handle, &finder) == 0);
	std::sort(ScriptArray.begin(), ScriptArray.end());
	//for (const auto& it : ScriptArray)
	//{
		//Debug::Log("[FireScript::Info] Mapped string: \"%s\":[0x%X]\n", it->FileName, it);
		//ScriptMap[std::string(it->FileName)] = it;
	//}
	Debug::Log("[FireScript::Info] Find %d Scripts \n", cnt);
}

void FireScriptTypeClass::LoadFromFile(std::string Path, std::string FileName)
{
	std::ifstream fin(Path);
	auto newScript = new FireScriptTypeClass();
	std::string line;
	int cnt = 0;
	while (std::getline(fin, line))
	{
		if (line.empty()) continue;
		int p = 0;
		while (p < (int)line.length() && line[p] != ';' && line[p] != '#') p++;
		if (p <= 0) continue;
		if (p < (int)line.length()) line = line.substr(0, p - 1);
		int symbolnum = std::count(line.begin(), line.end(), ',');
		if (symbolnum != 3)
		{
			Debug::Log("[FireScript::Error] Invalid Line %d: \"%s\"\n", line.c_str());
			continue;
		}
		p = 0;
		int num = 0;
		bool failed = false;
		while (p < (int)line.length())
		{
			if (line[p] == ',')
			{
				std::string tmp = line.substr(0, p);
				if (tmp.empty())
				{
					failed = true;
					Debug::Log("[FireScript::Error] Empty frame line %d\n", cnt);
					break;
				}
				num = atoi(tmp.c_str());
				break;
			}
			p++;
		}
		if (failed) continue;
		line = line.substr(p + 1);
		if (num < 0)
		{
			p = 0;
			int rtf = 0;
			while (p < (int)line.length())
			{
				if (line[p] == ',')
				{
					std::string tmp = line.substr(0, p);
					if (tmp.empty())
					{
						failed = true;
						Debug::Log("[FireScript::Error] Empty line_number line %d\n", cnt);
						break;
					}
					rtf = atoi(tmp.c_str());
					break;
				}
				p++;
			}
			if (failed) continue;
			line = line.substr(p + 1);
			p = 0;
			int looptime = 0;
			while (p < (int)line.length())
			{
				if (line[p] == ',')
				{
					std::string tmp = line.substr(0, p);
					if (tmp.empty())
					{
						failed = true;
						Debug::Log("[FireScript::Error] Empty loop_time line %d\n", cnt);
						break;
					}
					looptime = atoi(tmp.c_str());
					break;
				}
				p++;
			}
			if (failed) continue;
			auto newLine = new FireScriptLines(num, nullptr, rtf, looptime);
			newScript->ScriptLines.emplace_back(newLine);
			continue;
		}
		WeaponTypeClass* Weapon = nullptr;
		p = 0;
		while (p < (int)line.length())
		{
			if (line[p] == ',')
			{
				std::string tmp = line.substr(0, p);
				if (tmp.empty())
				{
					failed = true;
					Debug::Log("[FireScript::Error] Empty Weapon line %d\n", cnt);
					break;
				}
				Weapon = WeaponTypeClass::Find(tmp.c_str());
				if (Weapon == nullptr)
				{
					failed = true;
					Debug::Log("[FireScript::Error] Weapon not found [%s] in line %d\n", tmp.c_str(), cnt);
					break;
				}
				break;
			}
			p++;
		}
		if (failed) continue;
		int offsetx = 0;
		line = line.substr(p + 1);
		p = 0;
		while (p < (int)line.length())
		{
			if (line[p] == ',')
			{
				std::string tmp = line.substr(0, p);
				if (tmp.empty())
				{
					failed = true;
					Debug::Log("[FireScript::Error] Empty offset.x line %d\n", cnt);
					break;
				}
				offsetx = atoi(tmp.c_str());
				break;
			}
			p++;
		}
		line = line.substr(p + 1);
		if (line.empty())
		{
			Debug::Log("[FireScript::Error] Empty offset.y line %d\n", cnt);
			continue;
		}
		int offsety = atoi(line.c_str());
		auto newLine = new FireScriptLines(num, Weapon, offsetx, offsety);
		newScript->ScriptLines.emplace_back(newLine);
		cnt++;
	}
	if (cnt == 0)
	{
		Debug::Log("[FireScript::Error] Script file with zero valid lines \"%s\"\n", FileName.c_str());
		return;
	}
	Debug::Log("[FireScript::Info] Load a script file \"%s\" with %d lines Size[%u]\n", FileName.c_str(), cnt, newScript->ScriptLines.size());
	strcpy_s(newScript->FileName, FileName.c_str());
	ScriptArray.emplace_back(newScript);
}

bool FireScriptTypeClass::operator < (const FireScriptTypeClass& s)const
{
	return strcmp(FileName, s.FileName) < 0;
}

bool FireScriptTypeClass::operator > (const FireScriptTypeClass& s)const
{
	return strcmp(FileName, s.FileName) > 0;
}

bool FireScriptTypeClass::operator == (const FireScriptTypeClass& s)const
{
	return strcmp(FileName, s.FileName) == 0;
}

FireScriptTypeClass* FireScriptTypeClass::GetScript(const char* Name)
{
	if (ScriptArray.empty()) LoadFromDir();
	for (auto& it : ScriptArray)
	{
		if (strcmp(it->FileName, Name) == 0)
			return it;
	}
	return nullptr;
}

template <typename T>
void FireScriptTypeClass::Serialize(T& Stm)
{
	Stm
		.Process(this->FileName)
		.Process(this->ScriptLines)
		;
}

void FireScriptTypeClass::Load(PhobosStreamReader& Stm)
{
	this->Serialize(Stm);
}

void FireScriptTypeClass::Save(PhobosStreamWriter& Stm)
{
	this->Serialize(Stm);
}

template <typename T>
void FireScriptLines::Serialize(T& Stm)
{
	Stm
		.Process(this->frame)
		.Process(this->Weapon)
		.Process(this->Offset)
		;
}

void FireScriptLines::Load(PhobosStreamReader& Stm)
{
	this->Serialize(Stm);
}

void FireScriptLines::Save(PhobosStreamWriter& Stm)
{
	this->Serialize(Stm);
}