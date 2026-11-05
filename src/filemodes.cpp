// File::Open's modes, data only (0x4c8704), as th07's filemodes.cpp: the
// last of the game's .data, after mainmenu.cpp's, where archive.cpp's
// object or a data-only object linked last would put them. Split from
// archive.cpp at a plausible seam (nothing in the binary shows it).

// The game's data, as the original defines it here. The DLL build uses
// the original's copies instead.
#ifdef TH08_EXE
char *g_FileModes[3] = {
    "r",
    "w",
    "a",
};
#endif // TH08_EXE
