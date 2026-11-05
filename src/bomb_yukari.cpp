// Yukari's bombs, from 0x410c40 to 0x411b10, with her effect callbacks.
// The original starts an object here: Sakuya's code before it ends with
// the inline copies only it uses (Rng::GetRandomU16InRange), and the
// bombs' objects come in alphabetical order of the character after the
// shared code and Reimu's (bomb.cpp).

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

