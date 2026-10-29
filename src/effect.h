// Effects. Only what is known so far.

#pragma once

#include <windows.h>

#include "common.h"

// The effects. Only what is known so far.
class EffectManager
{
public:
    // Removes the effects' chain elements, as th07's (0x4286b0).
    static void RemoveChain();
    // Adds the effects' chain elements, as th07's (0x428620). Returns 0,
    // or -1 on failure.
    static int RegisterChain();
};
