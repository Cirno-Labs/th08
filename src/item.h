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
    // Collects every item on the screen, as th07's (0x4413e0).
    void CollectAll();

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
