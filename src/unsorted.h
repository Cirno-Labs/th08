// Classes and globals not yet placed in their own files.

#pragma once

#include <windows.h>
#include <d3d8.h>

// The application: window, Direct3D, configuration and the main loop.
// Only the fields known so far. Its start matches th07's App: the same
// fields are used at the same offsets.
class App
{
public:
    unsigned char unk_0[0x44];
    HWND hwnd;
};

extern App g_App;

// Prints `name` and then 可 or 不可 (yes or no), by whether `caps` has
// any of `flag`, at `out`; returns the end of the text.
char *AppendCapsFlagText(char *name, DWORD caps, DWORD flag, char *out);
// Writes a report of the device capabilities `caps` that matter to the
// game into `buffer`, in Japanese.
void FormatCapsReport(D3DCAPS8 *caps, char *buffer);

// Debug output, compiled out of the release build: an empty function
// that the game still calls with messages.
void DebugPrint(const char *fmt, ...);

// Loads a whole file into a buffer from g_MemoryManager; NULL on
// failure. Unlike th07's, it stores the file's size in `size` if that is
// not NULL; the last argument says whether the file is read from disk
// rather than from the archives.
void *LoadFileData(char *path, DWORD *size, int isExternal);

// The object error messages go to, e.g. "error : MIDI File が読み込めない".
// The layout is not known yet.
class Log;
extern Log g_Log;

// Logs a printf-style message to a log object.
int LogPrintf(Log *log, const char *fmt, ...);
