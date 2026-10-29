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
};
extern EnemyManager g_EnemyManager;
