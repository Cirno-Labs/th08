// Yukari's, Youmu's and Yuyuko's bombs, from 0x410c40 to 0x4143e0, with
// Yukari's and Youmu's effect callbacks. The original starts an object
// here: Sakuya's code before it ends with the inline copies only it uses
// (Rng::GetRandomU16InRange), and the bombs' objects come in alphabetical
// order of the character after the shared code and Reimu's (bomb.cpp).

#include "bullet.h"
#include "effect.h"
#include "enemy.h"
#include "item.h"
#include "player.h"
#include "screeneffect.h"
#include "sprite.h"
#include "unsorted.h"

void UpdateYukariBomb(Player *player)
{
    PlayerBomb *bomb = &player->bomb;

    if (bomb->timer.HasTicked() && bomb->timer == 0)
    {
        PlayerBombHitbox *hitbox;
        float theta;
        PlayerBombPart *part;

        StartBomb(player, 1, "\x8b\xab\x95\x84\x81\x75\x8e\x6c\x8f\x64\x8c\x8b\x8a\x45\x81\x76", 150, 200, 0);
        theta = -3.1415927f;
        part = bomb->parts;
        g_SoundPlayer.QueueSound(0xd, 0);
        part->pos = player->pos;
        player->AddHitAreaCircle(&player->pos, 100.0f, 1.0f, 40, 6);
        hitbox = player->AddBombHitboxCircle(&player->pos, 100.0f, 1.0f, 70, 40);
        hitbox->damageInterval = 5;
        D3DXVECTOR3 velocity(0.7853982f, 1.0f, 4.0f);
        g_EffectManager.SpawnReservedWithVelocity(0x24, &part->pos, &velocity, 4, 1, 0xffffffff);
    }
    if (bomb->timer.HasTickedTo(10))
    {
        Effect *eff;
        PlayerBombHitbox *hitbox;

        player->AddHitAreaCircle(&player->pos, 100.0f, 1.0f, 40, 6);
        hitbox = player->AddBombHitboxCircle(&player->pos, 100.0f, 1.0f, 70, 40);
        hitbox->damageInterval = 5;
        D3DXVECTOR3 velocity(1.1780972f, 1.0f, 4.0f);
        eff = g_EffectManager.SpawnReservedWithVelocity(0x24, &player->pos, &velocity, 5, 1, 0xffffffff);
        g_EffectManager.anm->StartScript(&eff->sprite, 0x59);
        bomb->parts[1].pos = player->pos;
    }
    if (bomb->timer.HasTickedTo(20))
    {
        Effect *eff;
        PlayerBombHitbox *hitbox;

        player->AddHitAreaCircle(&player->pos, 100.0f, 1.0f, 40, 6);
        hitbox = player->AddBombHitboxCircle(&player->pos, 100.0f, 1.0f, 70, 40);
        hitbox->damageInterval = 5;
        D3DXVECTOR3 velocity(1.5707964f, 1.0f, 4.0f);
        eff = g_EffectManager.SpawnReservedWithVelocity(0x24, &player->pos, &velocity, 6, 1, 0xffffffff);
        g_EffectManager.anm->StartScript(&eff->sprite, 0x5a);
        bomb->parts[2].pos = player->pos;
    }
    if (bomb->timer.HasTickedTo(30))
    {
        Effect *eff;
        PlayerBombHitbox *hitbox;

        player->AddHitAreaCircle(&player->pos, 100.0f, 1.0f, 40, 6);
        hitbox = player->AddBombHitboxCircle(&player->pos, 100.0f, 1.0f, 70, 40);
        hitbox->damageInterval = 5;
        D3DXVECTOR3 velocity(1.9634955f, 1.0f, 4.0f);
        eff = g_EffectManager.SpawnReservedWithVelocity(0x24, &player->pos, &velocity, 7, 1, 0xffffffff);
        g_EffectManager.anm->StartScript(&eff->sprite, 0x5b);
        bomb->parts[3].pos = player->pos;
    }
    g_SpriteEngine->StepSprites(&bomb->parts[0].sprites[0], 2);
}

void UpdateYukariLastWord(Player *player)
{
    PlayerBomb *bomb = &player->bomb;

    if (bomb->timer.HasTicked() && bomb->timer == 0)
    {
        PlayerBombHitbox *hitbox;
        float theta;
        PlayerBombPart *part;

        StartBomb(player, 1, "\x8b\xab\x8a\x45\x81\x75\x89\x69\x96\xe9\x8e\x6c\x8f\x64\x8c\x8b\x8a\x45\x81\x76", 250, 300, 1);
        theta = -3.1415927f;
        part = bomb->parts;
        g_SoundPlayer.QueueSound(0xd, 0);
        part->pos = player->pos;
        player->anm->StartScript(&bomb->parts[0].sprites[0], 0x15);
        player->anm->StartScript(&bomb->parts[0].sprites[1], 0x16);
        player->AddHitAreaCircle(&player->pos, 100.0f, 1.0f, 100, 6);
        hitbox = player->AddBombHitboxCircle(&player->pos, 100.0f, 1.0f, 70, 40);
        hitbox->damageInterval = 5;
        D3DXVECTOR3 velocity(0.7853982f, 1.0f, 4.0f);
        g_EffectManager.SpawnReservedWithVelocity(0x25, &part->pos, &velocity, 4, 1, 0xffffffff);
    }
    if (bomb->timer.HasTickedTo(10))
    {
        Effect *eff;
        PlayerBombHitbox *hitbox;

        player->AddHitAreaCircle(&player->pos, 100.0f, 1.0f, 40, 6);
        hitbox = player->AddBombHitboxCircle(&player->pos, 100.0f, 1.0f, 70, 40);
        hitbox->damageInterval = 5;
        D3DXVECTOR3 velocity(1.1780972f, 1.0f, 4.0f);
        eff = g_EffectManager.SpawnReservedWithVelocity(0x25, &bomb->parts[0].pos, &velocity, 5, 1, 0xffffffff);
        g_EffectManager.anm->StartScript(&eff->sprite, 0x5d);
        bomb->parts[1].pos = player->pos;
    }
    if (bomb->timer.HasTickedTo(20))
    {
        Effect *eff;
        PlayerBombHitbox *hitbox;

        player->AddHitAreaCircle(&player->pos, 100.0f, 1.0f, 100, 6);
        hitbox = player->AddBombHitboxCircle(&player->pos, 100.0f, 1.0f, 70, 40);
        hitbox->damageInterval = 5;
        D3DXVECTOR3 velocity(1.5707964f, 1.0f, 4.0f);
        eff = g_EffectManager.SpawnReservedWithVelocity(0x25, &bomb->parts[0].pos, &velocity, 6, 1, 0xffffffff);
        g_EffectManager.anm->StartScript(&eff->sprite, 0x5e);
        bomb->parts[2].pos = player->pos;
    }
    if (bomb->timer.HasTickedTo(30))
    {
        Effect *eff;
        PlayerBombHitbox *hitbox;

        player->AddHitAreaCircle(&player->pos, 100.0f, 1.0f, 100, 6);
        hitbox = player->AddBombHitboxCircle(&player->pos, 100.0f, 1.0f, 70, 40);
        hitbox->damageInterval = 5;
        D3DXVECTOR3 velocity(1.9634955f, 1.0f, 4.0f);
        eff = g_EffectManager.SpawnReservedWithVelocity(0x25, &bomb->parts[0].pos, &velocity, 7, 1, 0xffffffff);
        g_EffectManager.anm->StartScript(&eff->sprite, 0x5f);
        bomb->parts[3].pos = player->pos;
    }
    g_SpriteEngine->StepSprites(&bomb->parts[0].sprites[0], 2);
}

void DrawYukariLastWord(Player *player)
{
    PlayerBombPart *part = player->bomb.parts;
    Sprite *sprite;

    DimPlayAreaForBomb(player, 0x802020d0);
    sprite = &part->sprites[0];
    sprite->pos = part->pos + sprite->posOffset;
    sprite->pos.x += g_GameManager.arena[0];
    sprite->pos.y += g_GameManager.arena[1];
    sprite->pos.z = 0.01f;
    g_SpriteEngine->Draw(sprite);
    sprite++;
    sprite->pos = part->pos + sprite->posOffset;
    sprite->pos.x += g_GameManager.arena[0];
    sprite->pos.y += g_GameManager.arena[1];
    sprite->pos.z = 0.0f;
    g_SpriteEngine->Draw(sprite);
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
