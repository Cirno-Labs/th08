// The enemies, from 0x429e00, as th07's enemy.cpp.

#include <string.h>

#include "effect.h"
#include "enemy.h"
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
