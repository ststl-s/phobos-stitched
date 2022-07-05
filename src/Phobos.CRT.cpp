#include <Phobos.CRT.h>

#include <cstring>

<<<<<<< Updated upstream
void PhobosCRT::strCopy(char* Dest, const char* Source, size_t Count)
{
=======
void PhobosCRT::strCopy(char* Dest, const char* Source, size_t Count) {
>>>>>>> Stashed changes
	strncpy_s(Dest, Count, Source, Count - 1);
	Dest[Count - 1] = 0;
}

<<<<<<< Updated upstream
void PhobosCRT::wstrCopy(wchar_t* Dest, const wchar_t* Source, size_t Count)
{
=======
void PhobosCRT::wstrCopy(wchar_t* Dest, const wchar_t* Source, size_t Count) {
>>>>>>> Stashed changes
	wcsncpy_s(Dest, Count, Source, Count - 1);
	Dest[Count - 1] = 0;
}
