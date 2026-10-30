// The file with BulletManager, from 0x42f360, as th07's bullet.cpp.

#include <math.h>
#include <string.h>

#include "ascii.h"
#include "bullet.h"
#include "item.h"
#include "player.h"
#include "unsorted.h"

void BulletManager::Reset()
{
    int i;
    Bullet *bullet;

    memset(this, 0, sizeof(BulletManager));
    this->nextBullet = this->bullets;
    this->bullets[0x600].state = 6;
    this->clearItemType = 6;
    bullet = g_BulletManager.bullets;
    for (i = 0; i < 0x600; i++, bullet++)
    {
        bullet->sprites.sprite0.scriptIndex = -1;
        bullet->sprites.sprite4.scriptIndex = -1;
        bullet->sprites.sprite1.scriptIndex = -1;
        bullet->sprites.sprite2.scriptIndex = -1;
        bullet->sprites.sprite3.scriptIndex = -1;
    }
}

BulletManager::BulletManager()
{
    this->Reset();
}

// Copies `src` over `dst`. Inline; th07's also kept `dst` when it
// already ran the same script.
inline void CopyBulletSprite(Sprite *dst, Sprite *src)
{
    *dst = *src;
}

inline void ShowSpawnSprite(Sprite *dst, Sprite *src, Bullet *bullet, int color)
{
    if (dst->spriteIndex != src->spriteIndex + color)
    {
        if (bullet->sprites.sprite0.spriteData->height <= 16.0f)
        {
            g_BulletManager.etamaAnm->SetSprite(dst, src->spriteIndex + g_BulletColorOffsets16[color]);
        }
        else if (bullet->sprites.sprite0.spriteData->height <= 32.0f)
        {
            g_BulletManager.etamaAnm->SetSprite(dst, src->spriteIndex + g_BulletColorOffsets32[color]);
        }
        else
        {
            g_BulletManager.etamaAnm->SetSprite(dst, src->spriteIndex + color);
        }
    }
}

int BulletManager::SpawnBullet(BulletShooter *shooter, int col, int row, float aimAngle)
{
    float v;
    int i = 0;
    Bullet *cur;
    float angle;
    // Set and never read.
    int bulletFlags;

    cur = this->nextBullet;
    for (i = 0; i < 0x600; i++)
    {
        if (cur->state == 0)
        {
            break;
        }
        cur++;
        if (cur->state == 6)
        {
            cur = this->bullets;
        }
    }
    if (i >= 0x600)
    {
        return 1;
    }
    angle = 0.0f;
    if (shooter->count2 > 1)
    {
        v = shooter->speed1 - (shooter->speed1 - shooter->speed2) * row / shooter->count2;
    }
    else
    {
        v = shooter->speed1;
    }
    switch (shooter->aimMode)
    {
    case 0:
    case 1:
        if (shooter->count1 & 1)
        {
            angle += (col + 1) / 2 * shooter->angle2;
        }
        else
        {
            angle += col / 2 * shooter->angle2 + shooter->angle2 * 0.5f;
        }
        if (col & 1)
        {
            angle *= -1.0f;
        }
        if (shooter->aimMode == 0)
        {
            angle += aimAngle;
        }
        angle += shooter->angle1;
        break;
    case 2:
        angle += aimAngle;
    case 3:
        angle += col * 6.2831855f / shooter->count1;
        angle += row * shooter->angle2 + shooter->angle1;
        break;
    case 4:
        angle += aimAngle;
    case 5:
        angle += 3.1415927f / shooter->count1;
        angle += col * 6.2831855f / shooter->count1;
        angle += shooter->angle1;
        break;
    case 6:
        angle = g_Rng.GetRandomF32InRange(shooter->angle1 - shooter->angle2) + shooter->angle2;
        break;
    case 7:
        v = g_Rng.GetRandomF32InRange(shooter->speed1 - shooter->speed2) + shooter->speed2;
        angle += col * 6.2831855f / shooter->count1;
        angle += row * shooter->angle2 + shooter->angle1;
        break;
    case 8:
        angle = g_Rng.GetRandomF32InRange(shooter->angle1 - shooter->angle2) + shooter->angle2;
        v = g_Rng.GetRandomF32InRange(shooter->speed1 - shooter->speed2) + shooter->speed2;
        break;
    }
    cur->state = 1;
    cur->spawnedFlag = 1;
    cur->grazed = 0;
    cur->timer = 0;
    cur->unk_10b4 = 0;
    cur->grazeDelayTimer = 0;
    cur->speed = v;
    cur->angle = AddNormalizeAngle(angle, 0.0f);
    cur->pos = shooter->pos;
    cur->pos[2] = 0.1f;
    ((Vector2 *)&cur->velocity)->SetPolar(angle, v * g_App.gameSpeed);
    cur->activeFlags = shooter->flags;
    cur->color = shooter->color;
    cur->scriptFuncState = 0;
    cur->spawnedInArea = 0;
    CopyBulletSprite(&cur->sprites.sprite0, &shooter->typeSprites->sprite0);
    CopyBulletSprite(&cur->sprites.sprite4, &shooter->typeSprites->sprite4);
    cur->sprites.hitboxSize = shooter->typeSprites->hitboxSize;
    cur->sprites.unusedByteD40 = shooter->typeSprites->unusedByteD40;
    cur->sprites.height = shooter->typeSprites->height;
    cur->sprites.sizeClass = shooter->typeSprites->sizeClass;
    cur->sound = shooter->bulletSound;
    cur->boundsCheckDelay = 0;
    if (cur->sprites.sprite0.spriteIndex != shooter->typeSprites->sprite0.spriteIndex + shooter->color)
    {
        this->etamaAnm->SetSprite(&cur->sprites.sprite0, shooter->typeSprites->sprite0.spriteIndex + shooter->color);
    }
    if (cur->sprites.sprite4.spriteIndex != shooter->typeSprites->sprite4.spriteIndex + shooter->color)
    {
        if (cur->sprites.sprite0.spriteData->height <= 16.0f)
        {
            this->etamaAnm->SetSprite(&cur->sprites.sprite4, shooter->typeSprites->sprite4.spriteIndex + g_BulletColorOffsets16[shooter->color]);
        }
        else if (cur->sprites.sprite0.spriteData->height <= 32.0f)
        {
            this->etamaAnm->SetSprite(&cur->sprites.sprite4, shooter->typeSprites->sprite4.spriteIndex + g_BulletColorOffsets32[shooter->color]);
        }
        else
        {
            this->etamaAnm->SetSprite(&cur->sprites.sprite4, shooter->typeSprites->sprite4.spriteIndex + shooter->color);
        }
    }
    bulletFlags = shooter->flags;
    if (shooter->flags & 2)
    {
        CopyBulletSprite(&cur->sprites.sprite1, &shooter->typeSprites->sprite1);
        ShowSpawnSprite(&cur->sprites.sprite1, &shooter->typeSprites->sprite1, cur, shooter->color);
        cur->state = 2;
        cur->pos -= cur->velocity * 4.0f;
    }
    else if (shooter->flags & 4)
    {
        CopyBulletSprite(&cur->sprites.sprite2, &shooter->typeSprites->sprite2);
        ShowSpawnSprite(&cur->sprites.sprite2, &shooter->typeSprites->sprite2, cur, shooter->color);
        cur->state = 3;
        cur->pos -= cur->velocity * 4.0f;
    }
    else if (shooter->flags & 8)
    {
        CopyBulletSprite(&cur->sprites.sprite3, &shooter->typeSprites->sprite3);
        ShowSpawnSprite(&cur->sprites.sprite3, &shooter->typeSprites->sprite3, cur, shooter->color);
        cur->state = 4;
        cur->pos -= cur->velocity * 4.0f;
    }
    memcpy(cur->commands, shooter->commands, sizeof(cur->commands));
    cur->flags = shooter->flags;
    cur->activeFlags = 0;
    cur->commandIndex = shooter->commandIndex;
    RunBulletCommands(cur);
    if (this->clearAllFrames != 0 && !(cur->flags & 0x1000))
    {
        cur->state = 5;
    }
    cur++;
    if (cur->state == 6)
    {
        this->nextBullet = this->bullets;
    }
    else
    {
        this->nextBullet = cur;
    }
    return 0;
}

void RunBulletCommands(Bullet *bullet)
{
    BulletCommand *cmd;
    // Whether the bullet ends after firing (flag 0x1000000).
    int stop;

    for (;;)
    {
        if (bullet->commandIndex >= 18)
        {
            return;
        }
        cmd = &bullet->commands[bullet->commandIndex];
        if (cmd->flag == 0)
        {
            return;
        }
        if (cmd->concurrentFlag == 0 && bullet->activeFlags != 0)
        {
            return;
        }
        if ((bullet->flags & cmd->flag) == 0)
        {
            bullet->commandIndex++;
            continue;
        }
        switch (cmd->flag)
        {
        case 1:
            bullet->activeFlags |= 1;
            bullet->ex[0].timer = 0;
            bullet->ex[0].accel.z = 0.0f;
            break;
        case 0x10:
            bullet->activeFlags |= 0x10;
            bullet->ex[1].turnSpeed = cmd->arg0;
            bullet->ex[1].turnAngle = cmd->arg1 > -990.0f ? cmd->arg1 : bullet->angle;
            bullet->ex[1].timer = 0;
            bullet->ex[1].duration = cmd->frames;
            ((Vector2 *)&bullet->ex[1].accel)->SetPolar(bullet->ex[1].turnAngle, g_App.gameSpeed * bullet->ex[1].turnSpeed);
            if (bullet->commandIndex != 0 && bullet->sound >= 0)
            {
                g_SoundPlayer.QueueSound(bullet->sound, 0);
            }
            break;
        case 0x20:
            bullet->activeFlags |= 0x20;
            bullet->ex[2].turnSpeed = cmd->arg0;
            bullet->ex[2].turnAngle = cmd->arg1;
            bullet->ex[2].timer = 0;
            bullet->ex[2].duration = cmd->frames;
            if (bullet->commandIndex != 0 && bullet->sound >= 0)
            {
                g_SoundPlayer.QueueSound(bullet->sound, 0);
            }
            break;
        case 0x40:
        case 0x80:
        case 0x100:
            bullet->activeFlags |= cmd->flag;
            bullet->ex[3].turnAngle = cmd->arg0;
            bullet->ex[3].turnSpeed = cmd->arg1 > -999.0f ? cmd->arg1 : bullet->speed;
            bullet->ex[3].timer = 0;
            bullet->ex[3].duration = cmd->frames;
            bullet->ex[3].maxTurns = cmd->maxTurns;
            bullet->ex[3].turnCount = 0;
            break;
        case 0x400:
        case 0x800:
            bullet->activeFlags |= cmd->flag;
            if (cmd->arg0 >= 0.0f)
            {
                bullet->ex[4].turnSpeed = cmd->arg0;
            }
            else
            {
                bullet->ex[4].turnSpeed = bullet->speed;
            }
            bullet->ex[4].maxTurns = cmd->frames;
            bullet->ex[4].duration = 0;
            break;
        case 0x400000:
            bullet->activeFlags |= cmd->flag;
            bullet->ex[6].timer = cmd->frames;
            break;
        case 0x800000:
            bullet->activeFlags |= cmd->flag;
            bullet->ex[6].timer = cmd->frames;
            break;
        case 0x20000:
            bullet->activeFlags |= cmd->flag;
            bullet->ex[5].timer = cmd->frames;
            break;
        case 0x2000:
            bullet->boundsCheckDelay = cmd->frames;
            bullet->commandIndex++;
            continue;
        case 0x4000:
            bullet->sprites = g_BulletManager.typeSprites[cmd->frames];
            g_BulletManager.etamaAnm->SetSprite(&bullet->sprites.sprite0, bullet->sprites.sprite0.spriteIndex + cmd->maxTurns);
            bullet->commandIndex++;
            continue;
        case 0x40000:
            bullet->state = 5;
            break;
        case 0x80000:
            g_SoundPlayer.QueueSoundPanned(cmd->frames, bullet->pos.x);
            bullet->commandIndex++;
            continue;
        case 0x1000000:
        {
            BulletShooter shooter;

            shooter.pos = bullet->pos;
            stop = cmd->frames & 0x80000000;
            shooter.aimMode = ((DWORD)cmd->frames & 0x7f000000) >> 24;
            shooter.sprite = ((DWORD)cmd->frames & 0xff0000) >> 16;
            shooter.color = ((DWORD)cmd->frames & 0xff00) >> 8;
            shooter.commandIndex = cmd->frames & 0xff;
            shooter.count1 = cmd->maxTurns;
            shooter.speed1 = cmd->arg0;
            shooter.speed2 = cmd->arg1;
            cmd++;
            bullet->commandIndex++;
            shooter.count2 = cmd->frames;
            shooter.flags = cmd->maxTurns;
            shooter.angle1 = cmd->arg0;
            shooter.angle2 = cmd->arg1;
            memcpy(shooter.commands, bullet->commands, sizeof(shooter.commands));
            g_BulletManager.FirePattern(&shooter);
            bullet->commandIndex++;
            if (stop)
            {
                bullet->state = 5;
                break;
            }
            continue;
        }
        }
        bullet->commandIndex++;
        return;
    }
}
