/*FlyingStrings.h
Useable to get out messages from units
Used to output Bounty messages
By AlexB and Joshy
*/

#pragma once
#include <vector>
#include <ColorScheme.h>

class LockedGame
{
public:
	static int LockTheGame(char* pFilename, char* pSection, char* pKey, char* pString);
};
