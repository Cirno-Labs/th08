// Effects. Only what is known so far.

#pragma once

#include <windows.h>
#include <d3dx8math.h>

#include "common.h"

struct Effect;

// The effects, at 0x4ece60. Only what is known so far.
class EffectManager
{
public:
    // Starts `count` effects of `type` at `pos` in `color`, as th07's
    // (0x425430).
    Effect *SpawnEffect(int type, D3DXVECTOR3 *pos, int count, D3DCOLOR color);
    // Removes the effects' chain elements, as th07's (0x4286b0).
    static void RemoveChain();
    // Adds the effects' chain elements, as th07's (0x428620). Returns 0,
    // or -1 on failure.
    static int RegisterChain();
};
extern EffectManager g_EffectManager;
