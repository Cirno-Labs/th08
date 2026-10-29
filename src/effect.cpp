// The effects, 0x425410 to 0x428720, as th07's effect.cpp.

#include <string.h>

#include "effect.h"
#include "enemy.h"
#include "player.h"
#include "replay.h"
#include "unsorted.h"

void EffectManager::Reset()
{
    memset(this, 0, sizeof(EffectManager));
}
