#include "ExternVariableClass.h"

#include<io.h>
#include<fstream>
#include<cstdlib>

ValueableVector<ExternVariableClass*> ExternVariableClass::Array;
const std::string ExternVariableClass::DefaultDir = std::string("\\Extern\\");
std::map<std::string, ExternVariableClass*> ExternVariableClass::Mapper;
//this maybe can use a better logic like custom dir after

int ExternVariableClass::LoadVariablesFromDir(std::string Path)
{
	Array.clear();
	std::set<std::pair<std::string, std::string>> ext;
	intptr_t Handle = -1;
	_finddata_t finder;
	char cwd[260];
	GetCurrentDirectory(260, cwd);
	Handle = _findfirst((cwd + DefaultDir + Path).c_str(), &finder);
	Debug::Log("[ExternVar::Info] Find ini in \"%s\"\n", (cwd + DefaultDir + Path).c_str());
	if (Handle == -1)
	{
		Debug::Log("[ExternVar::Info] Empty dir: %s\n", (cwd + DefaultDir + Path).c_str());
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

int ExternVariableClass::LoadVariablesFromFile(std::string Path, std::string Filename, std::set<std::pair<std::string, std::string>>& ext)
{
	//Debug::Log("[ExternVar::Info] Start find vars from \"%s\"\n", Path.c_str());
	std::ifstream fin(Path);
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

		if (ext.count(std::make_pair(name, Path)))
		{
			Debug::Log("[ExternVar::Warning] Vars with same name in a same file: \"%s\" [%s]\n", name.c_str(), Filename.c_str());
			continue;
		}
		ext.emplace(name, Path);
		char cname[0x20];
		char cfilename[0x20];
		strcpy_s(cname, name.c_str());
		strcpy_s(cfilename, Filename.c_str());
		if (!subinfo.empty() && subinfo[0] == '.') subinfo = '0' + subinfo;
		if (IsFloatVar)
		{
			double floatValue = atof(subinfo.c_str());
			auto pExtVar = new ExternVariableClass(cname, cfilename, true, 0, floatValue, (int)Array.size());
			Array.emplace_back(pExtVar);
			//Debug::Log("[ExternVar::Info] Read a externvar: Name[%s],Value[%lf],FromFile[%s]\n", cname, floatValue, cpath);
		}
		else
		{
			int intValue = atoi(subinfo.c_str());
			auto pExtVar = new ExternVariableClass(cname, cfilename, false, intValue, 0, (int)Array.size());
			Array.emplace_back(pExtVar);
			//Debug::Log("[ExternVar::Info] Read a externvar: Name[%s],Value[%d],FromFile[%s]\n", cname, intValue, cpath);
		}
		Mapper[Filename + ":" + name] = Array.back();
		cnt++;
	}
	return cnt;
}

ExternVariableClass* ExternVariableClass::GetExternVariable(std::string Name)
{
	if (Mapper.count(Name))
		return Mapper[Name];
	Debug::Log("[ExternVar::Error] Can't find externvar[%s]", Name.c_str());
	return nullptr;
}

void ExternVariableClass::AddNewVar(std::string Name, bool IsFloatVar, int intValue, double floatValue, std::string Filename)
{
	bool Exist = false;
	if (Mapper.count(Name))
	{
		Exist = true;
		ExternVariableClass* pExtVar = Mapper[Name];
		pExtVar->IsFloatVar = IsFloatVar;
		pExtVar->intValue = intValue;
		pExtVar->floatValue = floatValue;
		SaveVariableToFile(*pExtVar);
	}
	if (!Exist)
	{
		if (Filename == "")
		{
			Debug::Log("[ExternVar::Error] Extern Var [%s] not exist and Filename is empty\n", Name.c_str());
			return;
		}
		Array.emplace_back(new ExternVariableClass(Name.c_str(), Filename.c_str(), IsFloatVar, intValue, floatValue));
		Mapper[Name + Filename] = Array.back();
		SaveVariableToFile(*Array.back());
	}
}

void ExternVariableClass::SaveVariableToFile(const ExternVariableClass& var)
{
	//fstream can't replace, maybe could be better
	char cwd[260];
	GetCurrentDirectory(260, cwd);

	std::ofstream fout(cwd + DefaultDir + var.FromFile, std::ios::out);
	Debug::Log("[Path] {%s}\n", (cwd + DefaultDir + var.FromFile).c_str());
	int cnt = 0;
	for (const auto& it : Array)
	{
		if (strcmp(it->FromFile, var.FromFile) != 0) continue;
		fout << it->Name << "=";
		if (var.IsFloatVar) fout << std::fixed << it->floatValue;
		else fout << it->intValue;
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

template <typename T>
bool ExternVariableClass::Serialize(T& Stm)
{
	return Stm
		//.Process(this->ID)
		//.Process(this->Name)
		//.Process(this->IsFloatVar)
		//.Process(this->intValue)
		//.Process(this->floatValue)
		//.Process(this->FromFile)
		.Success()
		;
}

bool ExternVariableClass::Load(PhobosStreamReader& Stm, bool RegisterForChange)
{
	return Serialize(Stm);
}

bool ExternVariableClass::Save(PhobosStreamWriter& Stm) const
{
	return const_cast<ExternVariableClass*>(this)->Serialize(Stm);
}