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
    unsigned char pad_20[0x14];
    // How fast the items move, as a factor of their velocity. New in th08.
    float itemSpeed;
};

// One of the player's bomb hit areas (0x40 bytes), in two arrays of
// 0xc0 at Player+0xb8834 and +0xbb834. Placeholder name; only what is
// known so far.
struct PlayerBombHitbox
{
    float x;
    float y;
    unsigned char unk_8[0x8];
    float width;
    float height;
    unsigned char unk_18[0x8];
    float angle;
    int unk_24;
    unsigned char unk_28[0x4];
    int unk_2c;
    unsigned char unk_30[0x8];
    int unk_38;
    BYTE active;
    unsigned char unk_3d[0x3];
};

class Player
{
public:
    // Adds the player's chain elements, as th07's (0x44c230). Returns 0,
    // or -1 on failure.
    static int RegisterChain(int arg);
    // Removes the player's chain elements, as th07's (0x44dd10).
    static void RemoveChain();
    // The angle from `pos` to the player, as th07's (0x44c1b0).
    float AngleFrom(D3DXVECTOR3 *pos);
    // Whether the box of `size` at `pos` touches the player's graze box,
    // as th07's (0x44a5a0).
    int TouchesGrazeBox(D3DXVECTOR3 *pos, D3DXVECTOR3 *size);
    // Starts the first free hit area of the second array at `pos` (x
    // and y), `width` by `height`, with unk_2c `a` and unk_24 `b`, and
    // returns it (0x44de60).
    PlayerBombHitbox *Func44de60(D3DXVECTOR3 *pos, float width, float height, int a, int b);
    // The same in the first array (0x44dfa0).
    PlayerBombHitbox *Func44dfa0(D3DXVECTOR3 *pos, float width, float height, int a, int b);
    // Whether unk_5 is set. Inline; the copy is at 0x40bc40.
    int IsUnk5Set()
    {
        return this->unk_5;
    }
    // Whether unk_5 is clear. Inline; the copy is at 0x40bc20.
    int IsUnk5Clear()
    {
        return !this->unk_5;
    }

    // 2 while dying, as th07's.
    char playerState;
    unsigned char unk_1[0x2];
    // Picks unk_e2a78 over shotType for the items' pull speed, and the
    // direction a time orb moves stats->unk_22. Probably the focus.
    BYTE unk_3;
    unsigned char unk_4;
    // Stage::StepObjects sends interrupt 1 to g_Stage.unk_844 as it is
    // set, 2 as it is cleared.
    BYTE unk_5;
    unsigned char unk_6[0x2ae];
    // The player's position.
    D3DXVECTOR3 pos;
    unsigned char unk_2c0[0xe27b4];
    ShotTypeData *shotType;
    ShotTypeData *unk_e2a78;
    // A time orb counts 100 points while it is set, as th07's
    // bombJustStarted did for point items.
    int unk_e2a7c;
    unsigned char unk_e2a80[0x44];
    // While it is below 0, items flying up (state 3) are drawn in.
    Timer unk_e2ac4;
    unsigned char unk_e2ad0[0xc];
    // Collecting a time orb moves stats->unk_22 while it is 0.
    Timer unk_e2adc;
};
extern Player g_Player;
