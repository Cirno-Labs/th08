// Sakuya's bombs, from 0x40fcd0 to 0x410c40, with her effect callback:
// an object of the original, which ends with the inline copies only it
// uses (Rng::GetRandomU16InRange at 0x410c00). Split from Remilia's at a
// plausible seam: nothing in the binary shows where Sakuya's object starts.

#include "bullet.h"
#include "effect.h"
#include "enemy.h"
#include "item.h"
#include "player.h"
#include "screeneffect.h"
#include "sprite.h"
#include "unsorted.h"

void UpdateSakuyaBomb(Player *player)
{
    PlayerBomb *pb = &player->bomb;
    PlayerBombPart *blade;
    int i;

    if (pb->timer.HasTicked() && pb->timer == 0)
    {
        StartBomb(player, 0, "\x8c\xb6\x95\x84\x81\x75\x8e\x45\x90\x6c\x83\x68\x81\x5b\x83\x8b\x81\x76", 250, 290, 0);
        for (blade = pb->parts, i = 0; i < 0x60; i++, blade++)
        {
            blade->state = 0;
        }
        player->moveScaleY = 0.5f;
        player->moveScaleX = 0.5f;
        pb->parts[0].effect = g_EffectManager.SpawnEffect(0x14, &player->pos, 1, 0xffffffff);
        g_SoundPlayer.QueueSound(5, 0);
    }
    if (pb->timer >= 0 && pb->timer <= 60)
    {
        pb->parts[0].effect->pos = player->pos;
    }
    if (pb->timer >= 20 && pb->timer < 116)
    {
        for (blade = player->bomb.parts, i = 0; i < 0x60; i++, blade++)
        {
            Sprite *s;
            float angle;

            if (!player->bomb.timer.HasTickedTo(i % 48 * 2 + 20))
            {
                continue;
            }
            if (blade->state != 0)
            {
                return;
            }
            blade->state = 1;
            s = &blade->sprites[0];
            player->anm->StartScriptAtOrigin(s, 0x16);
            angle = i * 6.2831855f / 96.0f - 3.1415927f;
            blade->angle = angle;
            blade->speed = g_Random.GetRandomF32InRange(1.0f) + g_Half;
            blade->homingSpeed = g_Random.GetRandomF32InRange(0.1f) + 0.03f;
            blade->acceleration.x = g_Random.GetRandomU16InRange(1) ? 0.15707964f : -0.15707964f;
            blade->velocity.x = cosf(blade->angle) * 24.0f;
            blade->velocity.y = sinf(blade->angle) * 24.0f;
            blade->pos = player->pos + blade->velocity;
            blade->timer = 0;
            blade->velocity.z = 0.0f;
            blade->hitArea = player->AddHitAreaCircle(&blade->pos, 32.0f, 0.0f, 500, 6);
            blade->hitbox = player->AddBombHitboxCircle(&blade->pos, 32.0f, 0.0f, 20, 500);
        }
        g_SoundPlayer.QueueSound(6, 0);
        ScreenEffect::StartEffect(1, 0x78, 4, 1, 0, 0x15);
    }
    for (blade = pb->parts, i = 0; i < 0x60; i++, blade++)
    {
        if (blade->state == 0)
        {
            continue;
        }
        if (blade->timer.AsFrames() < 30 || blade->timer.AsFrames() >= 70)
        {
            if (blade->timer.HasTickedTo(70))
            {
                if (player->orbHomingTarget.x > -100.0f)
                {
                    blade->angle = AddNormalizeAngle(atan2f(player->orbHomingTarget.y - blade->pos.y, player->orbHomingTarget.x - blade->pos.x), 0.0f);
                }
                blade->speed = 14.0f;
            }
            blade->speed += blade->homingSpeed;
            blade->velocity.x = cosf(blade->angle) * blade->speed;
            blade->velocity.y = sinf(blade->angle) * blade->speed;
        }
        else
        {
            blade->angle = AddNormalizeAngle(blade->angle, blade->acceleration.x);
            blade->velocity.x = 0.0f;
            blade->velocity.y = 0.0f;
        }
        if (blade->hitbox != NULL)
        {
            blade->hitbox->x = blade->pos.x;
            blade->hitbox->y = blade->pos.y;
            blade->hitArea->x = blade->pos.x;
            blade->hitArea->y = blade->pos.y;
            if (blade->timer >= 120)
            {
                blade->hitbox->active = 0;
                blade->hitArea->active = 0;
                blade->hitArea = NULL;
                blade->hitbox = NULL;
            }
            else if (blade->hitbox->hits > 0)
            {
                player->anm->StartScriptAtOrigin(&blade->sprites[0], 0x17);
                g_EffectManager.SpawnEffect(0, &pb->parts[i].pos, 1, 0xffff80ff);
                blade->hitbox->active = 0;
                blade->hitArea->active = 0;
                blade->hitArea = NULL;
                blade->hitbox = NULL;
                g_SoundPlayer.QueueSoundPanned(0x2b, blade->pos.x);
            }
        }
        blade->pos += blade->velocity;
        g_SpriteEngine->Step(&blade->sprites[0]);
        blade->timer++;
    }
}

void DrawSakuyaBomb(Player *player)
{
    PlayerBombPart *blade;
    int i;

    DimPlayAreaForBomb(player, 0x80404040);
    for (blade = player->bomb.parts, i = 0; i < 0x60; i++, blade++)
    {
        if (blade->state == 0)
        {
            continue;
        }
        blade->sprites[0].SetRotationZ(blade->angle);
        blade->sprites[0].pos = blade->pos;
        blade->sprites[0].pos.x += g_GameManager.arena[0];
        blade->sprites[0].pos.y += g_GameManager.arena[1];
        blade->sprites[0].pos.z = 0.0f;
        g_SpriteEngine->Draw(&blade->sprites[0]);
    }
}

void UpdateSakuyaLastWord(Player *player)
{
    PlayerBomb *pb = &player->bomb;
    PlayerBombPart *blade;
    int i;

    if (pb->timer.HasTicked() && pb->timer == 0)
    {
        StartBomb(player, 0, "\x8c\xb6\x91\x92\x81\x75\x96\xe9\x96\xb6\x82\xcc\x8c\xb6\x89\x65\x8e\x45\x90\x6c\x8b\x53\x81\x76", 320, 350, 1);
        for (blade = pb->parts, i = 0; i < 0x80; i++, blade++)
        {
            blade->state = 0;
        }
        player->moveScaleY = 0.5f;
        player->moveScaleX = 0.5f;
        pb->parts[0].effect = g_EffectManager.SpawnEffect(0x14, &player->pos, 1, 0xffffffff);
        g_SoundPlayer.QueueSound(5, 0);
        ScreenEffect::StartEffect(1, 0x32, 4, 1, 0, 0x15);
    }
    if (pb->timer >= 0 && pb->timer <= 60)
    {
        pb->parts[0].effect->pos = player->pos;
    }
    if (pb->timer >= 20 && pb->timer < 148)
    {
        for (blade = player->bomb.parts, i = 0; i < 0x80; i++, blade++)
        {
            Sprite *s;
            float angle;

            if (!player->bomb.timer.HasTickedTo(i % 64 * 2 + 20))
            {
                continue;
            }
            if (blade->state != 0)
            {
                return;
            }
            blade->state = 1;
            s = &blade->sprites[0];
            player->anm->StartScriptAtOrigin(s, 0x14);
            angle = AddNormalizeAngle(i * 6.2831855f / 64.0f - 3.1415927f, 0.0f);
            blade->angle = angle;
            blade->speed = g_Random.GetRandomF32InRange(1.0f) + g_Half;
            blade->homingSpeed = g_Random.GetRandomF32InRange(0.1f) + 0.03f;
            blade->acceleration.x = g_Random.GetRandomU16InRange(1) ? 0.15707964f : -0.15707964f;
            blade->velocity.x = cosf(blade->angle) * 24.0f;
            blade->velocity.y = sinf(blade->angle) * 24.0f;
            blade->pos = player->pos + blade->velocity;
            blade->timer = 0;
            blade->velocity.z = 0.0f;
            blade->hitArea = player->AddHitAreaCircle(&blade->pos, 32.0f, 0.0f, 500, 6);
            blade->hitbox = player->AddBombHitboxCircle(&blade->pos, 32.0f, 0.0f, 30, 500);
        }
        g_SoundPlayer.QueueSound(6, 0);
    }
    for (blade = pb->parts, i = 0; i < 0x80; i++, blade++)
    {
        if (blade->state == 0)
        {
            continue;
        }
        if (blade->timer.AsFrames() < 30 || blade->timer.AsFrames() >= 70)
        {
            if (blade->timer.HasTickedTo(70))
            {
                if (player->orbHomingTarget.x > -100.0f)
                {
                    blade->angle = AddNormalizeAngle(atan2f(player->orbHomingTarget.y - blade->pos.y, player->orbHomingTarget.x - blade->pos.x), 0.0f);
                }
                blade->speed = 14.0f;
                g_EffectManager.SpawnEffect(0x2e, &blade->pos, 1, 0xffffffff);
            }
            blade->speed += blade->homingSpeed;
            blade->velocity.x = cosf(blade->angle) * blade->speed;
            blade->velocity.y = sinf(blade->angle) * blade->speed;
        }
        else
        {
            blade->angle = AddNormalizeAngle(blade->angle, blade->acceleration.x);
            blade->velocity.x = 0.0f;
            blade->velocity.y = 0.0f;
        }
        if (blade->hitbox != NULL)
        {
            blade->hitbox->x = blade->pos.x;
            blade->hitbox->y = blade->pos.y;
            blade->hitArea->x = blade->pos.x;
            blade->hitArea->y = blade->pos.y;
            if (blade->timer >= 120)
            {
                blade->hitbox->active = 0;
                blade->hitArea->active = 0;
                blade->hitArea = NULL;
                blade->hitbox = NULL;
            }
            else if (blade->hitbox->hits > 0)
            {
                if (i % 3 == 0)
                {
                    ScreenEffect::StartEffect(3, 2, 1, 0x208080ff, 0, 0x15);
                }
                g_EffectManager.SpawnEffect(0, &blade->pos, 1, 0xffff80ff);
                player->anm->StartScriptAtOrigin(&blade->sprites[0], 0x15);
                blade->hitbox->active = 0;
                blade->hitArea->active = 0;
                blade->hitArea = NULL;
                blade->hitbox = NULL;
                g_SoundPlayer.QueueSoundPanned(0x2b, blade->pos.x);
            }
        }
        blade->pos += blade->velocity;
        g_SpriteEngine->Step(&blade->sprites[0]);
        blade->timer++;
    }
}

void DrawSakuyaLastWord(Player *player)
{
    PlayerBombPart *blade;
    int i;

    DimPlayAreaForBomb(player, 0x80202080);
    for (blade = player->bomb.parts, i = 0; i < 0x60; i++, blade++)
    {
        if (blade->state == 0)
        {
            continue;
        }
        blade->sprites[0].SetRotationZ(blade->angle);
        blade->sprites[0].pos = blade->pos;
        blade->sprites[0].pos.x += g_GameManager.arena[0];
        blade->sprites[0].pos.y += g_GameManager.arena[1];
        blade->sprites[0].pos.z = 0.0f;
        g_SpriteEngine->Draw(&blade->sprites[0]);
    }
}

int EffectUpdateBombRing46(Effect *effect)
{
    effect->radius += 8.0f;
    effect->verticesDirty = 1;
    effect->segmentCount = 0xc;
    effect->ringWidth = 32.0f;
    return 1;
}
