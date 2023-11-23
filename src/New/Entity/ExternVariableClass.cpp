#include "ExternVariableClass.h"

#include <cstdlib>
#include <fstream>
#include <io.h>
#include <string>

#include <Utilities/TemplateDef.h>

ValueableVector<std::unique_ptr<ExternVariableClass>> ExternVariableClass::Array;
const std::string ExternVariableClass::DefaultDir = std::string("\\Extern\\");
std::map<std::string, ExternVariableClass*> ExternVariableClass::Mapper;
//this maybe can use a better logic like custom dir after

int ExternVariableClass::LoadVariablesFromDir(std::string path)
{
	Array.clear();
	std::set<std::pair<std::string, std::string>> ext;
	intptr_t Handle = -1;
	_finddata_t finder;
	char cwd[_MAX_PATH];
	GetCurrentDirectory(sizeof(cwd), cwd);
	Handle = _findfirst((cwd + DefaultDir + path).c_str(), &finder);
	Debug::Log("[ExternVar::Info] Find ini in \"%s\"\n", (cwd + DefaultDir + path).c_str());
	if (Handle == -1)
	{
		Debug::Log("[ExternVar::Info] Empty dir: %s\n", (cwd + DefaultDir + path).c_str());
		return 0;
	}
	int cnt = 0;
	do
	{
		if (finder.attrib == FILE_ATTRIBUTE_DIRECTORY) continue;
		Debug::Log("[ExternVar::Info] Find a file \"%s\"\n", finder.name);
		LoadVariablesFromFile(cwd + DefaultDir + finder.name, finder.name, ext);
		cnt++;
	}
	while (_findnext(Handle, &finder) == 0);
	return cnt;
}

int ExternVariableClass::LoadVariablesFromFile(std::string path, std::string filename, std::set<std::pair<std::string, std::string>>& ext)
{
	//Debug::Log("[ExternVar::Info] Start find vars from \"%s\"\n", Path.c_str());
	std::ifstream fin(path);
	std::string info;
	int cnt = 0;
	while (std::getline(fin, info))
	{
		if (info == "") continue;
		//Debug::Log("[ExternVar::Info] Read a line \"%s\"\n", info.c_str());
		int p = 0;
		while (p < (int)info.length())
		{
			if (info[p] == ';' || info[p] == '#')
			{
				info = info.substr(0, p);
				break;
			}
			p++;
		}
		if (info == "") continue;
		int num = std::count(info.begin(), info.end(), '=');
		if (info.length() < 3 || num != 1)
		{
			Debug::Log("[ExternVar::Error] Invalid Line: \"%s\"\n", info.c_str());
			continue;
		}
		p = 0;
		while (p < (int)info.length() && info[p] != '=') p++;
		if (p >= 32)
		{
			Debug::Log("[ExternVar::Error] Invalid name \"length = %d > 31\"\n", p);
			continue;
		}
		std::string name = info.substr(0, p);
		std::string subinfo = info.substr(p + 1);
		if (subinfo.empty())
		{
			Debug::Log("[ExternVar::Error] Invalid empty value\n");
			continue;
		}
		int numofpoint = std::count(subinfo.begin(), subinfo.end(), '.');
		bool IsFloatVar = false;
		if (numofpoint == 0) IsFloatVar = false;
		else if (numofpoint == 1) IsFloatVar = true;
		else
		{
			Debug::Log("[ExternVar::Error] Invalid value: \"%s\"\n", subinfo.c_str());
			continue;
		}

		if (ext.count(std::make_pair(name, path)))
		{
			Debug::Log("[ExternVar::Warning] Vars with same name in a same file: \"%s\" [%s]\n", name.c_str(), filename.c_str());
			continue;
		}
		ext.emplace(name, path);
		char cname[0x20];
		char cfilename[0x20];
		strcpy_s(cname, name.c_str());
		strcpy_s(cfilename, filename.c_str());
		if (!subinfo.empty() && subinfo[0] == '.') subinfo = '0' + subinfo;
		if (IsFloatVar)
		{
			double floatValue = atof(subinfo.c_str());
			Array.emplace_back(std::make_unique<ExternVariableClass>(cname, cfilename, true, 0, floatValue, (int)Array.size()));
			//Debug::Log("[ExternVar::Info] Read a externvar: Name[%s],Value[%lf],FromFile[%s]\n", cname, floatValue, cpath);
		}
		else
		{
			int intValue = atoi(subinfo.c_str());
			Array.emplace_back(std::make_unique<ExternVariableClass>(cname, cfilename, false, intValue, 0, (int)Array.size()));
			//Debug::Log("[ExternVar::Info] Read a externvar: Name[%s],Value[%d],FromFile[%s]\n", cname, intValue, cpath);
		}
		Mapper[filename + ":" + name] = Array.back().get();
		cnt++;
	}
	return cnt;
}

ExternVariableClass* ExternVariableClass::GetExternVariable(std::string name)
{
	if (Mapper.count(name))
		return Mapper[name];
	Debug::Log("[ExternVar::Error] Can't find externvar[%s]", name.c_str());
	return nullptr;
}

void ExternVariableClass::AddNewVar(std::string name, bool isFloatVar, int intValue, double floatValue, std::string filename)
{
	bool Exist = false;
	if (Mapper.count(name))
	{
		Exist = true;
		ExternVariableClass* pExtVar = Mapper[name];
		pExtVar->IsFloatVar = isFloatVar;
		pExtVar->intValue = intValue;
		pExtVar->floatValue = floatValue;
		SaveVariableToFile(*pExtVar);
	}
	if (!Exist)
	{
		if (filename == "")
		{
			Debug::Log("[ExternVar::Error] Extern Var [%s] not exist and Filename is empty\n", name.c_str());
			return;
		}
		Array.emplace_back(std::make_unique<ExternVariableClass>(name.c_str(), filename.c_str(), isFloatVar, intValue, floatValue));
		Mapper[name + filename] = Array.back().get();
		SaveVariableToFile(*Array.back());
	}
}

void ExternVariableClass::SaveVariableToFile(const ExternVariableClass& var)
{
	//fstream can't replace, maybe could be better
	char cwd[_MAX_PATH];
	GetCurrentDirectory(sizeof(cwd), cwd);

	std::ofstream fout(cwd + DefaultDir + var.FromFile, std::ios::out);
	Debug::Log("[Path] {%s}\n", (cwd + DefaultDir + var.FromFile).c_str());
	int cnt = 0;
	for (const auto& it : Array)
	{
		if (it->FromFile != var.FromFile)
			continue;

		fout << it->Name << "=";

		if (var.IsFloatVar)
			fout << std::fixed << it->floatValue;
		else
			fout << it->intValue;

		fout << '\n';
		cnt++;
	}
	//Debug::Log("[ExternVar::Info] Saved %d vars\n", cnt);
}

bool ExternVariableClass::operator < (const ExternVariableClass& s)const
{
	return std::string(Name) + FromFile < std::string(s.Name) + s.FromFile;
}

bool ExternVariableClass::operator > (const ExternVariableClass& s)const
{
	return std::string(Name) + FromFile > std::string(s.Name) + s.FromFile;
}

bool ExternVariableClass::operator == (const ExternVariableClass& s)const
{
	return std::string(Name) + FromFile == std::string(s.Name) + s.FromFile;
}

bool ExternVariableClass::Load(PhobosStreamReader& stm, bool registerForChange)
{
	return stm
		.Process(id)
		.Process(Name)
		.Process(IsFloatVar)
		.Process(intValue)
		.Process(floatValue)
		.Process(FromFile)
		.Success();
}

bool ExternVariableClass::Save(PhobosStreamWriter& stm) const
{
	return stm
		.Process(id)
		.Process(Name)
		.Process(IsFloatVar)
		.Process(intValue)
		.Process(floatValue)
		.Process(FromFile)
		.Success();
}

void ExternVariableClass::Clear()
{
	Array.clear();
	Mapper.clear();
}

bool ExternVariableClass::LoadGlobals(PhobosStreamReader& stm)
{
	return stm
		.Process(Array)
		.Process(Mapper)
		.Success();
}

bool ExternVariableClass::SaveGlobals(PhobosStreamWriter& stm)
{
	return stm
		.Process(Array)
		.Process(Mapper)
		.Success();
}
