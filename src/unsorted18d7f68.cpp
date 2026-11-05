// Data only, 0x18d7f68: the last of the game's .bss, four unreferenced
// bytes after lzss.cpp's window and before D3DX8's. As th07's
// unsorted4b9e44.cpp, whose unused word likewise ends the game's
// globals, an object linked after archive.cpp's, which has none.

#include <windows.h>

// The game's data, as the original defines it here. The DLL build uses
// the original's copies instead.
#ifdef TH08_EXE
// Unused, a stand-in: the type and name are not recoverable.
DWORD g_LzssUnusedWord;
#endif // TH08_EXE
