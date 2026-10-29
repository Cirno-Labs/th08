// The bombs, from 0x40bc60, as th07's bomb.cpp: the shared helpers, the
// per-shot bomb callbacks of g_BombFuncs and the effect callbacks of
// g_EffectInfos. Only part of it so far.

#include "effect.h"
#include "enemy.h"
#include "item.h"
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

void StartBomb(Player *player, int face, char *name, int duration, int stateFrames, int sprite)
{
    DWORD i;
    PlayerBombPart *part;
    PlayerBomb *pb = &player->bomb;

    g_SpellCard.StartBombCard(face, name, sprite);
    pb->duration = duration;
    player->stateTimer = stateFrames;
    player->playerState = 3;
    SpawnBombAura(player);
    for (i = 0, part = pb->parts; i < 0x80; i++, part++)
    {
        part->state = 0;
    }
    g_ItemManager.CollectAll();
    pb->startPos = player->pos;
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

void UpdateReimuBomb(Player *player)
{
    PlayerBomb *pb = &player->bomb;
    PlayerBombPart *orbs;
    DWORD i;

    if (pb->timer.HasTicked() && pb->timer == 0)
    {
        float angle;

        StartBomb(player, 0, "\x97\xec\x95\x84\x81\x75\x96\xb2\x91\x7a\x96\xad\x8e\xec\x81\x76", 200, 260, 0);
        g_EffectManager.SpawnEffect(0xc, &player->pos, 1, 0xff4040ff);
        angle = -3.1415927f;
        for (orbs = pb->parts, i = 0; (int)i < 0x10; i++, orbs++)
        {
            player->anm->StartScript(&pb->parts[i].sprites[0], 0x13);
            pb->parts[i].angle = angle;
            angle += 0.3926991f;
            orbs->pos = player->pos;
            orbs->trail[0] = orbs->pos;
            orbs->unk_8 = 0.0f;
            orbs->state = 1;
            orbs->hitArea = player->AddHitAreaCircle(&player->pos, 96.0f, 0.0f, 200, 6);
            orbs->hitbox = player->AddBombHitboxCircle(&orbs->pos, 64.0f, 0.0f, 5, 200);
            orbs->hitbox->damageInterval = 2;
            orbs->hitbox->maxDamage = 200;
            orbs->hitbox->unk_3d = 1;
        }
        g_SoundPlayer.QueueSound(0xd, 0);
    }
    if (pb->timer < 40)
    {
        D3DXVECTOR3 oldPos;

        for (orbs = pb->parts, i = 0; (int)i < 0x10; i++, orbs++)
        {
            orbs->angle = AddNormalizeAngle(orbs->angle, i & 1 ? 0.05235988f : -0.05235988f);
            oldPos = orbs->pos;
            orbs->pos.x = sinf(orbs->angle) * orbs->unk_8 + orbs->trail[0].x;
            orbs->pos.y = cosf(orbs->angle) * orbs->unk_8 + orbs->trail[0].y;
            orbs->unk_8 += 3.2f;
            orbs->velocity = orbs->pos - oldPos;
        }
    }
    else
    {
        D3DXVECTOR3 target;
        float x;
        float y;
        float len;
        PlayerBombHitbox *hitbox;

        if (pb->timer == 40)
        {
            for (orbs = pb->parts, i = 0; (int)i < 0x10; i++, orbs++)
            {
                orbs->speed = sqrtf(orbs->velocity.x * orbs->velocity.x + orbs->velocity.y * orbs->velocity.y);
                orbs->angle = atan2f(orbs->velocity.x, orbs->velocity.y);
                orbs->burstTimer = 0;
                orbs->unk_8 = 8.0f;
            }
        }
        for (i = 0, orbs = pb->parts; (int)i < 0x10; i++, orbs++)
        {
            if (orbs->state == 0)
            {
                continue;
            }
            if (orbs->state == 1 && pb->timer.HasTicked())
            {
                if (player->orbHomingTarget.x > -100.0f)
                {
                    target = player->orbHomingTarget;
                }
                else
                {
                    target = player->pos;
                }
                x = target[0] - orbs->pos.x;
                y = target[1] - orbs->pos.y;
                len = sqrtf(x * x + y * y) / (orbs->unk_8 / 8.0f);
                if (len < 1.0f)
                {
                    len = 1.0f;
                }
                x = x / len + orbs->velocity.x;
                y = y / len + orbs->velocity.y;
                len = sqrtf(x * x + y * y);
                orbs->unk_8 = (len > 10.0f) ? 10.0f : len;
                if (orbs->unk_8 < 1.0f)
                {
                    orbs->unk_8 = 1.0f;
                }
                orbs->velocity.x = x * orbs->unk_8 / len;
                orbs->velocity.y = y * orbs->unk_8 / len;
                player->AddHitAreaCircle(&orbs->pos, 128.0f, 0.0f, 0, 6);
                if (orbs->hitbox->hits >= orbs->hitbox->maxDamage || pb->timer >= pb->duration - 30)
                {
                    orbs->hitArea->active = 0;
                    orbs->hitbox->active = 0;
                    player->AddHitAreaCircle(&player->pos, 64.0f, 4.2666669f, 30, 6);
                    hitbox = player->AddBombHitboxCircle(&orbs->pos, 64.0f, 12.8f, 500, 12);
                    hitbox->damageInterval = 4;
                    hitbox->maxDamage = 0;
                    g_EffectManager.SpawnEffect(6, &orbs->pos, 8, 0xffffffff);
                    orbs->state = 2;
                    orbs->sprites[0].pendingInterrupt = 1;
                    orbs->velocity / 8.0f;
                    g_SoundPlayer.QueueSoundPanned(0xf, orbs->pos.x);
                    ScreenEffect::StartEffect(1, 0x10, 8, 0, 0, 0x15);
                }
            }
            orbs->pos.x += g_App.gameSpeed * orbs->velocity.x;
            orbs->pos.y += g_App.gameSpeed * orbs->velocity.y;
        }
    }
    for (i = 0, orbs = pb->parts; (int)i < 0x10; i++, orbs++)
    {
        if (orbs->state == 0)
        {
            continue;
        }
        if (orbs->state == 1)
        {
            orbs->hitArea->x = orbs->pos.x;
            orbs->hitArea->y = orbs->pos.y;
            orbs->hitbox->x = orbs->pos.x;
            orbs->hitbox->y = orbs->pos.y;
        }
        else if (orbs->state != 0 && pb->timer.HasTicked())
        {
            orbs->burstTimer++;
            if (orbs->burstTimer >= 30)
            {
                orbs->state = 0;
            }
        }
        g_SpriteEngine->Step(&orbs->sprites[0]);
    }
}

void DrawReimuBomb(Player *player)
{
    Sprite *sprite;
    int k;
    PlayerBombPart *orbs;

    DimPlayAreaForBomb(player, 0x80404040);
    for (k = 0, orbs = player->bomb.parts; k < 0x10; k++, orbs++)
    {
        if (orbs->state == 0)
        {
            continue;
        }
        sprite = &orbs->sprites[0];
        sprite->pos = orbs->pos + sprite->posOffset;
        sprite->pos.x += g_GameManager.arena[0];
        sprite->pos.y += g_GameManager.arena[1];
        sprite->pos.z = 0.0f;
        g_SpriteEngine->DrawUnrotated(sprite);
    }
}

void UpdateReimuLastWord(Player *player)
{
    PlayerBomb *pb = &player->bomb;
    PlayerBombPart *orbs;
    DWORD i;

    if (pb->timer.HasTicked() && pb->timer == 0)
    {
        float angle;

        StartBomb(player, 0, "\x90\x5f\x97\xec\x81\x75\x96\xb2\x91\x7a\x95\x95\x88\xf3\x81\x40\x8f\x75\x81\x76", 200, 260, 1);
        g_EffectManager.SpawnEffect(0xc, &player->pos, 1, 0xff4040ff);
        angle = -3.1415927f;
        for (orbs = pb->parts, i = 0; (int)i < 0x10; i++, orbs++)
        {
            player->anm->StartScript(&pb->parts[i].sprites[0], 0x13);
            pb->parts[i].angle = angle;
            angle += 0.3926991f;
            orbs->pos = player->pos;
            orbs->trail[0] = orbs->pos;
            orbs->unk_8 = 0.0f;
            orbs->state = 1;
            orbs->hitArea = player->AddHitAreaCircle(&player->pos, 96.0f, 0.0f, 200, 6);
            orbs->hitbox = player->AddBombHitboxCircle(&orbs->pos, 64.0f, 0.0f, 5, 200);
            orbs->hitbox->damageInterval = 2;
            orbs->hitbox->maxDamage = 200;
            orbs->hitbox->unk_3d = 1;
        }
        pb->unk_14 = 0;
        g_SoundPlayer.QueueSound(0xd, 0);
    }
    {
        D3DXVECTOR3 oldPos;
        PlayerBombHitbox *hitbox;

        for (orbs = pb->parts, i = 0; (int)i < 0x10; i++, orbs++)
        {
            if (orbs->state == 1)
            {
                orbs->angle = AddNormalizeAngle(orbs->angle, i & 1 ? 0.05235988f : -0.05235988f);
                oldPos = orbs->pos;
                orbs->pos.x = sinf(orbs->angle) * orbs->unk_8 + orbs->trail[0].x;
                orbs->pos.y = cosf(orbs->angle) * orbs->unk_8 + orbs->trail[0].y;
                if (pb->timer < 40)
                {
                    if (i & 1)
                    {
                        orbs->unk_8 += 1.2f;
                    }
                    else
                    {
                        orbs->unk_8 += 2.4f;
                    }
                }
                orbs->velocity = orbs->pos - oldPos;
                if (pb->timer >= pb->duration - 40 - i)
                {
                    orbs->hitArea->active = 0;
                    orbs->hitbox->active = 0;
                    player->AddHitAreaCircle(&player->pos, 64.0f, 4.2666669f, 30, 6);
                    hitbox = player->AddBombHitboxCircle(&orbs->pos, 64.0f, 8.5333338f, 25, 15);
                    hitbox->damageInterval = 5;
                    hitbox->maxDamage = 50;
                    g_EffectManager.SpawnEffect(6, &orbs->pos, 8, 0xffffffff);
                    orbs->state = 2;
                    orbs->sprites[0].pendingInterrupt = 1;
                    orbs->velocity / 8.0f;
                    g_SoundPlayer.QueueSoundPanned(0xf, orbs->pos.x);
                    ScreenEffect::StartEffect(1, 0x10, 8, 0, 0, 0x15);
                }
                orbs->hitArea->x = orbs->pos.x;
                orbs->hitArea->y = orbs->pos.y;
                orbs->hitbox->x = orbs->pos.x;
                orbs->hitbox->y = orbs->pos.y;
            }
            g_SpriteEngine->Step(&orbs->sprites[0]);
        }
    }
    if (pb->timer >= 40)
    {
        D3DXVECTOR3 target;
        D3DCOLOR color;
        PlayerBombHitbox *hitbox;

        if (pb->timer % 20 == 0)
        {
            orbs = &pb->parts[pb->unk_14 + 0x10];
            player->anm->StartScript(&orbs->sprites[0], 0x14);
            pb->unk_14 = 1;
            orbs->state = 1;
            color = g_BombFlashColors[(DWORD)(pb->timer.AsFrames() / 20) % 7];
            if (player->orbHomingTarget.x > -100.0f)
            {
                target = player->orbHomingTarget;
            }
            else
            {
                target.x = g_Rng.GetRandomF32InRange(320.0f) + 32.0f;
                target.y = g_Rng.GetRandomF32InRange(384.0f) + 32.0f;
                target.z = 0.0f;
            }
            orbs->pos = target;
            g_EffectManager.SpawnEffect(0x31, &orbs->pos, 1, color);
            g_EffectManager.SpawnEffect(0x37, &orbs->pos, 1, color);
            orbs->hitArea = player->AddHitAreaCircle(&target, 64.0f, 4.2666669f, 30, 6);
            orbs->hitbox = player->AddBombHitboxCircle(&target, 64.0f, 8.5333338f, 400, 15);
            hitbox = orbs->hitbox;
            hitbox->damageInterval = 2;
            g_SoundPlayer.QueueSoundPanned(0xf, orbs->pos.x);
            ScreenEffect::StartEffect(1, 0x10, 8, 0, 0, 0x15);
            ScreenEffect::StartEffect(3, 8, 1, color, 0, 0x15);
        }
    }
    for (orbs = &pb->parts[0x10], i = 0x10; i < 0x80; i++, orbs++)
    {
        if (orbs->state == 0)
        {
            continue;
        }
        orbs->hitArea->x = orbs->pos.x;
        orbs->hitArea->y = orbs->pos.y;
        orbs->hitbox->x = orbs->pos.x;
        orbs->hitbox->y = orbs->pos.y;
        if (g_SpriteEngine->Step(&orbs->sprites[0]))
        {
            orbs->state = 0;
        }
    }
}

void UpdateDissolveBomb(Player *player)
{
    PlayerBomb *bomb = &player->bomb;
    Effect *effect;
    DWORD j;

    if (bomb->timer.HasTicked() && bomb->timer == 0)
    {
        if (!g_GameManager.IsInSpellCardPractice())
        {
            StartBomb(player, -1, "\x81\x75\x83\x66\x83\x42\x83\x5d\x83\x8b\x83\x75\x83\x58\x83\x79\x83\x8b\x81\x76", 120, 200, 0);
        }
        else
        {
            StartBomb(player, -1, "\x81\x75\x83\x66\x83\x42\x83\x5d\x83\x8b\x83\x75\x83\x58\x83\x79\x83\x8b\x81\x76", 40, 200, 0);
        }
        g_EffectManager.SpawnEffect(0xc, &player->pos, 1, 0xff4040ff);
        effect = g_EffectManager.SpawnReservedEffect(0x32, &player->pos, 4, 1, 0xff4040ff);
        effect->sprite.interpDuration[0] = 0;
        if (!g_GameManager.IsInSpellCardPractice())
        {
            effect->sprite.interpFrame[0] = 90;
        }
        else
        {
            effect->sprite.interpFrame[0] = 30;
        }
        effect->sprite.interpEasing = 5;
        effect->sprite.moveStart.x = 8.0f;
        effect->sprite.moveEnd.x = 128.0f;
        effect->sprite.moveStart.y = 64.0f;
        effect->sprite.moveEnd.y = 0.0f;
        effect->sprite.pos.x = 8.0f;
        effect->sprite.pos.y = 64.0f;
        effect->segmentCount = 0x40;
        effect->angle = 0.0f;
        effect->radius = 8.0f;
        effect->ringWidth = 15.0f;
        effect->unk_334 = 6.0f;
        g_SoundPlayer.QueueSound(0xd, 0);
        player->moveScaleY = 0.0f;
        player->moveScaleX = 0.0f;
        for (j = 0; j < 8; j++)
        {
            if (g_EnemyManager.bosses[j] != NULL)
            {
                g_EnemyManager.bosses[j]->unk_3324_6 = 0;
            }
        }
    }
}

void DrawDissolveBomb(Player *player)
{
    PlayerBomb *bomb = &player->bomb;
    int shade;

    if (bomb->timer < 60)
    {
        DimPlayAreaForBomb(player, 0x80404040);
    }
    else
    {
        shade = (bomb->timer.AsFrames() - 60) * 0xb0 / 60 + 0x40;
        DimPlayAreaForBomb(player, shade << 16 | 0x80000000 | shade << 8 | shade);
    }
}

void DrawAliceBomb(Player *player)
{
    DimPlayAreaForBomb(player, 0x80404040);
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

void DrawRemiliaBomb(Player *player)
{
    DimPlayAreaForBomb(player, 0x80d02020);
}

void DrawRemiliaLastWord(Player *player)
{
    DimPlayAreaForBomb(player, 0x80f00000);
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
