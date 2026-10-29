// Enemies. Only what is known so far.

#pragma once

#include <windows.h>
#include <d3dx8math.h>

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

    // Called on the bosses with 0 as a spell card bomb ends (0x42adb0).
    void Func42adb0(int arg);

    unsigned char unk_0[0x8];
    Enemy *unk_8;
    unsigned char unk_c[0x2d28];
    // Where it is.
    D3DXVECTOR3 pos;
    unsigned char unk_2d40[0x64];
    int unk_2da4;
    unsigned char unk_2da8[0x54];
    // Cleared for the bosses as a spell card bomb (Player::bomb.type 4)
    // ends.
    int unk_2dfc;
    unsigned char unk_2e00[0x524];
    // Bit 3: the petal effects drift towards it while it is the first
    // boss. Bit 30 is cleared with unk_2dfc.
    union
    {
        DWORD unk_3324;
        struct
        {
            DWORD unk_3324_0 : 30;
            DWORD unk_3324_30 : 1;
        };
    };
    unsigned char unk_3328[0x58];
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

    unsigned char unk_0[0x2e10];
    // Set by Player::AddedCallback from Player::unk_e2b2c.
    int unk_2e10;
    unsigned char unk_2e14[0x9d9f8c];
    // The bosses, as th07's.
    Enemy *bosses[8];
};
extern EnemyManager g_EnemyManager;
