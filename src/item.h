// Items. Only what is known so far.

#pragma once

#include <windows.h>

#include "common.h"

// The items, at 0x1653648. Only what is known so far.
class ItemManager
{
public:
    // Collects every item on the screen, as th07's (0x4413e0).
    void CollectAll();
};
extern ItemManager g_ItemManager;
