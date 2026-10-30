// Bullets. Only what is known so far.

#pragma once

#include <string.h>
#include <windows.h>
#include <d3dx8math.h>

#include "common.h"
#include "sprite.h"

// What an enemy fires bullets with, as th07's (0x210). Only what is
// known so far.
struct BulletShooter
{
    // Inline; the copy is at 0x42a410.
    BulletShooter()
    {
        memset(this, 0, sizeof(BulletShooter));
        this->bulletSound = -1;
    }

    // The bullet type, and its color, as th07's.
    short sprite;
    short color;
    D3DXVECTOR3 pos;
    unsigned char unk_10[0x1f0];
    int sound;
    // Bullet::sound, as th07's.
    int bulletSound;
    unsigned char unk_208[0x8];
};
ASSERT_SIZE(BulletShooter, 0x210);

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
