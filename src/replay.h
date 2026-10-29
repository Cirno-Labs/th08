// Replays. Only what is known so far.

#pragma once

#include <windows.h>

#include "common.h"

// The start of a replay file, as HandleReplaySaving copies it (0x134
// bytes; th07 0xe8). Only the fields known so far.
struct ReplayHeader
{
    // "T8RP".
    char magic[4];
    // 6 in its low 12 bits.
    WORD version;
    unsigned char unk_06[0x64];
    // The shot type.
    BYTE shotType;
    BYTE unk_6b;
    // The date it was saved, "%m/%d".
    char date[6];
    // The player's name.
    char name[0xc];
    unsigned char unk_7e[0x32];
    DWORD score;
    unsigned char unk_b4[0x80];
};
ASSERT_SIZE(ReplayHeader, 0x134);

// Checks and decodes the replay file `data` of `size` bytes, as th07's;
// returns a new copy, or NULL if it is not a valid replay (0x451d90).
ReplayHeader *ValidateReplay(void *data, int size);

// Saves the replay being recorded to `path` under `name`; with NULLs it
// frees the replay data instead, as th07's (0x4531f0).
void SaveReplay(char *path, char *name);
