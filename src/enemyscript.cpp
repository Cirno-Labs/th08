// The enemy scripts (ECL), from 0x418330, as th07's enemyscript.cpp.

#include <math.h>
#include <string.h>

#include "ascii.h"
#include "bullet.h"
#include "effect.h"
#include "enemy.h"
#include "item.h"
#include "player.h"
#include "screeneffect.h"
#include "unsorted.h"

// The game's data, as the original defines it here. The DLL build uses
// the original's copies instead.
#ifdef TH08_EXE
EnemyScriptInterp g_EnemyScriptInterps[8] = {
    ScriptMoveLinear,
    ScriptMoveLinear,
    ScriptMoveLinear,
    ScriptMoveLinear,
    ScriptMoveLinear,
    ScriptMoveLinear,
    ScriptMoveLinear,
    ScriptMoveHermite,
};
EnemyScriptFunc g_EnemyScriptFuncs[32] = {
    SetScriptFunc0Vars,
    FlashScreen60,
    BounceOffSides,
    SpawnTwinEffects38,
    BounceBulletsAtZones,
    EndTwinEffects,
    SpawnTwinEffects3A,
    BounceBulletsAtZones3,
    KeepFormation,
    BeamHitsPlayer160,
    FlashAndShakeScreen,
    BeamHitsPlayer240,
    ToggleMarkedBullets,
    TintStageRed,
    CycleMarkedBullets,
    ShakeScreenRamp,
    MarkFollowersNearBullets,
    FlashScreen180,
    SetGameSpeed,
    GetPracticeCard,
    SpawnTwinEffects41,
    BounceBulletsAtZones2,
    StartResurrection,
    ScriptEndSpellCardName,
    GetSpellCardsCaptured,
    BeamHitsPlayer288,
    SetFrozen,
    SpawnEnemiesAtMarkedBullets,
    StartSlowdown,
    EndSlowdown,
    SetScreenEffectHoldFrames,
    DropBombOrLifeItem,
};
// Constructed, but nothing uses it.
ChainElem g_UnusedEclChain;
EnemyScripts g_EnemyScripts;
#endif // TH08_EXE

// Argument `n` of `instr`, or the value of the variable it names, as
// th07's.
#define INT_ARG(n) \
    ((instr->paramMask & (1 << (n))) ? GetIntOperand(enemy, instr->args[n].i) : instr->args[n].i)
#define FLOAT_ARG(n) \
    ((instr->paramMask & (1 << (n))) ? GetFloatOperand(enemy, instr->args[n].f) : instr->args[n].f)
// INT_ARG and FLOAT_ARG for a value `v` whose variable bit is `bit`, as
// th07's. `v` must stay unparenthesized.
#define INT_OP(v, bit) ((instr->paramMask & (bit)) ? GetIntOperand(enemy, v) : v)
#define FLOAT_OP(v, bit) ((instr->paramMask & (bit)) ? GetFloatOperand(enemy, v) : v)

EXE_INLINE EnemyScripts::EnemyScripts()
{
}

EXE_INLINE EnemyScriptsUnusedBlock::EnemyScriptsUnusedBlock()
{
}

int EnemyScripts::Load(char *path)
{
    int i;

    this->file = (EnemyScriptHeader *)LoadFileData(path, NULL, 0);
    if (this->file == NULL)
    {
        LogPrintf(&g_Log, "敵データの読み込みに失敗しました、データが壊れてるか失われています\r\n");
        return -1;
    }
    if (this->file->version != 0x800)
    {
        LogPrintf(&g_Log, "敵データのバージョンが違います\r\n");
        return -1;
    }
    for (i = 0; i < 16; i++)
    {
        this->file->timelines[i] = (char *)this->file->timelines[i] + (int)this->file;
    }
    this->subs = (void **)((char *)this->file + sizeof(EnemyScriptHeader));
    for (i = 0; i < this->file->subCount; i++)
    {
        this->subs[i] = (char *)this->subs[i] + (int)this->file;
    }
    return 0;
}

void EnemyScripts::Release()
{
    if (this->file != NULL)
    {
        g_MemoryManager.Free(this->file);
    }
    this->file = NULL;
}

int EnemyScripts::StartSub(EnemyScriptState *ctx, short sub)
{
    if (sub < 0)
    {
        return 0;
    }
    ctx->code = this->subs[sub];
    ctx->timer = 0;
    ctx->waitTimer = 0;
    ctx->sub = sub;
    return 0;
}

// The variable argument `n` of `instr` names, through ResolveOperand, as
// th07's.
#define INT_VAR(n) (*ResolveOperand(enemy, &instr->args[n].i, instr->paramMask, (n)))
#define FLOAT_VAR(n) (*ResolveFloatOperand(enemy, &instr->args[n].f, instr->paramMask, (n)))

inline void SpellCard::SetEffectPos(float x, float y, float z)
{
    this->effect->pos.x = x;
    this->effect->pos.y = y;
    this->effect->pos.z = z;
}

int EnemyScripts::RunEnemy(Enemy *enemy)
{
    int value;
    int ctxIndex;
    EnemyScriptInstr *instr;

    ctxIndex = -1;
    enemy->callStack = enemy->savedCtx;
    enemy->currentCtx = &enemy->ctx;
    enemy->stackDepth = enemy->savedStackDepth;
restart:
    instr = (EnemyScriptInstr *)enemy->currentCtx->code;
    if (enemy->pendingInterrupt >= 0)
    {
        goto interrupt;
    }
loop:
    enemy->absPos = enemy->pos + enemy->posOrigin;
    if (enemy->currentCtx->waitTimer.AsFrames() > 0)
    {
        enemy->currentCtx->waitTimer--;
        enemy->currentCtx->timer--;
        goto done;
    }
    if (enemy->currentCtx->timer == instr->time)
    {
        if ((instr->difficultyMask & (g_GameManager.difficultyMask | enemy->difficultyMaskExtra)) !=
            (g_GameManager.difficultyMask | enemy->difficultyMaskExtra))
        {
            goto next;
        }
        switch (instr->opcode)
        {
        case 1:
            return -1;
        case 2:
            enemy->currentCtx->waitTimer = INT_ARG(0);
            break;
        case 5:
            (*ResolveOperand(enemy, &instr->args[2].i, instr->paramMask, 2))--;
            if (INT_ARG(2) <= 0)
            {
                break;
            }
        case 4:
            enemy->currentCtx->timer.current = instr->args[0].i;
            instr = (EnemyScriptInstr *)((char *)instr + instr->args[1].i);
            goto loop;
        case 6:
            INT_VAR(0) = INT_ARG(1);
            break;
        case 7:
            FLOAT_VAR(0) = FLOAT_ARG(1);
            break;
        case 8:
            INT_VAR(0) = (g_Rng.GetRandomU16() & 1 ? 1 : -1) * INT_ARG(1);
            break;
        case 9:
            FLOAT_VAR(0) = (g_Rng.GetRandomU16() & 1 ? 1.0f : -1.0f) * FLOAT_ARG(1);
            break;
        case 10:
            INT_VAR(0) += INT_ARG(1);
            break;
        case 15:
            FLOAT_VAR(0) += FLOAT_ARG(1);
            break;
        case 11:
            INT_VAR(0) -= INT_ARG(1);
            break;
        case 16:
            FLOAT_VAR(0) -= FLOAT_ARG(1);
            break;
        case 12:
            INT_VAR(0) *= INT_ARG(1);
            break;
        case 17:
            FLOAT_VAR(0) *= FLOAT_ARG(1);
            break;
        case 13:
            INT_VAR(0) /= INT_ARG(1);
            break;
        case 18:
            FLOAT_VAR(0) /= FLOAT_ARG(1);
            break;
        case 14:
            INT_VAR(0) %= INT_ARG(1);
            break;
        case 19:
            FLOAT_VAR(0) = fmodf(FLOAT_ARG(0), FLOAT_ARG(1));
            break;
        case 20:
            INT_VAR(0) = INT_ARG(1) + INT_ARG(2);
            break;
        case 25:
            FLOAT_VAR(0) = FLOAT_ARG(1) + FLOAT_ARG(2);
            break;
        case 21:
            INT_VAR(0) = INT_ARG(1) - INT_ARG(2);
            break;
        case 26:
            FLOAT_VAR(0) = FLOAT_ARG(1) - FLOAT_ARG(2);
            break;
        case 22:
            INT_VAR(0) = INT_ARG(1) * INT_ARG(2);
            break;
        case 27:
            FLOAT_VAR(0) = FLOAT_ARG(1) * FLOAT_ARG(2);
            break;
        case 23:
            INT_VAR(0) = INT_ARG(1) / INT_ARG(2);
            break;
        case 28:
            FLOAT_VAR(0) = FLOAT_ARG(1) / FLOAT_ARG(2);
            break;
        case 24:
            INT_VAR(0) = INT_ARG(1) % INT_ARG(2);
            break;
        case 29:
            FLOAT_VAR(0) = fmodf(FLOAT_ARG(1), FLOAT_ARG(2));
            break;
        case 30:
            INT_VAR(0)++;
            break;
        case 31:
            INT_VAR(0)--;
            break;
        case 32:
            FLOAT_VAR(0) = sinf(FLOAT_ARG(1));
            break;
        case 33:
            FLOAT_VAR(0) = cosf(FLOAT_ARG(1));
            break;
        case 34:
            FLOAT_VAR(0) = atan2f(FLOAT_ARG(4) - FLOAT_ARG(2), FLOAT_ARG(3) - FLOAT_ARG(1));
            break;
        case 37:
            FLOAT_VAR(0) = AddNormalizeAngle(FLOAT_ARG(0), 0.0f);
            break;
        case 35:
            ScriptLerp(enemy, instr);
            break;
        case 36:
            ScriptStartInterp(enemy, instr);
            break;
        case 38:
        {
            float radius;
            float angle;

            angle = AddNormalizeAngle(FLOAT_ARG(2), 0.0f);
            radius = FLOAT_ARG(3);
            FLOAT_VAR(0) = cosf(angle) * radius;
            FLOAT_VAR(1) = sinf(angle) * radius;
            break;
        }
        case 39:
        {
            float y;
            float x;

            x = FLOAT_ARG(1) - FLOAT_ARG(3);
            y = FLOAT_ARG(2) - FLOAT_ARG(4);
            FLOAT_VAR(0) = sqrtf(x * x + y * y);
            break;
        }
        case 40:
        case 41:
        case 42:
        case 43:
        case 44:
        case 45:
        case 46:
        case 47:
        case 48:
        case 49:
        case 50:
        case 51:
        {
            EnemyScriptInstr *target;

            target = ScriptJumpIf(enemy, instr);
            if (target != NULL)
            {
                instr = target;
                goto loop;
            }
            break;
        }
        case 52:
            ScriptCallSub(enemy, instr, instr->args[0].i);
            goto restart;
        case 53:
            if (!ScriptReturn(enemy, instr))
            {
                goto restart;
            }
            goto nextThread;
        case 54:
            g_EnemyManager.anm->StartScript(&enemy->sprite, INT_ARG(0));
            enemy->useStageAnm = 0;
            break;
        case 55:
            value = INT_ARG(0);
            ScriptSetTurnSprites(enemy, instr, value, value + 1, value + 2, value + 3, value + 4, value + 5);
            enemy->useStageAnm = 0;
            break;
        case 56:
            ScriptSetTurnSprites(enemy, instr, INT_ARG(0), INT_ARG(1), INT_ARG(2), INT_ARG(3), INT_ARG(4),
                                 INT_ARG(5));
            enemy->useStageAnm = 0;
            break;
        case 57:
            ScriptSetSubSprite(enemy, instr);
            enemy->useStageAnm = 0;
            break;
        case 58:
            g_EnemyManager.stageAnm->StartScript(&enemy->sprite, INT_ARG(0));
            enemy->useStageAnm = 1;
            break;
        case 59:
            value = INT_ARG(0);
            ScriptSetTurnSprites(enemy, instr, value, value + 1, value + 2, value + 3, value + 4, value + 5);
            enemy->useStageAnm = 1;
            break;
        case 60:
            ScriptSetTurnSprites(enemy, instr, INT_ARG(0), INT_ARG(1), INT_ARG(2), INT_ARG(3), INT_ARG(4),
                                 INT_ARG(5));
            enemy->useStageAnm = 1;
            break;
        case 61:
            enemy->useStageAnm = 1;
            ScriptSetSubSprite(enemy, instr);
            break;
        case 62:
            if (!enemy->useStageAnm)
            {
                g_EnemyManager.anm->StartScript(&enemy->sprite, enemy->turnExtra);
            }
            else
            {
                g_EnemyManager.stageAnm->StartScript(&enemy->sprite, enemy->turnExtra);
            }
            break;
        case 63:
            enemy->pos.x = FLOAT_ARG(0);
            enemy->pos.y = FLOAT_ARG(1);
            enemy->pos.z = 0.0f;
            enemy->ClampPos();
            break;
        case 64:
            ScriptMovePos(enemy, instr);
            break;
        case 65:
            enemy->angle = AddNormalizeAngle(FLOAT_ARG(0), 0.0f);
            enemy->speed = FLOAT_ARG(1);
            enemy->moveMode = 1;
            enemy->moveTimer = enemy->moveDuration = 0;
            break;
        case 66:
            if (INT_ARG(0) <= 0)
            {
                enemy->angle = AddNormalizeAngle(FLOAT_ARG(2), 0.0f);
                enemy->speed = FLOAT_ARG(3);
                enemy->moveMode = 1;
                enemy->moveTimer = enemy->moveDuration = 0;
            }
            else
            {
                ScriptMoveAngle(enemy, instr);
            }
            break;
        case 67:
            ScriptMoveRandom(enemy, instr);
            break;
        case 178:
            ScriptMoveRandom2(enemy, instr);
            break;
        case 68:
            enemy->angle = AddNormalizeAngle(FLOAT_ARG(0), g_Player.AngleFrom(&enemy->pos));
            enemy->speed = FLOAT_ARG(1);
            break;
        case 69:
            if (INT_ARG(0) <= 0)
            {
                enemy->angle = AddNormalizeAngle(FLOAT_ARG(2), g_Player.AngleFrom(&enemy->pos));
                enemy->speed = FLOAT_ARG(3);
                enemy->moveMode = 1;
                enemy->moveTimer = enemy->moveDuration = INT_ARG(0);
            }
            else
            {
                ScriptMoveAngle(enemy, instr);
            }
            break;
        case 70:
            enemy->angularVelocity = FLOAT_ARG(0);
            enemy->moveMode = 1;
            break;
        case 71:
            enemy->acceleration = FLOAT_ARG(0);
            enemy->moveMode = 1;
            break;
        case 72:
            enemy->moveTimer = enemy->moveDuration = INT_ARG(0);
            enemy->moveStart.x = FLOAT_ARG(1);
            enemy->moveStart.y = FLOAT_ARG(2);
            enemy->orbitAngle = FLOAT_ARG(3);
            enemy->orbitAngularVelocity = FLOAT_ARG(4);
            enemy->orbitRadius = FLOAT_ARG(5);
            enemy->orbitRadiusVelocity = FLOAT_ARG(6);
            enemy->moveMode = 3;
            break;
        case 73:
            enemy->moveTimer = enemy->moveDuration = INT_ARG(0);
            enemy->moveStart = enemy->pos;
            enemy->orbitAngle = FLOAT_ARG(1);
            enemy->orbitAngularVelocity = FLOAT_ARG(2);
            enemy->orbitRadius = 0.0f;
            enemy->orbitRadiusVelocity = FLOAT_ARG(3);
            enemy->moveMode = 3;
            break;
        case 74:
            enemy->moveTimer = enemy->moveDuration = INT_ARG(0);
            enemy->orbitAngularVelocity = FLOAT_ARG(1);
            enemy->orbitRadiusVelocity = FLOAT_ARG(2);
            enemy->moveMode = 3;
            break;
        case 75:
            enemy->lowerBound[0] = FLOAT_ARG(0);
            enemy->lowerBound[1] = FLOAT_ARG(1);
            enemy->upperBound[0] = FLOAT_ARG(2);
            enemy->upperBound[1] = FLOAT_ARG(3);
            enemy->clampPos = 1;
            break;
        case 76:
            enemy->clampPos = 0;
            break;
        case 77:
            enemy->hitboxSize.x = FLOAT_ARG(0);
            enemy->hitboxSize.y = FLOAT_ARG(1);
            break;
        case 78:
            enemy->extraHitboxSize.x = FLOAT_ARG(0);
            enemy->extraHitboxSize.y = FLOAT_ARG(1);
            break;
        case 79:
            value = INT_ARG(0);
            enemy->hitByShots = !(value & 1);
            enemy->shootable = !(value & 2);
            enemy->contactDamage = !(value & 4);
            enemy->hideFollowEffects = (value & 8) != 0;
            enemy->staysOffscreen = (value & 0x10) != 0;
            enemy->undying = (value & 0x20) != 0;
            break;
        case 80:
            value = INT_ARG(0);
            if (value & 1)
            {
                enemy->hitByShots = 0;
            }
            if (value & 2)
            {
                enemy->shootable = 0;
                if (enemy->attachedEffect != NULL)
                {
                    enemy->attachedEffect->sprite.useColor2 = 0;
                }
            }
            if (value & 4)
            {
                enemy->contactDamage = 0;
            }
            if (value & 8)
            {
                enemy->hideFollowEffects = 1;
            }
            if (value & 0x10)
            {
                enemy->staysOffscreen = 1;
            }
            if (value & 0x20)
            {
                enemy->undying = 1;
            }
            break;
        case 81:
            value = INT_ARG(0);
            if (value & 1)
            {
                enemy->hitByShots = 1;
            }
            if (value & 2)
            {
                enemy->shootable = 1;
                if (enemy->attachedEffect != NULL)
                {
                    enemy->attachedEffect->sprite.useColor2 = 1;
                }
            }
            if (value & 4)
            {
                enemy->contactDamage = 1;
            }
            if (value & 8)
            {
                enemy->hideFollowEffects = 0;
            }
            if (value & 0x10)
            {
                enemy->staysOffscreen = 0;
            }
            if (value & 0x20)
            {
                enemy->undying = 0;
            }
            break;
        case 86:
            INT_VAR(0) = (instr->paramMask & 2) ? GetIntOperand(g_EnemyManager.bosses[INT_ARG(2)], instr->args[1].i)
                                                : instr->args[1].i;
            break;
        case 87:
            if (g_EnemyManager.bosses[INT_ARG(2)] != NULL)
            {
                FLOAT_VAR(0) = (instr->paramMask & 2)
                                   ? GetFloatOperand(g_EnemyManager.bosses[INT_ARG(2)], instr->args[1].f)
                                   : instr->args[1].f;
            }
            break;
        case 88:
            // Calls subroutine argument 1 in boss argument 0's script.
            value = INT_ARG(0);
            ScriptCallSub(g_EnemyManager.bosses[value],
                          (EnemyScriptInstr *)g_EnemyManager.bosses[value]->currentCtx->code, instr->args[1].i);
            break;
        case 89:
            if (g_EnemyManager.bosses[INT_ARG(0)] != NULL)
            {
                g_EnemyManager.bosses[INT_ARG(0)]->pendingInterrupt = INT_ARG(1);
            }
            break;
        case 90:
        {
            Enemy *child;
            Enemy *tail;

            tail = enemy->GetLastFollower();
            child = ScriptSpawnEnemy(enemy, instr);
            if (!g_EnemyManager.spawnFailed)
            {
                child->followsYoukaiState = 1;
                child->isYoukai = g_Player.IsYoukai();
                child->drawListIndex = g_Player.IsYoukai() ? 0 : 2;
                child->shootable = 0;
                if (child->attachedEffect == NULL)
                {
                    child->attachedEffect = g_EffectManager.SpawnEffect2(0x20, &child->pos, 1, 0xffffffff);
                    child->attachedEffect->sprite.SetPendingInterrupt(g_Player.IsYoukai() ? 2 : 1);
                    child->attachedEffect->sprite.useColor2 = child->shootable != 0;
                    if (child->index & 1)
                    {
                        child->attachedEffect->sprite.rotationSpeed.z = -child->attachedEffect->sprite.rotationSpeed.z;
                    }
                }
                child->leader = enemy;
                tail->nextFollower = child;
                child->prevFollower = tail;
                enemy->followerCount++;
            }
            g_SoundPlayer.QueueSoundPanned(0x24, enemy->pos.x);
            break;
        }
        case 91:
        {
            Enemy *child;
            Enemy *tail;

            tail = enemy->GetLastFollower();
            child = ScriptSpawnEnemyRel(enemy, instr);
            if (!g_EnemyManager.spawnFailed)
            {
                child->followsYoukaiState = 1;
                child->isYoukai = g_Player.IsYoukai();
                child->drawListIndex = g_Player.IsYoukai() ? 0 : 2;
                child->shootable = 0;
                if (child->attachedEffect == NULL)
                {
                    child->attachedEffect = g_EffectManager.SpawnEffect2(0x20, &child->pos, 1, 0xffffffff);
                    child->attachedEffect->sprite.SetPendingInterrupt(g_Player.IsYoukai() ? 2 : 1);
                    child->attachedEffect->sprite.useColor2 = child->shootable != 0;
                    if (child->index & 1)
                    {
                        child->attachedEffect->sprite.rotationSpeed.z = -child->attachedEffect->sprite.rotationSpeed.z;
                    }
                }
                child->leader = enemy;
                tail->nextFollower = child;
                child->prevFollower = tail;
                enemy->followerCount++;
            }
            g_SoundPlayer.QueueSoundPanned(0x24, enemy->pos.x);
            break;
        }
        case 92:
        {
            Enemy *child;
            Enemy *tail;

            tail = enemy->GetLastFollower();
            child = ScriptSpawnEnemy(enemy, instr);
            if (!g_EnemyManager.spawnFailed)
            {
                child->followsYoukaiState = 1;
                child->isYoukai = g_Player.IsYoukai();
                child->drawListIndex = g_Player.IsYoukai() ? 0 : 2;
                child->posOrigin = enemy->pos;
                child->absPos = child->posOrigin + child->pos;
                child->shootable = 0;
                if (child->attachedEffect == NULL)
                {
                    child->attachedEffect = g_EffectManager.SpawnEffect2(0x20, &child->absPos, 1, 0xffffffff);
                    child->attachedEffect->sprite.SetPendingInterrupt(g_Player.IsYoukai() ? 2 : 1);
                    child->attachedEffect->sprite.useColor2 = child->shootable != 0;
                    if (child->index & 1)
                    {
                        child->attachedEffect->sprite.rotationSpeed.z = -child->attachedEffect->sprite.rotationSpeed.z;
                    }
                }
                child->posRelativeToLeader = 1;
                child->leader = enemy;
                tail->nextFollower = child;
                child->prevFollower = tail;
                enemy->followerCount++;
            }
            g_SoundPlayer.QueueSoundPanned(0x24, enemy->pos.x);
            break;
        }
        case 96:
        case 97:
        case 98:
        case 99:
        case 100:
        case 101:
        case 102:
        case 103:
        case 104:
            if (enemy->life <= 0)
            {
                break;
            }
            // While holdFire is set, the instruction is kept for the
            // periodic shots instead.
            if (enemy->holdFire == 1)
            {
                memcpy(enemy->shootInstr, instr, sizeof(enemy->shootInstr));
                break;
            }
            ScriptFireBullets(enemy, instr);
            break;
        case 111:
        {
            BulletCommand *cmd;

            cmd = &enemy->shooter.commands[INT_ARG(0)];
            cmd->flag = INT_ARG(1);
            cmd->concurrentFlag = INT_ARG(2);
            cmd->frames = INT_ARG(3);
            cmd->maxTurns = INT_ARG(4);
            cmd->arg0 = FLOAT_ARG(5);
            cmd->arg1 = FLOAT_ARG(6);
            break;
        }
        case 138:
            enemy->deathEffect = *(char *)instr->args;
            enemy->deathEffectIndex = ((BYTE *)instr->args)[1];
            enemy->deathEffectVariant = ((BYTE *)instr->args)[2];
            break;
        case 105:
            enemy->shootInterval = INT_ARG(0);
            if (enemy->shootInterval != 0)
            {
                enemy->shootInterval +=
                    g_GameManager.RankLerp(enemy->shootInterval / 5, -enemy->shootInterval / 5);
                enemy->shootTimer = 0;
            }
            break;
        case 106:
            enemy->shootInterval = INT_ARG(0);
            if (enemy->shootInterval != 0)
            {
                enemy->shootInterval +=
                    g_GameManager.RankLerp(enemy->shootInterval / 5, -enemy->shootInterval / 5);
                enemy->shootTimer = g_Rng.GetRandomU32InRange(enemy->shootInterval);
            }
            break;
        case 107:
            enemy->holdFire = 1;
            break;
        case 108:
            enemy->holdFire = 0;
            break;
        case 109:
            enemy->shooter.pos = enemy->pos + enemy->shootOffset;
            g_BulletManager.FirePattern(&enemy->shooter);
            break;
        case 110:
            enemy->shootOffset.x = FLOAT_ARG(0);
            enemy->shootOffset.y = FLOAT_ARG(1);
            enemy->shootOffset.z = 0.0f;
            break;
        case 114:
        case 115:
        {
            EnemyScriptArg *laserArgs;
            BulletShooter *shooter;

            laserArgs = instr->args;
            shooter = &enemy->laserShooter;
            shooter->pos = enemy->absPos + enemy->shootOffset;
            shooter->sprite = *(short *)&laserArgs[0];
            shooter->color = INT_OP(((short *)laserArgs)[1], 2);
            shooter->angle1 = FLOAT_OP(laserArgs[1].f, 4);
            shooter->speed1 = FLOAT_OP(laserArgs[2].f, 8);
            shooter->exFloats[0] = FLOAT_OP(laserArgs[3].f, 0x10);
            shooter->exFloats[1] = FLOAT_OP(laserArgs[4].f, 0x20);
            shooter->exFloats[2] = FLOAT_OP(laserArgs[5].f, 0x40);
            shooter->exFloats[3] = FLOAT_OP(laserArgs[6].f, 0x80);
            shooter->exInts[0] = INT_OP(laserArgs[7].i, 0x100);
            shooter->exInts[1] = INT_OP(laserArgs[8].i, 0x200);
            shooter->exInts[2] = INT_OP(laserArgs[9].i, 0x400);
            shooter->exInts[3] = laserArgs[10].i;
            shooter->laserEndHitFrames = laserArgs[11].i;
            shooter->flags = laserArgs[12].i;
            if (instr->opcode == 115)
            {
                shooter->aimMode = 0;
            }
            else
            {
                shooter->aimMode = 1;
            }
            enemy->lasers[enemy->laserIndex] = g_BulletManager.SpawnLaser(shooter);
            break;
        }
        case 116:
            enemy->laserIndex = INT_ARG(0);
            break;
        case 117:
            value = INT_ARG(0);
            if (enemy->lasers[value] != NULL)
            {
                enemy->lasers[value]->angle = AddNormalizeAngle(enemy->lasers[value]->angle, FLOAT_ARG(1));
            }
            break;
        case 167:
            value = INT_ARG(0);
            if (enemy->lasers[value] != NULL)
            {
                enemy->lasers[value]->angle = FLOAT_ARG(1);
            }
            break;
        case 118:
            value = INT_ARG(0);
            if (enemy->lasers[value] != NULL)
            {
                enemy->lasers[value]->angle = g_Player.AngleFrom(&enemy->lasers[value]->pos) + FLOAT_ARG(1);
            }
            break;
        case 119:
            value = INT_ARG(0);
            if (enemy->lasers[value] != NULL)
            {
                enemy->lasers[value]->pos.x = FLOAT_ARG(1) + enemy->absPos.x;
                enemy->lasers[value]->pos.y = FLOAT_ARG(2) + enemy->absPos.y;
                enemy->lasers[value]->pos.z = FLOAT_ARG(3) + enemy->absPos.z;
            }
            break;
        case 170:
            value = INT_ARG(0);
            if (enemy->lasers[value] != NULL)
            {
                enemy->lasers[value]->drawDelayFlag = INT_ARG(1);
            }
            break;
        case 120:
            // Whether the laser is still active, into ints2[2].
            value = INT_ARG(0);
            if (enemy->lasers[value] != NULL && enemy->lasers[value]->active)
            {
                enemy->currentCtx->vars.ints2[2] = 1;
            }
            else
            {
                enemy->currentCtx->vars.ints2[2] = 0;
            }
            break;
        case 121:
            value = INT_ARG(0);
            if (enemy->lasers[value] != NULL && enemy->lasers[value]->active && enemy->lasers[value]->state < 2)
            {
                enemy->lasers[value]->state = 2;
                enemy->lasers[value]->timer = 0;
                enemy->lasers[value]->width = enemy->lasers[value]->baseWidth;
            }
            break;
        case 154:
        {
            int i;

            for (i = 0; i < 0x20; i++)
            {
                enemy->lasers[i] = NULL;
            }
            break;
        }
        case 171:
            value = INT_ARG(0);
            if (enemy->lasers[value] != NULL)
            {
                enemy->lasers[value]->maxLength = FLOAT_ARG(1);
            }
            break;
        case 172:
            value = INT_ARG(0);
            if (enemy->lasers[value] != NULL)
            {
                enemy->lasers[value]->startOffset = FLOAT_ARG(1);
                enemy->lasers[value]->endOffset = FLOAT_ARG(2);
            }
            break;
        case 163:
            g_EnemyManager.scriptArg163 = INT_ARG(0);
            break;
        case 127:
            if (INT_ARG(0) >= 0)
            {
                g_EnemyManager.bosses[INT_ARG(0)] = enemy;
                if (INT_ARG(0) == 0)
                {
                    g_Gui.SetBossPresent(1);
                    g_Gui.SetBossHealth(1.0f);
                }
                enemy->isBoss = 1;
                enemy->bossIndex = INT_ARG(0);
                g_AsciiText.SetNearbyLabelInterrupt(enemy->bossIndex, 1);
                enemy->minShootDistSq = 0.0f;
            }
            else
            {
                if (enemy->bossIndex < 4)
                {
                    g_Gui.SetBossPresent(0);
                }
                g_EnemyManager.bosses[enemy->bossIndex] = NULL;
                enemy->isBoss = 0;
                g_AsciiText.SetNearbyLabelInterrupt(enemy->bossIndex, 2);
                ReleaseEnemyEffects(enemy);
                g_AsciiText.SetNearbyLabelPos(enemy->bossIndex, &D3DXVECTOR3(-999.0f, -999.0f, 0.0f));
            }
            break;
        case 128:
        {
            EnemyScriptArg *effectArgs;

            effectArgs = instr->args;
            enemy->effects[enemy->effectCount] = g_EffectManager.SpawnEffect(0xd, &enemy->pos, 1, 0xff6060d0);
            enemy->effects[enemy->effectCount]->axis = *(D3DXVECTOR3 *)&effectArgs[1];
            enemy->effectRadius = effectArgs[4].f;
            enemy->effectCount++;
            break;
        }
        case 159:
            enemy->drawListIndex = INT_ARG(0);
            break;
        case 124:
            g_SoundPlayer.QueueSoundPanned(INT_ARG(0), enemy->pos.x);
            break;
        case 129:
            if (!g_GameManager.isInSpellCardPractice || !g_GameManager.lastSpellState)
            {
                enemy->deathMode = *(BYTE *)instr->args;
            }
            break;
        case 130:
            if (!g_GameManager.isInSpellCardPractice || !g_GameManager.lastSpellState)
            {
                enemy->deathSub = *(short *)instr->args;
            }
            break;
        case 126:
            enemy->interrupts[INT_ARG(1)] = INT_ARG(0);
            break;
        case 125:
            enemy->pendingInterrupt = INT_ARG(0);
        interrupt:
            enemy->currentCtx->code = (char *)instr + instr->size;
            if (!enemy->disableCallStack)
            {
                enemy->callStack[enemy->stackDepth] = enemy->ctx;
            }
            g_EnemyScripts.StartSub(&enemy->ctx, enemy->interrupts[enemy->pendingInterrupt]);
            if (enemy->stackDepth < 15)
            {
                enemy->stackDepth++;
            }
            enemy->pendingInterrupt = -1;
            goto restart;
        case 131:
            enemy->phaseStartLife = enemy->life = enemy->maxLife = INT_ARG(0);
            if (!enemy->bossIndex && enemy->isBoss)
            {
                int i;

                for (i = 0; i < 8; i++)
                {
                    g_Gui.SetBossSection(i, 0.0f, 0.0f);
                }
            }
            break;
        case 158:
        {
            int idx;

            idx = INT_ARG(0);
            g_Gui.SetBossSection(idx, INT_ARG(1) / (float)enemy->maxLife, INT_ARG(2) / (float)enemy->maxLife);
            g_Gui.SetBossSectionColor(idx, INT_ARG(3));
            break;
        }
        case 122:
            StartSpellCard(enemy, instr);
            break;
        case 123:
            EndSpellCard(enemy, instr);
            break;
        case 132:
            enemy->callbackTimer = INT_ARG(0);
            break;
        case 133:
            if (!g_GameManager.isInSpellCardPractice || !g_GameManager.lastSpellState)
            {
                enemy->lifeCallbackLife[INT_ARG(0)] = INT_ARG(1);
                enemy->lifeCallbackSub[INT_ARG(0)] = INT_ARG(2);
            }
            else
            {
                enemy->lifeCallbackLife[INT_ARG(0)] = INT_ARG(1);
            }
            break;
        case 134:
            if (!g_GameManager.isInSpellCardPractice || !g_GameManager.lastSpellState)
            {
                enemy->timerCallbackTime = INT_ARG(0);
                enemy->timerCallbackSub = INT_ARG(1);
            }
            else
            {
                enemy->timerCallbackTime = INT_ARG(0);
            }
            enemy->callbackTimer = 0;
            break;
        case 135:
            value = INT_ARG(0);
            if (enemy->threads[value] != NULL)
            {
                g_MemoryManager.Free(enemy->threads[value]);
            }
            enemy->threads[value] = NULL;
            if (INT_ARG(1) >= 0)
            {
                enemy->threads[value] =
                    (EnemyScriptThread *)g_MemoryManager.Alloc(sizeof(EnemyScriptThread), "ECLInt");
                if (enemy->threads[value] != NULL)
                {
                    memset(enemy->threads[value], 0, sizeof(EnemyScriptThread));
                    enemy->threads[value]->sub = INT_ARG(1);
                    g_EnemyScripts.StartSub(&enemy->threads[value]->ctx, enemy->threads[value]->sub);
                    enemy->threads[value]->ctx.vars = enemy->currentCtx->vars;
                }
            }
            break;
        case 139:
            g_EffectManager.SpawnEffect(INT_ARG(0), &enemy->pos, INT_ARG(1), INT_VAR(2));
            break;
        case 140:
        {
            D3DXVECTOR3 velocity;

            velocity.x = FLOAT_ARG(3);
            velocity.y = FLOAT_ARG(4);
            velocity.z = FLOAT_ARG(5);
            g_EffectManager.SpawnEffectWithVelocity(INT_ARG(0), &enemy->pos, &velocity, INT_ARG(1), INT_VAR(2));
            break;
        }
        case 143:
            enemy->item = INT_ARG(0);
            break;
        case 144:
            enemy->pointItems = INT_ARG(0);
            enemy->powerItems = INT_ARG(1);
            break;
        case 142:
        {
            int count;
            int i;
            D3DXVECTOR3 pos;

            count = INT_ARG(0);
            for (i = 0; i < count; i++)
            {
                pos = enemy->pos;
                pos[0] += g_Rng.GetRandomF32ZeroToOne() * 128.0f - 64.0f;
                pos[1] += g_Rng.GetRandomF32ZeroToOne() * 128.0f - 64.0f;
                if (g_GameManager.GetPower() < 0x80)
                {
                    g_ItemManager.SpawnItem(&pos, i == 0 ? 2 : 0, 0);
                }
                else
                {
                    g_ItemManager.SpawnItem(&pos, 1, 0);
                }
            }
            break;
        }
        case 168:
        {
            int count;
            int i;
            D3DXVECTOR3 pos;

            count = INT_ARG(0);
            for (i = 0; i < count; i++)
            {
                pos = enemy->pos;
                pos[0] += g_Rng.GetRandomF32ZeroToOne() * 128.0f - 64.0f;
                pos[1] += g_Rng.GetRandomF32ZeroToOne() * 128.0f - 64.0f;
                g_ItemManager.SpawnItem(&pos, 1, 0);
            }
            break;
        }
        case 145:
            enemy->rotateSprite = *(BYTE *)instr->args;
            break;
        case 136:
            g_EnemyScriptFuncs[INT_ARG(0)](enemy, instr);
            break;
        case 137:
            if (INT_ARG(0) >= 0)
            {
                enemy->currentCtx->func = g_EnemyScriptFuncs[INT_ARG(0)];
                enemy->currentCtx->funcInstr = instr;
            }
            else
            {
                enemy->currentCtx->func = NULL;
            }
            break;
        case 146:
            enemy->currentCtx->timer += INT_ARG(0);
            break;
        case 141:
            g_ItemManager.SpawnItem(&enemy->pos, INT_ARG(0), 0);
            break;
        case 147:
            g_Stage.waitLabel = INT_ARG(0);
            break;
        case 148:
            g_Gui.SetBossLives(INT_ARG(0));
            g_GameManager.progressFrames += 1800;
            break;
        case 93:
        {
            Enemy *child;

            if (enemy->life > 0)
            {
                EnemySpawnArgs spawn;
                D3DXVECTOR3 pos;

                spawn = *(EnemySpawnArgs *)instr->args;
                pos.x = FLOAT_OP(spawn.pos.x, 2);
                pos.y = FLOAT_OP(spawn.pos.y, 4);
                pos.z = FLOAT_OP(spawn.pos.z, 8);
                child = g_EnemyManager.SpawnWithVars(spawn.sub, &pos, INT_ARG(4), INT_ARG(5), INT_ARG(6),
                                                     &enemy->currentCtx->vars);
            }
            break;
        }
        case 94:
        {
            Enemy *child;

            if (enemy->life > 0)
            {
                EnemySpawnArgs spawn;
                D3DXVECTOR3 pos;

                spawn = *(EnemySpawnArgs *)instr->args;
                pos.x = FLOAT_OP(spawn.pos.x, 2);
                pos.y = FLOAT_OP(spawn.pos.y, 4);
                pos.z = FLOAT_OP(spawn.pos.z, 8);
                pos += enemy->pos;
                child = g_EnemyManager.SpawnWithVars(spawn.sub, &pos, INT_ARG(4), INT_ARG(5), INT_ARG(6),
                                                     &enemy->currentCtx->vars);
            }
            break;
        }
        case 95:
            g_EnemyManager.ClearEnemiesForBonus(8000, 0);
            break;
        case 149:
            enemy->sprite.pendingInterrupt = INT_ARG(0);
            break;
        case 150:
            enemy->sprites[instr->args[0].i].pendingInterrupt = instr->args[1].i;
            break;
        case 112:
            g_BulletManager.ClearAllToItems();
            break;
        case 113:
            if (INT_ARG(0) >= 0)
            {
                enemy->shooter.sound = INT_ARG(0);
                enemy->shooter.flags |= 0x200;
            }
            else
            {
                enemy->shooter.flags &= ~0x200;
            }
            enemy->shooter.bulletSound = INT_ARG(1);
            break;
        case 151:
            enemy->disableCallStack = *(BYTE *)instr->args;
            break;
        case 152:
            enemy->rankSpeedLow = FLOAT_ARG(0);
            enemy->rankSpeedHigh = FLOAT_ARG(1);
            enemy->rankCount1Low = INT_ARG(2);
            enemy->rankCount1High = INT_ARG(3);
            enemy->rankCount2Low = INT_ARG(4);
            enemy->rankCount2High = INT_ARG(5);
            break;
        case 153:
            enemy->timerCallbackSub = enemy->deathSub;
            enemy->callbackTimer = 0;
            break;
        case 155:
            enemy->keepSpellCard = *(BYTE *)instr->args;
            g_SpellCard.startBonus = 99999990;
            break;
        case 156:
            enemy->grazeable = *(BYTE *)instr->args;
            enemy->drawListIndex = 2;
            break;
        case 157:
            enemy->trailFlags = instr->args[0].i;
            enemy->trailSegmentCount = INT_ARG(1);
            enemy->trailContactSegments = INT_ARG(2);
            enemy->trailSegmentStep = INT_ARG(3);
            if (enemy->trailFlags & 8)
            {
                g_SpriteEngine->FillSpriteStripVertices(&enemy->sprite, enemy->spriteStripVertices,
                                                        enemy->trailSegmentCount / enemy->trailSegmentStep * 2);
            }
            break;
        case 160:
            enemy->invulnTimer = INT_ARG(0);
            break;
        case 161:
            g_BulletManager.ConvertBulletsToItems(&enemy->absPos, FLOAT_ARG(0));
            break;
        case 162:
            g_BulletManager.ClearAll(4);
            break;
        case 164:
            value = INT_ARG(0);
            g_SpellCard.SetEffectFixed(value);
            if (value == 0)
            {
                g_SpellCard.SetEffectPos(FLOAT_ARG(1), FLOAT_ARG(2), FLOAT_ARG(3));
            }
            break;
        case 165:
            enemy->sprite.rotation.z = FLOAT_ARG(0);
            break;
        case 166:
            *ResolveFloatOperand(enemy, &instr->args[1].f, instr->paramMask, 1) = sinf(FLOAT_ARG(2)) * FLOAT_ARG(3);
            FLOAT_VAR(0) = cosf(FLOAT_ARG(2)) * FLOAT_ARG(3);
            break;
        case 169:
            if ((g_Player.pos.x < enemy->pos.x && enemy->pos.x > 96.0f) || enemy->pos.x > 288.0f)
            {
                FLOAT_VAR(0) = AddNormalizeAngle(g_Rng.GetRandomF32InRange(1.5707964f) + 2.3561945f, 0.0f);
            }
            else
            {
                FLOAT_VAR(0) = g_Rng.GetRandomF32InRange(1.5707964f) - 0.7853982f;
            }
            break;
        case 173:
            enemy->haltedByBomb = INT_ARG(0);
            break;
        case 183:
            enemy->intangibleDuringBomb = INT_ARG(0);
            break;
        case 176:
            g_GameManager.lastSpellState = 1;
            g_GameManager.bossHitAnywhere = 0;
            if (!g_GameManager.isInSpellCardPractice)
            {
                if (g_GameManager.stage == 6 || g_GameManager.stage == 7)
                {
                    g_GameManager.bossHitAnywhere = 1;
                }
            }
            else if ((g_GameManager.practiceSpellCard >= 0x8f && g_GameManager.practiceSpellCard <= 0x92) ||
                     (g_GameManager.practiceSpellCard >= 0xab && g_GameManager.practiceSpellCard <= 0xbe))
            {
                g_GameManager.bossHitAnywhere = 1;
            }
            enemy->haltedByBomb = 1;
            break;
        case 82:
            enemy->minShootDistSq = FLOAT_ARG(0);
            enemy->minShootDistSq *= enemy->minShootDistSq;
            break;
        case 83:
            enemy->emitsEffect26 = INT_ARG(0);
            break;
        case 174:
            if (enemy->attachedEffect != NULL)
            {
                enemy->attachedEffect->active = 0;
            }
            enemy->attachedEffect = g_EffectManager.SpawnEffect2(INT_ARG(0) + 0x20, &enemy->absPos, 1, 0xffffffff);
            enemy->attachedEffect->sprite.SetPendingInterrupt(g_Player.IsYoukai() ? 2 : 1);
            if (enemy->index & 1)
            {
                enemy->attachedEffect->sprite.rotationSpeed.z = -enemy->attachedEffect->sprite.rotationSpeed.z;
            }
            break;
        case 175:
            g_EnemyManager.holdTimelineSpawns = INT_ARG(0);
            break;
        case 177:
            enemy->phaseStartLife = INT_ARG(0);
            break;
        case 179:
            g_Gui.StartClock();
            break;
        case 180:
            g_Gui.HideClock();
            break;
        case 181:
            if (g_GameManager.GetClock() < 12)
            {
                g_SoundPlayer.QueueSound(0x2d, 0);
                g_GameManager.AddClock(1);
                if (g_GameManager.GetClock() == 12)
                {
                    g_Gui.AdvanceClockToEnd();
                }
                else
                {
                    g_Gui.AdvanceClock();
                }
            }
            break;
        case 182:
            enemy->useMainSpriteOffset = INT_ARG(0);
            break;
        case 184:
            g_SpellCard.SetBonusFrozen(INT_ARG(0));
            break;
        }
    next:
        instr = (EnemyScriptInstr *)((char *)instr + instr->size);
        goto loop;
    }
done:
    if (enemy->life > 0)
    {
        int i;
        BOOL moved;
        float time;
        EnemyScriptLoop *l;

        moved = FALSE;
        l = enemy->currentCtx->loops;
        D3DXVECTOR3 savedPos = enemy->pos;
        if (enemy->currentCtx->func != NULL)
        {
            enemy->currentCtx->func(enemy, enemy->currentCtx->funcInstr);
        }
        for (i = 0; i < 8; i++, l++)
        {
            if (l->func != NULL)
            {
                l->timer++;
                if (l->timer >= l->duration)
                {
                    l->timer = l->duration;
                }
                time = l->timer.AsFloat() / l->duration;
                switch (l->mode)
                {
                case 1:
                    time = time * time;
                    break;
                case 2:
                    time = time * time * time;
                    break;
                case 3:
                    time = time * time * time * time;
                    break;
                case 4:
                    time = (time = 1.0f - time) * time;
                    time = 1.0f - time;
                    break;
                case 5:
                    time = (time = 1.0f - time) * time * time;
                    time = 1.0f - time;
                    break;
                case 6:
                    time = (time = 1.0f - time) * time * time * time;
                    time = 1.0f - time;
                    break;
                }
                l->func(enemy, (EnemyScriptInstr *)l, time);
                if (l->timer >= l->duration)
                {
                    l->func = NULL;
                }
                // A position variable: the move becomes a velocity below.
                if (l->target == 10042.0f || l->target == 10043.0f || l->target == 10044.0f)
                {
                    moved = TRUE;
                }
            }
        }
        if (moved)
        {
            enemy->velocity.x = enemy->pos.x - savedPos.x;
            enemy->velocity.y = enemy->pos.y - savedPos.y;
            enemy->angle = atan2f(enemy->velocity.y, enemy->velocity.x);
            enemy->pos = savedPos;
        }
    }
    if (ctxIndex == -1)
    {
        enemy->savedStackDepth = enemy->stackDepth;
    }
    else
    {
        enemy->threads[ctxIndex]->stackDepth = enemy->stackDepth;
    }
    enemy->currentCtx->code = instr;
    enemy->currentCtx->timer++;
    // Then the threads after this one, each through the same loop.
    {
        int i;
        EnemyScriptThread *buf;

    nextThread:
        for (i = ctxIndex + 1; i < 4; i++)
        {
            if (enemy->threads[i] != NULL)
            {
                buf = enemy->threads[i];
                enemy->callStack = buf->callStack;
                enemy->currentCtx = &buf->ctx;
                instr = (EnemyScriptInstr *)enemy->currentCtx->code;
                enemy->currentCtx->timedCallActive = i + 1;
                enemy->stackDepth = buf->stackDepth;
                ctxIndex = i;
                goto loop;
            }
        }
    }
    enemy->callStack = enemy->savedCtx;
    enemy->currentCtx = &enemy->ctx;
    ScriptUpdateMovement(enemy);
    ScriptUpdateShooting(enemy);
    return 0;
}

Enemy *ScriptSpawnEnemy(Enemy *enemy, EnemyScriptInstr *instr)
{
    Enemy *spawned = &g_EnemyManager.enemies[0x1e0];

    if (enemy->life > 0 && !enemy->releasedFromLeader)
    {
        D3DXVECTOR3 pos;

        pos.x = FLOAT_ARG(1);
        pos.y = FLOAT_ARG(2);
        pos.z = 0.0f;
        spawned = g_EnemyManager.SpawnWithVars(instr->args[0].i, &pos, INT_ARG(3), INT_ARG(4), INT_ARG(5),
                                               &enemy->currentCtx->vars);
    }
    else
    {
        g_EnemyManager.spawnFailed = 1;
    }
    return spawned;
}

Enemy *ScriptSpawnEnemyRel(Enemy *enemy, EnemyScriptInstr *instr)
{
    Enemy *spawned = &g_EnemyManager.enemies[0x1e0];

    if (enemy->life > 0 && !enemy->releasedFromLeader)
    {
        D3DXVECTOR3 pos;

        pos.x = FLOAT_ARG(1);
        pos.y = FLOAT_ARG(2);
        pos.z = 0.0f;
        pos += enemy->absPos;
        spawned = g_EnemyManager.SpawnWithVars(instr->args[0].i, &pos, INT_ARG(3), INT_ARG(4), INT_ARG(5),
                                               &enemy->currentCtx->vars);
    }
    else
    {
        g_EnemyManager.spawnFailed = 1;
    }
    return spawned;
}

int GetIntOperand(Enemy *enemy, int id)
{
    switch (id)
    {
    case 10000:
        return enemy->currentCtx->vars.ints[0];
    case 10001:
        return enemy->currentCtx->vars.ints[1];
    case 10002:
        return enemy->currentCtx->vars.ints[2];
    case 10003:
        return enemy->currentCtx->vars.ints[3];
    case 10004:
        return enemy->currentCtx->vars.ints[4];
    case 10005:
        return enemy->currentCtx->vars.ints[5];
    case 10006:
        return enemy->currentCtx->vars.ints[6];
    case 10007:
        return enemy->currentCtx->vars.ints[7];
    case 10008:
        return enemy->enemyInts[0];
    case 10009:
        return enemy->enemyInts[1];
    case 10010:
        return enemy->enemyInts[2];
    case 10011:
        return enemy->enemyInts[3];
    case 10012:
        return enemy->enemyInts[4];
    case 10013:
        return enemy->enemyInts[5];
    case 10014:
        return enemy->enemyInts[6];
    case 10015:
        return enemy->enemyInts[7];
    case 10053:
        return enemy->currentCtx->vars.args.ints[0];
    case 10054:
        return enemy->currentCtx->vars.args.ints[1];
    case 10055:
        return enemy->currentCtx->vars.args.ints[2];
    case 10056:
        return enemy->currentCtx->vars.args.ints[3];
    case 10036:
        return enemy->currentCtx->vars.ints2[0];
    case 10037:
        return enemy->currentCtx->vars.ints2[1];
    case 10038:
        return enemy->currentCtx->vars.ints2[2];
    case 10039:
        return enemy->currentCtx->vars.ints2[3];
    case 10032:
        return g_Rng.GetRandomU32() & 0x7fffffff;
    case 10033:
        return g_Rng.GetRandomF32ZeroToOne();
    case 10034:
        return g_Rng.GetRandomU32();
    case 10035:
        return g_Rng.GetRandomF32MinusOneToOne();
    case 10040:
        return g_GameManager.difficulty;
    case 10041:
        return g_GameManager.rank;
    case 10049:
        return enemy->callbackTimer.current;
    case 10051:
        return enemy->life;
    case 10052:
        return g_GameManager.character;
    case 10094:
        return enemy->currentCtx->vars.floats2[0];
    case 10095:
        return enemy->currentCtx->vars.floats2[1];
    case 10016:
        return enemy->currentCtx->vars.floats[0];
    case 10017:
        return enemy->currentCtx->vars.floats[1];
    case 10018:
        return enemy->currentCtx->vars.floats[2];
    case 10019:
        return enemy->currentCtx->vars.floats[3];
    case 10020:
        return enemy->currentCtx->vars.floats[4];
    case 10021:
        return enemy->currentCtx->vars.floats[5];
    case 10022:
        return enemy->currentCtx->vars.floats[6];
    case 10023:
        return enemy->currentCtx->vars.floats[7];
    case 10024:
        return enemy->enemyFloats[0];
    case 10025:
        return enemy->enemyFloats[1];
    case 10026:
        return enemy->enemyFloats[2];
    case 10027:
        return enemy->enemyFloats[3];
    case 10028:
        return enemy->enemyFloats[4];
    case 10029:
        return enemy->enemyFloats[5];
    case 10030:
        return enemy->enemyFloats[6];
    case 10031:
        return enemy->enemyFloats[7];
    case 10057:
        return enemy->currentCtx->vars.args.floats[0];
    case 10058:
        return enemy->currentCtx->vars.args.floats[1];
    case 10059:
        return enemy->currentCtx->vars.args.floats[2];
    case 10060:
        return enemy->currentCtx->vars.args.floats[3];
    case 10061:
        return g_EnemyScripts.callArgs.ints[0];
    case 10062:
        return g_EnemyScripts.callArgs.ints[1];
    case 10063:
        return g_EnemyScripts.callArgs.ints[2];
    case 10064:
        return g_EnemyScripts.callArgs.ints[3];
    case 10065:
        return g_EnemyScripts.callArgs.floats[0];
    case 10066:
        return g_EnemyScripts.callArgs.floats[1];
    case 10067:
        return g_EnemyScripts.callArgs.floats[2];
    case 10068:
        return g_EnemyScripts.callArgs.floats[3];
    case 10042:
        return enemy->absPos.x;
    case 10043:
        return enemy->absPos.y;
    case 10044:
        return enemy->absPos.z;
    case 10045:
        return g_Player.pos.x;
    case 10046:
        return g_Player.pos.y;
    case 10047:
        return g_Player.pos.z;
    case 10074:
        return enemy->moveStart.x;
    case 10075:
        return enemy->moveStart.y;
    case 10076:
        return enemy->moveStart.z;
    case 10085:
        return enemy->posDelta.x;
    case 10086:
        return enemy->posDelta.y;
    case 10087:
        return enemy->posDelta.z;
    case 10088:
        return enemy->lifeCallbackLife[0];
    case 10089:
        return enemy->lifeCallbackLife[1];
    case 10090:
        return enemy->lifeCallbackLife[2];
    case 10091:
        return enemy->lifeCallbackLife[3];
    case 10069:
        return enemy->angle;
    case 10070:
        return enemy->angularVelocity;
    case 10071:
        return enemy->speed;
    case 10072:
        return enemy->acceleration;
    case 10073:
        return enemy->orbitRadius;
    case 10077:
        return enemy->orbitAngle;
    case 10078:
        return enemy->orbitAngularVelocity;
    case 10083:
        return enemy->lastDamage;
    case 10084:
        return enemy->bossIndex;
    case 10092:
        return enemy->item;
    case 10093:
        return enemy->score;
    case 10096:
        return enemy->IsLeader() ? enemy->CountFollowers() : (enemy->IsFollower() ? enemy->leader->CountFollowers() : 0);
    case 10048:
        return g_Player.AngleFrom(&enemy->absPos);
    case 10050:
    {
        D3DXVECTOR3 diff = g_Player.pos - enemy->absPos;

        return D3DXVec3Length(&diff);
    }
    case 10097:
        return g_Player.IsYoukai();
    case 10098:
        return g_GameManager.GetTimeOrbs() + g_SpellCard.GetTimeOrbBonus() + g_ItemManager.CountType7Items() < g_GameManager.GetTimeOrbsNeeded() ? 0 : 2;
    case 10099:
        return g_SpellCard.IsActive() ? g_SpellCard.IsCapturable() : g_SpellCard.IsCaptured();
    case 10100:
        return g_SpellCard.GetTimeLeft();
    }
    return id;
}

int *ResolveOperand(Enemy *enemy, int *operand, WORD mask, int bit)
{
    if (bit >= 0 && (mask & (1 << bit)) == 0)
    {
        return operand;
    }
    switch (*operand)
    {
    case 10000:
        return &enemy->currentCtx->vars.ints[0];
    case 10001:
        return &enemy->currentCtx->vars.ints[1];
    case 10002:
        return &enemy->currentCtx->vars.ints[2];
    case 10003:
        return &enemy->currentCtx->vars.ints[3];
    case 10004:
        return &enemy->currentCtx->vars.ints[4];
    case 10005:
        return &enemy->currentCtx->vars.ints[5];
    case 10006:
        return &enemy->currentCtx->vars.ints[6];
    case 10007:
        return &enemy->currentCtx->vars.ints[7];
    case 10008:
        return &enemy->enemyInts[0];
    case 10009:
        return &enemy->enemyInts[1];
    case 10010:
        return &enemy->enemyInts[2];
    case 10011:
        return &enemy->enemyInts[3];
    case 10012:
        return &enemy->enemyInts[4];
    case 10013:
        return &enemy->enemyInts[5];
    case 10014:
        return &enemy->enemyInts[6];
    case 10015:
        return &enemy->enemyInts[7];
    case 10053:
        return &enemy->currentCtx->vars.args.ints[0];
    case 10054:
        return &enemy->currentCtx->vars.args.ints[1];
    case 10055:
        return &enemy->currentCtx->vars.args.ints[2];
    case 10056:
        return &enemy->currentCtx->vars.args.ints[3];
    case 10036:
        return &enemy->currentCtx->vars.ints2[0];
    case 10037:
        return &enemy->currentCtx->vars.ints2[1];
    case 10038:
        return &enemy->currentCtx->vars.ints2[2];
    case 10039:
        return &enemy->currentCtx->vars.ints2[3];
    case 10040:
        return &g_GameManager.difficulty;
    case 10041:
        return &g_GameManager.rank;
    case 10049:
        return &enemy->callbackTimer.current;
    case 10051:
        return &enemy->life;
    case 10092:
        return &enemy->item;
    case 10093:
        return &enemy->score;
    case 10061:
        return &g_EnemyScripts.callArgs.ints[0];
    case 10062:
        return &g_EnemyScripts.callArgs.ints[1];
    case 10063:
        return &g_EnemyScripts.callArgs.ints[2];
    case 10064:
        return &g_EnemyScripts.callArgs.ints[3];
    }
    return operand;
}

float GetFloatOperand(Enemy *enemy, float id)
{
    switch ((int)id)
    {
    case 10000:
        return enemy->currentCtx->vars.ints[0];
    case 10001:
        return enemy->currentCtx->vars.ints[1];
    case 10002:
        return enemy->currentCtx->vars.ints[2];
    case 10003:
        return enemy->currentCtx->vars.ints[3];
    case 10004:
        return enemy->currentCtx->vars.ints[4];
    case 10005:
        return enemy->currentCtx->vars.ints[5];
    case 10006:
        return enemy->currentCtx->vars.ints[6];
    case 10007:
        return enemy->currentCtx->vars.ints[7];
    case 10008:
        return enemy->enemyInts[0];
    case 10009:
        return enemy->enemyInts[1];
    case 10010:
        return enemy->enemyInts[2];
    case 10011:
        return enemy->enemyInts[3];
    case 10012:
        return enemy->enemyInts[4];
    case 10013:
        return enemy->enemyInts[5];
    case 10014:
        return enemy->enemyInts[6];
    case 10015:
        return enemy->enemyInts[7];
    case 10053:
        return enemy->currentCtx->vars.args.ints[0];
    case 10054:
        return enemy->currentCtx->vars.args.ints[1];
    case 10055:
        return enemy->currentCtx->vars.args.ints[2];
    case 10056:
        return enemy->currentCtx->vars.args.ints[3];
    case 10036:
        return enemy->currentCtx->vars.ints2[0];
    case 10037:
        return enemy->currentCtx->vars.ints2[1];
    case 10038:
        return enemy->currentCtx->vars.ints2[2];
    case 10039:
        return enemy->currentCtx->vars.ints2[3];
    case 10032:
        return g_Rng.GetRandomU32() & 0x7fffffff;
    case 10033:
        return g_Rng.GetRandomF32ZeroToOne();
    case 10034:
        return (int)g_Rng.GetRandomU32();
    case 10035:
        return g_Rng.GetRandomF32MinusOneToOne();
    case 10082:
        return g_Rng.GetRandomF32InRange(6.2831855f) - 3.1415927f;
    case 10040:
        return g_GameManager.difficulty;
    case 10041:
        return g_GameManager.rank;
    case 10049:
        return enemy->callbackTimer.current;
    case 10051:
        return enemy->life;
    case 10052:
        return g_GameManager.character;
    case 10092:
        return enemy->item;
    case 10093:
        return enemy->score;
    case 10061:
        return g_EnemyScripts.callArgs.ints[0];
    case 10062:
        return g_EnemyScripts.callArgs.ints[1];
    case 10063:
        return g_EnemyScripts.callArgs.ints[2];
    case 10064:
        return g_EnemyScripts.callArgs.ints[3];
    case 10065:
        return g_EnemyScripts.callArgs.floats[0];
    case 10066:
        return g_EnemyScripts.callArgs.floats[1];
    case 10067:
        return g_EnemyScripts.callArgs.floats[2];
    case 10068:
        return g_EnemyScripts.callArgs.floats[3];
    case 10016:
        return enemy->currentCtx->vars.floats[0];
    case 10017:
        return enemy->currentCtx->vars.floats[1];
    case 10018:
        return enemy->currentCtx->vars.floats[2];
    case 10019:
        return enemy->currentCtx->vars.floats[3];
    case 10020:
        return enemy->currentCtx->vars.floats[4];
    case 10021:
        return enemy->currentCtx->vars.floats[5];
    case 10022:
        return enemy->currentCtx->vars.floats[6];
    case 10023:
        return enemy->currentCtx->vars.floats[7];
    case 10024:
        return enemy->enemyFloats[0];
    case 10025:
        return enemy->enemyFloats[1];
    case 10026:
        return enemy->enemyFloats[2];
    case 10027:
        return enemy->enemyFloats[3];
    case 10028:
        return enemy->enemyFloats[4];
    case 10029:
        return enemy->enemyFloats[5];
    case 10030:
        return enemy->enemyFloats[6];
    case 10031:
        return enemy->enemyFloats[7];
    case 10057:
        return enemy->currentCtx->vars.args.floats[0];
    case 10058:
        return enemy->currentCtx->vars.args.floats[1];
    case 10059:
        return enemy->currentCtx->vars.args.floats[2];
    case 10060:
        return enemy->currentCtx->vars.args.floats[3];
    case 10042:
        return enemy->absPos.x;
    case 10043:
        return enemy->absPos.y;
    case 10044:
        return enemy->absPos.z;
    case 10045:
        return g_Player.pos.x;
    case 10046:
        return g_Player.pos.y;
    case 10047:
        return g_Player.pos.z;
    case 10094:
        return enemy->currentCtx->vars.floats2[0];
    case 10095:
        return enemy->currentCtx->vars.floats2[1];
    case 10074:
        return enemy->moveStart.x;
    case 10075:
        return enemy->moveStart.y;
    case 10076:
        return enemy->moveStart.z;
    case 10079:
        return enemy->moveDelta.x;
    case 10080:
        return enemy->moveDelta.y;
    case 10081:
        return enemy->moveDelta.z;
    case 10085:
        return enemy->posDelta.x;
    case 10086:
        return enemy->posDelta.y;
    case 10087:
        return enemy->posDelta.z;
    case 10088:
        return enemy->lifeCallbackLife[0];
    case 10089:
        return enemy->lifeCallbackLife[1];
    case 10090:
        return enemy->lifeCallbackLife[2];
    case 10091:
        return enemy->lifeCallbackLife[3];
    case 10048:
        return g_Player.AngleFrom(&enemy->absPos);
    case 10069:
        return enemy->angle;
    case 10070:
        return enemy->angularVelocity;
    case 10071:
        return enemy->speed;
    case 10072:
        return enemy->acceleration;
    case 10073:
        return enemy->orbitRadius;
    case 10077:
        return enemy->orbitAngle;
    case 10078:
        return enemy->orbitAngularVelocity;
    case 10084:
        return enemy->bossIndex;
    case 10083:
        return enemy->lastDamage;
    case 10096:
        return enemy->IsLeader() ? enemy->CountFollowers() : (enemy->IsFollower() ? enemy->leader->CountFollowers() : 0);
    case 10050:
    {
        D3DXVECTOR3 diff = g_Player.pos - enemy->absPos;

        return D3DXVec3Length(&diff);
    }
    case 10097:
        return g_Player.IsYoukai();
    case 10099:
        return g_SpellCard.IsActive() ? g_SpellCard.IsCapturable() : g_SpellCard.IsCaptured();
    }
    return id;
}

float *ResolveFloatOperand(Enemy *enemy, float *operand, WORD mask, int bit)
{
    if (bit >= 0 && (mask & (1 << bit)) == 0)
    {
        return operand;
    }
    switch ((int)*operand)
    {
    case 10016:
        return &enemy->currentCtx->vars.floats[0];
    case 10017:
        return &enemy->currentCtx->vars.floats[1];
    case 10018:
        return &enemy->currentCtx->vars.floats[2];
    case 10019:
        return &enemy->currentCtx->vars.floats[3];
    case 10020:
        return &enemy->currentCtx->vars.floats[4];
    case 10021:
        return &enemy->currentCtx->vars.floats[5];
    case 10022:
        return &enemy->currentCtx->vars.floats[6];
    case 10023:
        return &enemy->currentCtx->vars.floats[7];
    case 10024:
        return &enemy->enemyFloats[0];
    case 10025:
        return &enemy->enemyFloats[1];
    case 10026:
        return &enemy->enemyFloats[2];
    case 10027:
        return &enemy->enemyFloats[3];
    case 10028:
        return &enemy->enemyFloats[4];
    case 10029:
        return &enemy->enemyFloats[5];
    case 10030:
        return &enemy->enemyFloats[6];
    case 10031:
        return &enemy->enemyFloats[7];
    case 10057:
        return &enemy->currentCtx->vars.args.floats[0];
    case 10058:
        return &enemy->currentCtx->vars.args.floats[1];
    case 10059:
        return &enemy->currentCtx->vars.args.floats[2];
    case 10060:
        return &enemy->currentCtx->vars.args.floats[3];
    case 10042:
        return &enemy->pos.x;
    case 10043:
        return &enemy->pos.y;
    case 10044:
        return &enemy->pos.z;
    case 10045:
        return &g_Player.pos.x;
    case 10046:
        return &g_Player.pos.y;
    case 10047:
        return &g_Player.pos.z;
    case 10094:
        return &enemy->currentCtx->vars.floats2[0];
    case 10095:
        return &enemy->currentCtx->vars.floats2[1];
    case 10065:
        return &g_EnemyScripts.callArgs.floats[0];
    case 10066:
        return &g_EnemyScripts.callArgs.floats[1];
    case 10067:
        return &g_EnemyScripts.callArgs.floats[2];
    case 10068:
        return &g_EnemyScripts.callArgs.floats[3];
    case 10074:
        return &enemy->moveStart.x;
    case 10075:
        return &enemy->moveStart.y;
    case 10076:
        return &enemy->moveStart.z;
    case 10079:
        return &enemy->moveDelta.x;
    case 10080:
        return &enemy->moveDelta.y;
    case 10081:
        return &enemy->moveDelta.z;
    case 10069:
        return &enemy->angle;
    case 10070:
        return &enemy->angularVelocity;
    case 10071:
        return &enemy->speed;
    case 10072:
        return &enemy->acceleration;
    case 10073:
        return &enemy->orbitRadius;
    case 10077:
        return &enemy->orbitAngle;
    case 10078:
        return &enemy->orbitAngularVelocity;
    }
    return operand;
}

void ScriptMoveAngle(Enemy *enemy, EnemyScriptInstr *instr)
{
    float angle;

    angle = AddNormalizeAngle(FLOAT_ARG(2), 0.0f);
    enemy->moveDelta.x = cosf(angle) * FLOAT_ARG(3) * INT_ARG(0);
    enemy->moveDelta.y = sinf(angle) * FLOAT_ARG(3) * INT_ARG(0);
    enemy->moveDelta.z = 0.0f;
    enemy->moveStart = enemy->absPos;
    enemy->moveTimer = (enemy->moveDuration = INT_ARG(0));
    enemy->moveEasing = INT_ARG(1);
    enemy->moveMode = 2;
    if (enemy->mirrored)
    {
        enemy->moveDelta.x = -enemy->moveDelta.x;
    }
}

void ScriptMovePos(Enemy *enemy, EnemyScriptInstr *instr)
{
    D3DXVECTOR3 target;

    target.x = FLOAT_ARG(2);
    target.y = FLOAT_ARG(3);
    target.z = 0.0f;
    enemy->moveDelta = target - enemy->absPos;
    enemy->moveStart = enemy->pos;
    enemy->moveTimer = (enemy->moveDuration = INT_ARG(0));
    enemy->moveEasing = INT_ARG(1);
    enemy->moveMode = 2;
    enemy->velocity = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    if (enemy->mirrored)
    {
        enemy->moveDelta.x = -enemy->moveDelta.x;
    }
}

void ScriptMoveLinear(Enemy *enemy, EnemyScriptInstr *instr, float t)
{
    float start;
    float end;

    start = GetFloatOperand(enemy, instr->args[4].f);
    end = GetFloatOperand(enemy, instr->args[5].f);
    *ResolveFloatOperand(enemy, &instr->args[8].f, 0, -1) = (end - start) * t + start;
}

void ScriptMoveHermite(Enemy *enemy, EnemyScriptInstr *instr, float t)
{
    float start;
    float hStart;
    float weightIn;
    float end;
    float slopeIn;
    float wEnd;
    float tangentOut;
    float outWeight;

    start = GetFloatOperand(enemy, instr->args[4].f);
    end = GetFloatOperand(enemy, instr->args[5].f);
    slopeIn = GetFloatOperand(enemy, instr->args[6].f);
    tangentOut = GetFloatOperand(enemy, instr->args[7].f);
    hStart = (t - 1.0f) * (t - 1.0f) * (2.0f * t + 1.0f);
    wEnd = t * t * (3.0f - 2.0f * t);
    weightIn = (1.0f - t) * (1.0f - t) * t;
    outWeight = (t - 1.0f) * t * t;
    *ResolveFloatOperand(enemy, &instr->args[8].f, 0, -1) =
        hStart * start + wEnd * end + weightIn * slopeIn + outWeight * tangentOut;
}

void StartSpellCard(Enemy *enemy, EnemyScriptInstr *instr)
{
    g_SpellCard.Start(*((WORD *)&instr->args[0] + 1), (char *)&instr->args[2], *(short *)&instr->args[0],
                      instr->args[1].i, enemy, (char *)&instr->args[14], (char *)&instr->args[26],
                      (char *)&instr->args[42]);
}

void EndSpellCard(Enemy *enemy, EnemyScriptInstr *instr)
{
    g_SpellCard.End();
}

void ScriptLerp(Enemy *enemy, EnemyScriptInstr *instr)
{
    float diff;

    diff = FLOAT_ARG(1) - FLOAT_ARG(2);
    *ResolveFloatOperand(enemy, &instr->args[0].f, instr->paramMask, 0) = diff * FLOAT_ARG(3) + FLOAT_ARG(2);
}

void ScriptStartInterp(Enemy *enemy, EnemyScriptInstr *instr)
{
    int i;
    EnemyScriptLoop *loop;

    for (loop = enemy->currentCtx->loops, i = 0; i < 8; i++, loop++)
    {
        if (loop->func != NULL && loop->target != instr->args[0].f)
        {
            continue;
        }
        loop->timer = 0;
        loop->target = instr->args[0].f;
        loop->duration = INT_ARG(1);
        loop->type = INT_ARG(2);
        loop->mode = INT_ARG(3);
        loop->func = g_EnemyScriptInterps[loop->type];
        loop->args[0] = FLOAT_ARG(4);
        loop->args[1] = FLOAT_ARG(5);
        loop->args[2] = FLOAT_ARG(6);
        loop->args[3] = FLOAT_ARG(7);
        break;
    }
}

EnemyScriptInstr *ScriptJumpIf(Enemy *enemy, EnemyScriptInstr *instr)
{
    switch (instr->opcode)
    {
    case 40:
        if (INT_ARG(0) == INT_ARG(1))
        {
            goto jump;
        }
        break;
    case 41:
        if (FLOAT_ARG(0) == FLOAT_ARG(1))
        {
            goto jump;
        }
        break;
    case 42:
        if (INT_ARG(0) != INT_ARG(1))
        {
            goto jump;
        }
        break;
    case 43:
        if (FLOAT_ARG(0) != FLOAT_ARG(1))
        {
            goto jump;
        }
        break;
    case 44:
        if (INT_ARG(0) < INT_ARG(1))
        {
            goto jump;
        }
        break;
    case 45:
        if (FLOAT_ARG(0) < FLOAT_ARG(1))
        {
            goto jump;
        }
        break;
    case 46:
        if (INT_ARG(0) <= INT_ARG(1))
        {
            goto jump;
        }
        break;
    case 47:
        if (FLOAT_ARG(0) <= FLOAT_ARG(1))
        {
            goto jump;
        }
        break;
    case 48:
        if (INT_ARG(0) > INT_ARG(1))
        {
            goto jump;
        }
        break;
    case 49:
        if (FLOAT_ARG(0) > FLOAT_ARG(1))
        {
            goto jump;
        }
        break;
    case 50:
        if (INT_ARG(0) >= INT_ARG(1))
        {
            goto jump;
        }
        break;
    case 51:
        if (FLOAT_ARG(0) >= FLOAT_ARG(1))
        {
            goto jump;
        }
        break;
    jump:
        enemy->currentCtx->timer.current = instr->args[2].i;
        return (EnemyScriptInstr *)((char *)instr + instr->args[3].i);
    }
    return NULL;
}

void ScriptCallSub(Enemy *enemy, EnemyScriptInstr *instr, int sub)
{
    enemy->currentCtx->code = (char *)instr + instr->size;
    if (!enemy->disableCallStack)
    {
        enemy->callStack[enemy->stackDepth] = *enemy->currentCtx;
    }
    g_EnemyScripts.StartSub(enemy->currentCtx, sub);
    enemy->currentCtx->vars.args = g_EnemyScripts.callArgs;
    if (!enemy->disableCallStack && enemy->stackDepth < 15)
    {
        enemy->stackDepth++;
    }
}

int ScriptReturn(Enemy *enemy, EnemyScriptInstr *instr)
{
    int i;

    if (enemy->disableCallStack)
    {
        DebugPrint("error : no Stack Ret\r\n");
    }
    enemy->stackDepth--;
    if (enemy->stackDepth < 0)
    {
        i = enemy->currentCtx->timedCallActive - 1;
        if (enemy->threads[i] != NULL)
        {
            g_MemoryManager.Free(enemy->threads[i]);
        }
        enemy->threads[i] = NULL;
        enemy->callStack = enemy->savedCtx;
        enemy->currentCtx = &enemy->ctx;
        enemy->stackDepth = enemy->savedStackDepth;
        return 1;
    }
    *enemy->currentCtx = enemy->callStack[enemy->stackDepth];
    return 0;
}

void DebugPrint(const char *fmt, ...)
{
}

void ScriptSetTurnSprites(Enemy *enemy, EnemyScriptInstr *instr, int still, int left, int right,
                          int stillFromLeft, int stillFromRight, int extra)
{
    enemy->turnStill = still;
    enemy->turnLeft = left;
    enemy->turnRight = right;
    enemy->turnStillFromLeft = stillFromLeft;
    enemy->turnStillFromRight = stillFromRight;
    enemy->turnExtra = extra;
    enemy->spriteDirection = 0xff;
}

void ScriptSetSubSprite(Enemy *enemy, EnemyScriptInstr *instr)
{
    if (INT_ARG(0) >= 2)
    {
        DebugPrint("error : sub anim overflow\r\n");
    }
    if (INT_ARG(1) >= 0)
    {
        if (enemy->useStageAnm)
        {
            g_EnemyManager.stageAnm->StartScript(&enemy->sprites[INT_ARG(0)], INT_ARG(1));
        }
        else
        {
            g_EnemyManager.anm->StartScript(&enemy->sprites[INT_ARG(0)], INT_ARG(1));
        }
    }
    else
    {
        enemy->sprites[INT_ARG(0)].scriptIndex = -1;
    }
}

void ScriptMoveRandom(Enemy *enemy, EnemyScriptInstr *instr)
{
    float angle;

    if (g_Player.pos.x < enemy->pos.x)
    {
        angle = AddNormalizeAngle(g_Rng.GetRandomF32InRange(1.5707964f) + 2.3561945f, 0.0f);
    }
    else
    {
        angle = g_Rng.GetRandomF32InRange(1.5707964f) - 0.7853982f;
    }
    if (enemy->pos[0] < enemy->lowerBound[0] + 96.0f)
    {
        if (angle > 1.5707964f)
        {
            angle = 3.1415927f - angle;
        }
        else if (angle < -1.5707964f)
        {
            angle = -3.1415927f - angle;
        }
    }
    if (enemy->pos[0] > enemy->upperBound[0] - 96.0f)
    {
        if (angle < 1.5707964f && angle >= 0.0f)
        {
            angle = 3.1415927f - enemy->angle;
        }
        else if (angle > -1.5707964f && angle <= 0.0f)
        {
            angle = -3.1415927f - angle;
        }
    }
    if (enemy->pos[1] < enemy->lowerBound[1] + 48.0f && angle < 0.0f)
    {
        angle = -angle;
    }
    if (enemy->pos[1] > enemy->upperBound[1] - 48.0f && angle > 0.0f)
    {
        angle = -angle;
    }
    if (INT_ARG(0) <= 0)
    {
        enemy->angle = angle;
        enemy->speed = FLOAT_ARG(2);
        enemy->moveMode = 1;
        enemy->moveDuration = 0;
        enemy->moveTimer = 0;
    }
    else
    {
        ScriptMoveAngleTimed(enemy, instr, angle);
    }
}

void ScriptMoveAngleTimed(Enemy *enemy, EnemyScriptInstr *instr, float angle)
{
    enemy->moveDelta.x = cosf(angle) * FLOAT_ARG(2) * INT_ARG(0);
    enemy->moveDelta.y = sinf(angle) * FLOAT_ARG(2) * INT_ARG(0);
    enemy->moveDelta.z = 0.0f;
    enemy->moveStart = enemy->absPos;
    enemy->moveTimer = (enemy->moveDuration = INT_ARG(0));
    enemy->moveEasing = INT_ARG(1);
    enemy->moveMode = 2;
}

void ScriptMoveRandom2(Enemy *enemy, EnemyScriptInstr *instr)
{
    float bound;
    float angle;

    if (g_Rng.GetRandomU32InRange(4))
    {
        if (g_Player.pos.x < enemy->pos.x)
        {
            bound = g_Player.pos.x + 384.0f;
            if (enemy->pos.x - g_Player.pos.x < bound - enemy->pos.x)
            {
                angle = AddNormalizeAngle(g_Rng.GetRandomF32InRange(1.5707964f) + 2.3561945f, 0.0f);
            }
            else
            {
                angle = AddNormalizeAngle(g_Rng.GetRandomF32InRange(1.5707964f) - 0.7853982f, 0.0f);
            }
        }
        else
        {
            bound = g_Player.pos.x - 384.0f;
            if (g_Player.pos.x - enemy->pos.x < enemy->pos.x - bound)
            {
                angle = g_Rng.GetRandomF32InRange(1.5707964f) - 0.7853982f;
            }
            else
            {
                angle = AddNormalizeAngle(g_Rng.GetRandomF32InRange(1.5707964f) + 2.3561945f, 0.0f);
            }
        }
    }
    else
    {
        angle = g_Rng.GetRandomF32SignedInRange(3.1415927f);
    }
    if (enemy->pos[1] < enemy->lowerBound[1] + 48.0f && angle < 0.0f)
    {
        angle = -angle;
    }
    if (enemy->pos[1] > enemy->upperBound[1] - 48.0f && angle > 0.0f)
    {
        angle = -angle;
    }
    if (INT_ARG(0) <= 0)
    {
        enemy->angle = angle;
        enemy->speed = FLOAT_ARG(2);
        enemy->moveMode = 1;
        enemy->moveDuration = 0;
        enemy->moveTimer = 0;
    }
    else
    {
        ScriptMoveAngleTimed(enemy, instr, angle);
    }
}

void ScriptFireBullets(Enemy *enemy, EnemyScriptInstr *instr)
{
    EnemyScriptArg *bulletArgs;
    int shortArg;
    BulletShooter *bs;

    bulletArgs = instr->args;
    bs = &enemy->shooter;
    if (((bulletArgs[7].i & 0x8000) && !enemy->isYoukai) || ((bulletArgs[7].i & 0x10000) && enemy->isYoukai))
    {
        return;
    }
    if (enemy->minShootDistSq > 0.0f &&
        (enemy->absPos.x - g_Player.pos.x) * (enemy->absPos.x - g_Player.pos.x) +
                (enemy->absPos.y - g_Player.pos.y) * (enemy->absPos.y - g_Player.pos.y) <
            enemy->minShootDistSq)
    {
        return;
    }
    bs->pos = enemy->absPos + enemy->shootOffset;
    shortArg = *(short *)&bulletArgs[0];
    bs->sprite = INT_OP(shortArg, 1);
    bs->aimMode = instr->opcode - 96;
    bs->count1 = INT_OP(bulletArgs[1].i, 4);
    bs->count2 = INT_OP(bulletArgs[2].i, 8);
    bs->angle1 = FLOAT_OP(bulletArgs[5].f, 0x40);
    bs->speed1 = FLOAT_OP(bulletArgs[3].f, 0x10);
    bs->angle2 = FLOAT_OP(bulletArgs[6].f, 0x80);
    bs->speed2 = FLOAT_OP(bulletArgs[4].f, 0x20);
    if (!g_SpellCard.IsActive())
    {
        bs->count1 += g_GameManager.RankLerp(enemy->rankCount1Low, enemy->rankCount1High);
        if (bs->count1 <= 0)
        {
            bs->count1 = 1;
        }
        bs->count2 += g_GameManager.RankLerp(enemy->rankCount2Low, enemy->rankCount2High);
        if (bs->count2 <= 0)
        {
            bs->count2 = 1;
        }
        if (bs->speed1 != 0.0f)
        {
            bs->speed1 += g_GameManager.RankLerpF(enemy->rankSpeedLow, enemy->rankSpeedHigh);
            if (bs->speed1 < 0.3f)
            {
                bs->speed1 = 0.3f;
            }
        }
        bs->speed2 += g_GameManager.RankLerpF(enemy->rankSpeedLow, enemy->rankSpeedHigh) / 2.0f;
        if (bs->speed2 < 0.3f)
        {
            bs->speed2 = 0.3f;
        }
    }
    bs->unusedAfterFireC2 = 0;
    bs->flags = bulletArgs[7].i;
    shortArg = ((short *)bulletArgs)[1];
    bs->color = INT_OP(shortArg, 2);
    g_BulletManager.FirePattern(bs);
}

void ScriptUpdateMovement(Enemy *enemy)
{
    switch (enemy->moveMode)
    {
    case 3:
    {
        // Unused.
        D3DXVECTOR3 unused;
        D3DXVECTOR3 polar;

        enemy->orbitAngle = AddNormalizeAngle(enemy->orbitAngle, g_App.gameSpeed * enemy->orbitAngularVelocity);
        enemy->orbitRadius = g_App.gameSpeed * enemy->orbitRadiusVelocity + enemy->orbitRadius;
        ((Vector2 *)&polar)->SetPolar(enemy->orbitAngle, enemy->orbitRadius);
        enemy->velocity.x = polar.x + enemy->moveStart.x - enemy->pos.x;
        enemy->velocity.y = polar.y + enemy->moveStart.y - enemy->pos.y;
        enemy->angle = atan2f(enemy->velocity.y, enemy->velocity.x);
        if (enemy->moveDuration > 0)
        {
            enemy->moveTimer--;
            if (enemy->moveTimer <= 0)
            {
                enemy->moveMode = 0;
            }
        }
        break;
    }
    case 1:
        enemy->angle = AddNormalizeAngle(enemy->angle, g_App.gameSpeed * enemy->angularVelocity);
        enemy->speed = g_App.gameSpeed * enemy->acceleration + enemy->speed;
        ((Vector2 *)&enemy->velocity)->SetPolar(enemy->angle, enemy->speed);
        enemy->velocity[2] = 0.0f;
        if (enemy->moveDuration > 0)
        {
            enemy->moveTimer--;
            if (enemy->moveTimer <= 0)
            {
                enemy->moveMode = 0;
            }
        }
        break;
    case 2:
    {
        float t;

        enemy->moveTimer--;
        t = 1.0f - enemy->moveTimer.AsFloat() / enemy->moveDuration;
        if (t < 0.0f)
        {
            t = 0.0f;
        }
        switch (enemy->moveEasing)
        {
        case 1:
            t = t * t;
            break;
        case 2:
            t = t * t * t;
            break;
        case 3:
            t = t * t * t * t;
            break;
        case 4:
            t = (t = 1.0f - t) * t;
            t = 1.0f - t;
            break;
        case 5:
            t = (t = 1.0f - t) * t * t;
            t = 1.0f - t;
            break;
        case 6:
            t = (t = 1.0f - t) * t * t * t;
            t = 1.0f - t;
            break;
        }
        enemy->velocity = enemy->moveStart + enemy->moveDelta * t - enemy->pos;
        if (enemy->mirrored)
        {
            enemy->velocity.x = -enemy->velocity.x;
        }
        enemy->angle = atan2f(enemy->velocity.y, enemy->velocity.x);
        if (enemy->moveTimer <= 0)
        {
            enemy->moveMode = 0;
            enemy->pos = enemy->moveStart + enemy->moveDelta;
            enemy->velocity = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        }
        break;
    }
    }
}

void ScriptUpdateShooting(Enemy *enemy)
{
    if (enemy->life > 0)
    {
        if (enemy->shootInterval > 0)
        {
            enemy->shootTimer++;
            if (enemy->shootTimer >= enemy->shootInterval)
            {
                ScriptFireBullets(enemy, (EnemyScriptInstr *)enemy->shootInstr);
                enemy->shootTimer = 0;
            }
        }
        if (enemy->turnLeft >= 0)
        {
            int dir;

            dir = 0;
            if (!enemy->mirrored)
            {
                if (enemy->velocity.x < -0.01f)
                {
                    dir = 1;
                }
                else if (enemy->velocity.x > 0.01f)
                {
                    dir = 2;
                }
            }
            else
            {
                if (enemy->velocity.x < -0.01f)
                {
                    dir = 2;
                }
                else if (enemy->velocity.x > 0.01f)
                {
                    dir = 1;
                }
            }
            if (enemy->spriteDirection != dir)
            {
                AnmLoaded *anm = enemy->useStageAnm ? g_EnemyManager.stageAnm : g_EnemyManager.anm;

                switch (dir)
                {
                case 0:
                    if (enemy->spriteDirection == 0xff)
                    {
                        anm->StartScript(&enemy->sprite, enemy->turnStill);
                    }
                    else if (enemy->spriteDirection == 1)
                    {
                        anm->StartScript(&enemy->sprite, enemy->turnStillFromLeft);
                    }
                    else
                    {
                        anm->StartScript(&enemy->sprite, enemy->turnStillFromRight);
                    }
                    break;
                case 1:
                    anm->StartScript(&enemy->sprite, enemy->turnLeft);
                    break;
                case 2:
                    anm->StartScript(&enemy->sprite, enemy->turnRight);
                    break;
                }
                enemy->spriteDirection = dir;
            }
        }
    }
}

void SetScriptFunc0Vars(Enemy *enemy, EnemyScriptInstr *instr)
{
    g_AsciiText.ringAlpha = enemy->currentCtx->vars.ints[0];
    g_AsciiText.ringRadius = enemy->currentCtx->vars.floats[0];
}

void FlashScreen60(Enemy *enemy, EnemyScriptInstr *instr)
{
    ScreenEffect::StartEffect(3, 60, 1, 0xffffffff, 0, 0x15);
}

void BounceOffSides(Enemy *enemy, EnemyScriptInstr *instr)
{
    int bounced = 0;

    if (enemy->pos.x <= 0.0f || enemy->pos.x >= 384.0f)
    {
        enemy->velocity.x = -enemy->velocity.x;
        bounced = 1;
    }
    if (enemy->velocity.y < enemy->currentCtx->vars.floats[7])
    {
        enemy->velocity.y += enemy->currentCtx->vars.floats[6];
        bounced = 1;
    }
    if (enemy->pos.y < -64.0f)
    {
        enemy->velocity.y = -enemy->velocity.y;
        bounced = 1;
    }
    else if (enemy->pos.y >= 480.0f)
    {
        enemy->staysOffscreen = 0;
    }
    if (bounced)
    {
        enemy->angle = atan2f(enemy->velocity.y, enemy->velocity.x);
    }
}

void SpawnTwinEffects38(Enemy *enemy, EnemyScriptInstr *instr)
{
    Effect *effect;

    effect = g_EffectManager.SpawnReservedEffect(0x38, &enemy->pos, 9, 1, 0xffffffff);
    effect = g_EffectManager.SpawnReservedEffect(0x38, &enemy->pos, 10, 1, 0xffffffff);
    g_EffectManager.anm->StartScript(&effect->sprite, 0x61);
    g_Stage.spellCardDrawCallback = DrawTwinEffects;
}

void DrawTwinEffects()
{
    Effect *first;
    D3DCOLOR saved;
    float r1;
    int i;
    Effect *second;
    Sprite *spr;
    float r2;
    VertexDiffuseXyzrwh vertices[10];

    first = g_EffectManager.GetReservedEffect(9);
    second = g_EffectManager.GetReservedEffect(10);
    spr = &g_Stage.dialogueSprites[0];
    r1 = first->sprite.pos.x * 0.70710683f;
    r2 = second->sprite.pos.x * 0.70710683f;
    vertices[0].pos.x = 32.0f + first->pos.x - r1;
    vertices[0].pos.y = 16.0f + first->pos.y - r1;
    vertices[1].pos.x = 32.0f + first->pos.x - r2;
    vertices[1].pos.y = 16.0f + first->pos.y - r2;
    vertices[2].pos.x = 32.0f + first->pos.x + r1;
    vertices[2].pos.y = 16.0f + first->pos.y - r1;
    vertices[3].pos.x = 32.0f + first->pos.x + r2;
    vertices[3].pos.y = 16.0f + first->pos.y - r2;
    vertices[4].pos.x = 32.0f + first->pos.x + r1;
    vertices[4].pos.y = 16.0f + first->pos.y + r1;
    vertices[5].pos.x = 32.0f + first->pos.x + r2;
    vertices[5].pos.y = 16.0f + first->pos.y + r2;
    vertices[6].pos.x = 32.0f + first->pos.x - r1;
    vertices[6].pos.y = 16.0f + first->pos.y + r1;
    vertices[7].pos.x = 32.0f + first->pos.x - r2;
    vertices[7].pos.y = 16.0f + first->pos.y + r2;
    vertices[8].pos = vertices[0].pos;
    vertices[9].pos = vertices[1].pos;
    for (i = 0; i < 10; i++)
    {
        vertices[i].pos.z = 0.8f;
        vertices[i].w = 1.0f;
        vertices[i].diffuse = 0xff000000;
    }
    g_App.d3dDevice->SetVertexShader(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
    g_App.SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
    g_App.d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 8, vertices, sizeof(VertexDiffuseXyzrwh));
    g_SpriteEngine->ClearVertexFormat();
    g_SpriteEngine->ClearColorOp();
    g_SpriteEngine->ClearBlendMode();
    g_SpriteEngine->ClearZWrite();
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    g_App.d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    g_App.SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
    g_Stage.dialogueSprites[0].scaleY = -1.5f;
    g_Stage.dialogueSprites[0].scaleX = -1.75f;
    g_Stage.dialogueSprites[0].pos.z = 0.7f;
    g_Stage.dialogueSprites[0].pos.x = 416.0f;
    g_Stage.dialogueSprites[0].pos.y = 464.0f;
    saved = g_Stage.dialogueSprites[0].color;
    g_Stage.dialogueSprites[0].color = 0xffe0c0c0;
    g_SpriteEngine->Draw(&g_Stage.dialogueSprites[0]);
    g_Stage.dialogueSprites[0].scaleY = 1.5f;
    g_Stage.dialogueSprites[0].scaleX = 1.75f;
    g_Stage.dialogueSprites[0].pos.z = 0.5f;
    g_Stage.dialogueSprites[0].pos.x = 32.0f;
    g_Stage.dialogueSprites[0].pos.y = 16.0f;
    g_Stage.dialogueSprites[0].color = saved;
    g_Stage.dialogueSprites[1].rotation.z = g_Stage.dialogueSprites[1].rotation.z * -1.0f;
    g_Stage.dialogueSprites[1].pos.z = 0.6f;
    saved = g_Stage.dialogueSprites[1].color;
    g_Stage.dialogueSprites[1].color = 0xffe0c0c0;
    g_SpriteEngine->DrawAndFlush(&g_Stage.dialogueSprites[1]);
    g_Stage.dialogueSprites[1].rotation.z = g_Stage.dialogueSprites[1].rotation.z * -1.0f;
    g_Stage.dialogueSprites[1].pos.z = 0.5f;
    g_Stage.dialogueSprites[1].color = saved;
    g_App.SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
}

void BounceBulletsAtZones(Enemy *enemy, EnemyScriptInstr *instr)
{
    Bullet *bullet = g_BulletManager.bullets;
    D3DXVECTOR3 posPrev;
    int i;
    int prevZone;
    int zone;

    for (i = 0; i < 0x600; i++, bullet++)
    {
        if (bullet->state == 0)
        {
            continue;
        }
        if (bullet->scriptFuncState != 0)
        {
            bullet->scriptFuncState--;
            continue;
        }
        posPrev = bullet->pos - bullet->velocity;
        if (bullet->pos.x > 124.117744f && bullet->pos.x < 259.88226f && bullet->pos.y > 140.11774f && bullet->pos.y < 275.88226f)
        {
            zone = 0;
        }
        else if (bullet->pos.x > 56.23549f && bullet->pos.x < 327.76453f && bullet->pos.y > 72.23549f && bullet->pos.y < 343.76453f)
        {
            zone = 1;
        }
        else
        {
            zone = 2;
        }
        if (posPrev.x > 124.117744f && posPrev.x < 259.88226f && posPrev.y > 140.11774f && posPrev.y < 275.88226f)
        {
            prevZone = 0;
        }
        else if (posPrev.x > 56.23549f && posPrev.x < 327.76453f && posPrev.y > 72.23549f && posPrev.y < 343.76453f)
        {
            prevZone = 1;
        }
        else
        {
            prevZone = 2;
        }
        if (zone != prevZone)
        {
            bullet->scriptFuncState = 2;
            bullet->velocity *= -1.0f;
            if (zone == 0 || prevZone == 0)
            {
                bullet->pos.x = (bullet->pos.x - 192.0f) * 135.76451f / 67.882256f + 192.0f;
                bullet->pos.y = (bullet->pos.y - 208.0f) * 135.76451f / 67.882256f + 208.0f;
            }
            else
            {
                bullet->pos.x = (bullet->pos.x - 192.0f) * 67.882256f / 135.76451f + 192.0f;
                bullet->pos.y = (bullet->pos.y - 208.0f) * 67.882256f / 135.76451f + 208.0f;
            }
            bullet->angle = AddNormalizeAngle(bullet->angle, 3.1415927f);
        }
    }
}

void SpawnTwinEffects41(Enemy *enemy, EnemyScriptInstr *instr)
{
    Effect *effect;

    effect = g_EffectManager.SpawnReservedEffect(0x41, &enemy->pos, 9, 1, 0xffffffff);
    effect = g_EffectManager.SpawnReservedEffect(0x41, &enemy->pos, 10, 1, 0xffffffff);
    g_EffectManager.anm->StartScript(&effect->sprite, 0x63);
    g_Stage.spellCardDrawCallback = DrawTwinEffects;
}

void BounceBulletsAtZones2(Enemy *enemy, EnemyScriptInstr *instr)
{
    Bullet *bullet = g_BulletManager.bullets;
    D3DXVECTOR3 posPrev;
    int i;
    int prevZone;
    int zone;

    for (i = 0; i < 0x600; i++, bullet++)
    {
        if (bullet->state == 0)
        {
            continue;
        }
        if (bullet->scriptFuncState != 0)
        {
            bullet->scriptFuncState--;
            continue;
        }
        posPrev = bullet->pos - bullet->velocity;
        if (bullet->pos.x > 112.80403f && bullet->pos.x < 271.19595f && bullet->pos.y > 128.80403f && bullet->pos.y < 287.19595f)
        {
            zone = 0;
        }
        else if (bullet->pos.x > 33.60807f && bullet->pos.x < 350.39194f && bullet->pos.y > 49.60807f && bullet->pos.y < 366.39194f)
        {
            zone = 1;
        }
        else
        {
            zone = 2;
        }
        if (posPrev.x > 112.80403f && posPrev.x < 271.19595f && posPrev.y > 128.80403f && posPrev.y < 287.19595f)
        {
            prevZone = 0;
        }
        else if (posPrev.x > 33.60807f && posPrev.x < 350.39194f && posPrev.y > 49.60807f && posPrev.y < 366.39194f)
        {
            prevZone = 1;
        }
        else
        {
            prevZone = 2;
        }
        if (zone != prevZone)
        {
            bullet->scriptFuncState = 2;
            bullet->velocity *= -1.0f;
            if (zone == 0 || prevZone == 0)
            {
                bullet->pos.x = (bullet->pos.x - 192.0f) * 158.39194f / 79.19597f + 192.0f;
                bullet->pos.y = (bullet->pos.y - 208.0f) * 158.39194f / 79.19597f + 208.0f;
            }
            else
            {
                bullet->pos.x = (bullet->pos.x - 192.0f) * 79.19597f / 158.39194f + 192.0f;
                bullet->pos.y = (bullet->pos.y - 208.0f) * 79.19597f / 158.39194f + 208.0f;
            }
            bullet->angle = AddNormalizeAngle(bullet->angle, 3.1415927f);
        }
    }
}

void EndTwinEffects(Enemy *enemy, EnemyScriptInstr *instr)
{
    g_EffectManager.GetReservedEffect(9)->active = 0;
    g_EffectManager.GetReservedEffect(10)->active = 0;
    g_Stage.dialogueSpriteCount = 2;
}

void SpawnTwinEffects3A(Enemy *enemy, EnemyScriptInstr *instr)
{
    Effect *effect;

    effect = g_EffectManager.SpawnReservedEffect(0x3a, &enemy->pos, 9, 1, 0xffffffff);
    effect = g_EffectManager.SpawnReservedEffect(0x3a, &enemy->pos, 10, 1, 0xffffffff);
    g_EffectManager.anm->StartScript(&effect->sprite, 0x65);
    g_Stage.spellCardDrawCallback = DrawTwinEffects;
}

void BounceBulletsAtZones3(Enemy *enemy, EnemyScriptInstr *instr)
{
    Bullet *bullet = g_BulletManager.bullets;
    D3DXVECTOR3 posPrev;
    int i;
    int prevZone;
    int zone;

    for (i = 0; i < 0x600; i++, bullet++)
    {
        if (bullet->state == 0)
        {
            continue;
        }
        if (bullet->scriptFuncState != 0)
        {
            bullet->scriptFuncState--;
            continue;
        }
        posPrev = bullet->pos - bullet->velocity;
        if (bullet->pos.x > 56.23549f && bullet->pos.x < 327.76453f && bullet->pos.y > 88.23549f && bullet->pos.y < 359.76453f)
        {
            zone = 0;
        }
        else if (bullet->pos.x > -32.0f && bullet->pos.x < 416.0f && bullet->pos.y > 0.0f && bullet->pos.y < 448.0f)
        {
            zone = 1;
        }
        else
        {
            zone = 2;
        }
        if (posPrev.x > 56.23549f && posPrev.x < 327.76453f && posPrev.y > 88.23549f && posPrev.y < 359.76453f)
        {
            prevZone = 0;
        }
        else if (posPrev.x > -31.100006f && posPrev.x < 416.0f && posPrev.y > 0.0f && posPrev.y < 448.0f)
        {
            prevZone = 1;
        }
        else
        {
            prevZone = 2;
        }
        if (zone != prevZone)
        {
            bullet->scriptFuncState = 2;
            bullet->velocity *= -1.0f;
            if (zone == 0 || prevZone == 0)
            {
                bullet->pos.x = (bullet->pos.x - 192.0f) * 224.0f / 135.76451f + 192.0f;
                bullet->pos.y = (bullet->pos.y - 224.0f) * 224.0f / 135.76451f + 224.0f;
            }
            else
            {
                bullet->pos.x = (bullet->pos.x - 192.0f) * 135.76451f / 224.0f + 192.0f;
                bullet->pos.y = (bullet->pos.y - 224.0f) * 135.76451f / 224.0f + 224.0f;
            }
            bullet->angle = AddNormalizeAngle(bullet->angle, 3.1415927f);
        }
    }
}

void KeepFormation(Enemy *enemy, EnemyScriptInstr *instr)
{
    int count;
    int key;
    Enemy *leader;
    Enemy *member;
    float targ;
    float diff;

    key = enemy->currentCtx->vars.ints2[2];
    member = enemy->leader;
    if (member == NULL)
    {
        return;
    }
    count = 0;
    while (member->nextFollower != NULL)
    {
        member = member->nextFollower;
        if (member->currentCtx->vars.ints2[2] == key)
        {
            member->currentCtx->vars.ints2[1] = count;
            if (count == 0)
            {
                leader = member;
            }
            count++;
        }
    }
    enemy->currentCtx->vars.ints[5] = 0;
    if (enemy->currentCtx->vars.ints[6] != count)
    {
        if (enemy->currentCtx->vars.ints[6] != 0)
        {
            enemy->currentCtx->vars.ints[5] = 1;
        }
        enemy->currentCtx->vars.ints[6] = count;
    }
    key = enemy->currentCtx->vars.ints2[1];
    enemy->currentCtx->vars.ints[7]++;
    if (key != 0)
    {
        targ = AddNormalizeAngle(leader->orbitAngle, key * 6.2831855f / count);
        if (leader->currentCtx->vars.ints[7] != enemy->currentCtx->vars.ints[7])
        {
            targ = AddNormalizeAngle(targ, leader->orbitAngularVelocity);
        }
        diff = AddNormalizeAngle(enemy->orbitAngle, enemy->orbitAngularVelocity);
        diff = targ - diff;
        if (fabsf(diff) > 3.1415927f)
        {
            diff = diff > 0.0f ? -6.2831855f + diff : 6.2831855f + diff;
        }
        diff = diff * 0.02f;
        enemy->orbitAngle = AddNormalizeAngle(enemy->orbitAngle, diff);
    }
}

void FlashAndShakeScreen(Enemy *enemy, EnemyScriptInstr *instr)
{
    ScreenEffect::StartEffect(3, 30, 5, 0x40ffffff, 0, 0x15);
    ScreenEffect::StartEffect(7, 4, 120, 190, 60, 0x15);
}

void BeamHitsPlayer160(Enemy *enemy, EnemyScriptInstr *instr)
{
    D3DXVECTOR3 origin(enemy->absPos.x - enemy->currentCtx->vars.floats[0], enemy->absPos.y - enemy->currentCtx->vars.floats[1], 0.0f);
    D3DXVECTOR3 size(590.0f, 160.0f, 0.0f);
    D3DXVECTOR3 size2(590.0f, 128.0f, 0.0f);
    D3DXVECTOR3 center(size.x / 2.0f + origin.x, origin.y, 0.0f);

    if (enemy->callbackTimer.HasTickedEvery(12))
    {
        g_Player.CheckLaserHit(&center, &size2, &origin, enemy->sprite.rotation.z, 1);
    }
    g_Player.CheckLaserHit(&center, &size, &origin, enemy->sprite.rotation.z, 0);
}

void BeamHitsPlayer240(Enemy *enemy, EnemyScriptInstr *instr)
{
    D3DXVECTOR3 origin(enemy->pos.x - enemy->currentCtx->vars.floats[0], enemy->pos.y - enemy->currentCtx->vars.floats[1], 0.0f);
    D3DXVECTOR3 size(590.0f, 240.0f, 0.0f);
    D3DXVECTOR3 size2(590.0f, 192.0f, 0.0f);
    D3DXVECTOR3 center(size.x / 2.0f + origin.x, origin.y, 0.0f);

    if (enemy->callbackTimer.HasTickedEvery(12))
    {
        g_Player.CheckLaserHit(&center, &size2, &origin, enemy->sprite.rotation.z, 1);
    }
    g_Player.CheckLaserHit(&center, &size, &origin, enemy->sprite.rotation.z, 0);
}

void BeamHitsPlayer288(Enemy *enemy, EnemyScriptInstr *instr)
{
    D3DXVECTOR3 origin(enemy->absPos.x - enemy->currentCtx->vars.floats[0], enemy->absPos.y - enemy->currentCtx->vars.floats[1], 0.0f);
    D3DXVECTOR3 size(590.0f, 288.0f, 0.0f);
    D3DXVECTOR3 size2(590.0f, 224.0f, 0.0f);
    D3DXVECTOR3 center(size.x / 2.0f + origin.x, origin.y, 0.0f);

    if (enemy->callbackTimer.HasTickedEvery(12))
    {
        g_Player.CheckLaserHit(&center, &size2, &origin, enemy->sprite.rotation.z, 1);
    }
    g_Player.CheckLaserHit(&center, &size, &origin, enemy->sprite.rotation.z, 0);
}

void SetScreenEffectHoldFrames(Enemy *enemy, EnemyScriptInstr *instr)
{
    g_ScreenEffectHoldFrames = instr->args[1].i;
}

void ToggleMarkedBullets(Enemy *enemy, EnemyScriptInstr *instr)
{
    Bullet *bullet = g_BulletManager.bullets;
    int i;

    for (i = 0; i < 0x600; i++, bullet++)
    {
        if (bullet->state == 0)
        {
            continue;
        }
        if (bullet->flags & enemy->currentCtx->vars.ints[0])
        {
            if (bullet->sprites.sprite0.autoRotate == 1)
            {
                bullet->sprites.sprite0.autoRotate = 0;
                bullet->sprites.sprite0.blendMode = 1;
                g_BulletManager.etamaAnm->SetSprite(&bullet->sprites.sprite0, bullet->sprites.sprite0.spriteIndex + 0x10);
                bullet->harmless = 1;
                ((Vector2 *)&bullet->velocity)
                    ->SetPolar(enemy->currentCtx->vars.floats[0], g_App.gameSpeed * enemy->currentCtx->vars.floats[1]);
            }
            else
            {
                bullet->sprites.sprite0.autoRotate = 1;
                bullet->sprites.sprite0.blendMode = 0;
                g_BulletManager.etamaAnm->SetSprite(&bullet->sprites.sprite0, bullet->sprites.sprite0.spriteIndex - 0x10);
                bullet->harmless = 0;
                ((Vector2 *)&bullet->velocity)->SetPolar(bullet->angle, g_App.gameSpeed * bullet->speed);
            }
        }
    }
    if (enemy->currentCtx->vars.ints[1] == 0)
    {
        Enemy *follower = enemy;

        while (follower->nextFollower != NULL)
        {
            follower = follower->nextFollower;
            follower->halted = 1;
        }
        g_Stage.dialogueSprites[0].SetPendingInterrupt(2);
        g_Stage.dialogueSprites[1].SetPendingInterrupt(2);
    }
    else
    {
        Enemy *follower = enemy;

        while (follower->nextFollower != NULL)
        {
            follower = follower->nextFollower;
            follower->halted = 0;
        }
        g_Stage.dialogueSprites[0].SetPendingInterrupt(1);
        g_Stage.dialogueSprites[1].SetPendingInterrupt(1);
    }
}

void CycleMarkedBullets(Enemy *enemy, EnemyScriptInstr *instr)
{
    Bullet *bullet = g_BulletManager.bullets;
    int i;

    for (i = 0; i < 0x600; i++, bullet++)
    {
        if (bullet->state == 0)
        {
            continue;
        }
        if (bullet->flags & enemy->currentCtx->vars.ints[0])
        {
            if (bullet->sprites.sprite0.autoRotate == 1)
            {
                bullet->sprites.sprite0.autoRotate = 0;
                bullet->sprites.sprite0.blendMode = 1;
                bullet->sprites.sprite0.alpha = 0;
                g_BulletManager.etamaAnm->SetSprite(&bullet->sprites.sprite0, bullet->sprites.sprite0.spriteIndex + 0x10);
                bullet->harmless = 1;
                ((Vector2 *)&bullet->velocity)
                    ->SetPolar(bullet->angle, g_App.gameSpeed * enemy->currentCtx->vars.floats[1]);
            }
            else if (bullet->sprites.sprite0.autoRotate == 0)
            {
                bullet->sprites.sprite0.autoRotate = 2;
                bullet->sprites.sprite0.alpha = 0;
                bullet->sprites.sprite0.SetAlphaInterp(15, 0, 0, 0xff);
            }
            else
            {
                bullet->sprites.sprite0.autoRotate = 1;
                bullet->sprites.sprite0.blendMode = 0;
                g_BulletManager.etamaAnm->SetSprite(&bullet->sprites.sprite0, bullet->sprites.sprite0.spriteIndex - 0x10);
                bullet->harmless = 0;
                ((Vector2 *)&bullet->velocity)->SetPolar(bullet->angle, g_App.gameSpeed * bullet->speed);
            }
        }
    }
}

void TintStageRed(Enemy *enemy, EnemyScriptInstr *instr)
{
    g_Stage.SetTintColor(0xffc03030);
}

void ShakeScreenRamp(Enemy *enemy, EnemyScriptInstr *instr)
{
    ScreenEffect::StartEffect(7, 16, 20, 20, 20, 0x15);
}

void MarkFollowersNearBullets(Enemy *enemy, EnemyScriptInstr *instr)
{
    Bullet *bullet = g_BulletManager.bullets;
    D3DXVECTOR3 diff;
    int i;
    Enemy *member;

    for (i = 0; i < 0x600; i++, bullet++)
    {
        if (bullet->state == 0)
        {
            continue;
        }
        if (bullet->flags & 0x100000)
        {
            member = enemy->nextFollower;
            while (member != NULL)
            {
                if (member->currentCtx->vars.ints2[2] == 0)
                {
                    diff = bullet->pos - member->pos;
                    if (D3DXVec3LengthSq(&diff) < 4096.0f)
                    {
                        member->currentCtx->vars.ints2[2] = 60;
                        member->currentCtx->vars.ints[7] = enemy->currentCtx->vars.ints[7];
                    }
                }
                member = member->nextFollower;
            }
        }
    }
}

void FlashScreen180(Enemy *enemy, EnemyScriptInstr *instr)
{
    ScreenEffect::StartEffect(3, 180, 1, 0xffffffff, 0, 0x15);
}

void SetGameSpeed(Enemy *enemy, EnemyScriptInstr *instr)
{
    int divisor;
    float newSpeed;

    divisor = instr->args[1].i;
    newSpeed = 1.0f / divisor;
    g_App.gameSpeed = newSpeed;
}

void GetPracticeCard(Enemy *enemy, EnemyScriptInstr *instr)
{
    enemy->currentCtx->vars.ints[0] = g_GameManager.practiceSpellCard;
}

void StartResurrection(Enemy *enemy, EnemyScriptInstr *instr)
{
    g_SpellCard.StartNameOnly("「リザレクション」", enemy);
}

void ScriptEndSpellCardName(Enemy *enemy, EnemyScriptInstr *instr)
{
    g_SpellCard.EndSpellCardName();
}

void GetSpellCardsCaptured(Enemy *enemy, EnemyScriptInstr *instr)
{
    enemy->currentCtx->vars.ints[0] = g_GameManager.stats->spellCardsCaptured;
}

void SetFrozen(Enemy *enemy, EnemyScriptInstr *instr)
{
    g_GameManager.isFrozen = instr->args[1].i;
    if (g_GameManager.isFrozen)
    {
        g_Stage.dialogueSprites[0].SetPendingInterrupt(2);
        g_Stage.dialogueSprites[1].SetPendingInterrupt(2);
    }
    else
    {
        g_Stage.dialogueSprites[0].SetPendingInterrupt(1);
        g_Stage.dialogueSprites[1].SetPendingInterrupt(1);
    }
}

void SpawnEnemiesAtMarkedBullets(Enemy *enemy, EnemyScriptInstr *instr)
{
    Bullet *bullet = g_BulletManager.bullets;
    // Unused.
    D3DXVECTOR3 pos;
    int i;

    for (i = 0; i < 0x600; i++, bullet++)
    {
        if (bullet->state == 0)
        {
            continue;
        }
        if (bullet->flags & 0x100000)
        {
            enemy->currentCtx->vars.floats[0] = bullet->angle;
            g_EnemyManager.SpawnWithVars(enemy->currentCtx->vars.ints2[2], &bullet->pos, 800, -2, 10,
                                         &enemy->currentCtx->vars);
            bullet->flags &= ~0x100000;
        }
    }
}

void StartSlowdown(Enemy *enemy, EnemyScriptInstr *instr)
{
    Bullet *bullet;
    int i;

    g_App.gameSpeed = 1.0f / instr->args[1].i;
    g_Stage.dialogueSprites[0].SetPendingInterrupt(2);
    g_Stage.dialogueSprites[1].SetPendingInterrupt(2);
    bullet = g_BulletManager.bullets;
    for (i = 0; i < 0x600; i++, bullet++)
    {
        if (bullet->state == 0)
        {
            continue;
        }
        bullet->velocity *= g_App.gameSpeed;
        bullet->sprites.sprite0.baseSpriteIndex = bullet->sprites.sprite0.spriteIndex;
        if (bullet->sprites.sprite0.spriteIndex >= 0x60 && bullet->sprites.sprite0.spriteIndex <= 0x6f)
        {
            g_BulletManager.etamaAnm->SetSprite(&bullet->sprites.sprite0, 0x6f);
        }
    }
}

void EndSlowdown(Enemy *enemy, EnemyScriptInstr *instr)
{
    Bullet *bullet = g_BulletManager.bullets;
    float scale = 1.0f / g_App.gameSpeed;
    int i;

    for (i = 0; i < 0x600; i++, bullet++)
    {
        if (bullet->state == 0)
        {
            continue;
        }
        bullet->velocity *= scale;
        if (bullet->sprites.sprite0.spriteIndex >= 0x60 && bullet->sprites.sprite0.spriteIndex <= 0x6f)
        {
            g_BulletManager.etamaAnm->SetSprite(&bullet->sprites.sprite0, bullet->sprites.sprite0.baseSpriteIndex);
        }
    }
    g_App.gameSpeed = 1.0f / instr->args[1].i;
    if (g_App.gameSpeed < 1.0f)
    {
        g_App.stateFlags |= 0x20;
    }
    g_App.gameSpeed = 1.0f;
    g_Stage.dialogueSprites[0].SetPendingInterrupt(1);
    g_Stage.dialogueSprites[1].SetPendingInterrupt(1);
}

void DropBombOrLifeItem(Enemy *enemy, EnemyScriptInstr *instr)
{
    if (g_Player.bomb.running)
    {
        g_ItemManager.SpawnItem(&enemy->pos, 3, 0);
    }
    else
    {
        g_ItemManager.SpawnItem(&enemy->pos, 5, 0);
    }
}
