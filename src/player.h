// The player. Only what is known so far.

#pragma once

#include <windows.h>

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

    unsigned char unk_0[0xe2a74];
    ShotTypeData *shotType;
};
extern Player g_Player;
