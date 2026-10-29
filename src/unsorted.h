// Classes and globals not yet placed in their own files.

#pragma once

#include <windows.h>

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
