// Remilia's bombs, from 0x40ee10 to 0x40fcd0. The original starts an
// object here: Marisa's code before it ends with the inline copies only
// it uses (from 0x40ebc0).

#include "bullet.h"
#include "effect.h"
#include "enemy.h"
#include "item.h"
#include "player.h"
#include "screeneffect.h"
#include "sprite.h"
#include "unsorted.h"

void UpdateRemiliaBomb(Player *player)
{
    PlayerBomb *bomb = &player->bomb;
    PlayerBombPart *part = bomb->parts;

    if (bomb->timer.HasTicked() && bomb->timer == 0)
    {
        StartBomb(player, 1, "\x8d\x67\x95\x84\x81\x75\x95\x73\x96\xe9\x8f\xe9\x83\x8c\x83\x62\x83\x68\x81\x76", 240, 290, 0);
        g_SoundPlayer.QueueSound(0xd, 0);
        part[0].pos = player->options[0].center;
        part[1].pos = player->options[1].center;
        part[2].pos = player->options[2].center;
        part[3].pos = player->options[3].center;
        g_SoundPlayer.QueueSound(6, 0);
        bomb->spawnCount = 0;
        player->moveScaleY = 0.0f;
        player->moveScaleX = 0.0f;
    }
    if (bomb->timer < 60)
    {
        float progress;

        progress = (progress = bomb->timer.AsFloat() / 60.0f) * progress;
        D3DXVECTOR3 target;
        target = player->pos;
        target.x -= 32.0f;
        player->options[0].center = (target - part[0].pos) * progress + part[0].pos;
        target.x += 32.0f;
        target.y -= 32.0f;
        player->options[1].center = (target - part[1].pos) * progress + part[1].pos;
        target.y += 64.0f;
        player->options[2].center = (target - part[2].pos) * progress + part[2].pos;
        target.x += 32.0f;
        target.y -= 32.0f;
        player->options[3].center = (target - part[3].pos) * progress + part[3].pos;
    }
    else
    {
        if (bomb->timer.HasTickedTo(60))
        {
            player->moveScaleY = 2.0f;
            player->moveScaleX = 2.0f;
            player->options[0].sprite.SetPendingInterrupt(2);
            player->options[1].sprite.SetPendingInterrupt(2);
            player->options[2].sprite.SetPendingInterrupt(2);
            player->options[3].sprite.SetPendingInterrupt(2);
        }
        D3DXVECTOR3 pos;
        pos = player->pos;
        pos.x -= 32.0f;
        player->options[0].center = pos;
        pos.x += 32.0f;
        pos.y -= 32.0f;
        player->options[1].center = pos;
        pos.y += 64.0f;
        player->options[2].center = pos;
        pos.x += 32.0f;
        pos.y -= 32.0f;
        player->options[3].center = pos;
        part->hitArea = player->AddHitAreaCircle(&player->pos, 96.0f, 0.0f, 0, 6);
        if (bomb->timer.HasTickedEvery(10))
        {
            Effect *eff;
            float scaleStart[2];
            float scaleTo[2];

            eff = g_EffectManager.SpawnReservedEffect(0x35, &player->pos, bomb->spawnCount % 4 + 4, 1, 0xffffffff);
            eff->segmentCount = 0x20;
            eff->waveCount = 4.0f;
            D3DXVECTOR3 start;
            D3DXVECTOR3 to;
            start.x = 0.0f;
            start.y = 0.0f;
            start.z = 0.0f;
            to.x = 192.0f;
            to.y = g_Random.GetRandomF32InRange(128.0f);
            to.z = 0.0f;
            eff->sprite.SetMoveInterp(30, 4, &start, &to);
            scaleStart[0] = 64.0f;
            scaleStart[1] = 0.0f;
            scaleTo[0] = 64.0f;
            scaleTo[1] = 0.0f;
            eff->sprite.SetScaleInterp(30, 1, (D3DXVECTOR2 *)scaleStart, (D3DXVECTOR2 *)scaleTo);
            eff->sprite.SetAlphaInterp(30, 3, 0xff, 0);
            eff->sprite.SetColorInterp(30, 0, 0xffffffff, 0xffff0000);
            bomb->spawnCount++;
            g_SoundPlayer.QueueSoundPanned(0x11, player->pos.x);
            g_SpriteEngine->Step(&eff->sprite);
        }
        if (player->fireTimer >= 5)
        {
            part->hitArea = player->AddHitArea(&player->pos, 96.0f, 800.0f, 6, 0);
            part->hitArea = player->AddHitArea(&player->pos, 800.0f, 96.0f, 6, 0);
        }
        if (bomb->timer.HasTickedTo(239))
        {
            player->options[0].active = 1;
            player->options[0].timer = 0;
            player->options[1].active = 1;
            player->options[1].timer = 0;
            player->options[2].active = 1;
            player->options[2].timer = 0;
            player->options[3].active = 1;
            player->options[3].timer = 0;
        }
    }
}

void DrawRemiliaBomb(Player *player)
{
    DimPlayAreaForBomb(player, 0x80d02020);
}

void UpdateRemiliaLastWord(Player *player)
{
    PlayerBomb *bomb = &player->bomb;
    PlayerBombPart *part = bomb->parts;

    if (bomb->timer.HasTicked() && bomb->timer == 0)
    {
        StartBomb(player, 1, "\x8d\x67\x96\x82\x81\x75\x83\x58\x83\x4a\x81\x5b\x83\x8c\x83\x62\x83\x67\x83\x66\x83\x72\x83\x8b\x81\x76", 280, 320, 1);
        g_SoundPlayer.QueueSound(0xd, 0);
        part[0].pos = player->options[0].center;
        part[1].pos = player->options[1].center;
        part[2].pos = player->options[2].center;
        part[3].pos = player->options[3].center;
        g_SoundPlayer.QueueSound(6, 0);
        bomb->spawnCount = 0;
        player->moveScaleY = 0.0f;
        player->moveScaleX = 0.0f;
    }
    if (bomb->timer < 60)
    {
        float progress;

        progress = (progress = bomb->timer.AsFloat() / 60.0f) * progress;
        D3DXVECTOR3 target;
        target = player->pos;
        target.x -= 32.0f;
        player->options[0].center = (target - part[0].pos) * progress + part[0].pos;
        target.x += 32.0f;
        target.y -= 32.0f;
        player->options[1].center = (target - part[1].pos) * progress + part[1].pos;
        target.y += 64.0f;
        player->options[2].center = (target - part[2].pos) * progress + part[2].pos;
        target.x += 32.0f;
        target.y -= 32.0f;
        player->options[3].center = (target - part[3].pos) * progress + part[3].pos;
    }
    else
    {
        if (bomb->timer.HasTickedTo(60))
        {
            player->moveScaleY = 3.0f;
            player->moveScaleX = 3.0f;
            player->options[0].sprite.SetPendingInterrupt(2);
            player->options[1].sprite.SetPendingInterrupt(2);
            player->options[2].sprite.SetPendingInterrupt(2);
            player->options[3].sprite.SetPendingInterrupt(2);
        }
        D3DXVECTOR3 pos;
        part->hitArea = player->AddHitAreaCircle(&player->pos, 96.0f, 0.0f, 0, 6);
        pos = player->pos;
        pos.x -= 32.0f;
        player->options[0].center = pos;
        pos.x += 32.0f;
        pos.y -= 32.0f;
        player->options[1].center = pos;
        pos.y += 64.0f;
        player->options[2].center = pos;
        pos.x += 32.0f;
        pos.y -= 32.0f;
        player->options[3].center = pos;
        if (bomb->timer.HasTickedEvery(10))
        {
            Effect *eff;
            float scaleStart[2];
            float scaleTo[2];

            eff = g_EffectManager.SpawnReservedEffect(0x35, &player->pos, bomb->spawnCount % 4 + 4, 1, 0xffffffff);
            eff->segmentCount = 0x20;
            eff->waveCount = 4.0f;
            D3DXVECTOR3 start;
            D3DXVECTOR3 to;
            start.x = 0.0f;
            start.y = 0.0f;
            start.z = 0.0f;
            to.x = 192.0f;
            to.y = g_Random.GetRandomF32InRange(128.0f);
            to.z = 0.0f;
            eff->sprite.SetMoveInterp(30, 4, &start, &to);
            scaleStart[0] = 64.0f;
            scaleStart[1] = 0.0f;
            scaleTo[0] = 128.0f;
            scaleTo[1] = 0.0f;
            eff->sprite.SetScaleInterp(30, 1, (D3DXVECTOR2 *)scaleStart, (D3DXVECTOR2 *)scaleTo);
            eff->sprite.SetAlphaInterp(30, 3, 0xff, 0);
            eff->sprite.SetColorInterp(30, 0, 0xffffffff, 0xffff0000);
            bomb->spawnCount++;
            g_SoundPlayer.QueueSoundPanned(0x11, player->pos.x);
            g_SpriteEngine->Step(&eff->sprite);
        }
        if (player->fireTimer >= 5)
        {
            part->hitArea = player->AddHitArea(&player->pos, 96.0f, 800.0f, 6, 0);
            part->hitArea = player->AddHitArea(&player->pos, 800.0f, 96.0f, 6, 0);
        }
        if (bomb->timer.HasTickedTo(279))
        {
            player->options[0].active = 1;
            player->options[0].timer = 0;
            player->options[1].active = 1;
            player->options[1].timer = 0;
            player->options[2].active = 1;
            player->options[2].timer = 0;
            player->options[3].active = 1;
            player->options[3].timer = 0;
        }
    }
}

void DrawRemiliaLastWord(Player *player)
{
    DimPlayAreaForBomb(player, 0x80f00000);
}

