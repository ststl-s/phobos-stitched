#include <Helpers/Macro.h>

DEBUG_HOOK(0x64CD11, ReconnectionError_BackTrace_Crashimmediately, 0x7)
{
	int* p = NULL;
	*p = 1;

	return 0;
}
