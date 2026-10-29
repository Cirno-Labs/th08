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

int EffectInitBossPetal2(Effect *effect)
{
    D3DXVECTOR3 back;

    back = -g_Stage.camera.at;
    effect->truePos = g_Stage.camera.at + g_Stage.camera.eye;
    effect->truePos.x += g_Rng.GetRandomF32SignedInRange(60.0f) + back.x / 2.0f;
    effect->truePos.y += g_Rng.GetRandomF32SignedInRange(200.0f) - 200.0f + back.y / 2.0f;
    effect->truePos.z += g_Rng.GetRandomF32InRange(100.0f) - 100.0f + back.z / 2.0f;
    effect->step.x = g_Rng.GetRandomF32SignedInRange(0.001f) + effect->velocity.x;
    effect->step.y = g_Rng.GetRandomF32SignedInRange(0.03f) + 0.4f;
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

int EffectUpdateBossPetal2(Effect *effect)
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
    if (g_EnemyManager.bosses[0] != NULL && g_EnemyManager.bosses[0]->unk_3324 >> 3 & 1)
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
    return 1;
}

int EffectInitConverge(Effect *effect)
{
    float f;

    effect->origin = effect->pos;
    effect->origin.z = 0.0f;
    f = g_Rng.GetRandomF32InRange(6.2831855f) - 3.1415927f;
    effect->axis.x = cosf(f);
    effect->axis.y = sinf(f);
    effect->axis.z = 0.0f;
    return 0;
}

int EffectUpdateConverge(Effect *effect)
{
    float f;

    f = 256.0f - effect->timer.AsFloat() * 256.0f / 60.0f;
    effect->pos = effect->axis * f + effect->origin;
    effect->pos.z = 0.0f;
    return 1;
}

int EffectUpdateFollowPlayer(Effect *effect)
{
    if (effect->sprite.IsScriptDone())
    {
        return 0;
    }
    effect->pos = g_Player.pos;
    return 1;
}

int EffectUpdateConvergeSlow(Effect *effect)
{
    float f;

    f = 256.0f - effect->timer.AsFloat() * 256.0f / 240.0f;
    effect->pos = effect->axis * f + effect->origin;
    return 1;
}

void OffsetPetalEffects(D3DXVECTOR3 *offset)
{
    Effect *eff = g_EffectManager.effects;
    int i;

    for (i = 0; i < 0x200; i++, eff++)
    {
        if (eff->type == 51)
        {
            eff->truePos += *offset;
        }
    }
}

int EffectUpdatePetal(Effect *effect)
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
    effect->sprite.SetRotationZ(AddNormalizeAngle(effect->sprite.rotation.z, effect->sprite.rotation.x));
    if (effect->pos.z >= 0.0f)
    {
        return 0;
    }
    return 1;
}

int EffectInitPetal(Effect *effect)
{
    D3DXVECTOR3 back;

    back = -g_Stage.camera.at;
    effect->truePos = g_Stage.camera.at + g_Stage.camera.eye;
    effect->truePos.x += g_Rng.GetRandomF32InRange(120.0f) - 60.0f + back.x / 2.0f;
    effect->truePos.y += g_Rng.GetRandomF32InRange(200.0f) - 100.0f + back.y / 2.0f;
    effect->truePos.z += g_Rng.GetRandomF32InRange(100.0f) - 100.0f + back.z / 2.0f;
    effect->step.x = g_Rng.GetRandomF32InRange(0.06f) - 0.03f + effect->velocity.x;
    effect->step.y = g_Rng.GetRandomF32InRange(0.06f) - 0.03f + effect->velocity.y;
    effect->step.z = g_Rng.GetRandomF32InRange(0.1f) + 0.03f + effect->velocity.z;
    effect->acceleration.x = g_Rng.GetRandomF32InRange(0.0002f) - 0.0001f;
    effect->acceleration.y = g_Rng.GetRandomF32InRange(0.0002f) - 0.0001f;
    effect->step = effect->step * g_App.gameSpeed;
    effect->acceleration = effect->acceleration * g_App.gameSpeed;
    effect->drawList = 1;
    effect->sprite.rotation.z = g_Rng.GetRandomF32InRange(6.2831855f) - 3.1415927f;
    effect->sprite.rotation.x = g_Rng.GetRandomF32InRange(0.031415928f) - 0.015707964f;
    return 0;
}

int EffectInitDiverge(Effect *effect)
{
    float angle;

    if (effect->velocity.x > -990.0)
    {
        angle = AddNormalizeAngle(effect->velocity.x, 0.0f);
    }
    else
    {
        angle = g_Rng.GetRandomF32InRange(6.2831855f) - 3.1415927f;
    }
    effect->origin = effect->pos;
    effect->origin.z = 0.0f;
    effect->axis.x = cosf(angle);
    effect->axis.y = sinf(angle);
    effect->axis.z = 0.0f;
    effect->axis *= g_Rng.GetRandomF32InRange(1.5f) + 1.0f;
    return 0;
}

int EffectUpdateDivergeEased(Effect *effect)
{
    float f;

    f = effect->timer.AsFloat() / 90.0f;
    f = 1.0f - (1.0f - f) * (1.0f - f);
    effect->pos = effect->axis * f * 128.0f + effect->origin;
    effect->pos.z = 0.0f;
    return 1;
}

int EffectUpdateNone(Effect *effect)
{
    return 1;
}

int EffectInitMoveOut(Effect *effect)
{
    D3DXVECTOR3 offset;

    ((Vector2 *)&offset)->SetPolar(effect->velocity.x, 256.0f);
    effect->pos.x += offset.x;
    effect->pos.y += offset.y;
    effect->sprite.rotation.z = AddNormalizeAngle(effect->velocity.x, 1.5707964f);
    return 0;
}

int EffectInitRing(Effect *effect)
{
    effect->vertices = (VertexTex1DiffuseXyzrwh *)g_MemoryManager.Alloc(0x1c38, "Effect");
    if (effect->vertices == NULL)
    {
        return -1;
    }
    effect->segmentCount = 3;
    effect->origin = effect->pos;
    effect->axis.x = 0.0f;
    effect->axis.y = 0.0f;
    effect->axis.z = 1.0f;
    effect->unk_2f8.x = 0.0f;
    effect->unk_2f8.y = -1.0f;
    effect->unk_2f8.z = 0.0f;
    effect->angle = effect->velocity.x;
    effect->radius = effect->velocity.y;
    effect->ringWidth = effect->velocity.z;
    g_SpriteEngine->FillSpriteStripVertices(&effect->sprite, effect->vertices, effect->segmentCount * 2);
    effect->verticesDirty = 1;
    effect->draw = EffectDrawRing;
    effect->unk_32c = 0.0f;
    effect->unk_330 = 0.0f;
    effect->unk_334 = 0.0f;
    effect->segmentCount = 0x18;
    return 0;
}

int EffectDrawRing(Effect *effect)
{
    float outer;
    float delta;
    VertexTex1DiffuseXyzrwh *vtx;
    float inner;
    int i;

    if (effect->verticesDirty)
    {
        delta = 6.2831855f / effect->segmentCount;
        outer = effect->ringWidth / sinf((3.1415927f - delta) / 2.0f);
        vtx = effect->vertices;
        g_SpriteEngine->FillVerticalStripVertices(&effect->sprite, effect->vertices, effect->segmentCount * 2 + 2);
        if (effect->unk_32c == 0.0f)
        {
            float angle = effect->angle;

            inner = effect->radius - outer;
            outer += effect->radius;
            for (i = effect->segmentCount + 1; i > 0; i--)
            {
                if (angle >= 3.1415927f)
                {
                    angle -= 6.2831855f;
                }
                vtx->pos.z = 0.0f;
                ((Vector2 *)&vtx->pos)->SetPolar(angle, outer);
                vtx->pos += effect->origin;
                vtx->pos.x = g_GameManager.arena[0] + vtx->pos.x;
                vtx->pos.y = g_GameManager.arena[1] + vtx->pos.y;
                vtx++;
                vtx->pos.z = 0.0f;
                ((Vector2 *)&vtx->pos)->SetPolar(angle, inner);
                vtx->pos += effect->origin;
                vtx->pos.x = g_GameManager.arena[0] + vtx->pos.x;
                vtx->pos.y = g_GameManager.arena[1] + vtx->pos.y;
                vtx++;
                angle += delta;
            }
        }
        else if (effect->unk_334 == 0.0f)
        {
            float angle = 0.0f;
            D3DXVECTOR3 pos;
            float outerY;
            float ryInner;

            outerY = outer + effect->unk_32c;
            ryInner = effect->unk_32c - outer;
            inner = effect->radius - outer;
            outer += effect->radius;
            for (i = effect->segmentCount + 1; i > 0; i--)
            {
                ((Vector2 *)&pos)->SetPolarEllipse(angle, outer, outerY);
                Rotate2D(&vtx->pos, &pos, effect->unk_330);
                vtx->pos += effect->origin;
                vtx->pos.x = g_GameManager.arena[0] + vtx->pos.x;
                vtx->pos.y = g_GameManager.arena[1] + vtx->pos.y;
                vtx->pos.z = 0.0f;
                vtx++;
                ((Vector2 *)&pos)->SetPolarEllipse(angle, inner, ryInner);
                Rotate2D(&vtx->pos, &pos, effect->unk_330);
                vtx->pos += effect->origin;
                vtx->pos.x = g_GameManager.arena[0] + vtx->pos.x;
                vtx->pos.y = g_GameManager.arena[1] + vtx->pos.y;
                vtx->pos.z = 0.0f;
                vtx++;
                angle += delta;
            }
        }
        else
        {
            float wobbleAngle = effect->unk_330;
            float angle = effect->angle;
            float phaseStep = 6.2831855f * effect->unk_334 / effect->segmentCount;
            D3DXVECTOR3 unused;
            float wobble;

            inner = effect->radius - outer;
            outer += effect->radius;
            for (i = effect->segmentCount + 1; i > 0; i--)
            {
                if (angle >= 3.1415927f)
                {
                    angle -= 6.2831855f;
                }
                if (wobbleAngle >= 3.1415927f)
                {
                    wobbleAngle -= 6.2831855f;
                }
                wobble = cosf(wobbleAngle) * effect->unk_32c;
                ((Vector2 *)&vtx->pos)->SetPolar(angle, outer + wobble);
                vtx->pos += effect->origin;
                vtx->pos.x = g_GameManager.arena[0] + vtx->pos.x;
                vtx->pos.y = g_GameManager.arena[1] + vtx->pos.y;
                vtx->pos.z = 0.0f;
                vtx++;
                ((Vector2 *)&vtx->pos)->SetPolar(angle, inner + wobble);
                vtx->pos += effect->origin;
                vtx->pos.x = g_GameManager.arena[0] + vtx->pos.x;
                vtx->pos.y = g_GameManager.arena[1] + vtx->pos.y;
                vtx->pos.z = 0.0f;
                vtx++;
                angle += delta;
                wobbleAngle += phaseStep;
            }
        }
        effect->verticesDirty = 0;
    }
    g_SpriteEngine->DrawVertexStrip(&effect->sprite, effect->vertices, effect->segmentCount * 2 + 2);
    return 1;
}

int EffectInitRing2(Effect *effect)
{
    EffectInitRing(effect);
    effect->unk_355 = 1;
    return 0;
}

int EffectUpdateRing(Effect *effect)
{
    effect->verticesDirty = 1;
    effect->ringWidth = effect->sprite.scaleY;
    effect->radius = effect->sprite.pos.x;
    return 1;
}

int EffectUpdateRingFull(Effect *effect)
{
    effect->segmentCount = effect->sprite.scriptVar0;
    effect->unk_334 = effect->sprite.scriptVar1;
    effect->ringWidth = effect->sprite.scaleY;
    effect->radius = effect->sprite.pos.x;
    effect->unk_32c = effect->sprite.pos.y;
    effect->angle = effect->sprite.rotation.z;
    effect->unk_330 = effect->sprite.rotation.y;
    effect->verticesDirty = 1;
    return 1;
}

int EffectUpdateRingTimed(Effect *effect)
{
    effect->segmentCount = 0x20;
    effect->ringWidth = effect->sprite.scaleY;
    effect->radius = effect->sprite.pos.x;
    effect->unk_32c = effect->sprite.pos.y;
    effect->verticesDirty = 1;
    if (effect->timer >= 120)
    {
        return 0;
    }
    return 1;
}

int EffectUpdateRingFade(Effect *effect)
{
    effect->verticesDirty = 1;
    effect->ringWidth = effect->sprite.scaleY;
    effect->radius = effect->sprite.pos.x;
    effect->unk_32c = effect->sprite.pos.y;
    effect->angle = effect->sprite.rotation.z;
    if (effect->sprite.alpha == 0)
    {
        return 0;
    }
    return 1;
}

int EffectUpdateRingAt(Effect *effect)
{
    effect->segmentCount = effect->sprite.scriptVar0;
    effect->unk_334 = effect->sprite.scriptVar1;
    effect->ringWidth = effect->sprite.scaleY;
    effect->radius = effect->sprite.scriptVarF1;
    effect->angle = effect->sprite.rotation.z;
    effect->unk_330 = effect->sprite.rotation.y;
    effect->verticesDirty = 1;
    effect->origin = effect->sprite.pos;
    return 1;
}

int EffectManager::OnUpdate(EffectManager *mgr)
{
    Effect *eff = mgr->effects;
    int i;

    mgr->activeCount = 0;
    mgr->drawListTails[0] = &mgr->drawList0;
    mgr->drawListTails[1] = &mgr->drawList1;
    mgr->drawListTails[2] = &mgr->drawList2;
    mgr->drawListTails[3] = &mgr->drawList3;
    mgr->drawListTails[4] = &mgr->drawList4;
    mgr->drawList0.next = NULL;
    mgr->drawList1.next = NULL;
    mgr->drawList2.next = NULL;
    mgr->drawList3.next = NULL;
    mgr->drawList4.next = NULL;
    for (i = 0; i < 0x28d; i++, eff++)
    {
        if (!eff->active)
        {
            if (eff->vertices != NULL)
            {
                g_MemoryManager.Free(eff->vertices);
                eff->vertices = NULL;
            }
            continue;
        }
        mgr->activeCount++;
        if (!(g_GameManager.unk_3dbac >> 10 & 1) || eff->unk_357)
        {
            if (eff->update != NULL && eff->update(eff) != 1)
            {
                eff->active = 0;
                continue;
            }
            if (g_SpriteEngine->Step(&eff->sprite) != 0)
            {
                eff->active = 0;
                continue;
            }
            eff->timer++;
        }
        eff->next = NULL;
        if (eff->type == 0x40)
        {
            continue;
        }
        if (eff->drawList == 1 || eff->drawList >= 3)
        {
            mgr->drawListTails[1]->next = eff;
            mgr->drawListTails[1] = eff;
        }
        else if (eff->drawList == 0)
        {
            if (eff->unk_355)
            {
                mgr->drawListTails[3]->next = eff;
                mgr->drawListTails[3] = eff;
            }
            else if (eff->sprite.blendMode == 1)
            {
                mgr->drawListTails[4]->next = eff;
                mgr->drawListTails[4] = eff;
            }
            else
            {
                mgr->drawListTails[0]->next = eff;
                mgr->drawListTails[0] = eff;
            }
        }
        else
        {
            mgr->drawListTails[2]->next = eff;
            mgr->drawListTails[2] = eff;
        }
    }
    mgr->frames++;
    if (mgr->frames % 300 == 100 && g_GameManager.ChecksumMismatch())
    {
        return 4;
    }
    return 1;
}

int EffectManager::OnDraw(EffectManager *mgr)
{
    Effect *eff;

    eff = mgr->drawList0.next;
    while (eff != NULL)
    {
        if (eff->draw != NULL)
        {
            eff->draw(eff);
        }
        else
        {
            eff->sprite.pos = eff->pos;
            eff->sprite.pos.x = g_GameManager.arena[0] + eff->sprite.pos.x;
            eff->sprite.pos.y = g_GameManager.arena[1] + eff->sprite.pos.y;
            eff->sprite.pos.z = 0.07f;
            eff->sprite.pos += eff->sprite.posOffset;
            g_SpriteEngine->Draw(&eff->sprite);
        }
        eff = eff->next;
    }
    eff = mgr->drawList2.next;
    while (eff != NULL)
    {
        eff->sprite.pos = eff->pos;
        g_SpriteEngine->DrawProjected(&eff->sprite);
        eff = eff->next;
    }
    eff = mgr->drawList4.next;
    while (eff != NULL)
    {
        if (eff->draw != NULL)
        {
            eff->draw(eff);
        }
        else
        {
            eff->sprite.pos = eff->pos;
            eff->sprite.pos.x = g_GameManager.arena[0] + eff->sprite.pos.x;
            eff->sprite.pos.y = g_GameManager.arena[1] + eff->sprite.pos.y;
            eff->sprite.pos.z = 0.07f;
            eff->sprite.pos += eff->sprite.posOffset;
            g_SpriteEngine->Draw(&eff->sprite);
        }
        eff = eff->next;
    }
    return 1;
}

int EffectManager::OnDraw3()
{
    Effect *eff;

    eff = this->drawList3.next;
    while (eff != NULL)
    {
        if (eff->draw != NULL)
        {
            eff->draw(eff);
        }
        else
        {
            eff->sprite.pos = eff->pos;
            eff->sprite.pos.x = g_GameManager.arena[0] + eff->sprite.pos.x;
            eff->sprite.pos.y = g_GameManager.arena[1] + eff->sprite.pos.y;
            eff->sprite.pos += eff->sprite.posOffset;
            eff->sprite.pos.z = 0.04f;
            g_SpriteEngine->Draw(&eff->sprite);
        }
        eff = eff->next;
    }
    return 1;
}

int EffectManager::OnDraw2()
{
    Effect *eff = this->drawList1.next;
    int n = 0;

    if (g_App.effectsQuality == 0)
    {
        return 1;
    }
    while (eff != NULL)
    {
        n++;
        if (g_App.effectsQuality == 1 && (n & 1) != 0)
        {
            return 1;
        }
        eff->sprite.pos = eff->pos;
        if (eff->drawList == 4)
        {
            g_SpriteEngine->Draw(&eff->sprite);
        }
        else if (eff->drawList == 1)
        {
            if (eff->type == 51 || eff->type == 63)
            {
                g_SpriteEngine->DrawProjectedWithCallback(&eff->sprite, EffectDrawBossPetal);
            }
            else
            {
                g_SpriteEngine->DrawProjected(&eff->sprite);
            }
        }
        else
        {
            g_SpriteEngine->DrawProjected3D(&eff->sprite);
        }
        eff = eff->next;
    }
    return 1;
}

void EffectDrawBossPetal(Sprite *vm, D3DXVECTOR3 *pos)
{
    D3DXVECTOR3 offset;
    D3DXVECTOR3 target;

    if (!g_GameManager.isPaused && !g_GameManager.isInContinueMenu)
    {
        target = *pos + vm->moveEnd;
        offset = vm->posOffset - target;
        if (vm->posOffset.x > -9999.0f)
        {
            offset.x += 32.0f;
            offset.y += 16.0f;
            offset.z = 0.0f;
            if (D3DXVec3LengthSq(&offset) < 25600.0f)
            {
                vm->moveStart.x += 0.0005f;
                vm->moveEnd += offset * vm->moveStart.x;
            }
        }
        offset = target - g_Player.pos;
        offset.x -= 32.0f;
        offset.y -= 16.0f;
        offset.z = 0.0f;
        if (D3DXVec3LengthSq(&offset) < 7744.0f)
        {
            vm->moveEnd += offset * 0.02f;
        }
    }
    *pos += vm->moveEnd;
}

int EffectManager::AddedCallback(EffectManager *mgr)
{
    mgr->Reset();
    mgr->anm = g_SpriteEngine->GetAnm(6);
    g_Stage.unk_b34 = 0;
    g_Stage.unk_b30 = 2;
    if (!IsRestarting())
    {
        if (!g_GameManager.IsInSpellCardPractice() || g_GameManager.unk_3dbb0 < 0xd8)
        {
            mgr->stageAnm = g_SpriteEngine->PreloadAnm(9, g_EffectAnmPaths[g_GameManager.stage]);
        }
        else
        {
            mgr->stageAnm = g_SpriteEngine->PreloadAnm(9, g_EffectAnmPaths[g_GameManager.unk_3dbb0 - 0xcf]);
        }
        if (mgr->stageAnm == NULL)
        {
            return -1;
        }
    }
    else
    {
        mgr->stageAnm = g_SpriteEngine->GetAnm(9);
    }
    return 0;
}

int EffectManager::DeletedCallback(EffectManager *mgr)
{
    Effect *eff = mgr->effects;
    int i;

    for (i = 0; i < 0x28d; i++, eff++)
    {
        if (eff->vertices != NULL)
        {
            g_MemoryManager.Free(eff->vertices);
            eff->vertices = NULL;
        }
    }
    if (!IsRestarting())
    {
        g_SpriteEngine->ReleaseAnm(9);
    }
    return 0;
}

int EffectManager::RegisterChain()
{
    EffectManager *mgr = &g_EffectManager;

    mgr->Reset();
    g_EffectCalcChain.SetCallback((ChainCallback)EffectManager::OnUpdate);
    g_EffectCalcChain.addedCallback = (ChainCallback)EffectManager::AddedCallback;
    g_EffectCalcChain.deletedCallback = (ChainCallback)EffectManager::DeletedCallback;
    g_EffectCalcChain.arg = mgr;
    if (g_Chain.AddToCalcChain(&g_EffectCalcChain, 13) != 0)
    {
        return -1;
    }
    g_EffectDrawChain.SetCallback((ChainCallback)EffectManager::OnDraw);
    g_EffectDrawChain.arg = mgr;
    g_Chain.AddToDrawChain(&g_EffectDrawChain, 12);
    return 0;
}

void EffectManager::RemoveChain()
{
    g_Chain.Remove(&g_EffectCalcChain);
    g_Chain.Remove(&g_EffectDrawChain);
}

void Vector2::SetPolar(float angle, float radius)
{
    __asm
    {
        mov eax, this
        fld angle
        fsincos
        fmul radius
        fstp [eax]
        fmul radius
        fstp [eax+4]
    }
}

void Vector2::SetPolarEllipse(float angle, float radiusX, float radiusY)
{
    __asm
    {
        mov eax, this
        fld angle
        fsincos
        fmul radiusX
        fstp [eax]
        fmul radiusY
        fstp [eax+4]
    }
}
