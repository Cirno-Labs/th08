// The enemy scripts (ECL), from 0x418330, as th07's enemyscript.cpp.

#include <string.h>

#include "enemy.h"
#include "item.h"
#include "player.h"
#include "unsorted.h"

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
        return enemy->currentCtx->vars.ints3[0];
    case 10054:
        return enemy->currentCtx->vars.ints3[1];
    case 10055:
        return enemy->currentCtx->vars.ints3[2];
    case 10056:
        return enemy->currentCtx->vars.ints3[3];
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
        return enemy->currentCtx->vars.floats3[0];
    case 10058:
        return enemy->currentCtx->vars.floats3[1];
    case 10059:
        return enemy->currentCtx->vars.floats3[2];
    case 10060:
        return enemy->currentCtx->vars.floats3[3];
    case 10061:
        return g_EnemyScripts.globalInts[0];
    case 10062:
        return g_EnemyScripts.globalInts[1];
    case 10063:
        return g_EnemyScripts.globalInts[2];
    case 10064:
        return g_EnemyScripts.globalInts[3];
    case 10065:
        return g_EnemyScripts.globalFloats[0];
    case 10066:
        return g_EnemyScripts.globalFloats[1];
    case 10067:
        return g_EnemyScripts.globalFloats[2];
    case 10068:
        return g_EnemyScripts.globalFloats[3];
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
        return enemy->unk_2dd0.x;
    case 10075:
        return enemy->unk_2dd0.y;
    case 10076:
        return enemy->unk_2dd0.z;
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
        return &enemy->currentCtx->vars.ints3[0];
    case 10054:
        return &enemy->currentCtx->vars.ints3[1];
    case 10055:
        return &enemy->currentCtx->vars.ints3[2];
    case 10056:
        return &enemy->currentCtx->vars.ints3[3];
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
        return &g_EnemyScripts.globalInts[0];
    case 10062:
        return &g_EnemyScripts.globalInts[1];
    case 10063:
        return &g_EnemyScripts.globalInts[2];
    case 10064:
        return &g_EnemyScripts.globalInts[3];
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
        return enemy->currentCtx->vars.ints3[0];
    case 10054:
        return enemy->currentCtx->vars.ints3[1];
    case 10055:
        return enemy->currentCtx->vars.ints3[2];
    case 10056:
        return enemy->currentCtx->vars.ints3[3];
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
        return g_EnemyScripts.globalInts[0];
    case 10062:
        return g_EnemyScripts.globalInts[1];
    case 10063:
        return g_EnemyScripts.globalInts[2];
    case 10064:
        return g_EnemyScripts.globalInts[3];
    case 10065:
        return g_EnemyScripts.globalFloats[0];
    case 10066:
        return g_EnemyScripts.globalFloats[1];
    case 10067:
        return g_EnemyScripts.globalFloats[2];
    case 10068:
        return g_EnemyScripts.globalFloats[3];
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
        return enemy->currentCtx->vars.floats3[0];
    case 10058:
        return enemy->currentCtx->vars.floats3[1];
    case 10059:
        return enemy->currentCtx->vars.floats3[2];
    case 10060:
        return enemy->currentCtx->vars.floats3[3];
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
        return enemy->unk_2dd0.x;
    case 10075:
        return enemy->unk_2dd0.y;
    case 10076:
        return enemy->unk_2dd0.z;
    case 10079:
        return enemy->unk_2dc4.x;
    case 10080:
        return enemy->unk_2dc4.y;
    case 10081:
        return enemy->unk_2dc4.z;
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
        return &enemy->currentCtx->vars.floats3[0];
    case 10058:
        return &enemy->currentCtx->vars.floats3[1];
    case 10059:
        return &enemy->currentCtx->vars.floats3[2];
    case 10060:
        return &enemy->currentCtx->vars.floats3[3];
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
        return &g_EnemyScripts.globalFloats[0];
    case 10066:
        return &g_EnemyScripts.globalFloats[1];
    case 10067:
        return &g_EnemyScripts.globalFloats[2];
    case 10068:
        return &g_EnemyScripts.globalFloats[3];
    case 10074:
        return &enemy->unk_2dd0.x;
    case 10075:
        return &enemy->unk_2dd0.y;
    case 10076:
        return &enemy->unk_2dd0.z;
    case 10079:
        return &enemy->unk_2dc4.x;
    case 10080:
        return &enemy->unk_2dc4.y;
    case 10081:
        return &enemy->unk_2dc4.z;
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
