// The items the enemies drop and the player collects, as th07's item.h.
// Only what is known so far.

#pragma once

#include <windows.h>
#include <d3dx8math.h>

#include "common.h"
#include "sprite.h"
#include "timer.h"

// An item (0x2e4 bytes, th07 0x288).
struct Item
{
    // Clears active and takes the item out of g_ItemManager's active
    // list (0x441770). New in th08.
    void Unlink();
    // What collecting a power item (type 0) and a big power item (type 2)
    // does, as th07's UpdateItems: unless at full power, adds 1 or 8
    // power (converting the other power items when it reaches 128) and
    // 10 points, with a popup of the points or of the new power level.
    // The small one also resets the power item combo and adds subrank
    // (0x440cf0, 0x441170). New in th08 as functions.
    void CollectPower();
    void CollectBigPower();
    // What collecting a point item (type 1) does, as th07's UpdateItems:
    // stats->unk_24 points above the auto-collect line or when it counts
    // in full (which also marks it so), else half of it less
    // unk_24 / 1000 per pixel below the line; doubled at unk22Low1. Then
    // counts it and gives the extends it reaches (0x440e40).
    void CollectPoint();
    // What collecting a type 8 item (a power item at full power) does:
    // CollectPoint's value in tenths, as points only (0x441020). New in
    // th08.
    void CollectFullPowerItem();
    // What collecting a time orb (type 7) does: points (half the point
    // items collected, in tens, at least 100; 10000 from 2000 stats->
    // unk_2c on; 100 while Player::unk_e2a7c is set) with a popup unless
    // there is no item, a time orb, g_SpellCard.Func416b10(8000), and
    // stats->unk_22 moved by 111 one way or the other while
    // Player::unk_e2adc is 0 (0x4412b0). New in th08.
    void CollectTimeOrb();

    Sprite sprite;
    D3DXVECTOR3 pos;
    D3DXVECTOR3 velocity;
    D3DXVECTOR3 target;
    Timer timer;
    // Its kind: picks its script, 0x3d + type.
    char type;
    char active;
    char onScreen;
    char state;
    // Set when it is drawn to the player while Player::bombState is 1:
    // a point item then scores in full.
    char countsFullValue;
    char pad_2d9[3];
    // The neighbours in the list of active items, which SpawnItem
    // appends to. th07 rebuilt a draw list every frame.
    Item *next;
    Item *prev;
};
ASSERT_SIZE(Item, 0x2e4);

// The items, at 0x1653648.
class ItemManager
{
public:
    // Constructs the items (0x440010).
    ItemManager();
    // Drops item `type` at `pos` in `state` into the next free slot,
    // appends it to the active list and returns it; returns the spare
    // last item when all are taken, when `pos` is outside x -64 to 448,
    // or for a type 7 item that does not find a free slot at once. At
    // full power (128), power items (0 and 2) turn into type 8; type 7
    // spawns in state 3, type 10 as type 7 in state 5. States 3 and 5 fly
    // up and sideways at random, unless the player is dying (0x4400a0).
    Item *SpawnItem(D3DXVECTOR3 *pos, int type, int state);
    // Collects every item on the screen, as th07's (0x4413e0).
    void CollectAll();
    // Turns every active power item (types 0 and 2) except `except` into
    // type 8, with effect 0 on it, as th07's ConvertPowerItems (type 7
    // there). Those not rising faster are set to drift up at 0.5
    // (0x441450).
    void ConvertPowerItems(Item *except);
    // Lets every item drawn to the player (state 1) fall again, starting
    // at a slow upward drift, as th07's (0x441530).
    void RecycleCollectedItems();
    // Draws the active items; those above the top as markers at its
    // edge, fading out with the distance, as th07's (0x4415a0).
    void DrawItems();
    // The active items of type 7 (0x4417e0). New in th08.
    int CountType7Items();

    // The last one is what SpawnItem returns when all others are taken.
    Item items[0x831];
    int nextIndex;
    int activeCount;
    // The head of the list of active items.
    Item activeList;
    // The last item of the list.
    Item *activeListTail;
};
ASSERT_SIZE(ItemManager, 0x17b094);
extern ItemManager g_ItemManager;

// The power levels, which play a sound when crossed, as th07's.
extern int g_PowerLevels[];
