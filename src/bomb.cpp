// The bombs, from 0x40bc60, as th07's bomb.cpp: the shared helpers, the
// per-shot bomb callbacks of g_BombFuncs and the effect callbacks of
// g_EffectInfos. Only part of it so far.

#include "effect.h"
#include "player.h"
#include "screeneffect.h"
#include "sprite.h"
#include "unsorted.h"

void DimPlayAreaForBomb(Player *player, D3DCOLOR tint)
{
    ColorBytes target;
    ColorBytes out;

    target.color = tint;
    if (player->bomb.timer < 60)
    {
        out.r = 0x80 - (0x80 - target.r) * player->bomb.timer.AsFrames() / 60;
        out.g = 0x80 - (0x80 - target.g) * player->bomb.timer.AsFrames() / 60;
        out.b = 0x80 - (0x80 - target.b) * player->bomb.timer.AsFrames() / 60;
    }
    else if (player->bomb.timer >= player->bomb.duration - 60)
    {
        out.r = 0x80 - (0x80 - target.r) * (player->bomb.duration - player->bomb.timer.AsFrames()) / 60;
        out.g = 0x80 - (0x80 - target.g) * (player->bomb.duration - player->bomb.timer.AsFrames()) / 60;
        out.b = 0x80 - (0x80 - target.b) * (player->bomb.duration - player->bomb.timer.AsFrames()) / 60;
    }
    else
    {
        out = target;
    }
    out.a = 0x80;
    g_SpriteEngine->ResetColorModulation();
    g_Stage.SetTintColor(out.color);
    g_Stage.keepTintColor = 1;
}

void SpawnBombAura(Player *player)
{
    Effect *effect;

    if (player->invulnerabilityEffect != NULL)
    {
        player->invulnerabilityEffect->active = 0;
    }
    effect = g_EffectManager.SpawnReservedEffect(0x17, &player->pos, 0, 1, 0xffffffff);
    effect->sprite.interpDuration[4] = 0;
    effect->sprite.interpFrame[4] = player->stateTimer;
    effect->sprite.scaleInterpEasing = 0;
    *(D3DXVECTOR2 *)&effect->sprite.scaleTween = *(D3DXVECTOR2 *)&effect->sprite.scaleY;
    effect->sprite.scaleTween.z = 0.0625f;
    effect->sprite.colorTween.x = 0.0625f;
    effect->sprite.scriptVar0 = player->stateTimer.AsFrames();
    effect->sprite.rotationSpeed.z *= -1.0f;
    effect->sprite.colorRgb[2] = 0xff;
    effect->sprite.colorRgb[1] = 0x40;
    effect->sprite.colorRgb[0] = 0x40;
    player->invulnerabilityEffect = effect;
}

int EffectUpdateBombRing42(Effect *effect)
{
    float t;

    t = (t = 1.0f - effect->timer.AsFloat() / 40.0f) * t;
    t = 1.0f - t;
    effect->radius = 256.0f * t;
    effect->segmentCount = 0x40;
    effect->verticesDirty = 1;
    effect->unk_334 = 5.0f;
    effect->unk_330 = 0.0f;
    if (effect->timer < 0x28)
    {
        effect->ringWidth = 8.0f;
    }
    else
    {
        effect->unk_32c = 64.0f * t;
        effect->ringWidth += 2.0f;
    }
    return 1;
}

int EffectUpdateBombRing43(Effect *effect)
{
    float t;

    t = (t = 1.0f - effect->timer.AsFloat() / 40.0f) * t;
    t = 1.0f - t;
    effect->radius = 256.0f * t;
    effect->segmentCount = 0x30;
    effect->verticesDirty = 1;
    effect->unk_334 = 0.0f;
    effect->unk_32c = 128.0f * t;
    effect->unk_330 = 0.7853982f;
    if (effect->timer < 0x28)
    {
        effect->ringWidth = 8.0f;
    }
    else
    {
        effect->ringWidth += 1.5f;
    }
    return 1;
}

int EffectUpdateBombRing44(Effect *effect)
{
    float t;

    t = (t = 1.0f - effect->timer.AsFloat() / 40.0f) * t;
    t = 1.0f - t;
    effect->radius = 256.0f * t;
    effect->segmentCount = 0x30;
    effect->verticesDirty = 1;
    effect->unk_32c = 128.0f * t;
    effect->unk_330 = -0.7853982f;
    if (effect->timer < 0x28)
    {
        effect->ringWidth = 8.0f;
    }
    else
    {
        effect->ringWidth += 1.5f;
    }
    return 1;
}

int EffectUpdateBombRing45(Effect *effect)
{
    float t;

    t = (t = 1.0f - effect->timer.AsFloat() / 40.0f) * t;
    t = 1.0f - t;
    effect->radius = 192.0f * t;
    effect->segmentCount = 8;
    effect->verticesDirty = 1;
    effect->ringWidth = 8.0f;
    return 1;
}

int EffectUpdateBombRing41(Effect *effect)
{
    effect->verticesDirty = 1;
    effect->segmentCount = 0x30;
    effect->ringWidth = 32.0f;
    effect->radius = (effect->timer.AsFrames() & 1 ? 8 : 0) + 64.0f;
    effect->angle = 0.0f;
    return 1;
}

int EffectUpdateBombRing46(Effect *effect)
{
    effect->radius += 8.0f;
    effect->verticesDirty = 1;
    effect->segmentCount = 0xc;
    effect->ringWidth = 32.0f;
    return 1;
}

int EffectUpdateBombRing36(Effect *effect)
{
    if (effect->timer < 0x28)
    {
        float t;

        t = 1.0f - effect->timer.AsFloat() / 40.0f;
        effect->ringWidth = 88.0f - effect->timer.AsFloat() * 80.0f / 40.0f;
        effect->radius = 192.0f - 384.0f * t * t;
        effect->segmentCount--;
        effect->verticesDirty = 1;
    }
    else
    {
        if (effect->timer == 0x28)
        {
            int i;
            PlayerBombHitbox *laser;
            float angle;
            float spread;

            ScreenEffect::StartEffect(3, 8, 1, 0x8ff08080, 0, 0x15);
            angle = effect->angle + 0.7853982f;
            spread = effect->radius * 0.70710671f;
            D3DXVECTOR3 pos;
            g_SpriteEngine->FillVerticalStripVertices(&effect->sprite, effect->vertices, effect->segmentCount * 2 + 2);
            for (i = 0; i < 4; i++)
            {
                if (angle >= 3.1415927f)
                {
                    angle -= 6.2831855f;
                }
                ((Vector2 *)&pos)->SetPolar(angle, spread);
                pos += effect->origin;
                laser = g_Player.AddBombHitbox(&pos, spread * 8.0f, effect->ringWidth * 4.0f, 0x3c, 0x46);
                laser->damageInterval = 4;
                laser->angle = AddNormalizeAngle(angle, 1.5707964f);
                angle = laser->angle;
                laser = g_Player.AddHitArea(&pos, spread * 4.0f, effect->ringWidth * 4.0f, 6, 100);
                laser->angle = angle;
            }
        }
        effect->verticesDirty = 1;
    }
    return 1;
}

int EffectInitBombRing36(Effect *effect)
{
    D3DXVECTOR3 pos = effect->pos;
    D3DXVECTOR3 velocity = effect->velocity;

    g_EffectManager.SpawnReservedWithVelocity(0x23, &pos, &velocity, effect->reservedSlot, 1, 0xffffffff);
    effect->update = EffectUpdateBombRing36;
    effect->segmentCount = 0x2c;
    effect->ringWidth = 4.0f;
    return 0;
}

int EffectUpdateBombRing37(Effect *effect)
{
    effect->angle = AddNormalizeAngle(effect->angle, effect->reservedSlot & 1 ? 0.03926991f : -0.03926991f);
    effect->verticesDirty = 1;
    if (effect->timer < 0x32)
    {
        float t;
        float depth;

        t = 1.0f - effect->timer.AsFloat() / 50.0f;
        depth = (effect->reservedSlot - 4) * 32.0f + 384.0f;
        effect->ringWidth = 88.0f - effect->timer.AsFloat() * 80.0f / 50.0f;
        effect->radius = (effect->reservedSlot - 4) * 32.0f + 192.0f - depth * t * t;
        effect->segmentCount--;
    }
    else
    {
        if (effect->timer == 0x32)
        {
            int i;
            PlayerBombHitbox *laser;
            float angle;
            float spread;

            ScreenEffect::StartEffect(1, 0x10, 8, 0, 0, 0x15);
            ScreenEffect::StartEffect(3, 8, 1, 0x8f6060f0, 0, 0x15);
            angle = effect->angle + 0.7853982f;
            spread = effect->radius * 0.70710671f;
            D3DXVECTOR3 pos;
            g_SpriteEngine->FillVerticalStripVertices(&effect->sprite, effect->vertices, effect->segmentCount * 2 + 2);
            for (i = 0; i < 4; i++)
            {
                if (angle >= 3.1415927f)
                {
                    angle -= 6.2831855f;
                }
                ((Vector2 *)&pos)->SetPolar(angle, spread);
                pos += effect->origin;
                laser = g_Player.AddBombHitbox(&pos, spread * 8.0f, effect->ringWidth * 4.0f, 0x3c, 100);
                laser->damageInterval = 2;
                laser->angle = AddNormalizeAngle(angle, 1.5707964f);
                angle = laser->angle;
                laser = g_Player.AddHitArea(&pos, spread * 4.0f, effect->ringWidth * 4.0f, 6, 0x96);
                laser->angle = angle;
            }
        }
        effect->verticesDirty = 1;
    }
    return 1;
}

int EffectInitBombRing37(Effect *effect)
{
    D3DXVECTOR3 pos = effect->pos;
    D3DXVECTOR3 velocity = effect->velocity;

    g_EffectManager.SpawnReservedWithVelocity(0x23, &pos, &velocity, effect->reservedSlot, 1, 0xffffffff);
    effect->update = EffectUpdateBombRing37;
    effect->segmentCount = 0x36;
    effect->ringWidth = 6.0f;
    return 0;
}

int EffectUpdateBombRing48(Effect *effect)
{
    float t;

    if (effect->timer < 0x1e)
    {
        effect->radius = 192.0f;
        effect->segmentCount = 0x30;
        effect->ringWidth = 3.0f;
        effect->unk_32c = 0.0001f;
        effect->unk_330 = 1.5707964f;
    }
    else
    {
        t = (t = (t = (effect->timer.AsFloat() - 30.0f) / 30.0f) * t) * t;
        effect->unk_32c = 192.0f * t + 0.0001f;
        effect->ringWidth = 80.0f * t + 3.0f;
    }
    effect->verticesDirty = 1;
    return 1;
}
