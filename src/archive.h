// The game's data archives (from 0x474860), as in th07's archive.h.
// Only what other files use so far.

#pragma once

#include <windows.h>

#include "common.h"

struct DatArchiveEntry;
class File;

// The game's data archive (th08.dat), at 0x164f588: files are looked up
// by name (without the directory). The same functions as th07's.
class DatArchive
{
public:
    // The decoded size of entry `name`; 0 if there is none.
    DWORD GetEntrySize(char *name);
    // Decodes entry `name` into `buffer`, which must hold GetEntrySize
    // bytes.
    void *Decode(char *name, void *buffer);

    DatArchiveEntry *entries;
    int numEntries;
    // The archive's path, GlobalAlloc'd.
    char *path;
    File *file;
};
ASSERT_SIZE(DatArchive, 0x10);
extern DatArchive g_Archive;
