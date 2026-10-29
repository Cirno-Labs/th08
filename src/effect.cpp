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

int EffectUpdateOrbit(Effect *effect)
{
    D3DXVECTOR3 v;
    float a2;
    D3DXVECTOR3 n2;
    float c;
    D3DXMATRIX m2;
    float s;

    D3DXVec3Normalize(&n2, &effect->axis);
    s = sinf(effect->angle);
    c = cosf(effect->angle);
    effect->rotation.x = n2.x * s;
    effect->rotation.y = n2.y * s;
    effect->rotation.z = n2.z * s;
    effect->rotation.w = c;
    D3DXMatrixRotationQuaternion(&m2, &effect->rotation);
    v.x = n2.y * 1.0f - n2.z * 0.0f;
    v.y = n2.z * 0.0f - n2.x * 1.0f;
    v.z = n2.x * 0.0f - n2.y * 0.0f;
    if (D3DXVec3LengthSq(&v) < 0.00001f)
    {
        // Sets the axis, not v: v stays (0, 0, 0).
        n2 = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
    }
    else
    {
        D3DXVec3Normalize(&v, &v);
    }
    v *= effect->radius;
    D3DXVec3TransformCoord(&v, &v, &m2);
    v.z *= 6.0f;
    effect->pos = v + effect->origin;
    effect->pos.z = 0.0f;
    if (effect->fadeOut != 0)
    {
        effect->fadeFrames++;
        if (effect->fadeFrames >= 16)
        {
            return 0;
        }
        a2 = 1.0f - effect->fadeFrames / 16.0f;
        effect->sprite.color = (effect->sprite.color & 0xffffff) | ((int)(a2 * 255.0f) << 24);
        effect->sprite.scaleX = 2.0f - a2;
        effect->sprite.scaleY = effect->sprite.scaleX;
    }
    return 1;
}

int EffectInitBossPetal(Effect *effect)
{
    D3DXVECTOR3 back;

    back = -g_Stage.camera.at;
    effect->truePos = g_Stage.camera.at + g_Stage.camera.eye;
    effect->truePos.x += g_Rng.GetRandomF32SignedInRange(60.0f) + back.x / 2.0f;
    effect->truePos.y += g_Rng.GetRandomF32SignedInRange(100.0f) - 50.0f + back.y / 2.0f;
    effect->truePos.z += g_Rng.GetRandomF32InRange(100.0f) - 100.0f + back.z / 2.0f;
    effect->step.x = g_Rng.GetRandomF32SignedInRange(0.001f) + effect->velocity.x;
    effect->step.y = g_Rng.GetRandomF32SignedInRange(0.03f) + effect->velocity.y;
    effect->step.z = -g_Rng.GetRandomF32InRange(0.1f) - 0.3f + effect->velocity.z;
    effect->acceleration.x = g_Rng.GetRandomF32SignedInRange(0.0001f);
    effect->acceleration.y = g_Rng.GetRandomF32SignedInRange(0.0001f);
    effect->acceleration.z = -0.0003f;
    effect->step = effect->step * g_App.gameSpeed;
    effect->acceleration = effect->acceleration * g_App.gameSpeed;
    effect->drawList = 1;
    effect->sprite.posOffset.x = -9999.0f;
    effect->sprite.moveStart.x = 0.0f;
    effect->sprite.moveEnd.x = 0.0f;
    effect->sprite.moveEnd.y = 0.0f;
    effect->sprite.moveEnd.z = 0.0f;
    effect->sprite.rotateStart.x = 0.0f;
    effect->sprite.rotateStart.y = 0.0f;
    effect->sprite.rotateStart.z = 0.0f;
    return 0;
}

int EffectUpdateBossPetal(Effect *effect)
{
    float cosine;

    effect->step += effect->acceleration;
    effect->truePos += effect->step;
    effect->pos = effect->truePos;
    D3DXVECTOR3 v;
    v = effect->pos - g_Stage.camera.eye;
    D3DXVec3Normalize(&v, &v);
    cosine = D3DXVec3Dot(&g_Stage.camera.dir, &v);
    if (cosine < 0.94f)
    {
        return 0;
    }
    if (g_EnemyManager.AnyBossActive() && g_EnemyManager.bosses[0]->unk_3324 >> 3 & 1)
    {
        if (effect->sprite.posOffset.x <= -9999.0f)
        {
            effect->sprite.posOffset = g_EnemyManager.bosses[0]->pos;
        }
        else
        {
            effect->sprite.posOffset =
                (g_EnemyManager.bosses[0]->pos - effect->sprite.posOffset) * 0.1f + effect->sprite.posOffset;
        }
    }
    effect->sprite.flags |= 0x20000;
    effect->sprite.color2Rgb[2] = (DWORD)effect->sprite.colorRgb[2] * g_Stage.unk_a34[2] >> 8;
    effect->sprite.color2Rgb[1] = (DWORD)effect->sprite.colorRgb[1] * g_Stage.unk_a34[1] >> 8;
    effect->sprite.color2Rgb[0] = (DWORD)effect->sprite.colorRgb[0] * g_Stage.unk_a34[0] >> 8;
    effect->sprite.color2Alpha = (DWORD)effect->sprite.alpha * g_Stage.unk_a34[3] >> 8;
    return 1;
}
