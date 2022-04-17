#pragma once
#include <GeneralStructures.h>

#include <Utilities/SavegameDef.h>
#include <Utilities/Constructs.h>
#include <Utilities/Template.h>
#include <Helpers/Template.h>
#include <Utilities/Enum.h>

#include<set>

class ExternVariableClass
{
public:
	int id;
	char Name[0x20];
	//float not use now
	bool IsFloatVar;
	int intValue;
	//float not use now
	double floatValue;
	char FromFile[260];

	static ValueableVector<ExternVariableClass*> Array;
	static const std::string DefaultDir;

	static int LoadVariablesFromDir(std::string Path = "*.ini");
	static int LoadVariablesFromFile(std::string Path, std::set<std::pair<std::string, std::string>>& ext);

	//this function don't use name and fromfile as parameter because stupid Action's parameter
	static ExternVariableClass* GetExternVariable(int id);

	//these function unused because stupid Action's parameter, both TAction::Value and TAction::Param are int
	//westwood don't know the function named atoi which declared in stdlib.h?
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

	~ExternVariableClass() { }

	void InvalidatePointer(void* ptr) { };
	bool Load(PhobosStreamReader& Stm, bool RegisterForChange);
	bool Save(PhobosStreamWriter& Stm) const;
private:
	template <typename T>
	bool Serialize(T& Stm);
};