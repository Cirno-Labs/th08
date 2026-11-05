// Youmu's bombs, from 0x411b10 to 0x413140, with her effect callback:
// split from Yukari's at a plausible seam (nothing in the binary shows
// this boundary; the original's bomb objects come in alphabetical order
// of the character).

#include "bullet.h"
#include "effect.h"
#include "enemy.h"
#include "item.h"
#include "player.h"
#include "screeneffect.h"
#include "sprite.h"
#include "unsorted.h"

void UpdateYoumuBomb(Player *player)
{
    PlayerBomb *b = &player->bomb;
    int x = 0;
    int y = 0;
    PlayerBombPart *part = b->parts;
    float t;

    if (b->timer.HasTicked() && b->timer == 0)
    {
        StartBomb(player, 0, "\x90\x6c\x95\x84\x81\x75\x8c\xbb\x90\xa2\x8e\x61\x81\x76", 220, 270, 0);
        g_SoundPlayer.QueueSound(0xd, 0);
        player->moveScaleY = 0.0f;
        player->moveScaleX = 0.0f;
        player->anm->StartScript(&player->sprite, 0);
        g_SoundPlayer.QueueSound(6, 0);
        part->pos = player->pos;
        part->velocity = part->pos;
        part->velocity.y = 416.0f;
    }
    if (b->timer < 40)
    {
        t = b->timer.AsFloat() / 40.0f;
        t = (t = 1.0f - t) * t;
        t = 1.0f - t;
        player->pos = (part->velocity - part->pos) * t + part->pos;
    }
    else if (b->timer.HasTickedTo(40))
    {
        g_EffectManager.SpawnEffect(0x28, &player->pos, 1, 0xff8080ff);
    }
    else if (b->timer.HasTickedTo(70))
    {
        D3DVECTOR pos;

        player->pos.y = 32.0f;
        pos = player->pos;
        pos.y = 224.0f;
        ScreenEffect::StartEffect(3, 8, 1, 0xefffffff, 0, 0x15);
        g_SoundPlayer.QueueSound(0x2a, 0);
        part->hitArea = player->AddHitArea((D3DXVECTOR3 *)&pos, 96.0f, 448.0f, 6, 60);
        part->hitbox = player->AddBombHitbox((D3DXVECTOR3 *)&pos, 96.0f, 448.0f, 300, 10);
        part->hitbox = player->AddBombHitbox((D3DXVECTOR3 *)&pos, 96.0f, 448.0f, 80, 60);
        part->hitbox->damageInterval = 5;
        g_EffectManager.SpawnEffect(0x30, (D3DXVECTOR3 *)&pos, 1, 0xff8080ff);
    }
    else if (b->timer.HasTickedTo(80))
    {
        D3DVECTOR pos;

        pos = player->pos;
        pos.y = 224.0f;
        pos.x -= 32.0f;
        player->AddHitArea((D3DXVECTOR3 *)&pos, 96.0f, 448.0f, 6, 60);
        part->hitbox = player->AddBombHitbox((D3DXVECTOR3 *)&pos, 96.0f, 448.0f, 100, 40);
        part->hitbox->damageInterval = 2;
        g_EffectManager.SpawnEffect(0x30, (D3DXVECTOR3 *)&pos, 1, 0xff8080ff);
        pos.x += 64.0f;
        player->AddHitArea((D3DXVECTOR3 *)&pos, 96.0f, 448.0f, 6, 60);
        part->hitbox = player->AddBombHitbox((D3DXVECTOR3 *)&pos, 96.0f, 448.0f, 100, 40);
        part->hitbox->damageInterval = 3;
        g_EffectManager.SpawnEffect(0x30, (D3DXVECTOR3 *)&pos, 1, 0xff8080ff);
        ScreenEffect::StartEffect(3, 8, 1, 0xcfffffff, 0, 0x15);
    }
    else if (b->timer.HasTickedTo(90))
    {
        D3DVECTOR pos;

        pos = player->pos;
        pos.y = 224.0f;
        pos.x -= 64.0f;
        player->AddHitArea((D3DXVECTOR3 *)&pos, 96.0f, 448.0f, 6, 60);
        part->hitbox = player->AddBombHitbox((D3DXVECTOR3 *)&pos, 96.0f, 448.0f, 100, 40);
        part->hitbox->damageInterval = 2;
        g_EffectManager.SpawnEffect(0x30, (D3DXVECTOR3 *)&pos, 1, 0xff8080ff);
        pos.x += 128.0f;
        player->AddHitArea((D3DXVECTOR3 *)&pos, 96.0f, 448.0f, 6, 60);
        part->hitbox = player->AddBombHitbox((D3DXVECTOR3 *)&pos, 96.0f, 448.0f, 100, 40);
        part->hitbox->damageInterval = 3;
        g_EffectManager.SpawnEffect(0x30, (D3DXVECTOR3 *)&pos, 1, 0xff8080ff);
        ScreenEffect::StartEffect(3, 8, 1, 0xbfffffff, 0, 0x15);
    }
    else if (b->timer.HasTickedTo(100))
    {
        D3DVECTOR pos;

        pos = player->pos;
        pos.y = 224.0f;
        pos.x -= 96.0f;
        player->AddHitArea((D3DXVECTOR3 *)&pos, 96.0f, 448.0f, 6, 60);
        part->hitbox = player->AddBombHitbox((D3DXVECTOR3 *)&pos, 96.0f, 448.0f, 80, 40);
        part->hitbox->damageInterval = 2;
        g_EffectManager.SpawnEffect(0x30, (D3DXVECTOR3 *)&pos, 1, 0xff8080ff);
        pos.x += 192.0f;
        player->AddHitArea((D3DXVECTOR3 *)&pos, 96.0f, 448.0f, 6, 60);
        part->hitbox = player->AddBombHitbox((D3DXVECTOR3 *)&pos, 96.0f, 448.0f, 80, 40);
        part->hitbox->damageInterval = 3;
        g_EffectManager.SpawnEffect(0x30, (D3DXVECTOR3 *)&pos, 1, 0xff8080ff);
        ScreenEffect::StartEffect(3, 8, 1, 0x8fffffff, 0, 0x15);
        part->velocity = player->pos;
    }
    else if (b->timer >= 120 && b->timer < 150)
    {
        t = (b->timer.AsFloat() - 120.0f) / 30.0f;
        t = (t = 1.0f - t) * t;
        t = 1.0f - t;
        player->pos = (part->pos - part->velocity) * t + part->velocity;
    }
    else if (b->timer.HasTickedTo(150))
    {
        player->moveScaleY = 1.0f;
        player->moveScaleX = 1.0f;
    }
}

void DrawYoumuBomb(Player *player)
{
    PlayerBomb *bomb = &player->bomb;

    DimPlayAreaForBomb(player, 0x80404040);
    if (bomb->timer >= 70)
    {
        ColorBytes color;
        float rect[4];

        DimPlayAreaForBomb(player, 0x80000030);
        rect[0] = 32.0f;
        rect[1] = 16.0f;
        rect[2] = 416.0f;
        rect[3] = 464.0f;
        color.r = 0xff;
        color.g = 0xff;
        color.b = 0xff;
        if (bomb->timer < 100)
        {
            color.a = 0xff;
        }
        else if (bomb->timer < 160)
        {
            color.a = 0xff - (bomb->timer.AsFrames() - 100) * 0xff / 60;
        }
        else
        {
            return;
        }
        DrawColorRect(rect, color.color);
    }
}

void UpdateYoumuLastWord(Player *player)
{
    PlayerBomb *b = &player->bomb;
    int x = 0;
    int y = 0;
    PlayerBombPart *part = b->parts;
    float t;

    if (b->timer.HasTicked() && b->timer == 0)
    {
        StartBomb(player, 0, "\x90\x6c\x8b\x53\x81\x75\x96\xa2\x97\x88\x89\x69\x8d\x85\x8e\x61\x81\x76", 250, 300, 0);
        g_SoundPlayer.QueueSound(0xd, 0);
        player->moveScaleY = 0.0f;
        player->moveScaleX = 0.0f;
        player->anm->StartScript(&player->sprite, 0);
        g_SoundPlayer.QueueSound(6, 0);
        part->pos = player->pos;
        part->velocity = part->pos;
        part->velocity.y = 416.0f;
    }
    if (b->timer < 40)
    {
        t = b->timer.AsFloat() / 40.0f;
        t = (t = 1.0f - t) * t;
        t = 1.0f - t;
        player->pos = (part->velocity - part->pos) * t + part->pos;
    }
    else if (b->timer.HasTickedTo(40))
    {
        g_EffectManager.SpawnEffect(0x28, &player->pos, 1, 0xffff8080);
    }
    else if (b->timer.HasTickedTo(70))
    {
        D3DVECTOR pos;

        player->pos.y = 32.0f;
        pos = player->pos;
        pos.y = 224.0f;
        ScreenEffect::StartEffect(3, 8, 1, 0xefffffff, 0, 0x15);
        g_SoundPlayer.QueueSound(0x2a, 0);
        part->hitArea = player->AddHitArea((D3DXVECTOR3 *)&pos, 96.0f, 448.0f, 6, 60);
        part->hitbox = player->AddBombHitbox((D3DXVECTOR3 *)&pos, 96.0f, 448.0f, 500, 0);
        part->hitbox = player->AddBombHitbox((D3DXVECTOR3 *)&pos, 96.0f, 448.0f, 100, 60);
        part->hitbox->damageInterval = 5;
        g_EffectManager.SpawnEffect(0x30, (D3DXVECTOR3 *)&pos, 1, 0xffff8080);
    }
    else if (b->timer.HasTickedTo(80))
    {
        D3DVECTOR pos;

        pos = player->pos;
        pos.y = 224.0f;
        pos.x -= 32.0f;
        player->AddHitArea((D3DXVECTOR3 *)&pos, 96.0f, 448.0f, 6, 60);
        part->hitbox = player->AddBombHitbox((D3DXVECTOR3 *)&pos, 96.0f, 448.0f, 100, 40);
        part->hitbox->damageInterval = 2;
        g_EffectManager.SpawnEffect(0x30, (D3DXVECTOR3 *)&pos, 1, 0xffff8080);
        pos.x += 64.0f;
        player->AddHitArea((D3DXVECTOR3 *)&pos, 96.0f, 448.0f, 6, 60);
        part->hitbox = player->AddBombHitbox((D3DXVECTOR3 *)&pos, 96.0f, 448.0f, 100, 40);
        part->hitbox->damageInterval = 3;
        g_EffectManager.SpawnEffect(0x30, (D3DXVECTOR3 *)&pos, 1, 0xffff8080);
        ScreenEffect::StartEffect(3, 8, 1, 0xcfffffff, 0, 0x15);
    }
    else if (b->timer.HasTickedTo(90))
    {
        D3DVECTOR pos;

        pos = player->pos;
        pos.y = 224.0f;
        pos.x -= 64.0f;
        player->AddHitArea((D3DXVECTOR3 *)&pos, 96.0f, 448.0f, 6, 60);
        part->hitbox = player->AddBombHitbox((D3DXVECTOR3 *)&pos, 96.0f, 448.0f, 100, 40);
        part->hitbox->damageInterval = 2;
        g_EffectManager.SpawnEffect(0x30, (D3DXVECTOR3 *)&pos, 1, 0xffff8080);
        pos.x += 128.0f;
        player->AddHitArea((D3DXVECTOR3 *)&pos, 96.0f, 448.0f, 6, 60);
        part->hitbox = player->AddBombHitbox((D3DXVECTOR3 *)&pos, 96.0f, 448.0f, 100, 40);
        part->hitbox->damageInterval = 3;
        g_EffectManager.SpawnEffect(0x30, (D3DXVECTOR3 *)&pos, 1, 0xffff8080);
        ScreenEffect::StartEffect(3, 8, 1, 0xafffffff, 0, 0x15);
    }
    else if (b->timer.HasTickedTo(100))
    {
        D3DVECTOR pos;

        pos = player->pos;
        pos.y = 224.0f;
        pos.x -= 96.0f;
        player->AddHitArea((D3DXVECTOR3 *)&pos, 96.0f, 448.0f, 6, 60);
        part->hitbox = player->AddBombHitbox((D3DXVECTOR3 *)&pos, 96.0f, 448.0f, 80, 40);
        part->hitbox->damageInterval = 2;
        g_EffectManager.SpawnEffect(0x30, (D3DXVECTOR3 *)&pos, 1, 0xffff8080);
        pos.x += 192.0f;
        player->AddHitArea((D3DXVECTOR3 *)&pos, 96.0f, 448.0f, 6, 60);
        part->hitbox = player->AddBombHitbox((D3DXVECTOR3 *)&pos, 96.0f, 448.0f, 80, 40);
        part->hitbox->damageInterval = 3;
        g_EffectManager.SpawnEffect(0x30, (D3DXVECTOR3 *)&pos, 1, 0xffff8080);
        ScreenEffect::StartEffect(3, 8, 1, 0x8fffffff, 0, 0x15);
    }
    else if (b->timer.HasTickedTo(110))
    {
        D3DVECTOR pos;

        pos = player->pos;
        pos.y = 224.0f;
        pos.x -= 128.0f;
        player->AddHitArea((D3DXVECTOR3 *)&pos, 96.0f, 448.0f, 6, 60);
        part->hitbox = player->AddBombHitbox((D3DXVECTOR3 *)&pos, 96.0f, 448.0f, 60, 40);
        part->hitbox->damageInterval = 2;
        g_EffectManager.SpawnEffect(0x30, (D3DXVECTOR3 *)&pos, 1, 0xffff8080);
        pos.x += 256.0f;
        player->AddHitArea((D3DXVECTOR3 *)&pos, 96.0f, 448.0f, 6, 60);
        part->hitbox = player->AddBombHitbox((D3DXVECTOR3 *)&pos, 96.0f, 448.0f, 60, 40);
        part->hitbox->damageInterval = 3;
        g_EffectManager.SpawnEffect(0x30, (D3DXVECTOR3 *)&pos, 1, 0xffff8080);
        ScreenEffect::StartEffect(3, 8, 1, 0x6fffffff, 0, 0x15);
    }
    else if (b->timer.HasTickedTo(120))
    {
        D3DVECTOR pos;

        pos = player->pos;
        pos.y = 224.0f;
        pos.x -= 160.0f;
        player->AddHitArea((D3DXVECTOR3 *)&pos, 96.0f, 448.0f, 6, 60);
        part->hitbox = player->AddBombHitbox((D3DXVECTOR3 *)&pos, 96.0f, 448.0f, 50, 40);
        part->hitbox->damageInterval = 2;
        g_EffectManager.SpawnEffect(0x30, (D3DXVECTOR3 *)&pos, 1, 0xffff8080);
        pos.x += 320.0f;
        player->AddHitArea((D3DXVECTOR3 *)&pos, 96.0f, 448.0f, 6, 60);
        part->hitbox = player->AddBombHitbox((D3DXVECTOR3 *)&pos, 96.0f, 448.0f, 50, 40);
        part->hitbox->damageInterval = 3;
        g_EffectManager.SpawnEffect(0x30, (D3DXVECTOR3 *)&pos, 1, 0xffff8080);
        ScreenEffect::StartEffect(3, 8, 1, 0x5fffffff, 0, 0x15);
    }
    else if (b->timer.HasTickedTo(130))
    {
        D3DVECTOR pos;

        pos = player->pos;
        pos.y = 224.0f;
        pos.x -= 192.0f;
        player->AddHitArea((D3DXVECTOR3 *)&pos, 96.0f, 448.0f, 6, 60);
        part->hitbox = player->AddBombHitbox((D3DXVECTOR3 *)&pos, 96.0f, 448.0f, 40, 40);
        part->hitbox->damageInterval = 2;
        g_EffectManager.SpawnEffect(0x30, (D3DXVECTOR3 *)&pos, 1, 0xffff8080);
        pos.x += 384.0f;
        player->AddHitArea((D3DXVECTOR3 *)&pos, 96.0f, 448.0f, 6, 60);
        part->hitbox = player->AddBombHitbox((D3DXVECTOR3 *)&pos, 96.0f, 448.0f, 40, 40);
        part->hitbox->damageInterval = 3;
        g_EffectManager.SpawnEffect(0x30, (D3DXVECTOR3 *)&pos, 1, 0xffff8080);
        ScreenEffect::StartEffect(3, 8, 1, 0x5fffffff, 0, 0x15);
        part->velocity = player->pos;
    }
    else if (b->timer >= 150 && b->timer < 180)
    {
        t = (b->timer.AsFloat() - 150.0f) / 30.0f;
        t = (t = 1.0f - t) * t;
        t = 1.0f - t;
        player->pos = (part->pos - part->velocity) * t + part->velocity;
    }
    else if (b->timer.HasTickedTo(180))
    {
        player->moveScaleY = 1.0f;
        player->moveScaleX = 1.0f;
    }
}

void DrawYoumuLastWord(Player *player)
{
    PlayerBomb *bomb = &player->bomb;

    DimPlayAreaForBomb(player, 0x80404040);
    if (bomb->timer >= 70)
    {
        float rect[4];
        ColorBytes color;

        DimPlayAreaForBomb(player, 0x80000030);
        rect[0] = 32.0f;
        rect[1] = 16.0f;
        rect[2] = 416.0f;
        rect[3] = 464.0f;
        color.r = 0xff;
        color.g = 0;
        color.b = 0;
        if (bomb->timer < 100)
        {
            color.a = 0xff;
        }
        else if (bomb->timer < 160)
        {
            color.a = 0xff - (bomb->timer.AsFrames() - 100) * 0xff / 60;
        }
        else
        {
            return;
        }
        DrawColorRect(rect, color.color);
    }
}

int EffectUpdateBombRing48(Effect *effect)
{
    float t;

    if (effect->timer < 0x1e)
    {
        effect->radius = 192.0f;
        effect->segmentCount = 0x30;
        effect->ringWidth = 3.0f;
        effect->radiusY = 0.0001f;
        effect->ellipseAngle = 1.5707964f;
    }
    else
    {
        t = (t = (t = (effect->timer.AsFloat() - 30.0f) / 30.0f) * t) * t;
        effect->radiusY = 192.0f * t + 0.0001f;
        effect->ringWidth = 80.0f * t + 3.0f;
    }
    effect->verticesDirty = 1;
    return 1;
}
