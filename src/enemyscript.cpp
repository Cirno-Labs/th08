// The enemy scripts (ECL), from 0x418330, as th07's enemyscript.cpp.

#include <math.h>
#include <string.h>

#include "bullet.h"
#include "effect.h"
#include "enemy.h"
#include "item.h"
#include "player.h"
#include "screeneffect.h"
#include "unsorted.h"

// Argument `n` of `instr`, or the value of the variable it names, as
// th07's.
#define INT_ARG(n) \
    ((instr->paramMask & (1 << (n))) ? GetIntOperand(enemy, instr->args[n].i) : instr->args[n].i)
#define FLOAT_ARG(n) \
    ((instr->paramMask & (1 << (n))) ? GetFloatOperand(enemy, instr->args[n].f) : instr->args[n].f)

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
        return enemy->unk_2d88.x;
    case 10043:
        return enemy->unk_2d88.y;
    case 10044:
        return enemy->unk_2d88.z;
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
        return enemy->unk_2db0;
    case 10077:
        return enemy->unk_2d9c;
    case 10078:
        return enemy->unk_2da0;
    case 10083:
        return enemy->lastDamage;
    case 10084:
        return enemy->bossIndex;
    case 10092:
        return enemy->item;
    case 10093:
        return enemy->score;
    case 10096:
        return enemy->Func41f000() ? enemy->Func41fd40() : (enemy->Func41fd20() ? enemy->unk_2da4->Func41fd40() : 0);
    case 10048:
        return g_Player.AngleFrom(&enemy->unk_2d88);
    case 10050:
    {
        D3DXVECTOR3 diff = g_Player.pos - enemy->unk_2d88;

        return D3DXVec3Length(&diff);
    }
    case 10097:
        return g_Player.IsUnk5Set();
    case 10098:
        return g_GameManager.GetUnk3c() + g_SpellCard.GetUnk10() + g_ItemManager.CountType7Items() < g_GameManager.GetUnk40() ? 0 : 2;
    case 10099:
        return g_SpellCard.Func4178a0() ? g_SpellCard.Func41fd90() : g_SpellCard.Func405260();
    case 10100:
        return g_SpellCard.Func41fdd0();
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
        return enemy->unk_2d88.x;
    case 10043:
        return enemy->unk_2d88.y;
    case 10044:
        return enemy->unk_2d88.z;
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
        return g_Player.AngleFrom(&enemy->unk_2d88);
    case 10069:
        return enemy->angle;
    case 10070:
        return enemy->angularVelocity;
    case 10071:
        return enemy->speed;
    case 10072:
        return enemy->acceleration;
    case 10073:
        return enemy->unk_2db0;
    case 10077:
        return enemy->unk_2d9c;
    case 10078:
        return enemy->unk_2da0;
    case 10084:
        return enemy->bossIndex;
    case 10083:
        return enemy->lastDamage;
    case 10096:
        return enemy->Func41f000() ? enemy->Func41fd40() : (enemy->Func41fd20() ? enemy->unk_2da4->Func41fd40() : 0);
    case 10050:
    {
        D3DXVECTOR3 diff = g_Player.pos - enemy->unk_2d88;

        return D3DXVec3Length(&diff);
    }
    case 10097:
        return g_Player.IsUnk5Set();
    case 10099:
        return g_SpellCard.Func4178a0() ? g_SpellCard.Func41fd90() : g_SpellCard.Func405260();
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
        return &enemy->unk_2db0;
    case 10077:
        return &enemy->unk_2d9c;
    case 10078:
        return &enemy->unk_2da0;
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
    enemy->moveStart = enemy->unk_2d88;
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
    enemy->moveDelta = target - enemy->unk_2d88;
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

void ScriptCallSub(Enemy *enemy, EnemyScriptInstr *instr, short sub)
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
        if (enemy->unk_3384[i] != NULL)
        {
            g_MemoryManager.Free(enemy->unk_3384[i]);
        }
        enemy->unk_3384[i] = NULL;
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

void ScriptSetTurnSprites(Enemy *enemy, EnemyScriptInstr *instr, short still, short left, short right,
                          short stillFromLeft, short stillFromRight, short unk)
{
    enemy->turnStill = still;
    enemy->turnLeft = left;
    enemy->turnRight = right;
    enemy->turnStillFromLeft = stillFromLeft;
    enemy->turnStillFromRight = stillFromRight;
    enemy->unk_333c = unk;
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
    enemy->moveStart = enemy->unk_2d88;
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

void ScriptSetUnknown4e3d28(Enemy *enemy, EnemyScriptInstr *instr)
{
    g_Unknown4e3d28 = enemy->currentCtx->vars.ints[0];
    g_Unknown4e3d24 = enemy->currentCtx->vars.floats[0];
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
    g_EtamaAnm->StartScript(&effect->sprite, 0x61);
    g_Stage.unk_625c = DrawTwinEffects;
}

void SpawnTwinEffects41(Enemy *enemy, EnemyScriptInstr *instr)
{
    Effect *effect;

    effect = g_EffectManager.SpawnReservedEffect(0x41, &enemy->pos, 9, 1, 0xffffffff);
    effect = g_EffectManager.SpawnReservedEffect(0x41, &enemy->pos, 10, 1, 0xffffffff);
    g_EtamaAnm->StartScript(&effect->sprite, 0x63);
    g_Stage.unk_625c = DrawTwinEffects;
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
    g_EtamaAnm->StartScript(&effect->sprite, 0x65);
    g_Stage.unk_625c = DrawTwinEffects;
}

void FlashAndShakeScreen(Enemy *enemy, EnemyScriptInstr *instr)
{
    ScreenEffect::StartEffect(3, 30, 5, 0x40ffffff, 0, 0x15);
    ScreenEffect::StartEffect(7, 4, 120, 190, 60, 0x15);
}

void SetUnknown18b8a2c(Enemy *enemy, EnemyScriptInstr *instr)
{
    g_Unknown18b8a2c = instr->args[1].i;
}

void TintStageRed(Enemy *enemy, EnemyScriptInstr *instr)
{
    g_Stage.SetTintColor(0xffc03030);
}

void ShakeScreenRamp(Enemy *enemy, EnemyScriptInstr *instr)
{
    ScreenEffect::StartEffect(7, 16, 20, 20, 20, 0x15);
}

void FlashScreen180(Enemy *enemy, EnemyScriptInstr *instr)
{
    ScreenEffect::StartEffect(3, 180, 1, 0xffffffff, 0, 0x15);
}

void StartSlowdown(Enemy *enemy, EnemyScriptInstr *instr)
{
    int divisor;
    float newSpeed;

    divisor = instr->args[1].i;
    newSpeed = 1.0f / divisor;
    g_App.gameSpeed = newSpeed;
}

void GetPracticeCard(Enemy *enemy, EnemyScriptInstr *instr)
{
    enemy->currentCtx->vars.ints[0] = g_GameManager.unk_3dbb0;
}

void StartResurrection(Enemy *enemy, EnemyScriptInstr *instr)
{
    g_SpellCard.Func415d10("「リザレクション」", enemy);
}

void SpellCardFunc416160(Enemy *enemy, EnemyScriptInstr *instr)
{
    g_SpellCard.Func416160();
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

void DropBombOrLifeItem(Enemy *enemy, EnemyScriptInstr *instr)
{
    if (g_Unknown17d6ed4)
    {
        g_ItemManager.SpawnItem(&enemy->pos, 3, 0);
    }
    else
    {
        g_ItemManager.SpawnItem(&enemy->pos, 5, 0);
    }
}
