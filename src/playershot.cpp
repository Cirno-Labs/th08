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
