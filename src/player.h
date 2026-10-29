// The player. Only what is known so far.

#pragma once

#include <windows.h>
#include <d3dx8math.h>

#include "common.h"
#include "timer.h"

// The player. Only what is known so far.
// A shot type's data, behind Player::shotType, as th07's. Only the
// fields known so far.
struct ShotTypeData
{
    unsigned char pad_0[0x4];
    // The bombs the player respawns with.
    float bombs;
    unsigned char pad_8[0xc];
    // The speed of the items drawn to the player, as th07's.
    float itemPullSpeed;
    // The width and height of the box the player collects items in, as
    // th07's.
    float itemCatchSize;
    // Above this line the player draws the items in, as th07's (which had
    // deathScoreShare before it).
    float itemAutoCollectLine;
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
    unsigned char unk_1[0x2];
    // Picks unk_e2a78 over shotType for the items' pull speed, and the
    // direction a time orb moves stats->unk_22. Probably the focus.
    BYTE unk_3;
    unsigned char unk_4[0x2b0];
    // The player's position.
    D3DXVECTOR3 pos;
    unsigned char unk_2c0[0xe27b4];
    ShotTypeData *shotType;
    ShotTypeData *unk_e2a78;
    // A time orb counts 100 points while it is set, as th07's
    // bombJustStarted did for point items.
    int unk_e2a7c;
    unsigned char unk_e2a80[0x5c];
    // Collecting a time orb moves stats->unk_22 while it is 0.
    Timer unk_e2adc;
};
extern Player g_Player;
