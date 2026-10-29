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

Effect *EffectManager::SpawnReservedWithVelocity(int type, D3DXVECTOR3 *pos, D3DXVECTOR3 *velocity, int slot, int count, D3DCOLOR color)
{
    Effect *effect;

    effect = &this->effects[slot + 0x280];
    if (effect->vertices != NULL)
    {
        g_MemoryManager.Free(effect->vertices);
    }
    memset(effect, 0, sizeof(Effect));
    effect->reservedSlot = slot;
    effect->velocity = *velocity;
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

Effect *EffectManager::SpawnEffect2(int type, D3DXVECTOR3 *pos, int count, D3DCOLOR color)
{
    Effect *eff;
    int i;

    eff = &this->effects[0x200];
    for (i = 0; i < 0x80; i++, eff++)
    {
        if (eff->active)
        {
            continue;
        }
        if (eff->vertices != NULL)
        {
            g_MemoryManager.Free(eff->vertices);
        }
        eff->vertices = NULL;
        eff->draw = NULL;
        eff->drawList = 0;
        eff->active = 1;
        eff->type = type;
        eff->pos = *pos;
        this->anm->StartScript(&eff->sprite, g_EffectInfos[type].script);
        eff->sprite.color = color;
        eff->sprite.posOffset.x = 0.0f;
        eff->sprite.posOffset.y = 0.0f;
        eff->sprite.posOffset.z = 0.0f;
        eff->update = g_EffectInfos[type].update;
        eff->timer = 0;
        eff->fadeOut = 0;
        eff->fadeFrames = 0;
        eff->velocity = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        if (g_EffectInfos[type].init != NULL && g_EffectInfos[type].init(eff) != 0)
        {
            eff->active = 0;
        }
        count--;
        if (count == 0)
        {
            break;
        }
    }
    g_ReplayManager->frameEvents |= 0x400;
    return i >= 0x80 ? &this->effects[0x28d] : eff;
}

int EffectInitDrift(Effect *effect)
{
    effect->step[0] = (g_Rng.GetRandomF32InRange(256.0f) - 128.0f) / 12.0f;
    effect->step[1] = (g_Rng.GetRandomF32InRange(256.0f) - 128.0f) / 12.0f;
    effect->step[2] = 0.0f;
    effect->acceleration = -effect->step / 19.0f;
    effect->step *= g_App.gameSpeed;
    effect->acceleration *= g_App.gameSpeed;
    return 0;
}

int EffectUpdateMove(Effect *effect)
{
    effect->pos += effect->step;
    effect->step += effect->acceleration;
    return 1;
}

int EffectInitDriftFast(Effect *effect)
{
    effect->step[0] = (g_Rng.GetRandomF32InRange(256.0f) - 128.0f) * 4.0f / 33.0f;
    effect->step[1] = (g_Rng.GetRandomF32InRange(256.0f) - 128.0f) * 4.0f / 33.0f;
    effect->step[2] = 0.0f;
    effect->acceleration = -effect->step / 20.0f;
    effect->step *= g_App.gameSpeed;
    effect->acceleration *= g_App.gameSpeed;
    return 0;
}

int EffectInitForeground(Effect *effect)
{
    effect->drawList = 2;
    effect->axis.x = 0.0f;
    effect->axis.y = 0.0f;
    effect->axis.z = 0.0f;
    effect->radius = 0.0f;
    return 0;
}
