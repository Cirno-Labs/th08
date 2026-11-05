// Yuyuko's bombs, from 0x413140 to 0x4143e0: split from Youmu's at a
// plausible seam (nothing in the binary shows this boundary; the
// original's bomb objects come in alphabetical order of the character).

#include "bullet.h"
#include "effect.h"
#include "enemy.h"
#include "item.h"
#include "player.h"
#include "screeneffect.h"
#include "sprite.h"
#include "unsorted.h"

void UpdateYuyukoBomb(Player *player)
{
    PlayerBomb *pb = &player->bomb;
    PlayerBombPart *butterfly;
    int i;

    if (pb->timer.HasTicked() && pb->timer == 0)
    {
        StartBomb(player, 1, "\x8e\x80\x95\x84\x81\x75\x83\x4d\x83\x83\x83\x58\x83\x67\x83\x8a\x83\x68\x83\x8a\x81\x5b\x83\x80\x81\x76", 300, 350, 0);
        for (butterfly = pb->parts, i = 0; i < 0x80; i++, butterfly++)
        {
            butterfly->state = 0;
        }
        for (butterfly = pb->parts, i = 0; i < 0x10; i++, butterfly++)
        {
            butterfly->state = 1;
            player->anm->StartScript(&butterfly->sprites[0], 0x12);
            butterfly->angle = i * 3.1415927f / 8.0f - 3.1415927f;
            butterfly->homingSpeed = 0.01308997f;
            butterfly->pos = butterfly->trail[0] = player->pos;
            butterfly->hitbox = player->AddBombHitboxCircle(&butterfly->pos, 24.0f, 0.0f, 50, 500);
            butterfly->hitArea = player->AddHitAreaCircle(&butterfly->pos, 24.0f, 0.0f, 500, 6);
            butterfly->hitbox->maxDamage = 800;
            butterfly->velocity.x = 0.0f;
            butterfly->velocity.y = 2.0f;
        }
        for (i = 0; i < 0x10; i++, butterfly++)
        {
            butterfly->state = 1;
            player->anm->StartScript(&butterfly->sprites[0], 0x13);
            butterfly->angle = i * 3.1415927f / 8.0f - 3.1415927f;
            butterfly->homingSpeed = -0.01308997f;
            butterfly->pos = butterfly->trail[0] = player->pos;
            butterfly->hitbox = player->AddBombHitboxCircle(&butterfly->pos, 24.0f, 0.0f, 50, 500);
            butterfly->hitArea = player->AddHitAreaCircle(&butterfly->pos, 24.0f, 0.0f, 500, 6);
            butterfly->hitbox->maxDamage = 800;
            butterfly->velocity.x = 0.0f;
            butterfly->velocity.y = 2.0f;
        }
        for (i = 0; i < 0x10; i++, butterfly++)
        {
            butterfly->state = 1;
            player->anm->StartScript(&butterfly->sprites[0], 0x12);
            butterfly->angle = i * 3.1415927f / 8.0f - 3.1415927f;
            butterfly->homingSpeed = 0.015707964f;
            butterfly->pos = butterfly->trail[0] = player->pos;
            butterfly->hitbox = player->AddBombHitboxCircle(&butterfly->pos, 24.0f, 0.0f, 50, 500);
            butterfly->hitArea = player->AddHitAreaCircle(&butterfly->pos, 24.0f, 0.0f, 500, 6);
            butterfly->hitbox->maxDamage = 800;
            butterfly->velocity.x = 0.0f;
            butterfly->velocity.y = 1.5f;
        }
        for (i = 0; i < 0x10; i++, butterfly++)
        {
            butterfly->state = 1;
            player->anm->StartScript(&butterfly->sprites[0], 0x13);
            butterfly->angle = i * 3.1415927f / 8.0f - 3.1415927f;
            butterfly->homingSpeed = -0.015707964f;
            butterfly->pos = butterfly->trail[0] = player->pos;
            butterfly->hitbox = player->AddBombHitboxCircle(&butterfly->pos, 24.0f, 0.0f, 50, 500);
            butterfly->hitArea = player->AddHitAreaCircle(&butterfly->pos, 24.0f, 0.0f, 500, 6);
            butterfly->hitbox->maxDamage = 800;
            butterfly->velocity.x = 0.0f;
            butterfly->velocity.y = 1.5f;
        }
        player->moveScaleY = 0.8f;
        player->moveScaleX = 0.8f;
        pb->parts[0].effect = g_EffectManager.SpawnEffect(0x14, &player->pos, 1, 0xffffffff);
        g_SoundPlayer.QueueSound(5, 0);
        ScreenEffect::StartEffect(1, 0x78, 0xc, 0, 0, 0x15);
    }
    for (butterfly = pb->parts, i = 0; i < 0x80; i++, butterfly++)
    {
        if (butterfly->state == 0)
        {
            continue;
        }
        butterfly->trail[1] = butterfly->pos;
        butterfly->velocity.x += butterfly->velocity.y;
        butterfly->angle = AddNormalizeAngle(butterfly->angle, butterfly->homingSpeed);
        ((Vector2 *)&butterfly->pos)->SetPolar(butterfly->angle, butterfly->velocity.x);
        butterfly->pos += butterfly->trail[0];
        butterfly->trail[1] = butterfly->pos - butterfly->trail[1];
        if (butterfly->velocity.x >= 500.0f)
        {
            butterfly->state = 0;
            butterfly->hitbox->active = 0;
            butterfly->hitArea->active = 0;
            continue;
        }
        if (butterfly->hitbox != NULL)
        {
            butterfly->hitbox->x = butterfly->pos.x;
            butterfly->hitbox->y = butterfly->pos.y;
            butterfly->hitArea->x = butterfly->pos.x;
            butterfly->hitArea->y = butterfly->pos.y;
        }
        if (g_SpriteEngine->Step(&butterfly->sprites[0]))
        {
            butterfly->state = 0;
            butterfly->hitbox->active = 0;
            butterfly->hitArea->active = 0;
        }
    }
}

void DrawYuyukoBomb(Player *player)
{
    PlayerBombPart *butterfly;
    int i;

    DimPlayAreaForBomb(player, 0x80404040);
    for (butterfly = player->bomb.parts, i = 0; i < 0x60; i++, butterfly++)
    {
        if (butterfly->state == 0)
        {
            continue;
        }
        butterfly->sprites[0].SetRotationZ(atan2f(butterfly->trail[1].y, butterfly->trail[1].x));
        butterfly->sprites[0].pos = butterfly->pos;
        butterfly->sprites[0].pos.x += g_GameManager.arena[0];
        butterfly->sprites[0].pos.y += g_GameManager.arena[1];
        butterfly->sprites[0].pos.z = 0.0f;
        g_SpriteEngine->Draw(&butterfly->sprites[0]);
    }
}

void UpdateYuyukoLastWord(Player *player)
{
    PlayerBomb *pb = &player->bomb;
    PlayerBombPart *butterfly;
    int i;

    if (pb->timer.HasTicked() && pb->timer == 0)
    {
        StartBomb(player, 1, "\x8e\x80\x92\xb1\x81\x75\x89\xd8\xe3\xef\x82\xcc\x89\x69\x96\xb0\x81\x76", 300, 350, 1);
        for (butterfly = pb->parts, i = 0; i < 0x80; i++, butterfly++)
        {
            butterfly->state = 0;
        }
        for (butterfly = pb->parts, i = 0; i < 0x10; i++, butterfly++)
        {
            butterfly->state = 1;
            player->anm->StartScript(&butterfly->sprites[0], 0x12);
            butterfly->angle = i * 3.1415927f / 8.0f - 3.1415927f;
            butterfly->homingSpeed = 0.01308997f;
            butterfly->pos = butterfly->trail[0] = player->pos;
            butterfly->hitbox = player->AddBombHitboxCircle(&butterfly->pos, 24.0f, 0.0f, 50, 500);
            butterfly->hitArea = player->AddHitAreaCircle(&butterfly->pos, 24.0f, 0.0f, 500, 6);
            butterfly->hitbox->maxDamage = 800;
            butterfly->velocity.x = 0.0f;
            butterfly->velocity.y = 2.0f;
        }
        for (i = 0; i < 0x10; i++, butterfly++)
        {
            butterfly->state = 1;
            player->anm->StartScript(&butterfly->sprites[0], 0x13);
            butterfly->angle = i * 3.1415927f / 8.0f - 3.1415927f;
            butterfly->homingSpeed = -0.01308997f;
            butterfly->pos = butterfly->trail[0] = player->pos;
            butterfly->hitbox = player->AddBombHitboxCircle(&butterfly->pos, 24.0f, 0.0f, 50, 500);
            butterfly->hitArea = player->AddHitAreaCircle(&butterfly->pos, 24.0f, 0.0f, 500, 6);
            butterfly->hitbox->maxDamage = 800;
            butterfly->velocity.x = 0.0f;
            butterfly->velocity.y = 2.0f;
        }
        for (i = 0; i < 0x10; i++, butterfly++)
        {
            butterfly->state = 1;
            player->anm->StartScript(&butterfly->sprites[0], 0x12);
            butterfly->angle = i * 3.1415927f / 8.0f - 3.1415927f;
            butterfly->homingSpeed = 0.015707964f;
            butterfly->pos = butterfly->trail[0] = player->pos;
            butterfly->hitbox = player->AddBombHitboxCircle(&butterfly->pos, 24.0f, 0.0f, 50, 500);
            butterfly->hitArea = player->AddHitAreaCircle(&butterfly->pos, 24.0f, 0.0f, 500, 6);
            butterfly->hitbox->maxDamage = 800;
            butterfly->velocity.x = 0.0f;
            butterfly->velocity.y = 1.5f;
        }
        for (i = 0; i < 0x10; i++, butterfly++)
        {
            butterfly->state = 1;
            player->anm->StartScript(&butterfly->sprites[0], 0x13);
            butterfly->angle = i * 3.1415927f / 8.0f - 3.1415927f;
            butterfly->homingSpeed = -0.015707964f;
            butterfly->pos = butterfly->trail[0] = player->pos;
            butterfly->hitbox = player->AddBombHitboxCircle(&butterfly->pos, 24.0f, 0.0f, 50, 500);
            butterfly->hitArea = player->AddHitAreaCircle(&butterfly->pos, 24.0f, 0.0f, 500, 6);
            butterfly->hitbox->maxDamage = 800;
            butterfly->velocity.x = 0.0f;
            butterfly->velocity.y = 1.5f;
        }
        player->moveScaleY = 0.8f;
        player->moveScaleX = 0.8f;
        pb->parts[0].effect = g_EffectManager.SpawnEffect(0x14, &player->pos, 1, 0xffffffff);
        g_SoundPlayer.QueueSound(5, 0);
        ScreenEffect::StartEffect(1, 0x3c, 0x10, 0, 0, 0x15);
    }
    if (pb->timer >= 60 && pb->timer < 200 && pb->timer.HasTickedEvery(20))
    {
        int count;
        float base;

        base = g_Random.GetRandomF32SignedInRange(3.1415927f);
        count = 0;
        for (butterfly = pb->parts, i = 0; i < 0x80; i++, butterfly++)
        {
            if (butterfly->state != 0)
            {
                continue;
            }
            butterfly->state = 1;
            player->anm->StartScript(&butterfly->sprites[0], 0x14);
            butterfly->angle = AddNormalizeAngle(i * 3.1415927f / 8.0f, base);
            butterfly->pos = butterfly->trail[0] = player->pos;
            butterfly->hitbox = player->AddBombHitboxCircle(&butterfly->pos, 64.0f, 0.0f, 100, 500);
            butterfly->hitbox->maxDamage = 1200;
            butterfly->hitArea = player->AddHitAreaCircle(&butterfly->pos, 64.0f, 0.0f, 500, 6);
            butterfly->velocity.x = 0.0f;
            butterfly->velocity.y = 8.0f;
            count++;
            if (count >= 0x10)
            {
                break;
            }
        }
        g_SoundPlayer.QueueSoundPanned(0xf, butterfly->pos.x);
        ScreenEffect::StartEffect(1, 0x1e, 8, 0, 0, 0x15);
        ScreenEffect::StartEffect(3, 8, 1, 0xe0f0f0f0, 0, 0x15);
    }
    for (butterfly = pb->parts, i = 0; i < 0x80; i++, butterfly++)
    {
        if (butterfly->state == 0)
        {
            continue;
        }
        butterfly->trail[1] = butterfly->pos;
        butterfly->velocity.x += butterfly->velocity.y;
        butterfly->angle = AddNormalizeAngle(butterfly->angle, butterfly->homingSpeed);
        ((Vector2 *)&butterfly->pos)->SetPolar(butterfly->angle, butterfly->velocity.x);
        butterfly->pos += butterfly->trail[0];
        butterfly->trail[1] = butterfly->pos - butterfly->trail[1];
        if (butterfly->velocity.x >= 500.0f)
        {
            butterfly->state = 0;
            butterfly->hitbox->active = 0;
            butterfly->hitArea->active = 0;
            continue;
        }
        if (butterfly->hitbox != NULL)
        {
            butterfly->hitbox->x = butterfly->pos.x;
            butterfly->hitbox->y = butterfly->pos.y;
            butterfly->hitArea->x = butterfly->pos.x;
            butterfly->hitArea->y = butterfly->pos.y;
        }
        if (g_SpriteEngine->Step(&butterfly->sprites[0]))
        {
            butterfly->state = 0;
            butterfly->hitbox->active = 0;
            butterfly->hitArea->active = 0;
        }
    }
}

void DrawYuyukoLastWord(Player *player)
{
    PlayerBombPart *butterfly;
    int i;

    DimPlayAreaForBomb(player, 0x80802020);
    for (butterfly = player->bomb.parts, i = 0; i < 0x80; i++, butterfly++)
    {
        if (butterfly->state == 0)
        {
            continue;
        }
        butterfly->sprites[0].SetRotationZ(atan2f(butterfly->trail[1].y, butterfly->trail[1].x));
        butterfly->sprites[0].pos = butterfly->pos;
        butterfly->sprites[0].pos.x += g_GameManager.arena[0];
        butterfly->sprites[0].pos.y += g_GameManager.arena[1];
        butterfly->sprites[0].pos.z = 0.0f;
        g_SpriteEngine->Draw(&butterfly->sprites[0]);
    }
}
