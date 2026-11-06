// Alice's bombs, from 0x40d430 to 0x40e3b0, with her effect callbacks: an
// object of the original. bomb.cpp's code before it ends with the inline
// copies its first function already uses (Timer::AsFrames, from
// 0x40d390), and Alice's with the copies only she uses (from 0x40e350).

#include "bullet.h"
#include "effect.h"
#include "enemy.h"
#include "item.h"
#include "player.h"
#include "screeneffect.h"
#include "sprite.h"
#include "unsorted.h"

void UpdateAliceBomb(Player *player)
{
    PlayerBomb *bomb = &player->bomb;
    PlayerBombPart *part = bomb->parts;

    if (bomb->timer.HasTicked() && bomb->timer == 0)
    {
        StartBomb(player, 1, "\x96\x82\x95\x84\x81\x75\x83\x41\x81\x5b\x83\x65\x83\x42\x83\x74\x83\x8b\x83\x54\x83\x4e\x83\x8a\x83\x74\x83\x40\x83\x43\x83\x58\x81\x76", 210, 250, 0);
        g_SoundPlayer.QueueSound(0xd, 0);
        part->pos = player->options[0].pos;
        g_SoundPlayer.QueueSound(6, 0);
    }
    if (bomb->timer < 60)
    {
        D3DXVECTOR3 target(192.0f, 224.0f, 0.0f);
        float progress;

        progress = (progress = bomb->timer.AsFloat() / 60.0f) * progress;
        player->options[0].pos = (target - part->pos) * progress + part->pos;
        player->options[0].sprite.rotation.z += -0.31415927f;
        player->AddHitAreaCircle(&player->options[0].pos, 32.0f, 0.0f, 0, 6);
        player->AddBombHitboxCircle(&player->options[0].pos, 32.0f, 0.0f, 40, 0);
    }
    else
    {
        player->options[0].sprite.rotation.z = 0.0f;
        player->options[0].pos.x = 192.0f;
        player->options[0].pos.y = 224.0f;
        if (bomb->timer >= 150)
        {
            player->options[0].sprite.alpha = 0;
        }
        if (bomb->timer.HasTickedTo(60))
        {
            g_EffectManager.SpawnEffect(0x28, &player->options[0].pos, 1, 0xffffffff);
        }
        else if (bomb->timer.HasTickedTo(64))
        {
            g_EffectManager.SpawnEffect(0x28, &player->options[0].pos, 1, 0xffffd0d0);
        }
        else if (bomb->timer.HasTickedTo(68))
        {
            g_EffectManager.SpawnEffect(0x28, &player->options[0].pos, 1, 0xffffb0b0);
        }
        else if (bomb->timer.HasTickedTo(72))
        {
            g_EffectManager.SpawnEffect(0x28, &player->options[0].pos, 1, 0xffff8080);
        }
        else if (bomb->timer.HasTickedTo(76))
        {
            g_EffectManager.SpawnEffect(0x28, &player->options[0].pos, 1, 0xffff4040);
        }
        else if (bomb->timer.HasTickedTo(90))
        {
            Effect *effect;
            PlayerHitArea *hitbox;

            g_SoundPlayer.QueueSound(0xf, 0);
            effect = g_EffectManager.SpawnEffect(0x2a, &player->options[0].pos, 1, 0xffffffff);
            effect = g_EffectManager.SpawnEffect(0x2b, &player->options[0].pos, 1, 0xffffffff);
            effect = g_EffectManager.SpawnEffect(0x2c, &player->options[0].pos, 1, 0xffffffff);
            player->AddHitAreaCircle(&player->options[0].pos, 1.0f, 5.0f, 110, 6);
            hitbox = player->AddBombHitboxCircle(&player->options[0].pos, 1.0f, 5.0f, 70, 110);
            hitbox->damageInterval = 5;
            ScreenEffect::StartEffect(1, 0x18, 8, 0, 0, 0x15);
            ScreenEffect::StartEffect(3, 8, 1, 0x8fffffff, 0, 0x15);
        }
        else if (bomb->timer.HasTickedTo(100))
        {
            Effect *effect = g_EffectManager.SpawnEffect(0x2d, &player->options[0].pos, 1, 0xffffffff);
        }
        else if (bomb->timer.HasTickedTo(110))
        {
            Effect *effect = g_EffectManager.SpawnEffect(0x2d, &player->options[0].pos, 1, 0xffffd0d0);
        }
        else if (bomb->timer.HasTickedTo(120))
        {
            Effect *effect = g_EffectManager.SpawnEffect(0x2d, &player->options[0].pos, 1, 0xffff8080);
        }
        else if (bomb->timer.HasTickedTo(130))
        {
            Effect *effect = g_EffectManager.SpawnEffect(0x2d, &player->options[0].pos, 1, 0xffff0000);
        }
        else if (bomb->timer.HasTickedTo(150))
        {
            ScreenEffect::StartEffect(3, 8, 1, 0x8fffffff, 0, 0x15);
            ScreenEffect::StartEffect(1, 0x18, 8, 0, 0, 0x15);
            g_SoundPlayer.QueueSound(0x19, 0);
        }
        else if (bomb->timer.HasTickedTo(209))
        {
            player->options[0].active = 1;
            player->options[0].timer = 0;
        }
    }
}

void DrawAliceBomb(Player *player)
{
    DimPlayAreaForBomb(player, 0x80404040);
}

void UpdateAliceLastWord(Player *player)
{
    PlayerBomb *bomb = &player->bomb;
    PlayerBombPart *part = bomb->parts;

    if (bomb->timer.HasTicked() && bomb->timer == 0)
    {
        StartBomb(player, 1, "\x96\x82\x91\x80\x81\x75\x83\x8a\x83\x5e\x81\x5b\x83\x93\x83\x43\x83\x69\x83\x6a\x83\x81\x83\x67\x83\x6c\x83\x58\x81\x76", 230, 280, 1);
        g_SoundPlayer.QueueSound(0xd, 0);
        part->pos = player->pos;
    }
    if (bomb->timer < 60)
    {
        D3DXVECTOR3 target(192.0f, 224.0f, 0.0f);
        float progress;

        progress = (progress = bomb->timer.AsFloat() / 60.0f) * progress;
        player->options[0].pos = (target - part->pos) * progress + part->pos;
        player->options[0].sprite.rotation.z += -0.31415927f;
        player->AddHitAreaCircle(&player->options[0].pos, 32.0f, 0.0f, 0, 6);
        player->AddBombHitboxCircle(&player->options[0].pos, 32.0f, 0.0f, 40, 0);
    }
    else
    {
        player->options[0].sprite.rotation.z = 0.0f;
        player->options[0].pos.x = 192.0f;
        player->options[0].pos.y = 224.0f;
        if (bomb->timer >= 128)
        {
            player->options[0].sprite.alpha = 0;
        }
        if (bomb->timer.HasTickedTo(60))
        {
            g_EffectManager.SpawnEffect(0x28, &player->options[0].pos, 1, 0xffffffff);
        }
        else if (bomb->timer.HasTickedTo(64))
        {
            g_EffectManager.SpawnEffect(0x28, &player->options[0].pos, 1, 0xffffd0d0);
        }
        else if (bomb->timer.HasTickedTo(68))
        {
            g_EffectManager.SpawnEffect(0x28, &player->options[0].pos, 1, 0xffffb0b0);
        }
        else if (bomb->timer.HasTickedTo(72))
        {
            g_EffectManager.SpawnEffect(0x28, &player->options[0].pos, 1, 0xffff8080);
        }
        else if (bomb->timer.HasTickedTo(76))
        {
            g_EffectManager.SpawnEffect(0x28, &player->options[0].pos, 1, 0xffff4040);
        }
        else if (bomb->timer.HasTickedTo(120))
        {
            Effect *effect;
            PlayerHitArea *hitbox;

            g_SoundPlayer.QueueSound(0xf, 0);
            effect = g_EffectManager.SpawnEffect(0x2a, &player->options[0].pos, 1, 0xffffffff);
            effect = g_EffectManager.SpawnEffect(0x2b, &player->options[0].pos, 1, 0xffffffff);
            effect = g_EffectManager.SpawnEffect(0x2c, &player->options[0].pos, 1, 0xffffffff);
            D3DXVECTOR3 pos(64.0f, 96.0f, 0.0f);
            effect = g_EffectManager.SpawnEffect(0x2d, &pos, 1, 0xff0000f0);
            pos.y = 352.0f;
            effect = g_EffectManager.SpawnEffect(0x2d, &pos, 1, 0xfff00000);
            pos.x = 320.0f;
            effect = g_EffectManager.SpawnEffect(0x2d, &pos, 1, 0xff00f000);
            pos.y = 96.0f;
            effect = g_EffectManager.SpawnEffect(0x2d, &pos, 1, 0xff00f0f0);
            player->AddHitAreaCircle(&player->options[0].pos, 1.0f, 5.0f, 110, 6);
            hitbox = player->AddBombHitboxCircle(&player->options[0].pos, 1.0f, 5.0f, 70, 110);
            hitbox->damageInterval = 5;
        }
        else if (bomb->timer.HasTickedTo(130))
        {
            Effect *effect = g_EffectManager.SpawnEffect(0x2d, &player->options[0].pos, 1, 0xffffffff);
        }
        else if (bomb->timer.HasTickedTo(140))
        {
            Effect *effect = g_EffectManager.SpawnEffect(0x2d, &player->options[0].pos, 1, 0xffffd0d0);
        }
        else if (bomb->timer.HasTickedTo(150))
        {
            Effect *effect = g_EffectManager.SpawnEffect(0x2d, &player->options[0].pos, 1, 0xffff8080);
        }
        else if (bomb->timer.HasTickedTo(160))
        {
            Effect *effect = g_EffectManager.SpawnEffect(0x2d, &player->options[0].pos, 1, 0xffff0000);
        }
        else if (bomb->timer.HasTickedTo(180))
        {
            ScreenEffect::StartEffect(3, 8, 1, 0xffffffff, 0, 0x15);
            ScreenEffect::StartEffect(1, 0x18, 8, 0, 0, 0x15);
            g_SoundPlayer.QueueSound(0x19, 0);
        }
        else if (bomb->timer.HasTickedTo(229))
        {
            player->options[0].active = 1;
            player->options[0].timer = 0;
        }
    }
}

void DrawAliceLastWord(Player *player)
{
    PlayerBomb *bomb = &player->bomb;
    PlayerBombPart *part = player->bomb.parts;

    if (bomb->timer < 90)
    {
        DimPlayAreaForBomb(player, 0x802020d0);
    }
    else if (bomb->timer <= 120)
    {
        float rect[4];
        int s;
        ColorBytes tint;

        s = (bomb->timer.AsFrames() - 90) * 0xd0 / 30;
        tint.r = s / 5 + 0xd0;
        tint.g = s + 0x20;
        tint.b = s + 0x20;
        tint.a = 0x80;
        DimPlayAreaForBomb(player, tint.color);
        rect[0] = 32.0f;
        rect[1] = 16.0f;
        rect[2] = 416.0f;
        rect[3] = 464.0f;
        tint.r = 0xff;
        tint.g = 0xff;
        tint.b = 0xff;
        tint.a = (bomb->timer.AsFrames() - 90) * 0xff / 30;
        DrawColorRect(rect, tint.color);
    }
    else if (bomb->timer <= 220)
    {
        float rect[4];
        ColorBytes color;

        rect[0] = 32.0f;
        rect[1] = 16.0f;
        rect[2] = 416.0f;
        rect[3] = 464.0f;
        color.r = 0xff;
        color.g = 0xff;
        color.b = 0xff;
        color.a = 0x70;
        DrawColorRect(rect, color.color);
    }
    else
    {
        DimPlayAreaForBomb(player, 0x802020d0);
    }
}

int EffectUpdateBombRing42(Effect *effect)
{
    float t;

    t = (t = 1.0f - effect->timer.AsFloat() / 40.0f) * t;
    t = 1.0f - t;
    effect->radius = 256.0f * t;
    effect->segmentCount = 0x40;
    effect->verticesDirty = 1;
    effect->waveCount = 5.0f;
    effect->ellipseAngle = 0.0f;
    if (effect->timer < 0x28)
    {
        effect->ringWidth = 8.0f;
    }
    else
    {
        effect->radiusY = 64.0f * t;
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
    effect->waveCount = 0.0f;
    effect->radiusY = 128.0f * t;
    effect->ellipseAngle = 0.7853982f;
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
    effect->radiusY = 128.0f * t;
    effect->ellipseAngle = -0.7853982f;
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
