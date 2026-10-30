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
    // The angles and speeds, as th07's.
    float angle1;
    float angle2;
    float speed1;
    float speed2;
    unsigned char unk_20[0x1d4];
    // How many bullets, as th07's.
    short count1;
    short count2;
    // Which of the fire instructions (96 on) set it, as th07's.
    WORD aimMode;
    // Cleared by the fire instructions; nothing reads it, as th07's.
    WORD unusedAfterFireC2;
    int flags;
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
    // Called with 4 as a timer callback ends a spell card, as th07's
    // ClearAll(10) was (0x430830).
    void Func430830(int arg);
    // Clears the bullets for a bonus of up to `maxBonus`, as th07's
    // (0x430aa0). Returns the total.
    int ClearAllWithBonus(int maxBonus, int toItems);
    // Fires `shooter`'s pattern, as th07's (0x430e10).
    int FirePattern(struct BulletShooter *shooter);

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
