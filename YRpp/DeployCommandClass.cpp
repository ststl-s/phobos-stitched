#include "CommandClass.h"

class DeployCommandClass : CommandClass
{
	//CommandClass
	virtual ~DeployCommandClass() = default;
	virtual const char* GetName() const = 0;
	virtual const wchar_t* GetUIName() const = 0;
	virtual const wchar_t* GetUICategory() const = 0;
	virtual const wchar_t* GetUIDescription() const = 0;

	virtual bool PreventCombinationOverride(WWKey eInput) const // Do we need to check extra value like SHIFT?
	{ return false; }										// If this value is true, the game won't process
	// Combination keys written here
	// e.g. To ignore SHIFT + this key
	// return eInput & WWKey::Shift;

	virtual bool ExtraTriggerCondition(WWKey eInput) const // Only with this key set to true will the game call the Execute
	{ return !(eInput & WWKey::Release); }

	virtual bool CheckLoop55E020(WWKey eInput) const // Stupid loop, I don't know what's it used for
	{ return false; }

	virtual void Execute(WWKey eInput) const = 0;

	void Execute() const JMP_THIS(0x730AF0);
};
