#include "LockedGame.h"
#include <CCINIClass.h>
#include <Phobos.h>

int LockedGame::LockTheGame(const char* pFilename, const char* pSection, const char* pKey, const char* pString)
{
	CCINIClass* pFile = Phobos::OpenConfig(pFilename);
	pFile->ReadString(pSection, pKey, "", Phobos::readBuffer);
	Phobos::CloseConfig(pFile);

	if (strcmp(Phobos::readBuffer, pString) == 0)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

int LockedGame::CheckTheGame()
{
	int locknumber = 0;

	locknumber += LockedGame::LockTheGame("", "", "", "");
	
	return locknumber;
}
