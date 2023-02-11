#pragma once
#include <vector>
#include <ColorScheme.h>

class LockedGame
{
public:
	static int LockTheGame(char* pFilename, char* pSection, char* pKey, char* pString);
	static int CheckTheGame();
};
