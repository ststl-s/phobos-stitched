#pragma once

// these classes handle alliances between players, eg Team:A B C D in the frontend

class MPTeam
{
public:
	//Destructor
	virtual ~MPTeam() JMP_THIS(0x5D8D50);
	virtual bool IsTeamIncluded(int idx) JMP_THIS(0x5D8C90);
	virtual bool SetPlayerTeam(int idxPlayer) JMP_THIS(0x5D8CB0);

	void AddToList(HWND hWnd)
		{ JMP_THIS(0x5D8D10); }

protected:
	//Constructor
	MPTeam(wchar_t **title, int idx)
		{ JMP_THIS(0x5D8C50); }

	MPTeam(noinit_t)
	{ }


	//===========================================================================
	//===== Properties ==========================================================
	//===========================================================================

public:

	wchar_t* Title;
	int Index;
};

class MPCombatTeam : public MPTeam
{
public:
	//Destructor
	virtual ~MPCombatTeam() RX;

protected:
	//Constructor
	MPCombatTeam()
		: MPTeam(noinit_t())
	{ }
};


class MPSiegeDefenderTeam : public MPTeam
{
public:
	//Destructor
	virtual ~MPSiegeDefenderTeam() JMP_THIS(0x5CAF10);
	virtual bool IsTeamIncluded(int idx) JMP_THIS(0x5CAE70);

protected:
	//Constructor
	MPSiegeDefenderTeam()
		: MPTeam(noinit_t())
	{ JMP_THIS(0x5CAE10); }
};


class MPSiegeAttackerTeam : public MPTeam
{
public:
	//Destructor
	virtual ~MPSiegeAttackerTeam() JMP_THIS(0x5CAF40);

	//Constructor
	MPSiegeAttackerTeam()
		: MPTeam(noinit_t())
	{ JMP_THIS(0x5CAEB0); }
};


class MPObserverTeam : public MPTeam
{
public:
	//Destructor
	virtual ~MPObserverTeam() JMP_THIS(0x5C94D0);
	virtual bool IsTeamIncluded(int idx) { return this->MPTeam::IsTeamIncluded(idx); }

	//Constructor
	MPObserverTeam()
		: MPTeam(noinit_t())
	{ JMP_THIS(0x5C9470); }
};
