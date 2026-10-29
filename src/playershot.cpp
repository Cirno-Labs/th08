// The player's options and shots, from 0x44e3a0: the shot half of
// th07's player.cpp, in an object of its own after player.obj.

#include <math.h>
#include <string.h>

#include "effect.h"
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
