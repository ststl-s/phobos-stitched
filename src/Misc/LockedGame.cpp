#include "LockedGame.h"
#include <CCINIClass.h>
#include <Phobos.h>

int LockedGame::LockTheGame(char* pFilename, char* pSection, char* pKey, char* pString)
{
	CCINIClass* pFile = Phobos::OpenConfig(pFilename);
	pFile->ReadString(pSection, pKey, "", Phobos::readBuffer);

	if (strcmp(Phobos::readBuffer, pString) == 0)
		return 1;
	else
		return 0;
}
