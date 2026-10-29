// The player's options and shots, from 0x44e3a0: the shot half of
// th07's player.cpp, in an object of its own after player.obj.

#include <math.h>
#include <string.h>

#include "effect.h"
#include "enemy.h"
#include "player.h"
#include "sprite.h"
#include "unsorted.h"

void FireShot(Player *player, PlayerBullet *bullet, ShotData *sd)
{
    if (sd->option == 0)
    {
        bullet->pos = player->pos;
    }
    else
    {
        bullet->pos = player->options[sd->option - 1].pos;
    }
    bullet->pos[0] += sd->offsetX;
    bullet->pos[1] += sd->offsetY;
    bullet->pos[2] = 0.495f;
    bullet->size.x = sd->sizeX;
    bullet->size.y = sd->sizeY;
    bullet->size.z = 1.0f;
    bullet->angle = sd->angle;
    bullet->speed = sd->speed;
    bullet->velocity.x = cosf(sd->angle) * sd->speed;
    bullet->velocity.y = sinf(sd->angle) * sd->speed;
    bullet->timer = 0;
    bullet->focused = player->focused;
    bullet->type = sd->type;
    bullet->damage = sd->damage;
    bullet->script = sd->script;
    if (sd->sound >= 0)
    {
        g_SoundPlayer.QueueSoundPanned(sd->sound, player->pos.x);
    }
    player->anm->StartScript(&bullet->sprite, sd->script + 10);
    bullet->red = 0;
    if (g_GameManager.IsUnk22AtHigh1() && sd->unk_1e > 0)
    {
        bullet->red = 1;
    }
}

int TryFireShot(Player *player, PlayerBullet *bullet, int frame, ShotData *sd)
{
    if (frame % sd->interval == sd->phase)
    {
        FireShot(player, bullet, sd);
        return 1;
    }
    return 0;
}

int TryFireShotNoBomb(Player *player, PlayerBullet *bullet, int frame, ShotData *sd)
{
    if (!player->bomb.running && frame % sd->interval == sd->phase)
    {
        FireShot(player, bullet, sd);
        return 1;
    }
    return 0;
}

int FireLaser(Player *player, PlayerBullet *bullet, int frame, ShotData *sd)
{
    int laser = sd->phase;
    int j;

    if (player->bomb.running)
    {
        return 0;
    }
    if (g_GameManager.unk_3dbac_13)
    {
        return 0;
    }
    if (player->lasers[laser].bullet != NULL)
    {
        if (player->laserShots[laser] != sd)
        {
            player->lasers[laser].bullet->sprite.pendingInterrupt = 1;
            player->lasers[laser].bullet = NULL;
        }
        return 0;
    }
    player->lasers[laser].timer = 999;
    player->lasers[laser].bullet = bullet;
    bullet->laserIndex = laser;
    bullet->option = sd->option;
    bullet->velocity.z = sd->offsetX;
    bullet->laserOffsetY = sd->offsetY;
    bullet->trailLength = sd->interval;
    FireShot(player, bullet, sd);
    for (j = 0x1f; j >= 0; j--)
    {
        bullet->trail[j].x = -999.0f;
    }
    bullet->pos.x = -999.0f;
    player->laserShots[laser] = sd;
    return 1;
}

int FireHomingShot(Player *player, PlayerBullet *bullet, int frame, ShotData *sd)
{
    float angle;
    float speed;

    if (frame % sd->interval == sd->phase)
    {
        FireShot(player, bullet, sd);
        if (player->homingTarget.x > -100.0f)
        {
            angle = AddNormalizeAngle(atan2f(player->homingTarget.y - bullet->pos.y,
                                             player->homingTarget.x - bullet->pos.x),
                                      sd->angle + 1.5707964f);
            speed = sd->speed * 1.5f;
            ((Vector2 *)&bullet->velocity)->SetPolar(angle, speed);
            bullet->angle = angle;
        }
        return 1;
    }
    return 0;
}

int FireAimedShot(Player *player, PlayerBullet *bullet, int frame, ShotData *sd)
{
    float angle;
    float speed;

    if (frame % sd->interval == sd->phase)
    {
        FireShot(player, bullet, sd);
        angle = AddNormalizeAngle(player->aimedShotAngle, sd->angle + 1.5707964f);
        speed = sd->speed;
        ((Vector2 *)&bullet->velocity)->SetPolar(angle, speed);
        bullet->angle = angle;
        return 1;
    }
    return 0;
}

int FireOptionAimedShot(Player *player, PlayerBullet *bullet, int frame, ShotData *sd)
{
    float angle;
    float speed;

    if (!player->bomb.running && frame % sd->interval == sd->phase)
    {
        FireShot(player, bullet, sd);
        angle = AddNormalizeAngle(player->options[2].angle, sd->angle);
        speed = sd->speed;
        ((Vector2 *)&bullet->velocity)->SetPolar(angle, speed);
        bullet->angle = angle;
        return 1;
    }
    return 0;
}

int FireRandomShot(Player *player, PlayerBullet *bullet, int frame, ShotData *sd)
{
    if (frame % sd->interval == sd->phase)
    {
        FireShot(player, bullet, sd);
        bullet->angle = g_Rng.GetRandomF32MinusOneToOne() * 3.1415927f / 48.0f - 1.5707964f;
        ((Vector2 *)&bullet->velocity)->SetPolar(bullet->angle, sd->speed);
        return 1;
    }
    return 0;
}

int FireEnemyHomingShot(Player *player, PlayerBullet *bullet, int frame, ShotData *sd)
{
    float angle;
    float speed;

    if (frame % sd->interval == sd->phase)
    {
        FireShot(player, bullet, sd);
        if (player->homingEnemy != NULL)
        {
            angle = AddNormalizeAngle(atan2f(player->homingEnemy->pos.y - bullet->pos.y,
                                             player->homingEnemy->pos.x - bullet->pos.x),
                                      sd->angle + 1.5707964f);
            speed = sd->speed * 1.5f;
            ((Vector2 *)&bullet->velocity)->SetPolar(angle, speed);
            bullet->angle = angle;
        }
        return 1;
    }
    return 0;
}

int UpdateHomingOrb(Player *player, PlayerBullet *bullet)
{
    float x;
    float y;
    float len;

    if (bullet->state == 1)
    {
        if (player->orbHomingTarget.x > -100.0f && bullet->timer.AsFrames() < 40 && bullet->timer.HasTicked())
        {
            x = player->orbHomingTarget.x - bullet->pos[0];
            y = player->orbHomingTarget.y - bullet->pos[1];
            len = sqrtf(x * x + y * y) / (bullet->speed / 4.0f);
            if (len < 1.0f)
            {
                len = 1.0f;
            }
            x = x / len + bullet->velocity.x;
            y = y / len + bullet->velocity.y;
            len = sqrtf(x * x + y * y);
            bullet->speed = (len > 10.0f) ? 10.0f : len;
            if (bullet->speed < 1.0f)
            {
                bullet->speed = 1.0f;
            }
            bullet->velocity.x = x * bullet->speed / len;
            bullet->velocity.y = y * bullet->speed / len;
        }
        else if (bullet->speed < 10.0f)
        {
            bullet->speed += 0.33333334f;
            x = bullet->velocity.x;
            y = bullet->velocity.y;
            len = sqrtf(x * x + y * y);
            bullet->velocity.x = x * bullet->speed / len;
            bullet->velocity.y = y * bullet->speed / len;
        }
    }
    bullet->angle = atan2f(bullet->velocity.y, bullet->velocity.x);
    return 0;
}

int UpdateAcceleratingBullet(Player *player, PlayerBullet *bullet)
{
    if (bullet->state == 1)
    {
        bullet->velocity.y -= g_Rng.GetRandomF32InRange(0.1f) + 0.27f;
    }
    return 0;
}

int UpdateLaserBullet(Player *player, PlayerBullet *bullet)
{
    if (player->lasers[bullet->laserIndex].bullet != bullet && bullet->sprite.IsStopped())
    {
        bullet->sprite.pendingInterrupt = 1;
    }
    if ((g_Gui.IsMsgRunning() || player->bomb.running || g_GameManager.unk_3dbac_13) &&
        player->lasers[bullet->laserIndex].timer.AsFrames() > 20)
    {
        player->lasers[bullet->laserIndex].timer = 20;
    }
    if (player->lasers[bullet->laserIndex].timer <= 0)
    {
        player->lasers[bullet->laserIndex].timer = 0;
        player->lasers[bullet->laserIndex].bullet = NULL;
        bullet->state = 0;
        return 1;
    }
    if (player->lasers[bullet->laserIndex].timer <= 70 && bullet->sprite.IsStopped())
    {
        bullet->sprite.pendingInterrupt = 1;
    }
    bullet->pos.x += bullet->velocity.z;
    bullet->pos.z = 0.44f;
    if (player->playerState == 2)
    {
        return 1;
    }
    bullet->sprite.scaleX = bullet->pos.y / 14.0f;
    bullet->size.y = bullet->pos.y;
    bullet->pos.y /= 2.0f;
    if (player->lasers[bullet->laserIndex].timer < 100)
    {
        player->lasers[bullet->laserIndex].timer--;
    }
    if (g_GameManager.IsUnk22AtHigh1())
    {
        bullet->sprite.colorRgb[2] = 0xff;
        bullet->sprite.colorRgb[1] = 0xd0;
        bullet->sprite.colorRgb[0] = 0xb0;
    }
    else
    {
        bullet->sprite.colorRgb[2] = 0xff;
        bullet->sprite.colorRgb[1] = 0xff;
        bullet->sprite.colorRgb[0] = 0xff;
    }
    return 0;
}

int UpdateTrailLaserBullet(Player *player, PlayerBullet *bullet)
{
    PlayerBombHitbox *area;
    int j;

    if (player->lasers[bullet->laserIndex].bullet != bullet || g_Gui.IsMsgRunning() ||
        player->unk_e2ac4.AsFrames() < 0 || player->playerState == 2 || player->bomb.running ||
        g_GameManager.unk_3dbac_13)
    {
        bullet->sprite.pendingInterrupt = 1;
        player->lasers[bullet->laserIndex].bullet = NULL;
        bullet->update = NULL;
    }
    if (!player->options[0].active)
    {
        player->lasers[bullet->laserIndex].bullet = NULL;
        return 1;
    }
    for (j = 0; j < bullet->trailLength; j++)
    {
        if (bullet->trail[j * 2].x >= -900.0f)
        {
            area = player->AddBombHitbox(&bullet->trail[j * 2], 16.0f, 448.0f, 1, 0);
            area->unk_3d = 1;
        }
    }
    for (j = 0x1f; j > 0; j--)
    {
        bullet->trail[j] = bullet->trail[j - 1];
        bullet->trail[j].y -= 1.0f;
    }
    bullet->trail[0] = bullet->pos;
    bullet->pos = player->options[0].pos;
    bullet->pos.z = 0.44f;
    bullet->size.y = 448.0f;
    bullet->pos.y -= 208.0f;
    if (g_GameManager.IsUnk22AtHigh1())
    {
        bullet->sprite.colorRgb[2] = 0xff;
        bullet->sprite.colorRgb[1] = 0xd0;
        bullet->sprite.colorRgb[0] = 0xb0;
    }
    else
    {
        bullet->sprite.colorRgb[2] = 0xff;
        bullet->sprite.colorRgb[1] = 0xff;
        bullet->sprite.colorRgb[0] = 0xff;
    }
    return 0;
}

int DrawTrailLaserBullet(Player *player, PlayerBullet *bullet)
{
    int alpha = bullet->sprite.alpha;
    int i;
    int savedAlpha;

    savedAlpha = alpha;
    alpha = alpha * 3 / 4;
    for (i = 0; i < bullet->trailLength * 2; i += 2)
    {
        if (bullet->trail[i].x == -999.0f)
        {
            break;
        }
        bullet->sprite.pos.x = bullet->trail[i].x;
        bullet->sprite.pos.y = bullet->trail[i].y;
        bullet->sprite.pos.z = bullet->trail[i].z;
        if (i != 0)
        {
            bullet->sprite.alpha = alpha - alpha / 2 * i / bullet->trailLength;
        }
        bullet->sprite.pos.x = g_GameManager.arena[0] + bullet->sprite.pos.x;
        bullet->sprite.pos.y = g_GameManager.arena[1] + bullet->sprite.pos.y;
        if (g_GameManager.IsUnk22AtHigh1())
        {
            bullet->sprite.colorRgb[2] = 0xff;
            bullet->sprite.colorRgb[1] = 0x40;
            bullet->sprite.colorRgb[0] = 0x40;
        }
        g_SpriteEngine->Draw(&bullet->sprite);
    }
    bullet->sprite.alpha = savedAlpha;
    return 0;
}

int UpdateBombShot(Player *player, PlayerBullet *bullet, D3DXVECTOR3 *pos)
{
    float angle;

    if (bullet->state == 2)
    {
        if (bullet->timer.AsFrames() % 2 != 0)
        {
            return 1;
        }
        if (g_SpellCard.Func4178a0() && bullet->timer.AsFrames() % 4 != 0)
        {
            return 1;
        }
        bullet->damage /= 3;
        if (bullet->damage == 0)
        {
            bullet->damage = 1;
        }
        bullet->velocity.x *= 0.88f;
        bullet->velocity.y *= 0.88f;
    }
    else
    {
        angle = g_Rng.GetRandomF32InRange(1.5707964f) - 2.3561945f;
        switch (bullet->sprite.scriptIndex)
        {
        case 0xc:
            bullet->size.x = 48.0f;
            bullet->size.y = 48.0f;
            ((Vector2 *)&bullet->velocity)->SetPolar(angle, 6.0f);
            break;
        case 0xe:
            bullet->size.x = 64.0f;
            bullet->size.y = 64.0f;
            ((Vector2 *)&bullet->velocity)->SetPolar(angle, 6.0f);
            break;
        case 0x10:
            bullet->size.x = 80.0f;
            bullet->size.y = 80.0f;
            ((Vector2 *)&bullet->velocity)->SetPolar(angle, 6.0f);
            break;
        case 0x12:
            bullet->size.x = 96.0f;
            bullet->size.y = 96.0f;
            ((Vector2 *)&bullet->velocity)->SetPolar(angle, 6.0f);
            break;
        case 0x14:
            bullet->size.x = 128.0f;
            bullet->size.y = 128.0f;
            ((Vector2 *)&bullet->velocity)->SetPolar(angle, 6.0f);
            break;
        }
    }
    if (bullet->timer.AsFrames() % 6 == 0)
    {
        g_EffectManager.SpawnEffect(5, pos, 1, 0xffffffff);
    }
    return 0;
}

int BurstHitEffect(Player *player, PlayerBullet *bullet, D3DXVECTOR3 *pos)
{
    player->hitCounter++;
    if (player->hitCounter % 8 == 0)
    {
        D3DXVECTOR3 effectPos;

        effectPos = *pos;
        effectPos.x = bullet->pos.x;
        g_EffectManager.SpawnEffect(5, &effectPos, 1, 0xffffffff);
    }
    return 0;
}

void FireShots(Player *player, int frame)
{
    ShotData *shot;
    int ret;
    PlayerBullet *curBullet;
    ShotTable *shotTable;
    int i;

    shotTable = player->focused == 0 ? player->shotType->tables : player->shotTypeFocused->tables;
    if (player->bomb.running &&
        ((g_GameManager.character == 2 && (player->bomb.type & 1)) || g_GameManager.character == 9) &&
        player->bomb.timer >= 60)
    {
        // These bombs fire the shot type's seventh or eighth table.
        shotTable += (player->bomb.type & 2) ? 7 : 6;
    }
    else
    {
        while (g_GameManager.GetPower() >= shotTable->maxPower)
        {
            shotTable++;
        }
    }
    shot = shotTable->shots;
    curBullet = player->bullets;
    for (i = 0; i < 0x80; i++, curBullet++)
    {
        if (curBullet->state != 0)
        {
            continue;
        }
    again:
        if (shot->fire != NULL)
        {
            ret = shot->fire(player, curBullet, frame, shot);
        }
        else
        {
            ret = TryFireShot(player, curBullet, frame, shot);
        }
        if (ret == 1)
        {
            curBullet->sprite.flags |= 0x2000;
            curBullet->state = 1;
            curBullet->shot = shot;
            curBullet->update = curBullet->shot->update;
            curBullet->draw = curBullet->shot->draw;
            curBullet->hit = curBullet->shot->hit;
        }
        shot++;
        if (shot->interval < 0)
        {
            return;
        }
        if (ret == 0)
        {
            goto again;
        }
    }
}

void UpdateBullets(Player *player)
{
    int i;
    PlayerBullet *bullet;

    if (g_GameManager.unk_3dbac_10)
    {
        return;
    }
    bullet = player->bullets;
    for (i = 0; i < 0x80; i++, bullet++)
    {
        if (bullet->state == 0)
        {
            continue;
        }
        if (bullet->update != NULL && bullet->update(player, bullet))
        {
            bullet->state = 0;
            continue;
        }
        bullet->pos[0] += g_App.gameSpeed * bullet->velocity.x;
        bullet->pos[1] += g_App.gameSpeed * bullet->velocity.y;
        if (bullet->type != 4 && bullet->type != 5 &&
            !g_GameManager.IsInPlayArea(bullet->pos[0], bullet->pos[1], bullet->sprite.spriteData->width,
                                        bullet->sprite.spriteData->height))
        {
            bullet->state = 0;
        }
        if (g_SpriteEngine->Step(&bullet->sprite))
        {
            bullet->state = 0;
        }
        bullet->timer++;
    }
}

void DrawBullets(Player *player)
{
    PlayerBullet *bullet = player->bullets;
    int i;

    for (i = 0; i < 0x80; i++, bullet++)
    {
        if (bullet->state != 1)
        {
            continue;
        }
        if (bullet->sprite.autoRotate != 0)
        {
            bullet->sprite.SetRotationZ(bullet->angle);
        }
        bullet->sprite.pos.x = g_GameManager.arena[0] + bullet->pos.x;
        bullet->sprite.pos.y = g_GameManager.arena[1] + bullet->pos.y;
        bullet->sprite.pos.z = 0.4f;
        if (bullet->red)
        {
            bullet->sprite.colorRgb[2] = 0xff;
            bullet->sprite.colorRgb[1] = 0x40;
            bullet->sprite.colorRgb[0] = 0x40;
        }
        g_SpriteEngine->Draw(&bullet->sprite);
        if (bullet->draw != NULL)
        {
            bullet->draw(player, bullet);
        }
    }
}
