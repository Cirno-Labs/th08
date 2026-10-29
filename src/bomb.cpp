// The bombs, from 0x40bc60, as th07's bomb.cpp: the shared helpers, the
// per-shot bomb callbacks of g_BombFuncs and the effect callbacks of
// g_EffectInfos. Only part of it so far.

#include "bullet.h"
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

void DrawReimuLastWord(Player *player)
{
    Sprite *sprite;
    DWORD k;
    PlayerBombPart *orbs;

    DimPlayAreaForBomb(player, 0x802020d0);
    for (k = 0, orbs = player->bomb.parts; k < 0x80; k++, orbs++)
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
            PlayerBombHitbox *hitbox;

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
            PlayerBombHitbox *hitbox;

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
        PlayerBombHitbox *hitbox;
        D3DXVECTOR3 pos;

        pos = player->pos;
        pos.x = 192.0f;
        pos.y /= 2.0f;
        player->AddHitArea(&pos, 384.0f, pos.y * 2.0f, 6, 0);
        pos = player->pos;
        pos.y /= 2.0f;
        hitbox = player->AddBombHitbox(&pos, 128.0f, pos.y * 2.0f, 12, 0);
        hitbox->unk_3d = 1;
        pos.x = 192.0f;
        hitbox = player->AddBombHitbox(&pos, 384.0f, pos.y * 2.0f, 6, 0);
        hitbox->unk_3d = 1;
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
        sprite->pos.x += cosf(theta) * sprite->spriteData->width * sprite->scaleY / 2.0f;
        sprite->pos.y += sinf(theta) * sprite->spriteData->width * sprite->scaleY / 2.0f;
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
        bomb->unk_14 = 0;
    }
    if (bomb->timer.HasTickedEvery(10))
    {
        Effect *eff;
        float scaleStart[2];
        float scaleTo[2];

        eff = g_EffectManager.SpawnReservedEffect(0x35, &player->pos, bomb->unk_14 % 4 + 4, 1, 0xffffffff);
        if (bomb->unk_14 & 1)
        {
            g_EtamaAnm->StartScript(&eff->sprite, 0x5c);
        }
        eff->segmentCount = 0x20;
        eff->unk_334 = 0.0f;
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
        bomb->unk_14++;
        g_SoundPlayer.QueueSoundPanned(0x11, player->pos.x);
    }
    if (player->bomb.timer.HasTicked() && player->bomb.timer.AsFrames() % 4 != 0)
    {
        PlayerBombHitbox *hitbox;
        D3DXVECTOR3 pos;

        pos = player->pos;
        pos.x = 192.0f;
        pos.y /= 2.0f;
        player->AddHitArea(&pos, 384.0f, pos.y * 2.0f, 6, 0);
        pos = player->pos;
        pos.y /= 2.0f;
        hitbox = player->AddBombHitbox(&pos, 128.0f, pos.y * 2.0f, 12, 0);
        hitbox->unk_3d = 1;
        pos.x = 192.0f;
        hitbox = player->AddBombHitbox(&pos, 384.0f, pos.y * 2.0f, 7, 0);
        hitbox->unk_3d = 1;
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
        bomb->unk_14 = 0;
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

            eff = g_EffectManager.SpawnReservedEffect(0x35, &player->pos, bomb->unk_14 % 4 + 4, 1, 0xffffffff);
            eff->segmentCount = 0x20;
            eff->unk_334 = 4.0f;
            D3DXVECTOR3 start;
            D3DXVECTOR3 to;
            start.x = 0.0f;
            start.y = 0.0f;
            start.z = 0.0f;
            to.x = 192.0f;
            to.y = g_Rng.GetRandomF32InRange(128.0f);
            to.z = 0.0f;
            eff->sprite.SetMoveInterp(30, 4, &start, &to);
            scaleStart[0] = 64.0f;
            scaleStart[1] = 0.0f;
            scaleTo[0] = 64.0f;
            scaleTo[1] = 0.0f;
            eff->sprite.SetScaleInterp(30, 1, (D3DXVECTOR2 *)scaleStart, (D3DXVECTOR2 *)scaleTo);
            eff->sprite.SetAlphaInterp(30, 3, 0xff, 0);
            eff->sprite.SetColorInterp(30, 0, 0xffffffff, 0xffff0000);
            bomb->unk_14++;
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
        bomb->unk_14 = 0;
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

            eff = g_EffectManager.SpawnReservedEffect(0x35, &player->pos, bomb->unk_14 % 4 + 4, 1, 0xffffffff);
            eff->segmentCount = 0x20;
            eff->unk_334 = 4.0f;
            D3DXVECTOR3 start;
            D3DXVECTOR3 to;
            start.x = 0.0f;
            start.y = 0.0f;
            start.z = 0.0f;
            to.x = 192.0f;
            to.y = g_Rng.GetRandomF32InRange(128.0f);
            to.z = 0.0f;
            eff->sprite.SetMoveInterp(30, 4, &start, &to);
            scaleStart[0] = 64.0f;
            scaleStart[1] = 0.0f;
            scaleTo[0] = 128.0f;
            scaleTo[1] = 0.0f;
            eff->sprite.SetScaleInterp(30, 1, (D3DXVECTOR2 *)scaleStart, (D3DXVECTOR2 *)scaleTo);
            eff->sprite.SetAlphaInterp(30, 3, 0xff, 0);
            eff->sprite.SetColorInterp(30, 0, 0xffffffff, 0xffff0000);
            bomb->unk_14++;
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
            blade->speed = g_Rng.GetRandomF32InRange(1.0f) + g_Half;
            blade->unk_8 = g_Rng.GetRandomF32InRange(0.1f) + 0.03f;
            blade->acceleration.x = g_Rng.GetRandomU16InRange(1) ? 0.15707964f : -0.15707964f;
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
            blade->speed += blade->unk_8;
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
            blade->speed = g_Rng.GetRandomF32InRange(1.0f) + g_Half;
            blade->unk_8 = g_Rng.GetRandomF32InRange(0.1f) + 0.03f;
            blade->acceleration.x = g_Rng.GetRandomU16InRange(1) ? 0.15707964f : -0.15707964f;
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
            blade->speed += blade->unk_8;
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
        g_EtamaAnm->StartScript(&eff->sprite, 0x59);
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
        g_EtamaAnm->StartScript(&eff->sprite, 0x5a);
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
        g_EtamaAnm->StartScript(&eff->sprite, 0x5b);
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
        g_EtamaAnm->StartScript(&eff->sprite, 0x5d);
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
        g_EtamaAnm->StartScript(&eff->sprite, 0x5e);
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
        g_EtamaAnm->StartScript(&eff->sprite, 0x5f);
        bomb->parts[3].pos = player->pos;
    }
    g_SpriteEngine->StepSprites(&bomb->parts[0].sprites[0], 2);
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
