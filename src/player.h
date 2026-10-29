// The player. Only what is known so far.

#pragma once

#include <windows.h>
#include <d3dx8math.h>

#include "common.h"

// The player. Only what is known so far.
// A shot type's data, behind Player::shotType, as th07's. Only the
// fields known so far.
struct ShotTypeData
{
    unsigned char pad_0[0x4];
    // The bombs the player respawns with.
    float bombs;
};

class Player
{
public:
    // Adds the player's chain elements, as th07's (0x44c230). Returns 0,
    // or -1 on failure.
    static int RegisterChain(int arg);
    // Removes the player's chain elements, as th07's (0x44dd10).
    static void RemoveChain();

    // 2 while dying, as th07's.
    char playerState;
    unsigned char unk_1[0x2b3];
    // The player's position.
    D3DXVECTOR3 pos;
    unsigned char unk_2c0[0xe27b4];
    ShotTypeData *shotType;
};
extern Player g_Player;
