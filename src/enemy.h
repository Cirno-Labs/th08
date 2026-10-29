// Enemies. Only what is known so far.

#pragma once

#include <windows.h>

#include "common.h"

// An enemy. Only what is known so far.
struct Enemy
{
    // Whether unk_2da4 is clear and unk_8 is set. Inline; the copy at
    // 0x41f000 is in an /Od object.
    BOOL Func41f000()
    {
        return this->unk_2da4 == 0 && this->unk_8 != NULL ? TRUE : FALSE;
    }
    // How many enemies follow it through unk_8, when Func41f000. Inline;
    // the copy at 0x41fd40 is in an /Od object. DrawBanners shows it
    // for the first boss.
    int Func41fd40()
    {
        Enemy *enemy;
        int count;

        enemy = this;
        count = 0;
        if (this->Func41f000())
        {
            while (enemy->unk_8 != NULL)
            {
                enemy = enemy->unk_8;
                count++;
            }
        }
        return count;
    }

    unsigned char unk_0[0x8];
    Enemy *unk_8;
    unsigned char unk_c[0x2d98];
    int unk_2da4;
    unsigned char unk_2da8[0x5d8];
    // Shown by DrawBanners next to Func41fd40.
    int unk_3380;
};

// The enemies, at 0x577f20. Only what is known so far.
class EnemyManager
{
public:
    // Whether any of the bosses is set, as th07's (0x42f1f0).
    int AnyBossActive();
    // Removes the enemies' chain elements, as th07's (0x42ef70).
    static void RemoveChain();
    // Adds the enemies' chain elements, as th07's (0x42c590). Returns 0,
    // or -1 on failure.
    static int RegisterChain();
    // Clears the enemies for a bonus, as th07's (0x42efb0).
    int ClearEnemiesForBonus(int maxBonus, int start);

    unsigned char unk_0[0x9dcda0];
    // The bosses, as th07's.
    Enemy *bosses[8];
};
extern EnemyManager g_EnemyManager;
