// Marisa's bombs, from 0x40e3b0 to 0x40ee10, with her effect callback: an
// object of the original. Alice's code before it and Marisa's end with
// the inline copies only they use (Timer::HasTickedTo and
// Timer::operator<= at 0x40e350, Timer::HasTickedEvery and the Sprite
// interpolation setters from 0x40ebc0).

#include "bullet.h"
#include "effect.h"
#include "enemy.h"
#include "item.h"
#include "player.h"
#include "screeneffect.h"
#include "sprite.h"
#include "unsorted.h"

void UpdateMarisaBomb(Player *player)
{
    PlayerBomb *bomb = &player->bomb;

    if (bomb->timer.HasTicked() && bomb->timer == 0)
    {
        float theta;
        PlayerBombPart *part;

        StartBomb(player, 0, "\x97\xf6\x95\x84\x81\x75\x83\x7d\x83\x58\x83\x5e\x81\x5b\x83\x58\x83\x70\x81\x5b\x83\x4e\x81\x76", 300, 350, 0);
        theta = -3.1415927f;
        part = bomb->parts;
        g_SoundPlayer.QueueSound(0x13, 0);
        part->pos = player->pos;
        player->anm->StartScript(&bomb->parts[0].sprites[0], 0x1e);
        player->anm->StartScript(&bomb->parts[0].sprites[1], 0x1f);
        player->anm->StartScript(&bomb->parts[0].sprites[2], 0x20);
        player->anm->StartScript(&bomb->parts[0].sprites[3], 0x21);
        player->anm->StartScript(&bomb->parts[0].sprites[4], 0x22);
        player->moveScaleY = 0.2f;
        player->moveScaleX = 0.2f;
        ScreenEffect::StartEffect(7, 0x10, 0x78, 0x3c, 0x78, 0x15);
    }
    if (player->bomb.timer.HasTicked() && player->bomb.timer.AsFrames() % 4 != 0)
    {
        PlayerHitArea *hitbox;
        D3DXVECTOR3 pos;

        pos = player->pos;
        pos.x = 192.0f;
        pos.y /= 2.0f;
        player->AddHitArea(&pos, 384.0f, pos.y * 2.0f, 6, 0);
        pos = player->pos;
        pos.y /= 2.0f;
        hitbox = player->AddBombHitbox(&pos, 128.0f, pos.y * 2.0f, 12, 0);
        hitbox->unusedByte3d = 1;
        pos.x = 192.0f;
        hitbox = player->AddBombHitbox(&pos, 384.0f, pos.y * 2.0f, 6, 0);
        hitbox->unusedByte3d = 1;
    }
    g_SpriteEngine->StepSprites(&bomb->parts[0].sprites[0], 5);
}

void DrawMarisaBomb(Player *player)
{
    Sprite *sprite;
    int k;
    float delta;
    float theta;

    DimPlayAreaForBomb(player, 0x80404040);
    delta = 0.20943952f;
    for (sprite = &player->bomb.parts[0].sprites[0], k = 0; k < 5; k++, sprite++)
    {
        theta = k * delta - 1.5707964f - delta * 2.0f;
        if (theta < -3.1415927f)
        {
            theta += 6.2831855f;
        }
        sprite->pos = player->pos;
        sprite->pos.x += cosf(theta) * sprite->spriteData->width * sprite->scaleX / 2.0f;
        sprite->pos.y += sinf(theta) * sprite->spriteData->width * sprite->scaleX / 2.0f;
        sprite->SetRotationZ(theta);
        sprite->pos.x += g_GameManager.arena[0];
        sprite->pos.y += g_GameManager.arena[1];
        sprite->pos.z = 0.0f;
        g_SpriteEngine->Draw(sprite);
    }
}

void UpdateMarisaLastWord(Player *player)
{
    PlayerBomb *bomb = &player->bomb;

    if (bomb->timer.HasTicked() && bomb->timer == 0)
    {
        float theta;
        PlayerBombPart *part;

        StartBomb(player, 0, "\x96\x82\x96\x43\x81\x75\x83\x74\x83\x40\x83\x43\x83\x69\x83\x8b\x83\x58\x83\x70\x81\x5b\x83\x4e\x81\x76", 350, 380, 1);
        theta = -3.1415927f;
        part = bomb->parts;
        g_SoundPlayer.QueueSound(0xd, 0);
        g_SoundPlayer.QueueSound(0x13, 0);
        part->pos = player->pos;
        player->anm->StartScript(&bomb->parts[0].sprites[0], 0x23);
        player->anm->StartScript(&bomb->parts[0].sprites[1], 0x24);
        player->anm->StartScript(&bomb->parts[0].sprites[2], 0x25);
        player->anm->StartScript(&bomb->parts[0].sprites[3], 0x26);
        player->anm->StartScript(&bomb->parts[0].sprites[4], 0x27);
        ScreenEffect::StartEffect(7, 0x10, 0x78, 0x3c, 0x78, 0x15);
        player->moveScaleY = 0.2f;
        player->moveScaleX = 0.2f;
        bomb->spawnCount = 0;
    }
    if (bomb->timer.HasTickedEvery(10))
    {
        Effect *eff;
        float scaleStart[2];
        float scaleTo[2];

        eff = g_EffectManager.SpawnReservedEffect(0x35, &player->pos, bomb->spawnCount % 4 + 4, 1, 0xffffffff);
        if (bomb->spawnCount & 1)
        {
            g_EffectManager.anm->StartScript(&eff->sprite, 0x5c);
        }
        eff->segmentCount = 0x20;
        eff->waveCount = 0.0f;
        D3DXVECTOR3 start;
        D3DXVECTOR3 to;
        start.x = 0.0f;
        start.y = 0.0f;
        start.z = 0.0f;
        to.x = 128.0f;
        to.y = 0.0f;
        to.z = 0.0f;
        eff->sprite.SetMoveInterp(30, 4, &start, &to);
        scaleStart[0] = 32.0f;
        scaleStart[1] = 0.0f;
        scaleTo[0] = 64.0f;
        scaleTo[1] = 0.0f;
        eff->sprite.SetScaleInterp(30, 1, (D3DXVECTOR2 *)scaleStart, (D3DXVECTOR2 *)scaleTo);
        eff->sprite.SetAlphaInterp(30, 3, 0xff, 0);
        eff->sprite.SetColorInterp(30, 0, 0xffffffff, 0xffff0000);
        g_SpriteEngine->Step(&eff->sprite);
        bomb->spawnCount++;
        g_SoundPlayer.QueueSoundPanned(0x11, player->pos.x);
    }
    if (player->bomb.timer.HasTicked() && player->bomb.timer.AsFrames() % 4 != 0)
    {
        PlayerHitArea *hitbox;
        D3DXVECTOR3 pos;

        pos = player->pos;
        pos.x = 192.0f;
        pos.y /= 2.0f;
        player->AddHitArea(&pos, 384.0f, pos.y * 2.0f, 6, 0);
        pos = player->pos;
        pos.y /= 2.0f;
        hitbox = player->AddBombHitbox(&pos, 128.0f, pos.y * 2.0f, 12, 0);
        hitbox->unusedByte3d = 1;
        pos.x = 192.0f;
        hitbox = player->AddBombHitbox(&pos, 384.0f, pos.y * 2.0f, 7, 0);
        hitbox->unusedByte3d = 1;
    }
    g_SpriteEngine->StepSprites(&bomb->parts[0].sprites[0], 5);
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
