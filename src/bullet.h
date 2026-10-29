// Bullets. Only what is known so far.

#pragma once

#include <windows.h>

#include "common.h"
#include "sprite.h"

// The bullets. Only what is known so far.
class BulletManager
{
public:
    // Removes the bullets' chain elements, as th07's (0x433850).
    static void RemoveChain();
    // Loads the bullet sprites from `path` and adds the bullets' chain
    // elements, as th07's (0x4311a0). Returns 0, or -1 on failure.
    static int RegisterChain(char *path);
    // Clears the bullets, as th07's ClearAll (0x415c60).
    void Func415c60();

    unsigned char unk_0[0x6ba538];
    // The bullets alive, as th07's.
    int bulletCount;
    unsigned char unk_6ba53c[0x38];
    // A copy of g_EtamaAnm, which the items also run their scripts from.
    AnmLoaded *etamaAnm;
};
extern BulletManager g_BulletManager;
// etama.anm (slot 6), which BulletManager's added callback loads.
extern AnmLoaded *g_EtamaAnm;
