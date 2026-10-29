// Bullets. Only what is known so far.

#pragma once

#include <windows.h>

#include "common.h"

// The bullets. Only what is known so far.
class BulletManager
{
public:
    // Removes the bullets' chain elements, as th07's (0x433850).
    static void RemoveChain();
    // Loads the bullet sprites from `path` and adds the bullets' chain
    // elements, as th07's (0x4311a0). Returns 0, or -1 on failure.
    static int RegisterChain(char *path);

    unsigned char unk_0[0x6ba538];
    // The bullets alive, as th07's.
    int bulletCount;
};
extern BulletManager g_BulletManager;
