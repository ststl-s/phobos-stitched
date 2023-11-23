#pragma once

#include <map>
#include <set>

#include <Utilities/Template.h>

class ExternVariableClass
{
public:
	int id = 0;
	std::string Name = "";
	//float not use now
	bool IsFloatVar = false;
	int intValue = 0;
	//float not use now
	double floatValue = 0.0;
	std::string FromFile = "";

	static ValueableVector<std::unique_ptr<ExternVariableClass>> Array;
	static std::map<std::string, ExternVariableClass*> Mapper;
	static const std::string DefaultDir;

	static void Clear();
	static bool LoadGlobals(PhobosStreamReader& stm);
	static bool SaveGlobals(PhobosStreamWriter& stm);

	static int LoadVariablesFromDir(std::string path = "*.ini");
	static int LoadVariablesFromFile(std::string path, std::string filename, std::set<std::pair<std::string, std::string>>& ext);

	static ExternVariableClass* GetExternVariable(std::string Name);

	static void AddNewVar(std::string name, bool isFloatVar, int intValue, double floatValue, std::string path);
	static void SaveVariableToFile(const ExternVariableClass& var);
	//---------------------------------------------------------------------------------------------------------

	bool operator < (const ExternVariableClass& s)const;
	bool operator > (const ExternVariableClass& s)const;
	bool operator == (const ExternVariableClass& s)const;

	bool Load(PhobosStreamReader& stm, bool registerForChange);
	bool Save(PhobosStreamWriter& stm) const;

	ExternVariableClass() = default;
	ExternVariableClass(const char* name, const char* fromFile, bool isFloatVar = false, int intValue = 0, double floatValue = 0.0, int id = 0)
		: IsFloatVar(isFloatVar)
		, Name(name)
		, FromFile(fromFile)
		, intValue(intValue)
		, floatValue(floatValue)
		, id(id)
	{ }

	~ExternVariableClass() = default;
};
