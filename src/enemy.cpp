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
    for (i = 0; i < ARRAY_SIZE(this->unk_9dcefc); i++)
    {
        this->unk_9dcefc[i] = -1;
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
    enemy->unk_3324_6 = 1;
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
    enemy->unk_3350 = 1024.0f;
    enemy->unk_2e10 = g_Unknown18b8a24;
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
        enemy->unk_2e04 = enemy->maxLife;
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
        enemy->unk_2e04 = enemy->maxLife;
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
                if (!g_Gui.IsBossPresent() && g_EnemyManager.unk_9dcf0c == 0)
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
                if (!g_Gui.IsBossPresent() && g_EnemyManager.unk_9dcf0c == 0)
                {
                    EnemyTimelineSpawnWithArgs *args = (EnemyTimelineSpawnWithArgs *)this->pos->args;
                    D3DXVECTOR3 pos;
                    Enemy *enemy;

                    pos.x = args->x;
                    pos.y = args->y;
                    pos.z = 0.0f;
                    enemy = g_EnemyManager.Spawn(args->sub, &pos, args->life, -1, args->score, mirror);
                    enemy->unk_3308 = args->unk_10;
                    enemy->unk_330c = args->unk_14;
                }
                break;
            case 4:
                mirror = 1;
            case 2:
                if (!g_Gui.IsBossPresent() && g_EnemyManager.unk_9dcf0c == 0)
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
                if (!g_Gui.IsBossPresent() && g_EnemyManager.unk_9dcf0c == 0)
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
                for (j = 0; j < ARRAY_SIZE(g_EnemyManager.unk_9dcefc); j++)
                {
                    if (g_EnemyManager.unk_9dcefc[j] == this->pos->args[0])
                    {
                        count++;
                        g_EnemyManager.unk_9dcefc[j] = -1;
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

                for (k = 0; k < ARRAY_SIZE(g_EnemyManager.unk_9dcefc); k++)
                {
                    if (g_EnemyManager.unk_9dcefc[k] >= 0)
                    {
                        continue;
                    }
                    g_EnemyManager.unk_9dcefc[k] = this->pos->args[0];
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

void Enemy::Func42b2f0()
{
    if (this->Func41fd20())
    {
        this->unk_4->unk_8 = this->unk_8;
        if (this->unk_8 != NULL)
        {
            this->unk_8->unk_4 = this->unk_4;
        }
        this->unk_2da4 = 0;
        this->unk_8 = NULL;
        this->unk_4 = NULL;
    }
    else
    {
        this->unk_4 = NULL;
        this->unk_8 = NULL;
    }
}

void Enemy::Func42b370(int damage)
{
    int share;
    int i;
    int threshold;

    if (!this->Func41fd20())
    {
        return;
    }
    if (g_Unknown17d6ed4 != 0)
    {
        return;
    }
    threshold = 0;
    for (i = 0; i < 4; i++)
    {
        if (threshold < this->unk_2da4->lifeCallbackLife[i])
        {
            threshold = this->unk_2da4->lifeCallbackLife[i];
        }
    }
    share = damage / 2;
    if (this->unk_2da4->invulnTimer > 0)
    {
        if (this->unk_2da4->isBoss)
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
    this->unk_2da4->life -= share;
    if (this->unk_2da4->life <= threshold)
    {
        this->unk_2da4->life = threshold;
    }
}

void Enemy::Func42bc90()
{
    int i;

    for (i = 0; i < 4; i++)
    {
        if (this->unk_3384[i] != NULL)
        {
            g_MemoryManager.Free(this->unk_3384[i]);
            this->unk_3384[i] = NULL;
        }
    }
}

void Enemy::Despawn()
{
    int i;

    this->Func42adb0(0);
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
    if (this->unk_53c8 != NULL)
    {
        this->unk_53c8->sprite.SetPendingInterrupt(3);
        this->unk_53c8 = NULL;
    }
    for (i = 0; i < 4; i++)
    {
        this->lifeCallbackLife[i] = -1;
    }
    this->timerCallbackTime = -1;
    this->Func42bc90();
    if (g_Unknown18b89b4 == this)
    {
        g_Unknown18b89b4 = NULL;
    }
}

void Enemy::DropItems(int byBomb)
{
    D3DXVECTOR3 pos;
    int i;

    if (this->item >= 0)
    {
        g_EffectManager.SpawnEffect(this->deathEffectIndex + 4, &this->unk_2d88, 3, 0xffffffff);
        g_ItemManager.SpawnItem(&this->unk_2d88, this->item, byBomb != 0);
    }
    else if (this->item == -1)
    {
        if (g_EnemyManager.dropCounter % 3 == 0)
        {
            g_EffectManager.SpawnEffect(this->deathEffectIndex + 4, &this->unk_2d88, 6, 0xffffffff);
            g_ItemManager.SpawnItem(&this->unk_2d88, g_EnemyItemDrops[g_EnemyManager.dropIndex], byBomb != 0);
            g_EnemyManager.dropIndex++;
            if (g_EnemyManager.dropIndex >= 0x20)
            {
                g_EnemyManager.dropIndex = 0;
            }
        }
        g_EnemyManager.dropCounter++;
    }
    if (this->unk_330c != 0)
    {
        for (i = 0; i < this->unk_330c; i++)
        {
            pos = this->unk_2d88;
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
        this->unk_330c = 0;
    }
    if (this->unk_3308 != 0)
    {
        for (i = 0; i < this->unk_3308; i++)
        {
            pos = this->unk_2d88;
            pos[0] += g_Rng.GetRandomF32ZeroToOne() * 128.0f - 64.0f;
            pos[1] += g_Rng.GetRandomF32ZeroToOne() * 128.0f - 64.0f;
            g_ItemManager.SpawnItem(&pos, 1, 0);
        }
        this->unk_3308 = 0;
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
    if ((g_GameManager.character == 0 || g_GameManager.character == 4) && this->Func41fd20())
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

    if (g_GameManager.unk_3dbac_10)
    {
        g_SpriteEngine->SetColorModulation(0xfff01010);
    }
    result = EnemyManager::DrawEnemies(mgr, 2, 4);
    if (g_GameManager.unk_3dbac_10)
    {
        g_SpriteEngine->ResetColorModulation();
    }
    return result;
}

int EnemyManager::AddedCallback(EnemyManager *mgr)
{
    Enemy *unusedEnemy = mgr->enemies;
    void *savedFile;
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
        if (!g_GameManager.isInSpellCardPractice || g_GameManager.unk_3dbb0 < 0xcd)
        {
            mgr->stageAnm = g_SpriteEngine->PreloadAnm(8, g_EnemyAnmPaths[g_GameManager.stage]);
            if (mgr->stageAnm == NULL)
            {
                return -1;
            }
        }
        else
        {
            mgr->stageAnm = g_SpriteEngine->PreloadAnm(8, g_EnemyAnmPaths[g_GameManager.unk_3dbb0 - 0xc4]);
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
        else if (g_GameManager.unk_3dbb0 >= 0xcd)
        {
            if (g_EnemyScripts.Load(g_EclPaths[g_GameManager.unk_3dbb0 - 0xba]) != 0)
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
        enemy->Func42bc90();
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
        if (enemy->unk_3328_6)
        {
            continue;
        }
        enemy->life = 0;
        if (enemy->grazeable)
        {
            enemy->unk_2d88 = enemy->pos + enemy->unk_2d40;
            g_ItemManager.SpawnItem(&enemy->unk_2d88, 6, 1);
            g_AsciiText.AddPopup(&enemy->unk_2d88, bonus, bonus >= maxBonus ? 0xffffff00 : 0xffffffff);
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
        enemy->Func42b2f0();
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
