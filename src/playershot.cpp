// The player's options and shots, from 0x44e3a0: the shot half of
// th07's player.cpp, in an object of its own after player.obj.

#include <math.h>
#include <string.h>

#include "effect.h"
#include "enemy.h"
#include "item.h"
#include "player.h"
#include "sprite.h"
#include "unsorted.h"

int UpdateHomingOption(Player *player, PlayerOption *option)
{
    switch (option->active)
    {
    case 1:
        player->anm->StartScript(&option->sprite, 0x12);
        option->pos = player->pos;
        option->pos.y -= 96.0f;
        if (option->pos.y < 32.0f)
        {
            option->pos.y = 32.0f;
        }
        option->active = 2;
        player->homingEnemy = NULL;
        break;
    case 2:
        switch (option->state)
        {
        case 0:
            MoveOptionToPlayer(player, option);
            if (option->velocity.x < 0.0f)
            {
                option->sprite.SetPendingInterrupt(2);
                option->state = 1;
                if (option->sprite.scaleY < 0.0f)
                {
                    option->sprite.scaleY = -option->sprite.scaleY;
                }
            }
            else if (option->velocity.x > 0.0f)
            {
                option->sprite.SetPendingInterrupt(2);
                option->state = 2;
                if (option->sprite.scaleY > 0.0f)
                {
                    option->sprite.scaleY = -option->sprite.scaleY;
                }
            }
            break;
        case 1:
            MoveOptionToPlayer(player, option);
            if (option->velocity.x == 0.0f)
            {
                option->sprite.SetPendingInterrupt(1);
                option->state = 0;
                if (option->sprite.scaleY < 0.0f)
                {
                    option->sprite.scaleY = -option->sprite.scaleY;
                }
            }
            else if (option->velocity.x > 0.0f)
            {
                option->sprite.SetPendingInterrupt(2);
                option->state = 2;
                if (option->sprite.scaleY > 0.0f)
                {
                    option->sprite.scaleY = -option->sprite.scaleY;
                }
            }
            break;
        case 2:
            MoveOptionToPlayer(player, option);
            if (option->velocity.x == 0.0f)
            {
                option->sprite.SetPendingInterrupt(1);
                option->state = 0;
                if (option->sprite.scaleY < 0.0f)
                {
                    option->sprite.scaleY = -option->sprite.scaleY;
                }
            }
            else if (option->velocity.x < 0.0f)
            {
                option->sprite.SetPendingInterrupt(2);
                option->state = 1;
                if (option->sprite.scaleY < 0.0f)
                {
                    option->sprite.scaleY = -option->sprite.scaleY;
                }
            }
            break;
        case 3:
            if (player->homingEnemy != NULL)
            {
                MoveOptionToEnemy(player, option);
            }
            if ((player->fireTimer < 0 && !(g_GameButtons & 1)) || player->homingEnemy == NULL)
            {
                player->homingEnemy = NULL;
                option->sprite.SetPendingInterrupt(1);
                option->state = 0;
            }
            break;
        }
        break;
    case 3:
        if (option->timer == 0)
        {
            option->sprite.SetPendingInterrupt(5);
        }
        if (option->timer > 16)
        {
            option->active = 0;
            option->update = NULL;
            option->draw = NULL;
        }
        break;
    }
    return 0;
}

void MoveOptionToPlayer(Player *player, PlayerOption *option)
{
    D3DXVECTOR3 target;
    D3DXVECTOR3 accel;

    target = player->pos;
    target.y -= 96.0f;
    if (target.y < 32.0f)
    {
        target.y = 32.0f;
    }
    accel = target - option->pos;
    accel /= 16.0f;
    option->velocity += (accel - option->velocity) * 0.2f;
    option->pos += option->velocity;
    if (fabsf(option->velocity.x) < 0.05f)
    {
        option->velocity.x = 0.0f;
    }
    if (player->fireTimer >= 0 && player->homingEnemy != NULL && option->timer >= 10)
    {
        option->sprite.SetPendingInterrupt(3);
        option->state = 3;
    }
    else
    {
        player->homingEnemy = NULL;
    }
}

void MoveOptionToEnemy(Player *player, PlayerOption *option)
{
    D3DXVECTOR3 target;
    D3DXVECTOR3 accel;

    target = player->homingEnemy->unk_2d88;
    target.y += 32.0f;
    if (target.y < 32.0f)
    {
        target.y = 32.0f;
    }
    accel = target - option->pos;
    accel /= 16.0f;
    option->velocity += (accel - option->velocity) * 0.2f;
    option->pos += option->velocity;
    if (fabsf(option->velocity.x) < 0.05f)
    {
        option->velocity.x = 0.0f;
    }
}

int DrawOption(Player *player, PlayerOption *option)
{
    option->sprite.pos.x = g_GameManager.arena[0] + option->pos.x;
    option->sprite.pos.y = g_GameManager.arena[1] + option->pos.y;
    option->sprite.pos.z = 0.49f;
    g_SpriteEngine->Draw(&option->sprite);
    return 0;
}

int UpdateFrontOption(Player *player, PlayerOption *option)
{
    switch (option->active)
    {
    case 1:
        player->anm->StartScript(&option->sprite, 0x1d);
        option->active = 2;
    case 2:
        if (!player->bomb.running)
        {
            option->pos = player->pos;
            option->pos.y -= 32.0f;
        }
        break;
    case 3:
        option->pos = player->pos;
        option->pos.y -= 32.0f;
        if (option->timer == 0)
        {
            option->sprite.SetPendingInterrupt(5);
        }
        if (option->timer > 16)
        {
            option->active = 0;
            option->update = NULL;
            option->draw = NULL;
        }
        break;
    }
    return 0;
}

int UpdateCirclingOption(Player *player, PlayerOption *option)
{
    switch (option->active)
    {
    case 1:
        player->anm->StartScript(&option->sprite, 0x18);
        option->active = 2;
        option->center = player->pos;
        switch (option->index)
        {
        case 0:
            option->center.x -= 30.0f;
            option->center.y -= 16.0f;
            option->orbitAngle = 0.0f;
            break;
        case 1:
            option->center.x -= 10.0f;
            option->center.y -= 32.0f;
            option->orbitAngle = 3.1415927f;
            break;
        case 2:
            option->center.x += 10.0f;
            option->center.y -= 32.0f;
            option->orbitAngle = 0.0f;
            break;
        case 3:
            option->center.x += 30.0f;
            option->center.y -= 16.0f;
            option->orbitAngle = 3.1415927f;
            break;
        }
    case 2:
        if (option->timer > 12)
        {
            switch (option->index)
            {
            case 0:
                option->orbitAngle = AddNormalizeAngle(option->orbitAngle, 0.02617994f);
                break;
            case 1:
                option->orbitAngle = AddNormalizeAngle(option->orbitAngle, -0.034906585f);
                break;
            case 2:
                option->orbitAngle = AddNormalizeAngle(option->orbitAngle, 0.034906585f);
                break;
            case 3:
                option->orbitAngle = AddNormalizeAngle(option->orbitAngle, -0.02617994f);
                break;
            }
        }
        ((Vector2 *)&option->pos)->SetPolar(option->orbitAngle, 8.0f);
        option->pos += option->center;
        break;
    case 3:
        if (option->timer == 0)
        {
            option->sprite.SetPendingInterrupt(5);
        }
        if (option->timer > 16)
        {
            option->active = 0;
            option->update = NULL;
            option->draw = NULL;
        }
        break;
    }
    return 0;
}

int UpdateFollowingCirclingOption(Player *player, PlayerOption *option)
{
    D3DXVECTOR3 target;

    switch (option->active)
    {
    case 1:
        player->anm->StartScript(&option->sprite, 0x18);
        option->active = 2;
        option->center = player->pos;
        switch (option->index)
        {
        case 0:
            option->center.x -= 30.0f;
            option->center.y -= 16.0f;
            option->orbitAngle = 0.0f;
            break;
        case 1:
            option->center.x -= 10.0f;
            option->center.y -= 32.0f;
            option->orbitAngle = 3.1415927f;
            break;
        case 2:
            option->center.x += 10.0f;
            option->center.y -= 32.0f;
            option->orbitAngle = 0.0f;
            break;
        case 3:
            option->center.x += 30.0f;
            option->center.y -= 16.0f;
            option->orbitAngle = 3.1415927f;
            break;
        }
    case 2:
        if (option->timer > 12)
        {
            switch (option->index)
            {
            case 0:
                option->orbitAngle = AddNormalizeAngle(option->orbitAngle, 0.02617994f);
                break;
            case 1:
                option->orbitAngle = AddNormalizeAngle(option->orbitAngle, -0.034906585f);
                break;
            case 2:
                option->orbitAngle = AddNormalizeAngle(option->orbitAngle, 0.034906585f);
                break;
            case 3:
                option->orbitAngle = AddNormalizeAngle(option->orbitAngle, -0.02617994f);
                break;
            }
        }
        option->sprite.color = 0xffff8080;
        ((Vector2 *)&option->pos)->SetPolar(option->orbitAngle, 8.0f);
        if (!player->focused)
        {
            target = player->pos;
            option->sprite.color = 0xff80ffff;
            switch (option->index)
            {
            case 0:
                target.x -= 30.0f;
                target.y -= 16.0f;
                break;
            case 1:
                target.x -= 10.0f;
                target.y -= 32.0f;
                break;
            case 2:
                target.x += 10.0f;
                target.y -= 32.0f;
                break;
            case 3:
                target.x += 30.0f;
                target.y -= 16.0f;
                break;
            }
            option->center = (target - option->center) * 0.2f + option->center;
        }
        option->pos += option->center;
        option->pos.z = 0.0f;
        g_EffectManager.SpawnEffect(0x2f, &option->pos, 1, 0x80602050);
        break;
    case 3:
        if (option->timer == 0)
        {
            option->sprite.SetPendingInterrupt(5);
        }
        if (option->timer > 16)
        {
            option->active = 0;
            option->update = NULL;
            option->draw = NULL;
        }
        break;
    }
    return 0;
}

int UpdateTrailingOption(Player *player, PlayerOption *option)
{
    float target;
    float dist;

    switch (option->active)
    {
    case 1:
        player->anm->StartScript(&option->sprite, 0x15);
        option->active = 2;
        option->center = player->posHistory[15];
        option->orbitAngle = 0.0f;
        option->angle = -1.5707964f;
    case 2:
        option->orbitAngle = AddNormalizeAngle(option->orbitAngle, 0.05235988f);
        ((Vector2 *)&option->pos)->SetPolar(option->orbitAngle, 8.0f);
        option->center = (player->posHistory[15] - option->center) * 0.05f + option->center;
        option->pos += option->center;
        option->pos.z = 0.0f;
        g_EffectManager.SpawnEffect(0x2f, &option->pos, 1, 0x80405080);
        switch (player->direction)
        {
        case 0:
            // Standing still keeps the angle.
            goto done;
        case 1:
            target = 1.5707964f;
            break;
        case 2:
            target = -1.5707964f;
            break;
        case 3:
            target = 0.0f;
            break;
        case 4:
            target = 3.1415927f;
            break;
        case 5:
            target = 0.7853982f;
            break;
        case 6:
            target = 2.3561945f;
            break;
        case 7:
            target = -0.7853982f;
            break;
        case 8:
            target = -2.3561945f;
            break;
        }
        dist = fabsf(option->angle - target);
        if (dist > 3.1415927f)
        {
            target += option->angle > target ? 6.2831855f : -6.2831855f;
            dist = fabsf(option->angle - target);
        }
        if (dist > 1.5707964f)
        {
            option->angle = target;
        }
        else
        {
            option->angle = AddNormalizeAngle((target - option->angle) * 0.07f, option->angle);
        }
    done:
        break;
    case 3:
        if (option->timer == 0)
        {
            option->sprite.SetPendingInterrupt(5);
        }
        if (option->timer > 16)
        {
            option->active = 0;
            option->update = NULL;
            option->draw = NULL;
        }
        break;
    }
    return 0;
}

int UpdateTintedTrailingOption(Player *player, PlayerOption *option)
{
    float target;
    float dist;

    switch (option->active)
    {
    case 1:
        player->anm->StartScript(&option->sprite, 0x15);
        option->active = 2;
        option->center = player->posHistory[15];
        option->orbitAngle = 0.0f;
        option->angle = -1.5707964f;
    case 2:
        option->orbitAngle = AddNormalizeAngle(option->orbitAngle, 0.05235988f);
        ((Vector2 *)&option->pos)->SetPolar(option->orbitAngle, 8.0f);
        option->center = (player->posHistory[15] - option->center) * 0.05f + option->center;
        option->pos += option->center;
        option->pos.z = 0.0f;
        option->sprite.color = 0xffff8080;
        if (!player->focused)
        {
            option->sprite.color = 0xffffffff;
            switch (player->direction)
            {
            case 0:
                // Standing still keeps the angle.
                goto done;
            case 1:
                target = 1.5707964f;
                break;
            case 2:
                target = -1.5707964f;
                break;
            case 3:
                target = 0.0f;
                break;
            case 4:
                target = 3.1415927f;
                break;
            case 5:
                target = 0.7853982f;
                break;
            case 6:
                target = 2.3561945f;
                break;
            case 7:
                target = -0.7853982f;
                break;
            case 8:
                target = -2.3561945f;
                break;
            }
            dist = fabsf(option->angle - target);
            if (dist > 3.1415927f)
            {
                target += option->angle > target ? 6.2831855f : -6.2831855f;
                dist = fabsf(option->angle - target);
            }
            if (dist > 1.5707964f)
            {
                option->angle = target;
            }
            else
            {
                option->angle = AddNormalizeAngle((target - option->angle) * 0.07f, option->angle);
            }
            g_EffectManager.SpawnEffect(0x2f, &option->pos, 1, 0x80405080);
        }
        else
        {
            g_EffectManager.SpawnEffect(0x2f, &option->pos, 1, 0xfff05080);
        }
    done:
        break;
    case 3:
        if (option->timer == 0)
        {
            option->sprite.SetPendingInterrupt(5);
        }
        if (option->timer > 16)
        {
            option->active = 0;
            option->update = NULL;
            option->draw = NULL;
        }
        break;
    }
    return 0;
}

int UpdateSwirlOption(Player *player, PlayerOption *option)
{
    D3DXVECTOR3 target = player->pos;

    switch (option->active)
    {
    case 1:
        player->anm->StartScript(&option->sprite, 0x15);
        option->active = 2;
        option->center = target;
        switch (option->index)
        {
        case 0:
            option->orbitAngle = 0.0f;
            break;
        case 1:
            option->orbitAngle = -3.1415927f;
            break;
        }
    case 2:
        switch (option->index)
        {
        case 0:
            target.x -= 32.0f;
            option->orbitAngle = AddNormalizeAngle(option->orbitAngle, 0.05235988f);
            break;
        case 1:
            target.x += 32.0f;
            option->orbitAngle = AddNormalizeAngle(option->orbitAngle, -0.05235988f);
            break;
        }
        ((Vector2 *)&option->pos)->SetPolar(option->orbitAngle, 6.0f);
        option->center = (target - option->center) * 0.09f + option->center;
        option->pos += option->center;
        option->pos.z = 0.0f;
        g_EffectManager.SpawnEffect(0x2f, &option->pos, 1, 0x80602050);
        break;
    case 3:
        if (option->timer == 0)
        {
            option->sprite.SetPendingInterrupt(5);
        }
        if (option->timer > 16)
        {
            option->active = 0;
            option->update = NULL;
            option->draw = NULL;
        }
        break;
    }
    return 0;
}

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
        player->fireTimer.AsFrames() < 0 || player->playerState == 2 || player->bomb.running ||
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
            area->unusedByte3d = 1;
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

void DrawHitBullets(Player *player)
{
    PlayerBullet *bullet = player->bullets;
    int i;

    for (i = 0; i < 0x80; i++, bullet++)
    {
        if (bullet->state != 2)
        {
            continue;
        }
        if (bullet->sprite.autoRotate != 0)
        {
            bullet->sprite.SetRotationZ(bullet->angle);
        }
        bullet->sprite.pos.x = g_GameManager.arena[0] + bullet->pos.x;
        bullet->sprite.pos.y = g_GameManager.arena[1] + bullet->pos.y;
        bullet->sprite.pos.z = 0.2f;
        if (bullet->red)
        {
            bullet->sprite.colorRgb[2] = 0xff;
            bullet->sprite.colorRgb[1] = 0x40;
            bullet->sprite.colorRgb[0] = 0x40;
        }
        g_SpriteEngine->DrawSprite(&bullet->sprite);
    }
}

int UpdateFiring(Player *player)
{
    if (g_GameManager.stageFrames < 20)
    {
        return 0;
    }
    if (player->fireTimer.AsFrames() < 0)
    {
        return 0;
    }
    if (player->IsSpellCardBombRunning())
    {
        return 0;
    }
    if (player->fireTimer.HasTicked() &&
        (!g_Unknown17d6ed4 ||
         (g_GameManager.character != 1 && g_GameManager.character != 7 && g_GameManager.character != 6)))
    {
        FireShots(player, player->fireTimer.AsFrames());
    }
    player->fireTimer++;
    if (player->fireTimer.AsFrames() >= 20)
    {
        player->fireTimer = -1;
    }
    if ((g_GameButtons & 1) && player->fireTimer.AsFrames() < 0 && !g_Gui.IsMsgRunning())
    {
        player->fireTimer = 0;
    }
    if (player->playerState == 2 || player->playerState == 1)
    {
        player->fireTimer = -1;
    }
    return 0;
}

void Player::StartFiring(Player *player)
{
    if (player->fireTimer.AsFrames() < 0)
    {
        player->fireTimer = 0;
    }
}

// The corners of a `size` box centred on `pos`, as th07's. Inline; the
// copy is at 0x451ce0.
inline void GetBox(D3DXVECTOR3 &min, D3DXVECTOR3 &max, D3DXVECTOR3 *pos, D3DXVECTOR3 *size)
{
    min.x = pos->x - size->x * 0.5f;
    min.y = pos->y - size->y * 0.5f;
    max.x = size->x * 0.5f + pos->x;
    max.y = size->y * 0.5f + pos->y;
}

int Player::CalcDamage(D3DXVECTOR3 *pos, D3DXVECTOR3 *size, int *damageTaken, int *hitByBomb)
{
    PlayerBullet *cur;
    // The enemy's box, and the box of the bullet or bomb hitbox tested.
    D3DXVECTOR3 enemyMin0;
    D3DXVECTOR3 eMax2;
    int idx;
    D3DXVECTOR3 targetLo;
    D3DXVECTOR3 targetHi;
    float rotZ;
    int damage2;
    PlayerBombHitbox *hitbox;

    damage2 = 0;
    if (!this->stateTimer.HasTicked())
    {
        return 0;
    }
    GetBox(enemyMin0, eMax2, pos, size);
    cur = this->bullets;
    if (hitByBomb != NULL)
    {
        *hitByBomb = 0;
    }
    for (idx = 0; idx < 0x80; idx++, cur++)
    {
        if (cur->state == 0 || (cur->state != 1 && cur->type != 3))
        {
            continue;
        }
        GetBox(targetLo, targetHi, &cur->pos, &cur->size);
        if (targetLo.y > eMax2.y || targetLo.x > eMax2.x || targetHi.y < enemyMin0.y || targetHi.x < enemyMin0.x)
        {
            continue;
        }
        if ((cur->type == 4 || cur->type == 5) && cur->timer % 2 != 0)
        {
            continue;
        }
        if (cur->hit != NULL && cur->hit(this, cur, pos))
        {
            continue;
        }
        if (!this->bomb.running)
        {
            damage2 += cur->damage;
        }
        else
        {
            damage2 += (cur->damage / 5 != 0) ? cur->damage / 5 : 1;
        }
        while (*damageTaken >= g_Unknown18b8a24)
        {
            if (g_GameManager.IsUnk22AtLow1() && cur->shot->unk_1e < 0)
            {
                g_ItemManager.SpawnItem(&cur->pos, 7, 3);
            }
            *damageTaken -= g_Unknown18b8a24;
        }
        if (cur->type != 4 && cur->type != 5 && cur->type != 6)
        {
            if (cur->state == 1)
            {
                rotZ = cur->sprite.rotation.z;
                this->anm->StartScript(&cur->sprite, cur->script + 11);
                cur->sprite.rotation.z = rotZ;
                g_EffectManager.SpawnEffect(5, &cur->pos, 1, 0xffffffff);
                cur->pos[2] = 0.1f;
            }
            cur->state = 2;
            if (cur->type != 3)
            {
                cur->velocity.x /= 8.0f;
                cur->velocity.y /= 8.0f;
            }
        }
    }
    *damageTaken += damage2 > 50 ? 50 : damage2;
    hitbox = this->bombHitboxes;
    for (idx = 0; idx < 0xc0; idx++, hitbox++)
    {
        if (!hitbox->active)
        {
            continue;
        }
        if (hitbox->framesLeft % hitbox->damageInterval != 0)
        {
            continue;
        }
        if (hitbox->radius == 0.0f)
        {
            if (hitbox->angle == 0.0f)
            {
                if (hitbox->x - hitbox->width / 2.0f > eMax2.x || hitbox->width / 2.0f + hitbox->x < enemyMin0.x ||
                    hitbox->y - hitbox->height / 2.0f > eMax2.y || hitbox->height / 2.0f + hitbox->y < enemyMin0.y)
                {
                    continue;
                }
            }
            else
            {
                targetLo.x = pos->x - hitbox->x;
                targetLo.y = pos->y - hitbox->y;
                Rotate2D(&targetHi, &targetLo, -hitbox->angle);
                if (-hitbox->width / 2.0f > size->x / 2.0f + targetHi.x ||
                    hitbox->width / 2.0f < targetHi.x - size->x / 2.0f ||
                    -hitbox->height / 2.0f > size->y / 2.0f + targetHi.y ||
                    hitbox->height / 2.0f < targetHi.y - size->y / 2.0f)
                {
                    continue;
                }
            }
        }
        else if (hitbox->radius * hitbox->radius <
                 (hitbox->x - pos->x) * (hitbox->x - pos->x) + (hitbox->y - pos->y) * (hitbox->y - pos->y))
        {
            continue;
        }
        damage2 += hitbox->damage;
        hitbox->hits += hitbox->damage;
        if (hitbox->maxDamage > 0 && hitbox->maxDamage <= hitbox->hits)
        {
            hitbox->damage = 0;
            damage2 -= hitbox->hits - hitbox->maxDamage;
        }
        if (!hitbox->unusedByte3d)
        {
            this->hitCounter++;
            if (this->hitCounter % 4 == 0)
            {
                if (idx < 0xc0)
                {
                    g_EffectManager.SpawnEffect(3, pos, 1, 0xffffffff);
                }
                else
                {
                    g_EffectManager.SpawnEffect(5, pos, 1, 0xffffffff);
                }
            }
        }
        if (this->bomb.running && hitByBomb != NULL)
        {
            *hitByBomb = 1;
        }
    }
    if (g_GameManager.IsUnk22AtHigh1() && damage2 != 0)
    {
        damage2 = damage2 * 106 / 100;
    }
    return damage2;
}
