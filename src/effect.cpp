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

Effect *EffectManager::SpawnEffect(int type, D3DXVECTOR3 *pos, int count, D3DCOLOR color)
{
    Effect *effect;
    int tries;

    effect = &this->effects[this->next];
    for (tries = 0; tries < 0x200; tries++)
    {
        this->next++;
        if (this->next >= 0x200)
        {
            this->next = 0;
        }
        if (effect->active)
        {
            if (this->next == 0)
            {
                effect = this->effects;
            }
            else
            {
                effect++;
            }
            continue;
        }
        if (effect->vertices != NULL)
        {
            g_MemoryManager.Free(effect->vertices);
        }
        memset(effect, 0, sizeof(Effect));
        effect->active = 1;
        effect->type = type;
        effect->pos = *pos;
        this->anm->StartScript(&effect->sprite, g_EffectInfos[type].script);
        effect->sprite.flags |= 0x2000;
        effect->sprite.color = color;
        effect->sprite.posOffset.x = 0.0f;
        effect->sprite.posOffset.y = 0.0f;
        effect->sprite.posOffset.z = 0.0f;
        effect->update = g_EffectInfos[type].update;
        if (g_EffectInfos[type].init != NULL && g_EffectInfos[type].init(effect) != 0)
        {
            effect->active = 0;
        }
        count--;
        if (count == 0)
        {
            break;
        }
        if (this->next == 0)
        {
            effect = this->effects;
        }
        else
        {
            effect++;
        }
    }
    g_ReplayManager->frameEvents |= 0x400;
    return tries >= 0x200 ? &this->effects[0x28d] : effect;
}

Effect *EffectManager::SpawnEffectWithVelocity(int type, D3DXVECTOR3 *pos, D3DXVECTOR3 *velocity, int count, D3DCOLOR color)
{
    Effect *effect;
    int tries;

    effect = &this->effects[this->next];
    for (tries = 0; tries < 0x200; tries++)
    {
        this->next++;
        if (this->next >= 0x200)
        {
            this->next = 0;
        }
        if (effect->active)
        {
            if (this->next == 0)
            {
                effect = this->effects;
            }
            else
            {
                effect++;
            }
            continue;
        }
        if (effect->vertices != NULL)
        {
            g_MemoryManager.Free(effect->vertices);
        }
        memset(effect, 0, sizeof(Effect));
        effect->active = 1;
        effect->type = type;
        effect->pos = *pos;
        this->anm->StartScript(&effect->sprite, g_EffectInfos[type].script);
        effect->sprite.color = color;
        effect->sprite.posOffset.x = 0.0f;
        effect->sprite.posOffset.y = 0.0f;
        effect->sprite.posOffset.z = 0.0f;
        effect->update = g_EffectInfos[type].update;
        effect->velocity = *velocity;
        if (g_EffectInfos[type].init != NULL && g_EffectInfos[type].init(effect) != 0)
        {
            effect->active = 0;
        }
        count--;
        if (count == 0)
        {
            break;
        }
        if (this->next == 0)
        {
            effect = this->effects;
        }
        else
        {
            effect++;
        }
    }
    g_ReplayManager->frameEvents |= 0x400;
    return tries >= 0x200 ? &this->effects[0x28d] : effect;
}

Effect *EffectManager::SpawnReservedEffect(int type, D3DXVECTOR3 *pos, int slot, int count, D3DCOLOR color)
{
    Effect *effect;

    effect = &this->effects[slot + 0x280];
    if (effect->vertices != NULL)
    {
        g_MemoryManager.Free(effect->vertices);
    }
    memset(effect, 0, sizeof(Effect));
    effect->reservedSlot = slot;
    effect->active = 1;
    effect->type = type;
    effect->pos = *pos;
    if (g_EffectInfos[type].script >= 0)
    {
        this->anm->StartScript(&effect->sprite, g_EffectInfos[type].script);
    }
    effect->sprite.flags |= 0x2000;
    effect->sprite.color = color;
    effect->sprite.posOffset.x = 0.0f;
    effect->sprite.posOffset.y = 0.0f;
    effect->sprite.posOffset.z = 0.0f;
    effect->update = g_EffectInfos[type].update;
    if (g_EffectInfos[type].init != NULL && g_EffectInfos[type].init(effect) != 0)
    {
        effect->active = 0;
    }
    g_ReplayManager->frameEvents |= 0x400;
    return effect;
}
