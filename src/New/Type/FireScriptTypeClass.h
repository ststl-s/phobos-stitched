#pragma once

#include <GeneralStructures.h>
#include <LaserDrawClass.h>
#include <HouseClass.h>
#include <Utilities/TemplateDef.h>

#include <Ext/WeaponType/Body.h>

#include <vector>
#include <map>

class FireScriptLines
{
public:
	int frame;
	WeaponTypeClass* Weapon;
	Vector2D<int> Offset;

	FireScriptLines(int frame, WeaponTypeClass* Weapon, Vector2D<int> Offset)
		:frame(frame), Weapon(Weapon), Offset(Offset)
	{
	}
	FireScriptLines(int frame, WeaponTypeClass* Weapon, int Offsetx, int Offsety)
		:frame(frame), Weapon(Weapon)
	{
		Offset.X = Offsetx;
		Offset.Y = Offsety;
	}
	~FireScriptLines() { }

	void InvalidatePointer(void* ptr) { };

	void Load(PhobosStreamReader& Stm);
	void Save(PhobosStreamWriter& Stm);

private:
	template <typename T>
	void Serialize(T& Stm);
};

class FireScriptTypeClass
{
public:
	static ValueableVector<FireScriptTypeClass*> ScriptArray;
	const static std::string DefaultDir;
	ValueableVector<FireScriptLines*> ScriptLines;
	char FileName[0x20];

	FireScriptTypeClass() { }
	~FireScriptTypeClass() { }

	bool operator < (const FireScriptTypeClass& s)const;
	bool operator > (const FireScriptTypeClass& s)const;
	bool operator == (const FireScriptTypeClass& s)const;

	static void LoadFromDir(std::string Name = "*.ini");
	static void LoadFromFile(std::string Path, std::string FileName);
	static FireScriptTypeClass* GetScript(const char* FileName);

	void InvalidatePointer(void* ptr) { };

	void Load(PhobosStreamReader& Stm);
	void Save(PhobosStreamWriter& Stm);

private:
	template <typename T>
	void Serialize(T& Stm);
};