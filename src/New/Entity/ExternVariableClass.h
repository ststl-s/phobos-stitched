#pragma once

#include <map>
#include <set>

#include <Utilities/Template.h>

class ExternVariableClass
{
public:
	int id = 0;
	char Name[0x20] = "";
	//float not use now
	bool IsFloatVar = false;
	int intValue = 0;
	//float not use now
	double floatValue = 0.0;
	char FromFile[0x20] = "";

	static ValueableVector<ExternVariableClass*> Array;
	static std::map<std::string, ExternVariableClass*> Mapper;
	static const std::string DefaultDir;

	static int LoadVariablesFromDir(std::string Path = "*.ini");
	static int LoadVariablesFromFile(std::string Path, std::string Filename, std::set<std::pair<std::string, std::string>>& ext);

	static ExternVariableClass* GetExternVariable(std::string Name);

	static void AddNewVar(std::string Name, bool IsFloatVar, int intValue, double floatValue, std::string Path);
	static void SaveVariableToFile(const ExternVariableClass& var);
	//---------------------------------------------------------------------------------------------------------

	bool operator < (const ExternVariableClass& s)const;
	bool operator > (const ExternVariableClass& s)const;
	bool operator == (const ExternVariableClass& s)const;

	ExternVariableClass(const char* Name, const char* FromFile, bool IsFloatVar = false, int intValue = 0, double floatValue = 0.0, int id = 0)
		:IsFloatVar(IsFloatVar), intValue(intValue), floatValue(floatValue), id(id)
	{
		strcpy_s(this->Name, Name);
		strcpy_s(this->FromFile, FromFile);
	}

	~ExternVariableClass() = default;
};
