// Enemies. Only what is known so far.

#pragma once

#include <windows.h>

#include "common.h"

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
};
extern EnemyManager g_EnemyManager;
