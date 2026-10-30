// The enemies, from 0x429e00, as th07's enemy.cpp.

#include <string.h>

#include "ascii.h"
#include "effect.h"
#include "enemy.h"
#include "item.h"
#include "player.h"
#include "sprite.h"
#include "unsorted.h"

void EnemyManager::Initialize()
{
    int i;
    Enemy *enemy = this->enemies;

    memset(this, 0, sizeof(EnemyManager));
    for (i = 0; i < ARRAY_SIZE(this->timelineSignals); i++)
    {
        this->timelineSignals[i] = -1;
    }
    enemy = &this->enemyTemplate;
    memset(enemy, 0, sizeof(Enemy));
    for (i = 0; i < 2; i++)
    {
        enemy->sprites[i].scriptIndex = -1;
    }
    for (i = 0; i < 0x60; i++)
    {
        enemy->trailBoxes[i].pos.x = -999.0f;
    }
    enemy->active = 1;
    enemy->callbackTimer = 0;
    enemy->onScreen = 0;
    enemy->hitboxSize = D3DXVECTOR3(24.0f, 24.0f, 24.0f);
    enemy->velocity = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    enemy->angularVelocity = 0.0f;
    enemy->angle = 0.0f;
    enemy->acceleration = 0.0f;
    enemy->speed = 0.0f;
    enemy->moveMode = 0;
    enemy->holdFire = 0;
    enemy->mirrored = 0;
    enemy->isBoss = 0;
    enemy->stackDepth = 0;
    enemy->life = 1;
    enemy->score = 100;
    enemy->deathEffect = 0;
    enemy->deathEffectIndex = 0;
    enemy->deathEffectVariant = 0;
    enemy->shootInterval = 0;
    enemy->shootTimer = 0;
    enemy->shootOffset = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    enemy->turnLeft = -1;
    enemy->turnRight = -1;
    enemy->turnStill = -1;
    enemy->shootable = 1;
    enemy->contactDamage = 1;
    enemy->hideFollowEffects = 0;
    enemy->hitByShots = 1;
    enemy->grazeable = 0;
    enemy->deathMode = 0;
    enemy->deathSub = -1;
    enemy->clampPos = 0;
    enemy->effectCount = 0;
    enemy->pendingInterrupt = -1;
    for (i = 0; i < 4; i++)
    {
        enemy->lifeCallbackLife[i] = -1;
    }
    enemy->timerCallbackTime = -1;
    enemy->laserIndex = 0;
    enemy->hitFlashTimer = 0;
    enemy->rotateSprite = 0;
    enemy->rankSpeedLow = -0.15f;
    enemy->rankSpeedHigh = 0.15f;
    enemy->shooter.sound = 7;
    enemy->shooter.bulletSound = 0x19;
    enemy->minShootDistSq = 1024.0f;
    enemy->timeOrbDamage = g_Player.damagePerTimeOrb;
}

EnemyManager::EnemyManager()
{
    this->Initialize();
}

Enemy *EnemyManager::Spawn(int sub, D3DXVECTOR3 *pos, int life, int item, int score, int mirrored)
{
    Enemy *enemy;
    int i;

    enemy = this->enemies;
    g_ReplayManager->frameEvents |= 0x1000;
    for (i = 0; i < 0x1e0; i++, enemy++)
    {
        if (enemy->active)
        {
            continue;
        }
        *enemy = this->enemyTemplate;
        enemy->index = i;
        enemy->mirrored = mirrored;
        if (life >= 0)
        {
            enemy->life = life;
        }
        enemy->pos = *pos;
        g_EnemyScripts.StartSub(&enemy->ctx, sub);
        if (g_EnemyScripts.RunEnemy(enemy) == -1)
        {
            enemy->active = 0;
            i = 0x1e0;
            break;
        }
        enemy->color = enemy->sprite.color;
        enemy->item = (char)item;
        if (score >= 0)
        {
            enemy->score = score;
        }
        enemy->maxLife = enemy->life;
        enemy->phaseStartLife = enemy->maxLife;
        break;
    }
    this->spawnFailed = i == 0x1e0;
    return enemy;
}

Enemy *EnemyManager::SpawnWithVars(int sub, D3DXVECTOR3 *pos, int life, int item, int score,
                                   EnemyScriptVars *vars)
{
    Enemy *enemy;
    int i;

    enemy = this->enemies;
    g_ReplayManager->frameEvents |= 0x1000;
    for (i = 0; i < 0x1e0; i++, enemy++)
    {
        if (enemy->active)
        {
            continue;
        }
        *enemy = this->enemyTemplate;
        enemy->index = i;
        if (life >= 0)
        {
            enemy->life = life;
        }
        enemy->pos = *pos;
        g_EnemyScripts.StartSub(&enemy->ctx, sub);
        enemy->ctx.vars = *vars;
        if (g_EnemyScripts.RunEnemy(enemy) == -1)
        {
            enemy->active = 0;
            i = 0x1e0;
            break;
        }
        enemy->color = enemy->sprite.color;
        enemy->item = (char)item;
        if (life >= 0)
        {
            enemy->life = life;
        }
        if (score >= 0)
        {
            enemy->score = score;
        }
        enemy->maxLife = enemy->life;
        enemy->phaseStartLife = enemy->maxLife;
        break;
    }
    this->spawnFailed = i == 0x1e0;
    return enemy;
}

void ReleaseEnemyEffects(Enemy *enemy)
{
    int i;

    for (i = 0; i < enemy->effectCount; i++)
    {
        if (enemy->effects[i] == NULL)
        {
            continue;
        }
        enemy->effects[i]->fadeOut = 1;
        enemy->effects[i] = NULL;
    }
    enemy->effectCount = 0;
}

void EnemyTimeline::RunTimeline()
{
    int mirror = 0;

    while (this->pos->time >= 0)
    {
        if (this->timer == this->pos->time)
        {
            mirror = 0;
            if (!(this->pos->difficulties & g_GameManager.difficultyMask))
            {
                goto next;
            }
            switch (this->pos->opcode)
            {
            case 16:
                g_GameManager.isInContinueMenu = 1;
                break;
            case 1:
                mirror = 1;
            case 0:
                if (!g_Gui.IsBossPresent() && g_EnemyManager.holdTimelineSpawns == 0)
            {
                EnemyTimelineSpawn *args = (EnemyTimelineSpawn *)this->pos->args;
                D3DXVECTOR3 pos;

                pos.x = args->x;
                pos.y = args->y;
                pos.z = 0.0f;
                g_EnemyManager.Spawn(args->sub, &pos, args->life, args->item, args->score, mirror);
            }
                break;
            case 15:
            {
                EnemyTimelineSpawn *args = (EnemyTimelineSpawn *)this->pos->args;
                D3DXVECTOR3 pos;

                pos.x = args->x;
                pos.y = args->y;
                pos.z = 0.0f;
                g_EnemyManager.Spawn(args->sub, &pos, args->life, args->item, args->score, mirror);
            }
                break;
            case 12:
                mirror = 1;
            case 11:
                if (!g_Gui.IsBossPresent() && g_EnemyManager.holdTimelineSpawns == 0)
                {
                    EnemyTimelineSpawnWithArgs *args = (EnemyTimelineSpawnWithArgs *)this->pos->args;
                    D3DXVECTOR3 pos;
                    Enemy *enemy;

                    pos.x = args->x;
                    pos.y = args->y;
                    pos.z = 0.0f;
                    enemy = g_EnemyManager.Spawn(args->sub, &pos, args->life, -1, args->score, mirror);
                    enemy->pointItems = args->pointItems;
                    enemy->powerItems = args->powerItems;
                }
                break;
            case 4:
                mirror = 1;
            case 2:
                if (!g_Gui.IsBossPresent() && g_EnemyManager.holdTimelineSpawns == 0)
                {
                    EnemyTimelineSpawnRandomX *args = (EnemyTimelineSpawnRandomX *)this->pos->args;
                    D3DXVECTOR3 pos;

                    pos.x = g_Rng.GetRandomF32InRange(args->xMax - args->xMin) + args->xMin;
                    pos.y = args->y;
                    pos.z = 0.0f;
                    g_EnemyManager.Spawn(args->sub, &pos, args->life, args->item, args->score, mirror);
                }
                break;
            case 5:
                mirror = 1;
            case 3:
                if (!g_Gui.IsBossPresent() && g_EnemyManager.holdTimelineSpawns == 0)
                {
                    EnemyTimelineSpawnAnyX *args = (EnemyTimelineSpawnAnyX *)this->pos->args;
                    D3DXVECTOR3 pos;

                    pos.x = g_Rng.GetRandomF32InRange(384.0f);
                    pos.y = args->y;
                    pos.z = 0.0f;
                    g_EnemyManager.Spawn(args->sub, &pos, args->life, args->item, args->score, mirror);
                }
                break;
            case 6:
                g_Gui.StartMsg(this->pos->args[0]);
                break;
            case 7:
                if (g_Gui.MsgHoldsTimeline())
                {
                    this->timer--;
                    goto tick;
                }
                break;
            case 8:
                g_EnemyManager.bosses[this->pos->args[0]]->pendingInterrupt = this->pos->args[1];
                break;
            case 9:
                g_GameManager.SetPower(this->pos->args[0]);
                break;
            case 10:
                if (g_EnemyManager.bosses[this->pos->args[0]] != NULL &&
                    g_EnemyManager.bosses[this->pos->args[0]]->active)
                {
                    this->timer--;
                    goto tick;
                }
                break;
            case 13:
            {
                int count;
                int j;

                count = 0;
                for (j = 0; j < ARRAY_SIZE(g_EnemyManager.timelineSignals); j++)
                {
                    if (g_EnemyManager.timelineSignals[j] == this->pos->args[0])
                    {
                        count++;
                        g_EnemyManager.timelineSignals[j] = -1;
                    }
                }
                if (count == 0)
                {
                    this->timer--;
                    goto tick;
                }
                break;
            }
            case 14:
            {
                int k;

                for (k = 0; k < ARRAY_SIZE(g_EnemyManager.timelineSignals); k++)
                {
                    if (g_EnemyManager.timelineSignals[k] >= 0)
                    {
                        continue;
                    }
                    g_EnemyManager.timelineSignals[k] = this->pos->args[0];
                }
                break;
            }
            }
        }
        else if (this->timer < this->pos->time)
        {
            break;
        }
    next:
        this->pos = (EnemyTimelineEntry *)((char *)this->pos + this->pos->size);
    }
tick:
    this->timer++;
}

void Enemy::ReleaseFollowers(int collect)
{
    int count;
    Enemy *nxt;
    Enemy *cur;
    D3DCOLOR tint;
    int k;

    count = this->CountFollowers();
    if (count != 0)
    {
        k = 0;
        D3DXVECTOR3 pos;
        cur = this->nextFollower;
        tint = count < 2 ? 0xffffffff : (count < 6 ? 0xffffffd0 : (count < 10 ? 0xffffffb0 : 0xffffff80));
        while (cur != NULL)
        {
            if (cur->posRelativeToLeader)
            {
                cur->posOrigin = this->pos;
            }
            nxt = cur->nextFollower;
            cur->releasedFromLeader = 1;
            cur->leader = NULL;
            cur->nextFollower = NULL;
            cur->prevFollower = NULL;
            if (collect)
            {
                int areaType;
                int i;
                int items;

                areaType = this->isBoss ? 7 : 9;
                if (g_GameManager.IsSoloYoukai())
                {
                    items = count >= 10 ? 26 : count * 2 + 6;
                }
                else if (g_GameManager.IsSoloHuman())
                {
                    items = count >= 4 ? 40 : count * 6 + 16;
                }
                else
                {
                    items = count >= 8 ? 26 : count * 2 + 10;
                }
                if (g_Player.bomb.running != 0)
                {
                    count /= 3;
                }
                g_AsciiText.AddNumberPopup(&cur->absPos, count, 0, tint);
                cur->absPos = cur->pos + cur->posOrigin;
                g_Player.AddHitAreaCircle(&cur->absPos, 32.0f, 2.0f, 8, areaType);
                for (i = 0; i < items; i++)
                {
                    ((Vector2 *)&pos)->SetPolar(g_Rng.GetRandomF32SignedInRange(3.1415927f),
                                                g_Rng.GetRandomF32InRange(items * 2.0f));
                    pos.z = 0.0f;
                    pos += cur->absPos;
                    g_ItemManager.SpawnItem(&pos, 7, 3);
                }
                if (!g_EnemyManager.AnyBossActive() || g_SpellCard.IsActive())
                {
                    cur->item = 8;
                    cur->DropItems(0);
                }
                g_SoundPlayer.QueueSoundPanned(k % 2 + 2, cur->absPos.x);
            }
            cur->powerItems = 0;
            cur->pointItems = 0;
            cur->item = -2;
            cur = nxt;
            k++;
        }
        if (collect)
        {
            g_AsciiText.SetScale(2.0f, 2.0f);
            g_AsciiText.AddNumberPopup(&this->absPos, this->followerCount, 0, 0xfff0f00f);
            g_AsciiText.SetScale(1.0f, 1.0f);
            for (count = 0; count < this->followerCount * 2; count++)
            {
                ((Vector2 *)&pos)->SetPolar(g_Rng.GetRandomF32SignedInRange(3.1415927f),
                                            g_Rng.GetRandomF32InRange(128.0f));
                pos.z = 0.0f;
                pos += this->absPos;
                g_ItemManager.SpawnItem(&pos, 7, 1);
            }
            g_Player.AddHitAreaCircle(&this->absPos, 32.0f, 1.0f, 16, 7);
            g_Player.orbGaugeCooldown = 0;
        }
    }
    if (this->IsFollower() && collect)
    {
        D3DXVECTOR3 unused;

        g_GameManager.AddGauge(-g_GameManager.GetGauge() / 12, 0);
        g_Player.gaugeShiftTimer = 0;
        g_Player.gaugeIdleTimer = 30;
        g_Player.orbGaugeCooldown = 50;
        this->absPos = this->pos + this->posOrigin;
        g_AsciiText.AddNumberPopup(&this->absPos, 1, 0, 0xffffffff);
        g_ItemManager.SpawnItem(&this->absPos, 7, 1);
        this->powerItems = 0;
        this->pointItems = 0;
        this->item = -2;
    }
    this->LeaveFollowerList();
}

void Enemy::LeaveFollowerList()
{
    if (this->IsFollower())
    {
        this->prevFollower->nextFollower = this->nextFollower;
        if (this->nextFollower != NULL)
        {
            this->nextFollower->prevFollower = this->prevFollower;
        }
        this->leader = 0;
        this->nextFollower = NULL;
        this->prevFollower = NULL;
    }
    else
    {
        this->prevFollower = NULL;
        this->nextFollower = NULL;
    }
}

void Enemy::ShareDamageWithLeader(int damage)
{
    int share;
    int i;
    int threshold;

    if (!this->IsFollower())
    {
        return;
    }
    if (g_Player.bomb.running != 0)
    {
        return;
    }
    threshold = 0;
    for (i = 0; i < 4; i++)
    {
        if (threshold < this->leader->lifeCallbackLife[i])
        {
            threshold = this->leader->lifeCallbackLife[i];
        }
    }
    share = damage / 2;
    if (this->leader->invulnTimer > 0)
    {
        if (this->leader->isBoss)
        {
            share /= 9;
        }
        else
        {
            share = 0;
        }
    }
    if (share == 0)
    {
        return;
    }
    this->leader->life -= share;
    if (this->leader->life <= threshold)
    {
        this->leader->life = threshold;
    }
}

void Enemy::FreeThreads()
{
    int i;

    for (i = 0; i < 4; i++)
    {
        if (this->threads[i] != NULL)
        {
            g_MemoryManager.Free(this->threads[i]);
            this->threads[i] = NULL;
        }
    }
}

void Enemy::Despawn()
{
    int i;

    this->ReleaseFollowers(0);
    if (this->deathMode == 0)
    {
        this->active = 0;
    }
    if (this->isBoss && this->bossIndex < 4)
    {
        g_Gui.SetBossPresent(0);
        g_EnemyManager.bosses[this->bossIndex] = NULL;
        this->isBoss = 0;
        g_AsciiText.SetNearbyLabelInterrupt(this->bossIndex, 2);
        g_AsciiText.SetNearbyLabelPos(this->bossIndex, &D3DXVECTOR3(-999.0f, -999.0f, 0.0f));
    }
    if (this->effectCount != 0)
    {
        ReleaseEnemyEffects(this);
    }
    if (this->isBoss)
    {
        g_EnemyManager.bosses[this->bossIndex] = NULL;
    }
    g_ReplayManager->frameEvents |= 0x20;
    if (this->attachedEffect != NULL)
    {
        this->attachedEffect->sprite.SetPendingInterrupt(3);
        this->attachedEffect = NULL;
    }
    for (i = 0; i < 4; i++)
    {
        this->lifeCallbackLife[i] = -1;
    }
    this->timerCallbackTime = -1;
    this->FreeThreads();
    if (g_Player.homingEnemy == this)
    {
        g_Player.homingEnemy = NULL;
    }
}

int Enemy::CheckLifeCallback()
{
    DWORD level;
    int count;
    int i;
    int tmp;
    Enemy *enemy;
    int k;

    count = 0;
    level = 0;
    this->lifeCallbackProximity = 0;
    for (i = 0; i < 4; i++)
    {
        if (this->lifeCallbackLife[i] < 0)
        {
            continue;
        }
        count++;
        if (this->life < this->lifeCallbackLife[i])
        {
            this->life = this->lifeCallbackLife[i];
            this->phaseStartLife = this->life;
            g_EnemyScripts.StartSub(&this->ctx, this->lifeCallbackSub[i]);
            this->lifeCallbackLife[i] = -1;
            this->phaseEndSecondsLeft = (this->timerCallbackTime - this->callbackTimer.AsFrames()) / 60;
            this->timerCallbackTime = -1;
            for (tmp = 0; tmp < 4; tmp++)
            {
                if (this->threads[tmp] != NULL)
                {
                    g_MemoryManager.Free(this->threads[tmp]);
                    this->threads[tmp] = NULL;
                }
            }
            this->ResetRankSpeeds();
            this->stackDepth = 0;
            this->lifeCallbackProximity = 0;
            this->shooter = g_EnemyManager.enemyTemplate.shooter;
            this->shootInterval = 0;
            this->ReleaseFollowers(1);
            for (enemy = g_EnemyManager.enemies, k = 0; k < 0x1e0; k++, enemy++)
            {
                if (!enemy->active)
                {
                    continue;
                }
                if (enemy->isBoss)
                {
                    continue;
                }
                enemy->life = 0;
                if (enemy->deathSub >= 0)
                {
                    g_EnemyScripts.StartSub(&enemy->ctx, enemy->deathSub);
                    enemy->deathSub = -1;
                }
            }
            if (this->isBoss && g_Player.playerState == 0)
            {
                g_Player.stateTimer = 70;
                g_Player.playerState = 3;
            }
            return 1;
        }
        tmp = this->life - this->lifeCallbackLife[i];
        if (g_SpellCard.IsActive())
        {
            if (tmp < 120)
            {
                level = 3;
            }
            else if (tmp < 200)
            {
                level = 2;
            }
            else if (tmp < 300)
            {
                level = 1;
            }
            else
            {
                level = 0;
            }
        }
        else
        {
            if (tmp < 500)
            {
                level = 3;
            }
            else if (tmp < 1500)
            {
                level = 2;
            }
            else if (tmp < 2200)
            {
                level = 1;
            }
            else
            {
                level = 0;
            }
        }
        if (this->lifeCallbackProximity < level)
        {
            this->lifeCallbackProximity = level;
        }
    }
    if (count == 0)
    {
        tmp = this->life;
        if (this->isBoss)
        {
            if (g_SpellCard.IsActive())
            {
                if (tmp < 120)
                {
                    level = 3;
                }
                else if (tmp < 300)
                {
                    level = 2;
                }
                else if (tmp < 400)
                {
                    level = 1;
                }
                else
                {
                    level = 0;
                }
            }
            else
            {
                if (tmp < 600)
                {
                    level = 3;
                }
                else if (tmp < 1600)
                {
                    level = 2;
                }
                else if (tmp < 2400)
                {
                    level = 1;
                }
                else
                {
                    level = 0;
                }
            }
        }
        else
        {
            if (g_SpellCard.IsActive())
            {
                if (tmp < 10)
                {
                    level = 3;
                }
                else
                {
                    level = 0;
                }
            }
            else
            {
                if (tmp < 50)
                {
                    level = 3;
                }
                else
                {
                    level = 0;
                }
            }
        }
        if (this->lifeCallbackProximity < level)
        {
            this->lifeCallbackProximity = level;
        }
    }
    return 0;
}

int Enemy::CheckTimerCallback()
{
    Enemy *enemy;
    int i;
    int j;
    int idx;
    int best;

    if (this->isBoss && this->bossIndex == 0)
    {
        g_Gui.SetBossTimer((this->timerCallbackTime - this->callbackTimer.AsFrames()) / 60);
    }
    if (this->callbackTimer >= this->timerCallbackTime)
    {
        this->phaseEndSecondsLeft = 0;
        best = 0;
        for (i = 0; i < 4; i++)
        {
            if (this->lifeCallbackLife[i] < 0)
            {
                continue;
            }
            if (best < this->lifeCallbackLife[i])
            {
                best = this->lifeCallbackLife[i];
                idx = i;
            }
        }
        if (best > 0)
        {
            this->life = this->lifeCallbackLife[idx];
            this->phaseStartLife = this->life;
            this->lifeCallbackLife[idx] = -1;
        }
        g_EnemyScripts.StartSub(&this->ctx, this->timerCallbackSub);
        this->timerCallbackTime = -1;
        this->timerCallbackSub = this->deathSub;
        this->callbackTimer = 0;
        if (!this->keepSpellCard)
        {
            g_SpellCard.TimeOut();
            g_BulletManager.ClearAll(4);
        }
        if (this->isBoss && g_Player.playerState == 0)
        {
            g_Player.stateTimer = 70;
            g_Player.playerState = 3;
        }
        this->ReleaseFollowers(0);
        for (enemy = g_EnemyManager.enemies, j = 0; j < 0x1e0; j++, enemy++)
        {
            if (!enemy->active)
            {
                continue;
            }
            if (enemy->isBoss)
            {
                continue;
            }
            enemy->life = 0;
            if (enemy->deathSub >= 0)
            {
                g_EnemyScripts.StartSub(&enemy->ctx, enemy->deathSub);
                enemy->deathSub = -1;
            }
        }
        for (idx = 0; idx < 4; idx++)
        {
            if (this->threads[idx] != NULL)
            {
                g_MemoryManager.Free(this->threads[idx]);
                this->threads[idx] = NULL;
            }
        }
        this->shooter = g_EnemyManager.enemyTemplate.shooter;
        this->shootInterval = 0;
        this->ResetRankSpeeds();
        this->stackDepth = 0;
        this->lifeCallbackProximity = 0;
        return 1;
    }
    else
    {
        return 0;
    }
}

void Enemy::DropItems(int byBomb)
{
    D3DXVECTOR3 pos;
    int i;

    if (this->item >= 0)
    {
        g_EffectManager.SpawnEffect(this->deathEffectIndex + 4, &this->absPos, 3, 0xffffffff);
        g_ItemManager.SpawnItem(&this->absPos, this->item, byBomb != 0);
    }
    else if (this->item == -1)
    {
        if (g_EnemyManager.dropCounter % 3 == 0)
        {
            g_EffectManager.SpawnEffect(this->deathEffectIndex + 4, &this->absPos, 6, 0xffffffff);
            g_ItemManager.SpawnItem(&this->absPos, g_EnemyItemDrops[g_EnemyManager.dropIndex], byBomb != 0);
            g_EnemyManager.dropIndex++;
            if (g_EnemyManager.dropIndex >= 0x20)
            {
                g_EnemyManager.dropIndex = 0;
            }
        }
        g_EnemyManager.dropCounter++;
    }
    if (this->powerItems != 0)
    {
        for (i = 0; i < this->powerItems; i++)
        {
            pos = this->absPos;
            pos[0] += g_Rng.GetRandomF32ZeroToOne() * 128.0f - 64.0f;
            pos[1] += g_Rng.GetRandomF32ZeroToOne() * 128.0f - 64.0f;
            if (g_GameManager.GetPower() < 0x80)
            {
                g_ItemManager.SpawnItem(&pos, 0, 0);
            }
            else
            {
                g_ItemManager.SpawnItem(&pos, 1, 0);
            }
        }
        this->powerItems = 0;
    }
    if (this->pointItems != 0)
    {
        for (i = 0; i < this->pointItems; i++)
        {
            pos = this->absPos;
            pos[0] += g_Rng.GetRandomF32ZeroToOne() * 128.0f - 64.0f;
            pos[1] += g_Rng.GetRandomF32ZeroToOne() * 128.0f - 64.0f;
            g_ItemManager.SpawnItem(&pos, 1, 0);
        }
        this->pointItems = 0;
    }
}

void Enemy::ClampPos()
{
    if (this->clampPos)
    {
        if (this->pos[0] < this->lowerBound[0])
        {
            this->pos[0] = this->lowerBound[0];
        }
        else if (this->pos[0] > this->upperBound[0])
        {
            this->pos[0] = this->upperBound[0];
        }
        if (this->pos[1] < this->lowerBound[1])
        {
            this->pos[1] = this->lowerBound[1];
        }
        else if (this->pos[1] > this->upperBound[1])
        {
            this->pos[1] = this->upperBound[1];
        }
    }
}

void Enemy::CheckPlayerContact(D3DXVECTOR3 *pos, D3DXVECTOR3 *size)
{
    D3DXVECTOR3 box;

    box = *size / 0.7f;
    if (this->grazeable && this->callbackTimer.HasTicked() && this->callbackTimer % 6 == 0)
    {
        g_Player.CheckGraze(pos, &box);
    }
    if ((g_GameManager.character == 0 || g_GameManager.character == 4) && this->IsFollower())
    {
        return;
    }
    box = *size / 1.5f;
    if (g_Player.CheckHitIgnoringAreas(pos, &box) == 1 && !this->isBoss && !this->grazeable)
    {
        this->life -= 10;
    }
}

void EnemyManager::AddRankOverTime()
{
    int interval;

    if (!g_Gui.IsMsgRunning())
    {
        interval = 2400;
        interval -= g_GameManager.GetLives() * 4 * 60;
        if (this->frameTimer.HasTicked() && this->frameTimer.AsFrames() % interval == 0)
        {
            g_GameManager.AddSubRank(100);
        }
    }
}

void Enemy::UpdateYoukaiState()
{
    if (!this->isYoukai)
    {
        if (g_Player.IsYoukai())
        {
            g_EffectManager.SpawnEffect(0x1f, &this->absPos, 1, 0x80303080);
            if (this->attachedEffect != NULL)
            {
                this->attachedEffect->sprite.SetPendingInterrupt(2);
            }
            g_SoundPlayer.QueueSound(0x28, 0);
            this->drawListIndex = 0;
        }
        if (this->emitsEffect26 && this->callbackTimer.HasTickedEvery(2))
        {
            g_EffectManager.SpawnEffect(0x26, &this->absPos, 1, 0xffffffff);
        }
    }
    else if (!g_Player.IsYoukai())
    {
        g_EffectManager.SpawnEffect(0x1e, &this->absPos, 1, 0x80803030);
        if (this->attachedEffect != NULL)
        {
            this->attachedEffect->sprite.SetPendingInterrupt(1);
        }
        g_SoundPlayer.QueueSound(0x27, 0);
        this->drawListIndex = 2;
    }
    this->isYoukai = g_Player.IsYoukai();
    this->difficultyMaskExtra = g_Player.IsYoukai() ? 0x40 : 0x20;
}

int EnemyManager::RegisterChain()
{
    EnemyManager *em = &g_EnemyManager;
    int unused = 0;

    em->Initialize();
    g_EnemyManagerCalcChain.SetCallback((ChainCallback)EnemyManager::OnUpdate);
    g_EnemyManagerCalcChain.addedCallback = (ChainCallback)EnemyManager::AddedCallback;
    g_EnemyManagerCalcChain.deletedCallback = (ChainCallback)EnemyManager::DeletedCallback;
    g_EnemyManagerCalcChain.arg = em;
    if (g_Chain.AddToCalcChain(&g_EnemyManagerCalcChain, 11) != 0)
    {
        return -1;
    }
    g_EnemyManagerDrawChain.SetCallback((ChainCallback)EnemyManager::OnDraw);
    g_EnemyManagerDrawChain.arg = em;
    if (g_Chain.AddToDrawChain(&g_EnemyManagerDrawChain, 8) != 0)
    {
        return -1;
    }
    g_EnemyManagerDrawChain2.SetCallback((ChainCallback)EnemyManager::OnDraw2);
    g_EnemyManagerDrawChain2.arg = em;
    if (g_Chain.AddToDrawChain(&g_EnemyManagerDrawChain2, 11) != 0)
    {
        return -1;
    }
    return 0;
}

int EnemyManager::OnUpdate(EnemyManager *mgr)
{
    int bomb = 0;
    int level;
    int hurt;
    int i;
    D3DXVECTOR3 boxSize;
    int extraHurt;
    int hit;
    int seg;
    Enemy *enemy;
    D3DXVECTOR3 targetDiff;
    int points;
    D3DXVECTOR3 toward;

    level = 10;
    if (!g_Gui.IsMsgRunning())
    {
        g_GameManager.progressFrames++;
        if (mgr->frameTimer.AsFrames() >= 16)
        {
            g_GameManager.humanRateFrames++;
            if (!g_Player.focused)
            {
                g_GameManager.humanFrames++;
            }
        }
    }
    if (g_GameManager.deathbombFreeze)
    {
        return 1;
    }
    if (g_GameManager.bossHitAnywhere && mgr->bosses[0] != NULL)
    {
        D3DXVECTOR3 size(384.0f, 448.0f, 0.0f);
        D3DXVECTOR3 center(192.0f, 224.0f, 0.0f);

        g_Player.CalcDamage(&center, &size, &mgr->bosses[0]->timeOrbDamage, &bomb);
    }
    mgr->AddRankOverTime();
    mgr->drawListHeads[3] = NULL;
    mgr->drawListHeads[2] = NULL;
    mgr->drawListHeads[1] = NULL;
    mgr->drawListHeads[0] = NULL;
    for (i = 0; i < g_EnemyScripts.GetTimelineCount(); i++)
    {
        if (mgr->timelines[i].pos == NULL)
        {
            mgr->timelines[i].pos = g_EnemyScripts.GetTimeline(i);
        }
        mgr->timelines[i].RunTimeline();
    }
    enemy = mgr->enemies;
    mgr->enemyCount = 0;
    for (i = 0; i < 0x1e0; i++, enemy++)
    {
        if (!enemy->active)
        {
            if (g_Player.homingEnemy == enemy)
            {
                g_Player.homingEnemy = NULL;
            }
            continue;
        }
        hit = 0;
        if (enemy->releasedFromLeader)
        {
            enemy->absPos = enemy->pos + enemy->posOrigin;
            enemy->absPos.z = 0.0f;
            goto die;
        }
        mgr->enemyCount++;
        if ((enemy->haltedByBomb && (g_Player.bomb.running || g_Player.playerState)) || enemy->halted)
        {
            enemy->callbackTimer--;
            goto flash;
        }
        if (enemy->followsYoukaiState)
        {
            enemy->UpdateYoukaiState();
        }
    again:
        if (g_EnemyScripts.RunEnemy(enemy) == -1)
        {
            enemy->active = 0;
            enemy->Despawn();
            continue;
        }
        if (!enemy->isWarpingToBoss)
        {
            enemy->ClampPos();
            enemy->Move();
            enemy->ClampPos();
            if (enemy->leader != NULL && enemy->posRelativeToLeader)
            {
                enemy->posOrigin = enemy->leader->pos;
            }
            enemy->absPos = enemy->pos + enemy->posOrigin;
            enemy->absPos.z = 0.0f;
        }
        else
        {
            enemy->absPos = enemy->pos + enemy->posOrigin;
            enemy->absPos.z = 0.0f;
        }
        if (enemy->attachedEffect != NULL)
        {
            enemy->attachedEffect->pos = enemy->absPos;
        }
        if (enemy->trailFlags)
        {
            for (seg = enemy->trailSegmentCount - 1; seg > 0; seg--)
            {
                enemy->trailBoxes[seg].pos = enemy->trailBoxes[seg - 1].pos;
                enemy->trailBoxes[seg].velocity = enemy->trailBoxes[seg - 1].velocity;
                enemy->trailBoxes[seg].angle = enemy->trailBoxes[seg - 1].angle;
            }
            enemy->trailBoxes[0].pos = enemy->absPos;
            enemy->trailBoxes[0].velocity = enemy->velocity;
            enemy->trailBoxes[0].angle = enemy->angle;
        }
        if (enemy->sprite.spriteData == NULL)
        {
            enemy->hideFollowEffects = 1;
        }
        if (!enemy->hideFollowEffects && !enemy->onScreen &&
            g_GameManager.IsInPlayArea(enemy->absPos[0], enemy->absPos[1], enemy->sprite.spriteData->width,
                                       enemy->sprite.spriteData->height))
        {
            enemy->onScreen = 1;
        }
        else if (enemy->onScreen == 1 && !enemy->staysOffscreen &&
                 ((!enemy->trailFlags &&
                   !g_GameManager.IsInPlayArea(enemy->absPos.x, enemy->absPos.y,
                                               enemy->sprite.spriteData->width,
                                               enemy->sprite.spriteData->height)) ||
                  (enemy->trailFlags &&
                   !g_GameManager.IsInPlayArea(enemy->absPos.x, enemy->absPos.y,
                                               enemy->sprite.spriteData->width,
                                               enemy->sprite.spriteData->height) &&
                   !g_GameManager.IsInPlayArea(enemy->trailBoxes[enemy->trailSegmentCount - 1].pos.x,
                                               enemy->trailBoxes[enemy->trailSegmentCount - 1].pos.y,
                                               enemy->sprite.spriteData->width,
                                               enemy->sprite.spriteData->height))))
        {
            enemy->active = 0;
            enemy->Despawn();
            continue;
        }
        if (enemy->CheckLifeCallback())
        {
            goto again;
        }
        if (enemy->timerCallbackTime >= 0 && enemy->CheckTimerCallback())
        {
            goto again;
        }
        enemy->sprite.color = enemy->color;
        g_SpriteEngine->Step(&enemy->sprite);
        enemy->color = enemy->sprite.color;
        for (seg = 0; seg < 2; seg++)
        {
            if (enemy->sprites[seg].scriptIndex >= 0 && g_SpriteEngine->Step(&enemy->sprites[seg]))
            {
                enemy->sprites[seg].scriptIndex = -1;
            }
        }
        bomb = g_Player.bomb.running;
        if (!enemy->hideFollowEffects && !enemy->hiddenByBomb && !enemy->isYoukai &&
            (!enemy->intangibleDuringBomb || !g_Player.bomb.running))
        {
            if (enemy->shootable)
            {
                enemy->CheckPlayerContact(&enemy->absPos, &enemy->hitboxSize);
                if (enemy->trailFlags)
                {
                    boxSize = enemy->hitboxSize;
                    for (seg = 1; seg < enemy->trailContactSegments; seg += 6)
                    {
                        if (enemy->trailFlags & 2)
                        {
                            boxSize = enemy->hitboxSize -
                                      enemy->hitboxSize * (float)seg / (float)enemy->trailContactSegments;
                        }
                        enemy->CheckPlayerContact(&enemy->trailBoxes[seg].pos, &boxSize);
                    }
                }
            }
            enemy->lastDamage = 0;
            if (enemy->hitByShots)
            {
                if (!g_SpellCard.IsActive() || !enemy->IsFollower() || !g_Player.bomb.running)
                {
                    hurt = g_Player.CalcDamage(&enemy->absPos, &enemy->hitboxSize, &enemy->timeOrbDamage, &bomb);
                }
                else
                {
                    hurt = 0;
                }
                if (enemy->extraHitboxSize.x > 0.0f)
                {
                    extraHurt = g_Player.CalcDamage(&enemy->absPos, &enemy->extraHitboxSize, &enemy->timeOrbDamage, &bomb);
                    if (bomb == 0)
                    {
                        if (g_GameManager.character == 3 || g_GameManager.character == 11)
                        {
                            hurt = hurt + extraHurt / 6.5f;
                        }
                        else
                        {
                            hurt = hurt + extraHurt / 1.7f;
                        }
                    }
                }
                if (hurt > 0)
                {
                    if ((enemy->isBoss || !g_Player.focused) && !g_Player.bomb.running)
                    {
                        if (enemy->isBoss && !g_Player.focused)
                        {
                            points = hurt / (10 - level / 3) * 10;
                        }
                        else
                        {
                            points = hurt / (30 - level) * 10;
                        }
                        if (points > 70)
                        {
                            points = 70;
                        }
                        if (points == 0 && (!g_Player.focused || enemy->callbackTimer.AsFrames() & 1))
                        {
                            points = 10;
                        }
                    }
                    if (hurt >= 70)
                    {
                        hurt = 70;
                    }
                    g_GameManager.AddScore(hurt / 5 * 10);
                    if (enemy->contactDamage)
                    {
                        if (g_SpellCard.IsActive())
                        {
                            if (bomb == 0)
                            {
                                if (hurt > 7)
                                {
                                    hurt /= 7;
                                }
                                else if (hurt != 0)
                                {
                                    hurt = 1;
                                }
                            }
                            else if (g_SpellCard.WasBombed() && !enemy->IsFollower())
                            {
                                if (hurt > 2)
                                {
                                    hurt = hurt / 2.5f;
                                }
                                else if (hurt != 0)
                                {
                                    hurt = 1;
                                }
                            }
                            else
                            {
                                hurt = 0;
                            }
                        }
                        if (enemy->invulnTimer > 0)
                        {
                            if (enemy->isBoss)
                            {
                                hurt /= 9;
                            }
                            else
                            {
                                hurt = 0;
                            }
                        }
                        enemy->life -= hurt;
                        enemy->lastDamage = hurt;
                        enemy->ShareDamageWithLeader(hurt);
                    }
                    hit = 1;
                }
                if (enemy->isBoss)
                {
                    targetDiff = g_Player.orbHomingTarget - g_Player.pos;
                    toward = enemy->absPos - g_Player.pos;
                    if (!g_Player.homingTargetSet || fabsf(targetDiff.x) > fabsf(toward.x))
                    {
                        g_Player.orbHomingTarget = enemy->absPos;
                    }
                    g_Player.homingTargetSet = 1;
                }
                if (!g_Player.homingTargetSet)
                {
                    if (g_Player.orbHomingTarget[1] < enemy->absPos[1])
                    {
                        g_Player.orbHomingTarget = enemy->absPos;
                    }
                }
                if (fabsf(enemy->absPos.x - g_Player.pos.x) < 64.0f && !enemy->IsFollower() &&
                    (g_Player.homingEnemy == NULL || g_Player.homingEnemy->pos.y > enemy->absPos.y))
                {
                    g_Player.homingEnemy = enemy;
                }
            }
        }
        if (enemy->hasDied && enemy->life > 0)
        {
            enemy->hasDied = 0;
        }
        if (enemy->life <= 0 && !enemy->hasDied && !enemy->undying)
        {
            int k;

        die:
            enemy->hasDied = 1;
            // Unused.
            D3DXVECTOR3 pos;
            enemy->phaseEndSecondsLeft = (enemy->timerCallbackTime - enemy->callbackTimer.AsFrames()) / 60;
            enemy->timerCallbackTime = -1;
            for (k = 0; k < 4; k++)
            {
                enemy->lifeCallbackLife[k] = -1;
            }
            for (k = 0; k < 4; k++)
            {
                if (enemy->threads[k] != NULL)
                {
                    g_MemoryManager.Free(enemy->threads[k]);
                    enemy->threads[k] = NULL;
                }
            }
            if (enemy->IsFollower())
            {
                enemy->leader->followerCount--;
            }
            enemy->ReleaseFollowers(1);
            if (!g_Player.focused)
            {
                k = -200;
            }
            else
            {
                k = 200;
            }
            g_GameManager.AddGauge(k, 0);
            switch (enemy->deathMode)
            {
            case 3:
                enemy->life = 1;
                enemy->contactDamage = 0;
                enemy->deathMode = 0;
                g_Gui.SetBossPresent(0);
                g_ReplayManager->frameEvents |= 0x20;
                if (enemy->deathEffect >= 0)
                {
                    g_EffectManager.SpawnEffect(enemy->deathEffect, &enemy->absPos, 1, 0xffffffff);
                    g_EffectManager.SpawnEffect(enemy->deathEffect, &enemy->absPos, 1, 0xffffffff);
                    g_EffectManager.SpawnEffect(enemy->deathEffect, &enemy->absPos, 1, 0xffffffff);
                }
                if (enemy->attachedEffect != NULL)
                {
                    enemy->attachedEffect->sprite.SetPendingInterrupt(3);
                    enemy->attachedEffect = NULL;
                }
                if (g_Player.playerState == 0)
                {
                    g_Player.stateTimer = 90;
                    g_Player.playerState = 3;
                }
                enemy->haltedByBomb = 0;
                enemy->intangibleDuringBomb = 0;
                break;
            case 1:
                g_GameManager.AddScore(enemy->score);
                enemy->unusedFlag3324_23 = 1;
                enemy->shootable = 0;
                enemy->contactDamage = 0;
                enemy->hitByShots = 0;
                goto dead;
            case 0:
                g_GameManager.AddScore(enemy->score);
                enemy->active = 0;
                if (enemy->attachedEffect != NULL)
                {
                    enemy->attachedEffect->sprite.SetPendingInterrupt(3);
                    enemy->attachedEffect = NULL;
                }
            dead:
                if (enemy->isBoss)
                {
                    g_Gui.SetBossPresent(0);
                    ReleaseEnemyEffects(enemy);
                }
            case 2:
                enemy->DropItems(bomb);
                if (enemy->isBoss && !g_SpellCard.IsActive())
                {
                    int bonus;

                    bonus = g_BulletManager.ClearAllWithBonus(8000, 1);
                    bonus = g_EnemyManager.ClearEnemiesForBonus(8000, bonus);
                    if (bonus != 0)
                    {
                        g_GameManager.AddScore(bonus);
                        g_Gui.ShowBonusPopup(bonus);
                    }
                }
                enemy->life = 0;
                g_ReplayManager->frameEvents |= 0x20;
                break;
            }
            if (!enemy->releasedFromLeader)
            {
                g_SoundPlayer.QueueSoundPanned(i % 2 + 2, enemy->absPos.x);
                if (enemy->deathEffect >= 0)
                {
                    g_EffectManager.SpawnEffect(enemy->deathEffect, &enemy->absPos, 1, 0xffffffff);
                    g_EffectManager.SpawnEffect(enemy->deathEffectIndex + 4, &enemy->absPos, 4, 0xffffffff);
                }
                if (g_GameManager.IsGaugeAtLow1() || g_GameManager.IsGaugeAtHigh1())
                {
                    g_ItemManager.SpawnItem(&enemy->absPos, 7, 1);
                }
            }
            if (enemy->deathSub >= 0)
            {
                int l;

                enemy->ResetRankSpeeds();
                enemy->stackDepth = 0;
                for (l = 0; l < 4; l++)
                {
                    enemy->lifeCallbackLife[l] = -1;
                }
                enemy->timerCallbackTime = -1;
                enemy->FreeThreads();
                enemy->shooter = g_EnemyManager.enemyTemplate.shooter;
                enemy->shootInterval = 0;
                g_EnemyScripts.StartSub(&enemy->ctx, enemy->deathSub);
                enemy->deathSub = -1;
            }
        }
    flash:
        if (!enemy->isYoukai)
        {
            if (enemy->hitFlashTimer)
            {
                enemy->hitFlashTimer--;
                enemy->sprite.useColor2 = 0;
            }
            else if (hit)
            {
                if (enemy->lifeCallbackProximity < 2)
                {
                    g_SoundPlayer.QueueSoundPanned(0x14, enemy->absPos.x);
                }
                else
                {
                    g_SoundPlayer.QueueSoundPanned(0x25, enemy->absPos.x);
                }
                enemy->sprite.color2Rgb[2] = 0xff;
                enemy->sprite.color2Rgb[1] = 0x60;
                enemy->sprite.color2Rgb[0] = 0x80;
                enemy->sprite.color2Alpha = enemy->sprite.alpha;
                enemy->sprite.useColor2 = 1;
                enemy->hitFlashTimer = 1;
            }
            else
            {
                enemy->sprite.useColor2 = 0;
            }
        }
        else
        {
            enemy->sprite.color2Rgb[2] = 0x20;
            enemy->sprite.color2Rgb[1] = 0x20;
            enemy->sprite.color2Rgb[0] = 0xc0;
            enemy->sprite.color2Alpha = enemy->sprite.alpha / 2;
            enemy->sprite.useColor2 = 1;
        }
        if (enemy->isBoss)
        {
            D3DXVECTOR3 markerPos;

            if (!g_Gui.IsMsgRunning() && enemy->bossIndex == 0)
            {
                g_Gui.SetBossHealth((float)enemy->life / enemy->maxLife);
            }
            if (enemy->isBoss < 4)
            {
                if (!enemy->hideFollowEffects)
                {
                    markerPos.x = enemy->absPos.x + 32.0f;
                }
                else
                {
                    markerPos.x = -999.0f;
                }
                markerPos.y = 472.0f;
                markerPos.z = 0.0f;
                g_AsciiText.SetNearbyLabelPos(enemy->bossIndex, &markerPos);
                if (enemy->lifeCallbackProximity == 0)
                {
                    g_AsciiText.SetNearbyLabelDimmed(enemy->bossIndex, enemy->sprite.useColor2 != 0);
                }
                else
                {
                    g_AsciiText.SetNearbyLabelDimmed(enemy->bossIndex, enemy->lifeCallbackProximity + 1);
                }
            }
        }
        MoveEnemyEffects(enemy);
        if (!g_GameManager.isFrozen)
        {
            enemy->callbackTimer++;
        }
        if (enemy->invulnTimer > 0)
        {
            enemy->invulnTimer--;
        }
        if (!enemy->hideFollowEffects && enemy->active)
        {
            enemy->nextInDrawList = mgr->drawListHeads[enemy->drawListIndex];
            mgr->drawListHeads[enemy->drawListIndex] = enemy;
        }
    }
    if (mgr->frameTimer % 200 == 0 && g_GameManager.ChecksumMismatch())
    {
        return 4;
    }
    mgr->frameTimer++;
    return 1;
}

void Enemy::Move()
{
    this->posDelta = this->pos - this->prevPos;
    this->prevPos = this->pos;
    if (!this->mirrored)
    {
        this->pos.x += g_App.gameSpeed * this->velocity.x;
    }
    else
    {
        this->pos.x -= g_App.gameSpeed * this->velocity.x;
    }
    this->pos.y += g_App.gameSpeed * this->velocity.y;
    this->pos.z += g_App.gameSpeed * this->velocity.z;
}

void MoveEnemyEffects(Enemy *enemy)
{
    Effect *eff;
    int i;

    for (i = 0; i < enemy->effectCount; i++)
    {
        eff = enemy->effects[i];
        if (eff == NULL)
        {
            continue;
        }
        eff->sprite.visible = !enemy->hideFollowEffects;
        eff->origin = enemy->pos;
        if (eff->radius < enemy->effectRadius)
        {
            eff->radius += 0.3f;
        }
        else
        {
            eff->radius = enemy->effectRadius;
        }
        eff->angle = AddNormalizeAngle(eff->angle, 0.031415928f);
    }
}

int EnemyManager::OnDraw(EnemyManager *mgr)
{
    return EnemyManager::DrawEnemies(mgr, 0, 2);
}

// Sprite::scaleY and scaleX, as DrawEnemies saves them around a trail.
// th07 used a D3DXVECTOR2, whose constructor th08 would call.
struct EnemySpriteScale
{
    float x;
    float y;
};

int EnemyManager::DrawEnemies(EnemyManager *mgr, int from, int to)
{
    float uSpan2;
    float cs2;
    float theta;
    float uDecr;
    VertexTex1DiffuseXyzrwh *out;
    float lastTheta;
    float tu;
    float sinA;
    int vertices;
    float width;
    float height;
    Enemy *enemy;
    int b;
    Sprite *sprite;
    D3DCOLOR savedColor;
    int i;
    EnemySpriteScale savedScale;

    for (i = from; i < to; i++)
    {
        enemy = mgr->drawListHeads[i];
        while (enemy != NULL)
        {
            for (sprite = enemy->sprites, b = 0; b < 1; b++, sprite++)
            {
                if (sprite->scriptIndex >= 0)
                {
                    if (sprite->autoRotate != 0)
                    {
                        sprite->SetRotationZ(enemy->angle);
                    }
                    if (!enemy->useMainSpriteOffset)
                    {
                        sprite->pos = enemy->absPos + sprite->posOffset;
                    }
                    else
                    {
                        sprite->pos = enemy->absPos + enemy->sprite.posOffset;
                    }
                    sprite->pos.z = 0.3f;
                    sprite->pos.x = g_GameManager.arena[0] + sprite->pos.x;
                    sprite->pos.y = g_GameManager.arena[1] + sprite->pos.y;
                    g_SpriteEngine->Draw(sprite);
                }
            }
            if (enemy->rotateSprite)
            {
                enemy->sprite.SetRotationZ(enemy->angle);
            }
            enemy->sprite.pos = enemy->absPos + enemy->sprite.posOffset;
            enemy->sprite.pos.x = g_GameManager.arena[0] + enemy->sprite.pos.x;
            enemy->sprite.pos.y = g_GameManager.arena[1] + enemy->sprite.pos.y;
            enemy->sprite.pos.z = 0.25f;
            if (enemy->trailFlags)
            {
                savedScale = *(EnemySpriteScale *)&enemy->sprite.scaleY;
                savedColor = enemy->sprite.color;
                if (!(enemy->trailFlags & 8))
                {
                    for (b = enemy->trailSegmentCount - 1; b > 0; b -= enemy->trailSegmentStep)
                    {
                        if (enemy->trailBoxes[b].pos.x < -990.0f)
                        {
                            continue;
                        }
                        if (enemy->rotateSprite)
                        {
                            enemy->sprite.SetRotationZ(enemy->trailBoxes[b].angle);
                        }
                        if (enemy->trailFlags & 2)
                        {
                            enemy->sprite.scaleY = savedScale.x - b * savedScale.x / enemy->trailSegmentCount;
                        }
                        if (enemy->trailFlags & 4)
                        {
                            enemy->sprite.alpha =
                                ((BYTE *)&savedColor)[3] - ((BYTE *)&savedColor)[3] * b / enemy->trailSegmentCount;
                        }
                        enemy->sprite.pos = enemy->trailBoxes[b].pos + enemy->sprite.posOffset;
                        enemy->sprite.pos.z = 0.3f;
                        enemy->sprite.pos.x = g_GameManager.arena[0] + enemy->sprite.pos.x;
                        enemy->sprite.pos.y = g_GameManager.arena[1] + enemy->sprite.pos.y;
                        g_SpriteEngine->Draw(&enemy->sprite);
                    }
                }
                else
                {
                    vertices = 0;
                    for (b = 0; b < enemy->trailSegmentCount; b += enemy->trailSegmentStep)
                    {
                        if (enemy->trailBoxes[b].pos.x < -990.0f)
                        {
                            break;
                        }
                        vertices += 2;
                    }
                    if (vertices > 2)
                    {
                        uSpan2 = enemy->sprite.spriteData->uvEndX - enemy->sprite.spriteData->uvStartX;
                        uDecr = uSpan2 / ((vertices + 1) / 2 - 1);
                        tu = enemy->sprite.spriteData->uvEndX + enemy->sprite.uvScrollX;
                        out = (VertexTex1DiffuseXyzrwh *)enemy->spriteStripVertices;
                        for (b = 0; b < enemy->trailSegmentCount; b += enemy->trailSegmentStep, tu -= uDecr)
                        {
                            if (enemy->trailBoxes[b].pos.x < -990.0f)
                            {
                                break;
                            }
                            if (b == 0)
                            {
                                theta = enemy->trailBoxes[0].angle;
                            }
                            else
                            {
                                theta = LerpAngle(enemy->trailBoxes[b - 1].angle, enemy->trailBoxes[b].angle, 0.5f);
                            }
                            if ((enemy->trailFlags & 2) && b > 0 && b + enemy->trailSegmentStep < enemy->trailSegmentCount)
                            {
                                sinA = LerpAngle(enemy->trailBoxes[b + enemy->trailSegmentStep - 1].angle,
                                                 enemy->trailBoxes[enemy->trailSegmentStep].angle, 0.5f);
                                if (fabsf(lastTheta - theta) < 0.00001f && fabsf(theta - sinA) < 0.00001f)
                                {
                                    vertices -= 2;
                                    continue;
                                }
                            }
                            lastTheta = theta;
                            sinA = sinf(theta);
                            cs2 = cosf(theta);
                            width = 0.0f;
                            height = savedScale.y * enemy->sprite.spriteData->height / 2.0f;
                            if (enemy->trailFlags & 2)
                            {
                                theta = 1.0f - (float)b / enemy->trailSegmentCount;
                                width *= theta;
                                height *= theta;
                            }
                            out[1].diffuse = enemy->sprite.color;
                            out[0].diffuse = out[1].diffuse;
                            if (enemy->trailFlags & 4)
                            {
                                ((BYTE *)&out[1].diffuse)[3] =
                                    ((BYTE *)&savedColor)[3] - ((BYTE *)&savedColor)[3] * b / enemy->trailSegmentCount;
                                ((BYTE *)&out[0].diffuse)[3] = ((BYTE *)&out[1].diffuse)[3];
                            }
                            out->pos = enemy->trailBoxes[b].pos;
                            out->pos.x = cs2 * width - sinA * height + 32.0f + out->pos.x;
                            out->pos.y = sinA * width + cs2 * height + 16.0f + out->pos.y;
                            out->uv.x = tu;
                            out->uv.y = enemy->sprite.spriteData->uvStartY + enemy->sprite.uvScrollY;
                            out++;
                            out->pos = enemy->trailBoxes[b].pos;
                            out->pos.x = cs2 * width + sinA * height + 32.0f + out->pos.x;
                            out->pos.y = sinA * width - cs2 * height + 16.0f + out->pos.y;
                            out->uv.x = tu;
                            out->uv.y = enemy->sprite.spriteData->uvEndY + enemy->sprite.uvScrollY;
                            out++;
                        }
                        if (vertices > 2)
                        {
                            g_SpriteEngine->DrawVertexStrip(&enemy->sprite,
                                                            (VertexTex1DiffuseXyzrwh *)enemy->spriteStripVertices, vertices);
                        }
                    }
                }
                *(EnemySpriteScale *)&enemy->sprite.scaleY = savedScale;
                enemy->sprite.color = savedColor;
            }
            if (!(enemy->trailFlags & 0x10) && !enemy->hiddenByBomb)
            {
                g_SpriteEngine->Draw(&enemy->sprite);
            }
            for (b = 1; b < 2; b++, sprite++)
            {
                if (sprite->scriptIndex >= 0)
                {
                    if (sprite->autoRotate != 0)
                    {
                        sprite->SetRotationZ(-enemy->angle);
                    }
                    if (!enemy->useMainSpriteOffset)
                    {
                        sprite->pos = enemy->absPos + sprite->posOffset;
                    }
                    else
                    {
                        sprite->pos = enemy->absPos + enemy->sprite.posOffset;
                    }
                    sprite->pos.z = 0.3f;
                    sprite->pos.x = g_GameManager.arena[0] + sprite->pos.x;
                    sprite->pos.y = g_GameManager.arena[1] + sprite->pos.y;
                    g_SpriteEngine->Draw(sprite);
                }
            }
            enemy = enemy->nextInDrawList;
        }
    }
    return 1;
}

float LerpAngle(float a, float b, float t)
{
    float diff;
    float wrap;

    if (a < b)
    {
        diff = b - a;
        wrap = a + 6.2831855f - b;
    }
    else
    {
        diff = a - b;
        wrap = b + 6.2831855f - a;
        a = b;
    }
    if (diff < wrap)
    {
        return diff * t + a;
    }
    else
    {
        return wrap * t + a;
    }
}

int EnemyManager::OnDraw2(EnemyManager *mgr)
{
    int result;

    if (g_GameManager.deathbombFreeze)
    {
        g_SpriteEngine->SetColorModulation(0xfff01010);
    }
    result = EnemyManager::DrawEnemies(mgr, 2, 4);
    if (g_GameManager.deathbombFreeze)
    {
        g_SpriteEngine->ResetColorModulation();
    }
    return result;
}

int EnemyManager::AddedCallback(EnemyManager *mgr)
{
    Enemy *unusedEnemy = mgr->enemies;
    EnemyScriptHeader *savedFile;
    void **subs;

    if (IsLoadingResources())
    {
        mgr->anm = g_SpriteEngine->PreloadAnm(7, "enemy.anm");
        if (mgr->anm == NULL)
        {
            return -1;
        }
    }
    else
    {
        mgr->anm = g_SpriteEngine->GetAnm(7);
    }
    if (!IsRestarting())
    {
        if (!g_GameManager.isInSpellCardPractice || g_GameManager.practiceSpellCard < 0xcd)
        {
            mgr->stageAnm = g_SpriteEngine->PreloadAnm(8, g_EnemyAnmPaths[g_GameManager.stage]);
            if (mgr->stageAnm == NULL)
            {
                return -1;
            }
        }
        else
        {
            mgr->stageAnm = g_SpriteEngine->PreloadAnm(8, g_EnemyAnmPaths[g_GameManager.practiceSpellCard - 0xc4]);
            if (mgr->stageAnm == NULL)
            {
                return -1;
            }
        }
    }
    else
    {
        mgr->stageAnm = g_SpriteEngine->GetAnm(8);
    }
    if (!IsRestarting())
    {
        memset(&g_EnemyScripts, 0, sizeof(EnemyScripts));
        if (!g_GameManager.isInSpellCardPractice)
        {
            if (g_EnemyScripts.Load(g_EclPaths[g_GameManager.stage]) != 0)
            {
                return -1;
            }
        }
        else if (g_GameManager.practiceSpellCard >= 0xcd)
        {
            if (g_EnemyScripts.Load(g_EclPaths[g_GameManager.practiceSpellCard - 0xba]) != 0)
            {
                return -1;
            }
        }
        else
        {
            if (g_EnemyScripts.Load(g_EclPaths[g_GameManager.stage + 9]) != 0)
            {
                return -1;
            }
        }
    }
    else
    {
        savedFile = g_EnemyScripts.file;
        subs = g_EnemyScripts.subs;
        memset(&g_EnemyScripts, 0, sizeof(EnemyScripts));
        g_EnemyScripts.file = savedFile;
        g_EnemyScripts.subs = subs;
    }
    mgr->dropCounter = g_Rng.GetRandomU16InRange(3);
    mgr->dropIndex = g_Rng.GetRandomU16InRange(8);
    D3DXVECTOR3 nowhere(-999.0f, -999.0f, -999.0f);
    g_AsciiText.SetNearbyLabelPos(0, &nowhere);
    g_AsciiText.SetNearbyLabelPos(1, &nowhere);
    g_AsciiText.SetNearbyLabelPos(2, &nowhere);
    g_AsciiText.SetNearbyLabelPos(3, &nowhere);
    return 0;
}

int EnemyManager::DeletedCallback(EnemyManager *mgr)
{
    int i;
    Enemy *enemy = mgr->enemies;

    for (i = 0; i < 0x1e0; i++, enemy++)
    {
        enemy->FreeThreads();
    }
    if (!IsRestarting())
    {
        g_SpriteEngine->ReleaseAnm(8);
    }
    if (IsFreeingResources())
    {
        g_SpriteEngine->ReleaseAnm(7);
    }
    if (!IsRestarting())
    {
        g_EnemyScripts.Release();
    }
    D3DXVECTOR3 nowhere(-999.0f, -999.0f, -999.0f);
    g_AsciiText.SetNearbyLabelPos(0, &nowhere);
    g_AsciiText.SetNearbyLabelPos(1, &nowhere);
    g_AsciiText.SetNearbyLabelPos(2, &nowhere);
    g_AsciiText.SetNearbyLabelPos(3, &nowhere);
    return 0;
}

void EnemyManager::RemoveChain()
{
    g_Chain.Remove(&g_EnemyManagerCalcChain);
    g_Chain.Remove(&g_EnemyManagerDrawChain);
    g_Chain.Remove(&g_EnemyManagerDrawChain2);
}

int EnemyManager::ClearEnemiesForBonus(int maxBonus, int start)
{
    Enemy *enemy = this->enemies;
    int total = start;
    int bonus = 2000;
    int k;
    int j;

    for (k = 0; k < 0x1e0; k++, enemy++)
    {
        if (!enemy->active)
        {
            continue;
        }
        if (enemy->isBoss)
        {
            continue;
        }
        if (enemy->undying)
        {
            continue;
        }
        enemy->life = 0;
        if (enemy->grazeable)
        {
            enemy->absPos = enemy->pos + enemy->posOrigin;
            g_ItemManager.SpawnItem(&enemy->absPos, 6, 1);
            g_AsciiText.AddPopup(&enemy->absPos, bonus, bonus >= maxBonus ? 0xffffff00 : 0xffffffff);
            total += bonus;
            bonus += 30;
            if (bonus > maxBonus)
            {
                bonus = maxBonus;
            }
            if (enemy->trailFlags)
            {
                for (j = 0; j < enemy->trailSegmentCount; j += 6)
                {
                    g_ItemManager.SpawnItem(&enemy->trailBoxes[j].pos, 6, 1);
                    g_AsciiText.AddPopup(&enemy->trailBoxes[j].pos, bonus,
                                         bonus >= maxBonus ? 0xffffff00 : 0xffffffff);
                    total += bonus;
                    bonus += 30;
                    if (bonus > maxBonus)
                    {
                        bonus = maxBonus;
                    }
                }
            }
        }
        enemy->LeaveFollowerList();
        if (enemy->deathSub >= 0)
        {
            g_EnemyScripts.StartSub(&enemy->ctx, enemy->deathSub);
            enemy->deathSub = -1;
        }
    }
    return total;
}

int EnemyManager::AnyBossActive()
{
    int i;

    for (i = 0; i < 8; i++)
    {
        if (this->bosses[i] != NULL)
        {
            return 1;
        }
    }
    return 0;
}
