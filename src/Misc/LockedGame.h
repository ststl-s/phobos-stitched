#pragma once
#include <vector>
#include <ColorScheme.h>

class LockedGame
{
public:
	static int LockTheGame(const char* pFilename, const char* pSection, const char* pKey, const char* pString);
	static int CheckTheGame();
};
